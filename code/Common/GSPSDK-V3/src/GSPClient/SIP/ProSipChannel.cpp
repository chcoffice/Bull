#include "ProSipChannel.h"
#include "../ClientChannel.h"
#include "../ClientSection.h"
#include "Log.h"
#include "StrFormater.h"



using namespace GSP;
using namespace GSP::SIP;
using namespace GSP::RTP;

#define H_LOG  m_pParent->m_pLog



#define TIMER_ID_KEEPALIVE 10
#define TIMER_ID_SEND_KEEPALIVE 11
#define TIMER_ID_ASYNC_WAKEUP 12




	
// 
// static void _FreeWaitSendQueueMember( CIPacket *p )
// {
// 	SAFE_DESTROY_REFOBJECT(&p);
// }
// 
// static void _FreeTcpRcvTaskMember( CRefBuffer *p )
// {
// 	SAFE_DESTROY_REFOBJECT(&p);
// }

 GSAtomicInter CSipChannel::s_iYSSRCSeq = 100000000;


CSipChannel::CSipChannel(CClientChannel *pParent)
:CIProChannel()
,m_pParent(pParent)
,m_iAutoID(pParent->m_iAutoID)
{
#ifdef ENABLE_SIP_MODULE	
	m_pSipSrv = m_pParent->m_pSection->m_pSipSrv;
	m_bTestRtcp = m_pSipSrv->IsForceRTCPKeepalive(); //检测RTCP
#else
	m_pSipSrv = NULL;
#endif
	m_iMaxKeepaliveTimeouts = 30;

	m_iKeepalivePlugs = 0;
	 m_iCTagSequence = 1; //命令CSeq


	GSStrUtil::Format(m_csSubject.m_strRcvStreamSeq, "%u", m_iAutoID);
	GSStrUtil::Format(m_csSubject.m_strSendStreamSeq, "%u", m_iAutoID+1);
	m_csSubject.m_strRcvDevID = m_csSubject.m_strRcvStreamSeq;
	m_csSubject.m_strSendDevID = m_csSubject.m_strSendStreamSeq;
	
	m_pRtpReader = NULL;
	m_bStreamStart = FALSE; //是否开始流接收
	m_bSendAck = TRUE;

	
	m_strUsername = m_csSubject.m_strRcvStreamSeq;
	m_iKpSN = 1;
	m_eRtpPT = (EnumRTPPayloadType)-1; 
	m_eInputCodeId = GS_CODEID_NONE;


	m_iBeginTimestamp = 0;
	m_iFileLengthTv = 0;
	m_iFileStartTv = 0;
	m_iLastPTS = MAX_UINT64;
	m_iLastPos = -1;
	m_iTimestamp = 0;

	m_bWouldSendByte=FALSE;

	m_hCliCnner = INVALID_SIP_CLIENT_CNNER;
	m_strInviteCallId.clear();

	bzero(&m_stInviteDlgKey, sizeof(m_stInviteDlgKey));

	m_iPSSampleRate = 90000;
	m_fSpeed = 1.0;
	m_bTransProxy = FALSE;
	while(1)
	{
		m_iYSSRC = (UINT32) AtomicInterInc(s_iYSSRCSeq);
		if( m_iYSSRC>=100000000 && m_iYSSRC <=999999998 )
		{
			break;
		}
		else
		{
			AtomicInterSet(s_iYSSRCSeq, 100000000);
		}
	}	

	m_strDevId = "";

	m_eOutputPkgType = eSTREAM_PKG_28181PS; //默认为 GS PS 标准
	// m_eOutputPkgType = eSTREAM_PKG_GSPS; 
	m_eInputPkgType = eSTREAM_PKG_NONE; 


	
	m_pPkgCvt = NULL;
#ifdef TEST_SERVER
	m_bTEST_SERVER = false;
#endif


}


CSipChannel::~CSipChannel(void)
{

	m_csSendKeepaliveTimer.Stop();
	m_csKeepaliveTestTimer.Stop();
	if( m_pRtpReader )
	{
		m_pRtpReader->Stop();
	}

	if( m_hCliCnner != INVALID_SIP_CLIENT_CNNER)
	{

		m_pSipSrv->SipDisconnectChannel(this);
		m_hCliCnner = INVALID_SIP_CLIENT_CNNER;
	}
	if( m_pRtpReader )
	{		
		delete m_pRtpReader;
		m_pRtpReader = NULL;
	}

	SAFE_DELETE_OBJECT(&m_pPkgCvt);
}

void CSipChannel::DestoryBefore(void)
{
	m_csSendKeepaliveTimer.Stop();
	m_csKeepaliveTestTimer.Stop();\
	if( m_pRtpReader )
	{
		m_pRtpReader->Stop();
	}

	if( m_hCliCnner != INVALID_SIP_CLIENT_CNNER)
	{
		
		SendBye();		
		m_pSipSrv->SipDisconnectChannel(this);
		m_hCliCnner = INVALID_SIP_CLIENT_CNNER;
	}
}

EnumErrno CSipChannel::FlowCtrl( BOOL bStart )
{
	//TODO...
	return eERRNO_SYS_EFUNC;
}


