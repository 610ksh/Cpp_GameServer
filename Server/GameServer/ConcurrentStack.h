#pragma once

#include <mutex>

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