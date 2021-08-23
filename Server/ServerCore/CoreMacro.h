#pragma once

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
