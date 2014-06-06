#if _LINUX 

#include "LinuxSocket.h"

using namespace NetServiceLib;

CLinuxTcpSocket::CLinuxTcpSocket(void)
{
}

CLinuxTcpSocket::~CLinuxTcpSocket(void)
{
	CloseSocket();
}

INT	CLinuxTcpSocket::Visitor(ICreater* pclsVisitor)
{
	return pclsVisitor->CreaterTcpChannel(this);
}
INT	CLinuxTcpSocket::CreateSocket(const char* pszHost,UINT16 unPort, INT iExclusive)//创建socket 并绑定IP端口
{
	//pszhost要判断
	m_hSocket  = socket( AF_INET, SOCK_STREAM, 0 );//: SOCK_DGRAM

	
	if (m_hSocket  == -1)
	{
		printf(" bind fail");
		return ERROR_NET_CREATE_SOCKET_FAIL;
	}	

	SetSockOption(iExclusive);
	
	sockaddr_in service;						//赋予套接字的地址
	service.sin_family = AF_INET;				//如果是IP地址 一直就这个AF_INET

	if ( strlen(pszHost) == 0 || pszHost[0] == '0')
	{
		service.sin_addr.s_addr = INADDR_ANY;

	}
	else
	{
		inet_aton( pszHost, &service.sin_addr);
	}

	service.sin_port = htons(unPort);	

	if (bind( m_hSocket , (sockaddr*) &service, sizeof(service)) == -1) 
	{
		close(m_hSocket);
		m_hSocket = -1;
		printf(" bind fail");
		return ERROR_NET_BIND_SOCKET_FAIL;
	}

	

	return ERROR_BASE_SUCCESS;
}
INT	CLinuxTcpSocket::CloseSocket()//关闭接口
{
	CGSAutoMutex	GAutoMutex( &m_GSMutex );

	if (m_hSocket != -1)
	{
		//shutdown(m_hSocket,2);
		close(m_hSocket);//关闭套接字
		m_hSocket = -1;
		return ERROR_BASE_SUCCESS;
	}

	return ERROR_NET_UNKNOWN;


}
/********************************************************************************
  Function:		SetSockOptionSec
  Description:	设置linux socket特有的参数
  Input:  		
  Output:      	   
  Return:  		0表示成功，负值失败       
  Note:					
  Author:        	CHC
  Date:				2010/09/03
********************************************************************************/
INT	CLinuxTcpSocket::SetSockOptionSec()
{
	SetBlockModeEx( m_bBlock );

	struct linger ling;	
	ling.l_onoff=1;		//在调用closesocket()时还有数据未发送完，允许等待
	ling.l_linger=0;	//等待时间2秒
	setsockopt(m_hSocket,SOL_SOCKET,SO_LINGER,(const   char*)&ling,sizeof(ling));   


	int alive=0; // 不需要保持活动包
	setsockopt(m_hSocket,SOL_SOCKET,SO_KEEPALIVE,(const char *)&alive,sizeof(int));
	

	return 0;

}

INT	CLinuxTcpSocket::Connect(const char *pszDesHost, UINT16 unDesPort)//连接主机
{
	if (m_hSocket == -1)
	{
		return ERROR_NET_SOCKET_NOT_EXIST;
	}

	if ( pszDesHost == NULL || unDesPort < 0)
	{
		return ERROR_NET_PARAM_WRONG;
	}

	CGSAutoMutex	GAutoMutex( &m_GSMutex );

	if (m_hSocket <= 0)
	{
		return ERROR_NET_SOCKET_NOT_EXIST;
	}

	sockaddr_in service;						//赋予套接字的地址
	service.sin_family = AF_INET;
	struct hostent *hp = NULL;
	if( 0 == inet_aton(pszDesHost, &service.sin_addr) )
	{ //转换失败则是主机串

		#ifdef HAVE_GETHOSTBYNAME2
		hp=(struct hostent*)gethostbyname2( pszDesHost, AF_INET );
		#else
		hp=(struct hostent*)gethostbyname( pszDesHost );
		#endif
		
		if( hp==NULL ) 
		{	
			return ERROR_NET_PARAM_WRONG;
		}	

		memcpy( (void*)&service.sin_addr, (void*)hp->h_addr_list[0], hp->h_length );

	}   

	service.sin_port = htons(unDesPort);

	connect( m_hSocket , (sockaddr*) &service, sizeof(service));

	if ( TestSocket(1) > 0 )
	{
		return ERROR_BASE_SUCCESS;
	}
	else
	{
		printf(" CLinuxTcpSocket::Connect error %d,%s \n ", errno,strerror(errno));
		close(m_hSocket );
		m_hSocket = NULL;
		return ERROR_NET_SOCKET_CONNECT_FAIL;	

	}	
	

}
INT	CLinuxTcpSocket::Listen()//监听
{
	if (m_hSocket == -1)
	{
		return ERROR_NET_SOCKET_NOT_EXIST;
	}

	//监听一定设置为阻塞
	int val = fcntl(m_hSocket, F_GETFL);
	fcntl(m_hSocket, F_SETFL, val & O_NONBLOCK);

	return listen(m_hSocket, 64);
}
SOCKETHANDLE CLinuxTcpSocket::AcceptConnect(struct sockaddr* pstruAddr, INT* piAddrLen)
{
	SOCKETHANDLE acpt_temp = accept(m_hSocket, pstruAddr, (socklen_t*)piAddrLen);
	return acpt_temp;
}

INT	CLinuxTcpSocket::SendData(void*	pData, UINT	uiLen)
{
	if (pData == NULL || uiLen <= 0)
	{
		return -1; 
	}

	if ( m_hSocket <= 0 )
	{
		return -2;
	}

	//CGSAutoMutex	GAutoMutex( &m_GSMutex );


	if ( m_hSocket <= 0 )
	{
		return -2;
	}
	
	int iCount = 0;
	while ( !TestSocket(1) )
	{
		MSLEEP(1);
		if (iCount++ == 2)
		{
			return -3;
		}

	}
	return send(m_hSocket, (char*)pData, uiLen, 0);
}
INT	CLinuxTcpSocket::SendDataTo(void*	pData, UINT	uiLen, char* pszDesIP, UINT16 unDesPort)
{
	//在UDP文件中实现
	return ERROR_BASE_SUCCESS;
}
INT	CLinuxTcpSocket::RecvData(void* pstruRecv)
{
	if ( NULL == pstruRecv )
	{
		return ERROR_NET_PARAM_WRONG;
	}

	//CGSAutoMutex	GAutoMutex( &m_GSMutex );

	if ( m_hSocket < 0 )
	{
		return -1;
	}

	LPPER_IO_OPERATION_DATA	pIOCP_IOData_Recv = (LPPER_IO_OPERATION_DATA)pstruRecv;

	return recv( m_hSocket, pIOCP_IOData_Recv->DataBuf.buf, pIOCP_IOData_Recv->DataBuf.len, 0);
}

INT	CLinuxTcpSocket::SendDataEx(void*	pData, UINT	uiLen)
{
	if (pData == NULL || uiLen <= 0)
	{
		return -1; 
	}

	if ( NULL == m_hSocket )
	{
		return -2;
	}
	//
	return send(m_hSocket, (char*)pData, uiLen, 0);
}
#endif	//end

