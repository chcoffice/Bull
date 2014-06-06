#include "ClientSection.h"
#include "ClientChannel.h"
#include "GSPProDebug.h"
#include "StreamPackConvert.h"

#ifdef ENABLE_SIP_MODULE
#include "./SIP/ProSipChannel.h"
#endif

#include "RTP/RtpStru.h"

using namespace GSP;

static void _FreeChannelHandle(CIClientChannel *pChannel )
{
	pChannel->Release();
}

CClientSection::CClientSection(void)
:CGSPObject()
,CIClientSection()
{
	m_pLog = NULL;
	m_pClientCallbackParam = NULL;
	m_fnClientCallback = NULL;



	m_bUseRTP = FALSE;
	m_iRtpUDPPort = 0;

	m_strLogPath = GSGetApplicationPath();
	m_strLogPath += "log/GSPClient";
	GSPathParser(m_strLogPath);

	m_lvLogConsole =  (CLog::LV_FATAL | CLog::LV_ERROR | CLog::LV_WARN | CLog::LV_NOTICE);
	m_lvLogFile =   (CLog::LV_FATAL | CLog::LV_ERROR | CLog::LV_WARN | CLog::LV_NOTICE);

	m_strSipUdpBindIp.clear();
	m_iSipUdpPort = 0;
	m_strSipServerName.clear();

	m_iRtpUdpPortBegin = 0;
	m_iRtpUdpPortEnd = 0;
#ifdef ENABLE_SIP_MODULE
	m_pSipSrv = NULL;
#endif

	m_bGspEnableRtp = FALSE;

	m_eGspStreamTransMode = -1;
}

CClientSection::~CClientSection(void)
{

	
#ifdef ENABLE_SIP_MODULE
	if( m_pSipSrv )
	{
		m_pSipSrv->Stop();
		m_pSipSrv->Uninit();
		delete m_pSipSrv;
		m_pSipSrv = NULL;
	}
#endif
	m_pLog->UnrefObject();
	m_pLog = NULL;
}


BOOL CClientSection::EnableAutoConnect(BOOL bEnable)
{
	GS_ASSERT(0);
	return FALSE;
}


BOOL CClientSection::SetReconnectInterval(UINT iSecs)
{
	GS_ASSERT(0);
	return FALSE;
}


BOOL CClientSection::SetReconnectTryMax(UINT iCounts)
{
	GS_ASSERT(0);
	return FALSE;
}

INT CClientSection::FetchClientChannel( FuncPtrFetchClientChannelCallback fnCallback,
					   void *pUserParam )
{
	m_csChannelMutex.LockReader();
	CList::CIterator<CIClientChannel *> csIt;
	for( csIt = m_csChannelList.First<CIClientChannel *>(); csIt.IsOk(); csIt.Next() )
	{
			fnCallback(csIt.Data(), pUserParam);
	}
	m_csChannelMutex.UnlockReader();
	return -1;
}

BOOL CClientSection::Release(void)
{


CList stTemp;
	m_csChannelMutex.LockWrite();
	m_csChannelList.Swap(stTemp);
	m_csChannelMutex.UnlockWrite();

	stTemp.SetFreeCallback((FuncPtrFree)_FreeChannelHandle);
CIClientChannel *p = NULL;
	while(eERRNO_SUCCESS== stTemp.RemoveFront( (void**)&p) )
	{	
		GS_ASSERT(p);
		SendEvent(p,GSP_EVT_CLI_DESTROY, NULL, 0);	
		_FreeChannelHandle(p);
		p = NULL;
	}
#ifdef ENABLE_SIP_MODULE
	if( m_pSipSrv )
	{
		m_pSipSrv->Stop();
		m_pSipSrv->Uninit();
	}
#endif

	delete this;

	GSPModuleUnint();
	return TRUE;
}

void CClientSection::InitLog( const char *czPathName, 
					 INT lvConsole, INT lvFile)
{

	if( m_pLog )
	{
		m_pLog->SetLogPath(czPathName);
		m_pLog->SetLogLevel(CLog::DIR_CONSOLE, (CLog::EnumLogLevel)lvConsole);
		m_pLog->SetLogLevel(CLog::DIR_FILE, (CLog::EnumLogLevel)lvFile);	
	}
	m_strLogPath = czPathName;
	m_lvLogConsole = lvConsole;
	m_lvLogFile = lvFile;
}


