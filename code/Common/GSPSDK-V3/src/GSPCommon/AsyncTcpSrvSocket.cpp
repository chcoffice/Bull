#if defined(_WIN32) || defined(WINCE)

// #ifdef FD_SETSIZE
// #undef FD_SETSIZE
// #endif
// 
// #define FD_SETSIZE 1024
#endif



#include "AsyncTcpSrvSocket.h"
#include "Log.h"


using namespace GSP;

namespace GSP
{
	class CMyATcpSrvSocket;
	class CMyATcpSrvEvtCtx;
#define MY_PIPE_READ 0
#define MY_PIPE_WRITE 1
	/*
	*********************************************************************
	*
	*@brief : CATcpSrvEvtCtx
	*
	*********************************************************************
	*/

	class CMyATcpSrvEvtCtx : public CAsyncTcpSrvEventContext
	{
	private :
		typedef std::set<CMyATcpSrvSocket*> CSetATcpSrvSk;
		CSetATcpSrvSk m_setSockets;

		CGSWRMutex m_csWRMutex;
		int m_fdMax;
		fd_set m_stWaitReadSet; 		
		SOCKET m_hPipe[2];
		unsigned char m_bPipeBuffer[1024];
		CGSThread m_csWorkerThread;
		BOOL m_bStarThread;
		BOOL m_bThreadRunning;
		
		
	public :
		static CMyATcpSrvEvtCtx* Create(void)
		{
			return new CMyATcpSrvEvtCtx();
		}
		
		virtual EnumErrno Init(void);
		virtual void Unint(void);
		virtual CAsyncTcpSrvSocket *Create(const char *czBindLocalIp, INT iListenPort );

		BOOL AddASocket(CMyATcpSrvSocket *pSocket);
		void RemoveASocket(CMyATcpSrvSocket *pSocket);

	protected :
		CMyATcpSrvEvtCtx(void);
		virtual ~CMyATcpSrvEvtCtx(void);
	private :
		static void ThreadEntry(CGSThread *gsThreadHandle, CMyATcpSrvEvtCtx *pObject)
		{
			pObject->WorkerEntry();
		}
		void WorkerEntry(void);	
		BOOL CreatePipe(void);
		void Wakeup(void);

		

	};

	/*
	*********************************************************************
	*
	*@brief :  
	*
	*********************************************************************
	*/

	class CMyATcpSrvSocket : public CAsyncTcpSrvSocket
	{	
	private :
		BOOL m_bBindCtx;
		CMyATcpSrvEvtCtx *m_pCtx;
	
	public :
		static CMyATcpSrvSocket *Create(CMyATcpSrvEvtCtx *pCtx)
		{
			return new CMyATcpSrvSocket(pCtx);
		}		

		virtual EnumErrno AsyncAccept( BOOL bStart );
		virtual void Disconnect(void);
		virtual CNetError Open( INT iPort, const char *czLocalIP);  


		void HandleAccept(void);
	protected :
		CMyATcpSrvSocket(CMyATcpSrvEvtCtx *pCtx);
		virtual ~CMyATcpSrvSocket(void);
	};


	CAsyncTcpSrvEventContext *CAsyncTcpSrvEventContext::Create(void)
	{
		return CMyATcpSrvEvtCtx::Create();
	}

} //end namespace GSP




/*
*********************************************************************
*
*@brief : CMyATcpSrvEvtCtx 实现
*
*********************************************************************
*/
CMyATcpSrvEvtCtx::CMyATcpSrvEvtCtx(void) : CAsyncTcpSrvEventContext()
{
	m_fdMax = -1;
	FD_ZERO(&m_stWaitReadSet);	
	m_hPipe[0] = INVALID_SOCKET;
	m_hPipe[1] = INVALID_SOCKET;
	m_bStarThread = FALSE;
	m_bThreadRunning = FALSE;
	m_setSockets.clear();
}

CMyATcpSrvEvtCtx::~CMyATcpSrvEvtCtx(void)
{
	GS_ASSERT(m_setSockets.empty());
	Unint();
}