EnumErrno CSipChannel::Open(const CUri &csUri, BOOL bAsync, INT iTimeouts)
{
	if( !m_pSipSrv )
	{
		GS_ASSERT(0);
		return eERRNO_EFATAL;
	}

	EnumErrno eErrno = eERRNO_SUCCESS;

	MY_LOG_NOTICE(H_LOG, _GSTX("CSipChannel(%u) Open 绑定URI(%s)\n"),
		m_iAutoID, csUri.GetURI());


	m_csSendKeepaliveTimer.Init(this, (FuncPtrTimerCallback)&CSipChannel::OnTimerEvent,
								TIMER_ID_SEND_KEEPALIVE, 30*1000 /*m_iMaxKeepaliveTimeouts*1000*/,FALSE);
	if( !m_csSendKeepaliveTimer.IsReady())
	{
		GS_ASSERT(0);
		MY_LOG_FATAL(H_LOG, _GSTX("CSipChannel(%u) 打开失败. 定时器初始化失败A.\n"), m_iAutoID );
		return eERRNO_SYS_EBUSY;
	}


	m_csKeepaliveTestTimer.Init(this, (FuncPtrTimerCallback)&CSipChannel::OnTimerEvent,
		TIMER_ID_KEEPALIVE, 10000,FALSE);
	if( !m_csKeepaliveTestTimer.IsReady())
	{
		GS_ASSERT(0);
		MY_LOG_FATAL(H_LOG, _GSTX("CSipChannel(%u) 打开失败. 定时器初始化失败C.\n"), m_iAutoID );
		return eERRNO_SYS_EBUSY;
	}


	if( iTimeouts<10 )
	{
		iTimeouts = 10;
	}


	CGSString strUsername;
	CGSString strPwd;
	CGSString strSubject;
	strUsername = csUri.GetKey();
	strPwd = m_csSubject.m_strRcvStreamSeq.c_str();
	StruUriAttr *pAttr;
	 
	if( (pAttr = csUri.GetAttr("username")))
	{
		strUsername = pAttr->szValue;
	}

	if( (pAttr = csUri.GetAttr("pwd")) )
	{
		strPwd = pAttr->szValue;
	}

	if( (pAttr = csUri.GetAttr("subject")))
	{
		m_csSubject.Decode(pAttr->szValue);
	}

	if( (pAttr= csUri.GetAttr("tranproxy") ) )
	{
		m_bTransProxy = TRUE;
	}
	else
	{
		m_bTransProxy = FALSE;
	}
	
	m_iFileLengthTv = 0;
	INT64 iTvStart = 0, iTvStop=0;

	if( (pAttr = csUri.GetAttr("TimeRange")) )
	{
		// 2012080305-2012090406
		CGSString strV;
		const char *pC = pAttr->szValue;

		CStrFormater::GetWordUntilChars(strV, "-", &pC );
		if( !strV.empty() )
		{
			CStrFormater::SkipSpaces(&pC);
			if( *pC=='-' )
			{
				pC++;
				CStrFormater::SkipSpaces(&pC);
			}
			iTvStart  = CStrFormater::ParserTimeString(strV);
			if( iTvStart > 0 )
			{
				m_iFileStartTv = iTvStart;
				m_iBeginTimestamp = iTvStart;
				strV = pC;
				if( strV.length() )
				{
					iTvStop = CStrFormater::ParserTimeString(strV);
					if( iTvStop>iTvStart )
					{
						m_iFileLengthTv = iTvStop-iTvStart;
					}
				}

			}

		}
	}

	BOOL bTestServer = FALSE;
#ifdef TEST_SERVER
	
	if( (pAttr = csUri.GetAttr("TestServer")) )
	{
		bTestServer = TRUE;
	}
#endif

	m_strUsername =  strUsername;


	

	StruUriAttr *pSdpAt = csUri.GetAttr("sdp");
	CGSString strSdp;

	m_csSubject.m_strSendDevID = csUri.GetKey();
	if(!bTestServer && pSdpAt )
	{
		strSdp = pSdpAt->szValue;
		m_csSdp.Parser(strSdp);
		m_csSubject.m_strRcvDevID=m_csSdp.m_strOUsername;
		m_csKeepaliveTestTimer.Stop(); //不用检查Keepalive 不会收流

		m_iFileLengthTv = m_csSdp.m_iTStop-m_csSdp.m_iTStop;
	}
	else
	{
#ifdef TEST_SERVER
		if( bTestServer && pSdpAt )
		{
			strSdp = pSdpAt->szValue;
			m_csSdp.Parser(strSdp);
			m_csSubject.m_strRcvDevID=m_csSdp.m_strOUsername;
			m_csKeepaliveTestTimer.Stop(); //不用检查Keepalive 不会收流
			m_iFileLengthTv = m_csSdp.m_iTStop-m_csSdp.m_iTStop;
			m_csSdp.m_vMedia.clear();
		}
#endif
		//构建 本地 sdp
		m_csSubject.m_strRcvStreamSeq = GSStrUtil::ToString(m_iAutoID);
		m_pRtpReader = m_pRtpReader->Create((const char *)NULL, !m_bTestRtcp, FALSE);
	//	m_pRtpReader = m_pRtpReader->Create(7000,7000,(const char *)NULL, TRUE, FALSE);
		if( !m_pRtpReader )
		{
			GS_ASSERT(0);
			MY_LOG_FATAL(H_LOG, _GSTX("CSipChannel(%u) 打开失败. 创建RtpReader %s:%d 对象失败.\n") , 
				m_iAutoID,csUri.GetHost(), csUri.GetPort() );
			return eERRNO_SYS_ENMEM;

		}
		CGSString strEmpty;
		m_csSdp.m_stCAddr.strIp = COSSocket::GuessLocalIp(csUri.GetHost(), strEmpty);
		m_csSdp.m_stOAddr.strIp = m_csSdp.m_stCAddr.strIp;
		m_csSdp.m_strOUsername = "#";

		m_csSdp.m_strSName = TransModel28181I2SName(m_pParent->m_eTranModel);
// 		GSStrUtil::Format(m_csSdp.m_strUUri, "sip://%s:%d/%s", csUri.GetHost(), 
// 						csUri.GetPort(), csUri.GetKey());
// 
#ifdef TEST_SERVER
		if( !bTestServer || m_csSdp.m_strUUri.empty() )
		{
			if( m_pParent->m_eTranModel != GSP_TRAN_RTPLAY )
			{

				GSStrUtil::Format(m_csSdp.m_strUUri, "%s:%s", 
					m_csSubject.m_strSendDevID.c_str(),
					m_csSubject.m_strSendStreamSeq.c_str() );
			}
		}
#else

 		if( m_pParent->m_eTranModel != GSP_TRAN_RTPLAY )
 		{
 
 			GSStrUtil::Format(m_csSdp.m_strUUri, "%s:%s", 
 				m_csSubject.m_strSendDevID.c_str(),
 				m_csSubject.m_strSendStreamSeq.c_str() );
 		}
#endif

		if( m_iFileLengthTv > 0)
		{
			//按文件播放
		    m_csSdp.m_iTStart = iTvStart;
			m_csSdp.m_iTStop = iTvStop;
		}
		

		//添加一个媒体流
		StruSdpMedia tmpSdpM;
		tmpSdpM.eGsMediaType = GS_MEDIA_TYPE_VIDEO;
		tmpSdpM.eTransType = eTRANSPORT_RTP_UDP;
		tmpSdpM.stPort.vPort[RTP_PORT_IDX] = m_pRtpReader->GetRtpSocket()->LocalPort();
		StruSdpRtpmap tmpRtpmap;
		tmpRtpmap.eRtpPlayloadType = MakeRtpPtInfo4GsCodeId(GS_CODEID_PS, tmpRtpmap.strCodeName); // ps	
		tmpSdpM.vRtpmap.push_back(tmpRtpmap);

		

		// 标准 h264
 		tmpRtpmap.eRtpPlayloadType = MakeRtpPtInfo4GsCodeId(GS_CODEID_ST_H264, tmpRtpmap.strCodeName); 
 		tmpSdpM.vRtpmap.push_back(tmpRtpmap);

		//标准MP4
		tmpRtpmap.eRtpPlayloadType = MakeRtpPtInfo4GsCodeId(GS_CODEID_ST_MP4, tmpRtpmap.strCodeName); 
		tmpSdpM.vRtpmap.push_back(tmpRtpmap);

		//增加海康编码
//		tmpRtpmap.eRtpPlayloadType = MakeRtpPtInfo4GsCodeId(GS_CODEID_HK_COMPLEX, tmpRtpmap.strCodeName);
//		tmpSdpM.vRtpmap.push_back(tmpRtpmap);

		//增加大华编码
//		tmpRtpmap.eRtpPlayloadType = MakeRtpPtInfo4GsCodeId(GS_CODEID_DH_COMPLEX, tmpRtpmap.strCodeName);
//		tmpSdpM.vRtpmap.push_back(tmpRtpmap);


		//增加c3mvideo编码
//		tmpRtpmap.eRtpPlayloadType = MakeRtpPtInfo4GsCodeId(GS_CODEID_GSC3MVIDEO, tmpRtpmap.strCodeName);
//		tmpSdpM.vRtpmap.push_back(tmpRtpmap);

		

		//Y 属性		
		CGSString strYAttr;		
		if( (pAttr = csUri.GetAttr("y")) )
		{
			strYAttr = pAttr->szValue;
		}
		if( strYAttr.empty() )
		{
			strYAttr = m_csSdp.EncodeYAtri(m_pParent->m_eTranModel != GSP_TRAN_RTPLAY , m_iYSSRC );
		}
		else
		{
			INT iType = 0;
			m_csSdp.DecodeYAttri(strYAttr, iType, m_iYSSRC);
		}
		StruSdpAtribute stGS_a; 
		if( !m_bTestRtcp )
		{
			
			stGS_a.strName ="recvonly";
			stGS_a.strValue.clear();
			tmpSdpM.vAtribute.push_back(stGS_a);
		}

		stGS_a.strName = "y";
		stGS_a.strValue = strYAttr;		
		tmpSdpM.strYValue = stGS_a.strValue;
		tmpSdpM.strYValue.clear();

		m_csSdp.m_vMedia.push_back(tmpSdpM);	


		
		
		strSdp = m_csSdp.Serial();		

		MY_LOG_DEBUG(g_pLog, _GSTX("CSipChannel(%u) RTP 接收监听：%s:%d \n"), 				
			m_iAutoID, 
			m_csSdp.m_stCAddr.strIp.c_str(), m_csSdp.m_vMedia[0].stPort.vPort[RTP_PORT_IDX]);
		
		//设置回调
		m_pRtpReader->SetEventListener(this, (FuncPtrRtpNetEvent)&CSipChannel::OnRtpReaderEvent);
	}
	INT iRet;
	
	StruSipData stSipData;
	bzero(&stSipData, sizeof(stSipData));

	stSipData.eDataType = eSIP_DATA_REQUEST;
	stSipData.eMethod = eSIP_METHOD_INVITE;
	stSipData.eContentType = eSIP_CONTENT_SDP;
	stSipData.iContentLength = strSdp.length();
	stSipData.stDialog.iCSeq = AtomicInterInc(m_iCTagSequence);
	
	if( stSipData.iContentLength>SIP_MAX_CONTENT_LEN )
	{
		GS_ASSERT(0);
		MY_LOG_FATAL(H_LOG, _GSTX("CSipChannel(%u) 打开失败. 创建Sdp 长度超长 %s:%d 对象失败.\n") , 
			m_iAutoID,csUri.GetHost(), csUri.GetPort() );
		return eERRNO_SYS_EFLOWOUT;
	}

	strncpy((char*)stSipData.vContent,strSdp.c_str(), SIP_MAX_CONTENT_LEN );
	GS_SNPRINTF(stSipData.stDialog.szSubject,SIP_MAX_SUBJECT_STRING,"%s", m_csSubject.Encode().c_str());
	

	MY_LOG_DEBUG(H_LOG, _GSTX("CSipChannel(%u) 发送SDP Subject:%s\n%s\r\n"), m_iAutoID, 
		   stSipData.stDialog.szSubject,
		   (char*)stSipData.vContent);

	StruSipData stSipResult;
	

	m_strDevId = csUri.GetKey();
	INT iTrys = 2;
	while( iTrys  )
	{
		iTrys--;
		bzero(&stSipResult, sizeof(stSipResult));	
		iRet = m_pSipSrv->SipCreateConnecter(this,m_strDevId, csUri.GetHost(),
			csUri.GetPort(),strUsername, strPwd, &stSipData, &stSipResult);
		if( iRet )
		{
			GS_ASSERT(0);
			MY_LOG_FATAL(H_LOG, _GSTX("CSipChannel(%u) 打开失败. %s:%d SipSession 发送 Invite 命令失败.\n") , 
				m_iAutoID,csUri.GetHost(), csUri.GetPort() );
			return eERRNO_SYS_EBUSY;
		}
		
		if( stSipResult.stResponseResult.bOk )
		{
			break;
		}
		else 
		{
			MSLEEP(100);
			m_pSipSrv->SipConnecterNetError(this,
				m_strDevId, 
				csUri.GetHost(),
				csUri.GetPort(),strUsername, strPwd, stSipResult.stResponseResult.iSipErrno );	
		}
		
	}

	if( !stSipResult.stResponseResult.bOk )
	{
		//GS_ASSERT(0);
		MY_LOG_FATAL(H_LOG, _GSTX("CSipChannel(%u) 打开失败. %s:%d 对端拒绝.\n") , 
			m_iAutoID,csUri.GetHost(), csUri.GetPort() );
		return eERRNO_CLI_ENSRC;
	}

	MY_LOG_DEBUG(H_LOG, _GSTX("CSipChannel(%u) 收到SDP:\n%s\r\n"), m_iAutoID, (char*)stSipResult.vContent);

	memcpy(&m_stInviteDlgKey, &stSipResult.stDialog, sizeof(m_stInviteDlgKey));

	if( m_stInviteDlgKey.szSubject[0] != 0 )
	{
		m_csSubject.Decode(m_stInviteDlgKey.szSubject);
	}

	m_bWouldSendByte = TRUE;

	if( stSipResult.iContentLength && stSipData.eContentType == eSIP_CONTENT_SDP )
	{
		//回复了SDP
		iRet = m_csSdp.Parser(stSipResult.vContent);
		if( iRet )
		{

			GS_ASSERT(0);
			MY_LOG_FATAL(H_LOG, _GSTX("CSipChannel(%u) 打开失败. %s:%d 对端发送错误的SDP.\n") , 
				m_iAutoID,csUri.GetHost(), csUri.GetPort() );
			return eERRNO_CLI_ENSRC;
		}
		if(m_csSdp.m_vMedia.size() &&  m_csSdp.m_vMedia[0].vRtpmap.size() )
		{
			m_eRtpPT = m_csSdp.m_vMedia[0].vRtpmap[0].eRtpPlayloadType;
			m_eInputCodeId = GetGsCodeId4RtpPtName(m_csSdp.m_vMedia[0].vRtpmap[0].strCodeName);
		}
		else
		{
			m_eRtpPT = (EnumRTPPayloadType) -10;
			m_eInputCodeId = GS_CODEID_NONE;
		}

		m_eInputPkgType = CMediaInfo::GetStreamPkt4GsCodeId(m_eInputCodeId);
		m_csSdp.ToGSMediaInfo(  m_pParent->m_csMediaInfo );

		if( m_eInputPkgType == eSTREAM_PKG_DaHua )
		{
			//海康编码为为特殊编码
			m_eOutputPkgType = eSTREAM_PKG_HiKVS;
		}
		else if( m_eInputPkgType == eSTREAM_PKG_HiKVS )
		{
			//海康编码为为特殊编码
			m_eOutputPkgType = eSTREAM_PKG_HiKVS;
		}
		else if( m_eInputPkgType == eSTREAM_PKG_Standard )
		{
			m_eOutputPkgType = m_eInputPkgType;
		}
		else if( m_eInputPkgType == eSTREAM_PKG_GSC3MVIDEO )
		{
			m_eOutputPkgType = eSTREAM_PKG_GSC3MVIDEO;
		}

		if( m_eInputPkgType  != m_eOutputPkgType )
		{
			if( (m_eOutputPkgType == eSTREAM_PKG_28181PS ||
				m_eOutputPkgType == eSTREAM_PKG_GSPS) )
			{

				CIMediaInfo::StruMediaChannelInfo *pInfo;
				UINT16 iCnts = m_pParent->m_csMediaInfo.GetChannelNums();
				for(UINT16 i = 0; i<iCnts; i++  )
				{
					pInfo = (CIMediaInfo::StruMediaChannelInfo *) m_pParent->m_csMediaInfo.GetChannel(i);
					if( pInfo )
					{

						pInfo->stDescri.unDescri.struVideo.eCodeID = (m_eOutputPkgType == eSTREAM_PKG_28181PS ? GS_CODEID_PS : GS_CODEID_GS_PS);
					}
				}
			}
			else
			{
				m_eOutputPkgType = m_eInputPkgType;
			}			
		}

		
		if(! m_pRtpReader )
		{
			m_bTestRtcp = FALSE;
		}
		else
		{
			//设置要接收的对端端口
			CGSString strRemoteIp;
			if( m_csSdp.m_stCAddr.strIp.length()>1)
			{
				strRemoteIp = m_csSdp.m_stCAddr.strIp;					
			}
			else if( m_csSdp.m_stOAddr.strIp.length())
			{
				strRemoteIp = m_csSdp.m_stOAddr.strIp;		
			}
			else
			{

				GS_ASSERT(0);
				MY_LOG_FATAL(H_LOG, _GSTX("CSipChannel(%u) 打开失败. %s:%d 对端发送错误的SDP.\n") , 
					m_iAutoID,csUri.GetHost(), csUri.GetPort() );
				return eERRNO_CLI_ENSRC;
			}
			m_pRtpReader->SetRemoteAddress(strRemoteIp,
												m_csSdp.m_vMedia[0].stPort.vPort[RTP_PORT_IDX], 0);
			MY_LOG_DEBUG(H_LOG, _GSTX("CSipChannel(%u) Remote RTP: %s:%d\n"), 
				m_iAutoID,strRemoteIp.c_str(), m_csSdp.m_vMedia[0].stPort.vPort[RTP_PORT_IDX] );
			if( m_pPkgCvt == NULL)
			{
				if( m_eInputPkgType == eSTREAM_PKG_HiKVS )
				{
					//海康编码为为特殊编码
					m_eOutputPkgType = eSTREAM_PKG_HiKVS;
				}
				m_pPkgCvt = m_pPkgCvt->Make( m_pParent->m_eTranModel==GSP_TRAN_RTPLAY, FALSE,
					m_eInputPkgType, m_eOutputPkgType,m_pParent->m_csMediaInfo, TRUE );
			}

			if( !m_pPkgCvt )
			{
				MY_LOG_ERROR(H_LOG, _GSTX("CSipChannel(%u) SDP 未知的流封装格式: <<\n%s\n>>\n"), 
					m_iAutoID, m_csSdp.Serial().c_str() );
				GS_ASSERT(0);
				return eERRNO_SYS_ECODEID;
			}

		}


	}

	m_pParent->m_iCtrlAbilities = GSP_CTRL_PLAY|GSP_CTRL_STOP;
	if( m_pParent->m_eTranModel!=GSP_TRAN_RTPLAY )
	{
		m_pParent->m_iCtrlAbilities |= GSP_CTRL_SETPOINT|GSP_CTRL_SLOW|GSP_CTRL_FAST|GSP_CTRL_PAUSE;
	}

	if( m_pRtpReader )
	{
		return m_pRtpReader->Start();
	}
	
	return eERRNO_SUCCESS;

}

