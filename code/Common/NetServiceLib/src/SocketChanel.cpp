#include "SocketChanel.h"

using namespace NetServiceLib;

CSocketChannel::CSocketChannel(void)
{
	memset(m_szLocalIP,0x0,16);
	memset(m_szRemoteHost,0x0,256);
	m_unLocalPort = 0;
	m_unRemotePort = 0;
	m_dwRemoteIP = 0;
	m_uiLastActiveTime = DoGetTickCount();

	m_pBaseSocket = NULL;
	m_pListenSocketChannel = NULL;

	m_pIOData_Recv = NULL;
	m_pIOData_Recv = (LPPER_IO_OPERATION_DATA) malloc(sizeof(PER_IO_OPERATION_DATA));
	assert(m_pIOData_Recv);
	memset(m_pIOData_Recv, 0x0, sizeof(PER_IO_OPERATION_DATA) );
#if OPERATING_SYSTEM

#elif _WIN32
	//m_pIOCP_HdlData = (LPPER_HANDLE_DATA) GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA));	
	m_pIOCP_HdlData = (LPPER_HANDLE_DATA) malloc(sizeof(PER_HANDLE_DATA));	
	assert(m_pIOCP_HdlData);
	//初始化
	InitIOCPHandleData();
#else
	m_pstruEpollEvent = ( epoll_event* ) malloc( sizeof(epoll_event));
	assert(m_pstruEpollEvent);
	//初始化
	InitEpollEvent();
#endif

	//默认是正常状态
	m_enumChannelStatus = CHANNEL_NORMAL;

	m_uiRefCount = 0;	

	m_bIOCPNoUse = FALSE;

	m_uiCloseTick = 0;
	
	m_pUserData = NULL;

	m_unSocketSendBuf = 0;

	m_unSocketRcvBuf = 0;

	m_clsLogPtr = NULL;

	m_uiChnIng = 0;
}

