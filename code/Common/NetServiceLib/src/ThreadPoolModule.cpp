#include "ThreadPoolModule.h"

using namespace NetServiceLib;

#ifdef _WIN32
unsigned   _stdcall ThreadProcFunc(HANDLE param)
#elif _LINUX
void *ThreadProcFunc(HANDLE param)
#endif 
{	
	CNetThread * pNetThread = (CNetThread *)param;
	if ( pNetThread->m_fnUser )
	{
		pNetThread->m_fnUser(pNetThread, pNetThread->m_pFnUserParam);
	}

#ifdef _WIN32	
	return 0;
#elif _LINUX
	return pNetThread;
#endif 
}
CNetThread::CNetThread():m_fnUser(NULL), m_hThread(0), m_pFnUserParam(NULL)
{

};

CNetThread::~CNetThread()
{
	if ( m_hThread )
	{
#ifdef _WIN32
		CloseHandle(m_hThread);
#elif _LINUX
		//分离线程
		if (pthread_detach(m_hThread)!=0)
		{
			assert(0);
		}	
#endif
	}
}

BOOL CNetThread::Start(ThreadCallbackFunction fnOnEvent, void *pUserParam)
{
	if ( !fnOnEvent )
	{
		assert(0);
	}
	m_fnUser = fnOnEvent;
	m_pFnUserParam = pUserParam;
#ifdef WINCE
	m_hThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadProcFunc,(LPVOID)this,0,NULL); //(NetThread)_beginthreadex(NULL,0,ThreadProcFunc,HANDLE(this),0,NULL);

#elif _WIN32
	m_hThread = (NetThread)_beginthreadex(NULL,0,ThreadProcFunc,HANDLE(this),0,NULL);
#elif _LINUX
	if(pthread_create(&m_hThread,0,ThreadProcFunc,HANDLE(this))!=0)
	{
		assert(0);

	}
#endif

	return TRUE;
}

CThreadPoolModule::CThreadPoolModule(void)
{
	m_uiCurThreadCount = 0;
	m_uiMaxThreadCount = THREADPOOL_MAX_THREAD_COUNT;
	m_DequeTask.clear();
	m_VectorThread.clear();
	m_VecBusyThread.clear();
	m_VecIdleThread.clear();
	m_bIsExitWorkerThreadProc = true;
	m_iIdleThreadMaxNum = 4;
	m_iIdleThreadMinNum = 1;
	m_iWaitDelThreadNum = 0;
	m_unExitTick = 0;
	m_bIsExitManangeThread = FALSE;
	m_clsLogPtr = NULL;
	m_ExitThreadCount=0;
}

CThreadPoolModule::~CThreadPoolModule(void)
{
}
INT CThreadPoolModule::Initialize( UINT uiThreads)
{
	//这个函数禁止重复调用
	if (m_uiCurThreadCount > 0)
	{
		//已经初始化
		return ERROR_NET_REPEAT_INIT;
	}
	if ( uiThreads < 0 )
	{
		return ERROR_NET_PARAM_WRONG;
	}

	m_uiMaxThreadCount = uiThreads > m_uiMaxThreadCount ? uiThreads : m_uiMaxThreadCount;
	if (m_uiMaxThreadCount - uiThreads <= 2)
	{
		// 否则没有线程来执行其它任务比如监听 、通道释放
		m_uiMaxThreadCount += 10;
	}


//#ifdef OPERATING_SYSTEM
//#else
	if(AddThread(uiThreads) != ERROR_BASE_SUCCESS)
	{
		//增加线程失败
		return ERROR_NET_PARAM_WRONG;
	}
//#endif

	// 管理线程 
	m_clsManagerThread.Start(CThreadPoolModule::ManagerThreadProc, (void *)this);
	//_beginthreadex(NULL,0,ManagerThreadProcEx,HANDLE(this),0,NULL);


	return ERROR_BASE_SUCCESS;

}