EnumErrno CSipChannel::CtrlOfManstrsp(const char *czMansrtsp, BOOL bAsync,INT iTimeouts, StruGSPCmdCtrl &stGspCtrl)
 {
	 CMansRtspPaser csParser;
	 if( csParser.Decode(czMansrtsp) )
	 {
		 return eERRNO_SYS_EINVALID;
	 }
	 memcpy(&stGspCtrl, &csParser.m_stGspCtrl,sizeof(stGspCtrl));
	 if( stGspCtrl.iCtrlID&(GSP_CTRL_FAST|GSP_CTRL_SLOW|GSP_CTRL_PLAY) )
	 {
		 m_fSpeed =  csParser.m_fSpeed;
	 }
	 return Ctrl(stGspCtrl, bAsync, iTimeouts);
 }

EnumErrno CSipChannel::Ctrl(const StruGSPCmdCtrl &stCtrl, BOOL bAsync,INT iTimeouts)
{
EnumErrno eErrno = eERRNO_SUCCESS;
	if( !m_pParent->IsOpened() )
	{
		//GS_ASSERT(0);
		return eERRNO_SYS_ESTATUS;
	}


	StruGSPCmdCtrl stTemp;
	memcpy(&stTemp, &stCtrl, sizeof(stTemp));
	if( stTemp.iCtrlID &(GSP_CTRL_FAST|GSP_CTRL_SLOW|GSP_CTRL_PLAY)  )
	{

		if( stTemp.iCtrlID == GSP_CTRL_FAST )
		{
			if( stTemp.iArgs1 && stTemp.iArgs1 > 0  )
			{
				m_fSpeed =  stTemp.iArgs1;
			}
			else
			{
				m_fSpeed *= 2.0;
			}
		} 
		else if( stTemp.iCtrlID == GSP_CTRL_SLOW )
		{
			if( stTemp.iArgs1 && stTemp.iArgs1 > 0  )
			{
				m_fSpeed = 1.0/stTemp.iArgs1;
			}
			else
			{
				m_fSpeed /=2.0;
			}
		}
		stTemp.iArgs1 = 0;
		stTemp.iArgs2 = 0;
		if( m_fSpeed == 1 || m_fSpeed==0 )
		{
			stTemp.iCtrlID = GSP_CTRL_PLAY;			
		} 
		else if( m_fSpeed < 1 )
		{
			stTemp.iCtrlID = GSP_CTRL_SLOW;
			stTemp.iArgs1 = (UINT16)(1.0/m_fSpeed);
		}
		else
		{
			stTemp.iCtrlID = GSP_CTRL_FAST;
			stTemp.iArgs1 = (UINT16)m_fSpeed;
		}


	}


	if( stTemp.iCtrlID == GSP_CTRL_PLAY )
	{
		
		m_bStreamStart = TRUE;
		if( m_bSendAck )
		{
			m_bSendAck = FALSE;
// 			if( !m_bTransProxy )
// 			{
				//非代理
				m_csSendKeepaliveTimer.Start(); //开始发送Keepalive
			//}
			if( m_pParent->m_eTranModel == GSP_TRAN_RTPLAY ||
				(!m_bTransProxy && m_bTestRtcp) )
			{
				m_csKeepaliveTestTimer.Start();
			}
			SendAck(); //发送ACK
		}		
	}
	else
	{
		if( stTemp.iCtrlID==GSP_CTRL_STOP)
		{
			m_bStreamStart = FALSE;
			if( m_pParent->m_eTranModel != GSP_TRAN_RTPLAY )
			{
			//	SendMANSRTSP(stTemp);
			}
			return SendBye();
		}
		else if( stTemp.iCtrlID==GSP_CTRL_PAUSE )
		{
			m_bStreamStart = FALSE;
			if( m_bSendAck )
			{
				m_bSendAck = FALSE;
				// 			if( !m_bTransProxy )
				// 			{
				//非代理
						m_csSendKeepaliveTimer.Start(); //开始发送Keepalive
				//}				
				SendAck(); //发送ACK
			}		

		}
		
	}
	if( m_pParent->m_eTranModel != GSP_TRAN_RTPLAY )
	{			
		if( stTemp.iCtrlID == GSP_CTRL_SETPOINT &&
			stTemp.iArgs1 == GSP_OFFSET_TYPE_RATE &&
			m_iFileLengthTv > 0 )
		{
			//转换为按时时间
			stTemp.iArgs1 = GSP_OFFSET_TYPE_SECS;
			stTemp.iArgs2 = (INT32)((m_iFileLengthTv*stTemp.iArgs2)/10000);
		}
		if( stTemp.iCtrlID == GSP_CTRL_SETPOINT && stTemp.iArgs2<= ( m_iFileLengthTv+m_iFileStartTv ) )
		{
			m_iBeginTimestamp = stTemp.iArgs2;  //m_iFileStartTv+stTemp.iArgs2;
			m_iTimestamp = 0;
			m_iLastPTS = MAX_UINT64;
			stTemp.iArgs2 -= m_iFileStartTv; //相对开头的偏移

			if(stTemp.iArgs2<0  )
			{
				stTemp.iArgs2 = 0;
			}
			else if(stTemp.iArgs2>m_iFileLengthTv  )
			{
			    stTemp.iArgs2 = m_iFileLengthTv;
			}
			return SendMANSRTSP(stTemp);	
		}
		else
		{
			return SendMANSRTSP(stTemp);
		}
	}
	return eERRNO_SUCCESS;	
}



