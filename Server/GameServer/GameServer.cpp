#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <Windows.h>
#include <future>
#include "ThreadManager.h"

class TestLock
{
	USE_LOCK;

public:
	int32 TestRead()
	{
		READ_LOCK;


		if (_queue.empty())
			return -1;

		return _queue.front();
	}

	void TestPush()
	{
		WRITE_LOCK;
		
		_queue.push(rand() % 100);
	}

	void TestPop()
	{
		WRITE_LOCK;

		while (true)
		{

		}

		if (_queue.empty() == false)
			_queue.pop();
	}

private:
	queue<int32> _queue;

};


TestLock testLock;

void ThreadWrite()
{
	while (true)
	{
		testLock.TestPush();
		this_thread::sleep_for(1ms);
		testLock.TestPop();
	}
}

void ThreadRead()
{
	while (true)
	{
		int32 value = testLock.TestRead();
		cout << value << endl;
		this_thread::sleep_for(1ms);
	}
}

int main()
{
	// 메인 스레드는 1번, 나머지는 2번부터 6번까지 생성됨.
	for (int32 i = 0; i < 2; ++i)
	{
		GThreadManager->Launch(ThreadWrite);
	}

	for (int32 i = 0; i < 5; ++i)
	{
		GThreadManager->Launch(ThreadRead);
	}

	GThreadManager->Join();

}