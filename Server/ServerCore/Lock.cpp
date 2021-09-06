#include "pch.h"
#include "Lock.h"
#include "CoreTLS.h" // Added
#include "DeadLockProfiler.h"

void Lock::WriteLock(const char* name)
{

	// DEBUG 상태일때만
#if _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif

	// 정책 : 동일한 쓰레드가 소유하고 있다면 무조건 성공.
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (LThreadId == lockThreadId)
	{
		_writeCount++;
		return;
	}

	// 의도적으로 크래쉬를 내려고 만듦. 시간을 체크해보자. 64bit
	const int64 beginTick = ::GetTickCount64(); // 현재 시간을 기록

	// 아무도 소유 및 공유하고 있지 않을 때, 경합해서 소유권을 얻는다.
	// 스레드 아이디를 16비트 이동시킴.
	const uint32 desired = ((LThreadId << 16) & WRITE_THREAD_MASK);
	// 스핀 방식
	while (true)
	{
		// 5000번 시도.
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; ++spinCount)
		{
			// 락이 텅 빈 상태
			uint32 expected = EMPTY_FLAG;
			if (_lockFlag.compare_exchange_strong(OUT expected, desired))
			{
				// 경합에서 이긴 상태. 락을 획득.
				_writeCount++; // 
				return;
			}
		}

		// 여기까지 걸린 시간이 ACQUIRE_TIMEOUT_TICK 보다 길다면 크래쉬 발생
		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
			CRASH("LOCK_TIMEOUT"); // 크래쉬를 발생시킬 예정.

		this_thread::yield(); // 소유권을 포기함. 컨텍스트 스위칭.
	}
}

void Lock::WriteUnlock(const char* name)
{

	// DEBUG 상태일때만
#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif

	// ReadLock을 다 풀기 전에는 WriteUnlock이 불가능.
	if ((_lockFlag.load() & READ_COUNT_MASK) != 0)
		CRASH("INVALID_UNLOCK_ORDER");

	const int32 lockCount = --_writeCount;
	if (lockCount == 0)
		_lockFlag.store(EMPTY_FLAG); // 0으로 밀어줌. 리셋.
}

void Lock::ReadLock(const char* name)
{

	// DEBUG 상태일때만
#if _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif

	// 동일한 쓰레드가 소유하고 있다면 무조건 성공
	// 정책 : 동일한 쓰레드가 소유하고 있다면 무조건 성공.
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (LThreadId == lockThreadId)
	{
		_lockFlag.fetch_add(1);
		return;
	}


	// 아무도 락을 소유하고 있지 않을 때 경합해서 공유 카운트를 올린다. 몇명이 잡고있는지.

	// 의도적으로 크래쉬를 내려고 만듦. 시간을 체크해보자. 64bit
	const int64 beginTick = ::GetTickCount64(); // 현재 시간을 기록

	while (true)
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; ++spinCount)
		{
			uint32 expected = (_lockFlag.load() & READ_COUNT_MASK);
			if (_lockFlag.compare_exchange_strong(OUT expected, expected + 1))
				return;
		}

		// 여기까지 걸린 시간이 ACQUIRE_TIMEOUT_TICK 보다 길다면 크래쉬 발생
		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
			CRASH("LOCK_TIMEOUT"); // 크래쉬를 발생시킬 예정.

		this_thread::yield(); // 소유권을 포기함. 컨텍스트 스위칭.
	}
}

void Lock::ReadUnlock(const char* name)
{
	// DEBUG 상태일때만
#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif


	// 아래와 같은 경우는 _lockFlag값이 1을 빼기 전 값으로 비교가 된다.
	if ((_lockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0)
		CRASH("MULTPLE_UNLOCK");
}
