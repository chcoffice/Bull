#include "SipServer.h"
#include "../Server.h"
#include "Log.h"
#include "SipSrvSession.h"
#include "ExtPackDecoder.h"

using namespace  GSP;
using namespace  GSP::SIP;
using namespace GSP::RTP;


static void _FreeAsyncTaskPoolMember( CSipSrvSession *pSession )
{
 	pSession->DeleteBefore();
 	delete pSession;
}



CSipServer::CSipServer(void)
:CIProServer(ePRO_SIP)
{
	m_hSipSrv = SIP_INVALID_HANDLE;
	m_iSipUdpPort =  SIP_SERVER_DEFAULT_UDP_LISTEN_PORT;
	m_strSipUdpBindIP = "";
	m_strServerName.clear();

	m_stConfig.iKeepaliveTimeouts = 30;
	m_stConfig.m_iRtpUdpPortBegin = RTP_DEFAULT_UDP_PORT_RANGE_BEGIN;
	m_stConfig.m_iRtpUdpPortEnd = RTP_DEFAULT_UDP_PORT_RANGE_END;


	m_csTaskDestorySession.SetFreedTaskDataFunction((FuncPtrFree)_FreeAsyncTaskPoolMember);

	m_iSSRCMin =  GSStrUtil::ToNumber<UINT32>("100000000");
	m_iSSRCSeq = m_iSSRCMin;
	m_iSSRCMax =  GSStrUtil::ToNumber<UINT32>("999999998");
}

CSipServer::~CSipServer(void)
{
	if( m_hSipSrv != SIP_INVALID_HANDLE )
	{
		SipService_Release(m_hSipSrv);
		m_hSipSrv = SIP_INVALID_HANDLE;
	}

}

UINT32 CSipServer::GetSSRC(void)
{
	UINT32 iRet = 1;
	m_mutexOfSSRC.Lock();
	while( ++iRet )
	{
		m_iSSRCSeq++;
		if( m_iSSRCSeq<m_iSSRCMin || m_iSSRCSeq>m_iSSRCMax )
		{
			continue;
		}
		if( m_setExistSSRC.find(m_iSSRCSeq) == m_setExistSSRC.end() )
		{
			iRet = m_iSSRCSeq;
			m_setExistSSRC.insert(iRet);
			m_mutexOfSSRC.Unlock();
			return iRet;
		}
	}
	m_mutexOfSSRC.Unlock();
	GS_ASSERT_EXIT(0, -1);
	return 0;
}

void CSipServer::ReleaseSSRC(UINT32 iSSRC )
{
	m_mutexOfSSRC.Lock();
	m_setExistSSRC.erase(iSSRC);
	m_mutexOfSSRC.Unlock();
}

EnumErrno CSipServer::Init( CServer *pServer )
{
	m_pServer = pServer;
	CConfigFile		&csCfg = m_pServer->GetConfig();


	ResigterExtPackDecoder(); //安装其他解码

	if( ! m_csTaskDestorySession.Init(this,
		(FuncPtrObjThreadPoolEvent)&CSipServer::OnAsyncDelSessionTaskPoolEvent,
		3, FALSE) )
	{
		MY_LOG_ERROR(g_pLog, _GSTX("初始化 SIP协议服务器失败. 初始化处理线程池失败.\n"));
		GS_ASSERT(0);
		return eERRNO_SYS_EBUSY;
	}


	m_iSipUdpPort = csCfg.GetProperty("SIP.NET","SipUdpPort", 
		SIP_SERVER_DEFAULT_UDP_LISTEN_PORT );
	m_strSipUdpBindIP = csCfg.GetProperty("SIP.NET", "SipUdpBindIP", "" );

	m_strServerName = csCfg.GetProperty("SIP.NET", "ServerName", "" );
	if( m_strServerName=="")
	{
		m_strServerName.clear();
	}

	m_bForceSendRTCP = csCfg.GetProperty("SIP.NET", "ForceSendRTCP", FALSE );

	m_stConfig.m_iRtpUdpPortBegin = csCfg.GetProperty("SIP.NET", "RtpUdpPortBegin", 
		RTP_DEFAULT_UDP_PORT_RANGE_BEGIN );
	m_stConfig.m_iRtpUdpPortEnd = csCfg.GetProperty("SIP.NET", "RtpUdpPortEnd",
		RTP_DEFAULT_UDP_PORT_RANGE_END );

	if(m_stConfig.m_iRtpUdpPortBegin!=m_stConfig.m_iRtpUdpPortEnd &&
		(m_stConfig.m_iRtpUdpPortBegin <1 || m_stConfig.m_iRtpUdpPortEnd<1) )
	{		
		GS_ASSERT(0);
		m_stConfig.m_iRtpUdpPortBegin = -1;
		m_stConfig.m_iRtpUdpPortEnd = -1;
	}

	m_iListenPort = m_iSipUdpPort;
	m_strListenIP = m_strSipUdpBindIP;

	MY_LOG_NOTICE(g_pLog, _GSTX("初始化 SIP协议服务器 完成. UDP 监听:(%s:%d), RtpUdpRange 监听(%d-%d)\n"),
		m_strSipUdpBindIP.c_str(), m_iSipUdpPort,
		m_stConfig.m_iRtpUdpPortBegin, m_stConfig.m_iRtpUdpPortEnd);


	return eERRNO_SUCCESS;
}

