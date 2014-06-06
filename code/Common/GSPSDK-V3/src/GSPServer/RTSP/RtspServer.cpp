#include "RtspServer.h"
#include "Log.h"
#include "../Server.h"
#include "RtspSrvSession.h"

using namespace GSP;
using namespace GSP::RTSP;

static void _FreeAsyncTaskPoolMember( CRtspSrvSession *pSession )
{
	pSession->DeleteBefore();
	delete pSession;
}

CRtspServer::CRtspServer(void)
:CIProServer(ePRO_RTSP)
{
	m_pListenSocket = NULL;
	m_iRtspTcpPort = RTSP_SERVER_DEFAULT_TCP_LISTEN_PORT;
	m_strRtspTcpBindIP = "";

	m_stConfig.iKeepaliveTimeouts = 30;
	m_stConfig.m_iRtpUdpPortBegin = RTP_DEFAULT_UDP_PORT_RANGE_BEGIN;
	m_stConfig.m_iRtpUdpPortEnd = RTP_DEFAULT_UDP_PORT_RANGE_END;

	m_csTaskDestorySession.SetFreedTaskDataFunction((FuncPtrFree)_FreeAsyncTaskPoolMember);
}

CRtspServer::~CRtspServer(void)
{
	if( m_pListenSocket )
	{
		m_pListenSocket->Release();
		m_pListenSocket=NULL;
	}
}

EnumErrno CRtspServer::Init( CServer *pServer )
{
	m_pServer = pServer;
	 CConfigFile		&csCfg = m_pServer->GetConfig();

	 if( ! m_csTaskDestorySession.Init(this,
		 (FuncPtrObjThreadPoolEvent)&CRtspServer::OnAsyncDelSessionTaskPoolEvent,
		 3, FALSE) )
	 {
		 MY_LOG_ERROR(g_pLog, _GSTX("初始化 RTSP协议服务器失败. 初始化处理线程池失败.\n"));
		 GS_ASSERT(0);
		 return eERRNO_SYS_EBUSY;
	 }

	
	m_iRtspTcpPort = csCfg.GetProperty("RTSP.NET","RtspTcpPort", 
									RTSP_SERVER_DEFAULT_TCP_LISTEN_PORT );
	m_strRtspTcpBindIP = csCfg.GetProperty("RTSP.NET", "RtspTcpBindIP", "" );

	m_stConfig.m_iRtpUdpPortBegin = csCfg.GetProperty("RTSP.NET", "RtpUdpPortBegin", 
									RTP_DEFAULT_UDP_PORT_RANGE_BEGIN );
	m_stConfig.m_iRtpUdpPortEnd = csCfg.GetProperty("RTSP.NET", "RtpUdpPortEnd",
									RTP_DEFAULT_UDP_PORT_RANGE_END );

	if(m_stConfig.m_iRtpUdpPortBegin!=m_stConfig.m_iRtpUdpPortEnd &&
		(m_stConfig.m_iRtpUdpPortBegin <1 || m_stConfig.m_iRtpUdpPortEnd<1) )
	{		
		GS_ASSERT(0);
		m_stConfig.m_iRtpUdpPortBegin = -1;
		m_stConfig.m_iRtpUdpPortEnd = -1;
	}
	


	MY_LOG_NOTICE(g_pLog, _GSTX("初始化 RTSP协议服务器 完成. TCP 监听:(%s:%d), RtpUdpRange 监听(%d-%d)\n"),
		m_strRtspTcpBindIP.c_str(), m_iRtspTcpPort,
		m_stConfig.m_iRtpUdpPortBegin, m_stConfig.m_iRtpUdpPortEnd);


	return eERRNO_SUCCESS;
}

void CRtspServer::Unint(void)
{
	if( m_pListenSocket )
	{
		m_pListenSocket->Disconnect();
	}
	m_csTaskDestorySession.Uninit(TRUE);
}

