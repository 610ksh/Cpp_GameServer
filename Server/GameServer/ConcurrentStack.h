#pragma once

#include <mutex>
#include <atomic>

template<typename T>
class LockStack
{
public:
	// �����ڴ� �ƹ��͵� ����
	LockStack() { }

	// Ȥ�� �����Ϸ��� �Ѵٸ� ���ƹ���.
	LockStack(const LockStack&) = delete;
	LockStack& operator=(const LockStack&) = delete;

	void Push(T value)
	{
		lock_guard<mutex> lock(_mutex); // lock�� ���
		_stack.push(std::move(value)); // �����͸� ����.
		// std::move�� �̿��� �����̶� �� ���� ó���� �� ������ ó����.

		_condVar.notify_one();
	}

	bool TryPop(T& value)
	{
		lock_guard<mutex> lock(_mutex);
		// �����Ͱ� ������ false
		if (_stack.empty())
			return false;

		// �����Ͱ� ������, top-> pop
		value = std::move(_stack.top());
		_stack.pop();
		return true;
	}

	/*
		���� �������� (Try)Pop�� �����Ͱ� ���������� while�� ���鼭 üũ�ϴµ�,
		�̴� �� ���� cpu �ڿ� �����̴�. �̰��� �ذ��ϱ� ���� Condition variable�� ����غ���.
		wait�� �ϵ���.
	*/

	void WaitPop(T& value)
	{
		unique_lock<mutex> lock(_mutex);
		// lock�� ���������ϰ�, ������ �����Ͱ� ���������� wait�Ѵ�. ����Ѵ�.
		// ������ ���� ������ ���� �ٽ� Ǯ���ְ� ���� �ȴ�. signal�� ��ٸ�.
		_condVar.wait(lock, [this] {return _stack.empty() == false; });

		// �����Ͱ� ������, top-> pop
		value = std::move(_stack.top());
		_stack.pop();
	}

	// ��Ƽ������ ȯ�濡���� Empty�� �ǹ̾���. �� ���̻��̿� ���������� ���� �� �����Ƿ�.
	bool Empty()
	{
		lock_guard<mutex> lock(_mutex); // lock�� ���
		return _stack.empty();
	}

private:
	stack<T> _stack;
	mutex _mutex;

	condition_variable _condVar;
};


template<typename T>
class LockFreeStack
{
	struct Node
	{
		Node(const T& value) :data(value), next(nullptr)
		{

		}

		T data;
		Node* next;
	};


public:
	// 1) �� ��带 �����
	// 2) �� ����� next = head
	// 3) head = �� ���

	// [ ] [ ] [ ] [ ] [ ] [ ]
	// [head]
	void Push(const T& value)
	{
		Node* node = new Node(value); // ��Ƽ�����忡�� ����x
		node->next = _head; // ��Ƽ�����忡�� ���� ����.

		// expected, desired
		// ���� ���� ������ �̷��� atomic ������ ������ �ޱ��Ѵ�.
		while (_head.compare_exchange_weak(node->next, node) == false)
		{
			// node->next = _head;
		}

		// �� ���̿� ��ġ�� ���ϸ�?
		//_head = node;
	}



	// [ ] [ ] [ ] [ ] [ ] [ ]
	// [head]

	// 1) head �б�
	// 2) head->next �б�
	// 3) head = head->next
	// 4) data �����ؼ� ��ȯ, value��.
	// 5) ������ ��带 ����
	bool TryPop(T& value)
	{
		++_popCount;

		// head�� �ٸ� �����嵵 �����ϴ� ��.
		Node* oldHead = _head;

		while (oldHead && _head.compare_exchange_weak(oldHead, oldHead->next) == false)
		{
			// �̷����ϸ� 1,2,3�ܰ谡 �ѹ��� ó���� �� ����.
		}

		if (oldHead == nullptr)
		{
			--_popCount;
			return false;
		}
		
		// -> ������� ������ oldHead�� ������ ����.

		// 4) ������ ����
		value = oldHead->data;

		// 5) ������ ��� ���� -> ������ ���Ƽ� ����.
		// delete oldHead;

		TryDelete(oldHead); //������ ���� �������� ������ üũ.
		
		return true;
	}

	void TryDelete(Node* oldHead)
	{
		// �� �ܿ� �ٸ� �����尡 �ִ��� üũ
		// �� ȥ���� ���
		if (_popCount == 1)
		{

			// �̿� ȥ���ΰ�, ���� ����� �ٸ� �����͵鵵 ����.
			// �����ִ� pendingList�� nullptr��, �׸��� �������� node�� ���޵�.
			Node* node = _pendingList.exchange(nullptr); // �����ϱ� ������ �׻� �̷��� �����͸� �и���.

			if (--_popCount == 0) // ���� 0�� ��. atomic�ϰ� �Ͼ.
			{
				// ����� �ְ� ���� -> ���� ���� ����.
				// �����ͼ� �ڴʰ� ����� ������ �����ʹ� �и��ص���.
				DeleteNodes(node);
			}
			else if (node)
			{
				// ���� ���������� �ٽ� ���� ����.
				ChainPendingNodeList(node);
			}

			// ������ ����
			delete oldHead;
		}
		// �ٸ��ְ� ������
		else
		{
			// ���� �����Ƿ�, ������ ���ุ��.
			ChainPendingNode(oldHead);
			--_popCount;
		}
	}
	// [ first ] [ ] [ ] [ last ] -����> [ ] [ ] (pendingList)
	void ChainPendingNodeList(Node* first, Node* last)
	{
		last->next = _pendingList;

		// CAS�Լ�
		while (_pendingList.compare_exchange_weak(last->next, first) == false)
		{

		}
	}

	// ù��° ��带 �ǳ��ָ�, ������ last�� ã�Ƽ� ���� ������ ��������ִ� �Լ�.
	void ChainPendingNodeList(Node* node)
	{
		Node* last = node;
		// next�� ����������, ������
		while (last->next)
		{
			last = last->next;
		}
		// ã������ �װɷ� ChaingPendingNodeList�� ����
		ChainPendingNodeList(node, last);
	}

	// �ϳ��ۿ� ������
	void ChainPendingNode(Node* node)
	{
		ChainPendingNodeList(node, node);
	}

	// node�� �����ϴ� ��带 ���� ��������.
	static void DeleteNodes(Node* node)
	{
		while (node)
		{
			Node* next = node->next;
			delete node;
			node = next;
		}
	}

private:
	atomic<Node*> _head;

	// for delete oldHead
	atomic<uint32> _popCount = 0; // pop�� �������� ������ ����
	atomic<Node*> _pendingList; // ���� �Ǿ���� ���� (ù��° ��常 ��� ������)
};