#include "ClientChannel.h"
#include "ClientSection.h"
#include "IProChannel.h"
#include "GSPProDebug.h"

#include "GSPMemory.h"

#ifdef ENABLE_GSP_MODULE
#include "./GSP/ProGspChannel.h"
#endif

#ifdef ENABLE_RTSP_MODULE
//#include "./RTSP/ProRtspChannel.h"
#endif

#ifdef ENABLE_SIP_MODULE
#include "./SIP/ProSipChannel.h"
#endif


#ifdef GSPCPAT
#include "Frame.h"
#endif

using namespace GSP;

GSAtomicInter CClientChannel::s_iAutoIDSequence = 0;

#define TIMER_ID_KEEEPALIVE_STREAM  3

#define TASK_STREAM     ((void*)1)
#define TASK_FINISH		((void*)2)
#define TASK_CLOSE		((void*)3)
#define TASK_STATUS		((void*)4)
#define TASK_ASSERT		((void*)5)
#define TASK_CLEAN_CACHE 	((void*)6) //清除缓冲


#define MAX_CACHE_SIZE		MBYTES*6
#define MAX_CACHE_MIN_SIZE  MBYTES*3


static void _FreeStreamCacheMember(CFrameCache *p)
{
	SAFE_DESTROY_REFOBJECT(&p);

}

static void _FreeStatusCacheMember( StruPlayStatus *p)
{
	CMemoryPool::Free(p);
}

//static GSAtomicInter s_iObjectCounts = 0;

CClientChannel::CClientChannel(CClientSection *pSestion)
:CGSPObject()
,CIClientChannel()
,m_iAutoID((UINT32)AtomicInterInc(s_iAutoIDSequence))
,m_pSection(pSestion)
,m_csStreameQueue(5000, NULL)
,m_csStatusQueue(200, NULL)
{
	m_bStep = FALSE;
	m_bAcceptEvent = TRUE;
	m_iDebugListTv = 0;
	m_stChannelInfo.iLostFrames  = 0;


	m_eStatus = CIClientChannel::ST_INIT;
	m_pProChannel = NULL;
	m_eProtocol = ePRO_UNK;
	m_strURI = "";
	m_pUserPrivateData = NULL;
	m_pLog = pSestion->m_pLog;

	m_iMaxKeepaliveStreamTimeouts = 35;
	m_iKeepaliveStreamPlugs = 0;

	m_iFlowCtrl = 0; //当前进行流控

	m_iOpenTimeouts = 5000;
	m_csAsyncOpenTaskPool; 	
	 m_eOpenErrno =eERRNO_EFATAL;
	 m_iCtrlAbilities = 0;


	m_csStreameQueue.SetFreeMemberFunction((FuncPtrFree)_FreeStreamCacheMember);
	m_csStatusQueue.SetFreeMemberFunction((FuncPtrFree)_FreeStatusCacheMember);
	m_csTaskPool.Init(this, 
		(FuncPtrObjThreadPoolEvent)&CClientChannel::OnTaskPoolEvent,1, FALSE);

	m_csAsyncOpenTaskPool.Init(this,(FuncPtrObjThreadPoolEvent)&CClientChannel::OnAsyncOpenTaskPoolEvent,1, FALSE );

	m_csKeepaliveStreamTimer.Init(this, 
		(FuncPtrTimerCallback)&CClientChannel::OnTimerEvent, TIMER_ID_KEEEPALIVE_STREAM,
		1000,
		FALSE);

	bzero(&m_stChannelInfo, sizeof(m_stChannelInfo));
	bzero(&m_vMediaType, sizeof(m_vMediaType));
	m_bClosing = FALSE;
	m_bOpenning = FALSE;
	

	m_bFinish = FALSE;

	m_iSpeed = 0;

	m_pSysHeaderCache = NULL;

	MY_LOG_DEBUG(m_pLog, _GSTX("CClientChannel(%u) Create.\n"), m_iAutoID);
//	AtomicInterInc(s_iObjectCounts);
}

BOOL CClientChannel::IsInit(void) 
{
	return m_csTaskPool.IsInit() && 
		   m_csAsyncOpenTaskPool.IsInit() && 
		   m_csKeepaliveStreamTimer.IsReady();
}

CClientChannel::~CClientChannel(void)
{
	GS_ASSERT(m_pProChannel==NULL);

	m_pUserPrivateData = NULL;

	SAFE_DESTROY_REFOBJECT(&m_pSysHeaderCache);
#if 0
	MY_LOG_NOTICE(m_pLog, _GSTX("CClientChannel(%u) Destory.Exist: %d\n"), m_iAutoID,
		AtomicInterDec(s_iObjectCounts) );
#else
	MY_LOG_DEBUG(m_pLog, _GSTX("CClientChannel(%u) Destory."), m_iAutoID );
#endif
}