void CSipChannel::HandleSipData(StruSipData *pData)
{

	m_iKeepalivePlugs = 0;
	if( m_bTransProxy  || !m_pRtpReader )
	{
		m_pParent->KeepStreamAlive();
	}

	if( pData->eDataType == eSIP_DATA_REQUEST  &&  pData->eMethod != eSIP_METHOD_BYE)
	{

		if( pData->eContentType!=eSIP_CONTENT_MANSCDP_XML)
		{//为什么有 Request ????		
			GS_ASSERT(0);
			ResponseBadRequest(pData, 405, _GSTX("方法不允许"));
		}
		//更新状态
		CMANSCDP_XMLParser *pParser;
		pParser = pParser->GuessParser( pData->vContent);
		if( !pParser )
		{
			GS_ASSERT(0);
			ResponseBadRequest(pData, 501, _GSTX("没有实现"));
		}
		if( pParser->m_strCmdType == CMXmlMediaStatusParser::s_czCmdType )
		{
			CMXmlMediaStatusParser *pMST = dynamic_cast<CMXmlMediaStatusParser*>(pParser);
			GS_ASSERT(pMST);	
			if( pMST->m_strNotifyType=="121" )
			{
				//播放结束
				ResponseSimple(pData, 200, "OK");
				MSLEEP(1);
				StruPlayStatus stStatusTemp;
				bzero( &stStatusTemp, sizeof(stStatusTemp));
				stStatusTemp.iPosition = 10000;
				stStatusTemp.iStatus = GSP_STATUS_NORMAL;
				m_iBeginTimestamp  = -1;
    			m_pParent->HandlePlayStatus(stStatusTemp);		
				m_pParent->OnProChannelSignalEvent(CClientChannel::eSIG_STREAM_FINISH);
			}
			else
			{
				GS_ASSERT(0);
				ResponseBadRequest(pData, 405, _GSTX("方法不允许"));
			}
		}
		else
		{
			//??? 当前应该没有其他命令
			GS_ASSERT(0);
			ResponseBadRequest(pData, 501, _GSTX("没有实现"));
		}		
		delete pParser;
		return;
	}

	switch(pData->eMethod)
	{
	
	case  eSIP_METHOD_INVITE :
		{     
			m_bWouldSendByte = TRUE;
			//INVITE 回复, 使用同步模式 不应该有
			GS_ASSERT(0);			
		}
		break;
	case eSIP_METHOD_BYE :
		{
			MY_LOG_DEBUG(H_LOG, _GSTX("CSipChannel(%u) Remote Disconnect. Rcv: Byte\n"), 
				m_iAutoID );
			m_bStreamStart = FALSE;
			ResponseSimple(pData, 200, "OK");
			
			//m_pParent->OnProChannelSignalEvent(CClientChannel::eSIG_REMOTE_DISCONNECT);
		}
		break;
	case eSIP_METHOD_ACK :
		{

			MY_LOG_DEBUG(H_LOG, _GSTX("CSipChannel(%u) Remote  Rcv: Ack\n"), 
				m_iAutoID );
			memcpy(&m_stInviteDlgKey, &pData->stDialog, sizeof(m_stInviteDlgKey));
			
		}
	break;
	}
	
}

