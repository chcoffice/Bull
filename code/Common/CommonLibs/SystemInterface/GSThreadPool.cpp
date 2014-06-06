#include "GSThreadPool.h"
#include <assert.h>
#include <list>
#include <set>

#ifdef _WIN32
#define THREAD_ID DWORD
#define INVALID_THREAD_ID  ((DWORD)-1)
#define CURRENT_THREAD_ID  GetCurrentThreadId() 
#else
#define  INVALID_THREAD_ID  ((pthread_t)-1)
#define THREAD_ID pthread_t
#define CURRENT_THREAD_ID  pthread_self() 

#endif


static UINT64 s_iExitThreadMarker = 0xaf123456af123456LL;



CGSThreadGlobal *CGSThreadGlobal::s_pDefaultGlobal = NULL;
GSAtomicInter CGSThreadGlobal::s_iInitRefs = 0;



#define IS_EXIT_MARKER(p)  ((CGSThreadPool*)p==(CGSThreadPool*)&s_iExitThreadMarker)

#define EXIT_MARKER   ((CGSThreadPool*)&s_iExitThreadMarker)


#if defined(_WIN32) || defined(WIN32)
static DWORD    s_sThreadStorageKey = -1;
#else
static pthread_key_t   s_sThreadStorageKey = -1;
#endif  

static INT16 s_bThreadInit = 0;
static void _ThreadInitialize()
{

	if( s_sThreadStorageKey!= -1 )
	{
		return;
	}

#if defined(_WIN32) || defined(WIN32)
	s_sThreadStorageKey = ::TlsAlloc();
	if( s_sThreadStorageKey < 0 )
	{
		assert(0);
		_exit(-1);
	}

#else
	pthread_key_create( &s_sThreadStorageKey, NULL); 
#endif

}

static void _ThreadSetSelfThreadPool( void *pObject )
{
#ifdef _WIN32
	BOOL theErr = ::TlsSetValue( s_sThreadStorageKey, pObject );
	if( !theErr )
	{
		assert(0);
		_exit(-1);
	}	
#else
	pthread_setspecific( s_sThreadStorageKey, pObject );
#endif
}

/*
*********************************************************************
*
*@brief :  CGSTaskThread 实现
*
*********************************************************************
*/
class CGSTaskThread
{
private :
	BOOL m_bDestorySelft;
public :
	friend CGSThreadPool;
	CGSThreadGlobal *m_pGlobal;
	BOOL m_bRuning;
	THREAD_ID m_iThreadID;
	BOOL m_bExited; //已经退出
#ifdef _WIN32
	HANDLE m_hHandle;
#else

#endif	
	CGSThreadPool *m_pCurRuning;
	CGSCondEx m_csCondEx;



	CGSTaskThread( CGSThreadGlobal *pGlobal )		
		:m_pGlobal(pGlobal)
		,m_bRuning(FALSE)		
	{
		pGlobal->Ref();
		m_pGlobal = pGlobal;
		m_iThreadID = INVALID_THREAD_ID;	
		m_pCurRuning  = NULL;		
		m_bDestorySelft = FALSE;
		m_bExited = FALSE;
	}