CSocketChannel::~CSocketChannel(void)
{
	m_csCallBackGSMutex.Lock();
	m_GSMutex.Lock();

	if ( CHANNEL_CLOSE != m_enumChannelStatus )
	{
		assert(0);
	}

	if ( m_pIOData_Recv )
	{
		free(m_pIOData_Recv);
		m_pIOData_Recv = NULL;
	}
	
#if OPERATING_SYSTEM

#elif _WIN32
	if ( m_pIOCP_HdlData )
	{
		free(m_pIOCP_HdlData);
		m_pIOCP_HdlData = NULL;
	}
	
	
#else //_LINUX
	if ( m_pstruEpollEvent )
	{
		free(m_pstruEpollEvent);
		m_pstruEpollEvent  = NULL;
	}
	
#endif

	if ( SERVER == m_enumServerType && NET_PROTOCOL_UDP == m_enumNetProType && COMM_CHANNEL == m_enumChannelType )
	{
		//附加通道不需要delete
		m_pBaseSocket = NULL;
	}	
	else
	{
		if ( NULL != m_pBaseSocket )
		{
			delete m_pBaseSocket;
			m_pBaseSocket = NULL;
		}
	}

	m_GSMutex.Unlock();
	m_csCallBackGSMutex.Unlock();
	

}
INT	CSocketChannel::GetLocalIPPort(char* pszIP, UINT16& unPort)
{
	CGSAutoMutex	GSAutoMutex(&m_GSMutex);

	if (strlen(m_szLocalIP) > 0)
	{
		strcpy(pszIP, m_szLocalIP);
	}
	else
	{
		pszIP = NULL;
	}
	
	unPort = m_unLocalPort;
	
	return ERROR_BASE_SUCCESS;
}
INT	CSocketChannel::GetReMoteIPPort(char* pszIP, UINT16& unPort)
{
	CGSAutoMutex	GSAutoMutex(&m_GSMutex);

	strcpy(pszIP, m_szRemoteHost);
	unPort = m_unRemotePort;

	return ERROR_BASE_SUCCESS;
}
/*************************************************
  Function:       SendData
  Description:    
  Input:  
  Output:         
  Return:         
  Note:			  发送数据，如果发送失败，通道应该做记录，并处理是否重连，是否通知上层等	
*************************************************/
INT	CSocketChannel::SendData(void* pData, UINT unDataLen)
{
	CTRLPRINTF_D(m_clsLogPtr,"通道%p准备发送数据0, localport=%u, RemotePort=%u, unDataLen=%u \n",this, m_unLocalPort, m_unRemotePort,unDataLen);
	if (m_pBaseSocket == NULL || pData == NULL || unDataLen > 8192)		// socket默认的缓冲区大小就是8192 我觉得没必要大于默认值
	{
		CTRLPRINTF(m_clsLogPtr,"通道%p发送数据失败1, localport=%u, RemotePort=%u \n",this, m_unLocalPort, m_unRemotePort);
		return ERROR_NET_PARAM_WRONG;
	}

	if ( CHANNEL_NORMAL != m_enumChannelStatus && CHANNEL_TIMEOUT != m_enumChannelStatus )//不是正常状态的通道不允许发数据 比如中断、关闭、或等待完成端口
	{ //超时也可以发数据
		CTRLPRINTF(m_clsLogPtr,"通道%p发送数据失败2, localport=%u, RemotePort=%u \n",this, m_unLocalPort, m_unRemotePort);
		return ERROR_NET_CHANNEL_STUTAS_CLOSE;
	}

	CGSAutoMutex	AutoMutex(&m_GSMutex);
	

	if ( CHANNEL_NORMAL != m_enumChannelStatus && CHANNEL_TIMEOUT != m_enumChannelStatus )//不是正常状态的通道不允许发数据 比如中断、关闭、或等待完成端口
	{ //超时也可以发数据
		CTRLPRINTF(m_clsLogPtr,"通道%p发送数据失败3, localport=%u, RemotePort=%u \n",this, m_unLocalPort, m_unRemotePort);
		return ERROR_NET_CHANNEL_STUTAS_CLOSE;
	}
	

	if ( NULL == m_pBaseSocket )
	{
		CTRLPRINTF(m_clsLogPtr,"通道%p发送数据失败4, localport=%u, RemotePort=%u \n",this, m_unLocalPort, m_unRemotePort);
		return ERROR_NET_INVALID_CHANNEL_POINT;//无效的通道指针
	}

	if ( NULL == m_pBaseSocket->GetSocket())
	{
		//socket已经被关闭或者未建立
		CTRLPRINTF(m_clsLogPtr,"通道%p发送数据失败5, localport=%u, RemotePort=%u \n",this, m_unLocalPort, m_unRemotePort);
		return ERROR_NET_SOCKET_NOT_EXIST;
	}
	
	INT nLen = 0;

	if ( NET_PROTOCOL_TCP == m_enumNetProType)
	{
		INT nSendLen = 0;
		INT nTotalLen = unDataLen;
		while ( nLen < nTotalLen )
		{
			nSendLen = m_pBaseSocket->SendData( ((char*)pData + nLen), nTotalLen - nLen );			
			if (nSendLen <= 0)
			{	//其中只要有失败 跳出
				break;
			}
			else
			{
				//未发送完成 继续发送
				nLen += nSendLen;
				continue;
			}
		}
	}
	else
	{
		nLen = m_pBaseSocket->SendDataTo(pData, unDataLen, m_szRemoteHost, m_unRemotePort);
	}
	
	//无论发送成功或失败都认为该通道是活动的
	m_uiLastActiveTime = DoGetTickCount();
	
	//处理发送情况

	if (nLen == unDataLen)
	{
		CTRLPRINTF_D(m_clsLogPtr,"通道%p成功发送数据*, localport=%u, RemotePort=%u, unDataLen=%u \n",this, m_unLocalPort, m_unRemotePort,unDataLen);
		return ERROR_BASE_SUCCESS;
	}
	else
	{
		CTRLPRINTF(m_clsLogPtr,"通道%p发送数据失败6, localport=%u, RemotePort=%u, nLen=%d,  unDataLen=%u \n",this, m_unLocalPort, m_unRemotePort, nLen, unDataLen);
		return ERROR_NET_UNKNOWN;
	}

	

}
/********************************************************************************
  Function:       CloseChannel
  Description:    关闭通道
  Input:  
  Output:         
  Return:         
  Note:				设置退出标志，关闭socket。对应线程检测到退出标志后，释放其资源
********************************************************************************/
INT	CSocketChannel::CloseChannel()
{

	if ( m_pBaseSocket )
	{
		if ( m_pBaseSocket->GetSocket() == NULL )
		{
			return ERROR_BASE_SUCCESS;
		}
	}

	if (m_enumChannelStatus == CHANNEL_CLOSE)
	{
		return ERROR_BASE_SUCCESS;
	}

	CTRLPRINTF(m_clsLogPtr,"通道%p被请求关闭, localport=%u, RemotePort=%u \n",this, m_unLocalPort, m_unRemotePort);
	//SetChannelStatus( CHANNEL_CLOSE );

#ifdef OPERATING_SYSTEM
#elif _LINUX
	SubRefCount();
#endif

	CGSAutoMutex	CallBackGSMutex(&m_csCallBackGSMutex);

	CGSAutoMutex	GSAutoMutex(&m_GSMutex);

	m_uiCloseTick = DoGetTickCount();

	m_enumChannelStatus = CHANNEL_CLOSE;

	//服务器端的UDP通讯通道不能关闭socket. 只需要设置通道标志位退出即可，线程会释放资源。
	if ( SERVER == m_enumServerType && NET_PROTOCOL_UDP == m_enumNetProType && COMM_CHANNEL == m_enumChannelType )
	{
		return ERROR_BASE_SUCCESS;
	}	
	//关闭sokcet
    UINT16 iRet = 	CloseHandle();

	

	return iRet;

}
/********************************************************************************
Function:       CloseChannel
Description:    关闭通道
Input:  
Output:         
Return:         
Note:			和CloseChannel不同的地方是此CloseChannelEx不对该通道加回调锁
********************************************************************************/
INT	CSocketChannel::CloseChannelEx()
{

	if ( m_pBaseSocket )
	{
		if ( m_pBaseSocket->GetSocket() == NULL )
		{
			return ERROR_BASE_SUCCESS;
		}
	}

	if (m_enumChannelStatus == CHANNEL_CLOSE)
	{
		return ERROR_BASE_SUCCESS;
	}

	CTRLPRINTF(m_clsLogPtr,"通道%p被请求关闭, localport=%u, RemotePort=%u \n",this, m_unLocalPort, m_unRemotePort);
	//SetChannelStatus( CHANNEL_CLOSE );

#ifdef OPERATING_SYSTEM
#elif _LINUX
	SubRefCount();
#endif

	CGSAutoMutex	GSAutoMutex(&m_GSMutex);

	m_uiCloseTick = DoGetTickCount();

	m_enumChannelStatus = CHANNEL_CLOSE;

	//服务器端的UDP通讯通道不能关闭socket. 只需要设置通道标志位退出即可，线程会释放资源。
	if ( SERVER == m_enumServerType && NET_PROTOCOL_UDP == m_enumNetProType && COMM_CHANNEL == m_enumChannelType )
	{
		return ERROR_BASE_SUCCESS;
	}	
	//关闭sokcet
	UINT16 iRet = 	CloseHandle();



	return iRet;

}

