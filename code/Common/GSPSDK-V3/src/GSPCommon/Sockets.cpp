/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : OSNET.H
Author :  邹阳星
Version : 1.0.0.1
Date: 2012/7/27 9:43
Description: 具体SOCKET 的实现 包括 CTCPServerSocket, CTCPClientSocket , CUDPSocket ...
********************************************
*/


#include "ISocket.h"
#include "Log.h"
using namespace GSP;




/*
****************************************
brief : CTCPServerSocket 实现
****************************************
*/


CTCPServerSocket::CTCPServerSocket(void)
: CISocket( (EnumSocketType)(eSOCKET_SERVER|eSOCKET_TCP) ) 
,m_csWorkerThread()

{
      m_bStarThread = FALSE;
	  m_bEnableAccept = FALSE;
	  m_csTaskPool.SetMaxWaitTask(500); //五百个连接
}

CTCPServerSocket::~CTCPServerSocket(void)
{	
	Disconnect();	
}

CNetError CTCPServerSocket::Open( INT iPort, const char *czLocalIP)
{   
CNetError csError;

	if( !m_csTaskPool.IsInit() )
	{
		csError.m_strError = _GSTX("任务线程池初始化失败\n");
			MY_LOG_ERROR(g_pLog, _GSTX("TcpServer %s:%d  启动失败. 任务线程池初始化失败\n"), 
			czLocalIP ? czLocalIP : "0" , iPort, csError.m_strError.c_str() );
		Disconnect();
		GS_ASSERT(0);
		csError.m_eErrno = eERRNO_SYS_ETKP;
	}

    csError = Socket();    
    GS_ASSERT_RET_VAL( csError.m_eErrno == eERRNO_SUCCESS ,csError);

  //  ReuseAddr(); 是否可以复用端口????

    csError = Bind(iPort, czLocalIP);
     

    if( csError.m_eErrno )
    {
        MY_LOG_ERROR(g_pLog, _GSTX("TcpServer Bind %s:%d 失败 sys:%d.\n"), 
            czLocalIP ? czLocalIP : "0" , iPort, csError.m_iSysErrno );
        Disconnect();
        GS_ASSERT(0);
        csError.m_eErrno = eERRNO_NET_EBIN;
        return csError;
    }
    csError = SetListenCounts(64);
	

    if( csError.m_eErrno )
    {
		MY_LOG_ERROR(g_pLog, _GSTX("TcpServer SetListenCounts %s:%d 失败 sys:%d.\n"), 
			czLocalIP ? czLocalIP : "0" , iPort, csError.m_iSysErrno );
        Disconnect();
        GS_ASSERT(0); 
        csError.m_eErrno = eERRNO_NET_EOPT;
        return csError;
    }   
	m_bConnect = TRUE;
    csError.m_eErrno = eERRNO_SUCCESS;
    return csError;

}



void CTCPServerSocket::Disconnect(void)
{
	m_bEnableAccept = FALSE; 
	m_bStarThread  = FALSE;	
	m_csWorkerThread.Stop();
	 CloseSocket();
	CISocket::Disconnect();
	m_csWorkerThread.Join(5000);		
}

EnumErrno CTCPServerSocket::AsyncAccept( BOOL bStart )
{
 EnumErrno eRet = eERRNO_SUCCESS;   
	m_bAcceptStart = bStart;
    if( bStart   )
    {   
        if( m_bEnableAccept  )
        {
            GS_ASSERT(0);
            return eERRNO_SUCCESS;
        }

        if( m_bConnect)
        {             
            if( !m_bStarThread)
            {
                m_bStarThread = TRUE;
                m_csWorkerThread.Start((GSThreadCallbackFunction)CTCPServerSocket::ThreadEntry, (void*) this );
            }
        }
        else
        {
            eRet = eERRNO_NET_ECLOSED;
            GS_ASSERT(0);
        }
    }
    if( eRet == eERRNO_SUCCESS )
    {
		
        m_bEnableAccept = bStart;        
    }
	else
	{
		m_bAcceptStart = FALSE;
	}
    return eRet;
}


void CTCPServerSocket::ThreadEntry(CGSThread *gsThreadHandle, CTCPServerSocket *pObject)
{                                                             
   pObject->WorkerEntry();  
   pObject->m_bStarThread = FALSE;
   MY_DEBUG(_GSTX("TCPServer %s:%d Worker Thread Exit.\n"), pObject->LocalIP(),pObject->LocalPort() );
}

