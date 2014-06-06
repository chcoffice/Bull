#include "ProSipChannel.h"
#include "../ClientChannel.h"
#include "../ClientSection.h"
#include "Log.h"
#include "GSPMemory.h"
#include "ExtPackDecoder.h"


using namespace GSP;
using namespace GSP::SIP;
using namespace GSP::RTP;
#define H_LOG  m_pParent->m_pLog

CProSipClientService::CProSipClientService(CClientSection *pParent ) : CGSPObject(), m_pParent(pParent)
{
	m_strSipUdpBindIp.clear();
	m_iSipUdpPort = 0;
	m_strSipServerName.clear();

	m_iRtpUdpPortBegin = 0;
	m_iRtpUdpPortEnd = 0;

	m_hSipSrv = SIP_INVALID_HANDLE;

	m_bReady = FALSE;

	m_seqConnecter =  1;

	m_bForceSendRTCP = FALSE;

	m_csKeepaliveTestTimer.Init(this, (FuncPtrTimerCallback)&CProSipClientService::OnTimerEvent, 1,1000L*10,FALSE, NULL);
}

CProSipClientService::~CProSipClientService(void)
{
	m_bReady = FALSE;
	GS_ASSERT(m_hSipSrv==SIP_INVALID_HANDLE);
}

void CProSipClientService::OnTimerEvent( CWatchTimer *pTimer )
{
	//去掉无用连接
	if( !m_bReady )
	{
		pTimer->Stop();
	}
	CMapSipClient::iterator csIt;
	m_mutexSipClients.LockWrite();
	for( csIt = m_mapSipClients.begin(); csIt!=m_mapSipClients.end();  )
	{
		StruMyCnnerInfo *pCnner = &(csIt->second);
		if( pCnner->mapMyChannels.empty() && pCnner->m_iBusy==0 && !pCnner->m_iRegistering &&
			pCnner->tvLastKeepalive++>30  )
		{
			//移除
			csIt = m_mapSipClients.erase(csIt);
		}
		else
		{
			++csIt;
		}
	}
	m_mutexSipClients.UnlockWrite();
}


EnumErrno CProSipClientService::Init( CConfigFile &cfCfg)
{
	if( !m_csKeepaliveTestTimer.IsReady() )
	{
		GS_ASSERT(0);
		return eERRNO_SYS_EINVALID;
	}
	//SIP 端口配置
	m_iSipUdpPort = cfCfg.GetProperty("SIP.NET","SipUdpPort", 
		m_iSipUdpPort );
	m_strSipUdpBindIp = cfCfg.GetProperty("SIP.NET", "SipUdpBindIP", "" );

	m_iRtpUdpPortBegin = cfCfg.GetProperty("SIP.NET", "RtpUdpPortBegin", 
		m_iRtpUdpPortBegin );
	m_iRtpUdpPortEnd = cfCfg.GetProperty("SIP.NET", "RtpUdpPortEnd",
		m_iRtpUdpPortBegin );

	m_strSipServerName = cfCfg.GetProperty("SIP.NET", "ServerName", "" );
	if( m_strSipServerName=="")
	{
		m_strSipServerName.clear();
	}

	m_bForceSendRTCP = cfCfg.GetProperty("SIP.NET", "ForceSendRTCP", FALSE );

	ResigterExtPackDecoder(); //安装其他解码

	return eERRNO_SUCCESS;	
}

