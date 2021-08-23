#include "pch.h"
#include "CoreTLS.h"


// 스레드 id를 우리식으로 지정해주자.
thread_local uint32 LThreadId = 0;