void CSipChannel::OnDisconnectEvent(void)
{
	MY_LOG_DEBUG(H_LOG, _GSTX("CSipChannel(%u) Remote Disconnect. Net Disconnect\n"), 
		m_iAutoID );
	m_pParent->OnProChannelSignalEvent(CClientChannel::eSIG_REMOTE_DISCONNECT);
}

UINT64 CSipChannel::IntervalOfPTS(UINT64 iStart, UINT64 iNow)
{
	//计算两个PTS 的时差
	//最大PTS 值
#define MAX_PTS 0x1FFFFFFFF 	
	if( iNow >= iStart )
	{
		//没有循环
		return iNow-iStart;
	}
	UINT64 iMax =  PTS_tag::DTS2MSec(MAX_PTS, m_iPSSampleRate);
	return (iNow+iMax)-iStart;

}

//Rtp 接收线程池 回调
void CSipChannel::OnRtpReaderEvent( EnumRtpNetEvent eEvt, void *pEvtArgs )

{
	
	//printf( "<");
	

	m_iKeepalivePlugs = 0;
	if( m_iMaxKeepaliveTimeouts == 30 )
	{
		m_iMaxKeepaliveTimeouts = 10;
	}	
	if( m_iBeginTimestamp<0 )
	{
		//播放结束
		return;
	}

	if(  m_pParent->m_eStatus==CIClientChannel::ST_ASSERT && !m_bStreamStart )
	{
		//已经关闭
		return;
	}
	if( eEvt == eEVT_RTPNET_STREAM_FRAME )
	{
		CFrameCache *pFrame = (CFrameCache*)pEvtArgs;
		bzero( &pFrame->m_stFrameInfo, sizeof(StruFrameInfo));
		HandleRtpStreamFrame(pFrame);
		//printf( "@");
		
	//	if( m_eRtpPT == eRTP_PT_PS )
		
	}
	else
	{
		m_pParent->KeepStreamAlive();
		MY_LOG_INFO(H_LOG, _GSTX("CSipChannel(%u) Rtp Rcv Event:%d.\n"), 
			m_iAutoID, (int) eEvt );
		if( eEvt ==  eEVT_RTPNET_RTCP_BYE && m_bTestRtcp )
		{
			//对端关闭
			m_pParent->OnProChannelSignalEvent(CClientChannel::eSIG_STREAM_FINISH);
		}

	}	
}



 
void CSipChannel::OnTimerEvent( CWatchTimer *pTimer )
{
	switch(pTimer->GetID() )
	{
	case TIMER_ID_KEEPALIVE :
		{
			//检查活动     

			m_iKeepalivePlugs++;
			if( m_iKeepalivePlugs > m_iMaxKeepaliveTimeouts )
			{				
				if( m_bTestRtcp ||  m_bStreamStart)
				{
					m_pParent->OnProChannelSignalEvent(CClientChannel::eSIG_ASSERT);
				}
				else
				{
					m_iKeepalivePlugs = 0;
				}
			} 
		}
		break;
	case TIMER_ID_SEND_KEEPALIVE :
		{
			//发送Keepalive			
			if( m_pParent->m_eStatus != CIClientChannel::ST_ASSERT && !m_bTransProxy )
			{
#ifdef TEST_SERVER
				if( !m_bTEST_SERVER )
#endif
				   SendKeepalive(FALSE);			
			}
		}
		break;	
	}
}