void CSipServer::Unint(void)
{
	if( m_hSipSrv != SIP_INVALID_HANDLE )
	{
		SipService_Release(m_hSipSrv);
		m_hSipSrv = SIP_INVALID_HANDLE;
	}
	m_csTaskDestorySession.Uninit(TRUE);
}

EnumErrno CSipServer::Start(void)
{
	GS_ASSERT_RET_VAL(m_hSipSrv==SIP_INVALID_HANDLE, eERRNO_SYS_ENEXIST);


	MY_LOG_DEBUG(g_pLog, _GSTX("开始 启动SIP协议服务器...\n"));

	

	StruSipListener stListen;
	bzero(&stListen, sizeof(stListen));
	stListen.OnClientConnectEvent = &CSipServer::SipClientConnectCallback;
	stListen.OnClientDisconnectEvent = &CSipServer::SipClientDisconnectCallback;
	stListen.OnSIPPacketEvent = &CSipServer::SipSIPPacketCallback;

	m_hSipSrv = SipService_Create(&stListen);

	if( m_hSipSrv == SIP_INVALID_HANDLE)
	{
		MY_LOG_FATAL(g_pLog, _GSTX("启动SIP协议服务器失败，创建SipService 对象失败.\n"));
		GS_ASSERT(0);
		return eERRNO_SYS_ENMEM;
	}

	SipService_SetUserData(m_hSipSrv, this);
	if( m_strServerName.length())
	{
		SipService_Set_ServerName(m_hSipSrv, m_strServerName.c_str());

	}

	EnumSipErrorCode eSipError = SipService_Start(m_hSipSrv,eSIP_CONNECT_UDP,
		m_strSipUdpBindIP.length()>1 ? m_strSipUdpBindIP.c_str(): NULL, m_iSipUdpPort);
	if( eSipError )
	{
		MY_LOG_FATAL(g_pLog, _GSTX("启动RTSP协议服务器失败，SipService 启动失败(%d).\n"),
			(int) eSipError );
		GS_ASSERT(0);
		SipService_Release(m_hSipSrv);
		m_hSipSrv = SIP_INVALID_HANDLE;
		return eERRNO_SYS_EINVALID;
	}
	m_iSipUdpPort = SipService_Get_LocalPort(m_hSipSrv);
	m_iListenPort = m_iSipUdpPort;
	MY_LOG_DEBUG(g_pLog, _GSTX("启动SIP协议服务器 成功!! UDP( %s:%d ) \n"),
		m_strSipUdpBindIP.length()>1 ? m_strSipUdpBindIP.c_str(): "0.0.0.0" , m_iSipUdpPort);
	return eERRNO_SUCCESS;



}

void CSipServer::Stop(void)
{
	if( m_hSipSrv != SIP_INVALID_HANDLE )
	{
		SipService_Stop(m_hSipSrv);		
	}
}


//SIP 协议栈回调
//连接
EnumSipErrorCode CSipServer::SipClientConnectCallback( SipServiceHandle hService,
												 SipSessionHandle hNewSession )
{
	CSipServer *pSrv = (CSipServer *)SipService_GetUserData(hService);
	return pSrv->OnSipClientConnectEvent(hNewSession);
}

//断开
void CSipServer::SipClientDisconnectCallback(SipServiceHandle hService, SipSessionHandle hSession)
{
	CSipServer *pSrv = (CSipServer *)SipService_GetUserData(hService);
	pSrv->OnSipClientDisconnectEvent(hSession);
}