BOOL CClientSection::Init( const char *szIniFilename )
{
	CGSString strTemp;

	m_pLog = CLog::Create();

	GS_ASSERT_RET_VAL(m_pLog, FALSE);

	m_pLog->SetLogPath(m_strLogPath.c_str() );
	m_pLog->SetLogLevel(CLog::DIR_CONSOLE, (CLog::EnumLogLevel )m_lvLogConsole);
	m_pLog->SetLogLevel(CLog::DIR_FILE, (CLog::EnumLogLevel )m_lvLogFile);		
	


	std::string strAppPath = GSGetApplicationPath();
	strTemp = strAppPath;
	strTemp += "LogGSPClient";

	if( szIniFilename )
	{	
		strTemp = szIniFilename;
	}
	else
	{
		strTemp = strAppPath;
		strTemp += "GSPClientConf.ini";		
	}

	CGSString strInitFilename = strTemp;

	if( m_csConfig.LoadFile( (char*) strTemp.c_str()) )
	{
		if( m_eGspStreamTransMode==-1)
		{
			m_eGspStreamTransMode = m_csConfig.GetProperty("GSP","StreamTranMode", 
				GSP_STREAM_TRANS_MODE_MULTI_TCP );
		}
		//SIP 端口配置
		m_iSipUdpPort = m_csConfig.GetProperty("SIP.NET","SipUdpPort", 
			m_iSipUdpPort );
		m_strSipUdpBindIp = m_csConfig.GetProperty("SIP.NET", "SipUdpBindIP", "" );

		m_iRtpUdpPortBegin = m_csConfig.GetProperty("SIP.NET", "RtpUdpPortBegin", 
			m_iRtpUdpPortBegin );
		m_iRtpUdpPortEnd = m_csConfig.GetProperty("SIP.NET", "RtpUdpPortEnd",
			m_iRtpUdpPortBegin );

		m_strSipServerName = m_csConfig.GetProperty("SIP.NET", "ServerName", "" );
		if( m_strSipServerName=="")
		{
			m_strSipServerName.clear();
		}


		//设置日志输出等级
		int iValue = 0;
		if( m_csConfig.GetProperty("DebugConsole", "FATAL", 1) )
		{
			iValue |= CLog::LV_FATAL;
		}
		if( m_csConfig.GetProperty("DebugConsole", "ERROR", 1) )
		{
			iValue |= CLog::LV_ERROR;
		}
		if( m_csConfig.GetProperty("DebugConsole", "WARN",1) )
		{
			iValue |= CLog::LV_WARN;
		}
		if( m_csConfig.GetProperty("DebugConsole", "DEBUG",1) )
		{
			iValue |= CLog::LV_DEBUG;
		}
		if( m_csConfig.GetProperty("DebugConsole", "INFO",1) )
		{
			iValue |= CLog::LV_INFO;
		}

		if( m_csConfig.GetProperty("DebugConsole", "NOTICE",1) )
		{
			iValue |= CLog::LV_NOTICE;
		}

		CLog::EnumLogLevel iLogLevelConsole = (CLog::EnumLogLevel)iValue;

		iValue = 0;
		if( m_csConfig.GetProperty("DebugFile", "FATAL", 1) )
		{
			iValue |= CLog::LV_FATAL;
		}
		if( m_csConfig.GetProperty("DebugFile", "ERROR", 1) )
		{
			iValue |= CLog::LV_ERROR;
		}
		if( m_csConfig.GetProperty("DebugFile", "WARN",1) )
		{
			iValue |= CLog::LV_WARN;
		}
		if( m_csConfig.GetProperty("DebugFile", "DEBUG",1) )
		{
			iValue |= CLog::LV_DEBUG;
		}
		if( m_csConfig.GetProperty("DebugFile", "INFO",1) )
		{
			iValue |= CLog::LV_INFO;
		}
		if( m_csConfig.GetProperty("DebugFile", "NOTICE",1) )
		{
			iValue |= CLog::LV_NOTICE;
		}
		CLog::EnumLogLevel iLogLevelFile = (CLog::EnumLogLevel)iValue;

		m_pLog->SetLogLevel(CLog::DIR_CONSOLE,iLogLevelConsole);
		m_pLog->SetLogLevel(CLog::DIR_FILE, iLogLevelFile);

		if( m_csConfig.GetProperty("GSP.NET", "EnableRTP",0) )
		{
			m_bUseRTP = TRUE;
		}
		else
		{
			m_bUseRTP = FALSE;
		}

		m_iRtpUDPPort =  m_csConfig.GetProperty("GSP.NET", "RTPPort", 0);		
		
	}
	else
	{
		MY_LOG_ERROR(m_pLog, _GSTX("GSP 客户端 协议栈加载配置文件 '%s' 失败.\n"),
			strTemp.c_str() );
	} 

	m_bGspEnableRtp = m_bUseRTP;
	
	
	MY_LOG_NOTICE(m_pLog, _GSTX("GSP 客户端 使用UDP 传输:(%s).\n"),
		m_bGspEnableRtp ? "Enable" : "Disable");

	COSSocket::LoadManualRouteTable(strInitFilename);

#ifdef ENABLE_SIP_MODULE
	m_pSipSrv = new SIP::CProSipClientService(this);
	if( !m_pSipSrv )
	{
		MY_LOG_NOTICE(m_pLog, _GSTX("SIP 客户端建立 CProSipClientService 失败.\n"),);
		GS_ASSERT(0);
		return FALSE;
	}
	if( m_pSipSrv->Init(m_csConfig))
	{
		MY_LOG_FATAL(g_pLog, _GSTX("启动RTSP协议栈，SipService 初始化失败.\n"));
		delete m_pSipSrv;
		m_pSipSrv =NULL;
		return FALSE;
	}
	if( m_pSipSrv->Start() )
	{
		MY_LOG_FATAL(g_pLog, _GSTX("启动RTSP协议栈，SipService 启动失败.\n"));
		m_pSipSrv->Uninit();
		delete m_pSipSrv;
		m_pSipSrv =NULL;
		return FALSE;
	}	
	MY_LOG_NOTICE(g_pLog, _GSTX("启动SIP协议栈 成功!!\n"));
#endif
	return TRUE;

}

