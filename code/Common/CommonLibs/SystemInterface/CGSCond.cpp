#include "ISystemLayInterface.h"


/******************************************************************************
功能说明：条件变量
******************************************************************************/

/********************************************************************************************
  Function		: CGSCond    
  DateTime		: 2010/6/10 9:38	
  Description	: 创建条件变量
  Input			: NULL
  Output		: NULL
  Return		: NULL
  Note			:				// 备注
********************************************************************************************/

CGSCond::CGSCond()
{
	
#ifdef _WIN32
	//win32的条件变量锁
	m_mutex = CreateMutexA (NULL, false, NULL); 
	m_GSCond = NULL;
	m_GSCond = CreateEvent(NULL, FALSE, FALSE, NULL);
#elif _LINUX
	//linux的条件变量锁
	m_CondMutex = NULL;
	if (!m_CondMutex)
	{
		m_CondMutex=new CGSMutex();
	}
	pthread_cond_init(&m_GSCond, NULL);
#endif	
}


/********************************************************************************************
  Function		: ~CGSCond    
  DateTime		: 2010/6/10 9:41	
  Description	: 销毁锁
  Input			: NULL
  Output		: NULL
  Return		: NULL
  Note			:				// 备注
********************************************************************************************/
CGSCond::~CGSCond()
{

#ifdef _WIN32
	CloseHandle(m_mutex);
	CloseHandle(m_GSCond);
	m_GSCond = NULL;
	m_mutex = NULL;
#elif _LINUX
	if (m_CondMutex)
	{
		delete	m_CondMutex;
		m_CondMutex = NULL;
	}
	pthread_cond_destroy(&m_GSCond);
#endif
}


/********************************************************************************************
  Function		: Wait   
  DateTime		: 2010/6/10 9:42	
  Description	: 等待条件变量
  Input			: NULL
  Output		: NULL
  Return		: 返回0成功，其他表示失败
  Note			:				// 备注
********************************************************************************************/
INT CGSCond::Wait()
{
#ifdef _WIN32
	INT32 iRet = -1;
	WaitForSingleObject(m_mutex,INFINITE);	
	iRet =SignalObjectAndWait(m_mutex, m_GSCond,INFINITE,FALSE) == WAIT_OBJECT_0 ? 0 : -1;
	ResetEvent(m_GSCond);

	return	iRet;

#elif _LINUX
	INT32 iRet = 0;
	m_CondMutex->Lock();
	iRet = pthread_cond_wait(&m_GSCond,&m_CondMutex->m_GSMutex);
	m_CondMutex->Unlock();
	return iRet;
#endif
}


/********************************************************************************************
  Function		: WaitTimeout    
  DateTime		: 2010/6/10 9:42	
  Description	: 等待条件变量，超时返回
  Input			: INT mseconds：等待的时间，毫秒
  Output		: NULL
  Return		: 返回0成功，其他表示失败
  Note			:				// 备注
********************************************************************************************/
INT CGSCond::WaitTimeout(INT mseconds)
{
#ifdef _WIN32

	INT iRet = -1;
	if (WaitForSingleObject(m_mutex,mseconds) == WAIT_OBJECT_0)
	{	
		iRet = SignalObjectAndWait(m_mutex, m_GSCond,mseconds,FALSE) == WAIT_OBJECT_0 ? 0 : -1;
		ResetEvent(m_GSCond);
	}
	return	iRet;

#elif _LINUX
	INT32	iRet = 0;
	struct timeval struTimeVal;
	struct timespec struTimeSpec;
	gettimeofday(&struTimeVal, NULL);
	struTimeSpec.tv_sec  = mseconds/1000;
	struTimeSpec.tv_nsec =1000L *(struTimeVal.tv_usec+(mseconds-struTimeSpec.tv_sec*1000)*1000L);
	struTimeSpec.tv_sec += struTimeVal.tv_sec;
	
	m_CondMutex->Lock();
	iRet = pthread_cond_timedwait( &m_GSCond, &m_CondMutex->m_GSMutex, &struTimeSpec);
	m_CondMutex->Unlock();

	return iRet;
#endif
}

/********************************************************************************************
  Function		: Signal    
  DateTime		: 2010/6/10 9:45	
  Description	: 发送信号
  Input			: NULL
  Output		: NULL
  Return		: 返回0成功，其他表示失败
  Note			:				// 备注
********************************************************************************************/
INT CGSCond::Signal()
{
#ifdef _WIN32
	DWORD dwRes;
	INT iRet=0;

	WaitForSingleObject(m_mutex,INFINITE);
	dwRes=SetEvent(m_GSCond);
	if (dwRes==0)
	{
		iRet  = (GetLastError() == ERROR_SUCCESS ? TRUE : FALSE);
	}
	ReleaseMutex (m_mutex);  
	return iRet;
#elif _LINUX
	return pthread_cond_signal(&m_GSCond);
#endif
}

