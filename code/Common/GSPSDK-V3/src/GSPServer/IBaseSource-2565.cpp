#include "IBaseSource.h"
#include "Server.h"
#include "Log.h"
#include "RefSource.h"
#include "MyObjectDebug.h"

using namespace GSP;

GSAtomicInter CBaseSource::s_iAutoIDSequence = 0;

static void _FreeListener( CRefSource *p)
{
	CIStreamConverter *pCnv;
	pCnv = (CIStreamConverter *)p->GetSourcePrivate();
	if( NULL != pCnv )
	{
		//转码
		pCnv->RemoveRefSourceListener(p);
	}
	delete p;
}

static void _FreeCacheMember( CSliceFrame *p)
{
	p->UnrefObject();
}


CBaseSource::CBaseSource(CServer *pServer, CISource::EnumMode  eMode)
:CGSPObject()
,CIBaseSource()
,m_pServer(pServer)
,m_iAutoID(AtomicInterInc(s_iAutoIDSequence))
,m_csStreamCache()
,m_csMediaInfo()
,m_csStreamTask("SoureStream")
{
	m_eMode  = eMode;
	m_fnCallback = NULL;
	m_pUserArgs = NULL;
	m_iObjectRefs = 1;
	GS_ASSERT(pServer);	
	m_csStreamCache.SetFreeCallback((FuncPtrFree)_FreeCacheMember);
	m_csStreamTask.SetFreedTaskDataFunction((FuncPtrFree)_FreeCacheMember);
	m_csStreamTask.SetMaxWaitTask(300); //大概十秒数据
	m_csSysFrameCache.resize(GSP_MAX_MEDIA_CHANNELS);


	m_csStreamTask.Init(this, (FuncPtrObjThreadPoolEvent)&CBaseSource::OnStreamTaskPoolEvent,1, FALSE);


	m_iSourceID = 0;
	m_iAbiliteMasrk = GSP_CTRL_PLAY|GSP_CTRL_STOP|GSP_CTRL_PAUSE|GSP_CTRL_FAST|GSP_CTRL_STEP|GSP_CTRL_SLOW|GSP_CTRL_SETPOINT;
	m_bEnableRef = TRUE;
	m_eTransModel = GSP_TRAN_RTPLAY;
	m_strKey="";
	m_bValid = TRUE; //有效
	
	bzero( &m_stPlayStatus, sizeof(m_stPlayStatus));

	m_iTestVWrite = 0;

	m_bRelease = FALSE;

	m_pPullTimer = NULL;
	
}

CBaseSource::~CBaseSource(void)
{
	m_csStreamTask.Uninit();
	GS_ASSERT(m_iObjectRefs==0);

	if( m_pPullTimer )
	{
		m_pPullTimer->Stop();
	}

	m_csWRMutex.LockWrite();
	for( UINT i = 0; i<m_csSysFrameCache.size(); i++ )
	{
		for( UINT j = 0; j<m_csSysFrameCache[i].size(); j++ )
		{
			if( m_csSysFrameCache[i][j] )
			{
				m_csSysFrameCache[i][j]->UnrefObject();
				m_csSysFrameCache[i][j] = NULL;
			}
		}
		m_csSysFrameCache[i].clear();
	}
	m_csWRMutex.UnlockWrite();

	GS_ASSERT(m_csListener.Size()==0);

	CList lsTemp;
	m_csWRMutex.LockWrite();
	m_csListener.Swap(lsTemp);
	m_csWRMutex.UnlockWrite();
	lsTemp.SetFreeCallback((FuncPtrFree)_FreeListener);
	lsTemp.Clear();

	if( m_fnCallback )
	{
		(*m_fnCallback)(this, CISource::eEVT_RELEASE,m_pUserArgs);
	}



	SetUserData(NULL);

	if( m_pPullTimer )
	{
		delete m_pPullTimer;
		m_pPullTimer = NULL;
	}	

	MY_LOG_DEBUG(g_pLog, _GSTX("Source(%lu) Destroy.\n"), m_iAutoID);

}

