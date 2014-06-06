#include "SipSrvSession.h"
#include "SipServer.h"
#include "Log.h"
#include "GSPMemory.h"
#include "SIP/SIPParser.h"
#include "RTP/SdpParser.h"
#include "MediaInfo.h"
#include "../Server.h"
#include "../IBaseSource.h"
#include "StrFormater.h"




using namespace  GSP;
using namespace  GSP::SIP;
using namespace GSP::RTP;




#define TIMER_ID_KEEPALIVE 1

static void _FreeMemory( void *pData )
{	
	CMemoryPool::Free(pData);
}

CSipSrvSession::CSipSrvSession(CSipServer *pParent)
:CISrvSession(pParent)
,m_pSipSrv(pParent)

{

	m_strInviteCallId.clear();
	m_hSipCnner = INVALID_SIP_SESSION_CONNECTER;

	m_pRefSource = NULL;

	m_eInputStreamPkt = eSTREAM_PKG_NONE;
	m_eOutputStreamPkt = eSTREAM_PKG_NONE;

	m_pPkgCvt = NULL;


	bzero(&m_stPlayStatus, sizeof(m_stPlayStatus));  //当前播放状态
	m_eTransModel = GSP_TRAN_RTPLAY; //传输模式
	m_pRtpSender = NULL;
	m_eSignal = eSIG_NONE;
	m_iKeepalivePlugs = 0;
	


	m_csHandleSipDataTask.Init(this,
		(FuncPtrObjThreadPoolEvent)&CSipSrvSession::OnHandleSipDataTaskPoolEvent,
		1, FALSE);
	m_csHandleSipDataTask.SetFreedTaskDataFunction((FuncPtrFree)_FreeMemory);
	m_csHandleSipDataTask.SetMaxWaitTask(100);
	GSStrUtil::Format(m_csSubject.m_strSendStreamSeq, "%u", m_iAutoID);
	m_csSubject.m_strSendDevID = m_csSubject.m_strSendStreamSeq;
	m_csSubject.m_strRcvDevID = m_csSubject.m_strSendDevID;
	m_csSubject.m_strSendStreamSeq = m_csSubject.m_strSendDevID;
	m_iRemoteRtpPort = -1;
	m_strRemoteRtpIp.clear();

	m_iPSSRC = pParent->GetSSRC();
	m_iYSSRC =  m_iYSSRC;


	m_eOutRtpPlayloadType = (EnumRTPPayloadType)127;
	


	m_pSysHeaderFrame = NULL;
	m_iSendSysHeaderTicks = 0;


	m_iSNSeq = 1;

	m_iFileBegin = 0;
	m_iFileEnd = 10000;



	bzero(&m_stInviteDlgKey, sizeof(m_stInviteDlgKey));

	m_fSpeed = 1.0;

	m_bFirstFrame = TRUE;

	m_bTestRtcp = pParent->IsForceRTCPKeepalive();

	MY_LOG_DEBUG(g_pLog, _GSTX("SipSrvSession(%u) Create.\n"), m_iAutoID);
}

CSipSrvSession::~CSipSrvSession(void)
{
	m_csKeepaliveTimer.Stop();
	m_pSipSrv->ReleaseSSRC(m_iPSSRC);

	if( m_hSipCnner!=INVALID_SIP_SESSION_CONNECTER)
	{
		m_pSipSrv->SessionDisconnectInvite(this);
		m_hSipCnner = INVALID_SIP_SESSION_CONNECTER;
	}


	if( m_pRtpSender )
	{
		m_pRtpSender->Stop();
		delete m_pRtpSender;
		m_pRtpSender = NULL;

	}

	if( m_pSysHeaderFrame )
	{
		m_pSysHeaderFrame->UnrefObject();
		m_pSysHeaderFrame = NULL;
	}

	if( m_pPkgCvt )
	{
		delete  m_pPkgCvt;
		m_pPkgCvt = NULL;
	}

	MY_LOG_DEBUG(g_pLog, _GSTX("SipSrvSession(%u) Destory.\n"), m_iAutoID);
}

BOOL CSipSrvSession::Init( SipSessionConnecter cnner, 
						  const CGSString &strInviteCallId,const StruSipConnnectInfo &stNetInfo)
{
	m_csWRMutex.LockWrite();
	GSStrUtil::Format(m_strClientIPInfo, "%s:%d",
		stNetInfo.szRemoteIp, stNetInfo.iRemotePort);

	GS_SNPRINTF(m_stClientInfo.szRemoteIP, GSP_STRING_IP_LEN, "%s", stNetInfo.szRemoteIp);
	m_stClientInfo.iReomtePort = stNetInfo.iRemotePort;

	m_csWRMutex.UnlockWrite();

	if(  m_csHandleSipDataTask.IsInit() )
	{
		m_hSipCnner = cnner;		
		m_strInviteCallId = strInviteCallId;
	}
	else
	{

		MY_LOG_NOTICE(g_pLog, _GSTX("SipSession(%u) Init Of Client '%s'.\n"),
			m_iAutoID, m_strClientIPInfo.c_str() );
		return FALSE;
	}

	m_csKeepaliveTimer.Init(this,
		(FuncPtrTimerCallback)&CSipSrvSession::OnTimerEvent
		,TIMER_ID_KEEPALIVE ,
		/*1000L*(m_pSipSrv->m_stConfig.iKeepaliveTimeouts/3+1), */
		1000*10,
		FALSE); 

	if( !m_csKeepaliveTimer.IsReady() )
	{
		return FALSE;
	}
	m_iKeepalivePlugs = 0;
	m_csKeepaliveTimer.Start();
	return TRUE;
}

