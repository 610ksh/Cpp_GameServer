#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <Windows.h>
#include <future>
#include "ThreadManager.h"
#include "CoreMacro.h"

CoreGlobal Core;

void ThreadMain()
{
	while (true)
	{
		cout << "Hello! I am thread ..." << LThreadId << endl;
		this_thread::sleep_for(1s);
	}
}

int main()
{
	// 메인 스레드는 1번, 나머지는 2번부터 6번까지 생성됨.
	for (int32 i = 0; i < 5; ++i)
	{
		GThreadManager->Launch(ThreadMain);
	}
	GThreadManager->Join();

}