	~CGSTaskThread(void)
	{

#ifdef _WIN32
		if( m_hHandle )
		{
			::CloseHandle(m_hHandle);
			m_hHandle = NULL;
		}
#endif
		if( m_pGlobal )
		{			
			m_pGlobal->Unref();
			m_pGlobal = NULL;
		}
	}


#ifdef _WIN32
static unsigned int WINAPI ThreadEntry( LPVOID pParam )
{
#else	
static void *ThreadEntry(void *pParam )
{
#endif
	CGSTaskThread *pThread = (CGSTaskThread*)pParam;
	_ThreadSetSelfThreadPool(pThread);
	INT i = pThread->Entry();	
	AtomicInterDec( pThread->m_pGlobal->m_iExitThreads);	
	pThread->m_bExited = TRUE;
	if( i )
	{
		delete pThread;
	}
#ifdef _WIN32
	//_endthreadex(0);
#endif
	return 0;

}


BOOL Stop( BOOL bSync )
{
	m_pGlobal->m_csMutex.Lock();
	if( !m_bRuning )
	{
		m_pGlobal->m_csMutex.Unlock();
		return TRUE;
	}
	m_bRuning = FALSE;
	m_csCondEx.BroadcastSignal();
	m_pGlobal->m_csMutex.Unlock();



	if( bSync )
	{

		if ( CURRENT_THREAD_ID == m_iThreadID )
		{
			//线程本身
			m_pGlobal->m_csMutex.Lock();
			m_bDestorySelft = TRUE;
			m_pGlobal->m_csMutex.Unlock();
			return FALSE;
		}


#ifdef _WIN32
		if( m_hHandle )
		{
			INT iTrys = 50;
			DWORD wRet;
			do{
				m_csCondEx.BroadcastSignal();					
				wRet = ::WaitForSingleObject(m_hHandle, 20 );
			} while( wRet==WAIT_TIMEOUT && iTrys-- && !m_bExited );

			if( wRet==WAIT_TIMEOUT )
			{
				MSLEEP(1);
				iTrys = -1;
			}

			if( iTrys<1 )
			{

#ifdef _DEBUG
				printf( "***Warnning : Terminate thread: %d.\n", m_hHandle);
#endif
				::TerminateThread(m_hHandle, -1);
				AtomicInterDec( m_pGlobal->m_iExitThreads );
			}
		}
#else
		if( m_iThreadID != INVALID_THREAD_ID )
		{
			pthread_join(&m_iThreadID, NULL);
		}
#endif
	}
	return TRUE;
}


INT Run( void )
{
	
	m_bRuning = TRUE;
	AtomicInterInc( m_pGlobal->m_iExitThreads );
#ifdef _WIN32	

	m_hHandle = (HANDLE)::_beginthreadex(NULL, 0, CGSTaskThread::ThreadEntry, this,
		0,(uintptr_t *)(&m_iThreadID) );

	if( m_hHandle == NULL )
	{
		AtomicInterDec(m_pGlobal->m_iExitThreads );
		m_bRuning = FALSE;
		m_iThreadID = INVALID_THREAD_ID;
		assert(0);
		return CGSThreadPool::EUNKNOWN;
	}

	return CGSThreadPool::RSUCCESS;
#else

	if( pthread_create(&m_iThreadID, 0, _threadProc, pParam ) == 0) 
	{

		return CGSThreadPool::RSUCCESS;
	}
	AtomicInterDec(m_pGlobal->m_iExitThreads );
	assert(0);
	m_bRuning = FALSE;
	m_iThreadID = INVALID_THREAD_ID;
	return CGSThreadPool::EUNKNOWN;
#endif
}

INT Entry(void)
{

	StruGSListNode *pNode;
	INT bCon = 0;
	StruGSListNode stNode;
	stNode.Init();
	stNode.Data = this;
	BOOL bExclusive;
	

	m_pGlobal->m_csMutex.Lock();
	while( m_bRuning && CGSThreadGlobal::s_iInitRefs )
	{		
		pNode = StruGSListNode::RemoveFront(&m_pGlobal->m_stSchedules);		
		if( pNode )
		{
			//有数据
			m_pCurRuning = (CGSThreadPool*) pNode->Data;
			

			if( IS_EXIT_MARKER(m_pCurRuning) )
			{
				//退出
				m_pGlobal->FreeListNode(pNode);
				m_pCurRuning = NULL;
				
				break;
			}
			bExclusive =  m_pCurRuning->m_pCore->m_bExclusive;
			if( bExclusive )
			{
				m_pGlobal->m_iGThreads--;
			}			
			m_pGlobal->m_csMutex.Unlock();
			bCon =  m_pCurRuning->Run(this);
			m_pGlobal->m_csMutex.Lock();
			m_pCurRuning = NULL;
			if( bExclusive )
			{
				m_pGlobal->m_iGThreads++;
			}
			
			if( bCon )
			{
				//继续
				StruGSListNode::AddTail(pNode, &m_pGlobal->m_stSchedules);				
			}
			else
			{
				m_pGlobal->FreeListNode(pNode);
				assert(m_pGlobal->m_iWaitQueueSize);
				m_pGlobal->m_iWaitQueueSize--;				
			}
		}
		else 
		{
			m_pGlobal->m_iGIdleThreads++;
			StruGSListNode::AddFirst(&stNode, &m_pGlobal->m_stGIdleThreads);	 //加到空闲队列

			if( m_pGlobal->m_iGIdleThreads>m_pGlobal->m_iGMaxIdleThreads )
			{
				//TODO
			}

			//m_csCondEx.WaitTimeout( &m_pGlobal->m_csMutex, 3000 );
			m_csCondEx.Wait( &m_pGlobal->m_csMutex);
			if( !stNode.Empty() )
			{
				//超时
				stNode.Unlink();
				m_pGlobal->m_iGIdleThreads--;
			}
			continue;
		}

	} //end while
	m_pGlobal->m_csMutex.Unlock();

	return m_bDestorySelft;
}


}; // end class CTaskThread


/*
*********************************************************************
*
*@brief : CGSThreadGlobal 实现
*
*********************************************************************
*/


CGSThreadGlobal::CGSThreadGlobal(void)
:m_csMutex()
,m_csThreadQueue()
{
	m_pCache = NULL;
	m_vCacheBufferBegin  = NULL;
	m_vCacheBufferEnd = NULL;
	m_iCaches = 0;
	m_stGIdleThreads.Init();


	m_vCacheBufferBegin = (StruGSListNode*)malloc(  MBYTES*5 );

	assert(m_vCacheBufferBegin);
	if( m_vCacheBufferBegin )
	{
		m_vCacheBufferEnd = (unsigned char*)m_vCacheBufferBegin+MBYTES*5-sizeof(StruGSListNode);
		StruGSListNode *p = m_vCacheBufferBegin;
		bzero(p, sizeof(*p));
		m_pCache = p;
		p++;
		m_iCaches++;
		while( (unsigned char *)p <= m_vCacheBufferEnd )
		{
			bzero(p, sizeof(*p));
			p->next = m_pCache;
			m_pCache = p;
			m_iCaches++;
			p++;				
		}
	}


	m_stSchedules.Init();

	m_csThreadQueue.clear();


	m_iExitThreads =  0;
	m_iWaitQueueSize = 0; //等待调度的线程池个数	m
	m_iGIdleThreads = 0; //当前空闲线程
	m_iGMaxIdleThreads = 32; //最大允许悠闲的公共线程数	
	m_iGMaxThreads = 0XFFFF; //最大公共线程数
	m_iGThreads = 0;  //当前公共线程数	
	m_iRefs = 1;
	m_iDefaultIdleWait = 500;

#ifdef WINCE
	m_iGMaxIdleThreads = 4; //最大允许悠闲的公共线程数 	
#else
	m_iGMaxIdleThreads = 32; //最大允许悠闲的公共线程数

#endif	
}


CGSThreadGlobal::~CGSThreadGlobal(void)
{
	ClearSource();
}

void CGSThreadGlobal::ClearSource(void)
{
		m_csMutex.Lock();
		m_iGMaxThreads = 0;		
		std::set<CGSTaskThread*> csTemp = m_csThreadQueue;
		m_csThreadQueue.clear();
		m_csMutex.Unlock();

		std::set<CGSTaskThread*>::iterator csIt;
		for( csIt = csTemp.begin(); 
			csIt != csTemp.end(); 
			csIt++ )
		{
			if( (*csIt)->Stop(TRUE) )
			{
				delete (*csIt);
			}
		}
		m_csMutex.Lock();
		GS_ASSERT(m_iGIdleThreads==0 && m_stGIdleThreads.Empty() );


		m_pCache = NULL;
		m_iCaches = 0;
		if( m_vCacheBufferBegin )
		{
			m_vCacheBufferEnd = NULL;
			::free(m_vCacheBufferBegin);
			m_vCacheBufferBegin = NULL;
		}	
		m_csMutex.Unlock();
	


	
}


StruGSListNode *CGSThreadGlobal::GetListNode(void)
{

	StruGSListNode *pNode;
	if( m_pCache )
	{
		pNode  = m_pCache;
		m_pCache = pNode->next;
		assert( m_iCaches );
		m_iCaches --;
	}
	else
	{

		pNode = (StruGSListNode*)::malloc( sizeof(StruGSListNode));

		if( pNode )
		{
			bzero(pNode, sizeof(*pNode) );
		}
		else
		{
			assert(pNode);
			return NULL;
		}
	}
	pNode->Init();
	return pNode;
}

void CGSThreadGlobal::FreeListNode(StruGSListNode *pNode)
{

	if( (unsigned char *) pNode>= (unsigned char *) m_vCacheBufferBegin &&
		(unsigned char *)pNode<= m_vCacheBufferEnd )
	{
		pNode->next = m_pCache;
		m_pCache = pNode;
		m_iCaches++;
	}
	else
	{
		::free(pNode );
	}
}

void CGSThreadGlobal::Ref(void)
{
	AtomicInterInc(m_iRefs);
}

void CGSThreadGlobal::Unref(void)
{
	if( AtomicInterDec(m_iRefs) == 0 )
	{		
		delete this;
	}
}

BOOL CGSThreadGlobal::Push( CGSThreadPool *pThread, BOOL bFirst  )
{
	CGSAutoMutex locker(&m_csMutex);
	StruGSListNode *pNode = GetListNode();
	if( !pNode )
	{
		assert(0);
		return FALSE;
	}

	pNode->Data = pThread;
	m_iWaitQueueSize++;
	if( bFirst )
	{
		StruGSListNode::AddFirst(pNode, &m_stSchedules);
	}
	else
	{
		StruGSListNode::AddTail(pNode, &m_stSchedules);
	}

	CGSTaskThread *pTaskThread = NULL;
	if( m_stGIdleThreads.Empty() )
	{
		//没有空闲
		if( m_iGThreads < m_iGMaxThreads )
		{
			//增加线程
			pTaskThread = new CGSTaskThread(this);
			if( pTaskThread )
			{
				
				if(CGSThreadPool::RSUCCESS==pTaskThread->Run() )
				{
					m_csThreadQueue.insert(pTaskThread);
					m_iGThreads++;	
					
				}
				else
				{
					delete pTaskThread;
					pTaskThread = NULL;
					pNode->Unlink();
					FreeListNode(pNode);	
					return FALSE;
				}
			}
		}		
	}
	else
	{
		StruGSListNode *pTheadNode = StruGSListNode::RemoveFront(&m_stGIdleThreads); //唤醒线程
		if( pTheadNode)
		{
			pTaskThread = (CGSTaskThread *)pTheadNode->Data;
			m_iGIdleThreads--;
			pTheadNode->Unlink();			
			pTaskThread->m_csCondEx.Signal();			
		}
	}
	return TRUE;

}



INT CGSThreadGlobal::Clear( CGSThreadPool *pThread )
{	

	CGSAutoMutex locker( &m_csMutex );
	StruGSListNode *pNode = m_stSchedules.next;
	StruGSListNode *pTemp;
	INT iRet = 0;

	while( pNode != &m_stSchedules )
	{
		pTemp = pNode->next;
		if( pNode->Data == pThread )
		{					
			pNode->Unlink();
			iRet++;			
			assert(m_iWaitQueueSize);
			m_iWaitQueueSize--;	
			FreeListNode(pNode);
		}
		pNode = pTemp;
	}
	return iRet;
}

CGSThreadGlobal *CGSThreadGlobal::Create(void)
{
	//GS_ASSERT_RET_VAL(CGSThreadGlobal::s_iInitRefs, NULL);
CGSThreadGlobal *p = new CGSThreadGlobal();	
	return p;
}

CGSThreadGlobal *CGSThreadGlobal::Global(void)
{	
	return s_pDefaultGlobal;
}

void CGSThreadGlobal::SetMaxGlobalThread(  UINT16 iNums)
{
	m_iGMaxThreads = iNums;
}

void   CGSThreadGlobal::SetMaxIdleThreads( UINT16 iNums)
{
	m_iGMaxIdleThreads = iNums;
}

void  CGSThreadGlobal::SetDefaultIdleWait( INT32 iMSec )
{
	m_iDefaultIdleWait = iMSec;
}






/*
*********************************************************************
*
*@brief : CGSThreadPool::CCoreData 实现
*
*********************************************************************
*/
#define THERAD_POOL_STATUS_NORMAL 0
#define THERAD_POOL_STATUS_DISCARD 0x01
#define THERAD_POOL_STATUS_DISABLE 0x02

CGSThreadPool::CCoreData::CCoreData(CGSThreadPool *pPool, CGSThreadGlobal *pThreadGlobal)
:m_pPool(pPool)
,m_pThreadGlobal(pThreadGlobal)
,m_csMutex()
,m_csCond()
,m_csExitCond()
{	
	pThreadGlobal->Ref();
	m_fnCallback = NULL;
	m_fnFreeFunc = NULL;
	m_pUserData = NULL;
	m_iIdleInterval = CGSThreadPool::s_iGIdleMSecs;

	m_iMaxJobs = -1;			//最大允许等待任务队列
	m_iPermitThreads = 0;    //最大允许线程数  
	m_iNumThreads = 0;		//当前使用的线程数
	m_iStatus = THERAD_POOL_STATUS_NORMAL;			//状态
	m_bExclusive = 0;		//是否独占
	m_bWaiting = 0;			//释放等待执行完成

	m_iRefs = 1;
	m_iWaitTask = 0;
	m_iCaches = 0;
	m_iIdleThreads = 0;

	m_stTaskQueue.Init(); //任务队列
	m_pTaskCache = NULL; //链表结构内存池队列
}

BOOL CGSThreadPool::CCoreData::Unref(void)
{
	m_iRefs--;
	if( m_iRefs ==0 )
	{
		m_csMutex.Unlock();
		delete this;
		return FALSE;
	}
	return TRUE;
}

void CGSThreadPool::CCoreData::FreeWaitTask(void)
{
	StruGSListNode *pNode;
	while( (pNode = StruGSListNode::RemoveFront(&m_stTaskQueue) ) )
	{		
		if( m_fnFreeFunc )
		{
			m_fnFreeFunc(pNode->Data);
		}
		FreeTaskNode(pNode);
		m_iWaitTask--;
	}
}

INT CGSThreadPool::CCoreData::RemoveTask(void *pTaskData)
{
	INT  iRet = 0;

	StruGSListNode *pNode, *pTemp;
	pNode = m_stTaskQueue.next;

	while( pNode != &m_stTaskQueue )
	{		
		if( pNode->Data == pTaskData )
		{		
			pTemp = pNode->next;
			pNode->Unlink();
			FreeTaskNode(pNode);
			m_iWaitTask--;
			pNode = pTemp;
		}
		else
		{
			pNode = pNode->next;
		}

	}
	return iRet;
}

void CGSThreadPool::CCoreData::FreeTaskNode(StruGSListNode *pNode)
{
	if( m_iCaches<1000 )
	{	
		pNode->next = m_pTaskCache;
		m_pTaskCache = pNode;
		m_iCaches++;
	}
	else
	{
		CGSAutoMutex locker(&m_pThreadGlobal->m_csMutex );
		m_pThreadGlobal->FreeListNode(pNode);
	}
}

StruGSListNode *CGSThreadPool::CCoreData::GetTaskNode(void)
{
	StruGSListNode *pNode = NULL;

	if( !m_pTaskCache )
	{

		CGSAutoMutex locker(&m_pThreadGlobal->m_csMutex );
		for( ; m_iCaches<20;  )
		{
			pNode = m_pThreadGlobal->GetListNode();
			if( pNode )
			{
				pNode->next = m_pTaskCache;
				m_pTaskCache = pNode;
				m_iCaches++;
			}
			else
			{
				break;
			}
		}
	}
	assert(m_iCaches && m_pTaskCache);
	if( m_pTaskCache )
	{

		pNode = m_pTaskCache;
		m_pTaskCache = pNode->next;
		pNode->Init();
		m_iCaches--;
		return pNode;
	}
	assert(m_iCaches==0);
	return NULL;
}

StruGSListNode *CGSThreadPool::CCoreData::AddTask( void *pData, BOOL bFirst)
{
	StruGSListNode *p;
	p = GetTaskNode();
	if( p )
	{
		p->Data = pData;		
		if( bFirst )
		{
			StruGSListNode::AddFirst(p, &m_stTaskQueue);
		}
		else
		{
			StruGSListNode::AddTail(p, &m_stTaskQueue);
		}
		m_iWaitTask++;
		return p;
	}
	assert(p);
	return NULL;

}

void CGSThreadPool::CCoreData::Ref(void)
{
	m_iRefs++;
}

CGSThreadPool::CCoreData::~CCoreData(void)
{
	m_csMutex.Lock();
	FreeWaitTask();
	m_csMutex.Unlock();

	if( m_pThreadGlobal )
	{
	m_pThreadGlobal->m_csMutex.Lock();
	StruGSListNode *pNode;
	while( (pNode = m_pTaskCache) )
	{
		m_pTaskCache = pNode->next;
		m_pThreadGlobal->FreeListNode(pNode);
	}
	m_pThreadGlobal->m_csMutex.Unlock();
	m_pThreadGlobal->Unref();
	}
}


/*
*********************************************************************
*
*@brief : CGSThreadPool 的实现
*
*********************************************************************
*/

INT32  CGSThreadPool::s_iGIdleMSecs = 500; 
GSAtomicInter CGSThreadPool::m_iInitRefs = 0;

CGSThreadPool::CGSThreadPool(CGSThreadGlobal *pThreadGlobal)
:m_pCore(NULL)
{
	if( !pThreadGlobal )
	{
		pThreadGlobal = CGSThreadGlobal::Global();		
	}
	if( pThreadGlobal )
	{
		m_pCore = new CCoreData(this, pThreadGlobal);
	}
	assert(m_pCore);


}

CGSThreadPool::~CGSThreadPool(void)
{
	Uninit(FALSE);
	if( m_pCore )
	{
		m_pCore->m_pPool = NULL;
		m_pCore->m_csMutex.Lock();
		if( m_pCore->Unref() )
		{
			m_pCore->m_csMutex.Unlock();
		}
		m_pCore = NULL;
	}

}

CGSThreadGlobal &CGSThreadPool::GetThreadGlobal(void)
{
	return *(m_pCore->m_pThreadGlobal);
}

void CGSThreadPool::SetIdleWait( INT32 iMSec )
{
	assert(m_pCore);
	m_pCore->m_iIdleInterval = iMSec;
}

BOOL CGSThreadPool::Init( GSThreadPoolCallback fnOnCallback, 
						 UINT16 iThreadCounts, 
						 BOOL bExclusive)
{
	if( !m_pCore  || iThreadCounts<1 )
	{
		assert(0);
		return FALSE;
	}

	if( !m_pCore->m_pThreadGlobal->m_vCacheBufferBegin )
	{
		assert(0);
		return FALSE;
	}
	m_pCore->m_iPermitThreads = iThreadCounts;
	m_pCore->m_fnCallback = fnOnCallback;
	m_pCore->m_bExclusive = bExclusive;
	return TRUE;

}

void CGSThreadPool::Uninit(BOOL bJobsComplete )
{
	GS_ASSERT_RET(m_pCore);
	BOOL bIsSelf = FALSE;
	int iTrys = 400;
	bIsSelf = IsSelfThread();

	m_pCore->m_csMutex.Lock();
	if( m_pCore->m_iStatus&THERAD_POOL_STATUS_DISCARD )
	{           
		if( bIsSelf )
		{
			GS_ASSERT(m_pCore->m_iNumThreads==1);
			m_pCore->m_csMutex.Unlock();		
			return;
		}
		while( m_pCore->m_iNumThreads > 0  )
		{
			m_pCore->m_csExitCond.WaitTimeout( &m_pCore->m_csMutex, 100);           
		}
		m_pCore->m_csMutex.Unlock();		
		return;
	}

	m_pCore->m_iStatus |= THERAD_POOL_STATUS_DISCARD;

	INT iClear = 0;


	if( bJobsComplete )
	{
		m_pCore->m_bWaiting = TRUE;
	}
	else if( !m_pCore->m_bExclusive )
	{
		iClear = m_pCore->m_pThreadGlobal->Clear(this);
	}

	m_pCore->m_iNumThreads -= iClear;

	


	m_pCore->m_csCond.BroadcastSignal();

	while( m_pCore->m_iNumThreads > 0 && iTrys-- )
	{
		if( bIsSelf && m_pCore->m_iNumThreads  == 1 )
		{				
			m_pCore->m_pPool = NULL;
			break;
		}
		m_pCore->m_csCond.BroadcastSignal();
		if( m_pCore->m_iNumThreads  > 0)
		{
			m_pCore->m_csExitCond.WaitTimeout( &m_pCore->m_csMutex, 100);           
		}
	}
	assert( iTrys > 0 );
	m_pCore->FreeWaitTask();
	m_pCore->m_csMutex.Unlock();

}

void CGSThreadPool::Disable( BOOL bJobsComplete )
{
	GS_ASSERT_RET(m_pCore);

	BOOL bIsSelf = FALSE;
	int iTrys = 400;
	bIsSelf = IsSelfThread();

	m_pCore->m_csMutex.Lock();
	if( m_pCore->m_iStatus  )
	{           
		if( bIsSelf )
		{
			m_pCore->m_csMutex.Unlock();		
			return;
		}
		while( m_pCore->m_iNumThreads > 0  )
		{
			m_pCore->m_csExitCond.WaitTimeout( &m_pCore->m_csMutex, 100);           
		}
		m_pCore->m_csMutex.Unlock();		
		return;
	}

	m_pCore->m_iStatus |= THERAD_POOL_STATUS_DISABLE;

	INT iClear = 0;


	if( bJobsComplete )
	{
		m_pCore->m_bWaiting = TRUE;
	}
	else if( !m_pCore->m_bExclusive )
	{
		iClear = m_pCore->m_pThreadGlobal->Clear(this);
	}

	m_pCore->m_iNumThreads -= iClear;

	



	m_pCore->m_csCond.BroadcastSignal();
	while( m_pCore->m_iNumThreads > 0 && iTrys-- )
	{
		if( bIsSelf && m_pCore->m_iNumThreads  == 1 )
		{	
			break;
		}
		m_pCore->m_csCond.BroadcastSignal();
		if( m_pCore->m_iNumThreads  > 0)
		{
			m_pCore->m_csExitCond.WaitTimeout( &m_pCore->m_csMutex, 100);           
		}
	}
	
	assert( iTrys > 0 );
	
	m_pCore->m_csMutex.Unlock();
}

void CGSThreadPool::Enable(void)
{
	GS_ASSERT_RET(m_pCore);
	m_pCore->m_csMutex.Lock();
	m_pCore->m_iStatus &= ~THERAD_POOL_STATUS_DISABLE;
	m_pCore->m_csMutex.Unlock();
}


BOOL CGSThreadPool::IsInit(void)
{
	if( m_pCore  && m_pCore->m_iPermitThreads )
	{
		return TRUE;
	}
	return FALSE;
}

BOOL CGSThreadPool::IsEnable(void)
{
	GS_ASSERT_RET_VAL(m_pCore, FALSE);
	return m_pCore->m_iStatus==0;
}

void CGSThreadPool::SetFreedTaskDataFunction( void (*fnFreeFunc)(void *)  )
{
	GS_ASSERT_RET(m_pCore);
	m_pCore->m_fnFreeFunc = fnFreeFunc;
}

void CGSThreadPool::SetMaxWaitTask( INT32 iMaxVal )
{
	GS_ASSERT_RET(m_pCore);
	m_pCore->m_iMaxJobs = iMaxVal;
}

INT CGSThreadPool::Task( void *pTaskData, BOOL bFirst)
{
	GS_ASSERT_RET_VAL(m_pCore, EUNKNOWN);

	CGSAutoMutex locker( &m_pCore->m_csMutex);
	if( m_pCore->m_iStatus )
	{
		return EINVALID;
	}

	if( m_pCore->m_iMaxJobs>-1 && (UINT32) m_pCore->m_iMaxJobs<= m_pCore->m_iWaitTask )
	{
		if( m_pCore->m_iIdleThreads )
		{
			m_pCore->m_csCond.Signal();
		}
		return EFLOWOUT;
	}

	StruGSListNode *p = m_pCore->AddTask(pTaskData, bFirst);
	if(  !p )
	{
		return ENMEMORY;
	}

	if( m_pCore->m_iNumThreads<m_pCore->m_iPermitThreads )
	{
		m_pCore->m_iNumThreads++;
		if( m_pCore->m_pThreadGlobal->Push(this, bFirst) )
		{
			return RSUCCESS;
		}
		m_pCore->m_iNumThreads--;
	}
	else 
	{			
		if( m_pCore->m_iIdleThreads )
		{
			m_pCore->m_csCond.Signal();	
		}
		return RSUCCESS;
	}	
	p->Unlink();
	p->Init();
	m_pCore->FreeTaskNode(p);
	return EUNKNOWN;
}


BOOL CGSThreadPool::IsSelfThread(void)
{
	CGSTaskThread *p = NULL;
#ifdef _WIN32
	p =  (CGSTaskThread *) ::TlsGetValue(s_sThreadStorageKey);
#else
	p =  (CGSTaskThread *)  pthread_getspecific(s_sThreadStorageKey);
#endif
	return (p && p->m_pCurRuning==this);
}

void CGSThreadPool::SetUserData( void *pUserData )
{
	assert(m_pCore);
	if( m_pCore )
	{
		m_pCore->m_pUserData = pUserData;
	}
}

void *CGSThreadPool::GetUserData(void)
{
	assert(m_pCore);
	if( m_pCore )
	{
		return m_pCore->m_pUserData;
	}
	return NULL;
}





void CGSThreadPool::InitModule(void)
{

	if( AtomicInterInc(CGSThreadGlobal::s_iInitRefs) ==  1 )
	{		
		_ThreadInitialize();
		CGSThreadGlobal::s_pDefaultGlobal = new CGSThreadGlobal();
	}
}

void CGSThreadPool::UninitModule(void)
{
	if( AtomicInterDec(CGSThreadGlobal::s_iInitRefs) ==  0 )
	{	
		if( CGSThreadGlobal::s_pDefaultGlobal )
		{
			CGSThreadGlobal::s_pDefaultGlobal->Unref();
			CGSThreadGlobal::s_pDefaultGlobal = NULL;
		}
	}
}

BOOL CGSThreadPool::Run(CGSTaskThread *pThread )
{
	BOOL bRet = FALSE;
	CCoreData *pCore = m_pCore;
	pCore->m_csMutex.Lock();
	pCore->Ref();
	StruGSListNode *pNode;

	if( pCore->m_bExclusive )
	{
		//独占线程
		
		while(CGSThreadGlobal::s_iInitRefs && pThread->m_bRuning && 
			(THERAD_POOL_STATUS_NORMAL==pCore->m_iStatus
			|| (pCore->m_bWaiting &&  pCore->m_iWaitTask>0 ) ) )
		{
			pNode = StruGSListNode::RemoveFront(&pCore->m_stTaskQueue);
			if( pNode )
			{
				pCore->m_iWaitTask--;
				pCore->m_csMutex.Unlock();
				pCore->m_fnCallback(this, pNode->Data , pThread );
				pCore->m_csMutex.Lock();
				pCore->FreeTaskNode(pNode);

			}
			else 
			{
				pCore->m_iIdleThreads++;				
				//pCore->m_csCond.Wait(&pCore->m_csMutex); //无限等待
				pCore->m_csCond.WaitTimeout(&pCore->m_csMutex, 2000);	
				pCore->m_iIdleThreads--;					
				
			}
		} //end while
		
	} 
	else
	{
		//共享线程
		INT iCurExcuteCounts; //当期运行的任务数， 防止一个线程长期占用		
		iCurExcuteCounts = pCore->m_iWaitTask;
		while(CGSThreadGlobal::s_iInitRefs && pThread->m_bRuning && 
			(THERAD_POOL_STATUS_NORMAL==pCore->m_iStatus || pCore->m_bWaiting) )
		{
			
			if( iCurExcuteCounts<1 )
			{
				bRet = pCore->m_iWaitTask >0  ?  TRUE : FALSE;
				break;
			}
			iCurExcuteCounts--;
			pNode = StruGSListNode::RemoveFront(&pCore->m_stTaskQueue);
			if( pNode )
			{
				pCore->m_iWaitTask--;
				pCore->m_csMutex.Unlock();
				pCore->m_fnCallback(this, pNode->Data , pThread );
				pCore->m_csMutex.Lock();
				pCore->FreeTaskNode(pNode);

			}
#if 0
			else 
			{
				pCore->m_iIdleThreads++;				
				if( pCore->m_iIdleInterval>0 )
				{
					pCore->m_csCond.WaitTimeout( &pCore->m_csMutex, pCore->m_iIdleInterval); 
					pCore->m_iIdleThreads--;
					if( pCore->m_stTaskQueue.Empty() )
					{
						//没数据, 把线程放回空闲线程池
						break;
					}
				}
				else
				{
					pCore->m_iIdleThreads--;
					break;
				}							

			}
#endif
		} // end while
	}
	

	pCore->m_iNumThreads--;

	if( pCore->m_iStatus && pCore->m_iNumThreads == 0 
		&& (pCore->m_iRefs==1 || !bRet) )
	{
		pCore->m_csExitCond.BroadcastSignal();
	}

	if( pCore->Unref() )
	{
		if( bRet )
		{
			pCore->m_iNumThreads++;	
		}
		pCore->m_csMutex.Unlock();	
	}
	else
	{
		bRet = FALSE;
	}

	return bRet;
}


UINT32 CGSThreadPool::GetWaitTask(void)
{
	if( m_pCore )
	{
		return m_pCore->m_iWaitTask;
	}
	return 0;
}

void CGSThreadPool::Clear(void)
{
	if( m_pCore )
	{
		CGSAutoMutex locker( &m_pCore->m_csMutex );
		m_pCore->FreeWaitTask();
	}
}

INT CGSThreadPool::RemoveTask(void *pTaskData)
{
	if( m_pCore )
	{
		CGSAutoMutex locker( &m_pCore->m_csMutex );
		m_pCore->RemoveTask(pTaskData);
	}
	return -1;
}


/*
*********************************************************************
*
*@brief : CObjThreadPool
*
*********************************************************************
*/
CObjThreadPool::CObjThreadPool( CGSThreadGlobal *pThreadGlobal, const char *czName )
:CGSThreadPool( pThreadGlobal ),						
m_pTaskFnOwner(NULL),
m_pFnTaskCallback(NULL),
m_strName("ThreadPool")

{




	m_strName = "ThreadPool";
	if( czName )
	{
		m_strName += "_";
		m_strName += czName;
	}



}

CObjThreadPool::CObjThreadPool( const char *czName) 
: CGSThreadPool(),						
m_pTaskFnOwner(NULL),
m_pFnTaskCallback(NULL),
m_strName("ThreadPool")

{




	m_strName = "ThdPool";
	if( czName )
	{
		m_strName += "_";
		m_strName += czName;
	}


}


CObjThreadPool::~CObjThreadPool(void)
{

}

BOOL CObjThreadPool::Init( CGSObject *pTaskFnOwner, FuncPtrObjThreadPoolEvent pFnTaskCallback,  
					   UINT16 iThreadCounts,  BOOL bExclusive)
{
	m_pTaskFnOwner = pTaskFnOwner;
	m_pFnTaskCallback = pFnTaskCallback;
	return CGSThreadPool::Init(&CObjThreadPool::ProxyEntry, iThreadCounts, bExclusive);
}

void CObjThreadPool::ProxyEntry( CGSThreadPool *pcsPool, void *TaskData, void *pDebugInfo)
{
	CObjThreadPool *p = static_cast<CObjThreadPool*>(pcsPool);
	(p->m_pTaskFnOwner->*(p->m_pFnTaskCallback))(p, TaskData );
}