EnumErrno CMyATcpSrvEvtCtx::Init(void)
{
	GS_ASSERT_RET_VAL(!m_bStarThread, eERRNO_SUCCESS);

	if( !CreatePipe() )
	{
		GS_ASSERT(0);
		return eERRNO_NET_ESOCKET;
	}
	
	 FD_SET(m_hPipe[MY_PIPE_READ], &m_stWaitReadSet);
	 m_fdMax = m_hPipe[MY_PIPE_READ];
	 m_bStarThread = TRUE;
	 m_bThreadRunning = TRUE;
	m_csWorkerThread.Start((GSThreadCallbackFunction)CMyATcpSrvEvtCtx::ThreadEntry, (void*) this );
	return eERRNO_SUCCESS;
}

void CMyATcpSrvEvtCtx::Unint(void)
{
	m_csWRMutex.LockWrite();
	m_bStarThread = FALSE;
	m_csWorkerThread.Stop();
	Wakeup();
	m_csWRMutex.UnlockWrite();
	m_csWorkerThread.Join(5000);		
	while( m_bThreadRunning )
	{
		MSLEEP(100);
		Wakeup();
	}

	if( m_hPipe[0] != INVALID_SOCKET )
	{
		closesocket(m_hPipe[0]);
		m_hPipe[0] = INVALID_SOCKET;
	}

	if( m_hPipe[1] != INVALID_SOCKET )
	{
		closesocket(m_hPipe[1]);
		m_hPipe[1] = INVALID_SOCKET;
	}

	
	m_setSockets.clear();
	m_fdMax = -1;


}

void CMyATcpSrvEvtCtx::WorkerEntry(void)
{
	int iRet;
	fd_set  rset;
	CSetATcpSrvSk::iterator csIt;
	int iMaxFd;
	struct timeval stTimeout;
	bzero(&stTimeout, sizeof(stTimeout));


	FD_ZERO(&rset);
	m_csWRMutex.LockReader();
	memcpy(&rset, &m_stWaitReadSet, sizeof(rset));	
	iMaxFd = m_fdMax+1;
	m_csWRMutex.UnlockReader();

	stTimeout.tv_sec = 10;
	stTimeout.tv_usec = 0;
	while(m_bStarThread && !m_csWorkerThread.TestExit() )
	{		
		
		iRet = ::select(iMaxFd, &rset,NULL, NULL, NULL );
		if( iRet<1 )
		{
			m_csWRMutex.LockReader();
			memcpy(&rset, &m_stWaitReadSet, sizeof(rset));	
			iMaxFd = m_fdMax+1;
			m_csWRMutex.UnlockReader();
			continue;
		}
		if( FD_ISSET(m_hPipe[MY_PIPE_READ], &rset ) )
		{
			iRet--;
			::recv(m_hPipe[MY_PIPE_READ],(char*)m_bPipeBuffer, 1024, 0 );
		}
		m_csWRMutex.LockReader();
		for( csIt=m_setSockets.begin();  csIt!=m_setSockets.end(); 
			  ++csIt )
		{
			
			if( FD_ISSET((*csIt)->GetOSSocket(), &rset)  )
			{
				//接受了连接
				iRet--;
				(*csIt)->HandleAccept();
			}
		}
		memcpy(&rset, &m_stWaitReadSet, sizeof(rset));	
		iMaxFd = m_fdMax+1;
		m_csWRMutex.UnlockReader();
	}
	m_bThreadRunning = FALSE;
}

CAsyncTcpSrvSocket *CMyATcpSrvEvtCtx::Create(const char *czBindLocalIp, INT iListenPort )
{
	GS_ASSERT_RET_VAL(m_bStarThread, NULL);
	CMyATcpSrvSocket *pRet = pRet->Create(this);
	if( !pRet )
	{
		GS_ASSERT(0);
		return NULL;
	}

	CNetError cError =  pRet->Open( iListenPort, czBindLocalIp );

	if( cError.m_eErrno )
	{
		GS_ASSERT(0);
		pRet->Release();
		return NULL;
	}
	return pRet;
}

void CMyATcpSrvEvtCtx::Wakeup(void)
{
	if( m_hPipe[MY_PIPE_WRITE]!=INVALID_SOCKET )
	{
		char c = 'a';
		send(m_hPipe[MY_PIPE_WRITE],&c, 1,0);
	}
	
}