//增加线程
INT CThreadPoolModule::AddThread(UINT16 usThreads)
{
	CGSAutoMutex	GSAutoMutex(&m_GSMutexDequeThread);

	if (m_uiCurThreadCount + usThreads > m_uiMaxThreadCount)
	{
		return ERROR_NET_OVERFLOW_MAX_THREADS;
	}
	if ( usThreads > 0 )
	{
		//分配新的线程
		return AllocNewThreads(usThreads);
	}

	return ERROR_NET_PARAM_WRONG;

}
//减少线程
INT CThreadPoolModule::SubThread(UINT16 usThreads)
{
	//return ERROR_NET_PARAM_WRONG;
	CGSAutoMutex	GSAutoMutex(&m_GSMutexDequeThread);

	m_iWaitDelThreadNum += usThreads;

	return 0;	

}
//从队列移出线程
INT	CThreadPoolModule::DeleteFromDeque( CNetThread* pclsThread )
{  
	if ( NULL == pclsThread )
	{
		return ERROR_NET_THREAD_NOT_EXIST;
	}

	vector<CNetThread*>::iterator pIter;
	pIter = find(m_VectorThread.begin(), m_VectorThread.end(), pclsThread);
	if (pIter != m_VectorThread.end())
	{
		m_VectorThread.erase(pIter);
		return ERROR_BASE_SUCCESS;
	}


	return ERROR_NET_THREAD_NOT_EXIST;
}
/*************************************************
Function:      Uninitialize 
Description:   释放线程池资源
Input:  
Output:         
Return:         
Note:		//1、停止所有线程,要考虑线程的状态
//2、释放线程资源
//3、释放任务队列中的任务		
*************************************************/
void CThreadPoolModule::Uninitialize()
{
	//释放任务资源
	m_GSMutexDequeTask.Lock();
	CTRLPRINTF(m_clsLogPtr,"网络库未执行任务数%d \n", m_DequeTask.size());
	for (deque<pstruThreadPoolTask>::size_type i=0; i< m_DequeTask.size(); i++)
	{
		//逐个释放
		free(m_DequeTask[i]);
	}
	m_DequeTask.clear();

	m_GSMutexDequeTask.Unlock();

	StruSysTime		struTime;
	DoGetLocalTime(&struTime);
	CTRLPRINTF(m_clsLogPtr,"网络库线程池开始退出  %d:%d:%d \n", struTime.wHour, struTime.wMinute, struTime.wSecond);
	
	// 管理线程也使用此变量
	m_unExitTick = DoGetTickCount();

	while ( !m_bIsExitManangeThread )
	{
		MSLEEP(10);
		if (  DoGetTickCount() - m_unExitTick > 20*1000 )
		{
			CTRLPRINTF(m_clsLogPtr," NetServiceLib 管理线程退出异常,参数m_VecBusyThread=%d, m_VecIdleThread=%d,\
								   m_VecWaitDelThread=%d, m_VectorThread=%d \n ",m_VecBusyThread.size(),m_VecIdleThread.size(),m_VecWaitDelThread.size(),m_VectorThread.size());
			assert(0);
			break;
		}
	}
	
	CTRLPRINTF(m_clsLogPtr,"网络库线程池退出耗时 %u 毫秒\n", DoGetTickCount() - m_unExitTick);


	m_GSMutexDequeThread.Lock();
	m_VectorThread.clear();
	m_GSMutexDequeThread.Unlock();

	return;
}
bool CThreadPoolModule::AssignTask(pstruThreadPoolTask pTask)
{
	if (pTask == NULL)
	{
		return false;
	}

	if ( !m_bIsExitWorkerThreadProc )
	{
		return false;
	}

	//保存任务至队列
	m_GSMutexDequeTask.Lock();
	m_DequeTask.push_back(pTask);
	m_GSMutexDequeTask.Unlock();

	return true;
}

// 
/********************************************************************************
  Function:		ManagerThreadProc
  Description:	线程池管理线程
  Input:  		
  Output:      	   
  Return:  		       
  Note:			静态函数		
  Author:        	CHC
  Date:				2010/09/01
********************************************************************************/
void CThreadPoolModule::ManagerThreadProc(CNetThread* pclsThread, void * pInfo)
{
	if ( !pInfo || !pclsThread ) return;	


	CThreadPoolModule *pThisThreadPool = (CThreadPoolModule*) pInfo;
	pThisThreadPool->ManagerThreadProc(pclsThread);

}