CIClientSection *CClientChannel::GetSection(void) const
{
	return m_pSection;
}

INLINE INT CClientChannel::SendEvent( EnumGSPClientEventType eEvtType, void *pEventData ,  INT iEvtDataLen)
{
	if(  m_bAcceptEvent || eEvtType==GSP_EVT_CLI_SIP_SEND )
	{

		return m_pSection->SendEvent(this, eEvtType, pEventData, iEvtDataLen);	
	}
	return 0;
}

void CClientChannel::Release(void)
{
	m_bAcceptEvent = FALSE;
	m_csKeepaliveStreamTimer.Stop();
	INT iii = 0;
	m_csTaskPool.Uninit();

	m_csMutex.Lock();
	CIProChannel *pTemp = m_pProChannel;
	m_pProChannel = NULL;
	m_csMutex.Unlock();
	if( pTemp )
	{
		pTemp->DestoryBefore();
		delete pTemp;
		
	}
	m_csStreameQueue.Clear();
	m_csStatusQueue.Clear();


	m_pSection->OnClientChannelReleaseEvent(this);
	delete this;
}

CIClientChannel::EnumErrno CClientChannel::GetErrno(void) const
{
	return m_eErrno;
}

BOOL CClientChannel::SetURIOfSip( const char *czURI, const char *czSdp )
{
	CGSAutoMutex locker( &m_csMutex );

	m_iDebugListTv = 0;
	m_stChannelInfo.iLostFrames  = 0;

	GS_ASSERT(NULL!=czURI);

	if( m_pProChannel!=NULL)
	{
		return FALSE;
	}
	m_bAcceptEvent = TRUE;
	m_csMediaInfo.Clear();

	for( INT i = 0; i<ARRARY_SIZE(m_VSeq); i++)
	{
		m_VSeq[i] = -1;
	}

	m_bFinish = FALSE;
	m_bStep = FALSE;

	bzero(&m_stChannelInfo, sizeof(m_stChannelInfo));
	m_iKeepaliveStreamPlugs = 0;

	m_strURI ="";
	if( !m_csURI.Analyse(czURI) )
	{
		m_eErrno = CIClientChannel::ERRNO_EURI;
		return FALSE;
	}
	if( !GSStrUtil::EqualsIgnoreCase(m_csURI.GetScheme(), "sip"  ))
	{
		//不是SIP 协议 
		//重新定义SIP 端口
		StruUriAttr *aSip = m_csURI.GetAttr("sip");
		if( !aSip )
		{
			m_eErrno = CIClientChannel::ERRNO_EURI;
			return FALSE;
		}
		m_csURI.SetPortArgs(GSStrUtil::ToNumber<UINT>(aSip->szValue));
		m_csURI.SetScheme("sip");
	}
	
	if( czSdp )
	{
		m_csURI.AddAttr("sdp", czSdp);

	}
	m_strURI = m_csURI.GetURI();

	MY_LOG_DEBUG(m_pLog, _GSTX("CClientChannel(%u) Bind URI:'%s'.\n"), m_iAutoID, czURI);
	m_eProtocol = GetProtocolType(m_csURI.GetScheme() );

	return m_eProtocol!=ePRO_UNK;

	return FALSE;
}

BOOL CClientChannel::SetURI( const char *czURI)
{
	CGSAutoMutex locker( &m_csMutex );

	m_iDebugListTv = 0;
	m_stChannelInfo.iLostFrames  = 0;
	m_bStep = FALSE;

	GS_ASSERT(NULL!=czURI);

	if( m_pProChannel!=NULL)
	{
		return FALSE;
	}
	m_bAcceptEvent = TRUE;
	m_csMediaInfo.Clear();

	for( INT i = 0; i<ARRARY_SIZE(m_VSeq); i++)
	{
		m_VSeq[i] = -1;
	}
	m_bFinish = FALSE;



	bzero(&m_stChannelInfo, sizeof(m_stChannelInfo));
	m_iKeepaliveStreamPlugs = 0;

	m_strURI ="";
	if( !m_csURI.Analyse(czURI) )
	{
		m_eErrno = CIClientChannel::ERRNO_EURI;
		return FALSE;
	}
	m_strURI = czURI;

	MY_LOG_DEBUG(m_pLog, _GSTX("CClientChannel(%u) Bind URI:'%s'.\n"), m_iAutoID, czURI);
	m_eProtocol = GetProtocolType(m_csURI.GetScheme() );
	
	return m_eProtocol!=ePRO_UNK;
}


const char *CClientChannel::GetURI( void ) const
{
	return m_strURI.c_str();
}