void CTCPServerSocket::WorkerEntry(void)
{
   
    StruLenAndSocketAddr stRemoteAddr;
    SOCKET sk ;
    CNetError csError;  
	CGSPString strIP;
	INT iPort;
     NonBlocking(FALSE);
    while(g_bModuleRunning && m_bStarThread && m_bConnect && m_bAcceptStart )
    {      
        csError = Accept(&sk, stRemoteAddr);    
        if( csError.m_eErrno  )
        {
			MY_LOG_ERROR(g_pLog, _GSTX("TCP 服务器%s:%d Accept 失败.err:%d(%d). %s\n"),
				 LocalIP(), LocalPort(),
				csError.m_eErrno, csError.m_iSysErrno,
				csError.m_strError.empty() ? "" :  csError.m_strError.c_str()  );    
        }
        else
        {
          
            iPort = CISocket::AddrToString( stRemoteAddr, strIP );

			MY_LOG_DEBUG(g_pLog,  _GSTX("TCP 服务器: %s:%d  收到连接:%s:%d\n"),
               LocalIP(), LocalPort(),  strIP.c_str(), iPort ); 
            CTCPCnnSocket *pSocket = CTCPCnnSocket::Create(this, sk, stRemoteAddr );
            if( pSocket )
            {
               if( !OnAIOEventAccept(pSocket) )
			   {
				   MY_LOG_ERROR(g_pLog, _GSTX("TCP 服务器: %s:%d. 拒绝连接:%s:%d.任务操作失败.\n"),
					     LocalIP(), LocalPort(), strIP.c_str(), iPort ); 
				   pSocket->Release();
			   }
            }
            else
            {
                closesocket(sk);                
				MY_LOG_FATAL(g_pLog, _GSTX("TCP 服务器: %s:%d. 拒绝连接:%s:%d. 内存分配失败.\n"),
					LocalIP(), LocalPort(), strIP.c_str(), iPort ); 

            }
        }
    }
}


/*
*********************************************************************
*
*@brief : CTCPClientSocket 实现
*
*********************************************************************
*/
/*
****************************************
brief :   CTCPCnnSocket 实现
****************************************
*/

CTCPCnnSocket::CTCPCnnSocket(CISocket *pServer, SOCKET hSocket)
:CISocket( (EnumSocketType)(eSOCKET_TCP|eSOCKET_CONNECT) )

{
	if( pServer )
	{
		GSStrUtil::Format(m_strServerDescri, "%s:%d", pServer->LocalIP(), pServer->LocalPort());
	}
	else
	{
		m_strServerDescri = "null";
	}
	m_hSocket = hSocket;	
	m_bConnect = TRUE;
}

CTCPCnnSocket::~CTCPCnnSocket(void)
{

}


CTCPCnnSocket *CTCPCnnSocket::Create( CISocket *pServer, SOCKET hSocket, 
									 const StruLenAndSocketAddr &stRmote )
{
	CTCPCnnSocket *pRet = new CTCPCnnSocket(pServer, hSocket);
	if( pRet )
	{
		pRet->m_iRemotePort = CISocket::AddrToString(stRmote, pRet->m_strRemoteIP );      
		pRet->RefreshLocalSockName(); 
		pRet->SetTCPConnectOptions();
		CNetError csError;
		if( !pRet->BindAsyncIO( csError ) )
		{
			pRet->Disconnect();
			MY_LOG_ERROR(g_pLog,  _GSTX("CTCPCnnSocket TCP连接(%s) sk(%d)  绑定异步消息失败. %s.\n"),
				pRet->GetDescri().c_str(),(INT)hSocket,
				csError.Error().c_str() );
			GS_ASSERT(0);
			delete pRet;            
			return NULL;
		}
	}
	return pRet;
}

/*
****************************************
brief : CTCPClientSocket 的实现
****************************************
*/

CTCPClientSocket::CTCPClientSocket( void )
:CISocket((EnumSocketType)(eSOCKET_TCP|eSOCKET_CLIENT))
{
	m_strSrvHostName = "0";
}

CTCPClientSocket::~CTCPClientSocket(void)
{

}


