#include "ISystemLayInterface.h"
/******************************************************************************
功能说明：线程部分
******************************************************************************/

/********************************************************************************************
  Function		: CGSThread    
  DateTime		: 2010/6/9 17:28	
  Description	: CGSThread类中变量的初始化
  Input			: NULL
  Output		: NULL
  Return		: NULL
  Note			: NULL
********************************************************************************************/
CGSThread::CGSThread()
{
	//初始化变量标识
	m_bExit = FALSE;
	m_bExiting = FALSE;
	m_bPause = FALSE;
	m_bRunningState = FALSE;
	m_bJoin=FALSE;
	m_bDetached=FALSE;
	m_bMutexLock =	FALSE;
	m_bthreadfinish = FALSE;

	m_GSCond = NULL;
	m_GSCondPause = NULL;
	m_GSMutexPause=NULL;
	m_GSMutexUnit = NULL;
	m_GSMutexthreadfinish = NULL;
	

	if(!m_GSCond)
	{
		m_GSCond = new CGSCond;
	}
	if (!m_GSCondPause)
	{
		m_GSCondPause = new	CGSCond;
	}
	if(!m_GSMutexPause)
	{
		m_GSMutexPause = new CGSMutex;
	}
	if (!m_GSMutexUnit)
	{
		m_GSMutexUnit = new CGSMutex;
	}
	if (!m_GSMutexthreadfinish)
	{
		m_GSMutexthreadfinish = new CGSMutex;
	}

	m_fnUser = NULL;
	m_pFnUserParam = NULL;

#ifdef	_WIN32	
	m_GSThread = NULL;
#endif

}

//析构函数
/********************************************************************************************
  Function		: ~CGSThread   
  DateTime		: 2010/6/9 17:29	
  Description	: 释放CGSThread类中的资源
  Input			: NULL
  Output		: NULL
  Return		: NULL
  Note			: NULL
********************************************************************************************/
CGSThread::~CGSThread()
{
	Stop();
	if (m_GSThread)
	{
		m_GSMutexUnit->Lock();
		UnInitData();
		m_GSMutexUnit->Unlock();
	}	
	if(m_GSCond)
	{
		delete m_GSCond;
		m_GSCond = NULL;
	}
	if (m_GSCondPause)
	{
		delete	m_GSCondPause;
		m_GSCondPause = NULL;
	}
	if(m_GSMutexPause)
	{
		delete m_GSMutexPause;
		m_GSMutexPause = NULL;
	}
	if (m_GSMutexUnit)
	{
		delete	m_GSMutexUnit;
		m_GSMutexUnit = NULL;
	}
	if (m_GSMutexthreadfinish)
	{
		delete m_GSMutexthreadfinish;
		m_GSMutexthreadfinish = NULL;
	}
	
}

/********************************************************************************************
  Function		: UnInitData    
  DateTime		: 2010/6/9 17:30	
  Description	: widows:释放线程句柄
				  linux: 分离程序
  Input			: NULL
  Output		: NULL
  Return		: NULL
  Note			: NULL
********************************************************************************************/
void CGSThread::UnInitData()
{
	
#ifdef _WIN32
	if(m_GSThread)
	{
		CloseHandle(m_GSThread);
		m_GSThread = NULL;
	}
#elif _LINUX
	Detach();
#endif
	m_bRunningState = FALSE;
}