/********************************************************************************
Function:       GetListenChannel
Description:    获取通道的监听通道。TCP或UDP均可以。
Input:  
Output:         
Return:         如果有，将返回监听通道的指针ISocketChannel*，如果没有将返回NULL。
Note:			  不是每个通道都有监听通道。比如客户端通道就没有。
********************************************************************************/
ISocketChannel*	CSocketChannel::GetListenChannel()
{
	CGSAutoMutex	GSAutoMutex(&m_GSMutex);

	if ( NULL == m_pListenSocketChannel)
	{
		return NULL;
	}

	return m_pListenSocketChannel;
}
/********************************************************************************
Function:       GetNetProType
Description:    获取网络协议类型
Input:  
Output:         
Return:         详见enumNetProtocolType
Note:				
********************************************************************************/
enumNetProtocolType	CSocketChannel::GetNetProType()
{
	return m_enumNetProType;
}
void CSocketChannel::SetLastActiveTime()
{ 
	m_GSMutex.Lock();
	m_uiLastActiveTime = DoGetTickCount();
	m_GSMutex.Unlock();
}
INT	CSocketChannel::SetLocalIPPort(const char* pszIP, UINT16 unPort)
{
	CGSAutoMutex	GSAutoMutex(&m_GSMutex);

	if (unPort < 0)
	{
		return ERROR_NET_PARAM_WRONG;
	}
	if ( pszIP != NULL )
	{
		memset(m_szLocalIP, 0x0, 16);
		strcpy(m_szLocalIP, pszIP);
	}
	
	m_unLocalPort = unPort;

	return ERROR_BASE_SUCCESS;
}
INT	CSocketChannel::SetReMoteIPPort(const char* pszIP, UINT16 unPort)
{
	CGSAutoMutex	GSAutoMutex(&m_GSMutex);

	if (pszIP == NULL || unPort <0)
	{
		return ERROR_NET_PARAM_WRONG;
	}

	if (strlen(pszIP) >= 256)
	{
		assert(0);
	}

	memset(m_szRemoteHost, 0x0, 256);
	strcpy(m_szRemoteHost, pszIP);
	m_unRemotePort = unPort;

	return ERROR_BASE_SUCCESS;

}
/*************************************************
  Function:       SetCbaseSocketPoint
  Description:    保存每个通道的CBaseSocket对象指针
  Input:  
  Output:         
  Return:         
  Note:				
*************************************************/
INT CSocketChannel::SetCbaseSocketPoint(CBaseSocket* pBaseSocket)
{
	CGSAutoMutex	GSAutoMutex(&m_GSMutex);

	if (pBaseSocket == NULL)
	{
		return ERROR_NET_PARAM_WRONG;
	}

	m_pBaseSocket = pBaseSocket;

	return ERROR_BASE_SUCCESS;
}

