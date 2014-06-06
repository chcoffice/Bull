#if _WIN32

#include "BaseSocket.h"

using namespace NetServiceLib;

CWinUdpSocket::CWinUdpSocket(void)
{

}

CWinUdpSocket:: ~CWinUdpSocket(void)
{
	CloseSocket();
}
INT	CWinUdpSocket::Visitor(ICreater* pclsVisitor)
{
	return pclsVisitor->CreaterUdpChannel(this);
}
INT	CWinUdpSocket::CreateSocket(const char* pszHost,UINT16 unPort, INT iExclusive)//创建socket 并绑定IP端口
{
	//pszhost要判断
	if ( NULL == pszHost)
	{
		return ERROR_NET_PARAM_WRONG;
	}

	m_hSocket  = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0,WSA_FLAG_OVERLAPPED);

	if (m_hSocket  == INVALID_SOCKET)
	{
		m_hSocket = NULL;
		return ERROR_NET_CREATE_SOCKET_FAIL;
	}	

	// 设置套接字为非阻塞的
	SetSockOption();

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
INT	CWinUdpSocket::BindSocket(sockaddr_in& service)
{
	if (bind( m_hSocket , (SOCKADDR*) &service, sizeof(service)) == SOCKET_ERROR) 
	{
		return -1;
	}

	return 0;
}
INT	CWinUdpSocket::CloseSocket()//关闭接口
{
	CGSAutoMutex	GAutoMutex( &m_GSMutex );

	if (m_hSocket != NULL)
	{
		shutdown(m_hSocket,SD_BOTH);
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
INT	CWinUdpSocket::SetSockOptionSec()
{
	// UDP没什么好设置的

	/*
	设置为非阻塞或阻塞方式
	*/
	u_long mode = m_bBlock;
	ioctlsocket(m_hSocket,FIONBIO,&mode);

	struct linger ling;	
	ling.l_onoff=1;		//在调用closesocket()时还有数据未发送完，允许等待
	ling.l_linger=0;	//等待时间0秒
	setsockopt(m_hSocket,SOL_SOCKET,SO_LINGER,(const   char*)&ling,sizeof(ling));  

	return 0;

}
INT	CWinUdpSocket::Connect(const char *pszDesHost, UINT16 unDesPort)//连接主机
{
	//不需要
	return ERROR_NET_UNKNOWN;

}
INT	CWinUdpSocket::Listen()//监听
{
	//不需要
	return ERROR_NET_UNKNOWN;
}
SOCKETHANDLE CWinUdpSocket::AcceptConnect(struct sockaddr* pstruAddr, INT* piAddrLen)
{
	//不需要
	return ERROR_NET_UNKNOWN;
}

INT	CWinUdpSocket::SendData(void*	pData, UINT	uiLen)
{
	//UDP不需要
	return ERROR_NET_UNKNOWN;

}
INT	CWinUdpSocket::SendDataTo(void*	pData, UINT	uiLen, char* pszDesIP, UINT16 unDesPort)
{
	if ( NULL == pData   || uiLen <= 0 || NULL == pszDesIP)
	{
		return ERROR_BASE_SUCCESS; //返回0算了 如果是其他值 用户程序说不定认为是发送了部分数据
	}

	CGSAutoMutex	GAutoMutex( &m_GSMutex );

	sockaddr_in desSockadr;	

	desSockadr.sin_family = AF_INET;				
	desSockadr.sin_addr.s_addr = inet_addr(pszDesIP);//IP
	desSockadr.sin_port = htons(unDesPort);			//端口
	
	return sendto(m_hSocket, (char*)pData, uiLen, 0, (sockaddr*)&desSockadr, sizeof(sockaddr));

}
INT	CWinUdpSocket::RecvData(void* pstruRecv )
{
	if ( NULL == pstruRecv )
	{
		return ERROR_NET_PARAM_WRONG;
	}	

	CGSAutoMutex	GAutoMutex( &m_GSMutex );

	LPPER_IO_OPERATION_DATA	pIOCP_IOData_Recv = (LPPER_IO_OPERATION_DATA)pstruRecv;
	DWORD	dwRecvBytes = 0;
	DWORD	dwFlags = 0;

	if ( m_hSocket )
	{
		
		int iRet = -1;

#if OPERATING_SYSTEM

		return recvfrom( m_hSocket, pIOCP_IOData_Recv->DataBuf.buf, pIOCP_IOData_Recv->DataBuf.len, 0,
			(sockaddr*)&(pIOCP_IOData_Recv->struSockAddrFrom), (INT32*)&(pIOCP_IOData_Recv->iAddrFromLen) );
#elif _WIN32

		iRet = WSARecvFrom( m_hSocket, &(pIOCP_IOData_Recv->DataBuf), 1, &dwRecvBytes, &dwFlags, 
			(sockaddr*)&(pIOCP_IOData_Recv->struSockAddrFrom), &(pIOCP_IOData_Recv->iAddrFromLen), &(pIOCP_IOData_Recv->Overlapped), NULL );
#endif


		if( iRet != 0 ) 
		{
			if(WSAGetLastError() == WSA_IO_PENDING  ) 
			{
				iRet = 0; 
			}
			else
			{
				return ERROR_NET_PARAM_WRONG;
			}
		}

		return ERROR_BASE_SUCCESS;
	}
	else
	{
		return ERROR_NET_PARAM_WRONG;
	}

	
	
}

INT	CWinUdpSocket::SendDataEx(void*	pData, UINT	uiLen)
{
	//UDP不需要
	return ERROR_NET_UNKNOWN;
}

#endif //end #if _WIN32


