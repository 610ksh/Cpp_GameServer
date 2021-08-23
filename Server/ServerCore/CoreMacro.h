#pragma once

/*-----------
	Crash
------------*/


// define�� �����ٷ� �ϰ� �ʹٸ�, \�� �߰��ϸ� ��.
#define CRASH(cause)						\
{											\
	uint32* crash = nullptr;				\
	__analysis_assume(crash != nullptr);	\
	*crash = 0xDEADBEEF;					\
}


// �� ���� true���� �ƴ��� ���Ǻη� crash�� �߻���Ű�� ��ũ��
#define ASSERT_CRASH(expr)					\
{											\
	if (!(expr))							\
	{										\
		CRASH("ASSERT_CRASH");				\
		__analysis_assume(expr);			\
	}										\
}



//// ����
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
