#pragma once

#include <mutex>
#include <atomic>

template<typename T>
class LockStack
{
public:
	// 생성자는 아무것도 안함
	LockStack() { }

	// 혹시 복사하려고 한다면 막아버림.
	LockStack(const LockStack&) = delete;
	LockStack& operator=(const LockStack&) = delete;

	void Push(T value)
	{
		lock_guard<mutex> lock(_mutex); // lock을 잡고
		_stack.push(std::move(value)); // 데이터를 넣음.
		// std::move를 이용해 조금이라도 더 빨리 처리할 수 있으면 처리함.

		_condVar.notify_one();
	}

	bool TryPop(T& value)
	{
		lock_guard<mutex> lock(_mutex);
		// 데이터가 없으면 false
		if (_stack.empty())
			return false;

		// 데이터가 있으면, top-> pop
		value = std::move(_stack.top());
		_stack.pop();
		return true;
	}

	/*
		위의 전통적인 (Try)Pop은 데이터가 있을때까지 while을 돌면서 체크하는데,
		이는 꽤 많은 cpu 자원 낭비이다. 이것을 해결하기 위해 Condition variable을 사용해보자.
		wait을 하도록.
	*/

	void WaitPop(T& value)
	{
		unique_lock<mutex> lock(_mutex);
		// lock을 잡으려고하고, 스택의 데이터가 있을때까지 wait한다. 대기한다.
		// 조건이 맞지 않으면 락을 다시 풀어주고 잠들게 된다. signal을 기다림.
		_condVar.wait(lock, [this] {return _stack.empty() == false; });

		// 데이터가 있으면, top-> pop
		value = std::move(_stack.top());
		_stack.pop();
	}

	// 멀티쓰레딩 환경에서는 Empty가 의미없다. 그 사이사이에 경합조건이 생길 수 있으므로.
	bool Empty()
	{
		lock_guard<mutex> lock(_mutex); // lock을 잡고
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
	// 1) 새 노드를 만들고
	// 2) 새 노드의 next = head
	// 3) head = 새 노드

	// [ ] [ ] [ ] [ ] [ ] [ ]
	// [head]
	void Push(const T& value)
	{
		Node* node = new Node(value); // 멀티쓰레드에선 에러x
		node->next = _head; // 멀티쓰레드에서 에러 가능.

		// expected, desired
		// 락이 없을 뿐이지 이러한 atomic 연산의 도움을 받긴한다.
		while (_head.compare_exchange_weak(node->next, node) == false)
		{
			// node->next = _head;
		}

		// 이 사이에 새치기 당하면?
		//_head = node;
	}



	// [ ] [ ] [ ] [ ] [ ] [ ]
	// [head]

	// 1) head 읽기
	// 2) head->next 읽기
	// 3) head = head->next
	// 4) data 추출해서 반환, value로.
	// 5) 추출한 노드를 삭제
	bool TryPop(T& value)
	{
		++_popCount;

		// head는 다른 스레드도 접근하는 것.
		Node* oldHead = _head;

		while (oldHead && _head.compare_exchange_weak(oldHead, oldHead->next) == false)
		{
			// 이렇게하면 1,2,3단계가 한번에 처리될 수 있음.
		}

		if (oldHead == nullptr)
		{
			--_popCount;
			return false;
		}
		
		// -> 여기까지 왔으면 oldHead를 선점한 상태.

		// 4) 데이터 추출
		value = oldHead->data;

		// 5) 추출한 노드 삭제 -> 문제가 많아서 보류.
		// delete oldHead;

		TryDelete(oldHead); //하지만 누가 끼어들수도 있으니 체크.
		
		return true;
	}

	void TryDelete(Node* oldHead)
	{
		// 나 외에 다른 스레드가 있는지 체크
		// 나 혼자인 경우
		if (_popCount == 1)
		{

			// 이왕 혼자인거, 삭제 예약된 다른 데이터들도 삭제.
			// 원래있던 pendingList는 nullptr로, 그리고 기존값은 node로 전달됨.
			Node* node = _pendingList.exchange(nullptr); // 삭제하기 전에는 항상 이렇게 데이터를 분리함.

			if (--_popCount == 0) // 빼고 0과 비교. atomic하게 일어남.
			{
				// 끼어든 애가 없음 -> 삭제 진행 가능.
				// 이제와서 뒤늦게 끼어들어도 어차피 데이터는 분리해뒀음.
				DeleteNodes(node);
			}
			else if (node)
			{
				// 누가 끼어들었으니 다시 갖다 놓자.
				ChainPendingNodeList(node);
			}

			// 데이터 삭제
			delete oldHead;
		}
		// 다른애가 있을때
		else
		{
			// 누가 있으므로, 삭제를 예약만함.
			ChainPendingNode(oldHead);
			--_popCount;
		}
	}
	// [ first ] [ ] [ ] [ last ] -연결> [ ] [ ] (pendingList)
	void ChainPendingNodeList(Node* first, Node* last)
	{
		last->next = _pendingList;

		// CAS함수
		while (_pendingList.compare_exchange_weak(last->next, first) == false)
		{

		}
	}

	// 첫번째 노드를 건네주면, 마지막 last를 찾아서 위의 버전을 실행시켜주는 함수.
	void ChainPendingNodeList(Node* node)
	{
		Node* last = node;
		// next가 없을때까지, 끝까지
		while (last->next)
		{
			last = last->next;
		}
		// 찾았으면 그걸로 ChaingPendingNodeList를 실행
		ChainPendingNodeList(node, last);
	}

	// 하나밖에 없을때
	void ChainPendingNode(Node* node)
	{
		ChainPendingNodeList(node, node);
	}

	// node로 시작하는 노드를 전부 날려버림.
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
	atomic<uint32> _popCount = 0; // pop을 실행중인 쓰레드 개수
	atomic<Node*> _pendingList; // 삭제 되어야할 노드들 (첫번째 노드만 들고 있을것)
};