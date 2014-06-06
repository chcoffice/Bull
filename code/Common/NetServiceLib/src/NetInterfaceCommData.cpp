#include "NetInterfaceCommData.h"
#include "ServerChannel.h"

using namespace NetServiceLib;

// 日志指针
ILogLibrary*			NetServiceLib::g_clsLogPtr = NULL;

#define		MAX_ACTIVE_TIME		0	//通道最大的连续未通讯时间，单位秒。默认为0表示不使用该功能
#define     MAX_CHANNEL_COUNT	65535 //最大通道数默认值。可由上层指定

CNetInterfaceCommData::CNetInterfaceCommData(void)
{
	m_bReConnect = false;
	m_bMsgBufFlag = false;
	m_unActiveTime = MAX_ACTIVE_TIME;
	m_unMaxChannel = MAX_CHANNEL_COUNT;

	m_pUserData = NULL;
	m_pfnOnEventCallBack = NULL;
	m_vectorSocketChannel.clear();

#if OPERATING_SYSTEM
	m_bBlockMode = TRUE;

#elif _WIN32

	m_bBlockMode = FALSE;
#else
	// linux 默认为非阻塞. 因为linux的接收机制和win不同。epoll和完成端口做法不同
	m_bBlockMode = TRUE;

#endif
	

	m_unSocketSendBuf = 0;
	m_unSocketRcvBuf = 0;

	m_clsLogPtr = NULL;
}

CNetInterfaceCommData::~CNetInterfaceCommData(void)
{
	m_GSMutexVerChnl.Lock();
	for ( VectorChannelPoint::size_type i=0; i< m_vectorSocketChannel.size(); i++ )
	{
		//逐个释放
		if ( NULL != m_vectorSocketChannel[i])
		{
			m_vectorSocketChannel[i]->CloseChannel();
			delete m_vectorSocketChannel[i];
			m_vectorSocketChannel[i] = NULL;
		}
	}

	m_vectorSocketChannel.clear();

	m_GSMutexVerChnl.Unlock();

	m_GSMutexVerFaultChnl.Lock();

	VectorChannelPoint::size_type i=0;
	for ( i=0; i< m_vectorFaultChannel.size(); i++)
	{
		
		//逐个释放
		if ( NULL != m_vectorFaultChannel[i])
		{
			m_vectorFaultChannel[i]->CloseChannel();
			delete m_vectorFaultChannel[i];
			m_vectorFaultChannel[i] = NULL;
		}

	}

	m_vectorFaultChannel.clear();

	m_GSMutexVerFaultChnl.Unlock();

	if ( m_clsLogPtr )
	{
		if( g_clsLogPtr == m_clsLogPtr ) // zouyx 添加
		{
			//g_clsLogPtr = NULL; // zouyx 添加
			MSLEEP(50); // zouyx 添加
		} 
		//delete m_clsLogPtr;
		m_clsLogPtr = NULL;
	}

}
void CNetInterfaceCommData::Init()
{
	// 不需要初始化
}
//保存通道
INT	CNetInterfaceCommData::SaveSocketChannel(CSocketChannel* pSocketChannel)
{
	if (pSocketChannel == NULL)
	{
		return ERROR_NET_PARAM_WRONG;
	}
	
	m_GSMutexVerChnl.Lock();

	vector<CSocketChannel*>::iterator pIter;
	pIter = find(m_vectorSocketChannel.begin(), m_vectorSocketChannel.end(), pSocketChannel);

	if (pIter == m_vectorSocketChannel.end())
	{
		m_vectorSocketChannel.push_back(pSocketChannel);

		m_GSMutexVerChnl.Unlock();

		return ERROR_BASE_SUCCESS;
	}

	

	m_GSMutexVerChnl.Unlock();

	return ERROR_NET_UNKNOWN;
}
INT	CNetInterfaceCommData::DeleteSocketChannel(CSocketChannel* pSocketChannel)
{

	if (pSocketChannel == NULL)
	{
		return ERROR_NET_PARAM_WRONG;
	}

	m_GSMutexVerChnl.Lock();

	vector<CSocketChannel*>::iterator pIter;
	pIter = find(m_vectorSocketChannel.begin(), m_vectorSocketChannel.end(), pSocketChannel);

	if (pIter != m_vectorSocketChannel.end())
	{
		m_vectorSocketChannel.erase(pIter);

		m_GSMutexVerChnl.Unlock();

		return ERROR_BASE_SUCCESS;
	}

	m_GSMutexVerChnl.Unlock();

	return ERROR_NET_CHANNEL_NOT_EXIST;

}