/********************************************************************************
  Function:		ManagerThreadProc
  Description:	线程池管理线程执行函数
  Input:  		
  Output:      	   
  Return:  		       
  Note:			在此函数中对线程的线程进行管理 如增加 减少 释放	。线程池强不强大就看此函数的逻辑了	
  Author:        	CHC
  Date:				2010/09/01
********************************************************************************/
void CThreadPoolModule::ManagerThreadProc(CNetThread* pclsThread)
{
	
	//UINT64 uiTick = DoGetTickCount();
	while ( 1 )
	{

		m_GSMutexDequeTask.Lock();
		m_GSMutexDequeThread.Lock();
		if ( m_DequeTask.size() > 0 && m_VecIdleThread.size())
		{
#if _WIN32
			int nCount = m_VecIdleThread.size();	//多发几个信号量也无妨吧
			while ( nCount-- )
			{
				m_GSCond.Signal();
			}
#else
			
			m_GSCond.Signal();

#endif
			
			
		}

		m_GSMutexDequeThread.Unlock();
		m_GSMutexDequeTask.Unlock();

		// 看看有无要删的线程
		FreeThread();

		if ( m_bIsExitWorkerThreadProc )
		{
			// 判断空闲线程数目
			//INT iIdleNum =  m_VecIdleThread.size();
			//if (iIdleNum > m_iIdleThreadMaxNum)
			//{
			//	//减少线程
			//	INT iSubNum = iIdleNum - ((m_iIdleThreadMaxNum - m_iIdleThreadMinNum)/2 + m_iIdleThreadMinNum);

			//	SubThread(iSubNum);	

			//	UINT64	unTick = DoGetTickCount();

			//	while ( m_iWaitDelThreadNum )
			//	{
			//		m_GSCond.Signal();
			//		MSLEEP(10);

			//		if ( DoGetTickCount() - unTick > 10*1000 )
			//		{
			//			CTRLPRINTF(m_clsLogPtr," NetServiceLib 逻辑异常之减少线程 \n ");
			//			assert(0);

			//			break;
			//		}
			//	}


			//}
			//else if (iIdleNum < m_iIdleThreadMinNum)
			//{
			//	//增加线程
			//	AddThread(m_iIdleThreadMinNum - iIdleNum);


			//}

		}
		else
		{
			// 退出	1、等待忙碌线程为0	2、释放全部空闲线程
			if ( m_VecBusyThread.size() == 0 )
			{
				if ( m_VecIdleThread.size() > 0 )
				{
					SubThread( m_VecIdleThread.size() );

					UINT64	unTick = DoGetTickCount();

					while ( m_iWaitDelThreadNum )
					{
						m_GSCond.BroadcastSignal();

						MSLEEP(10);

						if ( DoGetTickCount() - unTick > 10*1000 )
						{
							CTRLPRINTF(m_clsLogPtr," NetServiceLib 逻辑异常,参数m_VecBusyThread=%d, m_VecIdleThread=%d,\
								m_VecWaitDelThread=%d, m_VectorThread=%d \n ",m_VecBusyThread.size(),m_VecIdleThread.size(),m_VecWaitDelThread.size(),m_VectorThread.size());
							assert(0);

							break;
						}
					}
				}
						
			}
			

		}
		

		// 判断任务数

		if ( m_bIsExitWorkerThreadProc )
		{

			if ( m_uiCurThreadCount < m_uiMaxThreadCount )
			{
				deque<pstruThreadPoolTask>::size_type iSize = m_DequeTask.size();
				if ( iSize > 0 && m_VecIdleThread.size() == 0)
				{
					if (iSize < m_uiMaxThreadCount - m_uiCurThreadCount )
					{
						//增加线程
						AddThread(iSize);
						CTRLPRINTF(m_clsLogPtr," 增加线程条数 %d\n ",iSize);
					}
					else
					{
						//增加线程
						AddThread(m_iIdleThreadMinNum - m_uiCurThreadCount);
						CTRLPRINTF(m_clsLogPtr," 增加线程条数 %d\n ",m_iIdleThreadMinNum - m_uiCurThreadCount);
					}
				}

			}

		}

		/*if (DoGetTickCount() - uiTick > 20000)
		{
			CTRLPRINTF(m_clsLogPtr,"当前网络库中所有线程数目是 %d \n", GetAllThreadCount());
			CTRLPRINTF(m_clsLogPtr,"当前网络库中忙碌线程数目是 %d \n", GetBusyThreadCount());
			CTRLPRINTF(m_clsLogPtr,"当前网络库中空闲线程数目是 %d \n", GetIdleThreadCount());
			CTRLPRINTF(m_clsLogPtr,"当前网络库中任务数目是 %d \n", GetTaskCount());
			CTRLPRINTF(m_clsLogPtr,"当前网络库中待删除的线程数目是 %d \n",GetWaitDelThreadNum());
			CTRLPRINTF(m_clsLogPtr,"当前网络库中删除队列中的线程数目是 %d \n",GetVecWaitDelThread());
			uiTick = DoGetTickCount();

		}
		*/
		MSLEEP(10);
		

		if ( !m_bIsExitWorkerThreadProc )
		{
			// 退出	等待所有线程退掉后 管理线程退出

			if ( m_VectorThread.size() == 0 )
			{
				break;
			}
			else if( m_unExitTick > 0 )
			{
				if ( DoGetTickCount() - m_unExitTick > 30*1000 )
				{
					CTRLPRINTF(m_clsLogPtr," 有 %d 个线程未能正常退出  \n ",  m_VecBusyThread.size());
					assert(0);
					break;
				}
			}
			else
			{
				// 不需要做什么
			}
		}

	}//end while

	m_bIsExitManangeThread = TRUE;

}