BOOL CMyATcpSrvEvtCtx::AddASocket(CMyATcpSrvSocket *pSocket)
{
	CGSAutoWriterMutex wlocker(&m_csWRMutex);
	if( m_setSockets.size()>=FD_SETSIZE )
	{
		GS_ASSERT(0);
		return FALSE;
	}
	
	SOCKET fd = pSocket->GetOSSocket();
	GS_ASSERT(fd!=INVALID_SOCKET);
	m_setSockets.insert(pSocket);
	FD_SET(fd, &m_stWaitReadSet);	
	if( m_fdMax< (int) fd)
	{
		m_fdMax = fd;
	}	
	Wakeup();
	return TRUE;
}

void CMyATcpSrvEvtCtx::RemoveASocket(CMyATcpSrvSocket *pSocket)
{
	m_csWRMutex.LockWrite();
	CSetATcpSrvSk::iterator csIt;
	csIt = m_setSockets.find(pSocket);
	if( csIt==m_setSockets.end() )
	{
		m_csWRMutex.UnlockWrite();
		return;
	}
	m_setSockets.erase(csIt);
	SOCKET fd = pSocket->GetOSSocket();
	GS_ASSERT(fd!=INVALID_SOCKET);
	if( fd!=INVALID_SOCKET && m_fdMax == fd )
	{
		//重新计算最大SOCKET
		m_fdMax = m_hPipe[MY_PIPE_READ];
		for( csIt=m_setSockets.begin(); csIt!=m_setSockets.end(); ++csIt )
		{
			if( m_fdMax< (int)  (*csIt)->GetOSSocket() )
			{
				m_fdMax = (*csIt)->GetOSSocket();
			}
		}		
	}
	FD_CLR( fd, &m_stWaitReadSet);
	Wakeup();
	m_csWRMutex.UnlockWrite();
	
}



