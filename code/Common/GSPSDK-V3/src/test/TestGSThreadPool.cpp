/*
*********************************************************************
*
*@brief :  测试公共线程池
*
*********************************************************************
*/
#include "GSCommon.h"
#include "cmdline.h"
#include <vector>
#include <set>
#include "GSPObject.h"

using namespace GSP;


static BOOL  s_bPrint = FALSE;
static BOOL s_bRuning = FALSE;
static BOOL s_bPuase = FALSE;
static std::vector<CGSThreadPool *> s_vThreadPool;
static std::vector<CGSThread *> s_vRandThread;
static GSAtomicInter s_iAtomic = 0;
static CGSMutex s_csMutex;
static FILE *s_pFile = NULL;

#define TEST_FREE
#ifdef TEST_FREE

static std::set<UINT32> s_csFreeTest;
static UINT32  s_iFreeTest = 0;

void _PushTestFree( UINT32 iId )
{
	s_csMutex.Lock();
	s_csFreeTest.insert( iId);
	s_csMutex.Unlock();
}

void _FreeFunction( void *iId )
{
	s_csMutex.Lock();
	s_csFreeTest.erase( (UINT32) iId);
	s_csMutex.Unlock();

}
#endif

static  void OnThreadPoolEvent(CGSThreadPool *pcsPool, void *pTaskData, void *pDebugInfo )
{

	UINT32 iVal = (UINT32)pTaskData;
#ifdef TEST_FREE
	_FreeFunction(pTaskData);
#endif
	char czTemp[512];
	sprintf_s( czTemp, 511, "P:0x%02x =>T:%p, Data: 0x%08x, <* %lld *>\n",
		(INT)pcsPool->GetUserData(), pDebugInfo, iVal , (long long) DoGetTickCount() );
	if( s_bPrint )
	{
		GSP_PRINTF("%s", czTemp );
	}

	if( s_pFile )
	{		
		s_csMutex.Lock();		
		fputs(czTemp, s_pFile);
		s_csMutex.Unlock();
	}

}

static void RandThreadEnter(CGSThread *csTheadHandle, void *pUserParam )
{
	INT iIndex = (INT) pUserParam;
	INT iSize = s_vThreadPool.size();
	INT iRand;
	INT i;
	INT iCnts;


	
	while(!csTheadHandle->TestExit() && s_bRuning )
	{
		iCnts = 0;
		for(  i = 0; !s_bPuase && i<iSize; i++ )
		{
			UINT32 iVal = (UINT32) AtomicInterInc(s_iAtomic);
			iVal <<= 24;
			iVal |= (iIndex&0xFF);
			if( !s_vThreadPool[i]->Task((void*)iVal) )
			{
				iCnts++;
			}			
		}
		if( iCnts == 0 )
		{
			if( s_bPrint )
			{
				GSP_PRINTF("Add task flowout.\n");
			}
			iRand = 1+(int) (500.0*(rand()/(RAND_MAX+1.0)));
			MSLEEP(iRand);
		}
	}
}

static int _OnStartTest(const char *czCmd,const char *args)
{
	if( s_bRuning )
	{
		GSP_PRINTF("Tester is running\n");
		return -1;
	}
	s_bRuning = TRUE;

	CGSThread *pThread;
	UINT16 iIndex = 0;
	UINT16 iCnts = 1;
	


	s_bRuning = TRUE;
	char *pParser = ArgsGetParser(args,NULL);
	if( pParser )
	{
		char *czKey, *czValue = NULL;
		int ret;
		while( (ret=ParserFetch(&pParser, &czKey, &czValue ) )==1 )
		{
			if( 0==strcmp("c", czKey ) )
			{
				//添加的个数
				if( !czValue )
				{
					return -1;
				}
				iCnts = atoi(czValue);
			}  
			else if( 0==strcmp("f", czKey) )
			{
				char *czFilename = "./debug.txt";
				if( czValue )
				{
					czFilename = czValue;
				}
				s_pFile = fopen(czFilename, "w+");
				if( s_pFile )
				{
					GSP_PRINTF("Set ouput debug info to '%s'\n", czFilename);

				}
				else
				{
					GSP_PRINTF("Err Open '%s' fail.\n", czFilename);
				}
			}
			else
			{
				return -1;
			}
		}
	}

	if( iCnts<1 )
	{
		GSP_PRINTF("Number can't < 1 \n");
		return -1;
	}
	for( INT i = 0; i<iCnts; i++ )
	{
		CGSThreadPool *pPool = new CGSThreadPool();
		assert(pPool);
		pPool->SetUserData( (void*)i );
		if( !pPool->Init(OnThreadPoolEvent, 2, i<5 ) )
		{
			
			GSP_PRINTF("Thread Pool Init fail.\n");
			assert(0);
			delete pPool;
		}
		else
		{
			pPool->SetMaxWaitTask( 1000 );
			pPool->SetIdleWait(-1);
#ifdef TEST_FREE
			pPool->SetFreedTaskDataFunction(_FreeFunction);
#endif

			s_vThreadPool.push_back(pPool);
		}
	}

	for( INT i = 0; i< 3; i++ )
	{
		pThread = new CGSThread();
		pThread->Start((GSThreadCallbackFunction)RandThreadEnter,(void*)i); 
		s_vRandThread.push_back(pThread);
	}
	return 0;
}