EnumErrno  CProSipClientService::Start(void)
{
	StruSipListener stListen;
	bzero(&stListen, sizeof(stListen));
	stListen.OnClientConnectEvent = &CProSipClientService::SipClientConnectCallback;
	stListen.OnClientDisconnectEvent = &CProSipClientService::SipClientDisconnectCallback;
	stListen.OnSIPPacketEvent = &CProSipClientService::SipSIPPacketCallback;
	m_hSipSrv = SipService_Create(&stListen);
	if( m_hSipSrv == NULL )
	{
		MY_LOG_NOTICE(H_LOG, _GSTX("启动SIP协议栈*失败.  建立 SipService_Create 失败.\n"));
		GS_ASSERT(0);
		return eERRNO_SYS_ENMEM;
	}
	SipService_SetUserData(m_hSipSrv, this);
	if( m_strSipServerName.length())
	{
		SipService_Set_ServerName(m_hSipSrv, m_strSipServerName.c_str());
	}

	EnumSipErrorCode eSipError = SipService_Start(m_hSipSrv,eSIP_CONNECT_UDP,
		m_strSipUdpBindIp.length()>1 ? m_strSipUdpBindIp.c_str(): NULL, m_iSipUdpPort);
	if( eSipError )
	{
		MY_LOG_FATAL(g_pLog, _GSTX("启动SIP协议栈*失败，SipService_Start 启动失败(%d).\n"),
			(int) eSipError );
		GS_ASSERT(0);
		SipService_Release(m_hSipSrv);
		m_hSipSrv = SIP_INVALID_HANDLE;
		return eERRNO_SYS_EINVALID;
	}
	m_iSipUdpPort = SipService_Get_LocalPort(m_hSipSrv);
	MY_LOG_NOTICE( H_LOG, _GSTX("启动SIP协议栈*成功!! %s:%d \n"), 
		m_strSipUdpBindIp.empty() ? "0" : m_strSipUdpBindIp.c_str() , m_iSipUdpPort);
	m_bReady = TRUE;
	m_csKeepaliveTestTimer.Start();
	return eERRNO_SUCCESS;
}

EnumErrno CProSipClientService::Stop(void)
{
	m_bReady = FALSE;
	m_mutexSipClients.LockWrite();
	m_mapSipClients.clear();
	m_mutexSipClients.UnlockWrite();
	m_csKeepaliveTestTimer.Stop();

	return eERRNO_SUCCESS;		 
}

EnumErrno CProSipClientService::Uninit(void)
{
	m_bReady = FALSE;
	if( m_hSipSrv != SIP_INVALID_HANDLE )
	{
		SipService_Release(m_hSipSrv);
		m_hSipSrv = SIP_INVALID_HANDLE;
	}
	m_csKeepaliveTestTimer.Stop();
	return eERRNO_SUCCESS;		 
}


//SIP 协议栈回调
//连接
EnumSipErrorCode CProSipClientService::SipClientConnectCallback( SipServiceHandle hService,
														  SipSessionHandle hNewSession )
{
	CProSipClientService *pSrv = (CProSipClientService *)SipService_GetUserData(hService);
	return pSrv->OnSipClientConnectEvent(hNewSession);
}

//断开
void CProSipClientService::SipClientDisconnectCallback(SipServiceHandle hService, SipSessionHandle hSession)
{
	CProSipClientService *pSrv = (CProSipClientService *)SipService_GetUserData(hService);
	pSrv->OnSipClientDisconnectEvent(hSession);
}

//收包
void   CProSipClientService::SipSIPPacketCallback(SipServiceHandle hService, 
											SipSessionHandle hSession,
											StruSipData *pData)
{
	
	CProSipClientService *pSrv = (CProSipClientService *)SipService_GetUserData(hService);
	pSrv->OnSipPacketEvent(hSession, pData);
}


EnumSipErrorCode CProSipClientService::OnSipClientConnectEvent(SipSessionHandle hNewSession )
{
#ifdef TEST_SERVER
	m_mutexSipClients.LockWrite();
	StruMyCnnerInfo stInfo;
	do
	{
		stInfo.hCnner = m_seqConnecter++;
	}
	while( stInfo.hCnner==INVALID_SIP_CLIENT_CNNER );

	stInfo.hHandle = hNewSession;
	if( stInfo.hHandle==SIP_INVALID_HANDLE )
	{

		GS_ASSERT(0);
		m_mutexSipClients.UnlockWrite();
		return eSIP_RET_E_STATUS;
	}
	SipSession_SetUserData(stInfo.hHandle, (void*)stInfo.hCnner);	
	int iOptions = 1;
	SipSession_SetOptions(stInfo.hHandle,SIP_SESSION_O_UNREGISTER, &iOptions, sizeof(iOptions));
	stInfo.m_bTEST_SERVER = true;
	stInfo.m_iRegistering = true;
	CMapSipClient::iterator csIt = m_mapSipClients.insert(make_pair(stInfo.hCnner, stInfo) ).first;	
	m_mutexSipClients.UnlockWrite();
	return eSIP_RET_SUCCESS;
#endif
	

	//连接
	//为什么会有注册
	GS_ASSERT(0);
	return eSIP_RET_E_STATUS;

}

