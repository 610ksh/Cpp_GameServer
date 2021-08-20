#pragma once

#include <mutex>

template<typename T>
class LockQueue
{
public:
	// 생성자는 아무것도 안함
	LockQueue() {}

	// 복사생성자를 막아주자.
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
		// 데이터가 없으면 false
		if (_queue.empty())
			return false;

		// 데이터가 있으면, top-> pop
		value = std::move(_queue.front());
		_queue.pop();
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
		_condVar.wait(lock, [this] {return _queue.empty() == false; });

		// 데이터가 있으면, top-> pop
		value = std::move(_queue.front());
		_queue.pop();
	}


private:
	queue<T> _queue;
	mutex _mutex;
	condition_variable _condVar;
};