void CSipSrvSession::DeleteBefore(void)
{

	StopAllAction();
	m_csWRMutex.LockWrite();

	CRefSource *pTemp = m_pRefSource;
	m_pRefSource = NULL;
	m_csWRMutex.UnlockWrite();

	if( pTemp )
	{
		pTemp->Release();
	}



	m_csKeepaliveTimer.Stop();

	m_csHandleSipDataTask.Uninit();
	if( m_hSipCnner!=INVALID_SIP_SESSION_CONNECTER)
	{
		m_pSipSrv->SessionDisconnectInvite(this);
		m_hSipCnner = INVALID_SIP_SESSION_CONNECTER;
	}

	if( m_pRtpSender )
	{
		m_pRtpSender->Stop();
		delete m_pRtpSender;
		m_pRtpSender = NULL;

	}

}

void CSipSrvSession::Start(void)
{
	if( m_bTestRtcp && m_pRtpSender )
	{
		m_iKeepalivePlugs = 0;
		m_csKeepaliveTimer.Start();
	}
}


void CSipSrvSession::OnDisconnectEvent(void)
{	

	m_pSipSrv->AsyncDestroySession(this);
}

void CSipSrvSession::HandleSipData(StruSipData *pData)
{
	if( !m_bTestRtcp )
	{
		m_iKeepalivePlugs = 0;
	}
// 	if( pData->eMethod == eSIP_METHOD_REGISTER && pData->eDataType== eSIP_DATA_REQUEST )
// 	{
// 		pData->eDataType = eSIP_DATA_RESPONSE;
// 		pData->iContentLength = 0;
// 		pData->eContentType = eSIP_CONTENT_NONE;
// 		pData->stResponseResult.bOk = 1;
// 		pData->stResponseResult.iSipErrno = 200;
// 		m_pSipSrv->SessionSendSipData(this,pData, NULL, 0, NULL);
// 		return;
// 	}




	StruSipData *pBuf = (StruSipData *)CMemoryPool::Malloc(sizeof(*pData));
	if( !pBuf )
	{
		HandleSipDataOfError(pData, _GSTX("没有内存"));
		return;
	}
	memcpy( pBuf, pData, sizeof(*pBuf));
	INT iRet = m_csHandleSipDataTask.Task(pBuf);
	if( m_csHandleSipDataTask.RSUCCESS!= iRet )
	{
		GS_ASSERT(iRet != m_csHandleSipDataTask.EFLOWOUT);		
		CMemoryPool::Free(pBuf);
		HandleSipDataOfError(pData, _GSTX("没有内存"));
	}
}


void CSipSrvSession::HandleSipDataOfError(StruSipData *pData, const char *czError )
{
	if( pData->eDataType == eSIP_DATA_REQUEST )
	{
		//请求消息
		pData->eDataType = eSIP_DATA_RESPONSE;
		pData->iContentLength = 0;
		pData->eContentType = eSIP_CONTENT_NONE;
		pData->stResponseResult.bOk = 0;
		pData->stResponseResult.iSipErrno = 402;
		m_pSipSrv->SessionSendSipData(this,pData, NULL, 0, NULL);
	}
	MY_LOG_ERROR(g_pLog, _GSTX("不能处理 SIP 数据包. %s\n"), czError );
	Signal( eSIG_ASSERT);
}

void CSipSrvSession::OnHandleSipDataTaskPoolEvent( CObjThreadPool *pTkPool, StruSipData *pData )
{

	if( pData->eDataType==eSIP_DATA_RESPONSE )
	{
		//回复
		switch( pData->eMethod )
		{
		case eSIP_METHOD_ACK :
			{
				HandleRequestAck(pData);
			}
			break;
		}
	}
	else
	{ //请求 
		switch( pData->eMethod )	
		{
		case eSIP_METHOD_INVITE :
			{
				HandleRequestInvite(pData);
			}
			break;
		case eSIP_METHOD_ACK :
			{
				HandleRequestAck(pData);
			}
			break;
		case eSIP_METHOD_BYE :
			{
				HandleRequestBye(pData);
			}
			break;
		case eSIP_METHOD_MESSAGE :
			{
				//keepalive

			}
			break;
		case eSIP_METHOD_INFO :
			{
				// 控制命令
				if( pData->eContentType == eSIP_CONTENT_MANSRTSP )
				{

					HandleMANSRTSP(pData);
				}
			}
			break;
		default :
			GS_ASSERT(0);
			HandleSipDataOfError(pData, _GSTX("无效请求方法"));
			break;

		}
	}
	CMemoryPool::Free(pData);
}


void CSipSrvSession::Signal(EnumSignal eSig )
{

	m_csWRMutex.LockWrite();
	if( m_eSignal != eSIG_NONE )
	{
		//已经发送异常		
		m_csWRMutex.UnlockWrite();
		return;
	}

	GS_ASSERT(m_eSignal == eSIG_NONE );
	m_eSignal = eSig;
	m_csWRMutex.UnlockWrite();

	if( eSig == eSIG_REMOTE_CLOSE )
	{
		if( m_pRefSource )
		{
			m_pRefSource->Stop(); //TODO
		}		
	}	
	SendBye();
	StopAllAction();
	MSLEEP(1);
	m_pSipSrv->AsyncDestroySession(this);


}