void  CProSipClientService::OnSipClientDisconnectEvent(SipSessionHandle hSession)
{
	if( !m_bReady )
	{
		return;
	}
	StruMyCnnerInfo*pCnner = RefCnnerInfo((SipClientConnecter)SipSession_GetUserData(hSession));
	if( !pCnner )
	{
		//已经断开
		GS_ASSERT(0);
		return;
	}
	m_mutexSipClients.LockReader();
	for( CMapMyChannel::iterator csIt=pCnner->mapMyChannels.begin();
		csIt!=pCnner->mapMyChannels.end();
		++csIt)
	{
		csIt->second.pChn->OnDisconnectEvent();
	}
	pCnner->m_bRegistered = FALSE;
	pCnner->m_iRegistering = false;
	m_mutexSipClients.UnlockReader();
	UnrefCnnerInfo(pCnner);
}

EnumErrno CProSipClientService::OnTransRecvData(const StruSipData *pSendData)
{
	if( !m_bReady )
	{
		return eERRNO_SYS_ESTATUS;
	}
	m_mutexSipClients.LockReader();
	CMapMyChannel::iterator csIt;
	csIt = m_mapSipClientsProxy.find(pSendData->stDialog.czDialogKey);
	if( csIt == m_mapSipClientsProxy.end() )
	{
		//		GS_ASSERT(0);
		m_mutexSipClients.UnlockReader();		
		return eERRNO_SYS_ESTATUS;
	}
	csIt->second.pChn->HandleSipData((StruSipData *)pSendData);
	m_mutexSipClients.UnlockReader();	
	return eERRNO_SUCCESS;
}


void CProSipClientService::OnSipPacketEvent(SipSessionHandle hSession, StruSipData *pData)
{
	if( !m_bReady )
	{
		return;
	}
	StruMyCnnerInfo*pCnner = RefCnnerInfo((SipClientConnecter)SipSession_GetUserData(hSession));
	if( !pCnner )
	{
		//已经断开
		//GS_ASSERT(0);
		return;
	}
	pCnner->tvLastKeepalive = 0;
	
#ifdef TEST_SERVER
	if( pData->eMethod = eSIP_METHOD_REGISTER )
	{
		//注册// 	

		StruSipConnnectInfo stCnnInfo;
		bzero(&stCnnInfo, sizeof(stCnnInfo));
		SipSession_GetConnectInfo(hSession, &stCnnInfo);
		pCnner->strRemoteHost = stCnnInfo.szRemoteIp;
		pCnner->iRemotePort = stCnnInfo.iRemotePort;
		pCnner->m_bRegistered=true;
		pCnner->tvLastKeepalive = 0;
		pCnner->tvLoginTv=time(NULL);
		pCnner->m_iRegistering = false;
		pCnner->strUserName = SipSession_Authorization_Username(hSession);
		StruSipData stSend = *pData;
		stSend.eDataType = eSIP_DATA_RESPONSE;
		stSend.iContentLength = 0;
		stSend.stResponseResult.bOk=true;
		SipSession_SendMessage(hSession,&stSend,NULL, 500, NULL);
		UnrefCnnerInfo(pCnner);
		return;

	}

	if( pData->eDataType == eSIP_DATA_REQUEST  &&   pData->eContentType==eSIP_CONTENT_MANSCDP_XML && pData->iContentLength>0)
	{
		if( pCnner->m_bTEST_SERVER  )
		{
			if( strstr(pData->vContent, "Keepalive" ) )
			{				
				return;
			}
		}
	}
#endif
	m_mutexSipClients.LockReader();
	CMapMyChannel::iterator csIt;
	csIt = pCnner->mapMyChannels.find(pData->stDialog.czDialogKey);
	if( csIt == pCnner->mapMyChannels.end() )
	{
//		GS_ASSERT(0);
		m_mutexSipClients.UnlockReader();
		UnrefCnnerInfo(pCnner);
		return;
	}
	csIt->second.pChn->HandleSipData(pData);
	m_mutexSipClients.UnlockReader();
	UnrefCnnerInfo(pCnner);
}