//CGSThread线程过程
/********************************************************************************************
  Function		: CGSThreadProcFunc    
  DateTime		: 2010/6/9 17:33	
  Description	: 执行线程
  Input			: HANDLE param：线程函数的参数
  Output		: NULL
  Return		: windows:返回0，linux:返回线程函数的参数指针
  Note			: NULL
********************************************************************************************/
#ifdef _WIN32
//DWORD WINAPI CGSThreadProcFunc(HANDLE param)
unsigned   _stdcall CGSThreadProcFunc(HANDLE param)
#elif _LINUX
void *CGSThreadProcFunc(HANDLE param)
#endif 
{	
	CGSThread * pGSThread = (CGSThread *)param;

#ifdef _WIN32
	DWORD dwRet = 0;
	if(! pGSThread)
	{
		return dwRet;
	}
#endif
	//设置线程运行状态

	if(pGSThread->m_fnUser)
	{
		//执行用户线程函数
		pGSThread->m_fnUser(pGSThread,pGSThread->m_pFnUserParam);
	}
	//发送信号，唤醒join等待
	//pGSThread->m_GSCond->Signal();

	pGSThread->m_GSMutexthreadfinish->Lock();
	pGSThread->m_bthreadfinish = TRUE;
	pGSThread->m_bRunningState = FALSE;	
	pGSThread->m_GSMutexthreadfinish->Unlock();

	pGSThread->m_bExit = TRUE;
	//释放资源
	pGSThread->UnInitData();
	if (pGSThread->m_bMutexLock)
	{
		pGSThread->m_GSMutexUnit->Unlock();
		pGSThread->m_bMutexLock = FALSE;
	}
	//发送信号，唤醒join等待
	pGSThread->m_GSCond->Signal();

#ifdef _WIN32	
	return dwRet;
#elif _LINUX
	return pGSThread;
#endif 
}


//CGSThread线程开始
/********************************************************************************************
  Function		: Start    
  DateTime		: 2010/6/9 17:36	
  Description	: 创建线程
  Input			: GSThreadCallbackFunction fnOnEvent：用户线程函数；void *pUserParam：用户线程参数
  Output		: NULL
  Return		: 成功返回TRUE，失败返回FALSE
  Note			: NULL
********************************************************************************************/
BOOL CGSThread::Start(GSThreadCallbackFunction fnOnEvent/* =NULL */, void *pUserParam/* =NULL */)
{
	m_fnUser = fnOnEvent;
	m_pFnUserParam = pUserParam;
	m_bRunningState=FALSE;
	if (m_bMutexLock)
	{
		return	FALSE;
	}
	m_GSMutexUnit->Lock();
	m_bMutexLock = TRUE;

#ifdef WINCE
	m_GSThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)CGSThreadProcFunc,HANDLE(this),0,NULL);
	if (m_GSThread)
	{
		m_GSMutexthreadfinish->Lock();
		if (m_bthreadfinish)
		{
			m_bRunningState=FALSE;
			//释放资源
			UnInitData();
		}
		else
		{
			m_bRunningState=TRUE;
		}
		m_GSMutexthreadfinish->Unlock();
	}
#elif _WIN32
	//创建线程
	//m_GSThread = CreateThread(NULL,0,CGSThreadProcFunc,HANDLE(this),0,NULL);

	//创建线程
	m_GSThread = (HANDLE)_beginthreadex(NULL,0,CGSThreadProcFunc,HANDLE(this),0,NULL);
	
	if (m_GSThread)
	{
		m_GSMutexthreadfinish->Lock();
		if (m_bthreadfinish)
		{
			m_bRunningState=FALSE;
			//释放资源
			UnInitData();
		}
		else
		{
			m_bRunningState=TRUE;
		}
		m_GSMutexthreadfinish->Unlock();
	}
#elif _LINUX
	//创建线程
	if(pthread_create(&m_GSThread,0,CGSThreadProcFunc,HANDLE(this))==0)
	{
		m_GSMutexthreadfinish->Lock();
		if (m_bthreadfinish)
		{
			m_bRunningState=FALSE;
			//释放资源
			UnInitData();
		}
		else
		{
			m_bRunningState=TRUE;
		}
		m_GSMutexthreadfinish->Unlock();
	}
#endif 
	else
	{
		m_GSMutexUnit->Unlock();
		m_bMutexLock = FALSE;
	}

	return m_bRunningState;
}


//停止线程
/********************************************************************************************
  Function		: Stop    
  DateTime		: 2010/6/9 17:38	
  Description	: 设置停止线程标识
  Input			: NULL
  Output		: NULL
  Return		: 成功返回TRUE,失败返回FALSE
  Note			: NULL
********************************************************************************************/
BOOL CGSThread::Stop()
{
	//判断线程是否在运行
	if(m_bRunningState)
	{
		//设置停止线程标识
		m_bExiting = TRUE;
	}
	return m_bExiting;

}

