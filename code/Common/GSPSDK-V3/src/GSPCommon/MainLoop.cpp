#include "MainLoop.h"
#include "Log.h"
#include "MyObjectDebug.h"
using namespace GSP;


/*
****************************************
brief : CWatchTimer 的实现
****************************************
*/
DEFINE_OBJ_DEBUG(CWatchTimer)



#define	stTIMER_NORMAL		0x00
#define	stTIMER_RUNING		0x01       
#define	stTIMER_STOP		0x02
#define	stTIMER_ASSERT      0x04
	


CWatchTimer::CWatchTimer(void)
:CGSPObject()
,m_iInterval(1000)
,m_eStatus(stTIMER_STOP)
,m_pMainLoop(NULL)
{
	OBJ_DEBUG_NEW(CWatchTimer)

	
	m_iTimerID = -1;
	m_fnCallback = NULL;
	m_pCbOwner =  NULL;
    m_iLastTime = DoGetTickCount();
	OBJ_DEBUG_NEW(CWatchTimer);
	
}



CWatchTimer::~CWatchTimer(void)
{
	AtomicInterOr(m_eStatus,stTIMER_ASSERT);
	m_csTask.Uninit();
CMainLoop *p = m_pMainLoop;
	if( m_pMainLoop && p )
	{
		p->RemoveTimer(this);
	}
	else 
	{
		GS_ASSERT(!IsReady());
	}
	OBJ_DEBUG_DEL(CWatchTimer);
}

void CWatchTimer::Init(CGSPObject *pCbFuncOwner,  FuncPtrTimerCallback fnOnEvent,
		  TimerID_t iTimerID, UINT32 iInterval,
		  BOOL bInitStart, CMainLoop *pMainLoop)
{
	GS_ASSERT_RET(NULL==m_pMainLoop);
	m_iTimerID = iTimerID;
	m_pMainLoop = pMainLoop;
	m_iInterval = iInterval;
	if( !m_pMainLoop )
	{
		m_pMainLoop = CMainLoop::Global();
		GS_ASSERT_EXIT(m_pMainLoop, -1);
	}


	if(  bInitStart )
	{
		m_eStatus = stTIMER_NORMAL;
	}

	m_fnCallback = fnOnEvent;
	m_pCbOwner = pCbFuncOwner;

	if( !m_csTask.Init(this, (FuncPtrObjThreadPoolEvent)&CWatchTimer::OnTaskEvent, 1, FALSE) )
	{
		GS_ASSERT_EXIT(0,-1);
		m_eStatus = stTIMER_ASSERT;
	}
	else
	{
		m_csTask.SetMaxWaitTask(1);
		m_pMainLoop->AddTimer(this);
	}
}

void CWatchTimer::OnTaskEvent( CObjThreadPool *pcsPool, void *pTaskData )
{
	if( m_pCbOwner )
	{
		(m_pCbOwner->* m_fnCallback)(this);
	}
	m_iLastTime = DoGetTickCount();
	AtomicInterAnd(m_eStatus, ~stTIMER_RUNING );
}

void CWatchTimer::Test( UINT64 iCurTime )
{

    INT64 iElapsed;
    iElapsed = iCurTime-m_iLastTime;
    BOOL bRet = TRUE;
    if( m_eStatus ) 
    {
       bRet = FALSE;
    }
    else if( iElapsed<0 )
    {
        //时间进行了调整
        m_iLastTime = iCurTime;
        bRet = FALSE;
    } 
    else  if( iElapsed < (INT64)m_iInterval ) 
	{
        bRet = FALSE;
    } 

    if( bRet && m_eStatus==0 )
	{	
		if( 0 == (AtomicInterOr(m_eStatus, stTIMER_RUNING)&stTIMER_RUNING ) )
		{
			m_csTask.Task((void*) this, TRUE);
		}
	}
}    

void CWatchTimer::Stop(void)
{    
   AtomicInterOr(m_eStatus, stTIMER_STOP);
   m_csTask.Disable();

}

BOOL CWatchTimer::IsReady(void)
{    
	return 0==(m_eStatus&stTIMER_ASSERT);

}

