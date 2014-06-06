#include <time.h>
#include "ThreadPool.h"
#include "cmdline.h"

#include "OSThread.h"
#include "Log.h"

#include <set>


using namespace GSP;


static BOOL  s_bPrint = FALSE;
static BOOL s_bRuning = TRUE;
static BOOL s_bPause = FALSE;

#define TEST_FREE

#ifdef TEST_FREE
static std::set<UINT32> s_csFreeTest;
static UINT32  s_iFreeTest = 0;
static BOOL s_bPrintStatus = FALSE;





static void _FreeFunction( UINT32 iId )
{
	CGSPObject::g_csMutex.Lock();
    s_csFreeTest.erase(iId);
    CGSPObject::g_csMutex.Unlock();
}
#endif

static BOOL _iTEST = TRUE;

class ThreadPoolTester : public CGSPObject
{
   
public :
    INT m_iIndex;
    GSAtomicInter m_iCnts;
    INT32 m_bDelSelf ;
    ThreadPoolTester(INT iIndex)
        :CGSPObject()
        ,m_iIndex(iIndex)
        ,m_iCnts( 10 )
    {
         m_bDelSelf = 0;
    }
    void OnThreadPoolEvent(CObjThreadPool *pcsPool, void *pUserData )
    {
        INT iParam = (INT)pUserData;
        INT iSleep = 1+ (int) (100 * (rand() / (RAND_MAX + 1.0)));
#ifdef TEST_FREE
        _FreeFunction( (UINT32)pUserData );
#endif
	//	pcsPool->Disable();
        if( s_bPrint )
        {
            MY_DEBUG_PRINTF("OnThreadEvent: %d, [%d, %d], %d\n", m_iIndex, iParam, AtomicInterInc(m_iCnts), (int) COSThread::CurrentThreadID()  );
        }
//         MSLEEP(5);
//         if( m_iCnts > 1000 )
//         {   
//             pcsPool->Release();
//             if( CAtomic::G.CompareAndSet(m_bDelSelf, 0, 1) )
//             {
//                 MSLEEP(10);
//             }
//             
//             
//         }
        
    }

};


static void RandThreadEnter(CGSThread *csTheadHandle, void *pUserParam )
{

    UINT32 iParams = (UINT32)pUserParam;
    INT iSleep =  10;
    ThreadPoolTester **ppTester;
    INT iCounts = 0;
    INT bFlow = TRUE;
    INT iIndex=0;
    INT iBegin;
    int i = 0; 
    UINT32 iAdd;
    char czName[32];

    CGSPThreadPool **ppThreadPool;
    iCounts = iParams&0xFFFF;
    iIndex = (iParams>>16)&0xFFFF;
    iBegin = iIndex;

    ppThreadPool =  (CGSPThreadPool **)malloc(sizeof( CGSPThreadPool *)*(iCounts+1) );
    ppTester =  (ThreadPoolTester **)malloc(sizeof( ThreadPoolTester *)*(iCounts) );

    for(i=0; i<(iCounts+1); i++ )
    {
        ppThreadPool[i] = NULL;
    }
    for(i=0; i<iCounts; i++ )
    {
        GS_SNPRINTF(czName, 31, "THDTest%d", iIndex);
        ppTester[i] =  new ThreadPoolTester(iIndex++);
        GS_ASSERT_EXIT(ppTester[i]!=NULL, -1);
        ppThreadPool[i] = new CGSPThreadPool(czName);
		 GS_ASSERT_EXIT(ppThreadPool[i]!=NULL, -1);
		if( !ppThreadPool[i]->Init( ppTester[i],(FuncPtrObjThreadPoolEvent)&ThreadPoolTester::OnThreadPoolEvent,5, FALSE) )
		{
			 GS_ASSERT_EXIT(0, -1);
		}
        
#ifdef TEST_FREE
        ppThreadPool[i]->SetFreedTaskDataFunction((FuncPtrFree)_FreeFunction);
#endif

    }
    ppThreadPool[i] = NULL;

    //    MY_DEBUG_PRINTF("Create Thread usend thread: %d\n",1 );
    // pThreadPool->SetMaxWaitJobs(64);
    s_bRuning = 60;
    while(!csTheadHandle->TestExit() && s_bRuning  )
    {
    //    s_bRuning--;
        for( iIndex = 0, i=0; ppThreadPool[i] && _iTEST; i++)
        {
			if( s_bPrintStatus )
            {
                MY_DEBUG_PRINTF( "%d Thread Exist:%d\n",
                    i, CGSPThreadPool::MyGlobal()->GetGlobalThreads());
            }

            if( s_bPause )
            {
                 iIndex++;
                   continue;
            }
            if( ppTester[i]->m_bDelSelf )
            {
                iIndex++;
                continue;
            }

            if( ppThreadPool[i]->GetWaitTask() > 1000 )
            {

                iIndex++;
            }
            else
            {
#ifdef TEST_FREE
				CGSPObject::g_csMutex.Lock();
                iAdd =  s_iFreeTest++;
                ppTester[i]->m_iCnts++;
                s_csFreeTest.insert(iAdd);
                CGSPObject::g_csMutex.Unlock();

#else
        
                if( (void*)ppTester[i]->m_iCnts==GSP_INVALID_DATA_PTR )
                {
                    AtomicInterInc(ppTester[i]->m_iCnts);
                }
                iAdd = AtomicInterInc(ppTester[i]->m_iCnts);
 #endif

                if( ppThreadPool[i]->Task((void*) iAdd )  ) 
                {
                    MY_DEBUG_PRINTF("Fail %d add jobs.\n", iBegin+i);                    
                   // MSLEEP(iSleep);
                }
            }



        }

        if( s_bPrintStatus )
        {
            s_bPrintStatus = FALSE;
        }


        if( iIndex == i )
        {
            iSleep =  5 + (int) (20 * (rand() / (RAND_MAX + 1.0)));
            if(  bFlow )
            {
                MY_DEBUG_PRINTF("All thread flow out.\n");
                bFlow = FALSE;
            }
            MSLEEP(iSleep);
        }
	}

    for( i=0; _iTEST && ppThreadPool[i]; i++)
    {
     //   ppThreadPool[i]->Pause(FALSE);
        MY_DEBUG_PRINTF( "Discard: %d Thread Exist:%d\n",
            i,  CGSPThreadPool::MyGlobal()->GetGlobalThreads()  );
        if(  !ppTester[i]->m_bDelSelf )
        {

            ppThreadPool[i]->Uninit();
			delete ppThreadPool[i];
            ppThreadPool[i] = NULL;
        }
        delete ppTester[i];
        ppTester[i] = NULL;

    }
   csTheadHandle->Stop();
    MY_DEBUG_PRINTF("%d Exit\n", iBegin);
}