/********************************************************************************************
  Function		: IsRunning    
  DateTime		: 2010/6/9 17:40	
  Description	: 返回线程运行状态
  Input			: NULL
  Output		: NULL
  Return		: 线程正在运行返回TRUE,否则返回FALSE
  Note			:				// 备注
********************************************************************************************/
BOOL CGSThread::IsRunning()
{
	return m_bRunningState;
}

//线程挂起
/********************************************************************************************
  Function		: Suspend   
  DateTime		: 2010/6/9 17:42	
  Description	: 设置线程休眠状态
  Input			: NULL
  Output		: NULL
  Return		: 成功返回TRUE,失败返回FALSE
  Note			:				// 备注
********************************************************************************************/
BOOL CGSThread::Suspend()
{
	//判断线程运行状态
	if(m_bRunningState)
	{
		//设置线程休眠状态
		m_GSMutexPause->Lock();
		m_bPause = TRUE;
		m_GSMutexPause->Unlock();
	}

	return m_bPause;

}

//线程睡眠
/********************************************************************************************
  Function		: Resume    
  DateTime		: 2010/6/9 17:43	
  Description	: 发送信号唤醒线程，设置线程休眠状态
  Input			: NULL
  Output		: NULL
  Return		: 成功返回TRUE,失败返回FALSE
  Note			:				// 备注
********************************************************************************************/
BOOL CGSThread::Resume()
{
	if(m_bRunningState)
	{
		//判断线程休眠状态
		if(m_bPause)
		{
			//唤醒线程
			m_GSMutexPause->Lock();
			m_GSCondPause->Signal();
			m_bPause = FALSE;
			m_GSMutexPause->Unlock();
		}
	}
	//返回线程休眠状态
	return m_bPause;
}

//等待线程结束
/********************************************************************************************
  Function		: Join   
  DateTime		: 2010/6/9 17:44	
  Description	: 等待线程正常退出
  Input			: NULL
  Output		: NULL
  Return		: 成功返回TRUE,失败返回FALSE
  Note			:				// 备注
********************************************************************************************/
BOOL CGSThread::Join()
{
	//判断线程是否在运行，是否已join过
	if(m_bRunningState&&!m_bJoin)
	{
		//判断线程的休眠状态
		if(m_bPause)
		{
			//唤醒线程
			m_GSMutexPause->Lock();
			m_GSCondPause->Signal();
			m_bPause = FALSE;
			m_GSMutexPause->Unlock();;
		}
		//设置线程退出标识
		m_bExiting = TRUE;

#ifdef _WIN32
		//等待线程退出
		if(0 == m_GSCond->Wait())
		{
			//设置线程退出标志位和join标志位
			m_bExit=TRUE;
			m_bJoin=TRUE;
		}
#elif _LINUX
		//等待线程退出
		if (pthread_join(m_GSThread,NULL)==0)
		{
			m_bJoin=TRUE;
		}		
#endif
	}
	//返回线程Join标志位
	return m_bJoin;

}


/********************************************************************************************
  Function		: Join  
  DateTime		: 2010/6/9 17:47	
  Description	: 等待线程一段时间，判断是否退出 
  Input			: INT mseconds:等待时间，毫秒
  Output		: NULL
  Return		: 成功返回TRUE,失败返回FALSE
  Note			:				// 备注
********************************************************************************************/
BOOL CGSThread::Join(INT mseconds)
{
	//判断线程是否在运行，是否已join过
	if(m_bRunningState&&!m_bJoin)
	{
		//判断线程的休眠状态
		if(m_bPause)
		{
			//唤醒线程
			m_GSMutexPause->Lock();
			m_GSCondPause->Signal();
			m_bPause = FALSE;
			m_GSMutexPause->Unlock();;
		}
		//设置线程退出标识
		m_bExiting = TRUE;

#ifdef _WIN32
		//等待线程退出
		if(0 == m_GSCond->WaitTimeout(mseconds))
		{
			//设置线程退出标志位和join标志位
			m_bExit=TRUE;
			m_bJoin=TRUE;
		}

#elif _LINUX
		//等待线程退出
		if (pthread_join(m_GSThread,NULL)==0)
		{
			m_bExit=TRUE;
			m_bJoin=TRUE;
		}		
#endif
	}
	//返回线程Join标志位
	return m_bJoin;

}