BOOL CClientChannel::AddRequestInfo(const StruGSMediaDescri *pInfo, INT iLen)
{
	CGSAutoMutex locker( &m_csMutex );
	m_csMediaInfo.AddChannel(pInfo, -1, NULL);
	return TRUE;
}

void CClientChannel::ClearRequestInfo( EnumGSMediaType eType  ) 
{
	CGSAutoMutex locker( &m_csMutex );
	m_csMediaInfo.Clear(eType);
}


void CClientChannel::OnAsyncOpenTaskPoolEvent( CObjThreadPool *pTkPool, void *pData )
{	
	CGSAutoMutex locker( &m_csMutex );	
	m_eOpenErrno = m_pProChannel->Open(m_csURI, m_iOpenTimeouts<1, abs(m_iOpenTimeouts) );
	m_condOpen.Signal();
}


BOOL CClientChannel::Open(INT iTransType, INT iTimeouts )
{
	if( CMemoryPool::IsNoMemory() )
	{
		MY_LOG_WARN(m_pLog, _GSTX("CClientChannel(%u) %s 打开失败。 内存缺乏.\n")
			,m_iAutoID);
		return FALSE;
	}
	CGSAutoMutex locker( &m_csMutex );	
	while( m_bClosing || m_bOpenning )
	{
		//正在关闭
		m_csMutex.Unlock();
		MSLEEP(10);
		m_csMutex.Lock();
	}

	if(m_pProChannel != NULL )
	{
		return TRUE;
	}
	m_strSdp.clear();
	m_bAcceptEvent = TRUE;
	m_bOpenning = TRUE;
	m_bStep = FALSE;
	m_iSpeed = 0;

	m_csStreameQueue.Clear();
	m_csStatusQueue.Clear();
	m_iWaitFrameSize = 0;
	m_iKeepaliveStreamPlugs = 0;
	m_iFlowCtrl = 0; //当前进行流控
	SAFE_DESTROY_REFOBJECT(&m_pSysHeaderCache);
	m_iWaitFrameSize = 0;
	bzero(&m_vMediaType, sizeof(m_vMediaType));

	switch( m_eProtocol )
	{
	case ePRO_GSP :
#ifdef ENABLE_GSP_MODULE
		m_pProChannel =  new CGspChannel(this);
#endif
		break;
	case ePRO_SIP :
#ifdef ENABLE_SIP_MODULE
		m_pProChannel =  new SIP::CSipChannel(this);
#endif
		break;
	case ePRO_RTSP :
		break;

	default :
		m_eErrno = CIClientChannel::ERRNO_ENPROTOCOL;
		return FALSE;
		break;
	}

	if( !m_pProChannel )
	{
		GS_ASSERT(0);
		m_eErrno = CIClientChannel::ERRNO_EURI;
		m_bAcceptEvent = FALSE;
		m_bOpenning = FALSE;
		return FALSE;
	}
	m_eTranModel = iTransType;
	m_eStatus = CIClientChannel::ST_INIT;
	m_csTaskPool.Enable();
	m_csAsyncOpenTaskPool.Enable();
	
//	BOOL bRet = m_pProChannel->Open(m_csURI, iTimeouts<1, abs(iTimeouts) ) == GSP::eERRNO_SUCCESS;
	BOOL bRet = FALSE;
	m_eOpenErrno = eERRNO_ENONE;
	m_iOpenTimeouts = iTimeouts;
	int iTvCnts = 0;
	if(m_csAsyncOpenTaskPool.RSUCCESS == m_csAsyncOpenTaskPool.Task( (void*)1)  )
	{
		do{
			m_condOpen.WaitTimeout(&m_csMutex, 1000);
			if( m_eOpenErrno == eERRNO_SUCCESS )
			{
				bRet = TRUE;
				break;
			}
		} while( m_eOpenErrno == eERRNO_ENONE && iTvCnts++<300 );

	}
	

	m_bOpenning = FALSE;

	if( !bRet )
	{
		
		m_csMutex.Unlock();
		Close();
		m_csMutex.Lock();
	}
	else
	{
		m_csKeepaliveStreamTimer.Start();
	}
	return bRet;

}


