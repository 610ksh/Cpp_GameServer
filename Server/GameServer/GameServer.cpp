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

void Push()
{
	while (true)
	{
		// 100 미만의 랜덤 변수
		int32 value = rand() % 100;
		q.Push(value);

		this_thread::sleep_for(10ms); // 0.1초
	}
}

void Pop()
{
	while (true)
	{
		int32 data = 0;
		q.WaitPop(OUT data);
		cout << data << endl;

		/*if(q.TryPop(OUT data))
			cout << data << endl;*/
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