EnumErrno CTCPClientSocket::Connect( UINT16 iSrvPort, const char *czSrvHostName,  UINT16 iLocalPort, const char *czLocalIP)
{
	StruLenAndSocketAddr stAddr; 
	int timeout = 10;
	CNetError csError;
	if( !Host2Addr(czSrvHostName, iSrvPort, stAddr) )
	{
		MY_LOG_ERROR(g_pLog,  _GSTX("Host %s:%d to addr fail.\n"),
			czSrvHostName, iSrvPort);
		return eERRNO_SYS_EINVALID;
	}
	m_strSrvHostName = czSrvHostName;
	csError = Socket(  );
	if( csError.m_eErrno  )
	{
		MY_LOG_ERROR(g_pLog,  _GSTX("建立 %s Socket 失败. %s.\n"),
			GetTypeName(m_eSockeType), csError.Error().c_str() );
		return eERRNO_NET_ESOCKET;
	}
	csError = OSConnect(iSrvPort, czSrvHostName, iLocalPort, czLocalIP );
	if(eERRNO_SUCCESS== csError.m_eErrno )
	{
		SetTCPConnectOptions();
		if( !BindAsyncIO( csError ) )
		{
			
			MY_LOG_ERROR(g_pLog,_GSTX("TCP 客户端 sk(%d) 连接 %s:%d 绑定网络消息失败. %s.\n"),
				(int)m_hSocket,m_strSrvHostName.c_str(), iSrvPort, 
				csError.Error().c_str()   );
			Disconnect();
			return eERRNO_NET_EEVT;
		}
	}
	else
	{
		
		MY_LOG_ERROR(g_pLog,_GSTX("TCP 客户端 sk(%d) 连接 %s:%d 失败. %s.\n"),
			(int)m_hSocket,m_strSrvHostName.c_str(), iSrvPort, 
			csError.Error().c_str()   );
		Disconnect();
	}
	return csError.m_eErrno;;
}


/*
****************************************
brief :  CUDPSocket 实现 
****************************************
*/

CUDPSocket::CUDPSocket( void )
:CISocket((EnumSocketType)(eSOCKET_UDP|eSOCKET_CLIENT))
{
	m_bMulticast = FALSE;
}

CUDPSocket::~CUDPSocket(void)
{

}

void CUDPSocket::ResetBuffer( UINT iSendBufSize, UINT iRcvBufSize)
{
	if( iSendBufSize )
	{
		SetSendBuffer(iSendBufSize); 
	}

	if( iRcvBufSize )
	{
		SetRcvBuffer(iRcvBufSize); 
	}
}

CUDPSocket *CUDPSocket::Create(BOOL bSend, BOOL bRcv)
{
	CUDPSocket * pRet = new CUDPSocket();
	GS_ASSERT_RET_VAL(pRet, NULL);
	if( pRet->OpenSocket(bSend,bRcv) )
	{
		pRet->Release();
		return NULL;
	}
	return pRet;
}

EnumErrno CUDPSocket::OpenSocket(BOOL bSend, BOOL bRcv )
{
	GS_ASSERT(m_hSocket == INVALID_SOCKET	);
	CNetError csError;
	csError = Socket( );
	if( csError.m_eErrno )
	{
		MY_LOG_ERROR(g_pLog,  _GSTX("Create %s Socket fail. %s.\n"),
			GetTypeName(m_eSockeType), csError.Error().c_str());
		return eERRNO_NET_ESOCKET;
	}

	if( bRcv  )
	{
		SetRcvBuffer(KBYTES*128); 
	}

	if( bSend )
	{
		SetSendBuffer(KBYTES*128); 
	}

	if(bRcv && !BindAsyncIO( csError ) )
	{


		MY_LOG_ERROR(g_pLog,_GSTX("UDP 客户端 sk(%d) 绑定网络消息失败. %s.\n"),
			(int)m_hSocket, csError.Error().c_str()   );
		Disconnect();
		return eERRNO_NET_EEVT;
	}
	return eERRNO_SUCCESS;
}


EnumErrno CUDPSocket::InitSocket(  UINT16 iLocalPort, const char *czLocalIP,
								 BOOL bMulticast  )
{
	GS_ASSERT(m_hSocket!=INVALID_SOCKET && !bMulticast);
	CNetError csError;
	m_bMulticast = bMulticast;
	
	csError = Bind(iLocalPort, czLocalIP );
	if( csError.m_eErrno )
	{
		MY_LOG_WARN(g_pLog,  _GSTX("CUDPSocket Sk[%d, %s] Socket 绑定端口[%d] 失败. %s.\n"),
			(int)m_hSocket, GetTypeName(m_eSockeType),iLocalPort, csError.Error().c_str()  );		
		return eERRNO_NET_EBIN; 
	}	
	return eERRNO_SUCCESS;
}



