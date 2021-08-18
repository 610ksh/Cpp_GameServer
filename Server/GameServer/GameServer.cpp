#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>

mutex m;
queue<int32> q;
HANDLE handle;

std::condition_variable cv;

void Producer()
{
	while (true)
	{
		{
			unique_lock<mutex> lock(m);
			q.push(100);
		}

		// ﻿조건변수를 통해 다른 쓰레드에게 통지
		cv.notify_one();
	}
}

void Consumer()
{
	while (true)
	{
		unique_lock<mutex> lock(m);
		// 람다를 통해 조건변수를 설정함. 언제 탈출하는지 조건을 설정.
		// 일반 함수를 넣어도 됨(Callable형태)
		cv.wait(lock, []() {return q.empty() == false; });
		// 내부구조 순서
		// 1. Lock을 잡으려고 시도.
		// 2. 조건을 확인함.
		// - 만족하면, 빠져나와서 이어서 코드를 실행.
		// - 조건이 만족하지 않으면, Lock을 풀어주고 대기 상태로 빠짐★

		//if (q.empty() == false)
		{
			int32 data = q.front();
			q.pop();
			cout << q.size() << endl;
		}
	}
}

int main()
{
	// window api로 이벤트 만듦.
	handle = ::CreateEvent(NULL/*보안속성*/, FALSE/*bManualReset*/, FALSE/*bInitialState*/, NULL/*이름을 정해주는것. 안해줘도됨*/);
	// bManualReset : true였다면, manual은 수동이라는 의미이기에 수동으로 리셋된다는 의미. false라면 autoReset으로 자동으로 리셋되는 이벤트 작동.
	// bInitialState : 이벤트의 초기상태를 지정함. FALSE라면 시그널이 없는 상태로 시작함. 불이 꺼진거. 
	// 커널쪽으로 가서 이벤트를 생성함. 그래서 이 이벤트를 커널 오브젝트라고 부르기도 함.
	// HANDLE : 이벤트는 HANDLE 형태로 반환됨. 우리가 사용하는 일반 정수와 같다. 숫자. 일종의 번호표. 이벤트끼리를 구별하는 용도.
	// 커널 오브젝트는 Usage Count라는 이 커널 오브젝트를 몇명이 사용하고 있는지를 나타냄.
	// Signal / Non-Signla(빨간불) <<- bool로 관리함. 여기까지는 거의 모든 커널 오브젝트가 가지고 있는 속성임.
	// Auto vs Manual은 이벤트관련 커널 오브젝트에 있는것. bool값으로 분리함.


	thread t1(Producer);
	thread t2(Consumer);

	t1.join();
	t2.join();

	// 핸들을 닫아주자. 일종의 소멸자.
	::CloseHandle(handle);
}