/********************************************************************************
  Function:       GetExistSocketChannel
  Description:    
  Input:  
  Output:         
  Return:        CSocketChannel*	存在就返回该通道指针，如果不存在就返回NULL 
  Note:				
********************************************************************************/
CSocketChannel*	 CNetInterfaceCommData::GetExistSocketChannel( DWORD dwRemoteIP, UINT16 unRemotePort )
{
	m_GSMutexVerChnl.Lock();

	for ( VectorChannelPoint::size_type i=0; i< m_vectorSocketChannel.size(); i++)
	{

		//算了 先逐个找 简单点
		DWORD   dwIP;
		UINT16	unPort = 0;
		m_vectorSocketChannel[i]->GetDWORDRemoteIPPort(dwIP, unPort);
		if ( dwIP == dwRemoteIP && unPort == unRemotePort)
		{
			
			m_GSMutexVerChnl.Unlock();
			return m_vectorSocketChannel[i];
		}

	}

	m_GSMutexVerChnl.Unlock();

	m_GSMutexVerFaultChnl.Lock();

	for ( VectorChannelPoint::size_type i=0; i< m_vectorFaultChannel.size(); i++)
	{

		//算了 先逐个找 简单点
		DWORD   dwIP;
		UINT16	unPort = 0;
		m_vectorFaultChannel[i]->GetDWORDRemoteIPPort(dwIP, unPort);
		if ( dwIP == dwRemoteIP && unPort == unRemotePort)
		{
			
			m_GSMutexVerFaultChnl.Unlock();
			return m_vectorFaultChannel[i];
		}

	}

	m_GSMutexVerFaultChnl.Unlock();

	return NULL;
}

/********************************************************************************
  Function:		IfExistSocketChannel
  Description:	根据通道指针判断通道是否存在。存在返回TRUE，不存在返回FALSE
  Input:  		pclsSocketChannel
  Output:      	   
  Return:  		TRUE 通道存在 FALSE 通道不存在       
  Note:					
  Author:        	CHC
  Date:				2010/09/25
********************************************************************************/
BOOL CNetInterfaceCommData::IfExistSocketChannel( CSocketChannel* pclsSocketChannel )
{
	if ( NULL == pclsSocketChannel )
	{
		return FALSE;
	}
	m_GSMutexVerChnl.Lock();

	VectorChannelPoint::iterator pIter = find( m_vectorSocketChannel.begin(), m_vectorSocketChannel.end(), pclsSocketChannel);

	if ( pIter != m_vectorSocketChannel.end() )
	{
		// 找到
		m_GSMutexVerChnl.Unlock();
		return TRUE;
	}

	m_GSMutexVerChnl.Unlock();

	m_GSMutexVerFaultChnl.Lock();

	pIter = find( m_vectorFaultChannel.begin(), m_vectorFaultChannel.end(), pclsSocketChannel);

	if ( pIter != m_vectorFaultChannel.end() )
	{
		// 找到
		m_GSMutexVerFaultChnl.Unlock();
		return TRUE;
	}	

	m_GSMutexVerFaultChnl.Unlock();

	return NULL;
}

INT	CNetInterfaceCommData::SetOnEventCallBack(void* pUserData, pOnEventCallBack OnEventCallBack)
{
	if ( NULL == OnEventCallBack)
	{
		return ERROR_NET_PARAM_WRONG;
	}

	m_pUserData = pUserData;
	m_pfnOnEventCallBack = OnEventCallBack;

	return ERROR_BASE_SUCCESS;
}