void CClientSection::SetGspStreamTransMode(int eGspStreamTransMode )
{
	if( eGspStreamTransMode>=0 && eGspStreamTransMode<3 )
	{
		m_eGspStreamTransMode = eGspStreamTransMode;
	}
	else
	{
		GS_ASSERT(0);
	}
}


CIClientChannel *CClientSection::CreateChannel(void)
{
	CIClientChannel *p =  new CClientChannel(this);
	if( p )
	{
		CGSWRMutexAutoWrite wlocker(&m_csChannelMutex);
		if( eERRNO_SUCCESS != m_csChannelList.AddTail(p) )
		{
			p->Release();
			p = NULL;
		}
	}
	GS_ASSERT(NULL != p);
	return p;
}

void CClientSection::OnClientChannelReleaseEvent( CIClientChannel *pChannel)
{
	CGSWRMutexAutoWrite wlocker(&m_csChannelMutex);
	m_csChannelList.Remove(pChannel);
}

BOOL CClientSection::OnTransRecvCmdData(const char *czProtocol,const void *pData, int iDataSize )
{
	EnumProtocol ePro = GetProtocolType(czProtocol);
#ifdef ENABLE_SIP_MODULE
	GS_ASSERT_RET_VAL(iDataSize==sizeof(StruSipData), FALSE);
	if( m_pSipSrv )
	{
		if( eERRNO_SUCCESS ==m_pSipSrv->OnTransRecvData((const StruSipData*)pData) )
		{
			return TRUE;
		}
		return FALSE;
	}
#endif
	GS_ASSERT(0);
	return FALSE;
}


void CClientSection::SetEventListener( GSPClientEventFunctionPtr fnCallback, 
							  void *pUserParam)
{
	m_pClientCallbackParam = pUserParam;
	m_fnClientCallback = fnCallback;
}


GSP::CIClientSection *GSP::CreateGSPClientSectionInterface(void)
{     
	GSPModuleInit();
	GSP::CClientSection *pRet = new GSP::CClientSection();
	if( !pRet )
	{
		GSPModuleUnint();
	}
	return pRet;
}

