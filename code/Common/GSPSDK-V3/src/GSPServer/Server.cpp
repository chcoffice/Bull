#include "Server.h"
#include "IBaseSource.h"
#include "RefSource.h"
#include "Log.h"
#include "GSPProDebug.h"
#include "OSThread.h"
#include "GSPMemory.h"

#ifdef ENABLE_GSP_MODULE
#include "GSP/GspServer.h"
#endif

#ifdef ENABLE_RTSP_MODULE 
#include "RTSP/RtspServer.h"
#endif

#ifdef ENABLE_SIP_MODULE
#include "SIP/SipServer.h"
#endif

using namespace GSP;


static int s_iMark = 1;
static int s_iDMark = 2;
#define HOLD_SOURCE_MARK ((CISource*)&s_iMark)
#define HOLD_SOURCE_DMARK ((CISource*)&s_iDMark)

#define SOURCE_FLAGS_INTERNAL 1

#define IS_VALID_SRCE(p) ((p)!=NULL && (p)!=HOLD_SOURCE_MARK && (p)!=HOLD_SOURCE_DMARK )
#define IS_FREE_SRCE(p)  ((p)==NULL)


CGSPServerOptions::CGSPServerOptions(void)
{

}


CServer::CServer(void)
:CGSPObject()
,CIServer()

{
	m_pLog = NULL;
	m_pEventFnParam = NULL;       //事件回调参数
	m_fnOnEvent = NULL; //事件回调

	m_pSecurityCenter = NULL;
	m_bDestroying = FALSE;
}

CServer::~CServer(void)
{
	m_bDestroying = 1;
	Stop();
	CProServerModuleMap::iterator csIt;
	for( csIt=m_mIServer.begin(); csIt != m_mIServer.end(); ++csIt )
	{
		csIt->second->Unint();		
	}

	m_csMutex.Lock();
	m_csMutex.Unlock();

	m_csSrcWRMutex.LockWrite();
	std::vector<CBaseSource *> vSrc = m_vSource;
	m_vSource.clear();
	m_mSource.clear();
	m_csSrcWRMutex.UnlockWrite();
	for( UINT i = 0; i<vSrc.size();i++ )
	{
		if( IS_VALID_SRCE(vSrc[i]) )
		{
			/*vSrc[i]->UnrefObject();*/
			INT iRef = vSrc[i]->GetRefCounts();
			vSrc[i]->Release();
			if( iRef>1 )
			{
				vSrc[i]->UnrefObject();
			}
		}
	}
	m_bDestroying = 2;
	CISrvSession*pSession;	
	m_csSessionMutex.LockWrite();
	while( eERRNO_SUCCESS==m_csSessionList.RemoveFront( (void**) &pSession) )
	{
		m_csSessionMutex.UnlockWrite();
		pSession->DeleteBefore();
		delete pSession;
		m_csSessionMutex.LockWrite();
	}
	m_csSessionMutex.UnlockWrite();


	for( csIt=m_mIServer.begin(); csIt != m_mIServer.end(); ++csIt )
	{		
		delete csIt->second;
	}
	m_mIServer.clear();

	if( m_pLog )
	{
		m_pLog->UnrefObject();
		m_pLog = NULL;
	}

	GSPModuleUnint();

	
}

CIUri *CServer::CreateEmptyUriObject(void)
{
	return new CUri();
}

