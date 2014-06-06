#include "ThreadPool.h"
#include "Log.h"
#include "OSThread.h"




using namespace  GSP;
/*
****************************************
brief :  CThreadPool 类的函数实现
****************************************
*/
// #if defined(_DEBUG) || defined(DEBUG)
// #define THREAD_POOL_TEST
// #endif

#ifdef   THREAD_POOL_TEST

#define THREAD_POOL_DEBUG  MY_DEBUG
static GSAtomicInter s_iTestNumber = 0;
#else
#define THREAD_POOL_DEBUG(...) do{}while(0)
#endif

CGSThreadGlobal *CGSPThreadPool::s_pThreadGlobal = NULL;

CGSPThreadPool::CGSPThreadPool( const char *czName) 
						 : CObjThreadPool(s_pThreadGlobal)
						 ,m_strName("ThreadPool")

{




	m_strName = "GspThreadPool";
	if( czName )
	{
		m_strName += "_";
		m_strName += czName;
	}
	
#ifdef  THREAD_POOL_TEST 
	GSStrUtil::AppendWithFormat(m_strName, "#%p", this);
	THREAD_POOL_DEBUG("%s Create. exist: %d\n", m_strName.c_str() ,
		AtomicInterInc(s_iTestNumber) );
#endif

}


CGSPThreadPool::~CGSPThreadPool(void)
{

#ifdef  THREAD_POOL_TEST 
	THREAD_POOL_DEBUG("%s Destroy. exist: %d\n", m_strName.c_str(), 
		AtomicInterDec(s_iTestNumber) );
#endif	

}

void CGSPThreadPool::InitModule(void)
{
	CGSThreadPool::InitModule();
	s_pThreadGlobal = CGSThreadGlobal::Create();
	if( s_pThreadGlobal )
	{
#ifndef _WINCE
		INT iThread;
		iThread = COSThread::CurrentDeviceCPUNumber();
		iThread = iThread*4+4;
		if( iThread<32 )
		{
			iThread = 32;
		}
		s_pThreadGlobal->SetMaxGlobalThread(iThread);
		s_pThreadGlobal->SetMaxIdleThreads(iThread+1);
#else
		s_pThreadGlobal->SetMaxGlobalThread(8);
		s_pThreadGlobal->SetMaxIdleThreads(8);
#endif
	}
}

void CGSPThreadPool::UninitModule(void)
{
	if( s_pThreadGlobal )
	{
		s_pThreadGlobal->Release();
		s_pThreadGlobal = NULL;
	}
	CGSThreadPool::UninitModule();
}

