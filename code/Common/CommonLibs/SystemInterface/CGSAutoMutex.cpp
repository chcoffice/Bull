#include "ISystemLayInterface.h"

/******************************************************************************
功能说明：自动锁
******************************************************************************/

/********************************************************************************************
  Function		: CGSAutoMutex函数   
  DateTime		: 2010/6/10 9:32	
  Description	: 加锁
  Input			: CGSMutex *locker：普通锁的指针
  Output		: NULL
  Return		: NULL
  Note			:				// 备注
********************************************************************************************/
CGSAutoMutex::CGSAutoMutex(CGSMutex *locker): m_locker( locker )
{

	if( m_locker ) 
	{
		m_locker->Lock();
	}

}

//
/********************************************************************************************
  Function		: ~CGSAutoMutex   
  DateTime		: 2010/6/10 9:36	
  Description	: 解锁
  Input			: NULL
  Output		: NULL
  Return		: NULL
  Note			:				// 备注
********************************************************************************************/
CGSAutoMutex::~CGSAutoMutex()
{

	if( m_locker )
	{
		m_locker->Unlock();
	}

}

