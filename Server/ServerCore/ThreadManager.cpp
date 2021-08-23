#include "pch.h"
#include "ThreadManager.h"
#include "CoreTLS.h"
#include "CoreGlobal.h"


ThreadManager::ThreadManager()
{
	// Main Thread => 1번
	InitTLS();
}

ThreadManager::~ThreadManager()
{
	Join();
}

void ThreadManager::Launch(std::function<void(void)> callback)
{
	// 락을 걸어주고
	LockGuard guard(_lock);

	// 표준 스레드를 생성. 람다형태로 동시에 여러 함수를 실행하도록함.
	_threads.push_back(thread([=]()
	{
		InitTLS(); // 처음 스레드 생성시 TLS 생성
		callback(); // 콜백함수를 실행
		DestoryTLS(); // 끝나면 TLS 정리
	}));
}

void ThreadManager::Join()
{
	for (thread& t : _threads)
	{
		if (t.joinable())
			t.join();
	}
	// 다 실행했으면 clear
	_threads.clear();
}

void ThreadManager::InitTLS()
{
	static Atomic<uint32> SThreadId = 1;
	LThreadId = SThreadId.fetch_add(1);
}

void ThreadManager::DestoryTLS()
{

}