BOOL CServer::Init(  const char *csConfigFilename,const char *csLogPathName )
{
	
	CGSAutoMutex locker( &m_csMutex);
	m_vSource.resize(s_iMaxServerManageSource, NULL);

	//GS_ASSERT(csConfigFilename && csLogPathName);
	 std::string strAppPath = GSGetApplicationPath();


	CGSString strTemp;
	if( csLogPathName )
	{
		strTemp = csLogPathName;
	}
	else
	{
		strTemp = strAppPath;
		strTemp += "log/GSPSrv";
		GSPathParser(strTemp);
	}
	m_pLog = CLog::Create();
	GS_ASSERT_RET_VAL(m_pLog, FALSE);
	m_pLog->SetLogPath(strTemp.c_str());


	
	if( csConfigFilename )
	{	
		strTemp = csConfigFilename;
	}
	else
	{
		strTemp = strAppPath;
		strTemp += "GSPSrvConf.ini";		
	}
	CGSString strIniFilename = strTemp;
	if( !m_csConfig.LoadFile( (char*) strTemp.c_str()) )
	{
		MY_LOG_FATAL(m_pLog, _GSTX("GSP 协议栈加载配置文件 '%s' 失败.\n"),
			strTemp.c_str() );
		GS_ASSERT(0);
		return FALSE;
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

	//加载路由信息
	COSSocket::LoadManualRouteTable(strIniFilename);

	return InitProModule();
}

void CServer::InitLog( const char *czPathName )
{
	//
	MY_DEBUG(_GSTX("该功能函数已经作废.\n") );

}

BOOL CServer::InitProModule(void)
{
	
	CIProServer *pServer ;
	EnumErrno eErrno;
#ifdef   ENABLE_GSP_MODULE
	pServer = new CGspServer();
	if( !pServer )
	{
		MY_LOG_FATAL(m_pLog, _GSTX("错误: 建立GSP协议服务失败.\n" ));
		GS_ASSERT(0);
		return FALSE;
	}
	eErrno = pServer->Init(this);
	if( eERRNO_SUCCESS != eErrno )
	{
		MY_LOG_FATAL(m_pLog, _GSTX("GSP协议服务初始化失败.\n" ) );
		GS_ASSERT(0);
		return FALSE;
	}
	m_mIServer.insert(make_pair(pServer->Protocol(), pServer));
	
#endif

#ifdef   ENABLE_RTSP_MODULE
	pServer = new RTSP::CRtspServer();
	if( !pServer )
	{
		MY_LOG_FATAL(m_pLog, _GSTX("错误: 建立RTSP协议服务失败.\n" ));
		GS_ASSERT(0);
		return FALSE;
	}
	eErrno = pServer->Init(this);
	if( eERRNO_SUCCESS != eErrno )
	{
		MY_LOG_FATAL(m_pLog, _GSTX("RTSP协议服务初始化失败.\n" ) );
		GS_ASSERT(0);
		return FALSE;
	}
	m_mIServer.insert(make_pair(pServer->Protocol(), pServer));
#endif 
	

#ifdef   ENABLE_SIP_MODULE
	pServer = new SIP::CSipServer();
	if( !pServer )
	{
		MY_LOG_FATAL(m_pLog, _GSTX("错误: 建立Sip协议服务失败.\n" ));
		GS_ASSERT(0);
		return FALSE;
	}
	eErrno = pServer->Init(this);
	if( eERRNO_SUCCESS != eErrno )
	{
		MY_LOG_FATAL(m_pLog, _GSTX("Sip协议服务初始化失败.\n" ) );
		GS_ASSERT(0);
		return FALSE;
	}
	m_mIServer.insert(make_pair(pServer->Protocol(), pServer));

#endif


	CProServerModuleMap::iterator csIt;
	for( csIt = m_mIServer.begin(); csIt != m_mIServer.end(); ++csIt)
	{
		pServer = csIt->second;		
		if( eERRNO_SUCCESS == pServer->Start()  )
		{
			MY_LOG_INFO(m_pLog, _GSTX("%s 协议服务启动成功.\n"),
				GetProtocolName(csIt->second->Protocol()));

		}
		else
		{
			MY_LOG_FATAL(m_pLog, _GSTX("%s 协议服务启动成功.\n"),
				GetProtocolName(pServer->Protocol()));
			return FALSE;

		}
	}

	return TRUE;
}

BOOL  CServer::Stop(void)
{
	CProServerModuleMap::iterator csIt;
	CIProServer *pServer ;
	for( csIt = m_mIServer.begin(); csIt != m_mIServer.end(); ++csIt)
	{
		pServer = csIt->second;		
		pServer->Stop();
		MY_LOG_INFO(m_pLog, _GSTX("%s 协议服务停止.\n"),
					GetProtocolName(csIt->second->Protocol() ));
	}
	return TRUE;
}

CISource *CServer::FindSource(const char *csKey)
{
	CGSAutoReaderMutex rlocker( &m_csSrcWRMutex );
	CSourceMap::iterator csIt;
	csIt = m_mSource.find(csKey );
	if( csIt == m_mSource.end() )
	{
		return NULL;
	}
	return csIt->second;

}

CISource *CServer::FindSource(UINT16 iSourceIndex )
{
	GS_ASSERT_RET_VAL(iSourceIndex<m_vSource.size(), NULL);
	if( IS_VALID_SRCE(m_vSource[iSourceIndex]) )
	{
		return m_vSource[iSourceIndex];
	}
	return NULL;
}

CISource *CServer::AddPullSource( const char *czKey )
{
	return AddSource(czKey, CISource::eMODE_PULL);
}

CISource *CServer::AddSource( const char *szKey )
{
	return AddSource(szKey, CISource::eMODE_PUSH);
}

CISource *CServer::AddSource( const char *szKey, CISource::EnumMode eMode )
{
	if( CMemoryPool::IsNoMemory() )
	{
		MY_LOG_WARN(g_pLog, _GSTX("GSP服务器 拒绝增加数据源(%s). 内存缺乏.\n"),
			szKey);
		return  NULL;
	}

	CGSAutoWriterMutex wlocker(&m_csSrcWRMutex);
	if( m_bDestroying )
	{
		GS_ASSERT(0);
		return NULL;
	}
	
	CSourceMap::iterator csIt;
	csIt = m_mSource.find(szKey );
	if( csIt != m_mSource.end() )
	{
		MY_LOG_ERROR(m_pLog, _GSTX("数据源 '%s'已经存在.\n")
					,szKey);	
		return NULL;
	}
	//查找空闲
	INT iIdx = -1;
	for(UINT i = 0; i<m_vSource.size(); i++ )
	{
		if( IS_FREE_SRCE(m_vSource[i]) )
		{
			iIdx = i;
			break;
		}
	}
	if( iIdx==-1 )
	{
		//已经没有空闲
		MY_LOG_ERROR(m_pLog, _GSTX("GSP 数据源管理已经满负载.\n"));		
		return NULL;
	}
	CBaseSource *pSource = CBaseSource::Create(this, eMode);
	if( !pSource )
	{
		MY_LOG_FATAL(m_pLog, _GSTX("创建数据源对象失败.\n"));
		return NULL;
	}
	pSource->RefObject();
	pSource->SetKeyInfo(szKey, iIdx);
	m_vSource[iIdx] = pSource;
	m_mSource[szKey] = pSource;
	return pSource;
	
	
}

CISource::EnumRetNO CServer::WriteData(UINT16 iSourceIndex,  const void *pData, INT iLen, UINT iChn, BOOL bKey)
{

	GS_ASSERT_RET_VAL( iSourceIndex < s_iMaxServerManageSource , CISource::SRC_RET_EINVALID );
	if( IS_VALID_SRCE(m_vSource[iSourceIndex]) )
	{
		return m_vSource[iSourceIndex]->WriteData(pData, iLen,iChn, bKey);
	}
	return CISource::SRC_RET_EINVALID;
}


CISource::EnumRetNO CServer::WriteSysHeader(UINT16 iSourceIndex, const void *pData, INT iLen, UINT iChn)
{

	GS_ASSERT_RET_VAL( iSourceIndex < s_iMaxServerManageSource , CISource::SRC_RET_EINVALID );
	if( IS_VALID_SRCE(m_vSource[iSourceIndex]) )
	{
		return m_vSource[iSourceIndex]->WriteSysHeader(pData, iLen,iChn);
	}
	return CISource::SRC_RET_EINVALID;	
}

CISource::EnumRetNO CServer::WriteDataV(UINT16 iSourceIndex,  const StruIOV *pIOV, INT iVNums, UINT iChn, BOOL bKey)
{
	GS_ASSERT_RET_VAL( iSourceIndex < s_iMaxServerManageSource , CISource::SRC_RET_EINVALID );
	if( IS_VALID_SRCE(m_vSource[iSourceIndex]) )
	{
		return m_vSource[iSourceIndex]->WriteData(pIOV, iVNums,iChn, bKey);
	}
	return CISource::SRC_RET_EINVALID;	
	
}


CISource::EnumRetNO CServer::WriteSysHeaderV(UINT16 iSourceIndex,  const StruIOV *pIOV, INT iVNums, UINT iChn)
{
	GS_ASSERT_RET_VAL( iSourceIndex < s_iMaxServerManageSource , CISource::SRC_RET_EINVALID );
	if( IS_VALID_SRCE(m_vSource[iSourceIndex]) )
	{
		return m_vSource[iSourceIndex]->WriteSysHeader(pIOV, iVNums,iChn);
	}
	return CISource::SRC_RET_EINVALID;		
}

void CServer::SetEventCallback( GSPServerEventFunctionPtr fnOnEvent, void *pParam)
{
	m_fnOnEvent = fnOnEvent;
	m_pEventFnParam = pParam;
}






void CServer::InitURI( CIUri &csRet, const char *czKey, const char *czPro,
					  const char *szRemoteIP)
{
	

	if( !czPro || m_bDestroying )
	{
		csRet.SetHost("0");
		csRet.SetScheme("unk");
		GS_ASSERT(0);
		return;
	}
EnumProtocol ePro = GetProtocolType(czPro);

	

	CIProServer *pServer;
	CProServerModuleMap::iterator csIt;
	csIt = m_mIServer.find(ePro);
	if( csIt==m_mIServer.end() )
	{
		csRet.SetHost("0");
		csRet.SetScheme("unk");
		GS_ASSERT(0);
		MY_LOG_ERROR(m_pLog, _GSTX("GSP 协议栈不支持 '%s' 协议\n"),
					czPro );
		return;
	}
	pServer = csIt->second;
	csRet.SetKey(czKey);
	csRet.SetHost("127.0.0.1");
	csRet.SetScheme(GetProtocolName(ePro));

	CGSString strIP = GetRouteIP(szRemoteIP, pServer);
	if( strIP.length()>0 )
	{
		csRet.SetHost(strIP.c_str());
	}


	csRet.SetPortArgs( pServer->ListenPort() );
	csRet.AddAttr("pro", "tcp");      

	


	if( m_pSecurityCenter )
	{
		//安全信息
		CGSPString strPermit;
		strPermit = m_pSecurityCenter->GetSecurityKey(czKey);
		csRet.AddAttr("S", strPermit.c_str());
	}

	//增加其他协议端口
	CProServerModuleMap::iterator csOIt;
	for( csOIt = m_mIServer.begin(); csOIt!=m_mIServer.end(); ++csOIt  )
	{
		if( csOIt->second == csIt->second )
		{
			continue;
		}
		pServer = csOIt->second;
		ePro = pServer->Protocol();
		csRet.AddAttr(GetProtocolName(ePro),
			         GSStrUtil::ToString( pServer->ListenPort()).c_str());
	}

}


BOOL CServer::QueryStatus(const EnumGSPServerStatus eQueryStatus, 
				 char **ppResult,INT *pResultSize  )
{
	//TODO...
	*ppResult = NULL;
	*pResultSize = 0;
	switch(eQueryStatus)
	{
	case GSP_SRV_STATUS_SUPPORT_PROTOCOL :
		{
			INT i = m_mIServer.size();
			INT iSize = sizeof(StruProtocolService)*(i+1);
			StruProtocolService *pResult = (StruProtocolService *)CMemoryPool::Malloc(iSize);
			GS_ASSERT_RET_VAL(NULL!=pResult, FALSE);
			
			bzero(pResult, iSize );
			i = 0;
			for(CProServerModuleMap::iterator csIt = m_mIServer.begin(); 
				csIt != m_mIServer.end(); ++csIt )
			{					
				pResult->vService[pResult->iNums].czProtocol = GetProtocolName((*csIt).second->Protocol());
				pResult->vService[pResult->iNums].czSrvBindIP = (*csIt).second->ListenIP().c_str();
				pResult->vService[pResult->iNums].iSrvBindPort = (*csIt).second->ListenPort();
				pResult->vService[pResult->iNums].iStatus = 0; //不成功的不会加到队列
				pResult->iNums++;
			}
			*ppResult = (char*)pResult;
			*pResultSize = iSize;
		}
	break;
	case GSP_SRV_STATUS_SOURCE_LIST :
		{
			CGSAutoReaderMutex rlocker( &m_csSrcWRMutex);
			INT i = m_mSource.size();
			INT iSize =  sizeof(UINT16)*(i+1)+sizeof(INT);
			StruSourceVector *pResult = (StruSourceVector *)CMemoryPool::Malloc(iSize);
			GS_ASSERT_RET_VAL(NULL!=pResult, FALSE);
			bzero(pResult,iSize);
			;
			for(CSourceMap::iterator csIt = m_mSource.begin(); 
				csIt != m_mSource.end(); ++csIt )
			{	
				pResult->vSourceIndex[pResult->iNums] = (*csIt).second->GetSourceID();
				pResult->iNums++;
			}
			*ppResult = (char*)pResult;
			*pResultSize = iSize;
			
		}
	break;
	case GSP_SRV_STATUS_GET_CLIENTS_INFO :
		{
			//返回客户端数据
			CGSAutoReaderMutex rlocker( &m_csSessionMutex);
			INT i = m_csSessionList.Size();
			INT iSize =  sizeof(StruClientInfo)*(i+1)+sizeof(INT);
			StruClientVector *pResult = (StruClientVector *)CMemoryPool::Malloc(iSize);
			GS_ASSERT_RET_VAL(NULL!=pResult, FALSE);
			bzero(pResult, iSize );
			CList::CIterator<CISrvSession *> csIt;
			for(csIt = m_csSessionList.First<CISrvSession*>(); 
						csIt.IsOk(); csIt.Next() )
			{	
				::memcpy( &pResult->vClient[pResult->iNums], csIt.Data()->ClientInfo(), 
							sizeof(StruClientInfo));
				pResult->iNums++;
			}
			*ppResult = (char*)pResult;
			*pResultSize = iSize;
		}
	break;
	default:
		{
			GS_ASSERT(0);
			return FALSE;
		}
	break;
	}
	return TRUE;
}




void CServer::FreeQueryStatusResult( char *pResult)
{

		CMemoryPool::Free(pResult);
	
}

EnumErrno CServer::RequestSource(const CUri &csUri,const CIMediaInfo &csRquestMedia , INT eTransModel,
						CRefSource **ppResult)
{

	//先安全认证
	*ppResult = NULL;
	if( m_pSecurityCenter )
	{
		const StruUriAttr *p = csUri.GetAttr("S");
		if( !p )
		{
			MY_LOG_ERROR(m_pLog, _GSTX("GSP 请求：'%s' 没有安全认证.\n"),
				csUri.GetKey() );
			return eERRNO_SYS_EPERMIT;
		}
		if( !m_pSecurityCenter->SecurityCheck(p->szValue, csUri.GetKey() ) )
		{
			MY_LOG_ERROR(m_pLog, _GSTX("GSP 请求：'%s' 安全认证不通过.\n"),
				csUri.GetKey() );
			return eERRNO_SYS_EPERMIT;
		}
	}



	if( m_bDestroying )
	{
		return eERRNO_SYS_EBUSY;
	}
	
	CSourceMap::iterator csIt;

	m_csSrcWRMutex.LockReader();
	csIt = m_mSource.find(csUri.GetKey());
	if( csIt == m_mSource.end() )
	{
		//不存在数据源
		MY_LOG_ERROR(m_pLog, _GSTX("GSP 不存在数据源： '%s'.\n"),
					csUri.GetKey() );
		m_csSrcWRMutex.UnlockReader();
		CIUri *pUri = (CIUri*) &csUri;
		CISource *pOutSource = (CISource *) ProcessServerEvent( NULL, 
					GSP_SRV_EVT_RQUEST_SOURCE, pUri, sizeof(CIUri *) );
		if( pOutSource )
		{
			pOutSource->UnrefObject();

			m_csSrcWRMutex.LockReader();
			csIt = m_mSource.find(csUri.GetKey());
			if( csIt == m_mSource.end() )
			{
				m_csSrcWRMutex.UnlockReader();
				return eERRNO_SRC_ENXIST;
			}
		}
		else
		{
			return eERRNO_SRC_ENXIST;
		}
	}
	CBaseSource *pSource = NULL;

	pSource = csIt->second;
	pSource->RefObject();
	m_csSrcWRMutex.UnlockReader();


	*ppResult  = pSource->RefSource(&csRquestMedia);
	pSource->UnrefObject();
	if( NULL == *ppResult)
	{
		MY_LOG_ERROR(m_pLog, _GSTX("GSP 数据源： '%s' 建立引用失败.\n"),
			csUri.GetKey() );
		
		return eERRNO_SRC_ENXIST;
	}
	return eERRNO_SUCCESS;
}



void  CServer::OnProServerEvent(CIProServer *pServer, EnumProServerEvent eEvt, void *pEvtArg)
{
	
	switch(eEvt)
	{
	case eEVT_PROSRV_ACCEPT :
		{
			CISrvSession*pSession = (CISrvSession*)pEvtArg;
			m_csSessionMutex.LockWrite();
			if( eERRNO_SUCCESS!= m_csSessionList.AddTail(pSession) )
			{
				GS_ASSERT(0);
				MY_LOG_ERROR(m_pLog, _GSTX("分配内存失败!\n"));
			}
			m_csSessionMutex.UnlockWrite();
		}
		break;
	case eEVT_PROSRV_SESSION_RELEASE :
		{
			CISrvSession*pSession = (CISrvSession*)pEvtArg;
			m_csSessionMutex.LockWrite();
			m_csSessionList.Remove(pSession);
			m_csSessionMutex.UnlockWrite();
		}
	break;

	}
}

void  *CServer::OnSourceEvent(CBaseSource *pSource, EnumSourceEvent eEvt, void *pEvtArg)
{
	if( m_bDestroying )
	{
		return 0;
	}

	switch(eEvt)
	{
	case eEVT_SRC_REF :
		{			
			return ProcessServerEvent(pSource, GSP_SRV_EVT_REF, NULL, 0);
		}
		break;
	case eEVT_SRC_UNREF :
		{    
			return ProcessServerEvent(pSource, GSP_SRV_EVT_UNREF, NULL, 0);			
		}
		break;
	case eEVT_SRC_CTRL :
		{
			if( m_bDestroying )
			{
				return 0;
			}
			return ProcessServerEvent(pSource, GSP_SRV_EVT_CTRL, pEvtArg, sizeof(StruGSPCmdCtrl));
		}
		break;
	case eEVT_SRC_RELEASE :
		{			
			RemoveSource(pSource);
		}
		break;
	default :
		break;

	}
	return NULL;
	
}

void *CServer::ProcessServerEvent(  CISource *pSource,EnumGSPServerEvent eEvent, 
								 void *pEventPararms, INT iLen)
{
	if( m_fnOnEvent )
	{
		return m_fnOnEvent(this, pSource, eEvent, pEventPararms, 
			iLen, m_pEventFnParam);
	}
	return 0;
}

void CServer::RemoveSource( CISource *pSource)
{
	if( m_bDestroying )
	{		
		return;
	}

	UINT16 iIndex = pSource->GetSourceID();
	GS_ASSERT_RET(iIndex<m_vSource.size() );

	m_csSrcWRMutex.LockWrite();

	if( (CISource*)m_vSource[iIndex] == pSource)
	{
		CBaseSource *pInnerSource = m_vSource[iIndex];
		MY_LOG_DEBUG(m_pLog, _GSTX("GSP 协议栈移除数据源: '%s'.\n."),				
			pInnerSource->GetKey());
		m_vSource[iIndex] = NULL;
		m_mSource.erase(pInnerSource->GetKey());
		m_csSrcWRMutex.UnlockWrite();
		pInnerSource->UnrefObject();
		return;

	}
	else
	{
		GS_ASSERT(0);
	}
	m_csSrcWRMutex.UnlockWrite();
}


CGSString CServer::GetRouteIP(const char * czRemoteIP, CIProServer *pServer )
{

	if( czRemoteIP == NULL )
	{
		return COSSocket::GuessLocalIp( CGSString(), pServer->ListenIP() );
	}
	return COSSocket::GuessLocalIp( czRemoteIP, pServer->ListenIP() );



}




GSP::CIServer *GSP::CreateGSPServerInterface(void)
{      
	GSPModuleInit();
	GSP::CIServer *pRet = new GSP::CServer();
	if( !pRet )
	{
		GSPModuleUnint();
	}
	return pRet;
}

