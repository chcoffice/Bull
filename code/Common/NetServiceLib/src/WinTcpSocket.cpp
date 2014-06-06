#if _WIN32

#include "BaseSocket.h"

using namespace NetServiceLib;

INT		CWinTcpSocket::m_iCount=0;

CWinTcpSocket::CWinTcpSocket(void)
{
}

CWinTcpSocket::~CWinTcpSocket(void)
{
	CloseSocket();
}
INT	CWinTcpSocket::Visitor(ICreater* pclsVisitor)
{
	return pclsVisitor->CreaterTcpChannel(this);

}
INT	CWinTcpSocket::CreateSocket(const char* pszHost,UINT16 unPort, INT iExclusive)//创建socket 并绑定IP端口
{
	//pszhost要判断
	m_hSocket  = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0,WSA_FLAG_OVERLAPPED);

	if (m_hSocket  == INVALID_SOCKET)
	{
		int iErr = GetLastError();
		CTRLPRINTF(g_clsLogPtr," CWinTcpSocket::CreateSocket,Error = %d ,pszHost = %s, unPort= %u \n", iErr, pszHost, unPort);

		m_hSocket = NULL;
		return ERROR_NET_CREATE_SOCKET_FAIL;
	}	

	SetSockOption(iExclusive);//设置socket选项，基类函数

	sockaddr_in service;						//赋予套接字的地址
	service.sin_family = AF_INET;				//如果是IP地址 一直就这个AF_INET
	if ( strlen(pszHost) == 0 || pszHost[0] == '0')
	{
		service.sin_addr.s_addr = INADDR_ANY;
		
	}
	else
	{
		service.sin_addr.s_addr = inet_addr(pszHost);//htonl(INADDR_ANY);//INADDR_ANY表示任意分配给本机的IP都可以使用该套接字 有些机器有多个网卡IP
	}
	
	service.sin_port = htons(unPort);	

	if (BindSocket(service) == -1) 
	{
		closesocket(m_hSocket);
		m_hSocket = NULL;
		return ERROR_NET_BIND_SOCKET_FAIL;
	}

	return ERROR_BASE_SUCCESS;
}
INT	CWinTcpSocket::BindSocket(sockaddr_in& service)
{
	if (bind( m_hSocket , (SOCKADDR*) &service, sizeof(service)) == SOCKET_ERROR) 
	{
		INT iErr = GetLastError();
		CTRLPRINTF(g_clsLogPtr," CWinTcpSocket::BindSocket ,Error = %d \n", iErr);

		return -1;
	}

	return 0;
}
INT	CWinTcpSocket::CloseSocket()//关闭接口
{
	CGSAutoMutex	GAutoMutex( &m_GSMutex );

	if (m_hSocket != NULL)
	{
		//shutdown(m_hSocket,SD_SEND);
		closesocket(m_hSocket);//关闭套接字
		m_hSocket = NULL;

		return ERROR_BASE_SUCCESS;
	}

	return ERROR_NET_UNKNOWN;

	
}

/********************************************************************************
  Function:		SetSockOptionSec
  Description:	设置windows socket 特有的参数
  Input:  		
  Output:      	   
  Return:  		       
  Note:					
  Author:        	CHC
  Date:				2010/09/03
********************************************************************************/
INT	CWinTcpSocket::SetSockOptionSec()
{
	if ( NULL == m_hSocket)
	{
		return -1;
	}

	/*
	设置为非阻塞或阻塞方式
	*/
	SetBlockModeEx( m_bBlock );

	struct linger ling;	
	ling.l_onoff=1;		//在调用closesocket()时还有数据未发送完，允许等待
	ling.l_linger=0;	//等待时间2秒
	setsockopt(m_hSocket,SOL_SOCKET,SO_LINGER,(const   char*)&ling,sizeof(ling));   

	
	int alive=0; // 不需要保持活动包
	setsockopt(m_hSocket,SOL_SOCKET,SO_KEEPALIVE,(const char *)&alive,sizeof(int));

	
	
	return 0;
}