void  CWatchTimer::Start(void)
{
	m_csTask.Enable();
    m_iLastTime =  DoGetTickCount();
	AtomicInterAnd(m_eStatus, ~stTIMER_STOP );
	
   
}

BOOL CWatchTimer::AlterTimer(UINT32 iNewInterval)
{
     m_iInterval =  iNewInterval;     
     return TRUE;
}


/*
****************************************
brief :  CMainLoop 的实现
****************************************
*/
CMainLoop* CMainLoop::s_pGMainLoop = NULL;

CMainLoop::CMainLoop(INT iThreadNum )
:CGSPObject()
,m_csThPool("MainlLoop")
,m_csTimerList()
,m_csListMutex()
,m_bRun(TRUE)
,m_bReady(FALSE)
{
	iThreadNum += 1;
	GS_ASSERT_EXIT(iThreadNum>1, -1);

	m_bReady = m_csThPool.Init(this, (FuncPtrObjThreadPoolEvent)&CMainLoop::OnThreadEntry, 
								1, TRUE);
	GS_ASSERT_EXIT(m_bReady, -1);

	if( m_bReady )
	{
		if(CGSPThreadPool::RSUCCESS != m_csThPool.Task((void*)1) )
		{
			m_bReady = FALSE;
			GS_ASSERT_EXIT(m_bReady, -1);
		}
	}

}

CMainLoop::~CMainLoop(void)
{
	m_bRun = FALSE;
	m_bReady = FALSE;
	m_csThPool.Uninit();
	

	m_csListMutex.LockWrite();
	CWatchTimer *pcsTimer;
	for( CList::CIterator<CWatchTimer *> csIter = m_csTimerList.First<CWatchTimer *>();
		  csIter.IsOk() && m_bRun; csIter.Next() )
	{
		pcsTimer = csIter.Data();
		pcsTimer->m_pMainLoop = NULL;
		
	}
	m_csTimerList.Clear();
	m_csListMutex.UnlockWrite();
	

}

void CMainLoop::RemoveTimer( CWatchTimer *pTimer )
{
	m_csListMutex.LockWrite();
	m_csTimerList.Remove((void*)pTimer);
	m_csListMutex.UnlockWrite();
}

void CMainLoop::AddTimer( CWatchTimer *pTimer )
{
	m_csListMutex.LockWrite();
	m_csTimerList.AddTail((void*)pTimer);
	m_csListMutex.UnlockWrite();
}

void CMainLoop::OnThreadEntry( CObjThreadPool *pcsPool, void *pTaskData )
{
	//定时器轮询

    CWatchTimer *pcsTimer;
    CWatchTimer *pcsFirst;


	CList::CIterator<CWatchTimer *> csIter;
    while(m_bRun && g_bModuleRunning)
    {
        pcsFirst = NULL;
        iCurTime = DoGetTickCount();       
		m_csListMutex.LockReader();		
		 for( csIter = m_csTimerList.First<CWatchTimer *>();
			 csIter.IsOk() && m_bRun; csIter.Next() )
		{
			pcsTimer = csIter.Data();
			pcsTimer->Test(iCurTime);
		}
		m_csListMutex.UnlockReader();


#ifdef _WIN32
          MSLEEP(5);
#else
        MSLEEP(5);
#endif
       
    } //end while(m_bRun)
}



CMainLoop *CMainLoop::Global(void)
{
     GS_ASSERT(s_pGMainLoop);
     return   s_pGMainLoop;
}  

void CMainLoop::InitModule(void)
{
	if( s_pGMainLoop )
	{
		return ;
	}
	CGSAutoMutex locker( &g_csMutex );
	if( s_pGMainLoop )
	{	
		return ;
	}
	s_pGMainLoop = new CMainLoop(1);
	GS_ASSERT(s_pGMainLoop!=NULL);
	
}

void CMainLoop::UninitModule(void)
{
	CGSAutoMutex locker( &g_csMutex );
	if( !s_pGMainLoop )
	{
	
		return;
	}
	delete s_pGMainLoop;
	s_pGMainLoop = NULL;
}