EnumErrno CUDPSocket::InitSocket(  UINT16 iMinLocalPort,UINT16 iMaxLocalPort,
									   const char *czLocalIP ,BOOL bMulticast   )
{
	GS_ASSERT(m_hSocket!=INVALID_SOCKET && !bMulticast);

	m_bMulticast = bMulticast;
	CNetError csError;
	for( UINT16 i = iMinLocalPort; i<iMaxLocalPort; i++ )
	{
		csError = Bind(i, czLocalIP );
		if( csError.m_eErrno==eERRNO_SUCCESS )
		{
			break;
		}
	}

	if( csError.m_eErrno!=eERRNO_SUCCESS )
	{
		MY_LOG_WARN(g_pLog,  _GSTX("SysSk[%d, %s] Socket 绑定端口(%d-%d) fail. %s.\n"),
			(int)m_hSocket, GetTypeName(m_eSockeType),
			iMinLocalPort,iMaxLocalPort,
			csError.Error() );		
		return eERRNO_NET_EBIN; 
	}	
	return eERRNO_SUCCESS;
}



EnumErrno CUDPSocket::AsyncSend( void *pKey,  std::vector<CProPacket *> vProPacket  )
{
	
	return eERRNO_SYS_ENEXIST;
}

EnumErrno CUDPSocket::AsyncSend( void *pKey,  std::vector<CProFrame *> vProFrame  )
{
	return eERRNO_SYS_ENEXIST;
}

EnumErrno CUDPSocket::SetRemoteAddress(  const CGSPString &strRemoteHost, int iPort)
{
	GS_ASSERT_RET_VAL(!m_bMulticast, eERRNO_SYS_EINVALID);	
	if( !COSSocket::Host2Addr(strRemoteHost.c_str(),iPort, m_stRemoteAddr.stAddr) )
	{		
		GS_ASSERT(0);
		m_stRemoteAddr.stAddr.len = 0;
		return eERRNO_SYS_EINVALID;
	}
	m_stRemoteAddr.strHostName = strRemoteHost;
	m_stRemoteAddr.iPort = iPort;
	return eERRNO_SUCCESS;
}


void CUDPSocket::ClearRemoteAddress( void )
{
	m_stRemoteAddr.stAddr.len = 0;
}

EnumErrno CUDPSocket::SendTo(const CProPacket *pPacket)
{
	if( m_hSocket==INVALID_SOCKET )
	{
		return eERRNO_NET_ECLOSED;
	}
	if( m_stRemoteAddr.stAddr.len > 0 )
	{
		CISocket::SendTo(pPacket, &m_stRemoteAddr.stAddr);
		return eERRNO_SUCCESS;
	}
	return eERRNO_SYS_ENEXIST;	
}

EnumErrno CUDPSocket::SendTo(const CProFrame *pFrame)
{
	if( m_hSocket==INVALID_SOCKET )
	{
		return eERRNO_NET_ECLOSED;
	}
	if( m_stRemoteAddr.stAddr.len > 0 )
	{
		CISocket::SendTo(pFrame,  &m_stRemoteAddr.stAddr);
		return eERRNO_SUCCESS;
	}
	return eERRNO_SYS_ENEXIST;	
}

EnumErrno CUDPSocket::SendTo( const struct iovec *vIO, INT iVecs)
{
	if( m_hSocket==INVALID_SOCKET )
	{
		return eERRNO_NET_ECLOSED;
	}
	if( m_stRemoteAddr.stAddr.len > 0 )
	{
		COSSocket::OSSendTo(vIO,iVecs, &m_stRemoteAddr.stAddr);
		return eERRNO_SUCCESS;
	}
	return eERRNO_SYS_ENEXIST;	
}

EnumErrno CUDPSocket::JoinMulticast(const CGSPString &strRemoteHost )
{
	//TODO ...
	return eERRNO_SYS_EFUNC;
}

EnumErrno CUDPSocket::LeaveMulticast(const CGSPString &strRemoteHost )
{
	//TODO ...
	return eERRNO_SYS_EFUNC;
}

EnumErrno    CUDPSocket::SetTtl(UINT16 timeToLive)
{
	//TODO ...
	return eERRNO_SYS_EFUNC;
}

EnumErrno    CUDPSocket::SetMulticastInterface(const CGSPString &strLocalIp )
{
	//TODO ...
	return eERRNO_SYS_EFUNC;
}
