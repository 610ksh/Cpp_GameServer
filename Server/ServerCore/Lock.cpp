#include "pch.h"
#include "Lock.h"
#include "CoreTLS.h" // Added
#include "DeadLockProfiler.h"

void Lock::WriteLock(const char* name)
{

	// DEBUG �����϶���
#if _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif

	// ��å : ������ �����尡 �����ϰ� �ִٸ� ������ ����.
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (LThreadId == lockThreadId)
	{
		_writeCount++;
		return;
	}

	// �ǵ������� ũ������ ������ ����. �ð��� üũ�غ���. 64bit
	const int64 beginTick = ::GetTickCount64(); // ���� �ð��� ���

	// �ƹ��� ���� �� �����ϰ� ���� ���� ��, �����ؼ� �������� ��´�.
	// ������ ���̵� 16��Ʈ �̵���Ŵ.
	const uint32 desired = ((LThreadId << 16) & WRITE_THREAD_MASK);
	// ���� ���
	while (true)
	{
		// 5000�� �õ�.
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; ++spinCount)
		{
			// ���� �� �� ����
			uint32 expected = EMPTY_FLAG;
			if (_lockFlag.compare_exchange_strong(OUT expected, desired))
			{
				// ���տ��� �̱� ����. ���� ȹ��.
				_writeCount++; // 
				return;
			}
		}

		// ������� �ɸ� �ð��� ACQUIRE_TIMEOUT_TICK ���� ��ٸ� ũ���� �߻�
		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
			CRASH("LOCK_TIMEOUT"); // ũ������ �߻���ų ����.

		this_thread::yield(); // �������� ������. ���ؽ�Ʈ ����Ī.
	}
}

void Lock::WriteUnlock(const char* name)
{

	// DEBUG �����϶���
#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif

	// ReadLock�� �� Ǯ�� ������ WriteUnlock�� �Ұ���.
	if ((_lockFlag.load() & READ_COUNT_MASK) != 0)
		CRASH("INVALID_UNLOCK_ORDER");

	const int32 lockCount = --_writeCount;
	if (lockCount == 0)
		_lockFlag.store(EMPTY_FLAG); // 0���� �о���. ����.
}

void Lock::ReadLock(const char* name)
{

	// DEBUG �����϶���
#if _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif

	// ������ �����尡 �����ϰ� �ִٸ� ������ ����
	// ��å : ������ �����尡 �����ϰ� �ִٸ� ������ ����.
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (LThreadId == lockThreadId)
	{
		_lockFlag.fetch_add(1);
		return;
	}


	// �ƹ��� ���� �����ϰ� ���� ���� �� �����ؼ� ���� ī��Ʈ�� �ø���. ����� ����ִ���.

	// �ǵ������� ũ������ ������ ����. �ð��� üũ�غ���. 64bit
	const int64 beginTick = ::GetTickCount64(); // ���� �ð��� ���

	while (true)
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; ++spinCount)
		{
			uint32 expected = (_lockFlag.load() & READ_COUNT_MASK);
			if (_lockFlag.compare_exchange_strong(OUT expected, expected + 1))
				return;
		}

		// ������� �ɸ� �ð��� ACQUIRE_TIMEOUT_TICK ���� ��ٸ� ũ���� �߻�
		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
			CRASH("LOCK_TIMEOUT"); // ũ������ �߻���ų ����.

		this_thread::yield(); // �������� ������. ���ؽ�Ʈ ����Ī.
	}
}

void Lock::ReadUnlock(const char* name)
{
	// DEBUG �����϶���
#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif


	// �Ʒ��� ���� ���� _lockFlag���� 1�� ���� �� ������ �񱳰� �ȴ�.
	if ((_lockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0)
		CRASH("MULTPLE_UNLOCK");
}