void CSipChannel::SendKeepalive( BOOL bResponse )
{
	
	if( m_hCliCnner != INVALID_SIP_CLIENT_CNNER )
	{
		StruSipData stData;
		bzero(&stData, sizeof(StruSipData));
		memcpy(&stData.stDialog, &m_stInviteDlgKey, sizeof(m_stInviteDlgKey));

		stData.stDialog.iCSeq = 0;
		stData.eDataType = eSIP_DATA_REQUEST;
		stData.iContentLength = 0;
		stData.eContentType = eSIP_CONTENT_MANSCDP_XML;
		stData.eMethod  = eSIP_METHOD_MESSAGE;

		GS_SNPRINTF((char*)stData.vContent,SIP_MAX_CONTENT_LEN,
			"<?xml version=\"1.0\"?>\r\n"
			"<Notify>\r\n"
			"<CmdType>Keepalive</CmdType>\r\n"
			"<SN>%d</SN>\r\n"
			"<DeviceID>%s</DeviceID>\r\n"
			"<Status>OK</Status>\r\n"
			"</Notify>\r\n"
			, m_iKpSN++, /*m_strUsername.c_str()*/  m_strDevId.c_str() );
		stData.iContentLength = strlen((char*)stData.vContent);
		strncpy(stData.stDialog.szSubject, m_csSubject.Encode().c_str(), SIP_MAX_SUBJECT_STRING );
		StruSipData stRes;
		bzero(&stRes, sizeof(stRes));
		if( m_pSipSrv->SessionSendSipData(this,&stData, &stRes, 700, NULL) )
		{
			MY_LOG_ERROR(H_LOG, _GSTX("CSipChannel(%u). 发送Keepalive命令失败.\n"), 
				m_iAutoID );
		}
		else
		{
			MY_LOG_DEBUG(H_LOG, _GSTX("CSipChannel(%u)  发送Keepalive命令 %d.\n"), 
				m_iAutoID, stRes.stResponseResult.bOk );
			if( stRes.stResponseResult.bOk )
			{
				m_iKeepalivePlugs = 0;
				if( m_bTransProxy  || !m_pRtpReader )
				{

				m_pParent->KeepStreamAlive();
				}
			}

		}
	}	
	
}