void CSipSrvSession::HandleRequestBye(StruSipData *pData)
{
	pData->eDataType = eSIP_DATA_RESPONSE;
	pData->iContentLength = 0;
	pData->eContentType = eSIP_CONTENT_NONE;
	pData->stResponseResult.bOk = 1;
	pData->stResponseResult.iSipErrno = 200;
	m_pSipSrv->SessionSendSipData(this,pData, NULL, 0, NULL);
	Signal(eSIG_REMOTE_CLOSE);
}

void CSipSrvSession::HandleMANSRTSP(StruSipData *pData)
{
	bool bOk = true;
	if( pData->iContentLength < 0 )
	{
		bOk = false;
	}

	CMansRtspPaser csParser;

	if( csParser.Decode(pData->vContent ) && csParser.m_stGspCtrl.iCtrlID==0 )
	{
		GS_ASSERT(0);
		bOk = false;
	}

	pData->eDataType = eSIP_DATA_RESPONSE;
	pData->iContentLength = 0;
	pData->eContentType = eSIP_CONTENT_NONE;
	pData->stResponseResult.bOk = bOk;
	pData->stResponseResult.iSipErrno = bOk ? 200 : 488;
	m_pSipSrv->SessionSendSipData(this,pData, NULL, 0, NULL);

	if(bOk &&  m_pRefSource )
	{
		if( csParser.m_stGspCtrl.iCtrlID == GSP_CTRL_SETPOINT )
		{
			//转换时间
			if( csParser.m_stGspCtrl.iArgs1 == GSP_OFFSET_TYPE_RATE  )
			{
				csParser.m_stGspCtrl.iArgs1 = GSP_OFFSET_TYPE_SECS;
				csParser.m_stGspCtrl.iArgs2 = (INT32)((csParser.m_stGspCtrl.iArgs2*(m_iFileEnd-m_iFileBegin))/1000);
			}
			else
			{
				//相对时间转为绝对时间
				csParser.m_stGspCtrl.iArgs2 = csParser.m_stGspCtrl.iArgs2+m_iFileBegin;
			}
			if( csParser.m_stGspCtrl.iArgs2 <0 )
			{
				csParser.m_stGspCtrl.iArgs2  = 0;
			}
			else if( csParser.m_stGspCtrl.iArgs2 >=(m_iFileEnd-m_iFileBegin) )
			{
				csParser.m_stGspCtrl.iArgs2 = (m_iFileEnd-m_iFileBegin);
			}
		}

		if(csParser.m_stGspCtrl.iCtrlID == GSP_CTRL_PLAY && m_fSpeed!=1.00 )
		{
			csParser.m_stGspCtrl.iCtrlID =  m_fSpeed>1.00 ? GSP_CTRL_FAST : GSP_CTRL_SLOW;
		}

		if( csParser.m_stGspCtrl.iCtrlID != GSP_CTRL_SETPOINT )
		{
			m_fSpeed = csParser.m_fSpeed;
		}
		m_pRefSource->Ctrl(csParser.m_stGspCtrl);
	}

}


void CSipSrvSession::SendBye(void)
{

	if( m_hSipCnner != INVALID_SIP_SESSION_CONNECTER )
	{
		StruSipData stData;
		bzero(&stData, sizeof(StruSipData));
		stData.eDataType = eSIP_DATA_REQUEST;
		stData.iContentLength = 0;
		stData.eContentType = eSIP_CONTENT_NONE;
		stData.eMethod  = eSIP_METHOD_BYE;
		m_pSipSrv->SessionSendSipData(this,&stData, NULL, 0, NULL);
	}	
}

void CSipSrvSession::SendEndMessage(void)
{

	if( m_hSipCnner != INVALID_SIP_SESSION_CONNECTER )
	{
		StruSipData stData;
		bzero(&stData, sizeof(stData));
		memcpy(&stData.stDialog, &m_stInviteDlgKey, sizeof(m_stInviteDlgKey));
		stData.eDataType = eSIP_DATA_REQUEST;

		CMXmlMediaStatusParser csParser;
		csParser.m_strSn = GSStrUtil::ToString(m_iSNSeq++); 
		csParser.m_strDevId = m_csSubject.m_strSendDevID;
		csParser.m_strNotifyType = "121";

		CGSString strMessage = csParser.Encode();
		stData.iContentLength = strMessage.length();
		stData.eContentType = eSIP_CONTENT_MANSCDP_XML;
		strncpy(stData.vContent, strMessage.c_str(),SIP_MAX_CONTENT_LEN );
		stData.eMethod  = eSIP_METHOD_MESSAGE;
		stData.stDialog.szSubject[0] = '\0';
		//strncpy(stData.stDialog.szSubject, m_csSubject.Encode().c_str(), SIP_MAX_SUBJECT_STRING );
		//StruSipData stRes;
		//bzero(&stRes, sizeof(stRes));
		m_pSipSrv->SessionSendSipData(this,&stData, NULL, 0, NULL);
	}
}

void CSipSrvSession::StopAllAction(void)
{

	m_csKeepaliveTimer.Stop();	
	m_csHandleSipDataTask.Uninit();
	if( m_pRefSource )
	{
		m_pRefSource->Stop();
	}

	if( m_hSipCnner!=INVALID_SIP_SESSION_CONNECTER)
	{
		m_pSipSrv->SessionDisconnectInvite(this);
		m_hSipCnner = INVALID_SIP_SESSION_CONNECTER;
	}
	if( m_pRtpSender )
	{
		m_pRtpSender->Stop();
	}

}