CISource::EnumMode CBaseSource::GetMode(void) const
{
	return m_eMode;
}

void CBaseSource::SetEventCallback(CISource::FuncPtrISourceEventCallback fnCallback, 
							  void *pUserArgs)
{
	m_pUserArgs = pUserArgs;
	m_fnCallback = fnCallback;
}

BOOL CBaseSource::EnablePullEvent( BOOL bStart, INT iMSecs )
{
	GS_ASSERT_RET_VAL(m_eMode==CISource::eMODE_PULL, FALSE);
	if( iMSecs<1 )
	{
		iMSecs = 1;
	}

	if( m_pPullTimer == NULL )
	{
		CGSAutoWriterMutex wlocker( &m_csWRMutex);
		if( m_pPullTimer == NULL  )
		{

			m_pPullTimer = new CWatchTimer();
			GS_ASSERT_RET_VAL(m_pPullTimer, FALSE);
			m_pPullTimer->Init(this,(FuncPtrTimerCallback)&CBaseSource::OnPullTimerEvent, 
				1, iMSecs, bStart );
			if( m_pPullTimer->IsReady() )
			{
				TRUE;
			}
			delete m_pPullTimer;
			m_pPullTimer = NULL;
			return FALSE;
		}
	}

	if( bStart )
	{
		m_pPullTimer->AlterTimer(iMSecs);
		m_pPullTimer->Start();
	}
	else
	{
		m_pPullTimer->Stop();
	}

	return TRUE;
}


CISource * CBaseSource::RefObject(void)
{
	AtomicInterInc(m_iObjectRefs);
	return this;
}

void CBaseSource::UnrefObject(void)
{
	if( AtomicInterDec(m_iObjectRefs) == 0 )
	{		
		delete this;
	}
}

UINT CBaseSource::GetSrcRefs(void)
{
	return m_csListener.Size();
}

UINT32 CBaseSource::GetAutoID(void)
{
	return m_iAutoID;
}

UINT16 CBaseSource::GetSourceID(void)
{
	return m_iSourceID;
}

void CBaseSource::SetMediaInfo( UINT iChn,const StruGSMediaDescri *pInfo )
{
	m_csMediaInfo.AddChannel(pInfo, iChn, NULL);
}

void CBaseSource::SetCtrlAbilities(UINT32 iAbilities)
{
	m_iAbiliteMasrk = iAbilities;

}

void CBaseSource::SourceEnableRef(BOOL bEnable )
{
	m_bEnableRef = bEnable;
}

void CBaseSource::SourceEnableValid(BOOL bEnable )
{
	m_bValid = bEnable;
}

CIUri *CBaseSource::MakeURI( const char *czPro, const char *pRemoteIP )
{
	if( !m_bValid )
	{
		return NULL;
	}

	CUri *pRet;
	pRet = new CUri();
	if( !pRet )
	{
		MY_LOG_FATAL(g_pLog,_GSTX("Source(%lu): '%s' 内存分配失败.\n"),
								m_iAutoID, m_strKey.c_str() );
	}
	m_pServer->InitURI( *pRet, m_strKey.c_str() ,czPro, pRemoteIP );
	return pRet;
}

void CBaseSource::SetPlayStatus( const StruPlayStatus *pStatus )
{
	if( m_bValid )
	{

		::memcpy( &m_stPlayStatus, pStatus, sizeof(m_stPlayStatus));
		NotifyRefSource(CRefSource::eEVT_PLAY_STATUS, (void*) pStatus);
	}
}

void CBaseSource::ReplayEnd(void)
{
	if( m_bValid )
	{
	NotifyRefSource(CRefSource::eEVT_PLAY_END, NULL);
	}
}