EnumErrno CSipChannel::SendAck(void)
{
	if( m_hCliCnner != INVALID_SIP_CLIENT_CNNER )
	{
		StruSipData stData;
		bzero(&stData, sizeof(StruSipData));	
		memcpy(&stData.stDialog, &m_stInviteDlgKey, sizeof(m_stInviteDlgKey));
		stData.eDataType = eSIP_DATA_REQUEST;
		stData.iContentLength = 0;
		stData.eContentType = eSIP_CONTENT_NONE;
		stData.eMethod  = eSIP_METHOD_ACK;
		strncpy(stData.stDialog.szSubject, m_csSubject.Encode().c_str(), SIP_MAX_SUBJECT_STRING );

		if( !m_pSipSrv->SessionSendSipData(this,&stData, NULL, 0, NULL) )
		{
			MSLEEP(5);
			return eERRNO_SUCCESS;
		}
	}	
	return eERRNO_SYS_EINVALID;
}

EnumErrno CSipChannel::SendMANSRTSP( const StruGSPCmdCtrl &stCtrl )
{
	CMansRtspPaser csParser;
	memcpy( &csParser.m_stGspCtrl, &stCtrl, sizeof(stCtrl));
	csParser.m_fSpeed = m_fSpeed;

	UINT32 iCSeq =  (UINT32)AtomicInterInc(m_iCTagSequence);
	CGSString strSip = csParser.Encode(iCSeq);
	if( !strSip.empty() )
	{
		StruSipData stData;
		bzero(&stData, sizeof(stData));		
		memcpy(&stData.stDialog, &m_stInviteDlgKey, sizeof(m_stInviteDlgKey));
		stData.stDialog.iCSeq = iCSeq;
		stData.eDataType = eSIP_DATA_REQUEST;

		stData.iContentLength = strSip.length();
		stData.eContentType = eSIP_CONTENT_MANSRTSP;
		strncpy(stData.vContent, strSip.c_str(),SIP_MAX_CONTENT_LEN );
		stData.eMethod  = eSIP_METHOD_INFO;
		strncpy(stData.stDialog.szSubject, m_csSubject.Encode().c_str(), SIP_MAX_SUBJECT_STRING );
		StruSipData stRes;
		bzero(&stRes, sizeof(stRes));
		if( !m_pSipSrv->SessionSendSipData(this,&stData, &stRes /*NULL*/, 10000, NULL) )
		{
			return eERRNO_SUCCESS;
		}
	}
	return eERRNO_SUCCESS;

	return eERRNO_SYS_EINVALID;
}

EnumErrno CSipChannel::SendBye(void)
{
	if( m_bWouldSendByte )
	{
		m_bWouldSendByte = FALSE;
		if( m_hCliCnner != INVALID_SIP_CLIENT_CNNER  )
		{
			StruSipData stData;
			bzero(&stData, sizeof(StruSipData));
			memcpy(&stData.stDialog, &m_stInviteDlgKey, sizeof(m_stInviteDlgKey));
		
			stData.eDataType = eSIP_DATA_REQUEST;
			stData.iContentLength = 0;
			stData.eContentType = eSIP_CONTENT_NONE;
			stData.eMethod  = eSIP_METHOD_BYE;
			strncpy(stData.stDialog.szSubject, m_csSubject.Encode().c_str(), SIP_MAX_SUBJECT_STRING );
			StruSipData stRes;
			bzero(&stRes, sizeof(stRes));
			if( !m_pSipSrv->SessionSendSipData(this,&stData, &stRes, 1000, NULL) )
			{
				return eERRNO_SUCCESS;
			}
		}	
		return eERRNO_SYS_EINVALID;
	}
	return eERRNO_SUCCESS;
}

void CSipChannel::ResponseBadRequest(StruSipData *pData,int iSipErrno, const char *czError  )
{
	if( m_hCliCnner != INVALID_SIP_CLIENT_CNNER  )
	{
		//请求消息
		pData->eDataType = eSIP_DATA_RESPONSE;
		pData->iContentLength = 0;
		pData->eContentType = eSIP_CONTENT_NONE;
		pData->stResponseResult.bOk = 0;
		pData->stResponseResult.iSipErrno = iSipErrno;
		
		if( !m_pSipSrv->SessionSendSipData(this,pData, NULL, 0, NULL) )
		{
			return;
		}
	}

	MY_LOG_DEBUG(H_LOG, _GSTX("CSipChannel(%u) Accert. 没法回复命令\n"), 
		m_iAutoID );
	m_pParent->OnProChannelSignalEvent(CClientChannel::eSIG_ASSERT);
	
}