static int _OnStopTest(const char *czCmd,const char *args)
{
	if( !s_bRuning )
	{
		GSP_PRINTF("Test not runnnig\n");
		return 0;
	}
	s_bRuning = FALSE;
INT iCnt = s_vRandThread.size();
	for( INT i = 0; i<iCnt; i++ )
	{
		
		s_vRandThread[i]->Join(3000);
		assert( !s_vRandThread[i]->IsRunning());
		MSLEEP(10);
		delete s_vRandThread[i];
	}
	s_vRandThread.clear();

	iCnt = s_vThreadPool.size();
	for( INT i = 0; i<iCnt; i++ )
	{
		s_vThreadPool[i]->Uninit();
		delete s_vThreadPool[i];

	}
	s_vThreadPool.clear();
#ifdef TEST_FREE
	s_csMutex.Lock();
	GSP_PRINTF("**FreeDatas Exists: %d\n", s_csFreeTest.size() );
	if( s_pFile )
	{
		fclose(s_pFile);
		s_pFile = NULL;
	}
	s_csMutex.Unlock();
#endif
	
	return 0;
}

static int _OnPuaseTest(const char *czCmd,const char *args)
{
	s_bPuase = !s_bPuase;
	GSP_PRINTF("*Set Data ** %s\n", s_bPuase ? "Pause" : "Resume");
	return 0;
}

static int _OnTurnOnTest(const char *czCmd,const char *args)
{
	s_bPrint = !s_bPrint;
	return 0;
}


static int _OnPrintTest(const char *czCmd,const char *args)
{
// 	GSP_PRINTF("ThreadPool Global Exist: %d Unuse:%d\n", 
// 		CGSThreadGlobal::Global()->GetGlobalThreads(),
// 		CGSThreadGlobal::Global()->GetIdleThreads() );
#ifdef TEST_FREE
	s_csMutex.Lock();
	GSP_PRINTF("FreeDatas Exists: %d.\n", s_csFreeTest.size() );
	s_csMutex.Unlock();
#endif
	return 0;
}


static StruOptionLine _Options[]=
{
	{
		"-a",
			"-add",
			"add pthread pool to rand test. \n\
			 -c:number   Manay thread pool to test, defatul is 1. \n\
			 -f:filename Enable output information to file,  filename default is ./debug.txt.", 
			_OnStartTest
	},
	{
		"-p",
		"-pause",
		"Trun on/off pause set task to threadpool.\n",
		_OnPuaseTest,
	},
	{
		"-q",
			"-quit",
			"stop test.",
			_OnStopTest
		},
		{
			"-pt",
				"-print",
				"print info.",
				_OnPrintTest
		},
		{
			"-o",
				"-on",
				"Trun on/off print.",
				_OnTurnOnTest
			},


			{
				NULL,
					NULL,
					NULL,
					NULL,
			}
};


int TestGSThreadPool(const char *czCmd,const char *args)
{
	GSP_PRINTF("Test gosun thread pool enter..\n");
	CGSThreadPool::InitModule();
//	CGSThreadGlobal::Global()->SetMaxIdleThreads(64);
//	CGSThreadGlobal::Global()->SetMaxGlobalThread(32);
	OptionsEntery(_Options);
	_OnStopTest("", "");
	CGSThreadPool::UninitModule();
	GSP_PRINTF("Test gosun thread pool  leave..\n");

	return 0;


}