CISource::EnumRetNO CBaseSource::WriteData( const void *pData, INT iLen, UINT iChn, BOOL bKey)
{
	StruIOV struIO;
	struIO.iSize = iLen;
	struIO.pBuffer = (void*) pData;
	return WriteDataV(&struIO, 1, iChn, bKey);
}


CISource::EnumRetNO CBaseSource::WriteSysHeader( const void *pData, INT iLen, UINT iChn)
{
	StruIOV struIO;
	struIO.iSize = iLen;
	struIO.pBuffer = (void*) pData;
	return WriteSysHeaderV(&struIO, 1, iChn);
}


CISource::EnumRetNO CBaseSource::WriteDataV( const StruIOV *pIOV, INT iVNums, UINT iChn, BOOL bKey)
{
	GS_ASSERT(m_eMode==CISource::eMODE_PUSH);

	GS_ASSERT_RET_VAL(m_bValid && iVNums<GSP_MAX_MEDIA_CHANNELS, CISource::SRC_RET_EINVALID);

	const CIMediaInfo::StruMediaChannelInfo *pInfo = m_csMediaInfo.GetChannel(iChn);
	GS_ASSERT_RET_VAL(pInfo, CISource::SRC_RET_EINVALID );
	EnumGSMediaType eType;
	eType = (EnumGSMediaType)pInfo->stDescri.eMediaType;
	BOOL bCache;
	bCache = (eType==GS_MEDIA_TYPE_VIDEO && m_bEnableRef );

	if( bCache && bKey )
	{
		//只缓冲一个I帧间隔的数据
		m_csWRMutex.LockWrite();
		m_csStreamCache.Clear();
		m_csWRMutex.UnlockWrite();
	}

	CSliceFrame *pFrame; 
	pFrame = CSliceFrame::Create( iChn,eType );

	if( !pFrame )
	{
		GS_ASSERT(0);
		MY_LOG_FATAL( g_pLog,  _GSTX("Source(%lu): '%s' 新建CSliceFrame对象失败.\n"),
			 m_iAutoID, m_strKey.c_str() ); 
		return CISource::SRC_RET_EFLOWOUT;
	}

	if( eERRNO_SUCCESS!= pFrame->SetFrameData( pIOV[0].pBuffer, pIOV[0].iSize,bKey ) )
	{
		GS_ASSERT(0);
		pFrame->UnrefObject();
		MY_LOG_FATAL( g_pLog,  _GSTX("Source(%lu): '%s' 分配内存失败.\n"), 
										m_iAutoID, m_strKey.c_str());
		return CISource::SRC_RET_EFLOWOUT;
	}

	for( int i = 1; i<iVNums; i++)
	{
		if( eERRNO_SUCCESS!= pFrame->AddFrameData( pIOV[i].pBuffer, pIOV[i].iSize ) )
		{
			GS_ASSERT(0);
			pFrame->UnrefObject();
			MY_LOG_FATAL( g_pLog,  _GSTX("Source(%lu): '%s' 分配内存失败.\n"), 
								m_iAutoID, m_strKey.c_str());
			return CISource::SRC_RET_EFLOWOUT;
		}
	}

	if( bCache )
	{
		//加到缓冲区
		pFrame->RefObject();
		m_csWRMutex.LockWrite();		
		if( m_csStreamCache.AddTail(pFrame) )
		{
			MY_LOG_FATAL( g_pLog,  _GSTX("Source(%lu): '%s' 分配内存失败.\n"), 
				m_iAutoID, m_strKey.c_str());
			GS_ASSERT(0);
			pFrame->UnrefObject();
			m_csWRMutex.UnlockWrite();
		}
		m_csWRMutex.UnlockWrite();
	}

	if( eType == GS_MEDIA_TYPE_SYSHEADER )
	{    
		m_csWRMutex.LockWrite();	
		/*iChn*/
		for( UINT i = 0; i<m_csSysFrameCache[0].size(); i++ )
		{
			m_csSysFrameCache[0][i]->UnrefObject();
			m_csSysFrameCache[0][i] = NULL;
		}
		m_csSysFrameCache[0].clear();    

		m_csSysFrameCache[0].push_back(pFrame);
		pFrame->RefObject();
		m_csWRMutex.UnlockWrite();	
	}  
	CISource::EnumRetNO eRet = CISource::SRC_RET_EFLOWOUT;
	if( m_eTransModel==GSP_TRAN_RTPLAY )
	{

		if( m_csStreamTask.RSUCCESS == m_csStreamTask.Task(pFrame) )
		{

			return CISource::SRC_RET_SUCCESS;
		}	
	} 
	else
	{
		eRet = NotifyRefSource(pFrame);
	}
	pFrame->UnrefObject();
	return eRet;
}