void CProSipClientService::SipDisconnectChannel(CSipChannel *pChannel)
{
	if( !m_bReady )
	{
		return ;
	}
	if(pChannel->m_bTransProxy )
	{
		m_mutexSipClients.LockWrite();
		m_mapSipClientsProxy.erase(pChannel->m_strInviteCallId);
		m_mutexSipClients.UnlockWrite();
		return;
	}
	StruMyCnnerInfo*pCnner = RefCnnerInfo(pChannel->m_hCliCnner);
	if( !pCnner )
	{
		//已经断开		
		return ;
	}
	m_mutexSipClients.LockWrite();
	pCnner->mapMyChannels.erase(pChannel->m_strInviteCallId);
	m_mutexSipClients.UnlockWrite();
	UnrefCnnerInfo(pCnner);
}

EnumErrno CProSipClientService::SessionSendSipData( CSipChannel *pChannel, 
							 const StruSipData *pSendData, StruSipData *pRes,
							 int iTimeouts,  StruSipDialogKey *pOutDlgKey )
{
	if( !m_bReady )
	{
		return eERRNO_SYS_ESTATUS;
	}
	if( pChannel->m_bTransProxy )
	{
		return SessionSendProxySipData(pChannel, pSendData, pRes, iTimeouts, pOutDlgKey);
	}

	StruMyCnnerInfo*pCnner = RefCnnerInfo(pChannel->m_hCliCnner);
	if( !pCnner )
	{
		//已经断开		
		return eERRNO_SYS_EINVALID;
	}
	EnumErrno eRet = eERRNO_SUCCESS;
	if( SipSession_SendMessage(pCnner->hHandle, pSendData, pRes, iTimeouts, pOutDlgKey))
	{
		eRet = eERRNO_SRV_REFUSE;
	}
	UnrefCnnerInfo(pCnner);
	return eRet;
}

void CProSipClientService::SipConnecterNetError(CSipChannel *pChannel,const CGSString &strDevId,
						  const CGSString &strRemoteHost,int iRemotePort,
						  const CGSString &strUserName, 
						  const  CGSString &strPassword, INT eSipErrno)
{
	if( !m_bReady )
	{
		return;
	}
	m_mutexSipClients.LockWrite();
	StruMyCnnerInfo *pInfo = NULL;	
	if( !pChannel->m_bTransProxy )
	{		
		for( CMapSipClient::iterator csIt = m_mapSipClients.begin(); 
			csIt!=m_mapSipClients.end();
			++csIt )
		{
			pInfo = &(csIt->second);
			if( pInfo->strRemoteHost == strRemoteHost &&
				pInfo->iRemotePort == pInfo->iRemotePort &&
				pInfo->strUserName == strUserName)
			{
				//已经存在
				break;
			}
			pInfo = NULL;
		}
	}
	if( pInfo  )
	{
	   if( !pInfo->m_iRegistering )
	   {
		   pInfo->m_bRegistered = false;
	   }
	}
	m_mutexSipClients.UnlockWrite();
}