#ifdef ENABLE_SIP_MODULE

//SIP 协议栈回调
//连接
EnumSipErrorCode CClientSection::SipClientConnectCallback( SipServiceHandle hService,
													  SipSessionHandle hNewSession )
{
	CClientSection *pSrv = (CClientSection *)SipService_GetUserData(hService);
	return pSrv->OnSipClientConnectEvent(hNewSession);
}

//断开
void CClientSection::SipClientDisconnectCallback(SipServiceHandle hService, SipSessionHandle hSession)
{
	CClientSection *pSrv = (CClientSection *)SipService_GetUserData(hService);
	pSrv->OnSipClientDisconnectEvent(hSession);
}

//收包
void   CClientSection::SipSIPPacketCallback(SipServiceHandle hService, 
										SipSessionHandle hSession,
										StruSipData *pData)
{
	CClientSection *pSrv = (CClientSection *)SipService_GetUserData(hService);
	pSrv->OnSipPacketEvent(hSession, pData);
}


EnumSipErrorCode CClientSection::OnSipClientConnectEvent(SipSessionHandle hNewSession )
{
	//连接
	//为什么会有注册
	GS_ASSERT(0);
	return eSIP_RET_E_NMEN;

}

void  CClientSection::OnSipClientDisconnectEvent(SipSessionHandle hSession)
{
	SIP::CSipChannel *pSession = (SIP::CSipChannel *)SipSession_GetUserData(hSession);
	GS_ASSERT_RET(pSession);
	pSession->OnDisconnectEvent();
}


void CClientSection::OnSipPacketEvent(SipSessionHandle hSession, StruSipData *pData)
{
	
	SIP::CSipChannel *pSession = (SIP::CSipChannel *)SipSession_GetUserData(hSession);
	GS_ASSERT(pSession);
	if( !pSession )
	{
		if( pData->eDataType == eSIP_DATA_REQUEST )
		{
			//请求消息
			StruSipData stData = *pData;
			stData.eDataType = eSIP_DATA_RESPONSE;
			stData.iContentLength = 0;
			stData.eContentType = eSIP_CONTENT_NONE;
			stData.stResponseResult.bOk = 0;
			stData.stResponseResult.iSipErrno = 402;
			SipSession_SendMessage(hSession, &stData, NULL, 0, NULL);
		}
		MY_LOG_DEBUG(g_pLog, _GSTX("收到没有会话管理者的SIP 数据包!!\n"));
		return;
	}

	pSession->HandleSipData(pData);

}

#endif



/*
*********************************************************************
*
*@brief : CIPkgConverter
*
*********************************************************************
*/
namespace GSP
{

	class  CPkgConverter : public CIPkgConverter
	{
	private :
		CStreamPackConvert *m_pConverter;
		CMediaInfo m_csMedia;
		StruPkg m_stPkg;
		CFrameCache *m_curFrame;
	public :
		CPkgConverter(CStreamPackConvert *p, const CMediaInfo &csMedia) : CIPkgConverter(),
			m_pConverter(p)
		{
			m_csMedia = csMedia;
			GS_ASSERT(m_pConverter);
			m_curFrame = NULL;
			bzero(&m_stPkg,sizeof(m_stPkg));
		}
		virtual ~CPkgConverter(void)
		{
			SAFE_DELETE_OBJECT(&m_pConverter);
			SAFE_DESTROY_REFOBJECT(&m_curFrame);
		}

		virtual BOOL InputData(const void *pData, long iSize);

		//返回当前数据, 返回NULL 表示读取完成
		virtual const StruPkg *GetData(void);

		//下一数据
		virtual void  NextData(void);


		static CPkgConverter *Create( EnumGSCodeID eInputDataCodeID,
			EnumPkgType ePkgType);

	};


	GSP::CIPkgConverter *CreateGSPPkgConverter( EnumGSCodeID eInputDataCodeID,
		EnumPkgType ePkgType  )
	{
		return CPkgConverter::Create(eInputDataCodeID,ePkgType);
	}
} //end namespace GSP



