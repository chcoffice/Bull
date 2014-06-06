
#include "ISystemLayInterface.h"

/******************************************************************************
功能说明：进程间的锁
******************************************************************************/

/********************************************************************************************
Function		: CGSProcessMutex   
DateTime		: 2010/6/9 19:01	
Description		: 创建进程锁
Input			: NULL
Output			: NULL
Return			: NULL
Note			:				// 备注
********************************************************************************************/
CGSProcessMutex::CGSProcessMutex(const char *czKey)
{
#ifdef _WIN32
	
	m_GSProcessMutex=NULL;
	if (czKey==NULL)
	{
		return;
	}
	LPCWSTR lpMutexName=(WCHAR *)(czKey);
	m_GSProcessMutex=CreateMutex(NULL, FALSE, lpMutexName);
	
#elif _LINUX

	m_GSProcessMutex=0;
	if (czKey==NULL)
	{
		return;
	}
	m_GSProcessMutex=open(czKey,O_RDWR|O_CREAT,S_IWRITE);
	if(m_GSProcessMutex<0)
	{
		return;
	}

#endif

}

/********************************************************************************************
Function		: ~CGSProcessMutex   
DateTime		: 2010/6/9 19:04	
Description		: 删除锁
Input			: NULL
Output			: NULL
Return			: NULL
Note			:				// 备注
********************************************************************************************/
CGSProcessMutex::~CGSProcessMutex()
{

#ifdef _WIN32
	if (m_GSProcessMutex)
	{
		CloseHandle(m_GSProcessMutex);
		m_GSProcessMutex=NULL;
	}	
#elif _LINUX
	close(m_GSProcessMutex);
#endif

}

/********************************************************************************************
Function		: LockProcess   
DateTime		: 2010/6/9 19:08	
Description		: 加锁
Input			: NULL
Output			: NULL
Return			: 成功返回TRUE,失败返回FALSE
Note			:				// 备注
********************************************************************************************/
BOOL CGSProcessMutex::LockProcess()
{

#ifdef _WIN32
	if (m_GSProcessMutex)
	{
		return	WaitForSingleObject(m_GSProcessMutex, INFINITE)==WAIT_OBJECT_0 ? TRUE :FALSE;
	}
	else
	{
		return FALSE;
	}
#elif _LINUX

	if(lockf(m_GSProcessMutex,F_LOCK,0)<0) //参数使用F_LOCK，则如果已经加锁，则阻塞到前一个进程释放锁
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
#endif


}

/********************************************************************************************
Function		: TryLockProcess   
DateTime		: 2010/6/9 19:10	
Description		: 非阻塞的加锁
Input			: NULL
Output			: NULL
Return			: 成功返回TRUE,失败返回FALSE
Note			:				// 备注
********************************************************************************************/
BOOL CGSProcessMutex::TryLockProcess()
{

#ifdef _WIN32
	if (m_GSProcessMutex)
	{
		return	WaitForSingleObject(m_GSProcessMutex, 0)==WAIT_OBJECT_0 ? TRUE :FALSE;
	}
	else
	{
		return FALSE;
	}
#elif _LINUX

	if(lockf(m_GSProcessMutex,F_TLOCK,0)<0) //参数使用F_TLOCK，当文件已经被加锁时，调用进程不会阻塞而是直接打印错误信息并返回
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
#endif

}

/********************************************************************************************
Function		: UnlockProcess    
DateTime		: 2010/6/9 19:11	
Description		: 解锁
Input			: NULL
Output			: NULL
Return			: NULL
Note			:				// 备注
********************************************************************************************/
void CGSProcessMutex::UnlockProcess()
{

#ifdef _WIN32
	if (m_GSProcessMutex)
	{
		ReleaseMutex(m_GSProcessMutex);
	}	
#elif _LINUX
	lockf(m_GSProcessMutex,F_ULOCK,0);
#endif

}