EnumErrno CProSipClientService::SipCreateConnecter( CSipChannel *pChannel,
												   const CGSString &strDevId,
												   const CGSString &strRemoteHost,int iRemotePort,
												   const CGSString &strUserName, 
												   const  CGSString &strPassword,
												   const StruSipData *pInviteReq, 
												   StruSipData *pResInviteRes )
{
	if( !m_bReady )
	{
		return eERRNO_SYS_ESTATUS;
	}
	
	StruMyCnnerInfo *pInfo = NULL;
	int iRet;
	if( !pChannel->m_bTransProxy )
	{
		m_mutexSipClients.LockWrite();
		for( CMapSipClient::iterator csIt = m_mapSipClients.begin(); 
			csIt!=m_mapSipClients.end();
			++csIt )
		{
			pInfo = &(csIt->second);
			if( pInfo->strRemoteHost == strRemoteHost &&
				pInfo->iRemotePort == pInfo->iRemotePort &&
				pInfo->strUserName == strUserName)
			{
				//已经存在
				break;
			}
			pInfo = NULL;
		}
		
		if( NULL==pInfo )
		{
			StruMyCnnerInfo stInfo;
			do
			{
				stInfo.hCnner = m_seqConnecter++;
			}
			while( stInfo.hCnner==INVALID_SIP_CLIENT_CNNER );

			stInfo.hHandle = SipService_Connect(m_hSipSrv,eSIP_CONNECT_UDP, 
				strRemoteHost.c_str(), iRemotePort);
			if( stInfo.hHandle==SIP_INVALID_HANDLE )
			{

				GS_ASSERT(0);
				m_mutexSipClients.UnlockWrite();
				return eERRNO_SYS_ENMEM;
			}
			SipSession_SetUserData(stInfo.hHandle, (void*)stInfo.hCnner);	
			int iOptions = 1;
			SipSession_SetOptions(stInfo.hHandle,SIP_SESSION_O_UNREGISTER, &iOptions, sizeof(iOptions));
			stInfo.strRemoteHost = strRemoteHost;
			stInfo.iRemotePort = iRemotePort;
			stInfo.strUserName = strUserName;
			stInfo.strPassword = strPassword;

			CMapSipClient::iterator csIt = m_mapSipClients.insert(make_pair(stInfo.hCnner, stInfo) ).first;
			pInfo = &(csIt->second);
		}


		if( !pInfo )
		{
			GS_ASSERT(0);
			m_mutexSipClients.UnlockWrite();
			return eERRNO_SYS_ENMEM;
		}
		AtomicInterInc(pInfo->m_iBusy); //增加引用
		time_t tvCur = time(NULL);
#ifdef TEST_SERVER
		if( !pInfo->m_bTEST_SERVER && (!pInfo->m_bRegistered || (tvCur-pInfo->tvLoginTv)<1800) )
#else
		if( !pInfo->m_bRegistered || (tvCur-pInfo->tvLoginTv)<1800 )
#endif
		{
			do 
			{
				if(  !pInfo->m_iRegistering )
				{
					pInfo->m_iRegistering = TRUE;
					break;
				}
				m_mutexSipClients.UnlockWrite(); 
				MSLEEP(10); 
				if( !m_bReady )
				{
					return eERRNO_SYS_EBUSY;
				}
				m_mutexSipClients.LockWrite(); 
			} while ( pInfo->m_iRegistering);
			m_mutexSipClients.UnlockWrite(); 

			if( !pInfo->m_bRegistered  )
			{

				StruSipData stRes;
				bzero(&stRes, sizeof(stRes));
				iRet = SipSession_Resigter(pInfo->hHandle, pInfo->strUserName.c_str(),
					pInfo->strPassword.c_str(), &stRes,  1000L*10);

				if( iRet || !stRes.stResponseResult.bOk )
				{
					//注册失败
					GS_ASSERT(0);	
					MY_LOG_FATAL(H_LOG, _GSTX("CSipChannel 打开失败. %s:%d SipSession 发送 Resiter 命令失败.\n") , 
						pInfo->strRemoteHost.c_str(), pInfo->iRemotePort );
					AtomicInterDec(pInfo->m_iBusy);
					pInfo->m_bRegistered = FALSE;
					pInfo->m_iRegistering = FALSE;  //放在后面
					return eERRNO_SRV_REFUSE;
				}
				pInfo->m_bRegistered = TRUE;
				pInfo->m_iRegistering = FALSE;   //放在后面
			}
		}
		else
		{
			m_mutexSipClients.UnlockWrite();
		}
	}


	StruMyChnInfo stChnInfo;
	bzero(&stChnInfo,sizeof(stChnInfo));

	CGSString strTo;
	GS_SNPRINTF((char*)pInviteReq->stDialog.szTo,255, "sip:%s@%s:%d",
		strDevId.c_str(), strRemoteHost.c_str(),iRemotePort);
	if( pChannel->m_bTransProxy )
	{
		iRet = SessionSendProxySipData(pChannel, pInviteReq, pResInviteRes, 1000L*10,
			&stChnInfo.stInviteDlgKey);
	}
	else
	{

	iRet = SipSession_SendMessage(pInfo->hHandle, pInviteReq, pResInviteRes, 1000L*10,
								&stChnInfo.stInviteDlgKey);
	}
	if( iRet )
	{
		//GS_ASSERT(0);
		if( pInfo )
		{
			MY_LOG_FATAL(H_LOG, _GSTX("CSipChannel 打开失败. %s:%d SipSession 发送 Invite 命令失败.\n") , 
			pInfo->strRemoteHost.c_str(), pInfo->iRemotePort );
			AtomicInterDec(pInfo->m_iBusy);
		}
		else
		{
			MY_LOG_FATAL(H_LOG, _GSTX("CSipChannel 打开失败. %s:%d Proxy 发送 Invite 命令失败.\n") , 
				strRemoteHost.c_str(),iRemotePort );			
		}
		return eERRNO_SYS_EBUSY;
	}
	GS_ASSERT(stChnInfo.stInviteDlgKey.czDialogKey[0]);
	memcpy(&stChnInfo.stInviteDlgKey, &pResInviteRes->stDialog, sizeof(stChnInfo.stInviteDlgKey));
	
	m_mutexSipClients.LockWrite();
	
	pChannel->m_strInviteCallId = stChnInfo.stInviteDlgKey.czDialogKey;
		
	stChnInfo.pChn = pChannel;
	if( pChannel->m_bTransProxy )
	{
		pChannel->m_hCliCnner = (SipClientConnecter)-1;
		m_mapSipClientsProxy.insert(make_pair(pChannel->m_strInviteCallId, stChnInfo));
	}
	else
	{
#ifdef TEST_SERVER
		pChannel->m_bTEST_SERVER =  pInfo->m_bTEST_SERVER;
#endif
		pChannel->m_hCliCnner = pInfo->hCnner;

		pInfo->mapMyChannels.insert(make_pair(pChannel->m_strInviteCallId, stChnInfo));
	}
	m_mutexSipClients.UnlockWrite();
	if( !pChannel->m_bTransProxy )
	{
		AtomicInterDec(pInfo->m_iBusy);	
	}
	return eERRNO_SUCCESS;	
}


CProSipClientService::StruMyCnnerInfo* CProSipClientService::RefCnnerInfo(SipClientConnecter hCnner )
{
	CGSAutoReaderMutex rLocker(&m_mutexSipClients);
	CMapSipClient::iterator csIt = m_mapSipClients.find(hCnner);
	if( csIt==m_mapSipClients.end() )
	{
		return NULL;
	}
	AtomicInterInc(csIt->second.m_iBusy);
	return &(csIt->second);
}

EnumErrno CProSipClientService::SessionSendProxySipData( CSipChannel *pChannel, 
								  const StruSipData *pSendData, StruSipData *pRes,
								  int iTimeouts,  StruSipDialogKey *pOutDlgKey )
{
	StruGSPSendSip stData;
	bzero(&stData, sizeof(stData));
	stData.iTimeouts = iTimeouts;
	stData.pOutDlgKey = pOutDlgKey;
	stData.pRes = pRes;
	stData.pSendSipData = pSendData;

	if( pChannel->GetParent()->SendEvent(GSP_EVT_CLI_SIP_SEND, &stData, sizeof(stData)) )
	{
		return eERRNO_SUCCESS;
	}
	return eERRNO_SYS_EINVALID;

}