INT	CSocketChannel::SetListenSocketChannel(CSocketChannel*	pSocketChannel)
{
	CGSAutoMutex	GSAutoMutex(&m_GSMutex);

	if (pSocketChannel == NULL)
	{
		return ERROR_NET_PARAM_WRONG;
	}

	m_pListenSocketChannel = pSocketChannel;

	return ERROR_BASE_SUCCESS;
}

/*************************************************
  Function:       RecvData
  Description:    读数据
  Input:	
  Output:         
  Return:         
  Note:				
*************************************************/
INT	CSocketChannel::RecvData()
{	
	CGSAutoMutex	GSAutoMutex(&m_GSMutex);
	if ( m_pBaseSocket->GetSocket() <= 0 )
	{
		CTRLPRINTF(m_clsLogPtr,"通道%p已经关闭,不再接收数据 \n",this);
#if OPERATING_SYSTEM

#elif _WIN32

		m_bIOCPNoUse = FALSE; //IOCP不再使用该通道 

#endif
		return -1;
	}
#if _WIN32

	m_bIOCPNoUse = TRUE; //IOCP还要使用该通道 

#endif
	
	//只有接收线程调用此函数，因此不加锁
	memset(&(m_pIOData_Recv->Overlapped), 0x0,sizeof(OVERLAPPED));	
	//memset(&(m_pIOData_Recv->Buffer),0x0,DATA_BUFSIZE);				 //就这行  清0  比较耗时 暂时不要
	m_pIOData_Recv->DataBuf.len = DATA_BUFSIZE;
	m_pIOData_Recv->DataBuf.buf = m_pIOData_Recv->Buffer;
	m_pIOData_Recv->OptionType = RECV_POSTED;
	memset(&(m_pIOData_Recv->struSockAddrFrom), 0x0, sizeof(sockaddr_in));
	m_pIOData_Recv->iAddrFromLen = sizeof(sockaddr_in);

	int iRet = m_pBaseSocket->RecvData((void*)m_pIOData_Recv);

#ifdef OPERATING_SYSTEM

	return iRet;

#elif _LINUX

	return iRet;

#else	// 完成端口

	if (  1 != iRet  )
	{
		CTRLPRINTF(m_clsLogPtr,"通道%p,接收数据失败\n",this);

		m_bIOCPNoUse = FALSE; //IOCP不再使用该通道 
		// 接收数据失败 说明不是上层自己主动关闭，因此需要告诉上层中断
		if (m_enumChannelStatus != CHANNEL_CLOSE)
		{
			m_enumChannelStatus = CHANNEL_DISCNN;
		}
		else
		{
			CTRLPRINTF(m_clsLogPtr,"通道%p,已经是关闭状态\n",this);
		}
		
	}

	return iRet;

#endif
}

#if OPERATING_SYSTEM

#elif _LINUX
void CSocketChannel::InitEpollEvent()
{
	m_pstruEpollEvent->events = EPOLLIN | EPOLLET;
	m_pstruEpollEvent->data.ptr = this;
}

#endif

INT	CSocketChannel::CloseHandle()
{
	if (m_pBaseSocket == NULL)
	{
		return ERROR_NET_PARAM_WRONG;
	}

	return m_pBaseSocket->CloseSocket();
}

//人工重连，仅限于TCP客户端调用	
INT	CSocketChannel::ReConnect()
{
	CGSAutoMutex	GSAutoMutex(&m_GSMutex);

	if ( m_enumNetProType == NET_PROTOCOL_TCP && m_enumServerType == CLIENT && m_pBaseSocket && m_enumChannelStatus == CHANNEL_DISCNN) //应该增加一个通道是否中断的状态判断才稳妥
	{
		m_pBaseSocket->CloseSocket();
		m_pBaseSocket->CreateSocket( m_szLocalIP, m_unLocalPort );
		INT iRet = m_pBaseSocket->Connect( m_szRemoteHost, m_unRemotePort );
		if (iRet == ERROR_BASE_SUCCESS )
		{
			SetChannelStatus( CHANNEL_ADD_IOCPOREPOLL );//重连成功后 需要重新加入完成端口或epoll
		}

		return iRet;
	}
	else
	{
		return ERROR_NET_PARAM_WRONG;
	}
}