CISource::EnumRetNO CBaseSource::WriteSysHeaderV( const StruIOV *pIOV, INT iVNums, UINT iChn)
{
	GS_ASSERT(m_eMode==CISource::eMODE_PUSH);

	GS_ASSERT_RET_VAL(m_bValid && iVNums<GSP_MAX_MEDIA_CHANNELS, CISource::SRC_RET_EINVALID);

	const CIMediaInfo::StruMediaChannelInfo *pInfo = m_csMediaInfo.GetChannel(iChn);
	GS_ASSERT_RET_VAL(pInfo, CISource::SRC_RET_EINVALID );
	EnumGSMediaType eType;
	eType = GS_MEDIA_TYPE_SYSHEADER;  //(EnumGSMediaType)pInfo->stDescri.eMediaType;

	CSliceFrame *pFrame; 
	pFrame = CSliceFrame::Create( iChn,eType );
	
	if( !pFrame )
	{
		GS_ASSERT(0);
		MY_LOG_FATAL( g_pLog,  _GSTX("Source(%lu): '%s' 分配内存失败.\n"), 
			m_iAutoID, m_strKey.c_str());
		return CISource::SRC_RET_EFLOWOUT;
	}

	pFrame->m_bSysHeader = TRUE;
	if( eERRNO_SUCCESS != pFrame->SetFrameData( pIOV[0].pBuffer, pIOV[0].iSize, TRUE ) )
	{
		GS_ASSERT(0);
		pFrame->UnrefObject();
		MY_LOG_FATAL( g_pLog,  _GSTX("Source(%lu): '%s' 分配内存失败.\n"), 
			m_iAutoID, m_strKey.c_str());
		return CISource::SRC_RET_EFLOWOUT;
	}

	for( int i = 1; i<iVNums; i++)
	{
		if( eERRNO_SUCCESS!= pFrame->AddFrameData( pIOV[i].pBuffer, pIOV[i].iSize ) )
		{
			GS_ASSERT(0);
			pFrame->UnrefObject();
			MY_LOG_FATAL( g_pLog,  _GSTX("Source(%lu): '%s' 分配内存失败.\n"), 
				m_iAutoID, m_strKey.c_str());
			return CISource::SRC_RET_EFLOWOUT;
		}
	}

	m_csWRMutex.LockWrite();	
	/*iChn*/
	for( UINT i = 0; i<m_csSysFrameCache[0].size(); i++ )
	{
		GS_ASSERT(m_csSysFrameCache[0][i]);
		m_csSysFrameCache[0][i]->UnrefObject();		
	}
	m_csSysFrameCache[0].clear();    

	m_csSysFrameCache[0].push_back(pFrame);
	pFrame->RefObject();
	m_csWRMutex.UnlockWrite();	

	CISource::EnumRetNO eRet = CISource::SRC_RET_EFLOWOUT;
	if( m_eTransModel==GSP_TRAN_RTPLAY )
	{

		if( m_csStreamTask.RSUCCESS == m_csStreamTask.Task(pFrame) )
		{

			return CISource::SRC_RET_SUCCESS;
		}	
	} 
	else
	{
		eRet = NotifyRefSource(pFrame);
	}
	pFrame->UnrefObject();
	return eRet;

}