EnumErrno CSipSrvSession::OnSourceEvent(CRefSource *pRefSource,
										CRefSource::EnumRefSourceEvent eEvt, void *pParam)
{
	switch(eEvt)
	{
	case CRefSource::eEVT_STREAM_FRAME :
		{
			//数据帧
			return SendStreamFrame( (CFrameCache*) pParam);
		}
		break;
	case CRefSource::eEVT_PLAY_STATUS :
		{
			//播放状态 TODO...
			::memcpy(&m_stPlayStatus, pParam, sizeof(m_stPlayStatus));
			//	SendCommand(GSP_CMD_ID_RET_STATUS, &m_stPlayStatus, sizeof(m_stPlayStatus));
			if( m_stPlayStatus.iPosition == 10000 )
			{
				//发送结束命令				
				SendEndMessage();
			}
		}
		break;
	case CRefSource::eEVT_SOURCE_RELEASE :
		{
			//数据源被释放	TODO...
			// 			EnumGSPCommandID iCmd = GSP_CMD_ID_ASSERT_AND_CLOSE;
			// 			if( m_eTransModel!=GSP_TRAN_RTPLAY )
			// 			{
			// 				if( m_stPlayStatus.iPosition == 100 )
			// 				{					
			// 					iCmd =  GSP_CMD_ID_COMPLETE; //流结束
			// 				}
			// 				else
			// 				{
			// 					m_bStopStream = TRUE;
			// 				}
			// 			}
			// 			StruGSPCmdReturn stCmd;
			// 			bzero(&stCmd, sizeof(stCmd));
			// 			stCmd.iErrno = GSP_PRO_RET_EEND;
			// 			SendCommand(iCmd,&stCmd, sizeof(stCmd));
			// 	
			Signal(eSIG_RELEASE);
		}
		break;
	case CRefSource::eEVT_SOURCE_ASSERT :
		{
			//数据源异常 TODO...

			// 			StruGSPCmdReturn stCmd;
			// 			bzero(&stCmd, sizeof(stCmd));
			// 			stCmd.iErrno = GSP_PRO_RET_ESTREAM_ASSERT;
			// 			SendCommand(GSP_CMD_ID_ASSERT_AND_CLOSE,&stCmd, sizeof(stCmd));

			Signal(eSIG_ASSERT);
		}
		break;
	default :
		;
	}
	return eERRNO_SUCCESS;
}


EnumErrno CSipSrvSession::SendStreamFrame( CFrameCache* pSrcFrame )
{
	//发送流		
	if( !m_pRtpSender  || !m_pPkgCvt)
	{
		return eERRNO_SRC_EUNUSED;
	}

	if( pSrcFrame->m_stFrameInfo.eMediaType == GS_MEDIA_TYPE_SYSHEADER )
	{
		m_iSendSysHeaderTicks  = 80;
	}

	EnumErrno eRet = m_pPkgCvt->Conver(pSrcFrame, TRUE);
	if( eRet )
	{
		return eRet;
	}

	CFrameCache *pNewFrame;

	
	
	while( (pNewFrame = m_pPkgCvt->Get()) )
	{		
// 		if( m_pSysHeaderFrame )
// 		{
// 			if( m_iSendSysHeaderTicks++ > 90 	&&
// 				pSrcFrame->m_stFrameInfo.eMediaType == GS_MEDIA_TYPE_VIDEO &&  
// 				pSrcFrame->m_stFrameInfo.bKey )
// 			{
// 				//重复发信息帧
// 				m_pRtpSender->Send(m_pSysHeaderFrame);
// 				m_iSendSysHeaderTicks = 0;
// 			}
// 		}

		if( pNewFrame->m_stFrameInfo.eMediaType != GS_MEDIA_TYPE_VIDEO &&
			(m_eOutputStreamPkt == eSTREAM_PKG_Standard || 
			(m_eTransModel == GSP_TRAN_REPLAY && (m_fSpeed<0.99 || m_fSpeed>1.01 ) ) ) )
		{
			//不发音频
			pNewFrame->UnrefObject();
			continue;
		}

		m_pRtpSender->Send(pNewFrame);

		if( pNewFrame->m_stFrameInfo.eMediaType == GS_MEDIA_TYPE_SYSHEADER )
		{
			if( m_bFirstFrame )
			{
				MSLEEP(1);
				m_pRtpSender->Send(pNewFrame);
				MSLEEP(1);
				m_pRtpSender->Send(pNewFrame);
				m_bFirstFrame = FALSE;
			}
			SAFE_DESTROY_REFOBJECT(&m_pSysHeaderFrame);
			m_pSysHeaderFrame = pNewFrame;
			m_pSysHeaderFrame->RefObject();
		}
		pNewFrame->UnrefObject();
	}
	return eERRNO_SUCCESS;	
}

