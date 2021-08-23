#include "pch.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"


ThreadManager* GThreadManager = nullptr;

class CoreGlobal
{
public:
	CoreGlobal()
	{
		GThreadManager = new ThreadManager();
	}
	~CoreGlobal()
	{
		delete GThreadManager;
	}
} GCoreGlobal;
// 위에처럼 마지막에 변수명을 적어주면 전여변수 하나를 생성해준 효과가 있음.