//收包
void   CSipServer::SipSIPPacketCallback(SipServiceHandle hService, 
								   SipSessionHandle hSession,
								   StruSipData *pData)
{
	CSipServer *pSrv = (CSipServer *)SipService_GetUserData(hService);
	pSrv->OnSipPacketEvent(hSession, pData);
}


EnumSipErrorCode CSipServer::OnSipClientConnectEvent(SipSessionHandle hNewSession )
{
	//Sip连接

	StruMySessionInfo stInfo;
	stInfo.tvLoginTv = time(NULL);
	stInfo.tvLastKeepalive = DoGetTickCount();
	stInfo.hHandle = hNewSession;
	
	m_mutexSipClients.LockWrite();
	stInfo.hCnner = m_seqConnecter++;
	if( stInfo.hCnner == 0 )
	{
		stInfo.hCnner = m_seqConnecter++;
	}
	m_mapSipClients.insert( make_pair(stInfo.hCnner, stInfo));
	SipSession_SetUserData(hNewSession,(void*) stInfo.hCnner );	
	m_mutexSipClients.UnlockWrite();
	return eSIP_RET_SUCCESS;


}

void  CSipServer::OnSipClientDisconnectEvent(SipSessionHandle hSession)
{
	SipSessionConnecter cnner = (SipSessionConnecter)SipSession_GetUserData(hSession);
	m_mutexSipClients.LockWrite();
	CMapSipClient::iterator csIt = m_mapSipClients.find(cnner);
	if( csIt!=m_mapSipClients.end() )
	{
		for( CMapMySession::iterator cs2 = csIt->second.mapMySessions.begin();
			cs2 != csIt->second.mapMySessions.end(); ++cs2)
		{
			cs2->second->OnDisconnectEvent();
		}		
		m_mapSipClients.erase(csIt);
		m_mutexSipClients.UnlockWrite();	
		SipSession_Release(hSession);
	}
	else
	{
		m_mutexSipClients.UnlockWrite();
	}
}


void CSipServer::OnSipPacketEvent(SipSessionHandle hSession, StruSipData *pData)
{
	m_mutexSipClients.LockReader();

	SipSessionConnecter cnner = (SipSessionConnecter)SipSession_GetUserData(hSession);
	CMapSipClient::iterator csIt = m_mapSipClients.find(cnner);

	if(csIt==m_mapSipClients.end() )
	{
		GS_ASSERT(0);
		m_mutexSipClients.UnlockReader();
		if( pData->eDataType== eSIP_DATA_REQUEST )
		{
			pData->eDataType = eSIP_DATA_RESPONSE;
			pData->iContentLength = 0;
			pData->eContentType = eSIP_CONTENT_NONE;
			pData->stResponseResult.bOk = 0;
			pData->stResponseResult.iSipErrno = SIP_RESPONSE_CODE_NOTFOUND;
			
			SipSession_SendMessage(hSession,pData, NULL, 0, NULL);

		}
		MY_LOG_DEBUG(g_pLog, _GSTX("收到没有会话管理者的SIP 数据包!!\n"));
		return;
	}

	StruMySessionInfo *pMyInfo = &(csIt->second);
	
	pMyInfo->tvLastKeepalive = DoGetTickCount();

	if( pData->eMethod == eSIP_METHOD_REGISTER  )
	{
		//注册
		m_mutexSipClients.UnlockReader();
		if( pData->eDataType== eSIP_DATA_REQUEST )
		{
			pData->eDataType = eSIP_DATA_RESPONSE;
			pData->iContentLength = 0;
			pData->eContentType = eSIP_CONTENT_NONE;
			pData->stResponseResult.bOk = 1;
			pData->stResponseResult.iSipErrno = SIP_RESPONSE_CODE_SUCCESS;
			SipSession_SendMessage(hSession,pData, NULL, 0, NULL);
			
		}		
		return;
	}

	if( pData->eDataType != eSIP_DATA_REQUEST )
	{
		m_mutexSipClients.UnlockReader();
		return;
	}

	
	CGSString strCallid = pData->stDialog.czDialogKey;

	CMapMySession::iterator cs2 = pMyInfo->mapMySessions.find(strCallid);
	if( cs2 != pMyInfo->mapMySessions.end() )
	{
		//存在
		cs2->second->HandleSipData(pData);
		m_mutexSipClients.UnlockReader();
		return;
	}
	m_mutexSipClients.UnlockReader();


	if( pData->eMethod != eSIP_METHOD_INVITE || strCallid.length()<1  )
	{
		if( pData->eDataType == eSIP_DATA_REQUEST)
		{
		//	GS_ASSERT(0);
			if( pData->eDataType== eSIP_DATA_REQUEST )
			{
				pData->eDataType = eSIP_DATA_RESPONSE;
				pData->iContentLength = 0;
				pData->eContentType = eSIP_CONTENT_NONE;
				pData->stResponseResult.bOk = 0;
				pData->stResponseResult.iSipErrno = SIP_RESPONSE_CODE_NOTALLOWED;
				SipSession_SendMessage(hSession,pData, NULL, 0, NULL);

			}
			return;
		}
	}

	if(pData->eDataType != eSIP_DATA_REQUEST )
	{
		GS_ASSERT(0);
		return;		
	}

	//新的INVITE
	
		
	CSipSrvSession *pSession = new CSipSrvSession(this);
	if( pSession )
	{
		StruSipConnnectInfo stInfo;
		bzero(&stInfo, sizeof(stInfo));
		SipSession_GetConnectInfo(pMyInfo->hHandle, &stInfo);
		if( pSession->Init(pMyInfo->hCnner, strCallid, stInfo) )
		{			
			m_pServer->OnProServerEvent(this, eEVT_PROSRV_ACCEPT,(CISrvSession*)  pSession);
			pSession->Start();
			m_mutexSipClients.LockWrite();
			csIt = m_mapSipClients.find(cnner);
			if( csIt != m_mapSipClients.end() )
			{
				csIt->second.mapMySessions.insert( make_pair(strCallid, pSession));
				pSession->HandleSipData(pData);
				m_mutexSipClients.UnlockWrite();
				return;
			}
			else
			{
				m_mutexSipClients.UnlockWrite();
				pSession->DeleteBefore();
			}		
		}		
		delete pSession;
	}
	pData->eDataType = eSIP_DATA_RESPONSE;
	pData->iContentLength = 0;
	pData->eContentType = eSIP_CONTENT_NONE;
	pData->stResponseResult.bOk = 0;
	pData->stResponseResult.iSipErrno = SIP_RESPONSE_CODE_BUSY;
	SipSession_SendMessage(hSession,pData, NULL, 0, NULL);	
}