static int _OnStartTest(const char *czCmd,const char *args)
{
    CGSThread *pThread;
    static UINT16 iIndex = 0;
    UINT16 iCnts = 1;
    UINT32 iParams;

    s_bRuning = TRUE;
    s_bPause = FALSE;
    char *pParser = ArgsGetParser(args,NULL);
    if( pParser )
    {
        char *czKey, *czValue = NULL;
        int ret;
        ret=ParserFetch(&pParser, &czKey, &czValue );
        if( ret != 1 || !czKey )
        {
            return -1;
        }
        if( strcmp(czKey, "c")== 0 && czValue )
        {
            iCnts = atoi(czValue);
        }
    }
    if( iCnts<1 )
    {
        MY_DEBUG_PRINTF("Number can't < 1 \n");
        return -1;
    }

    iParams = iIndex;
    iParams <<= 16;
    iParams |= iCnts;

    pThread = new CGSThread();
    pThread->Start((GSThreadCallbackFunction)RandThreadEnter,(void*)iParams); 
    iIndex += iCnts;

    return 0;
}

static int _OnStopTest(const char *czCmd,const char *args)
{
    s_bRuning = FALSE;
    return 0;
}

static int _OnPauseResume(const char *czCmd,const char *args)
{
    s_bPause = !s_bPause;

    MY_DEBUG_PRINTF("Pause: %s\n", s_bPause ? "YES":"NO");
    return 0;
}

static int _OnPrintTest(const char *czCmd,const char *args)
{
    s_bPrintStatus = TRUE;
#ifdef TEST_FREE
	CGSPObject::g_csMutex.Lock();
    MY_DEBUG_PRINTF("Alloc object: %d.\n", s_csFreeTest.size() );
   CGSPObject::g_csMutex.Unlock();
#endif
    return 0;
}

static int _OnTurnOnTest(const char *czCmd,const char *args)
{
    s_bPrint = !s_bPrint;
    return 0;
}

static StruOptionLine _Options[]=
{
    {
        "-a",
            "-add",
            "add pthread pool to rand test. args: [-c:number] number defatul is 1.",
            _OnStartTest
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
                "trun on/off print.",
                _OnTurnOnTest
            },

            {
                "-p",
                    "-pause",
                    "trun on/off pause.",
                    _OnPauseResume
            },

             

            {
                NULL,
                    NULL,
                    NULL,
                    NULL,
            }
};


int TestThreadPool(const char *czCmd,const char *args)
{
    MY_DEBUG_PRINTF("Test thread pool enter..\n");
  //  CThreadPool::SetGlobalMaxThreads(13);
    OptionsEntery(_Options);
    MY_DEBUG_PRINTF("Test thread pool  leave..\n");

    return 0;


}
