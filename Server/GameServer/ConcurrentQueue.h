#pragma once

#include <mutex>

template<typename T>
class LockQueue
{
public:
	// �����ڴ� �ƹ��͵� ����
	LockQueue() {}

	// ��������ڸ� ��������.
	LockQueue(const LockQueue&) = delete;
	LockQueue& operator=(const LockQueue&) = delete;

	void Push(T value)
	{
		lock_guard<mutex> lock(_mutex);
		_queue.push(std::move(value));
		_condVar.notify_one();
	}

	bool TryPop(T& value)
	{
		lock_guard<mutex> lock(_mutex);
		// �����Ͱ� ������ false
		if (_queue.empty())
			return false;

		// �����Ͱ� ������, top-> pop
		value = std::move(_queue.front());
		_queue.pop();
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
		_condVar.wait(lock, [this] {return _queue.empty() == false; });

		// �����Ͱ� ������, top-> pop
		value = std::move(_queue.front());
		_queue.pop();
	}


private:
	queue<T> _queue;
	mutex _mutex;
	condition_variable _condVar;
};