INT	CNetInterfaceCommData::OnEventCallBack(ISocketChannel* pSocketChnl,  enumNetEventType enumType, void* pData, UINT32 uiDataSize)
{
	//pData可以是null 比如接受连接时
	if (pSocketChnl == NULL)
	{
		return ERROR_NET_PARAM_WRONG;
	}
	
	if ( m_pfnOnEventCallBack != NULL )
	{
		
		
		return m_pfnOnEventCallBack(pSocketChnl, m_pUserData, enumType, pData, uiDataSize);
	}
	
	return ERROR_NET_CALLBACK_NOT_INIT;
	
}
/********************************************************************************
  Function:       TestActiveTime
  Description:    通道的活动时间检测
  Input:  
  Output:         
  Return:         
  Note:				监听通道不做超时检查 
					//删除通道CSocketChannel时 必须是读写锁都允许时. 
					如果超时通知上层后，上层不理睬，下一次检测又超时怎办？ 频繁地通知上层？
********************************************************************************/
INT	CNetInterfaceCommData::TestActiveTime()
{
	if ( m_vectorSocketChannel.size() == 0)
	{
		return ERROR_NET_UNKNOWN;
	}

	m_GSMutexVerChnl.Lock();

	for ( VectorChannelPoint::size_type i=0; i< m_vectorSocketChannel.size(); i++)
	{
		CSocketChannel*  pSocketChannel = (CSocketChannel*)m_vectorSocketChannel[i];
		if ( COMM_CHANNEL == pSocketChannel->GetChannelType() && pSocketChannel->GetChannelStatus() != CHANNEL_TIMEOUT )
		{
			if (DoGetTickCount() - pSocketChannel->GetLastActiveTime() > m_unActiveTime * 1000)
			{
				//通知用户超时
				OnEventCallBack(pSocketChannel, NET_TIMEOUT, NULL, 0); //如果这个回调执行很久，那么这个m_GSWRMutex会锁定很久
				pSocketChannel->SetChannelStatus( CHANNEL_TIMEOUT );//设置为超时
			}
		}
		
	}

	m_GSMutexVerChnl.Unlock();

	

	return ERROR_BASE_SUCCESS;
}
/********************************************************************************
  Function:       TestMaxChannelCount
  Description:    最大连接数检查
  Input:  
  Output:         
  Return:         true: 已经达到最大连接数     false：未达到最大连接数
  Note:				
********************************************************************************/
bool CNetInterfaceCommData::TestMaxChannelCount()
{
	m_GSMutexVerChnl.Lock();
	m_GSMutexVerFaultChnl.Lock();


	if (m_vectorSocketChannel.size() + m_vectorFaultChannel.size() >= m_unMaxChannel)
	{
		m_GSMutexVerFaultChnl.Unlock();
		m_GSMutexVerChnl.Unlock();
		return true;
	}

	m_GSMutexVerFaultChnl.Unlock();
	m_GSMutexVerChnl.Unlock();

	return false;

}


/********************************************************************************
  Function:		GetAllChannelNum
  Description:	获取所有通道数目
  Input:  		
  Output:      	   
  Return:  		       
  Note:					
********************************************************************************/
INT	CNetInterfaceCommData::GetAllChannelNum()
{
	m_GSMutexVerChnl.Lock();
	m_GSMutexVerFaultChnl.Lock();

	INT16 iNum = m_vectorSocketChannel.size() + m_vectorFaultChannel.size();

	m_GSMutexVerFaultChnl.Unlock();
	m_GSMutexVerChnl.Unlock();

	return iNum;

}

//存入故障通道队列
INT	CNetInterfaceCommData::SaveToFaultVector(CSocketChannel* pSocketChannel)
{
	if (pSocketChannel == NULL)
	{
		return ERROR_NET_PARAM_WRONG;
	}

	m_GSMutexVerFaultChnl.Lock();

	VectorChannelPoint::iterator pIter;
	pIter = find(m_vectorFaultChannel.begin(), m_vectorFaultChannel.end(), pSocketChannel);

	if (pIter == m_vectorFaultChannel.end())
	{
		m_vectorFaultChannel.push_back(pSocketChannel);
		m_GSMutexVerFaultChnl.Unlock();

		return ERROR_BASE_SUCCESS;
	}	

	m_GSMutexVerFaultChnl.Unlock();

	return ERROR_NET_UNKNOWN;

}

