#pragma once

#include <mutex>

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