void CSipServer::SessionDisconnectInvite( CSipSrvSession *pSession)
{
	CGSAutoWriterMutex wlocker(&m_mutexSipClients);
	CMapSipClient::iterator csIt = m_mapSipClients.find(pSession->m_hSipCnner);
	if( csIt != m_mapSipClients.end() )
	{
		csIt->second.mapMySessions.erase(pSession->m_strInviteCallId);
	}
}

EnumErrno CSipServer::SessionSendSipData( CSipSrvSession *pSession, 
							 const StruSipData *pSendData, StruSipData *pRes,
							 int iTimeouts,  StruSipDialogKey *pOutDlgKey )
{
	CGSAutoReaderMutex rlocker(&m_mutexSipClients);
	CMapSipClient::iterator csIt = m_mapSipClients.find(pSession->m_hSipCnner);
	if( csIt == m_mapSipClients.end() )
	{
		//printf( "Warn...\n");
		return eERRNO_SYS_EINVALID;
		
	}
	CMapMySession::iterator cs2 = csIt->second.mapMySessions.find(pSession->m_strInviteCallId);
	if( cs2 == csIt->second.mapMySessions.end() )
	{
		//printf( "Warn...\n");
		return eERRNO_SYS_EINVALID;
	}
	if( SipSession_SendMessage(csIt->second.hHandle,pSendData, pRes, iTimeouts, pOutDlgKey ))
	{
		return eERRNO_SYS_EINVALID;
	}
	return eERRNO_SUCCESS;
}


void CSipServer::AsyncDestroySession( CSipSrvSession *pSession )
{

	
	if( m_csTaskDestorySession.Task(pSession) == CGSPThreadPool::RSUCCESS)
	{
		m_pServer->OnProServerEvent(this, eEVT_PROSRV_SESSION_RELEASE,(CISrvSession*) pSession);
	}
	// 		pSession->DeleteBefore();
	// 		m_pServer->OnProServerEvent(this, eEVT_PROSRV_SESSION_RELEASE,(CISrvSession*)  pSession);
	// 		delete pSession;


}

void CSipServer::OnAsyncDelSessionTaskPoolEvent( CObjThreadPool *pTkPool, void *pData )
{
	
	CSipSrvSession *pSession = (CSipSrvSession *)pData;	
	SessionDisconnectInvite(pSession);
	pSession->DeleteBefore();
	delete pSession;
}