//从故障通道队列中删除通道，和释放通道资源不是一回事
INT	CNetInterfaceCommData::DeleteoFromFaultVector( CSocketChannel* pSocketChannel )
{
	if (pSocketChannel == NULL)
	{
		return ERROR_NET_PARAM_WRONG;
	}

	m_GSMutexVerFaultChnl.Lock();

	VectorChannelPoint::iterator pIter;
	pIter = find(m_vectorFaultChannel.begin(), m_vectorFaultChannel.end(), pSocketChannel);

	if (pIter != m_vectorFaultChannel.end())
	{
		m_vectorFaultChannel.erase(pIter);

		m_GSMutexVerFaultChnl.Unlock();

		return ERROR_BASE_SUCCESS;
	}

	m_GSMutexVerFaultChnl.Unlock();

	return ERROR_NET_CHANNEL_NOT_EXIST;
}
//设置最大通道数
INT	CNetInterfaceCommData::SetMaxChannel(UINT16 unMaxChannel)
{
	m_GSMutexVerChnl.Lock();

	if ( unMaxChannel > MAX_CHANNEL_COUNT || unMaxChannel < 2)
	{
		m_unMaxChannel = MAX_CHANNEL_COUNT;
	}
	else
	{
		m_unMaxChannel = unMaxChannel; 

	}

	m_GSMutexVerChnl.Unlock();
	
	return ERROR_BASE_SUCCESS;
}




// 设置socket发送缓冲区大小
INT	CNetInterfaceCommData::SetSendBuf(INT iBufSize)
{
	
	if ( iBufSize <=0 )
	{
		return -1;
	}
	m_GSMutexVerChnl.Lock();

	m_unSocketSendBuf = iBufSize;

	m_GSMutexVerChnl.Unlock();

	return ERROR_BASE_SUCCESS;

}
// 设置socket接收缓冲区大小
INT	CNetInterfaceCommData::SetRcvBuf(INT iBufSize)
{
	
	if ( iBufSize <=0 )
	{
		return -1;
	}

	m_GSMutexVerChnl.Lock();

	m_unSocketRcvBuf = iBufSize;

	m_GSMutexVerChnl.Unlock();

	return ERROR_BASE_SUCCESS;
}

// 设置日志路径
void CNetInterfaceCommData::SetLogPath( const char*	czPathName )
{
	if ( NULL == czPathName || strlen(czPathName) == 0)
	{
		return;
	}

	m_GSMutexVerChnl.Lock();

	if ( NULL == m_clsLogPtr )
	{
		m_clsLogPtr = GetLogInstance();
		g_clsLogPtr = m_clsLogPtr;
	}

	char szPathName[256] = { 0 };
	strcpy(szPathName,czPathName);

	INT iLen = strlen(szPathName);

	if ( szPathName[iLen-1] != '\\' && szPathName[iLen-1] != '/')
	{
		strcat( szPathName, "/" );
	}

	strcat(szPathName, "NetServiceLog/");
	m_clsLogPtr->SetLogDir(szPathName, "NetService");
	m_clsLogPtr->SetLogLevel(MFILE, LNOTSET);
	m_clsLogPtr->SetLogSize(WCREATE, 100);

	m_GSMutexVerChnl.Unlock();

}
// 设置日志库指针，对于同时初始化多个服务端、客户端的情况
void CNetInterfaceCommData::SetLogInstancePtr(ILogLibrary* pLog)
{ 
	if (pLog != NULL)
	{
		m_clsLogPtr = pLog;
	}
}

/**************************************************************************
Function    : Utility::GetApplicationPath    
DateTime    : 2010/8/26 11:13	
Description : 获取应用程序目录
Input       : 无
Output      : 无
Return      : 返回应用程序目录
Note        :	
**************************************************************************/
string NetServiceLib::GetApplicationPath(void)
{
#ifdef WINCE
	return "";

#elif _WIN32
	// 获取执行程序文件路径
	CHAR exeFullPath[MAX_PATH];
	ZeroMemory(exeFullPath, MAX_PATH);
	GetModuleFileNameA(NULL, exeFullPath, MAX_PATH);

	string AppPath = exeFullPath;
	return AppPath.substr(0, AppPath.rfind("\\")) + "\\";
#else

	CHAR exeFullPath[260];
	memset(exeFullPath, 0x0, 260);
	int n;

	n = readlink("/proc/self/exe" , exeFullPath , sizeof(exeFullPath));

	string AppPath = "";
	if( n > 0 && n < sizeof(exeFullPath))
	{
		AppPath = exeFullPath;

		return AppPath.substr(0, AppPath.rfind("/")) + "/";
	}
	return "./";	// CHC修改 2010-10-18 linux的当前路径
#endif
}





