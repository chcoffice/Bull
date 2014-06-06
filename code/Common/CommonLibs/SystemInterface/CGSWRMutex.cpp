
#ifdef WINCE
#pragma once
#include <Windows.h>
//模拟win32下的_InterlockedAnd.
//如果lpAddend在线程中被改变则lpAddend!=oldValue,则继续等待.
long _InterlockedAnd(PLONG lpAddend,LONG flag)
{
	long lReturn;

	while (1)
	{
		long oldValue = *lpAddend;

		lReturn = InterlockedCompareExchange(lpAddend, oldValue & flag, oldValue);
		if (lReturn == oldValue)
			break;
	}

	return lReturn;
}
long _InterlockedOr(PLONG lpAddend,LONG flag)
{
	long lReturn;
	while (1)
	{
		long oldValue = *lpAddend;

		lReturn = InterlockedCompareExchange(lpAddend, oldValue | flag, oldValue);
		if (lReturn == oldValue)
			break;
	}

	return lReturn;
}
#elif _WIN32
#pragma once
#include <intrin.h>
#endif
#include "ISystemLayInterface.h"

/******************************************************************************
功能说明：读写锁
这个读写锁的特点： 
一、数据结构简单，仅使用两个event对象和两个计数变量。 
二、作为写优先的读写锁，支持多个写线程。 
三、其工作方式非常高效。在没有线程执行写操作时，其读锁的获取和释放过程不产生任何系统调用。 
四、整个工作过程不存在轮询等待。 
******************************************************************************/

/********************************************************************************************
Function		: CGSWRMutex   
DateTime		: 2010/6/9 19:01	
Description		: 初始化锁
Input			: NULL
Output			: NULL
Return			: NULL
Note			:				// 备注
********************************************************************************************/
CGSWRMutex::CGSWRMutex()

#ifdef _WIN32

//初始化读写锁标识变量
: m_readCount(0)
, m_writeCount(0)
{
	//创建读写锁事件
	m_GSReadEvent = CreateEvent( NULL, TRUE, TRUE, NULL );
	m_GSWriteEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

#ifdef _MUTEX_DEBUG
	m_iRRefDebug = 0;
	m_iWRefDebug = 0;
#endif 
}

#elif _LINUX
{
	//初始化读写锁
	if (pthread_rwlock_init( &m_GSRwmutex, NULL )!=0)
	{
		return;
	}
}	
#endif


/********************************************************************************************
Function		: ~CGSWRMutex   
DateTime		: 2010/6/9 19:04	
Description		: 销毁锁
Input			: NULL
Output			: NULL
Return			: NULL
Note			:				// 备注
********************************************************************************************/
CGSWRMutex::~CGSWRMutex()
{
#ifdef _WIN32

	//关闭读写锁
	CloseHandle( m_GSReadEvent );
	CloseHandle( m_GSWriteEvent);
#elif _LINUX

	//销毁读写锁
	if (pthread_rwlock_destroy( &m_GSRwmutex )!=0)
	{
		return;
	}
#endif

}

/********************************************************************************************
Function		: LockReader   
DateTime		: 2010/6/9 19:08	
Description		: 加读锁
Input			: NULL
Output			: NULL
Return			: 成功返回TRUE,失败返回FALSE
Note			:				// 备注
********************************************************************************************/
BOOL CGSWRMutex::LockReader()
{
	BOOL bRet=TRUE;
#ifdef _WIN32
	//循环加读锁操作
	while(1)
	{
		//加锁读锁变量，返回增值变量的结果
		LONG count = InterlockedIncrement( &m_readCount );
		//判断加读锁是否成功，小于0，CAN_NOT_READ_SIGN标记置位，加读锁失败
		if( count < 0 )
		{
			//读锁变量减1，取消加锁操作
			InterlockedDecrement( &m_readCount );

			//读取读锁变量的值，判断是否有线程加写锁
			count = InterlockedCompareExchange( &m_readCount, CAN_NOT_READ_SIGN,
				CAN_NOT_READ_SIGN | CAN_NOT_WRITE_SIGN );
			if( count == (CAN_NOT_READ_SIGN | CAN_NOT_WRITE_SIGN) )
			{
				//发送写锁事件信号
				SetEvent( m_GSWriteEvent );
			}
			//等待读锁事件信号
			WaitForSingleObject( m_GSReadEvent, INFINITE );
		}
		else
		{
#ifdef _MUTEX_DEBUG
			assert(bRet);
			long iR = InterlockedIncrement( &m_iRRefDebug );
			long  iW = m_iWRefDebug;
			if( iW )
			{
				assert(0);
			}	
#endif
			return bRet;
		}
	}

#elif _LINUX

	//加读锁
	if(pthread_rwlock_rdlock(&m_GSRwmutex)!=0)
	{
		bRet=FALSE;
	}
#endif
	return bRet;

}