CPkgConverter * CPkgConverter::Create( EnumGSCodeID eInputDataCodeID,
									  EnumPkgType ePkgType)
{
	EnumStreamPackingType eSrcPktType;
		EnumStreamPackingType eDestPktType = eSTREAM_PKG_NONE;
	EnumGSCodeID eDestCodeID = GS_CODEID_NONE;

		eSrcPktType = CMediaInfo::GetStreamPkt4GsCodeId(eInputDataCodeID);
	if( eSrcPktType == eSTREAM_PKG_NONE )
	{
		return false;
	}

	switch(ePkgType)
	{
	case ePkgType_PS :
		eDestPktType = eSTREAM_PKG_28181PS;
		eDestCodeID = GS_CODEID_PS;
		if(  !GsCodeIDTestPT_PS(eInputDataCodeID) )
		{
			return FALSE;
		}
	break;
	case ePkgType_H264 :
		eDestPktType = eSTREAM_PKG_Standard;
		eDestCodeID = GS_CODEID_ST_H264;
		if(  !GsCodeIDTestPT_H264(eInputDataCodeID) )
		{
			return FALSE;
		}
	break;
	case ePkgType_MP4 :
		eDestPktType = eSTREAM_PKG_Standard;
		eDestCodeID = GS_CODEID_ST_MP4;
		if(  !GsCodeIDTestPT_MP4(eInputDataCodeID) )
		{
			return FALSE;
		}
		break;	
	}
	if( eDestPktType == eSTREAM_PKG_NONE )
	{
		return FALSE;
	}

	CMediaInfo csMedia;
	StruGSMediaDescri desc;
	bzero(&desc,sizeof(desc));

	desc.eMediaType = GS_MEDIA_TYPE_VIDEO;
	desc.unDescri.struVideo.eCodeID = eInputDataCodeID;

	csMedia.AddChannel(&desc, 0, NULL );

	desc.eMediaType = GS_MEDIA_TYPE_AUDIO;
	desc.unDescri.struVideo.eCodeID = eInputDataCodeID;
	csMedia.AddChannel(&desc, 1, NULL );


	CStreamPackConvert *p = CStreamPackConvert::Make(TRUE, FALSE, 
		eSrcPktType,eDestPktType, csMedia, FALSE );

	if( !p )
	{
		return FALSE;
	}
	
	CPkgConverter *ret = new CPkgConverter(p, csMedia);
	if( ret )
	{
		return ret;
	}
	delete p;
	return NULL;
}


BOOL CPkgConverter::InputData(const void *pData, long iSize)
{
	

	CFrameCache *p = p->Create(iSize+1024);
	if( !p )
	{
		return FALSE;
	}

	p->m_stFrameInfo.iChnNo = 0;
	p->m_stFrameInfo.bKey = TRUE;
	p->m_stFrameInfo.eMediaType = GS_MEDIA_TYPE_VIDEO;
	p->m_stFrameInfo.iTimestamp = (unsigned long ) time(NULL);

	if( p->GetBuffer().SetData((const BYTE*)pData, iSize ) )
	{
		SAFE_DESTROY_REFOBJECT(&p);
		return FALSE;
	}

	if( m_pConverter->Conver(p,FALSE) )
	{
		SAFE_DESTROY_REFOBJECT(&p);
		return FALSE;
	}
	SAFE_DESTROY_REFOBJECT(&p);
	return TRUE;
}


const CIPkgConverter::StruPkg *CPkgConverter::GetData(void)
{
	if( m_curFrame )
	{
		return &m_stPkg;
	}
	m_curFrame = m_pConverter->Get();
	if( !m_curFrame )
	{
		return NULL;
	}
	m_stPkg.eMediaType = m_curFrame->m_stFrameInfo.eMediaType;
	m_stPkg.isKey = m_curFrame->m_stFrameInfo.bKey;
	m_stPkg.iTimestamp =(unsigned long) m_curFrame->m_stFrameInfo.iTimestamp;
	m_stPkg.pData = (void *)m_curFrame->GetBuffer().GetData();
	m_stPkg.iDataSize = m_curFrame->GetBuffer().GetDataSize();
	return &m_stPkg;

}


void  CPkgConverter::NextData(void)
{
	SAFE_DESTROY_REFOBJECT(&m_curFrame);
}