void CSipSrvSession::HandleRequestAck(StruSipData *pData)
{
	GS_ASSERT(m_pRtpSender  );

	//不用回复
	// 	CGSString strSdp = m_csSdp.Serial();
	// 	if(m_pRefSource &&  strSdp.length() <= SIP_MAX_CONTENT_LEN)
	// 	{
	// 
	// 		pData->eDataType = eSIP_DATA_RESPONSE;
	// 		pData->iContentLength = strSdp.length();
	// 		pData->eContentType = eSIP_CONTENT_SDP;
	// 		pData->stResponseResult.bOk = 1;
	// 		pData->stResponseResult.iSipErrno = 200;
	// 		SipSession_SendMessage(m_hSipSession,pData, NULL, 0, NULL);
	// 	}
	// 	else
	// 	{
	// 		HandleSipDataOfError(pData, _GSTX("Sip 协议栈溢出."));
	// 		GS_ASSERT(0);
	// 		return;	
	// 	}
	m_pRefSource->Start();

	memcpy(&m_stInviteDlgKey, &pData->stDialog, sizeof(m_stInviteDlgKey));

	if( m_eTransModel == GSP_TRAN_RTPLAY )
	{
		//实时流
		StruGSPCmdCtrl stCtrl;
		bzero(&stCtrl, sizeof(stCtrl));
		stCtrl.iCtrlID = GSP_CTRL_PLAY;
		m_pRefSource->Ctrl(stCtrl);		
	}
}

void CSipSrvSession::OnTimerEvent( CWatchTimer *pTimer )
{
	switch(pTimer->GetID() )
	{
	case TIMER_ID_KEEPALIVE :
		{
			//检查活动      
			m_iKeepalivePlugs++;
			if( m_iKeepalivePlugs > 7 )
			{				
				MY_LOG_ERROR(g_pLog, _GSTX("SipSrvSession(%u) 长时间没有收到Keepalive 被关闭.\n"),
					m_iAutoID);
				Signal(eSIG_ASSERT);
			}
		}
		break;
		// 	case TIMER_ID_SEND_KEEPALIVE :
		// 		{
		// 			//发送Keepalive
		// // 			if( m_eStatus != eST_ASSERT )
		// // 			{
		// // 				StruGSPCmdKeepalive stKeepalive;
		// // 				bzero(&stKeepalive, sizeof(stKeepalive));
		// // 				stKeepalive.iMagic = GSP_MAGIC;
		// // 				stKeepalive.iArgs = 0;
		// // 				SendCommand(GSP_CMD_ID_KEEPAVLIE, &stKeepalive, sizeof(stKeepalive));
		// // 			}
		// 		}
		// 		break;
		// 	case TIMER_ID_SEND_RTCP :
		// 		{			
		// 			if( m_pContent )
		// 			{
		// 				m_pContent->TimerTicks();
		// 			}
		// 		}
		// 		break;
	}
}



void CSipSrvSession::HandleRequestInvite(StruSipData *pData)
{
	memcpy(&m_stInviteDlgKey, &pData->stDialog, sizeof(m_stInviteDlgKey));

	if( m_pRefSource != NULL )
	{
		//重复命令
		pData->eDataType = eSIP_DATA_RESPONSE;
		pData->iContentLength = 0;
		pData->eContentType = eSIP_CONTENT_NONE;
		pData->stResponseResult.bOk = 1;
		pData->stResponseResult.iSipErrno = 101;
		m_pSipSrv->SessionSendSipData(this,pData, NULL, 0, NULL);
		return;
	}

	if( pData->stDialog.szSubject[0] =='\0'  )
	{
		HandleSipDataOfError(pData, _GSTX("Invite 没有Subject 项") );
		return;
	}


	if( !m_csSubject.Decode( pData->stDialog.szSubject ) )
	{
		HandleSipDataOfError(pData, _GSTX("Invite Subject项个数不对"));
		return;
	}

	CSdpParser *pRequestSdp = NULL;
	CSdpParser csSdpParser;
	if( pData->eContentType == eSIP_CONTENT_SDP )
	{
		//带有SDP


		if( csSdpParser.Parser(pData->vContent) )
		{
			HandleSipDataOfError(pData, _GSTX("Sdp不符合规范"));
			GS_ASSERT(0);
			return;
		}
		pRequestSdp = &csSdpParser;

		if( !m_bTestRtcp )
		{
			//是否发送RTCP
			m_bTestRtcp = TRUE;
			for( int i = 0; i<csSdpParser.m_vMedia.size(); i++ )
			{
				for( int j = 0; j<csSdpParser.m_vMedia[i].vAtribute.size(); j++ )
				{
					if( GSStrUtil::ToLower(csSdpParser.m_vMedia[i].vAtribute[i].strName)=="recvonly" )
					{
						m_bTestRtcp = FALSE;
						break;
					}
				}
			}
			
		}
	}

	if( !BuildSource( m_csSubject.m_strSendDevID, pRequestSdp))
	{
		HandleSipDataOfError(pData, _GSTX("获取数据源失败不符合规范"));
		GS_ASSERT(0);
		return;
	}
	CGSString strSdp = m_csSdp.Serial();
	if( strSdp.length() <= SIP_MAX_CONTENT_LEN)
	{
		pData->eDataType = eSIP_DATA_RESPONSE;
		pData->iContentLength = strSdp.length();
		strncpy(pData->vContent, strSdp.c_str(), SIP_MAX_CONTENT_LEN);
		pData->eContentType = eSIP_CONTENT_SDP;
		pData->stResponseResult.bOk = 1;
		pData->stResponseResult.iSipErrno = 200;
		m_pSipSrv->SessionSendSipData(this,pData, NULL, 0, NULL);
	}
	else
	{
		HandleSipDataOfError(pData, _GSTX("Sip 协议栈溢出."));
		GS_ASSERT(0);
		return;	
	}

	MY_LOG_DEBUG(g_pLog, _GSTX("SipSrvSession(%u) RTP 发送：%s:%d => %s:%d \n"), 				
		m_iAutoID, 
		m_csSdp.m_stCAddr.strIp.c_str(), m_csSdp.m_vMedia[0].stPort.vPort[RTP_PORT_IDX],
		m_strRemoteRtpIp.c_str(), m_iRemoteRtpPort);

}