/********************************************************************************************
Function		: TryLockReader   
DateTime		: 2010/6/9 19:10	
Description		: 非阻塞的加读锁
Input			: NULL
Output			: NULL
Return			: 成功返回TRUE,失败返回FALSE
Note			:				// 备注
********************************************************************************************/
BOOL CGSWRMutex::TryLockReader()
{
	BOOL bRet=TRUE;

#ifdef _WIN32
	//加锁读锁变量，返回增值变量的结果
	LONG count = InterlockedIncrement( &m_readCount );
	//判断加读锁是否成功，小于0，有写锁，加读锁失败
	if( count < 0 )
	{
		//读锁变量减1，取消此次加锁操作
		InterlockedDecrement( &m_readCount );
		bRet=FALSE;
	}
	else
	{
#ifdef _MUTEX_DEBUG
		if( bRet )
		{

			long iR = InterlockedIncrement( &m_iRRefDebug );
			long  iW = m_iWRefDebug;
			if( iW )
			{
				assert(0);
			}
		}
#endif
		return bRet;
	}

#elif _LINUX
	if (pthread_rwlock_tryrdlock( &m_GSRwmutex )!=0)
	{
		bRet=FALSE;
	}
#endif

	return bRet;

}

/********************************************************************************************
Function		: UnlockReader    
DateTime		: 2010/6/9 19:11	
Description		: 解读锁
Input			: NULL
Output			: NULL
Return			: NULL
Note			:				// 备注
********************************************************************************************/
void CGSWRMutex::UnlockReader()
{

#ifdef _WIN32
#ifdef _MUTEX_DEBUG
	long iR = InterlockedDecrement( &m_iRRefDebug );
	long iW = m_iWRefDebug;
	if( iW || iR<0  )
	{
		assert(0);
	}
#endif
	//读锁变量减1
	InterlockedDecrement( &m_readCount );

	//判断是否有线程要加写锁
	LONG count = InterlockedCompareExchange( &m_readCount, CAN_NOT_READ_SIGN,
		CAN_NOT_READ_SIGN | CAN_NOT_WRITE_SIGN );	
	if( count == (CAN_NOT_READ_SIGN | CAN_NOT_WRITE_SIGN) )
	{
		//发送写锁事件信号
		SetEvent( m_GSWriteEvent );
	}

#elif _LINUX
	if (pthread_rwlock_unlock( &m_GSRwmutex )!=0)
	{
		return;
	}
#endif

}


/********************************************************************************************
Function		: LockWrite   
DateTime		: 2010/6/9 19:08	
Description		: 加写锁
Input			: NULL
Output			: NULL
Return			: 成功返回TRUE,失败返回FALSE
Note			:				// 备注
********************************************************************************************/
BOOL CGSWRMutex::LockWrite()
{

	BOOL bRet=TRUE;

#ifdef _WIN32

	//加锁写锁变量加1
	LONG count = InterlockedIncrement( &m_writeCount );
	//判断写锁是否成功，返回1，没有其他线程加写锁
	if( count == 1 )
	{
		//判断是否有读锁，如果没有读锁，加写锁，有线程已加锁，则等待写锁信号
		if( !GSStopReadWaitSetWrite() )
		{
			//写锁加锁成功，返回
			#ifdef _MUTEX_DEBUG
						assert(bRet);   
						long iW = InterlockedIncrement ( &m_iWRefDebug );
						long iR = m_iRRefDebug;
						if( iR || iW>1)
						{
							assert(0);
						}
			#endif
			return bRet;
		}
	}
	//有其他线程加了锁，等待写锁事件信号
	WaitForSingleObject( m_GSWriteEvent, INFINITE );

	//写锁加锁成功，返回
#ifdef _MUTEX_DEBUG
	assert(bRet);   
	long iW = InterlockedIncrement ( &m_iWRefDebug );
	long iR = m_iRRefDebug;
	if( iR || iW>1)
	{
		assert(0);
	}
#endif

#elif _LINUX
	if(pthread_rwlock_wrlock( &m_GSRwmutex )!=0)
	{
		bRet=FALSE;
	}

#endif
	
	return bRet;

}