void CBaseSource::SetUserData(void *pData)
{
	m_pUserData = pData;
}

void *CBaseSource::GetUserData(void )
{
	return m_pUserData;
}

void CBaseSource::Release(void)
{
	//通知所有引用者
	

	m_bValid = FALSE;

	if( m_pPullTimer )
	{
		m_pPullTimer->Stop();
	}

	m_csWRMutex.LockWrite();
	if( m_bRelease )
	{
		m_csWRMutex.UnlockWrite();
		UnrefObject(); 
		return;
	}
	m_bRelease = TRUE;
	for( UINT i = 0; i<m_csSysFrameCache.size(); i++ )
	{
		for( UINT j = 0; j<m_csSysFrameCache[i].size(); j++ )
		{
			GS_ASSERT(m_csSysFrameCache[i][j]);
			m_csSysFrameCache[i][j]->UnrefObject();			
		}
		m_csSysFrameCache[i].clear();
	}
	m_csWRMutex.UnlockWrite();

	NotifyRefSource( CRefSource::eEVT_SOURCE_RELEASE, NULL);    

	m_csStreamTask.Uninit();
	
	//通知上层删除数据源
	m_pServer->OnSourceEvent(this, eEVT_SRC_RELEASE, NULL);
	
	




	MY_LOG_DEBUG( g_pLog,  
		"Source(%u, %u, '%s' ) 被释放, SourceRef:%d Object Refs:%d\n",
		m_iAutoID, m_iSourceID,
		m_strKey.c_str(), GetSrcRefs(), GetRefCounts() );	
	UnrefObject(); 
	
	
}

const char *CBaseSource::GetKey(void)
{
	return m_strKey.c_str();
}


void CBaseSource::SetTransMode( INT32 eMode )
{
	m_eTransModel = eMode;
	if( eMode == GSP_TRAN_DOWNLOAD )
	{
		m_bEnableRef = FALSE;
	}
	else
	{
		m_bEnableRef = TRUE;
	}

	if( eMode == GSP_TRAN_RTPLAY )
	{
		m_iAbiliteMasrk &= ~(GSP_CTRL_FAST|GSP_CTRL_STEP|GSP_CTRL_SLOW|GSP_CTRL_SETPOINT);
	}
}	


EnumErrno CBaseSource::Ctrl(const StruGSPCmdCtrl &stCtrl)
{
	if( !m_bValid )
	{
		return eERRNO_SRC_EUNUSED;
	}

	if( 0 == (stCtrl.iCtrlID&m_iAbiliteMasrk) )
	{
		MY_LOG_WARN(g_pLog, _GSTX("Source(%lu %s) 收不支持的控制命令: 0x%x\n"),
			stCtrl.iCtrlID );
		return eERRNO_SYS_ENCTRL;
	}
	m_pServer->OnSourceEvent(this,eEVT_SRC_CTRL, (void*)&stCtrl);
	return eERRNO_SUCCESS;
}

EnumErrno CBaseSource::GetPlayStatus( StruPlayStatus *pStatus )
{
	GS_ASSERT(pStatus);
	::memcpy(pStatus, &m_stPlayStatus, sizeof(m_stPlayStatus));
	return eERRNO_SUCCESS;
}