void CClientChannel::Close(void)
{
	m_bAcceptEvent = FALSE;
	m_eOpenErrno = eERRNO_EFATAL;
	m_csKeepaliveStreamTimer.Stop();
	m_csTaskPool.Disable();
	m_csAsyncOpenTaskPool.Disable();
	m_condOpen.Signal();
	MSLEEP(1);


	m_csMutex.Lock();

	while(  m_bClosing || m_bOpenning  )
	{
		m_csMutex.Unlock();
		MSLEEP(10);
		m_csMutex.Lock();
	}

	m_bClosing = TRUE;
	CIProChannel *pTemp = m_pProChannel;
	m_pProChannel = NULL;
	m_csMutex.Unlock();
	if( pTemp )
	{
		pTemp->DestoryBefore();
		delete pTemp;
	}
	m_csMutex.Lock();
	m_csStreameQueue.Clear();
	m_csStatusQueue.Clear();
	m_bClosing = FALSE;
	m_csMutex.Unlock();
	if( m_stChannelInfo.iLostFrames > 0 )
	{
		MY_LOG_WARN(m_pLog, _GSTX("CClientChannel(%u) %s 累计丢帧 (%lld).\n")
			,m_iAutoID,m_strURI.c_str(),  (long long ) m_stChannelInfo.iLostFrames);
		m_stChannelInfo.iLostFrames = 0;
	}


	//重置 状态
	m_bAcceptEvent = TRUE;
	m_iDebugListTv = 0;
	m_stChannelInfo.iLostFrames  = 0;
	m_bStep = FALSE;


	m_eStatus = CIClientChannel::ST_INIT;
	m_pProChannel = NULL;
	m_eProtocol = ePRO_UNK;
	m_strURI = "";
	m_iMaxKeepaliveStreamTimeouts = 35;
	m_iKeepaliveStreamPlugs = 0;
	m_iFlowCtrl = 0; //当前进行流控
	m_iOpenTimeouts = 5000;
	m_csAsyncOpenTaskPool; 	
	m_eOpenErrno =eERRNO_EFATAL;
	m_iCtrlAbilities = 0;
	bzero(&m_stChannelInfo, sizeof(m_stChannelInfo));
	MY_LOG_INFO(m_pLog, _GSTX("CClientChannel(%u) Close Reset.\n"), m_iAutoID);
	m_bClosing = FALSE;
	m_bOpenning = FALSE;
	bzero(m_vMediaType, sizeof(m_vMediaType));
	m_bFinish = FALSE;
	m_iSpeed = 0;
	m_iWaitFrameSize = 0;
}

CIClientChannel::EnumStatus CClientChannel::GetStatus(void) const
{
	return m_eStatus;
}

CIMediaInfo &CClientChannel::GetMediaInfo(void)
{
	return m_csMediaInfo;
}

const char *CClientChannel::GetSdp(void)
{

	m_csMutex.Lock();
	if( m_strSdp.empty() )
	{
		if( m_pProChannel && m_strSdp.empty() )
		{
			m_strSdp = m_pProChannel->GetSdp();
		}

		m_csMutex.Unlock();
	}
	return m_strSdp.empty() ? NULL : m_strSdp.c_str();
}

BOOL CClientChannel::CtrlOfManstrsp(const char *czMansrtsp, INT iTimeouts)
{
	CGSAutoMutex locker( &m_csMutex );

	if( !m_pProChannel )
	{
		//	GS_ASSERT(0);
		m_eErrno = CIClientChannel::ERRNO_EURI;
		return FALSE;
	}
	StruGSPCmdCtrl stCtrl;
	GSP::EnumErrno eErrno =  m_pProChannel->CtrlOfManstrsp(czMansrtsp, iTimeouts<1, abs(iTimeouts), stCtrl  );

	if( eErrno==eERRNO_SUCCESS )
	{
		switch( stCtrl.iCtrlID )
		{
		case GSP_CTRL_BSTEP :
		case GSP_CTRL_STEP :
			{
				m_eStatus = CIClientChannel::ST_PLAYING;
				m_bStep = TRUE;
			}
		break;
		case GSP_CTRL_SLOW:
		case GSP_CTRL_BSLOW :
		case GSP_CTRL_PLAY:
		case GSP_CTRL_FAST :
		case GSP_CTRL_BFAST :
			{
				m_eStatus = CIClientChannel::ST_PLAYING;
				m_bStep = FALSE;
			}
			break;
		
		case GSP_CTRL_PAUSE :
			{
				m_eStatus = CIClientChannel::ST_PAUSE;
				m_bStep = FALSE;
			}
			break;
		case GSP_CTRL_STOP :
			{
				m_bAcceptEvent = FALSE;		
				m_bStep = FALSE;
			}
			break;
		default :
			{
				m_bStep = FALSE;
			}
		break;
		}
		m_iKeepaliveStreamPlugs = 0;
		return TRUE;
	}
	m_eErrno = CIClientChannel::ERRNO_ENCTRL;	
	return FALSE;
}


static INLINE INT SpeedConvert(int iSpeed )
{
	switch(iSpeed )
	{
	case 0 : return 1;
	case 1 : return 2;
	case 2 : return 4;
	case 3 : return 8;
	case 4 : return 16;
	case 5 : return 32;
	default :
	break;
	}
	return 32;
}