/********************************************************************************************
Function		: TryLockWrite   
DateTime		: 2010/6/9 19:10	
Description		: 非阻塞的加写锁
Input			: NULL
Output			: NULL
Return			: 成功返回TRUE,失败返回FALSE
Note			:				// 备注
********************************************************************************************/
BOOL CGSWRMutex::TryLockWrite()
{
	BOOL bRet=TRUE;

#ifdef _WIN32
	//加锁写锁变量	
	LONG count = InterlockedIncrement( &m_writeCount );
	//写锁加锁成功
	if( count == 1 )
	{
		//判断是否有读锁
		if( !GSStopReadWaitSetWrite() )
		{
			//写锁加锁成功，返回
#ifdef _MUTEX_DEBUG
			assert(bRet);   
			long iW = InterlockedIncrement ( &m_iWRefDebug );
			long iR = m_iRRefDebug;
			if( iR || iW>1)
			{
				assert(0);
			}
#endif
			return bRet;
		}
	}
	//加锁失败，返回
	InterlockedDecrement( &m_writeCount );
	return	FALSE;

#elif _LINUX
	if(pthread_rwlock_trywrlock(&m_GSRwmutex)!=0)
	{
		bRet=FALSE;
	}

#endif

	return bRet;

}

/********************************************************************************************
Function		: UnlockWrite    
DateTime		: 2010/6/9 19:11	
Description		: 解写锁
Input			: NULL
Output			: NULL
Return			: NULL
Note			:				// 备注
********************************************************************************************/
void CGSWRMutex::UnlockWrite()
{

#ifdef _WIN32
#ifdef _MUTEX_DEBUG
	long iW = InterlockedDecrement( &m_iWRefDebug );
	long iR = m_iRRefDebug;
	if( iR || iW )
	{
		assert(0);
	}
#endif
	LONG count = m_writeCount;
	//自身线程加了写锁
	if( count == 1 )
	{
		//清除读锁变量的CAN_NOT_READ_SIGN标识
		_InterlockedAnd( &m_readCount, ~CAN_NOT_READ_SIGN );
		//发送读锁事件信号
		SetEvent( m_GSReadEvent );
		//解锁写锁
		count = InterlockedDecrement( &m_writeCount );
		//解锁成功
		if( !count )
		{
			return;
		}
		//有其他线程要加写锁
		if( GSStopReadWaitSetWrite() )
		{
			return;
		}
	}
	else
	{
		//写锁变量减1
		InterlockedDecrement( &m_writeCount );
	}
	//发送写锁事件信号
	SetEvent( m_GSWriteEvent );

#elif _LINUX
	if (pthread_rwlock_unlock( &m_GSRwmutex )!=0)
	{
		return;
	}
#endif

}

#ifdef _WIN32

//
/********************************************************************************************
  Function		: GSStopReadWaitSetWrite   
  DateTime		: 2010/6/9 19:29	
  Description	: 停止读锁设置写锁
  Input			: NULL
  Output		: NULL
  Return		: 返回TRUE，有其他线程要加写锁，返回FALSE，无其他线程加写锁
  Note			:				// 备注
********************************************************************************************/
BOOL CGSWRMutex::GSStopReadWaitSetWrite()
{
	//重置读锁事件
	ResetEvent( m_GSReadEvent );

	//判断读锁变量，设置读锁变量的CAN_NOT_READ_SIGN标记
	LONG count = _InterlockedOr( &m_readCount, CAN_NOT_READ_SIGN );
	//返回0，说明锁未被占用，返回非0，另一个线程占用锁
	if( count )
	{
		//设置读锁的CAN_NOT_WRITE_SIGN标记
		_InterlockedOr( &m_readCount, CAN_NOT_WRITE_SIGN );

		//判断读锁变量的写锁标记
		count = InterlockedCompareExchange( &m_readCount, CAN_NOT_READ_SIGN,
			CAN_NOT_READ_SIGN | CAN_NOT_WRITE_SIGN );
		//有其他线程加锁
		if( count != (CAN_NOT_READ_SIGN | CAN_NOT_WRITE_SIGN) )
		{			
			return TRUE;
		}
	}
	//读锁未上锁
	return FALSE;
}
#endif
