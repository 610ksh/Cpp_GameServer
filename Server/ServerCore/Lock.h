#pragma once
#include "Types.h"

/*-------------
	RW SpinLock
-------------*/

/*--------------------------------------------------------
[WWWWWWWW][WWWWWWWW][RRRRRRRR][RRRRRRRR] (32bit)
상위 16비트 W, 하위 16비트 R.

W : WriteFlag (Exclusive Lock Owner ThreadId)
=> 현재 락을 획득한 쓰레드의 아이디를 기록.
R : ReadFlag (Shared Lock Count)
=> 공유하고 있는 read count를 기록. 몇명이 R하고있는지 카운팅.
--------------------------------------------------------*/
// W -> W (o)
// R -> R (o)
// W -> R (o)
// R -> W (x)
class Lock
{
	enum : uint32
	{
		ACQUIRE_TIMEOUT_TICK = 10000,
		MAX_SPIN_COUNT = 5000,
		WRITE_THREAD_MASK = 0xFFFF'0000,
		READ_COUNT_MASK = 0x0000'FFFF,
		EMPTY_FLAG = 0x0000'0000
	};

public:
	void WriteLock();
	void WriteUnlock();
	void ReadLock();
	void ReadUnlock();

private:
	Atomic<uint32> _lockFlag = EMPTY_FLAG;
	// 재귀적으로 락을 잡을 수 있도록 하기 위한 변수
	uint16 _writeCount = 0;
};


/*---------------------
	LockGuards
----------------------*/

class ReadLockGuard
{
public:
	ReadLockGuard(Lock& lock) : _lock(lock) { _lock.ReadLock(); }
	~ReadLockGuard() { _lock.ReadUnlock(); }
private:
	Lock& _lock;
};

class WriteLockGuard
{
public:
	WriteLockGuard(Lock& lock) : _lock(lock) { _lock.WriteLock(); }
	~WriteLockGuard() { _lock.WriteUnlock(); }
private:
	Lock& _lock;
};