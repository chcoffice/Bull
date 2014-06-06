/******************************************************************************
功能说明：信号量
******************************************************************************/

#include "ISystemLayInterface.h"

/********************************************************************************************
  Function		: CGSSem   
  DateTime		: 2010/6/10 9:48	
  Description	: 创建信号量
  Input			: const char *czKey ：信号量的名称；BOOL bProcess：是否是进程间的信号量
  Output		: NULL
  Return		: NULL
  Note			:				// 备注
********************************************************************************************/
#ifdef WINCE
HANDLE OpenSemaphore(DWORD dwState,bool bFlag,WCHAR* wcKey)
{
	return  CreateSemaphore(NULL,GS_INIT_SEM_COUNT,GS_MAX_SEM_COUNT,wcKey);
}
#endif

CGSSem::CGSSem( const char *czKey,BOOL bProcess)
{
	m_bIsValid=FALSE;
	m_GSSem=NULL;


#ifdef _WIN32

	WCHAR * wcKey = (WCHAR *)(czKey);

	if(NULL == czKey)
	{
		return;
	}

	//判断信号量是否已存在，若存在，则打开信号量
	if( (m_GSSem=OpenSemaphore(SEMAPHORE_ALL_ACCESS,FALSE,wcKey))==NULL)
	{
		//如果没有其他进程创建这个信号灯，则创建信号量，如执行成功，返回信号量对象的句柄；零表示出错
		m_GSSem = CreateSemaphore(NULL,GS_INIT_SEM_COUNT,GS_MAX_SEM_COUNT,wcKey);
	} 

	if (m_GSSem)
	{
		m_bIsValid=TRUE;
	}

#elif _LINUX

	char *wcKey=(char *)(czKey);
	if (NULL==czKey)
	{
		return;
	}

	//判断信号量是否已存在，若存在，则打开信号量
	if ((m_GSSem=sem_open(wcKey, 0))==SEM_FAILED)
	{
		//创建信号灯，成功时返回指向信号量的指针，出错时为SEM_FAILED
		if ((m_GSSem=sem_open(wcKey,O_RDWR | O_CREAT | O_EXCL,0644, 1))==SEM_FAILED)
		{
			m_bIsValid=FALSE;
			return;
		}
	}
	m_bIsValid=TRUE;

#endif

}

//析构函数
/********************************************************************************************
  Function		: ~CGSSem   
  DateTime		: 2010/6/10 10:05	
  Description	: 释放信号量的指针
  Input			: NULL
  Output		: NULL
  Return		: NULL
  Note			:				// 备注
********************************************************************************************/
CGSSem::~CGSSem()
{

	if (m_GSSem)
	{
#ifdef _WIN32
		CloseHandle(m_GSSem);
		m_GSSem=NULL;
#elif _LINUX
		sem_close(m_GSSem);
		m_GSSem=NULL;
#endif

	}
	m_bIsValid=FALSE;

}


/********************************************************************************************
  Function		: IsValid   
  DateTime		: 2010/6/10 10:06	
  Description	: 返回信号量的有效标识
  Input			: NULL
  Output		: NULL
  Return		: NULL
  Note			:				// 备注
********************************************************************************************/
BOOL CGSSem::IsValid()
{
	return m_bIsValid;
}

//发送信号量
/********************************************************************************************
  Function		: Signal    
  DateTime		: 2010/6/10 10:07	
  Description	: 递增一个信号量计数
  Input			:  NULL
  Output		:         	// 对输出参数的说明。
  Return		: TRUE表示成功，FALSE表示失败
  Note			:				// 备注
********************************************************************************************/
BOOL CGSSem::Signal()
{
	BOOL bRet=TRUE;
#ifdef _WIN32

	//递增一个信号量计数,TRUE表示成功，FALSE表示失败。
	bRet=ReleaseSemaphore(m_GSSem, 1, NULL);

#elif _LINUX

	//把所指定的信号量的值加1，然后唤醒正在等待该信号灯值变为正数的任意线程。若成功则返回0，否则返回-1。
	if(sem_post(m_GSSem)!=0)
	{
		bRet=FALSE;
	}

#endif
	return bRet;
}


/********************************************************************************************
  Function		: Wait    
  DateTime		: 2010/6/10 10:28	
  Description	: 等待获取信号量
  Input			: NULL
  Output		: NULL
  Return		: 成功返回TRUE，失败返回FALSE
  Note			:				// 备注
********************************************************************************************/
BOOL CGSSem::Wait()
{

	BOOL bRet=TRUE;

#ifdef _WIN32

	bRet=WaitForSingleObject(m_GSSem, INFINITE) == WAIT_OBJECT_0 ? TRUE :FALSE;

#elif _LINUX

	if(sem_wait(m_GSSem)!=0)
	{
		bRet=FALSE;
	}

#endif

	return bRet;
}

/********************************************************************************************
Function		: Wait    
DateTime		: 2010/6/10 10:28	
Description		: 等待一段时间获取信号量
Input			: UINT mSeconds：等待的时间，毫秒
Output			: NULL
Return			: 成功返回TRUE，失败返回FALSE
Note			:				// 备注
********************************************************************************************/
BOOL CGSSem::Wait(UINT mSeconds)
{
	BOOL bRet=TRUE;

#ifdef _WIN32

	DWORD dwWaitTime=(DWORD)mSeconds;
	bRet= WaitForSingleObject(m_GSSem, dwWaitTime) == WAIT_TIMEOUT  ? FALSE :TRUE;

#elif _LINUX

	struct timeval struTimeVal;
	struct timespec struTimeSpec;
	gettimeofday(&struTimeVal, NULL);
	struTimeSpec.tv_sec  = mSeconds/1000;
	struTimeSpec.tv_nsec =1000L *(struTimeVal.tv_usec+(mSeconds-struTimeSpec.tv_sec*1000)*1000L);
	struTimeSpec.tv_sec += struTimeVal.tv_sec;

	if(sem_timedwait(m_GSSem,  &struTimeSpec)!=0)
	{
		bRet=FALSE;
	}

#endif

	return bRet;
}