BOOL CClientChannel::Ctrl(const StruGSPCmdCtrl &stCtrl, INT iTimeouts)
{
	CGSAutoMutex locker( &m_csMutex );
	StruGSPCmdCtrl stCtrTemp;
	memcpy(&stCtrTemp, &stCtrl, sizeof(stCtrl));
	m_iKeepaliveStreamPlugs = 0; //流检测
	if( !m_pProChannel )
	{
		//	GS_ASSERT(0);
		if( stCtrl.iCtrlID==GSP_CTRL_STOP )
		{
			return TRUE;
		}
		m_eErrno = CIClientChannel::ERRNO_EURI;
		return FALSE;
	}
	if( 0==(m_iCtrlAbilities&stCtrl.iCtrlID) )
	{
		if( stCtrl.iCtrlID==GSP_CTRL_STOP )
		{
			return TRUE;
		}
		m_eErrno = CIClientChannel::ERRNO_ENCTRL;
		return FALSE;
	}


	CIClientChannel::EnumStatus eSt = m_eStatus;


	switch(stCtrTemp.iCtrlID )
	{
	case GSP_CTRL_FAST :
		if( stCtrTemp.iArgs1 == 0 )
		{
			m_iSpeed++;		
		    if( m_iSpeed>=0 )
			{
				stCtrTemp.iArgs1 = SpeedConvert(m_iSpeed);
			}
			else
			{
				stCtrTemp.iArgs1 = SpeedConvert(-m_iSpeed);		
				stCtrTemp.iCtrlID = GSP_CTRL_SLOW;
			}
		}
		else
		{
			m_iSpeed = stCtrTemp.iArgs1;
		}
	break;
	case GSP_CTRL_SLOW :
		if( stCtrTemp.iArgs1 == 0 )
		{
			m_iSpeed--;
			if( m_iSpeed<1 )
			{
				stCtrTemp.iArgs1 = SpeedConvert(-m_iSpeed);
			}
			else 
			{
				stCtrTemp.iArgs1 = SpeedConvert(m_iSpeed);
				stCtrTemp.iCtrlID = GSP_CTRL_FAST;
			}
		}
		else
		{
			m_iSpeed = stCtrTemp.iArgs1;
		}
		break;
	}


	switch( stCtrTemp.iCtrlID )
	{
	case GSP_CTRL_SECTION :
	case GSP_CTRL_BSTEP :
	case GSP_CTRL_STEP :	
		{
			m_eStatus = CIClientChannel::ST_PLAYING;
			m_bStep = TRUE;
		}
		break;
	case GSP_CTRL_SLOW:
	case GSP_CTRL_BSLOW :	
	case GSP_CTRL_PLAY:
	case GSP_CTRL_FAST :
	case GSP_CTRL_BFAST :	
		{
			m_eStatus = CIClientChannel::ST_PLAYING;
			m_bStep = FALSE;
		}
		break;	
	case GSP_CTRL_PAUSE :
		{
			m_eStatus = CIClientChannel::ST_PAUSE;
			m_bStep = FALSE;
		}
		break;
	case GSP_CTRL_STOP :
		{
			m_bAcceptEvent = FALSE;			
			m_bStep = FALSE;
		}
		break;
	default :
		{
			m_bStep = FALSE;
		}
		break;
	}
	m_iKeepaliveStreamPlugs = 0;

	if( stCtrl.iCtrlID == GSP_CTRL_SETPOINT )
	{
		//清除缓冲区
		m_csTaskPool.Task(TASK_CLEAN_CACHE,TRUE);
		MSLEEP(1);
	}

	GSP::EnumErrno eErrno =  m_pProChannel->Ctrl(stCtrTemp, iTimeouts<1, abs(iTimeouts) );

	if( eErrno==eERRNO_SUCCESS  )
	{
		return TRUE;
	}
	m_eStatus = eSt;
	if( stCtrl.iCtrlID==GSP_CTRL_STOP )
	{
		return TRUE;
	}

	m_eErrno = CIClientChannel::ERRNO_ENCTRL;
	return FALSE;
}

void CClientChannel::RefreshMediaInfo(void)
{
INT iCnts;
	bzero(m_vMediaType, sizeof(m_vMediaType));
	iCnts = m_csMediaInfo.GetChannelNums();
const CIMediaInfo::StruMediaChannelInfo *p;
	for( INT i = 0; i<iCnts; i++ )
	{	
		p = m_csMediaInfo.GetChannel(i);
		if( p && p->iSubChannel<GSP_MAX_MEDIA_CHANNELS && p->iSubChannel>=0 )
		{
			m_vMediaType[p->iSubChannel] = (EnumGSMediaType) p->stDescri.eMediaType;
		}
	}
}


UINT32 CClientChannel::GetCtrlAbilities(void) const
{
	return m_iCtrlAbilities;
}