INT	CWinTcpSocket::Connect(const char *pszDesHost, UINT16 unDesPort)//连接主机
{
	if (m_hSocket == NULL)
	{
		return ERROR_NET_SOCKET_NOT_EXIST;
	}
	if ( pszDesHost == NULL || unDesPort < 0)
	{
		return ERROR_NET_PARAM_WRONG;
	}
	sockaddr_in service;						//赋予套接字的地址
	service.sin_family = AF_INET;				//如果是IP地址 一直就这个AF_INET

	struct hostent *hp = NULL;
	if( inet_addr( pszDesHost )==INADDR_NONE )
	{  //是主机串

		#ifdef HAVE_GETHOSTBYNAME2
		hp=(struct hostent*)gethostbyname2( pszDesHost, AF_INET );
		#else
		hp=(struct hostent*)gethostbyname( pszDesHost );
		#endif

		if( hp==NULL ) 
		{	
			return -1;
		}	

		memcpy( (void*)&service.sin_addr, (void*)hp->h_addr_list[0], hp->h_length );
	} 
	else
	{  //是IP
		service.sin_addr.s_addr = inet_addr(pszDesHost);//服务器IP
	}
	
	service.sin_port = htons(unDesPort);			//端口

	// 无论是什么方式都先设为非阻塞来连接，下面再设回来
	u_long mode = 1;
	ioctlsocket(m_hSocket,FIONBIO,&mode);


	WSAConnect( m_hSocket , (SOCKADDR*) &service, sizeof(service), NULL, NULL,NULL,NULL);
	
	int nErr = GetLastError();
	INT32 iRetErr = 0;
	if ( TestSocketEx( 1, 1, iRetErr ) > 0 )
	{
		int err_len = sizeof(int);
		int oerr = -1;
		oerr = -1;
		nErr =  ::getsockopt(m_hSocket,SOL_SOCKET,SO_ERROR, (char *) &oerr,  &err_len);
		if( nErr < 0) 
		{
			nErr = WSAGetLastError();
			CTRLPRINTF(g_clsLogPtr," CWinTcpSocket::Connect error nErr= %d, iRetErr= %d, pszDesHost = %s, unDesPort = %u \n ",
				nErr,iRetErr, pszDesHost,unDesPort);
			closesocket(m_hSocket );
			m_hSocket = NULL;
			return ERROR_NET_SOCKET_CONNECT_FAIL;
		}
		if( oerr ) 
		{
			CTRLPRINTF(g_clsLogPtr," CWinTcpSocket::Connect error nErr= %d, iRetErr= %d, pszDesHost = %s, unDesPort = %u \n ",
				nErr,iRetErr, pszDesHost,unDesPort);
			closesocket(m_hSocket );
			m_hSocket = NULL;
			return ERROR_NET_SOCKET_CONNECT_FAIL;
		}

		
		
		// 原先是什么就设回什么
		u_long mode = m_bBlock;
		ioctlsocket(m_hSocket,FIONBIO,&mode);

		return ERROR_BASE_SUCCESS;
	}
	else
	{
		CTRLPRINTF(g_clsLogPtr," CWinTcpSocket::Connect error nErr= %d, pszDesHost = %s, unDesPort = %u \n ", nErr,
			pszDesHost,unDesPort);
		closesocket(m_hSocket );
		m_hSocket = NULL;
		return ERROR_NET_SOCKET_CONNECT_FAIL;	

	}	

}
INT	CWinTcpSocket::Listen()//监听
{
	if (m_hSocket == NULL)
	{
		return ERROR_NET_SOCKET_NOT_EXIST;
	}

	// 重新设置监听端口 改为阻塞模式
	SetBlockMode(FALSE);
	
	int iRet = listen(m_hSocket, 64);
	if (iRet == SOCKET_ERROR )
	{
		INT iErr = GetLastError();
		CTRLPRINTF(g_clsLogPtr," CWinTcpSocket::Listen ,Error = %d \n", iErr);
	}

	return iRet;
}

SOCKETHANDLE	CWinTcpSocket::AcceptConnect(struct sockaddr* pstruAddr, INT* piAddrLen)
{
	SOCKETHANDLE acpt_temp;
	if ((acpt_temp = WSAAccept(m_hSocket, pstruAddr, piAddrLen, NULL, 0))==SOCKET_ERROR)
	{
		return NULL;
	}
	return acpt_temp;
}
/*************************************************
  Function:       SendData
  Description:    发送数据
  Input:  
  Output:         
  Return:         
  Note:			  发送数据时不使用完成端口，因而不使用wsasend函数。		
*************************************************/