//静态方法
void CThreadPoolModule::WorkerThreadProc( CNetThread* pclsThread, void * pInfo)
{
	if ( NULL == pInfo || NULL == pclsThread ) 
	{
		return;	
	}


	CThreadPoolModule *pThisThreadPool = (CThreadPoolModule*) pInfo;
	pThisThreadPool->WorkerThreadProc(pclsThread);


}
void CThreadPoolModule::WorkerThreadProc(CNetThread* pclsThread)
{
	
	pstruThreadPoolTask	pQueueTask = NULL;

	while (1)
	{

		m_GSMutexDequeThread.Lock();
		if (m_iWaitDelThreadNum > 0)
		{		
			--m_iWaitDelThreadNum;
			m_GSMutexDequeThread.Unlock();
			break;

		}

		m_GSMutexDequeThread.Unlock();

		m_GSMutexDequeTask.Lock();

		if (m_DequeTask.empty() == false)
		{
			pQueueTask = m_DequeTask.front();
			m_DequeTask.pop_front();

			m_GSMutexDequeTask.Unlock();
		}
		else
		{
			//任务队列无任务返回
			m_GSMutexDequeTask.Unlock();
			
			if (m_bIsExitWorkerThreadProc)
			{	// 不退出继续等待任务

				m_GSCond.Wait();
			}

			continue;
		}

		if ( NULL == pQueueTask)
		{
			//有任务，但没获取成功，正常情况不会出现此情况
			assert(0);
			return;
		}


		//移到忙碌队列
		MoveToBusyVec(pclsThread);

		//执行任务
		if ( NULL != pQueueTask->pFunction && NULL != pQueueTask->pObject )
		{		

			
			((CCommunicationManager*)(pQueueTask->pObject)->*(pQueueTask->pFunction))(pQueueTask->enumEvent, pQueueTask->pObject2);

			StruSysTime		struTime;
			DoGetLocalTime(&struTime);
#if OPERATING_SYSTEM

#elif _WIN32
			if ( ACCEPTDATA == pQueueTask->enumEvent )
			{
				++m_ExitThreadCount;
				CTRLPRINTF(m_clsLogPtr,"完成端口线程退出 %d:%d:%d \n", struTime.wHour, struTime.wMinute, struTime.wSecond);
			}
#endif
			if ( LISTEN == pQueueTask->enumEvent )
			{
				CTRLPRINTF(m_clsLogPtr,"监听线程退出 %d:%d:%d \n", struTime.wHour, struTime.wMinute, struTime.wSecond);
			}
			else if ( EPOLLEVENTWAIT == pQueueTask->enumEvent )
			{
				CTRLPRINTF(m_clsLogPtr,"epoll事件线程退出 %d:%d:%d \n", struTime.wHour, struTime.wMinute, struTime.wSecond);
			}
			else if ( CHANNELSTATUS == pQueueTask->enumEvent )
			{
				CTRLPRINTF(m_clsLogPtr,"通道状态线程退出 %d:%d:%d \n", struTime.wHour, struTime.wMinute, struTime.wSecond);
			}
			else if ( ACTIVETEST == pQueueTask->enumEvent )
			{
				CTRLPRINTF(m_clsLogPtr,"通道活动检查线程退出 %d:%d:%d \n", struTime.wHour, struTime.wMinute, struTime.wSecond);
			}
			else if (ACCEPTNOTICE == pQueueTask->enumEvent)
			{
				CTRLPRINTF(m_clsLogPtr,"通知上层新连接到达线程退出 %d:%d:%d \n", struTime.wHour, struTime.wMinute, struTime.wSecond);
			}

		}
		
		//这个任务完成了 释放该任务的内存。
		if ( NULL != pQueueTask )
		{
			free(pQueueTask);
			pQueueTask = NULL;
		}

		MoveToIdleVec(pclsThread);



	}

	m_GSMutexDequeThread.Lock();

	m_VecWaitDelThread.push_back(pclsThread);

	m_GSMutexDequeThread.Unlock();


}