EnumErrno CRtspServer::Start(void)
{
	GS_ASSERT_RET_VAL(m_pServer, eERRNO_SYS_ENEXIST);
	GS_ASSERT(m_pListenSocket==NULL);

	MY_LOG_DEBUG(g_pLog, _GSTX("开始 启动RTSP协议服务器...\n"));
	m_pListenSocket = CTCPServerSocket::Create();
	if( !m_pListenSocket )
	{
		MY_LOG_FATAL(g_pLog, _GSTX("启动RTSP协议服务器失败，创建Socket 对象失败.\n"));
		GS_ASSERT(0);
		return eERRNO_SYS_ENMEM;
	}
	CNetError csEerror = m_pListenSocket->Open(m_iRtspTcpPort, 
		m_strRtspTcpBindIP.length()>1 ? m_strRtspTcpBindIP.c_str(): NULL);
	if( csEerror.m_eErrno )
	{
		MY_LOG_FATAL(g_pLog, _GSTX("启动RTSP协议服务器失败，绑定端口失败:%s.\n"),
			csEerror.m_strError.c_str() );
		GS_ASSERT(0);
		m_pListenSocket->Release();
		m_pListenSocket = NULL;
		return eERRNO_SYS_ENMEM;
	}


	m_pListenSocket->SetListener(this, (FuncPtrSocketEvent)&CRtspServer::OnTcpListenSocketEvent);
	csEerror.m_eErrno = m_pListenSocket->AsyncAccept(TRUE);

	m_iListenPort = m_iRtspTcpPort;
	m_strListenIP = m_strRtspTcpBindIP;

	if( csEerror.m_eErrno )
	{
		MY_LOG_FATAL(g_pLog, _GSTX("启动RTSP协议服务器失败，启动结束线程失败 Errno:0x%x.\n"),
									csEerror.m_eErrno );
		GS_ASSERT(0);
		m_pListenSocket->Release();
		m_pListenSocket = NULL;
		return eERRNO_SYS_ENMEM;
	}
	MY_LOG_DEBUG(g_pLog, _GSTX("启动RTSP协议服务器 成功!! \n"));
	return csEerror.m_eErrno;



}

void CRtspServer::Stop(void)
{
	if( m_pListenSocket )
	{
		m_pListenSocket->AsyncAccept(FALSE);
		m_pListenSocket->Release();
		m_pListenSocket = NULL;
	}
}




void* CRtspServer::OnTcpListenSocketEvent(CISocket *pSocket,
										EnumSocketEvent iEvt,void *pParam, void *pParamExt)
{
	if( iEvt==eEVT_SOCKET_ACCEPT )
	{
		CISocket *pCliSocket = (CISocket*)pParam;
		MY_LOG_DEBUG(g_pLog, _GSTX("RTSP协议服务器 接受到连接(%s).\n"),
						pCliSocket->GetDescri().c_str());

		CRtspSrvSession *pSession = new CRtspSrvSession(this);

		if( !pSession )
		{
			MY_LOG_ERROR(g_pLog, _GSTX("RTSP协议服务器 拒绝(%s)连接. 创建会话对象失败.\n"),
				pCliSocket->GetDescri().c_str());	
			pCliSocket->Release();
		}
		if(eERRNO_SUCCESS != pSession->Init(pCliSocket) )
		{
			MY_LOG_ERROR(g_pLog, _GSTX("RTSP协议服务器 拒绝(%s)连接. 会话对象初始化失败.\n"),
				pCliSocket->GetDescri().c_str());	
			pCliSocket->Release();
			pSession->DeleteBefore();
			delete pSession;
		}
		else
		{
			m_pServer->OnProServerEvent(this, eEVT_PROSRV_ACCEPT,(CISrvSession*)  pSession);
			pSession->Start();
		}
	}
	else if( iEvt==eEVT_SOCKET_ERR )
	{
		CNetError *pError = (CNetError*)pParam;
		MY_LOG_ERROR(g_pLog, _GSTX("RTSP协议服务器 (%s) Socket 出错. %s (Sys:%d,Errno:0x%x)\n"),
				pSocket->GetDescri().c_str(),
				pError->m_strError.c_str(), pError->m_iSysErrno,
				pError->m_eErrno);
		return (void*)FALSE;
	}
	else
	{
		GS_ASSERT(0);
	}
	return (void*)TRUE;
}

void CRtspServer::AsyncDestroySession( CRtspSrvSession *pSession )
{
	
	if( m_csTaskDestorySession.Task(pSession) == CGSPThreadPool::RSUCCESS)
	{
		m_pServer->OnProServerEvent(this, eEVT_PROSRV_SESSION_RELEASE,(CISrvSession*) pSession);
	}
// 		pSession->DeleteBefore();
// 		m_pServer->OnProServerEvent(this, eEVT_PROSRV_SESSION_RELEASE,(CISrvSession*)  pSession);
// 		delete pSession;
	

}

void CRtspServer::OnAsyncDelSessionTaskPoolEvent( CObjThreadPool *pTkPool, void *pData )
{
	CRtspSrvSession *pSession = (CRtspSrvSession *)pData;	
	pSession->DeleteBefore();
	delete pSession;
}