CRefSource *CBaseSource::RefSource(const CIMediaInfo *pMediaInfo )
{

	if( !m_bValid )
	{
		return NULL;
	}	

	if( pMediaInfo && !m_csMediaInfo.IsSupport(pMediaInfo) )
	{
		GS_ASSERT(0);
		MY_LOG_FATAL(g_pLog, _GSTX("Source(%lu %s) 不提供转码.\n"), 
					m_iAutoID, m_strKey.c_str());
		return NULL;
	}

	CRefSource *pRet;
	pRet = new CRefSource(this);
	if( NULL==pRet )
	{
		GS_ASSERT(0);
		MY_LOG_FATAL(g_pLog, _GSTX("Source(%lu) 建立引用对象失败.\n"), m_iAutoID);
		return pRet;
	}

	m_csWRMutex.LockWrite();
	if( eERRNO_SUCCESS == m_csListener.AddTail(pRet) )
	{
		if( eERRNO_SUCCESS== m_csOriginalListener.AddTail(pRet) )
		{
			m_csWRMutex.UnlockWrite();
			m_pServer->OnSourceEvent(this, eEVT_SRC_REF, NULL); //通知服务增加引用
			return pRet;
		}
		m_csListener.Remove(pRet);
	}
	m_csWRMutex.UnlockWrite();
	UnrefSource(pRet);	
	delete pRet;
	return NULL;

}
void CBaseSource::UnrefSource(CRefSource *p)
{
	m_csWRMutex.LockWrite();
	m_csListener.Remove(p);
	m_csOriginalListener.Remove(p);
	CIStreamConverter *pCnv;
	pCnv = (CIStreamConverter *)p->GetSourcePrivate();
	if( NULL != pCnv )
	{
		//转码
		pCnv->RemoveRefSourceListener(p);
	}
	m_csWRMutex.UnlockWrite();	
	if( m_bValid )
	{
		m_pServer->OnSourceEvent(this, eEVT_SRC_UNREF, NULL); //通知服务减少引用
	}
}

INT32 CBaseSource::GetCtrlAbilities(void)
{
	return m_iAbiliteMasrk;
}