INT CThreadPoolModule::AllocNewThreads(UINT uiThreadCount)
{
	UINT ixThreadData = m_uiCurThreadCount;
	while ( ixThreadData <  (m_uiCurThreadCount + uiThreadCount) )
	{
		CNetThread* pclsGSThread = new CNetThread();
		if ( NULL == pclsGSThread )
		{
			assert(0);
			return ERROR_NET_PARAM_WRONG;
		}
		pclsGSThread->Start(CThreadPoolModule::WorkerThreadProc, (void *)this);

		m_VectorThread.push_back(pclsGSThread);
		//同时也加入空闲队列
		m_VecIdleThread.push_back(pclsGSThread);

		//_beginthreadex(NULL,0,WorkerThreadProcEx,HANDLE(this),0,NULL);

		ixThreadData++; 
		// 达到最大线程数时退出
		if ( m_uiMaxThreadCount == ixThreadData )
		{
			break;
		}

	}
	// 保存当前的线程数
	m_uiCurThreadCount = ixThreadData;

	return ERROR_BASE_SUCCESS;
}


INT	CThreadPoolModule::MoveToBusyVec(CNetThread* pclsThread)
{
	m_GSMutexDequeThread.Lock();

	m_VecBusyThread.push_back(pclsThread);

	vector<CNetThread*>::iterator pIter;
	pIter = find(m_VecIdleThread.begin(), m_VecIdleThread.end(), pclsThread);
	if (pIter != m_VecIdleThread.end())
	{
		m_VecIdleThread.erase(pIter);

	}

	m_GSMutexDequeThread.Unlock();

	return ERROR_BASE_SUCCESS;
}

INT	CThreadPoolModule::MoveToIdleVec(CNetThread* pclsThread)
{
	m_GSMutexDequeThread.Lock();

	m_VecIdleThread.push_back(pclsThread);

	vector<CNetThread*>::iterator pIter;
	pIter = find(m_VecBusyThread.begin(), m_VecBusyThread.end(), pclsThread);
	if (pIter != m_VecBusyThread.end())
	{
		m_VecBusyThread.erase(pIter);

	}
	m_GSMutexDequeThread.Unlock();

	return ERROR_BASE_SUCCESS;
}



/********************************************************************************
  Function:		FreeThread
  Description:	释放线程
  Input:  		
  Output:      	   
  Return:  		       
  Note:					
  Author:        	CHC
  Date:				2010/09/01
********************************************************************************/
INT	CThreadPoolModule::FreeThread()
{
	CGSAutoMutex	GSAutoMutex(&m_GSMutexDequeThread);

	for (vector<CNetThread*>::size_type i=0; i<m_VecWaitDelThread.size(); ++i)
	{
		DeleteFromDeque( m_VecWaitDelThread[i] );

		vector<CNetThread*>::iterator pIter;
		pIter = find(m_VecIdleThread.begin(), m_VecIdleThread.end(), m_VecWaitDelThread[i]);
		if (pIter != m_VecIdleThread.end())
		{
			m_VecIdleThread.erase(pIter);

		}

		delete m_VecWaitDelThread[i]; 
	}

	m_VecWaitDelThread.clear();

	m_uiCurThreadCount = m_VectorThread.size();

	return 0;

}
INT16 CThreadPoolModule::GetIdleThreadCount()
{ 
	CGSAutoMutex	AutoMutex(&m_GSMutexDequeThread);
	return m_VecIdleThread.size();
}