//发送广播信号
/********************************************************************************************
  Function		: BroadcastSignal    
  DateTime		: 2010/6/10 9:46	
  Description	: 发送广播信号
  Input			: NULL
  Output		: NULL
  Return		: 返回0成功，其他表示失败
  Note			:				// 备注
********************************************************************************************/
INT CGSCond::BroadcastSignal()
{
#ifdef _WIN32
	DWORD dwRes;
	INT iRet=0;

	WaitForSingleObject(m_mutex,INFINITE);
	dwRes=SetEvent(m_GSCond);
	if (dwRes==0)
	{
		iRet  = (GetLastError() == ERROR_SUCCESS ? TRUE : FALSE);
	}
	ReleaseMutex (m_mutex);  
	return iRet;
#elif _LINUX
	return pthread_cond_broadcast(&m_GSCond);
#endif
}




/*
*********************************************************************
*
*@brief : CGSCondEx 实现
*
*********************************************************************
*/
CGSCondEx::CGSCondEx(void)
{
#ifdef _WIN32
	m_iWaitConts = 0;
	m_hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	if( !m_hEvent )
	{
		assert(0);
		_exit(-2);
	}
#else
	//linux的条件变量锁
	::pthread_cond_init(&m_hEvent, NULL);
#endif	

}

CGSCondEx::~CGSCondEx(void)
{
#ifdef _WIN32
	if( m_hEvent )
	{
		::CloseHandle(m_hEvent );
		m_hEvent = NULL;
	}
#else
	::pthread_cond_destroy(&m_hEvent);
#endif	
}

INT	CGSCondEx::Wait( CGSMutex *pMutex )
{
#ifdef _WIN32
	DWORD wRet;
	m_iWaitConts++;
	pMutex->Unlock();
	wRet = WaitForSingleObject(m_hEvent,INFINITE);
	ResetEvent(m_hEvent);
	pMutex->Lock();
	m_iWaitConts--;
	if (wRet == WAIT_OBJECT_0)
	{
		return R_SUCCESS;
	} 			
	return R_ESYSTEM;
#else
	INT32 iRet = 0;	
	iRet = ::pthread_cond_wait(&m_hEvent,&pMutex->m_GSRwmutex);
	return (iRet==0 ? R_SUCCESS ? R_ESYSTEM);
#endif

}


INT	CGSCondEx::WaitTimeout(CGSMutex *pMutex, INT mseconds)
{
#ifdef _WIN32
	DWORD wRet;
	m_iWaitConts++;
	pMutex->Unlock();
	wRet = ::WaitForSingleObject(m_hEvent,mseconds);
	ResetEvent(m_hEvent);
	pMutex->Lock();
	m_iWaitConts--;

	if (wRet == WAIT_OBJECT_0)
	{
		return R_SUCCESS;
	} 
	else if( wRet == WAIT_TIMEOUT )
	{
		return R_ETIMEOUT;
	}
	return R_ESYSTEM;

#else
	struct timespec ts;
	struct timeval tv;
	struct timezone tz;
	int sec, usec;

	gettimeofday(&tv, &tz);
	sec = mseconds / 1000;
	mseconds = mseconds - (sec * 1000);
	assert( mseconds < 1000 );
	usec = mseconds * 1000;
	assert(tv.tv_usec < 1000000);
	ts.tv_sec = tv.tv_sec + sec;
	ts.tv_nsec = (tv.tv_usec + usec) * 1000;
	assert(ts.tv_nsec < 2000000000);
	if(ts.tv_nsec > 999999999)
	{
		ts.tv_sec++;
		ts.tv_nsec -= 1000000000;
	}

	INT32 iRet = 0;		
	iRet = ::pthread_cond_timedwait(&m_hEvent,&pMutex->m_GSRwmutex, &ts);
	if( iRet )
	{
		if(iRet==ETIMEDOUT )
		{
			return R_ETIMEOUT;
		}
		else
		{
			return R_ESYSTEM;
		}
	}
	return R_SUCCESS;
#endif

}

INT	CGSCondEx::Signal(void)
{
#ifdef _WIN32
	if( ::SetEvent(m_hEvent) )
	{
		return R_SUCCESS;
	}
	return R_ESYSTEM;
#else
	if( ::pthread_cond_signal(&m_hEvent) )
	{
		return R_ESYSTEM;
	}
	return R_SUCCESS;
#endif

}


INT	CGSCondEx::BroadcastSignal(void)
{
#ifdef _WIN32
	::SetEvent(m_hEvent);
	INT x = m_iWaitConts-1;
	for( ;x>0; x-- )
	{

		if( !::SetEvent(m_hEvent) )
		{
			assert(0);
		}
		MSLEEP(1);
	}
	//ResetEvent(m_hEvent);
	return R_SUCCESS;


#else
	if( ::pthread_cond_broadcast(&m_hEvent) )
	{
		return R_ESYSTEM;
	}
	return R_SUCCESS;
#endif
}