INT	CWinTcpSocket::SendData(void*	pData, UINT	uiLen)
{
	if (pData == NULL || uiLen <= 0)
	{
		CTRLPRINTF(g_clsLogPtr," CWinTcpSocket::SendData error pData == NULL || uiLen <= 0 \n");
		return ERROR_BASE_SUCCESS; //返回0算了 如果是其他值 用户程序说不定认为是发送了部分数据
	}

	//CGSAutoMutex	GAutoMutex( &m_GSMutex );

	if ( m_hSocket <= 0 )
	{
		CTRLPRINTF(g_clsLogPtr," CWinTcpSocket::SendData error m_hSocket <= 0 \n");
		return -2;
	}
	
	int iCount = 0;
	int iErrNo = 0;
	INT iRet;
	while ( 0x1ffff != (iRet=TestSocketEx(1,1, iErrNo)  ) )
	{
		
		if (iCount++ == 5 || iRet != 0 ) //发送两次失败就返回
		{
			CTRLPRINTF(g_clsLogPtr," CWinTcpSocket::SendData error TestSocket m_hSocket = %d, ErrNo= %d \n",
				m_hSocket, iErrNo);
			return -1;
		}
		MSLEEP(1);
	}

	

	 iRet = send(m_hSocket, (char*)pData, uiLen, 0);
	if (iRet <= 0)
	{
		INT iErr = GetLastError();
		if ( m_bBlock )
		{
			if (iErr == WSAEWOULDBLOCK)
			{
				CTRLPRINTF(g_clsLogPtr," CWinTcpSocket::SendData WSAEWOULDBLOCK \n");
				return iRet;
			}
		}

		// 如果有必要的话 可以根据错误号来判断是否socket发生了中断，让网络库进行重连
		CTRLPRINTF(g_clsLogPtr," CWinTcpSocket::SendData error %d \n", iErr);
	}
	
	return iRet;
	

}
INT	CWinTcpSocket::SendDataTo(void*	pData, UINT	uiLen, char* pszDesIP, UINT16 unDesPort)
{
	//UDP的发送函数 TCP不需要实现
	return ERROR_NET_UNKNOWN;
}

/*************************************************
  Function:       RecvData
  Description:    收数据
  Input:  
  Output:         
  Return:         
  Note:			  	
*************************************************/
INT	CWinTcpSocket::RecvData( void* pstruRecv )
{
	if ( NULL == pstruRecv )
	{
		return -1;
	}

	if ( m_hSocket <= 0 )
	{
		return -1;
	}

	//CGSAutoMutex	GAutoMutex( &m_GSMutex );

	if ( m_hSocket <= 0 )
	{
		return -1;
	}

	LPPER_IO_OPERATION_DATA	pIOCP_IOData_Recv = (LPPER_IO_OPERATION_DATA)pstruRecv;
	DWORD	dwRecvBytes = 0;
	DWORD	dwFlags = 0;

	int iRet = -1;

#if OPERATING_SYSTEM

	return recv( m_hSocket, pIOCP_IOData_Recv->DataBuf.buf, pIOCP_IOData_Recv->DataBuf.len, 0);

#elif _WIN32

	iRet = WSARecv(m_hSocket,&(pIOCP_IOData_Recv->DataBuf), 1, &dwRecvBytes, &dwFlags,
		&(pIOCP_IOData_Recv->Overlapped), NULL);

#endif


	if( iRet == SOCKET_ERROR ) 
	{
		
		int iErr = WSAGetLastError();
		
		if(iErr != WSA_IO_PENDING  )	// 等于WSA_IO_PENDING时完成端口也会收到完成通知
		{			
			CTRLPRINTF(g_clsLogPtr," CWinTcpSocket::WSARecv error %d \n", iErr);
			CancelIo((HANDLE) m_hSocket);
			closesocket(m_hSocket);
			m_hSocket = NULL;
			return -1;
		}
		
	}

	return 1;
	

}

//此函数和SendData()的区别是不调用select函数来测试socket是否能写数据
INT	CWinTcpSocket::SendDataEx(void*	pData, UINT	uiLen)
{
	if (pData == NULL || uiLen <= 0)
	{
		return -1; //
	}


	INT iRet = send(m_hSocket, (char*)pData, uiLen, 0);
	if (iRet < 0)
	{
		INT iErr = GetLastError();
		if ( m_bBlock )
		{
			if (iErr == WSAEWOULDBLOCK)
			{
				return 0;
			}
		}

		// 如果有必要的话 可以根据错误号来判断是否socket发生了中断，让网络库进行重连
		printf(" CWinTcpSocket::SendData error %d \n", iErr);
	}

	return iRet;
}

#endif //end #if _WIN32



