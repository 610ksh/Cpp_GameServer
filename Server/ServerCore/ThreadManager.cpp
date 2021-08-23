#include "pch.h"
#include "ThreadManager.h"
#include "CoreTLS.h"
#include "CoreGlobal.h"


ThreadManager::ThreadManager()
{
	// Main Thread => 1��
	InitTLS();
}

ThreadManager::~ThreadManager()
{
	Join();
}

void ThreadManager::Launch(std::function<void(void)> callback)
{
	// ���� �ɾ��ְ�
	LockGuard guard(_lock);

	// ǥ�� �����带 ����. �������·� ���ÿ� ���� �Լ��� �����ϵ�����.
	_threads.push_back(thread([=]()
	{
		InitTLS(); // ó�� ������ ������ TLS ����
		callback(); // �ݹ��Լ��� ����
		DestoryTLS(); // ������ TLS ����
	}));
}

void ThreadManager::Join()
{
	for (thread& t : _threads)
	{
		if (t.joinable())
			t.join();
	}
	// �� ���������� clear
	_threads.clear();
}

void ThreadManager::InitTLS()
{
	static Atomic<uint32> SThreadId = 1;
	LThreadId = SThreadId.fetch_add(1);
}

void ThreadManager::DestoryTLS()
{

}