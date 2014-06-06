
#include "ISystemLayInterface.h"

/******************************************************************************
功能说明：普通锁
******************************************************************************/

/********************************************************************************************
  Function		: CGSMutex   
  DateTime		: 2010/6/9 19:01	
  Description	: 初始化锁
  Input			: NULL
  Output		: NULL
  Return		: NULL
  Note			:				// 备注
********************************************************************************************/
CGSMutex::CGSMutex()
{
#ifdef _WIN32
	InitializeCriticalSection(&m_GSMutex);
#elif _LINUX
	pthread_mutex_init( &m_GSMutex,NULL);
#endif

}


/********************************************************************************************
  Function		: ~CGSMutex   
  DateTime		: 2010/6/9 19:04	
  Description	: 删除锁
  Input			: NULL
  Output		: NULL
  Return		: NULL
  Note			:				// 备注
********************************************************************************************/
CGSMutex::~CGSMutex()
{
	//Unlock();

#ifdef _WIN32
	DeleteCriticalSection(&m_GSMutex);
#elif _LINUX
	pthread_mutex_destroy(&m_GSMutex);
#endif

}


/********************************************************************************************
  Function		: Lock   
  DateTime		: 2010/6/9 19:08	
  Description	: 加锁
  Input			: NULL
  Output		: NULL
  Return		: 成功返回TRUE,失败返回FALSE
  Note			:				// 备注
********************************************************************************************/
BOOL CGSMutex::Lock()
{
	BOOL bRet=TRUE;

#ifdef _WIN32
	EnterCriticalSection(&m_GSMutex);
#elif _LINUX
	if(pthread_mutex_lock(&m_GSMutex)!=0)
	{
		bRet=FALSE;
	};
#endif
	return bRet;
}

//
/********************************************************************************************
  Function		: TryLock   
  DateTime		: 2010/6/9 19:10	
  Description	: 非阻塞的加锁
  Input			: NULL
  Output		: NULL
  Return		: 成功返回TRUE,失败返回FALSE
  Note			:				// 备注
********************************************************************************************/
BOOL CGSMutex::TryLock()
{
	BOOL bRet=TRUE;

#ifdef _WIN32
	bRet=TryEnterCriticalSection(&m_GSMutex);
#elif _LINUX
	if (pthread_mutex_trylock(&m_GSMutex)!=0)
	{
		bRet=FALSE;
	}

#endif
	return bRet;
}


/********************************************************************************************
  Function		: Unlock    
  DateTime		: 2010/6/9 19:11	
  Description	: 解锁
  Input			: NULL
  Output		: NULL
  Return		: NULL
  Note			:				// 备注
********************************************************************************************/
void CGSMutex::Unlock()
{
#ifdef _WIN32
		LeaveCriticalSection(&m_GSMutex);
#elif _LINUX
		pthread_mutex_unlock(&m_GSMutex);	
#endif

}


