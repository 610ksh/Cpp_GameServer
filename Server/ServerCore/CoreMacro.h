#pragma once

#define OUT



/*-----------
	Lock
------------*/

#define USE_MANY_LOCKS(count)	Lock _locks[count];
#define USE_LOCK				USE_MANY_LOCKS(1)
#define READ_LOCK_IDX(idx)		ReadLockGuard readLockGuard_##idx(_locks[idx])
#define READ_LOCK				READ_LOCK_IDX(0)
#define WRITE_LOCK_IDX(idx)		WriteLockGuard writeLockGuard_##idx(_locks[idx])
#define WRITE_LOCK				WRITE_LOCK_IDX(0)


/*-----------
	Crash
------------*/


// define을 여러줄로 하고 싶다면, \를 추가하면 됨.
#define CRASH(cause)						\
{											\
	uint32* crash = nullptr;				\
	__analysis_assume(crash != nullptr);	\
	*crash = 0xDEADBEEF;					\
}


// 이 값이 true인지 아닌지 조건부로 crash를 발생시키는 매크로
#define ASSERT_CRASH(expr)					\
{											\
	if (!(expr))							\
	{										\
		CRASH("ASSERT_CRASH");				\
		__analysis_assume(expr);			\
	}										\
}



//// 선생
//#define CRASH(cause)						\
//{											\
//	uint32* crash = nullptr;				\
//	__analysis_assume(crash != nullptr);	\
//	*crash = 0xDEADBEEF;					\
//}
//
//#define ASSERT_CRASH(expr)			\
//{									\
//	if (!(expr))					\
//	{								\
//		CRASH("ASSERT_CRASH");		\
//		__analysis_assume(expr);	\
//	}								\
//}