BOOL CSipSrvSession::BuildSource( const CGSString &strSourceID, CSdpParser *pRequestSdp)
{
	m_csWRMutex.LockWrite();
	GS_SNPRINTF(m_stClientInfo.szSourceKey, GSP_MAX_URI_KEY_LEN, "%s", strSourceID.c_str() );
	m_csWRMutex.UnlockWrite();

	m_eOutputStreamPkt = eSTREAM_PKG_NONE;
	CMediaInfo csInfo;
	CUri csUri;
	if( pRequestSdp && !pRequestSdp->m_strUUri.empty() )
	{
		if( !csUri.Analyse(pRequestSdp->m_strUUri.c_str()) )
		{
			csUri.SetScheme("sip");
			csUri.SetHost("127.0.0.1");
		}
	}
	else
	{
		csUri.SetScheme("sip");
		csUri.SetHost("127.0.0.1");
	}

	csUri.SetKey(strSourceID.c_str());


	if(pRequestSdp && !pRequestSdp->m_strSName.empty() )
	{
		m_eTransModel = TransModel28181SSName2I(pRequestSdp->m_strSName);
	}





	if( m_pSipSrv->m_pServer->RequestSource( csUri, csInfo, m_eTransModel, &m_pRefSource) )
	{
		MY_LOG_ERROR(g_pLog, _GSTX("SIP 请求数据源: %s 失败!\n"),  strSourceID.c_str() );
		return FALSE;
	}
	m_pRefSource->SetListener(this,(CRefSource::FuncPtrRefSourceEvent) &CSipSrvSession::OnSourceEvent);
	m_pRefSource->SetTransMode(m_eTransModel);
	CIBaseSource *pSrc = m_pRefSource->Source();
	const CIMediaInfo &rInfo = pSrc->MediaInfo();
	INT iCnts = rInfo.GetChannelNums();
	EnumGSCodeID iCodeId = GS_CODEID_NONE;
	CGSString strEmpty;
	EnumGSCodeID iOutCodeID = GS_CODEID_NONE;

	const CIMediaInfo::StruMediaChannelInfo *pInfo = NULL;
	for( INT k=0; k<iCnts; k++ )
	{
		pInfo = rInfo.GetSubChannel(k);
		if( pInfo && pInfo->stDescri.eMediaType==GS_MEDIA_TYPE_VIDEO )
		{
			iCodeId = (EnumGSCodeID)pInfo->stDescri.unDescri.struVideo.eCodeID;
			break;
		}
		else
		{
			pInfo = NULL;
		}
	}
	GS_ASSERT(iCodeId);

	m_eInputStreamPkt = CMediaInfo::GetStreamPkt4GsCodeId(iCodeId);
	iOutCodeID = iCodeId;



	//TODO??????? 按每种流发送多个媒体

	//构建Sdp
	m_pRtpSender = m_pRtpSender->Create(NULL, !m_bTestRtcp , FALSE );
	if( !m_pSipSrv )
	{
		MY_LOG_ERROR(g_pLog, _GSTX("SIP 请求数据源: %s 失败!, 建立RTP 发送端失败.\n"),  strSourceID.c_str() );
		return FALSE;
	}

	m_pRtpSender->SetTimestampInterval(3600);
	m_pRtpSender->SetSSRC(m_iYSSRC);

	m_pRtpSender->SetEventListener(this, (FuncPtrRtpNetEvent)&CSipSrvSession::OnRtpReaderEvent);


	CGSString strYAtri; //y 属性

	if( pRequestSdp )
	{
		//按请求的SDP 构建
		m_csSdp = *pRequestSdp;

		if( m_csSdp.m_iTStart > 0 )
		{
			m_iFileBegin = m_csSdp.m_iTStart;
			m_iFileEnd = m_csSdp.m_iTStop;
		}

		
		if( !pRequestSdp->m_stCAddr.strIp.empty() )
		{
			m_csSdp.m_stCAddr.strIp = COSSocket::GuessLocalIp(pRequestSdp->m_stCAddr.strIp, strEmpty);
		}

		if( !pRequestSdp->m_stOAddr.strIp.empty() )
		{
			m_csSdp.m_stOAddr.strIp =  COSSocket::GuessLocalIp(pRequestSdp->m_stOAddr.strIp, strEmpty);
		}

		m_csSdp.m_vMedia.clear();
		for( UINT i = 0; i<pRequestSdp->m_vMedia.size() ; i++ )
		{
			if(pRequestSdp->m_vMedia[i].eGsMediaType != GS_MEDIA_TYPE_VIDEO )
			{
				//只支持视频方式
				continue;
			}

			if( m_csSdp.m_vMedia.size() )
			{
				break;
			}
			if( pRequestSdp->m_stCAddr.strIp.size()>1 )
			{
				m_strRemoteRtpIp = pRequestSdp->m_stCAddr.strIp;				
			}
			else if( pRequestSdp->m_stOAddr.strIp.size()>1 )
			{
				m_strRemoteRtpIp = pRequestSdp->m_stOAddr.strIp;				
			}
			else
			{
				GS_ASSERT(0);
				return FALSE;
			}

			m_iRemoteRtpPort = pRequestSdp->m_vMedia[i].stPort.vPort[RTP_PORT_IDX];

			m_pRtpSender->SetRemoteAddress(m_strRemoteRtpIp, m_iRemoteRtpPort, 0);
			m_csSdp.m_vMedia.push_back(pRequestSdp->m_vMedia[i]);

			int idx = m_csSdp.m_vMedia.size()-1;
			m_csSdp.m_vMedia[idx].stPort.vPort[RTP_PORT_IDX] = m_pRtpSender->GetRtpSocket()->LocalPort();
			m_csSdp.m_vMedia[idx].vCAddr.clear();
			CVectorSdpRtpmap &vAllowPt = m_csSdp.m_vMedia[idx].vRtpmap; //允许的Pt


			CGSString strYN("y");
			for(UINT  j = 0; j<pRequestSdp->m_vMedia[i].vAtribute.size(); j++ )
			{
				if( pRequestSdp->m_vMedia[i].vAtribute[j].strName == strYN )
				{
					strYAtri = pRequestSdp->m_vMedia[i].vAtribute[j].strValue;
				}
			}	


			vAllowPt.clear();

			//首先使用高兴新兴封装模式
			CVectorSdpRtpmap &vRtpmap = pRequestSdp->m_vMedia[i].vRtpmap;
			for( UINT j = 0; j<vRtpmap.size(); j++  )
			{
				EnumGSCodeID eOutputCodeId = GetGsCodeId4RtpPtName(vRtpmap[j].strCodeName );
				EnumStreamPackingType eOutStreamPkt = CMediaInfo::GetStreamPkt4GsCodeId(eOutputCodeId);
				if( eSTREAM_PKG_GSC3MVIDEO==eOutStreamPkt )
				{
					vAllowPt.clear();
					m_eOutputStreamPkt = eOutStreamPkt;		
					m_eInputStreamPkt = eOutStreamPkt;
					m_eOutRtpPlayloadType = vRtpmap[j].eRtpPlayloadType;
					vAllowPt.push_back(vRtpmap[j]);					
					break;					
				}
			}



			//优先使用 和本地源一直的流方式
			if( vAllowPt.empty() )
			{
				for( UINT j = 0; j<vRtpmap.size(); j++  )
				{
					EnumGSCodeID eOutputCodeId = GetGsCodeId4RtpPtName(vRtpmap[j].strCodeName );
					EnumStreamPackingType eOutStreamPkt = CMediaInfo::GetStreamPkt4GsCodeId(eOutputCodeId);
					if( m_eInputStreamPkt==eOutStreamPkt )
					{
						vAllowPt.clear();
						m_eOutputStreamPkt = eOutStreamPkt;
						iOutCodeID = eOutputCodeId;
						m_eOutRtpPlayloadType = vRtpmap[j].eRtpPlayloadType;
						vAllowPt.push_back(vRtpmap[j]);					
						break;
					}
				}
			}

			if( vAllowPt.empty() )
			{

				for( UINT j = 0; j<vRtpmap.size(); j++  )
				{
					EnumGSCodeID eOutputCodeId = GetGsCodeId4RtpPtName(vRtpmap[j].strCodeName );
					switch( eOutputCodeId )
					{
					case GS_CODEID_PS :
						{
							//优先选择
							if( GsCodeIDTestPT_PS(iCodeId) )
							{
								vAllowPt.clear();
								vAllowPt.push_back(vRtpmap[j]);

								m_eOutputStreamPkt = eSTREAM_PKG_28181PS;
								iOutCodeID = GS_CODEID_PS;
								m_eOutRtpPlayloadType = vRtpmap[j].eRtpPlayloadType;
								j = 100000;									
							}
						}
						break;			
					case GS_CODEID_ST_H264 :
						{
							if( GsCodeIDTestPT_H264(iCodeId)  && vAllowPt.empty() )
							{								
								vAllowPt.push_back(vRtpmap[j]);

								m_eOutputStreamPkt = eSTREAM_PKG_Standard;
								iOutCodeID = GS_CODEID_ST_H264;
								m_eOutRtpPlayloadType = vRtpmap[j].eRtpPlayloadType;
								//j = 100000;
								break;
							}
						}
						break;
					case GS_CODEID_ST_MP4 :
						{
							if( GsCodeIDTestPT_MP4(iCodeId) && vAllowPt.empty() )
							{

								vAllowPt.push_back(vRtpmap[j]);

								m_eOutputStreamPkt = eSTREAM_PKG_Standard;		
								iOutCodeID = GS_CODEID_ST_MP4;
								m_eOutRtpPlayloadType = vRtpmap[j].eRtpPlayloadType;
								//j = 100000;	
								break;
							}
						}
						break;					
					default :
						break;
					} //end switch
				} // end for j
			} //end if

			if( vAllowPt.empty() )
			{
					//增加一个
				StruSdpRtpmap tmpRtpmap;
				tmpRtpmap.eRtpPlayloadType = MakeRtpPtInfo4GsCodeId(iCodeId, tmpRtpmap.strCodeName);
				m_eOutputStreamPkt = CMediaInfo::GetStreamPkt4GsCodeId(iOutCodeID);
				m_eOutRtpPlayloadType = tmpRtpmap.eRtpPlayloadType;
				vAllowPt.push_back(tmpRtpmap);
			}
		} //end for i
	}
	else
	{
		m_csSdp.m_stCAddr.strIp = COSSocket::GuessLocalIp(strEmpty,strEmpty);
		m_csSdp.m_stOAddr.strIp = m_csSdp.m_stCAddr.strIp;
		m_csSdp.m_strSName = TransModel28181I2SName(m_eTransModel);
	}

	csUri.SetScheme("sip");
	csUri.SetHost(m_csSdp.m_stCAddr.strIp.c_str());
	csUri.SetPortArgs( m_pSipSrv->ListenPort());

	m_csSdp.m_strUUri = csUri.GetURI();

	if( 0 == m_csSdp.m_vMedia.size() )
	{
		//添加一个媒体流
		StruSdpMedia tmpSdpM;
		tmpSdpM.eGsMediaType = GS_MEDIA_TYPE_VIDEO;
		tmpSdpM.eTransType = eTRANSPORT_RTP_UDP;
		tmpSdpM.stPort.vPort[RTP_PORT_IDX] = m_pRtpSender->GetRtpSocket()->LocalPort();
		StruSdpRtpmap tmpRtpmap;
		tmpRtpmap.eRtpPlayloadType = MakeRtpPtInfo4GsCodeId(iOutCodeID, tmpRtpmap.strCodeName); 	
		m_eOutRtpPlayloadType = tmpRtpmap.eRtpPlayloadType;
		tmpSdpM.vRtpmap.push_back(tmpRtpmap);	
		m_csSdp.m_vMedia.push_back(tmpSdpM);

	}

	m_csSdp.m_vMedia[0].vAtribute.clear();
	
	CMediaInfo csInfoTemp;
	StruGSMediaDescri stInfo;
	bzero(&stInfo, sizeof(stInfo));
	stInfo.eMediaType = GS_MEDIA_TYPE_VIDEO;
	csInfoTemp.AddChannel(&stInfo, 0, NULL);
	stInfo.eMediaType = GS_MEDIA_TYPE_AUDIO;
	csInfoTemp.AddChannel(&stInfo, 1, NULL);
	stInfo.eMediaType = GS_MEDIA_TYPE_SYSHEADER;
	csInfoTemp.AddChannel(&stInfo, 2, NULL);

	if( m_eOutputStreamPkt == eSTREAM_PKG_NONE )
	{
		GS_ASSERT(0);
		return FALSE;
	}

	//初始化打包器
	m_pPkgCvt = m_pPkgCvt->Make(m_eTransModel==GSP_TRAN_RTPLAY, TRUE,
		m_eInputStreamPkt, m_eOutputStreamPkt, csInfoTemp, FALSE);
		
	if(!m_pPkgCvt )
	{

		GS_ASSERT(0);
		MY_LOG_ERROR(g_pLog, _GSTX("SIP 请求数据源: %s 失败!, 初始化流分析器失败.\n"),  strSourceID.c_str() );
		return FALSE;
	}




	StruSdpAtribute stGS_a; 

	if( !m_bTestRtcp )
	{
		stGS_a.strName = "sendonly";
		stGS_a.strValue.clear();
		m_csSdp.m_vMedia[0].vAtribute.push_back(stGS_a);
	}

	//增加y属性
	stGS_a.strName = "y";
	if( strYAtri.empty() )
	{
		m_iYSSRC = m_iPSSRC;
		stGS_a.strValue = m_csSdp.EncodeYAtri( m_eTransModel != GSP_TRAN_RTPLAY, m_iYSSRC);
	}
	else
	{
		stGS_a.strValue = strYAtri;
		int iType = 0;	
		m_csSdp.DecodeYAttri(strYAtri,iType, m_iYSSRC  );		
	}
	if( m_pRtpSender )
	{
		m_pRtpSender->SetSSRC(m_iYSSRC);
	}
	m_csSdp.m_vMedia[0].strYValue = stGS_a.strValue;



	if( eSTREAM_PKG_GSC3MVIDEO==m_eOutputStreamPkt ) 
	{
		//GXX 属性
		GS_ASSERT(pInfo);
		stGS_a.strName = "gsfmt";
		CGSString strTemp;
		MakeRtpPtInfo4GsCodeId(iCodeId, strTemp);
		stGS_a.strValue = " ";
		if( strTemp.empty() )
		{
			stGS_a.strValue += "#";
		}
		else
		{
			stGS_a.strValue  += strTemp;
		}
		
		CStrFormater::BinaryToString((BYTE*)&pInfo->stDescri, sizeof(StruGSMediaDescri), strTemp);
		stGS_a.strValue += " ";
		stGS_a.strValue += strTemp;
		m_csSdp.m_vMedia[0].vAtribute.push_back(stGS_a);
		
	}
	m_pRtpSender->SetPlayloadType( m_eOutRtpPlayloadType, MAX_UINT16 );

	if( m_pRtpSender && m_bTestRtcp )
	{
		m_pRtpSender->Start();
	}

	return TRUE;
}


//Rtp 接收线程池 回调
void CSipSrvSession::OnRtpReaderEvent( EnumRtpNetEvent eEvt, void *pEvtArgs )

{

	m_iKeepalivePlugs = 0;
	if( eEvt == eEVT_RTPNET_STREAM_FRAME )
	{	
		GS_ASSERT(0);
	}
	else
	{

		MY_LOG_INFO(g_pLog, _GSTX("CSipSrvSession(%u) Rtp Rcv Event:%d.\n"), 
			m_iAutoID, (int) eEvt );
		if( eEvt ==  eEVT_RTPNET_RTCP_BYE && m_bTestRtcp )
		{
			//对端关闭
			Signal(eSIG_REMOTE_CLOSE);			
		}

	}	
}