/********************************************************************************************
  Function		: Cancel   
  DateTime		: 2010/6/9 17:51	
  Description	: 强制退出线程
  Input			: NULL
  Output		: NULL
  Return		: NULL
  Note			:				// 备注
********************************************************************************************/
void CGSThread::Cancel()
{
	//判断线程运行状态
	if(m_bRunningState)
	{
		//设置线程退出状态
		m_bExiting = TRUE;
		
		//判断线程休眠状态
		if(m_bPause)
		{
			//唤醒线程
			m_GSMutexPause->Lock();
			m_GSCondPause->Signal();
			m_bPause = FALSE;
			m_GSMutexPause->Unlock();;
		}
		//判断线程退出状态
		if(!m_bExit)
		{
			//强制终止线程
			Kill();
		}
	}

}

/********************************************************************************************
  Function		: TestExit   
  DateTime		: 2010/6/9 17:57	
  Description	: 检测线程的退出状态，执行线程的join唤醒和休眠
  Input			: NULL
  Output		: NULL
  Return		: 返回TRUE，线程已退出，返回FALSE,线程未退出
  Note			:				// 备注
********************************************************************************************/
BOOL CGSThread::TestExit()
{

	//判断线程退出标识
	if(m_bExiting==TRUE)
	{
		m_bExit = TRUE;
	}
	//判断线程的休眠状态
	else if(m_bPause==TRUE)
	{
		//线程休眠
		m_GSMutexPause->Lock();
		if (m_bPause)
		{
			m_GSMutexPause->Unlock();
			m_GSCondPause->Wait();			
		}
		else
		{
			m_GSMutexPause->Unlock();
		}
		
	}
	
	//释放资源改为线程执行完毕后释放，在线程代理函数的结尾释放，hf于2010.11.01修改
	//判断线程的退出状态
	//if (m_bExit==TRUE)
	//{
	//	//释放线程句柄
	//	UnInitData();
	//	return TRUE;
	//}

	return m_bExit;
}


/********************************************************************************************
  Function		: Kill    
  DateTime		: 2010/6/9 18:06	
  Description	: 强制终止线程
  Input			: NULL
  Output		: NULL
  Return		: NULL
  Note			:				// 备注
********************************************************************************************/
void CGSThread::Kill()
{
	//判断线程运行状态和退出状态
	if(m_bRunningState || (! m_bExit))
	{
#ifdef _WIN32

		//终止线程
		DWORD dwExitCode;
		GetExitCodeThread(m_GSThread,&dwExitCode);
		TerminateThread(m_GSThread,dwExitCode);
#elif _LINUX
		//强制退出线程
		pthread_cancel(m_GSThread);
#endif
		//释放线程句柄
		UnInitData();
		if (m_bMutexLock)
		{
			m_GSMutexUnit->Unlock();
			m_bMutexLock = FALSE;
		}
	}
}

/********************************************************************************************
  Function		:  Detach   
  DateTime		: 2010/6/9 18:06	
  Description	:  分离线程
  Input			:  NULL
  Output		:  NULL
  Return		:  NULL
  Note			:				// 备注
********************************************************************************************/
void CGSThread::Detach()
{
#ifdef _LINUX
	//判断线程的分离标识和Join标识
	if (!m_bDetached&&!m_bJoin)
	{
		//分离线程
		if (pthread_detach(m_GSThread)==0)
		{
			m_bDetached=TRUE;
		}		
	}
#endif

}
/********************************************************************************************
Function		:  GetThreadHandle   
DateTime		: 2010/9/6 09:06	
Description		:  获取线程句柄
Input			:  NULL
Output			:  GSThread:线程句柄，失败返回NULL
Return			:  NULL
Note			:				// 备注
********************************************************************************************/
GSThread CGSThread::GetThreadHandle()
{

	if (m_GSThread)
	{
		return	m_GSThread;
	}
	return	NULL;
}
