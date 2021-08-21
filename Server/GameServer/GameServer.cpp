#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>

#include "ConcurrentQueue.h"
#include "ConcurrentStack.h"

LockQueue<int32> q;
LockStack<int32> s;
LockFreeStack<int32> s2;

void Push()
{
	while (true)
	{
		// 100 미만의 랜덤 변수
		int32 value = rand() % 100;
		s2.Push(value);

		this_thread::sleep_for(10ms); // 0.1초
	}
}

void Pop()
{
	while (true)
	{
		int32 data = 0;
		//s2.WaitPop(OUT data);
		//cout << data << endl;

		if(s2.TryPop(OUT data))
			cout << data << endl;
	}
}


int main()
{
	thread t1(Push);
	thread t2(Pop);
	thread t3(Pop);

	t1.join();
	t2.join();
	t3.join(); 
}