BOOL CMyATcpSrvEvtCtx::CreatePipe(void)
{
	SOCKET s1 = INVALID_SOCKET, s2 = INVALID_SOCKET; 
	struct sockaddr_in sin;
#ifdef _LINUX
	socklen_t iLen;
#else
	int iLen;
#endif
	int iRet;

	s1 = ::socket(AF_INET,  SOCK_DGRAM ,0 );
	if(s1==-1)
	{
		goto fail;
	}
	s2 = ::socket(AF_INET,SOCK_DGRAM ,0);
	if(s1==-1)
	{
		goto fail;
	}

	bzero( &sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = 0;
	sin.sin_addr.s_addr = inet_addr("127.0.0.1");

	iLen = sizeof(sin);
	iRet = ::bind( s1,(struct sockaddr *) &sin ,iLen);
	if( iRet)
	{
		goto fail;
	}
	bzero( &sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = 0;
	sin.sin_addr.s_addr = inet_addr("127.0.0.1");
	iRet = ::bind( s2,(struct sockaddr *) &sin ,iLen);
	if( iRet)
	{
		goto fail;
	}


	struct sockaddr_in sin1;
	struct sockaddr_in sin2;

	bzero( &sin1, sizeof( sin1 ) );
	iLen = sizeof(sin1);

	if( ::getsockname(s1, (struct sockaddr *) &sin1 , & iLen ) )  {
		goto fail;
	}
	bzero( &sin2, sizeof( sin2 ) );
	iLen = sizeof(sin2);

	if( ::getsockname(s2, (struct sockaddr *) &sin2 , & iLen ) )  {
		goto fail;
	}
	iLen = sizeof(sin2);
	if( ::connect( s1, (struct sockaddr *) &sin2 ,iLen) )
	{
		goto fail;
	}
	if( ::connect( s2, (struct sockaddr *) &sin1 ,iLen) )
	{
		goto fail;
	}
	m_hPipe[0] = s1;
	m_hPipe[1] = s2;
#ifdef _WIN32
	u_long val = 1;
	if( ::ioctlsocket( s1, FIONBIO, &val ) )
	{
		GS_ASSERT(0);
		goto fail;
	}
	val = 1;
	if( ::ioctlsocket( s2, FIONBIO, &val ) )
	{
		GS_ASSERT(0);
		goto fail;
	}
#else
	int opts;

	opts = ::fcntl(s1,F_GETFL);
	if(opts<0) {
		GS_ASSERT(0);
		goto fail;

	}
	opts = opts|O_NONBLOCK;
	if( ::fcntl(s2,F_SETFL,opts)<0) {
		GS_ASSERT(0);
		goto fail;
	} 



	opts = ::fcntl(s2,F_GETFL);
	if(opts<0) {
		GS_ASSERT(0);
		goto fail;

	}
	opts = opts|O_NONBLOCK;
	if( ::fcntl(s2,F_SETFL,opts)<0) {
		GS_ASSERT(0);
		goto fail;
	} 
#endif

	return TRUE;
fail:
	if(s1!=INVALID_SOCKET)
	{
		closesocket(s1);
	}
	if(s2!=INVALID_SOCKET)
	{
		closesocket(s2);
	}
	GS_ASSERT_EXIT(0, -1);
	return FALSE;

}

/*
*********************************************************************
*
*@brief : 
*
*********************************************************************
*/
CMyATcpSrvSocket::CMyATcpSrvSocket(CMyATcpSrvEvtCtx *pCtx) : 
CAsyncTcpSrvSocket()
,m_bBindCtx(FALSE)
,m_pCtx(pCtx)
{
	pCtx->RefObject();
}

CMyATcpSrvSocket::~CMyATcpSrvSocket(void)
{
	m_pCtx->UnrefObject();
}


EnumErrno CMyATcpSrvSocket::AsyncAccept( BOOL bStart )
{
	if( bStart )
	{
		if( !m_bBindCtx )
		{
			if( !m_pCtx->AddASocket(this) )
			{
				return eERRNO_NET_EBIN;
			}
		}
		m_bBindCtx = TRUE;
	}
	else if( m_bBindCtx)
	{
		m_pCtx->RemoveASocket(this);
		m_bBindCtx = FALSE;
	}
	m_bAcceptStart = bStart;
	return eERRNO_SUCCESS;
}

void CMyATcpSrvSocket::Disconnect(void)
{
	if( m_bBindCtx)
	{
		m_pCtx->RemoveASocket(this);
		m_bBindCtx = FALSE;
	}
	CISocket::Disconnect();
}

CNetError CMyATcpSrvSocket::Open( INT iPort, const char *czLocalIP)
{
	CNetError csError;

	if( !m_csTaskPool.IsInit() )
	{
		csError.m_strError = _GSTX("任务线程池初始化失败\n");
		MY_LOG_ERROR(g_pLog, _GSTX("CMyATcpSrvSocket %s:%d  启动失败. 任务线程池初始化失败\n"), 
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
		MY_LOG_ERROR(g_pLog, _GSTX("CMyATcpSrvSocket Bind %s:%d 失败 sys:%d.\n"), 
			czLocalIP ? czLocalIP : "0" , iPort, csError.m_iSysErrno );
		Disconnect();
		GS_ASSERT(0);
		csError.m_eErrno = eERRNO_NET_EBIN;
		return csError;
	}
	csError = SetListenCounts(64);	
	if( csError.m_eErrno )
	{
		MY_LOG_ERROR(g_pLog, _GSTX("CMyATcpSrvSocket SetListenCounts %s:%d 失败 sys:%d.\n"), 
			czLocalIP ? czLocalIP : "0" , iPort, csError.m_iSysErrno );
		Disconnect();
		GS_ASSERT(0); 
		csError.m_eErrno = eERRNO_NET_EOPT;
		return csError;
	}   
	csError.m_eErrno = eERRNO_SUCCESS;
	m_bConnect = TRUE;
	NonBlocking(FALSE);
	return csError;
}

void CMyATcpSrvSocket::HandleAccept(void)
{
	StruLenAndSocketAddr stRemoteAddr;
	SOCKET sk ;
	CNetError csError;  
	CGSPString strIP;
	INT iPort;
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
		if( !m_bAcceptStart )
		{
			closesocket(sk);
			return;
		}

		iPort = CISocket::AddrToString( stRemoteAddr, strIP );		
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