BOOL CClientChannel::EnableAutoConnect(BOOL bEnable )
{
	GS_ASSERT(0);
	m_eErrno = CIClientChannel::ERRNO_ENFUNC;
	return FALSE;
}

BOOL CClientChannel::SetReconnectInterval(UINT iSecs)
{
	GS_ASSERT(0);
	m_eErrno = CIClientChannel::ERRNO_ENFUNC;
	return FALSE;
}

BOOL CClientChannel::SetReconnectTryMax(UINT iCounts)
{
	GS_ASSERT(0);
	m_eErrno = CIClientChannel::ERRNO_ENFUNC;
	return FALSE;
}


const char *CClientChannel::GetDescri(void) const
{
	return m_strURI.c_str();
}


const StruPlayStatus *CClientChannel::GetPlayStatus(void) const
{
	return &m_stPlayStatus;
}

const CIClientChannel::StruChannelInfo *CClientChannel::GetInfo(void) const
{
	return &m_stChannelInfo;
}

void CClientChannel::SetUserData(void *pData)
{
	m_pUserPrivateData = pData;
}

void *CClientChannel::GetUserData(void) const
{
	return m_pUserPrivateData;
}

UINT32 CClientChannel::GetAutoID(void) const
{
	return m_iAutoID;
}

GSP::EnumErrno CClientChannel::HandleStream( CFrameCache *pFrame, BOOL bSafeThread )
{
	m_iKeepaliveStreamPlugs = 0; //流检测

	if( pFrame->m_stFrameInfo.eMediaType == GS_MEDIA_TYPE_SYSHEADER )
	{
		if( m_pSysHeaderCache )
		{
			CGSPBuffer &csBuf1 = pFrame->GetBuffer();
			CGSPBuffer &csBuf2 = pFrame->GetBuffer();
			if( csBuf2.GetDataSize() == csBuf1.GetDataSize() &&
				csBuf1.GetDataSize() > sizeof(StruGSFrameHeader) )
			{
				if( 0 == ::memcmp(csBuf1.GetData()+sizeof(StruGSFrameHeader),
					csBuf2.GetData()+ sizeof(StruGSFrameHeader), 
					csBuf1.GetDataSize()- sizeof(StruGSFrameHeader) ) )
				{
					//已经接收过
					return eERRNO_SUCCESS;
				}
			}
			m_pSysHeaderCache->UnrefObject();
			m_pSysHeaderCache = pFrame;
			pFrame->RefObject();			

		}
		else
		{
			m_pSysHeaderCache = pFrame;
			pFrame->RefObject();
		}
	}

	GSP::EnumErrno eErrno = eERRNO_SUCCESS;	
	if( m_eTranModel==GSP_TRAN_RTPLAY )
	{
		//实时流
		
		if( (m_iWaitFrameSize > MAX_CACHE_MIN_SIZE &&
			 pFrame->m_stFrameInfo.eMediaType == GS_MEDIA_TYPE_VIDEO)
			|| m_iWaitFrameSize > (MAX_CACHE_MIN_SIZE*3/2) )
		{
			if( m_csStreameQueue.Size() == 0 )
			{
				m_iWaitFrameSize = 0;
			}
			else
			{

				//缓冲区满		
				m_stChannelInfo.iLostFrames++;
				long tCur = (long)time(NULL);
				if( labs(tCur-m_iDebugListTv)> 10 )
				{
					m_iDebugListTv = tCur;
					MY_LOG_WARN(m_pLog, _GSTX("CClientChannel(%u) 累计丢帧(%lld)， 本地缓冲区(%d Bytes)溢出.\n")
						,m_iAutoID, (long long ) m_stChannelInfo.iLostFrames,m_iWaitFrameSize);
				}
				return eERRNO_SYS_EFLOWOUT;	
			}
		}
	}
	else if( m_iWaitFrameSize > MAX_CACHE_SIZE )
	{
		if( m_csStreameQueue.Size() == 0 )
		{
			m_iWaitFrameSize = 0;
		}
		else if( m_iWaitFrameSize> (MAX_CACHE_SIZE*2) )
		{
			m_stChannelInfo.iLostFrames++;
			long tCur = (long)time(NULL);
			if( labs(tCur-m_iDebugListTv)> 10 )
			{
				m_iDebugListTv = tCur;
				MY_LOG_WARN(m_pLog, _GSTX("CClientChannel(%u) 累计丢帧(%lld)， 本地缓冲区(%d Bytes)溢出.\n")
					,m_iAutoID, (long long ) m_stChannelInfo.iLostFrames,m_iWaitFrameSize);
			}
			return eERRNO_SYS_EFLOWOUT;	
		}
		else

		{
			eErrno = eERRNO_SYS_EFLOWOUT;
		}
	}


	pFrame->RefObject();
	INT iTrys = 0;


	long iInsertSize = pFrame->GetBuffer().GetDataSize();

	m_csMutexCacheSize.Lock();
	m_iWaitFrameSize += iInsertSize; 
	m_csMutexCacheSize.Unlock();

	if( !bSafeThread )
	{
		m_csCacheMutex.Lock();
	}	
	BOOL bInsert = TRUE;

	while( eERRNO_SUCCESS != m_csStreameQueue.Write(pFrame) )
	{	
		
		if( m_eTranModel!=GSP_TRAN_RTPLAY )
		{
			//下载不要丢掉			
			if(iTrys++<200 && m_eStatus!=CIClientChannel::ST_ASSERT)
			{				
				if( !bSafeThread )
				{
					m_csCacheMutex.Unlock();
				}

				MSLEEP(10);
				if( !bSafeThread )
				{
					m_csCacheMutex.Lock();
				}
				continue;
			}			
		}	
		//丢帧
		m_stChannelInfo.iLostFrames++;
		pFrame->UnrefObject();	
		if( !eErrno )
		{
			eErrno = eERRNO_SYS_ENMEM;
		}
		bInsert = FALSE;
		break;		
	}

	if( !bSafeThread )
	{
		m_csCacheMutex.Unlock();
	}

	if(  !bInsert )
	{
		m_csMutexCacheSize.Lock();
		m_iWaitFrameSize -= iInsertSize;
		if( iInsertSize < 0 )
		{
			iInsertSize  = 0;
		}
		m_csMutexCacheSize.Unlock();
	}

	if( 0 != m_csTaskPool.Task(TASK_STREAM) )
	{
		eErrno = eERRNO_SYS_ENMEM;
	}


	if( eErrno == eERRNO_SYS_EFLOWOUT )
	{
		if(m_iFlowCtrl<100 )
		{
			//进行流控
			m_csMutex.Lock();		
			if(m_iFlowCtrl<100 && 0==(m_iFlowCtrl%25) )
			{				
				
				m_pProChannel->FlowCtrl(TRUE);
				MY_LOG_WARN(m_pLog, _GSTX("CClientChannel(%u)  .FlowCtrl TRUE\n") ,  m_iAutoID);
			}
			m_iFlowCtrl++;
			m_csMutex.Unlock();
		}
	}	
	else if( eErrno == eERRNO_SYS_ENMEM )
	{
		
		m_csMutex.Lock();
		m_eStatus = CIClientChannel::ST_ASSERT;
		if( m_pProChannel )
		{
			m_pProChannel->DestoryBefore();
		}
		m_csMutex.Unlock();
		OnProChannelSignalEvent(eSIG_ASSERT);
	}
	return eErrno;
}