CISource::EnumRetNO CBaseSource::NotifyRefSource( CSliceFrame *pFrame)
{
	CRefSource *pRef;
	EnumErrno eRet = eERRNO_SRC_EUNUSED;
	StruGSListNode *pNode = NULL;
	UINT iSendCnts = 0;
	CGSAutoReaderMutex rlocker( &m_csWRMutex);
	CList::CIterator<CRefSource*> csIt;
	CList::CIterator<CSliceFrame*> csItFrame;
	for( csIt = m_csOriginalListener.First<CRefSource*>(); csIt.IsOk(); csIt.Next() )
	{
		pRef = csIt.Data();
		GS_ASSERT(pRef!=NULL);
		//实时流
		if( !pRef->IsRequestStream()  )
		{
			continue;
		}
		//发送信息头
		if( pRef->WaitSysHeader()  )
		{
			if( m_csSysFrameCache[0].size() ) /*pFrame->m_iChnID*/
			{

				eRet = pRef->OnSourceEvent( CRefSource::eEVT_STREAM_FRAME, 
					m_csSysFrameCache[0][0]);		
				pRef->EnableWaitSysHeader(eRet!=eERRNO_SUCCESS);
				if( eRet != eERRNO_SUCCESS )
				{
					continue;
				}
			}
			else
			{
				pRef->EnableWaitSysHeader(FALSE);
			}
		}

		if(  m_eTransModel == GSP_TRAN_RTPLAY )
		{

			

			if( pRef->IsQuickFrame() )
			{
				//发送缓冲数据
				for( csItFrame = m_csStreamCache.First<CSliceFrame*>(); 
					csItFrame.IsOk(); 
					csItFrame.Next() )
				{		
					 
					if( csItFrame.Data() == pFrame)
					{
						//当前帧
						break;
					}
					if( pRef->OnSourceEvent( CRefSource::eEVT_STREAM_FRAME,  csItFrame.Data()))
					{
						break;
					}
				} 
				pRef->EnableQuickFrame(FALSE);	
			}
		}
		eRet = pRef->OnSourceEvent(CRefSource::eEVT_STREAM_FRAME,  pFrame);
		if( eRet == eERRNO_SUCCESS )
		{
			iSendCnts ++;
		}
	}

	
	//码流转换
	CIStreamConverter *pCnv;
	for( std::vector<CIStreamConverter*>::iterator csItCnv =  m_csConverter.begin(); 
		 csItCnv != m_csConverter.end(); ++csItCnv )
	{
		pCnv = *csItCnv;
		GS_ASSERT(pCnv!=NULL);
		//实时流
		if( !pCnv->IsRequestStream()  )
		{
			continue;
		}

		//发送信息头
		if( pCnv->WaitSysHeader()  )
		{
			if( m_csSysFrameCache[0].size() ) /*pFrame->m_iChnID*/
			{

				pCnv->InputStreamFrame( m_csSysFrameCache[0][0]);		
				pRef->EnableWaitSysHeader(FALSE);					
			}
		}

		if(  m_eTransModel == GSP_TRAN_RTPLAY )
		{

			if( pRef->IsQuickFrame() )
			{
				//发送缓冲数据
				for( csItFrame = m_csStreamCache.First<CSliceFrame*>(); 
					csItFrame.IsOk(); csItFrame.Next() )
				{					
					if( csItFrame.Data() == pFrame)
					{
						//当前帧
						break;
					}
					pCnv->InputStreamFrame( csItFrame.Data() );		
				    pCnv->EnableQuickFrame(FALSE);	
				}            
			}			
		}
		iSendCnts ++;
		pCnv->InputStreamFrame( pFrame );
	}
		 
	return iSendCnts==0 ? CISource::SRC_RET_EUNUSED : CISource::SRC_RET_SUCCESS;
}

 CISource::EnumRetNO  CBaseSource::NotifyRefSource( INT eRefSourceEvent, void *pParam)
 {
	 CList::CIterator<CRefSource*> csIt;
	CRefSource *pRef;
	 EnumErrno eErrno = eERRNO_SUCCESS;
	 m_csWRMutex.LockReader();
	 for( csIt = m_csOriginalListener.First<CRefSource*>(); csIt.IsOk(); )
	 {
		 pRef = csIt.Data();
		  csIt.Next();
		 m_csWRMutex.UnlockReader();
		
		 GS_ASSERT(pRef!=NULL);
		 if( eERRNO_SUCCESS!= pRef->OnSourceEvent((CRefSource::EnumRefSourceEvent)eRefSourceEvent,
					pParam ) )
		 {
			 eErrno = eERRNO_EUNKNOWN;
		 }
		  m_csWRMutex.LockReader();
	 }
	 m_csWRMutex.UnlockReader();

	
	return eErrno == eERRNO_SUCCESS ? CISource::SRC_RET_EUNUSED : CISource::SRC_RET_SUCCESS;
 }

void CBaseSource::SetKeyInfo( const char *szKey, UINT16 iSourceID)
{
	m_strKey = szKey;
	m_iSourceID = iSourceID;
	MY_LOG_INFO(g_pLog, _GSTX("Source(%lu) 被绑定到: SourceID:%u, Key:'%s'\n"),
					m_iAutoID, m_iSourceID, m_strKey.c_str());
}

 void CBaseSource::OnStreamTaskPoolEvent( CObjThreadPool *pTkPool, void *pData )
 {
CSliceFrame *pFrame = static_cast<CSliceFrame*>(pData);
	
	NotifyRefSource(pFrame);
	pFrame->UnrefObject();
 }


  void CBaseSource::OnPullTimerEvent(  CWatchTimer *pTimer  )
  {
	  if( !m_bValid || !m_fnCallback )
	  {
		  return;
	  }

	  INT iTrys = 10;
	  INT iInterval = 6; 
	  do 
	  {
		  iInterval = (INT) (*m_fnCallback)(this, CISource::eEVT_REQUEST_FRAME, m_pUserArgs);
	  } while ( iTrys-- && iInterval == 0 && m_bValid );

	  if( iInterval > 0 )
	  {
		  pTimer->AlterTimer(iInterval);
	  }
	  else if( iInterval < 0 )
	  {
		  pTimer->Stop();
	  }
	  else
	  {
		  pTimer->AlterTimer(10);
	  }


  }