void CSipChannel::ResponseSimple( StruSipData *pData,int iSipErrno, const char *czInfo  )
{
	if( m_hCliCnner != INVALID_SIP_CLIENT_CNNER  )
	{
		//请求消息
		pData->eDataType = eSIP_DATA_RESPONSE;
		pData->iContentLength = 0;
		pData->eContentType = eSIP_CONTENT_NONE;
		pData->stResponseResult.bOk = iSipErrno==200;
		pData->stResponseResult.iSipErrno = iSipErrno;
		if( !m_pSipSrv->SessionSendSipData(this,pData, NULL, 0, NULL) )
		{
			return;
		}
	}

	MY_LOG_DEBUG(H_LOG, _GSTX("CSipChannel(%u) Accert. 没法回复命令\n"), 
		m_iAutoID );
	m_pParent->OnProChannelSignalEvent(CClientChannel::eSIG_ASSERT);
}

EnumErrno CSipChannel::ConvertFrameInfo(CFrameCache *pFrame,  StruFrameInfo &stInfo )
{
	memcpy( &stInfo,  &pFrame->m_stFrameInfo, sizeof(stInfo));
	if( m_eOutputPkgType == eSTREAM_PKG_28181PS )
	{
		stInfo.bKey = TRUE;
		stInfo.eMediaType = GS_MEDIA_TYPE_VIDEO;
		stInfo.iChnNo = 0;		
	}
	else if( m_eOutputPkgType == eSTREAM_PKG_Standard )
	{
		stInfo.bKey = TRUE;
		stInfo.eMediaType = GS_MEDIA_TYPE_VIDEO;
		stInfo.iChnNo = 0;		
	}
	return eERRNO_SUCCESS;
	
}

void CSipChannel::HandleRtpStreamFrame(CFrameCache *pFrame)
{

	GS_ASSERT(m_pPkgCvt);

	StruFrameInfo stInfo, stInfo2;
	memcpy( &stInfo2,  &pFrame->m_stFrameInfo, sizeof(stInfo2));
	ConvertFrameInfo(pFrame, stInfo);
	memcpy( &pFrame->m_stFrameInfo, &stInfo, sizeof(stInfo) );
	
	EnumErrno eRet = m_pPkgCvt->Conver(pFrame, FALSE);
	
	


	if( eRet )
	{
		GS_ASSERT(0);
		m_pParent->m_stChannelInfo.iLostFrames++; 
		MY_LOG_FATAL(H_LOG, _GSTX("CSipChannel(%u) Assert. 码流封装失败.\n"), 
			m_iAutoID );
		m_pParent->OnProChannelSignalEvent(CClientChannel::eSIG_ASSERT);		
	}	

	CFrameCache *pCvtFrame;
	while( (pCvtFrame = m_pPkgCvt->Get() ))
	{		
		if( pCvtFrame->m_stFrameInfo.eMediaType == GS_MEDIA_TYPE_SYSHEADER )
		{
			//
			int iBreak = 0;
		}
		if( pCvtFrame->m_stFrameInfo.eMediaType != GS_MEDIA_TYPE_VIDEO &&
			(m_eInputPkgType == eSTREAM_PKG_Standard || 
			(m_pParent->m_eTranModel == GSP_TRAN_REPLAY && (m_fSpeed<0.99 || m_fSpeed>1.01 ) ) ))
		{
			//不要音频
			pCvtFrame->UnrefObject();
			continue;
		}
		INT64 iCurPos = -1;
		if( m_pParent->m_eTranModel == GSP_TRAN_RTPLAY )
		{
			//实时流， 计算时戳
			UINT32 iCur = (UINT32) time(NULL);
			StruGSFrameHeader *pH = (StruGSFrameHeader *) pCvtFrame->GetBuffer().m_bBuffer;
			pH->iTimeStamp = iCur;
			pCvtFrame->m_stFrameInfo.iTimestamp = DoGetTickCount();
		}
		else if( m_iBeginTimestamp >= 0 && m_iFileLengthTv>0 )
		{
			//回放或录像
			if( pCvtFrame->m_stFrameInfo.eMediaType == GS_MEDIA_TYPE_VIDEO )
			{

				if( m_eInputPkgType == eSTREAM_PKG_28181PS )
				{
					if( m_iLastPTS == MAX_UINT64 )
					{					
						m_iTimestamp = 0;
					}
					else
					{
						m_iTimestamp += 40; // (UINT32) IntervalOfPTS(m_iLastPTS, stInfo2.iTimestamp);
					}
					m_iLastPTS = stInfo2.iTimestamp;
				}
				else
				{
					m_iTimestamp += 40;	
				}
			}
			INT64 iCurTimestamp = m_iBeginTimestamp+m_iTimestamp/1000;

			StruGSFrameHeader *pH = (StruGSFrameHeader *) pCvtFrame->GetBuffer().m_bBuffer;
			pH->iTimeStamp = (UINT32) iCurTimestamp;
			pCvtFrame->m_stFrameInfo.iTimestamp = m_iTimestamp;

			iCurPos = (iCurTimestamp-m_iBeginTimestamp)*10000/m_iFileLengthTv;

		}
		m_pParent->HandleStream(pCvtFrame, FALSE);
		pCvtFrame->UnrefObject();

		//发送进度
		if( iCurPos != -1  && iCurPos != m_iLastPos )
		{
				
			MY_LOG_INFO(H_LOG,_GSTX("CSipChannel(%u) 进度: 文件长(%d), 相对间戳(%lu)  进度(%d)\n"), 
				m_iAutoID,(int)m_iFileLengthTv,  (unsigned long) m_iTimestamp, (int) iCurPos );

			//非实时流 发送进度			
			StruPlayStatus stStatus;         
			bzero(&stStatus,  sizeof(stStatus));
			stStatus.iSpeed = 0;
			stStatus.iPosition = (UINT16)iCurPos;
		//	m_pParent->HandlePlayStatus(stStatus);		
			m_iLastPos = stStatus.iPosition;
		}
	}
}