void CClientChannel::HandlePlayStatus(const StruPlayStatus &stPlayStatus)
{
	m_iKeepaliveStreamPlugs = 0;
	StruPlayStatus *p = (StruPlayStatus*) CMemoryPool::Malloc(sizeof(*p));
	if( !p )
	{
		MY_LOG_FATAL(m_pLog, _GSTX("CClientChannel(%u) 分配内存失败.\n"), m_iAutoID );
		GS_ASSERT(0);
		return;
	}
/*	CGSWRMutexAutoWrite wlocker(&m_csCacheWRMutex);*/
	::memcpy(p, &stPlayStatus, sizeof(*p));
	if(eERRNO_SUCCESS != m_csStatusQueue.Write(p))
	{
		MY_LOG_FATAL(m_pLog, _GSTX("CClientChannel(%u) 分配内存失败.\n"), m_iAutoID );
		GS_ASSERT(0);
		CMemoryPool::Free(p);
		return;
	}

	if( 0 != m_csTaskPool.Task(TASK_STATUS) )
	{
		MY_LOG_FATAL(m_pLog, _GSTX("CClientChannel(%u) 分配内存失败.\n"), m_iAutoID );
		GS_ASSERT(0);		
	}

}

void CClientChannel::OnProChannelSignalEvent( EnumSignal eSignal )
{
	switch(eSignal)
	{
	case eSIG_STREAM_FINISH :
		{
			m_csTaskPool.Task(TASK_FINISH);
		}
		break;
	case eSIG_REMOTE_CLOSE :
		{
			m_csTaskPool.Task(TASK_CLOSE);
			m_csTaskPool.Task(TASK_ASSERT);
		}
		break;
	default :
		{
			m_csTaskPool.Task(TASK_ASSERT);
			m_eStatus = CClientChannel::ST_ASSERT;
		}
		break;
	}

}

