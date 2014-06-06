#if _LINUX
#include "LinuxSocket.h"

using namespace NetServiceLib;

CLinuxUdpSocket::CLinuxUdpSocket(void)
{

}

CLinuxUdpSocket:: ~CLinuxUdpSocket(void)
{
	CloseSocket();
}

INT	CLinuxUdpSocket::Visitor(ICreater* pclsVisitor)
{
	return pclsVisitor->CreaterUdpChannel(this);
}
INT	CLinuxUdpSocket::CreateSocket(const char* pszHost,UINT16 unPort)//创建socket 并绑定IP端口
{
	//pszhost要判断
	m_hSocket  = socket( AF_INET, SOCK_DGRAM, 0 );

	if (m_hSocket  == -1)
	{
		printf(" bind fail");
		return ERROR_NET_CREATE_SOCKET_FAIL;
	}	

	SetSockOption();//设置为非阻塞，基类函数

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
INT	CLinuxUdpSocket::CloseSocket()//关闭接口
{
	if (m_hSocket != -1)
	{
		shutdown(m_hSocket,2);
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
INT	CLinuxUdpSocket::SetSockOptionSec()
{
	SetBlockModeEx( m_bBlock );

	struct linger ling;	
	ling.l_onoff=1;		//在调用closesocket()时还有数据未发送完，允许等待
	ling.l_linger=0;	//等待时间2秒
	setsockopt(m_hSocket,SOL_SOCKET,SO_LINGER,(const   char*)&ling,sizeof(ling)); 

	return 0;

}

INT	CLinuxUdpSocket::Connect(const char *pszDesHost, UINT16 unDesPort)//连接主机
{
	//UDP不需要
	return ERROR_NET_UNKNOWN;

}
INT	CLinuxUdpSocket::Listen()//监听
{
	//UDP不需要
	return ERROR_NET_UNKNOWN;
}
SOCKETHANDLE CLinuxUdpSocket::AcceptConnect(struct sockaddr* pstruAddr, INT* piAddrLen)
{
	//UDP不需要
	return ERROR_NET_UNKNOWN;
}
INT	CLinuxUdpSocket::SendData(void*	pData, UINT	uiLen)
{
	//UDP不需要
	return ERROR_NET_UNKNOWN;
}
INT	CLinuxUdpSocket::SendDataTo(void*	pData, UINT	uiLen, char* pszDesIP, UINT16 unDesPort)
{
	if ( NULL == pData   || uiLen <= 0 || NULL == pszDesIP)
	{
		return ERROR_BASE_SUCCESS; //返回0算了 如果是其他值 用户程序说不定认为是发送了部分数据
	}

	CGSAutoMutex	GAutoMutex( &m_GSMutex );

	sockaddr_in desSockadr;	

	desSockadr.sin_family = AF_INET;				
	
	struct hostent *hp = NULL;
	if( 0 == inet_aton(pszDesIP, &desSockadr.sin_addr) )
	{ //转换失败则是主机串

		#ifdef HAVE_GETHOSTBYNAME2
		hp=(struct hostent*)gethostbyname2( pszDesIP, AF_INET );
		#else
		hp=(struct hostent*)gethostbyname( pszDesIP );
		#endif

		if( hp==NULL ) 
		{	
			return ERROR_NET_PARAM_WRONG;
		}	

		memcpy( (void*)&desSockadr.sin_addr, (void*)hp->h_addr_list[0], hp->h_length );
	}   

	desSockadr.sin_port = htons(unDesPort);			//端口

	return sendto(m_hSocket, (char*)pData, uiLen, 0, (sockaddr*)&desSockadr, sizeof(sockaddr));
}
INT	CLinuxUdpSocket::RecvData(void* pstruRecv)
{
	LPPER_IO_OPERATION_DATA	pIOCP_IOData_Recv = (LPPER_IO_OPERATION_DATA)pstruRecv;
	return recvfrom( m_hSocket, pIOCP_IOData_Recv->DataBuf.buf, pIOCP_IOData_Recv->DataBuf.len, 0,
		(sockaddr*)&(pIOCP_IOData_Recv->struSockAddrFrom), (socklen_t*)&(pIOCP_IOData_Recv->iAddrFromLen) );	
}

INT	CLinuxUdpSocket::SendDataEx(void*	pData, UINT	uiLen)
{
	//UDP不需要
	return ERROR_NET_UNKNOWN;
}

#endif	//end #if _WIN32