void CSocketChannel::SetChannelStatus(enumChannelStatus enumStatus)
{
	CGSAutoMutex	GSAutoMutex(&m_GSMutex);

	if ( CHANNEL_CLOSE == m_enumChannelStatus )
	{
		//处于关闭状态的通道 不需要再设置超时等其他状态，以免无法关闭
		return;
	}

	m_enumChannelStatus = enumStatus;
}
// 设置socket发送缓冲区大小
INT	CSocketChannel::SetSendBuf(INT iBufSize)
{
	if ( iBufSize <=0 )
	{
		return ERROR_NET_PARAM_WRONG;
	}

	if ( NULL == m_pBaseSocket )
	{
		return ERROR_NET_PARAM_WRONG;
	}

	CGSAutoMutex	GSAutoMutex(&m_GSMutex);

	if ( 0 != m_pBaseSocket->SetSendBuf( iBufSize ) )
	{
		return ERROR_NET_PARAM_WRONG;
	}

	m_unSocketSendBuf = iBufSize;

	return ERROR_BASE_SUCCESS;

}
// 设置socket接收缓冲区大小
INT	CSocketChannel::SetRcvBuf(INT iBufSize)
{
	if ( iBufSize <=0 )
	{
		return ERROR_NET_PARAM_WRONG;
	}
	if ( NULL == m_pBaseSocket )
	{
		return ERROR_NET_PARAM_WRONG;
	}

	CGSAutoMutex	GSAutoMutex(&m_GSMutex);

	if ( 0 != m_pBaseSocket->SetRcvBuf( iBufSize ) )
	{
		return ERROR_NET_PARAM_WRONG;
	}

	m_unSocketRcvBuf = iBufSize;

	return ERROR_BASE_SUCCESS;
}

//设置网络模式。阻塞或非阻塞
INT CSocketChannel::SetNetBlockMode( BOOL bMode)
{
	CGSAutoMutex	GSAutoMutex(&m_GSMutex);

	if ( NULL == m_pBaseSocket )
	{
		return ERROR_NET_PARAM_WRONG;
	}

	// 监听通道不允许重新设置模式。网络库一定强制为阻塞方式
	if ( LISTEN_CHANNEL == m_enumChannelType )
	{
		return ERROR_NET_PARAM_WRONG;
	}

	m_pBaseSocket->SetBlockModeEx( bMode );

	return ERROR_BASE_SUCCESS;

}

INT	CSocketChannel::SendDataEx(void* pData, UINT unDataLen)
{
	if (m_pBaseSocket == NULL || pData == NULL || unDataLen > 8192)		// socket默认的缓冲区大小就是8192 我觉得没必要大于默认值
	{
		return -1;
	}

	if ( CHANNEL_NORMAL != m_enumChannelStatus && CHANNEL_TIMEOUT != m_enumChannelStatus )//不是正常状态的通道不允许发数据 比如中断、关闭、或等待完成端口
	{ //超时也可以发数据
		return -2;
	}


	if ( NULL == m_pBaseSocket )
	{
		return -3;//无效的通道指针
	}

	if ( NULL == m_pBaseSocket->GetSocket())
	{
		//socket已经被关闭或者未建立
		return -4;
	}


	INT nLen = 0;

	m_GSMutex.Lock();

	if ( NET_PROTOCOL_TCP == m_enumNetProType)
	{
		
		nLen = m_pBaseSocket->SendDataEx( (char*)pData , unDataLen );
	}
	else
	{
		nLen = m_pBaseSocket->SendDataTo(pData, unDataLen, m_szRemoteHost, m_unRemotePort);
	}

	//m_GSWRMutex.UnlockWrite();
	m_GSMutex.Unlock();

	//无论发送成功或失败都认为该通道是活动的
	SetLastActiveTime();

	//处理发送情况

	return nLen;


}

INT	CSocketChannel::GetSocketHandle(void)
{
	CGSAutoMutex	GSAutoMutex(&m_GSMutex);
	return m_pBaseSocket->GetSocket();
}



