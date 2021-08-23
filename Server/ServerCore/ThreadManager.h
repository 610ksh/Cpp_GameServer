#pragma once

#include <thread>
#include <functional>


/*--------------------
	ThreadManager
---------------------*/

class ThreadManager
{
public:
	ThreadManager();
	~ThreadManager();

	// std::function을 사용한 이유는 온갖 함수, 람다등의 형태를 받아줄 수 있기 때문.
	// 펑터같은것보다 더 좋음.
	void Launch(std::function<void(void)> callback);
	void Join();

	static void InitTLS();
	static void DestoryTLS();

private:
	Mutex			_lock;
	vector<thread>	_threads;

};