void CClientChannel::OnTaskPoolEvent( CObjThreadPool *pTkPool, void *pData )
{

	if(pData == TASK_STREAM )
	{
		CFrameCache *p = NULL;
		//m_csCacheWRMutex.LockReader();
		//m_csStreamCache.RemoveFront(&p);
		//m_csCacheWRMutex.UnlockReader();
		
		if( eERRNO_SUCCESS == m_csStreameQueue.Pop(&p) )
		{
			if( p )
			{
				m_csMutexCacheSize.Lock();
				m_iWaitFrameSize -= p->GetBuffer().GetDataSize();
				if( m_iWaitFrameSize < 0)
				{
					m_iWaitFrameSize = 0;
				}
				m_csMutexCacheSize.Unlock();
			}
			CPAT::CMyCpatFrame csFrame;
			if( csFrame.Set(p) )
			{
				if( !m_bFinish )
				{
					SendEvent(GSP_EVT_CLI_FRAME, (CPAT::CFrame*) &csFrame, sizeof(&csFrame));
				}
			}
			else
			{
				MY_LOG_FATAL(m_pLog, _GSTX("CClientChannel(%u) 丢帧. 分配内存失败.\n"), m_iAutoID );
			}
			p->UnrefObject();
		}
		if( m_csStreameQueue.Size() == 0 && m_iWaitFrameSize)
		{
			m_csMutexCacheSize.Lock();
			m_iWaitFrameSize = 0;
			m_csMutexCacheSize.Unlock();
		}
		if(m_iFlowCtrl>0 && m_iWaitFrameSize < MAX_CACHE_SIZE*2/3 )
		{
			//回复流控
			m_csMutex.Lock();
			if(m_iFlowCtrl>0 && m_iWaitFrameSize < MAX_CACHE_SIZE*2/3 )
			{
				m_iFlowCtrl = 0;
				m_pProChannel->FlowCtrl(FALSE);
				MY_LOG_WARN(m_pLog, _GSTX("CClientChannel(%u)  . FlowCtrl  FALSE\n"),m_iAutoID);
			}
			m_csMutex.Unlock();
		}

		return;
	}

	if( pData==TASK_STATUS)
	{
		StruPlayStatus *p;
		//播放状态
//		m_csCacheWRMutex.LockReader();
// 		m_csStatusCache.RemoveFront(&p);
// 		m_csCacheWRMutex.UnlockReader();
		if( eERRNO_SUCCESS == m_csStatusQueue.Pop(&p) )
		{
			if( !m_bFinish )
			{
				SendEvent(GSP_EVT_CLI_STATUS, (StruPlayStatus*) p, sizeof(StruPlayStatus));
				if( p->iPosition==10000 )
				{
					m_bFinish = TRUE;
				}
			}
			CMemoryPool::Free(p);
		}
		return;
	}

	if( pData == TASK_FINISH )
	{
		//结束
		m_pProChannel->DestoryBefore();		
		SendEvent( GSP_EVT_CLI_COMPLETE );		
		m_bFinish = TRUE;		
		return;
	}

	if( pData == TASK_CLOSE )
	{		
		return;
	}
	if( pData==TASK_CLEAN_CACHE )
	{
		//清除缓冲区
		m_csStreameQueue.Clear();
		m_csStatusQueue.Clear();
		return;
	}

	if( m_pProChannel )
	{
		m_pProChannel->DestoryBefore();
		if( m_stChannelInfo.iLostFrames > 0 )
		{
			MY_LOG_WARN(m_pLog, _GSTX("CClientChannel(%u) %s 累计丢帧 (%lld).\n")
				,m_iAutoID,m_strURI.c_str(), (long long )  m_stChannelInfo.iLostFrames);
			m_stChannelInfo.iLostFrames = 0;
		}
	}
	if( !m_bFinish )
	{
		SendEvent(GSP_EVT_CLI_ASSERT_CLOSE);
	}
	
}

void CClientChannel::OnTimerEvent( CWatchTimer *pTimer )
{
	if( pTimer->GetID() == TIMER_ID_KEEEPALIVE_STREAM )
	{
		//断流检测
		if( m_eStatus == CIClientChannel::ST_PLAYING && !m_bStep)
		{
			m_iKeepaliveStreamPlugs++;
			if( m_iKeepaliveStreamPlugs > m_iMaxKeepaliveStreamTimeouts )
			{
				//断流通知
				m_iKeepaliveStreamPlugs = 0;
				m_csTaskPool.Task(TASK_ASSERT);

			}
		}
	}
}


INT CClientChannel::CountSeqStep(INT iCurSeq, INT iLastSeq )
{
	if( iLastSeq==-1 )
	{
		return 0;
	}

	INT iRet = iCurSeq-iLastSeq;
	if( iRet>=0 )
	{
		return iRet;
	}
	//  ---c--l---
	return 0xFFFF+iRet+1;
}
