#include "VIOCPSimulator.h"
#include "Log.h"
#include "OSThread.h"
#include "ISocket.h"


using namespace GSP;

#define READ_MASK  ((void*)1)
#define WRITE_MASK  ((void*)2)

#define PIPE_READ 0
#define PIPE_WRITE 1


#define eEVT_SOCKET_MASK_RCVFROM  (eEVT_SOCKET_MASK_READ|0x4)

namespace GSP
{


	class CSkEvtClient : public CGSPObject
	{    

	public :	
		CGSCondEx csExitCond;
		CGSMutex  csMutex;

		UINT iBusyRefs;

	
		std::vector<CProPacket *> vSendPackets;
		INT iSendPktIdx;
		void *pSendKey;
		UINT64 iSendBytes;

		struct iovec stVSendIO[3];
		UINT iVSendIOCounts;	
		UINT iVSendIOCurIdx;

		EnumSocketEventMask iEvtRequest; //请求的事件


		BOOL bUnregister;
		BOOL bWaitUnregister;

		CGSPThreadPool csWTaskPool;

		SOCKET sk;
		CISocket *pSocket;

		CVIOCPSimulator *pGIO;

		void *pChildClassData;
		FuncPtrFree fnFreeChildClassData;

		CSkEvtClient(CVIOCPSimulator *pIO)
			:csExitCond()
			,csMutex()			
		{

			iEvtRequest = eEVT_SOCKET_MASK_NONE;

			bUnregister = FALSE; 
			iBusyRefs = 0;

			bWaitUnregister = 0;
			iVSendIOCounts = 0;				
			iVSendIOCurIdx = 0;

		
			iSendPktIdx = 0;
			iSendBytes = 0;		
			pSendKey = NULL;
		

			sk = INVALID_SOCKET;

			pSocket = NULL;
			pGIO = pIO;

			pChildClassData = NULL;
			fnFreeChildClassData = NULL;

		}

		~CSkEvtClient(void)
		{	

		}	

		void Release(void)
		{
			iEvtRequest = eEVT_SOCKET_MASK_NONE;
			csWTaskPool.Uninit();
			sk = INVALID_SOCKET;

			csExitCond.BroadcastSignal();
			
			iSendPktIdx = 0;
			iSendBytes = 0;
			iVSendIOCurIdx = 0;

			for( UINT i = 0; i<vSendPackets.size(); i++ )
			{
				vSendPackets[i]->UnrefObject();
			}
			vSendPackets.clear();

			if( fnFreeChildClassData )
			{
				fnFreeChildClassData(pChildClassData);
				fnFreeChildClassData = NULL;
				pChildClassData = NULL;
			}
			delete this;
		}


		INLINE void SetSendData(void *pKey, std::vector<CProPacket *> vPkt)
		{

			GS_ASSERT(vSendPackets.empty() && iVSendIOCounts==0  );
			vSendPackets = vPkt;
			for( UINT i = 0; i<vPkt.size(); i++ )
			{
				vPkt[i]->RefObject();
			}
			pSendKey = pKey;
			iSendPktIdx = 0;
			iSendBytes = 0;
			iVSendIOCounts = 0;
			iVSendIOCurIdx = 0;
			
		}

		INLINE void SetSendData(void *pKey, std::vector<CProFrame *> vFrame )
		{
			GS_ASSERT(vSendPackets.empty() && iVSendIOCounts==0 );
			
			CProPacket **ppPks = NULL;
			UINT iCounts;
			for( UINT i = 0; i<vFrame.size(); i++ )
			{
				ppPks = NULL;
				iCounts = vFrame[i]->GetPackets(&ppPks);
				for( UINT j = 0; j<iCounts; j++ )
				{
					vSendPackets.push_back(ppPks[j]);
					ppPks[j]->RefObject();
				}
			}
			pSendKey = pKey;
			iSendPktIdx = 0;
			iSendBytes = 0;
			iVSendIOCounts = 0;
			iVSendIOCurIdx = 0;
		}

		INLINE BOOL PrepareSendData(void)
		{
			if( iSendPktIdx>= (INT) vSendPackets.size() )
			{
				return FALSE;
			}

			iVSendIOCounts = 0;
			iVSendIOCurIdx = 0;
			while( iVSendIOCounts<ARRARY_SIZE(stVSendIO) && iSendPktIdx< (INT) vSendPackets.size() )
			{
				const StruPktInfo &stInfo = vSendPackets[iSendPktIdx]->GetParser();
				/*GS_ASSERT(stInfo.iHeaderSize==10);*/
				if( stInfo.iHeaderSize > 0 )
				{

					stVSendIO[iVSendIOCounts].iov_base = (char*) stInfo.bHeader;
					stVSendIO[iVSendIOCounts].iov_len = stInfo.iHeaderSize;
					iVSendIOCounts++;
				}
				if( stInfo.iPlayloadSize > 0 )
				{
					stVSendIO[iVSendIOCounts].iov_base = (char*) stInfo.bPlayload;
					stVSendIO[iVSendIOCounts].iov_len = stInfo.iPlayloadSize;
					iVSendIOCounts++;
				}
				if( stInfo.iTailerSize > 0 )
				{
					stVSendIO[iVSendIOCounts].iov_base = (char*) stInfo.bTailer;
					stVSendIO[iVSendIOCounts].iov_len = stInfo.iTailerSize;
					iVSendIOCounts++;
				}
				iSendPktIdx++;
			}
			return TRUE;
		}

		INLINE void ClearSendData(void)
		{		

			for( UINT i = 0; i<vSendPackets.size(); i++ )
			{
				vSendPackets[i]->UnrefObject();
			}
			vSendPackets.clear();
			iSendPktIdx = 0;
			iSendBytes = 0;
			iVSendIOCounts = 0;
			iVSendIOCurIdx = 0;
		}

		BOOL OnUnregisterEvent(void)
		{
			BOOL bSelfThread = csWTaskPool.IsSelfThread();
			csMutex.Lock();
			bUnregister = TRUE;
			iEvtRequest = eEVT_SOCKET_MASK_NONE;
			sk = INVALID_SOCKET;
			csMutex.Unlock();
			csWTaskPool.Uninit();
			return bSelfThread;
		}

		static CSkEvtClient *Create(CVIOCPSimulator *pIO,
			SOCKET hSocket , CISocket *pHSocket, CNetError &csError)
		{
			CSkEvtClient *pRet = new CSkEvtClient(pIO);
			if( !pRet )
			{
				csError.m_eErrno = eERRNO_SYS_ENMEM;
				csError.m_iSysErrno = 0;
				csError.m_strError = _GSTX("内存分配失败");
				return NULL;
			}
			if( !pRet->csWTaskPool.Init(pRet,
				(FuncPtrObjThreadPoolEvent)&CSkEvtClient::OnWriteTaskEvent, 1, FALSE) )
			{
				csError.m_eErrno = eERRNO_SYS_ETKP;
				csError.m_iSysErrno = 0;
				csError.m_strError = _GSTX("线程池Init失败");
				pRet->Release();
				return NULL;
			}
			pRet->sk = hSocket;
			pRet->pSocket = pHSocket;
			return pRet;
		}

		void OnWriteTaskEvent( CObjThreadPool *pThread, void *pArg )
		{

			INT iRet = 0;
			StruAsyncSendEvent stEvt;
			CGSAutoMutex locker(&csMutex);
			csMutex.Lock();
			if( bUnregister || sk==INVALID_SOCKET )
			{
				//已经停止
				ClearSendData();			
				return;
			}

			if( vSendPackets.empty() )
			{
				//没有数据			
				return;
			}
			iBusyRefs++;
			csMutex.Unlock();
try_write :			
			if( iVSendIOCurIdx<iVSendIOCounts  )
			{

				iRet = ::send(sk, stVSendIO[iVSendIOCurIdx].iov_base, stVSendIO[iVSendIOCurIdx].iov_len, 0);
				if( iRet >0 )
				{
					stVSendIO[iVSendIOCurIdx].iov_base+=iRet;
					stVSendIO[iVSendIOCurIdx].iov_len -= iRet;
					iSendBytes+= iRet;
					if( stVSendIO[iVSendIOCurIdx].iov_len==0 )
					{
						iVSendIOCurIdx++;
						goto try_write;
					}
				}
				else if( iRet == 0 )
				{
					//缓冲区已经满了					
					if( pGIO->AddWatch(sk, eEVT_SOCKET_MASK_WRITE, TRUE) )
					{
						//失败
						GS_ASSERT(0);
						pSocket->OnAIOEventError(CNetError(eERRNO_NET_ESEND, 0, _GSTX("注册写消息失败")));
					}
				}
			}
			else
			{
				//发送完成
				if( PrepareSendData() )
				{
					//还有数据
					goto try_write;
				}
				else
				{
					//完成发送
					stEvt.pKey  = pSendKey;
					stEvt.iSends = iSendBytes;		
					csMutex.Lock();
					ClearSendData();
					csMutex.Unlock();
					pSocket->OnAIOEventSend(stEvt);
				}
			}
			csMutex.Lock();
			iBusyRefs--;
		}
	};   


	static void SafeFreePrivateData( CSkEvtClient **ppData )
	{
		if( ppData && *ppData )
		{
			GS_ASSERT((*ppData)->iBusyRefs==0);
			(*ppData)->Release();
			*ppData = NULL;
		}
	}


} //end namespace GSP


#define MY_PRI(pPri)  ((CSkEvtClient*)(pPri));


void *CVIOCPSimulator::GetChildClassPriData( CISocket *pSocket )
{
	CSkEvtClient *p = MY_PRI(pSocket->m_pEvtPri);
	if( p )
	{
		return p->pChildClassData;
	}
	return NULL;
}

CVIOCPSimulator::CVIOCPSimulator(INT iID, UINT iMaxChannels)
:CIAsyncIO(iID)  
,m_csWatchTaskPool("VIOCPSimulator")
{
	m_iMaxChannels = iMaxChannels;

	m_hPipe[PIPE_READ] = INVALID_SOCKET;
	m_hPipe[PIPE_WRITE] = INVALID_SOCKET;	
	m_bRunning = TRUE;

	m_csWatchTaskPool.Init(this, (FuncPtrObjThreadPoolEvent)&CVIOCPSimulator::OnWatchTaskEvent, 1, TRUE);

}

CVIOCPSimulator::~CVIOCPSimulator(void)
{

	m_bRunning = FALSE;
	Wakeup();
	for( int i = 0; i<ARRARY_SIZE(m_hPipe); i++ )
	{
		if( m_hPipe[i]!=INVALID_SOCKET )
		{
			closesocket(m_hPipe[i]);
			m_hPipe[i] = INVALID_SOCKET;
		}
	}	
	m_csWatchTaskPool.Uninit();
}

BOOL CVIOCPSimulator::Register(CISocket *pSocket, CNetError &csError)
{
	GS_ASSERT_RET_VAL( NULL ==pSocket->m_pEvtPri, FALSE );

	SOCKET sk = pSocket->GetOSSocket();
	CSkEvtClient *pPri;
	pPri = pPri->Create(this, sk, pSocket, csError);
	if( !pPri )
	{
		return FALSE;
	}


	EnumSocketType eSkType = pSocket->SocketType();
	if( eSkType == eSOCKET_TCP_SERVER  )
	{
		GS_ASSERT(0);
		csError.m_eErrno = eERRNO_SYS_EOPER;
		csError.m_strError = _GSTX("IOCP not support for tcp server");
		//监听端口不使用完成端口
		return FALSE;
	}
	CGSWRMutexAutoWrite wlocker(&m_csWRMutexChn);
	if( m_mapChannels.find(sk) != m_mapChannels.end() )
	{
		//已经存在
		GS_ASSERT(0);
		csError.m_eErrno = eERRNO_SYS_EEXIST;
		csError.m_strError = _GSTX("已经注册过");
		return FALSE;
	}
	void *pChildData = NULL;
	FuncPtrFree fnFreeChidPri = NULL;
	csError.m_eErrno = Watch(sk, TRUE,&pChildData, &fnFreeChidPri );
	if( csError.m_eErrno )
	{
		//失败
		SafeFreePrivateData(&pPri);		
		csError.m_iSysErrno = 0;
		csError.m_strError = _GSTX("注册异步事件失败");
		return FALSE;
	}
	pPri->pChildClassData = pChildData;
	pPri->fnFreeChildClassData = fnFreeChidPri;
	pSocket->RefObject();
	pSocket->m_pEvtPri = pPri;
	pSocket->m_fnSaftFreeEvtPri = (void(*)(void**)) SafeFreePrivateData;
	m_mapChannels.insert(make_pair(pPri->sk, pSocket));
	return TRUE;

}



void CVIOCPSimulator::Unregister( CISocket *pSocket )
{
	GS_ASSERT_RET(pSocket->m_pEvtPri); 
	CSkEvtClient *pPri = MY_PRI(pSocket->m_pEvtPri);
	if( pPri->bUnregister )
	{
		GS_ASSERT(0);
		return ;
	}
	
	SOCKET sk =  pPri->sk;

	BOOL bSelfThread = pPri->OnUnregisterEvent();

	pPri->csMutex.Lock();	
	if( INVALID_SOCKET == sk || m_mapChannels.find(sk)==m_mapChannels.end() )
	{
		GS_ASSERT(0);
		pPri->csMutex.Unlock();
		return ;
	}

	pSocket->Shutdown();
	pPri->bWaitUnregister = TRUE; 
	pPri->bUnregister = TRUE;

	UninitWatch(sk, TRUE);


	GS_ASSERT(!bSelfThread);

	while( pPri->iSendBytes )
	{		
		if( bSelfThread && pPri->iBusyRefs == 1 )
		{
			break;
		}
		pPri->csExitCond.WaitTimeout(&pPri->csMutex, 10);
	}
	pPri->bWaitUnregister = FALSE;
	m_mapChannels.erase(sk);
	pPri->sk = INVALID_SOCKET;
	pPri->csMutex.Unlock();
	pSocket->UnrefObject();

}

INT  CVIOCPSimulator::GetMaxChannelContain(void)
{
	return m_iMaxChannels;
}

EnumErrno CVIOCPSimulator::AsyncRcvFrom(CISocket *pSocket,BOOL bStart)
{
	GS_ASSERT_RET_VAL(pSocket->m_pEvtPri, eERRNO_NET_EREG); 
	CSkEvtClient *pPri = MY_PRI(pSocket->m_pEvtPri);
	EnumErrno eRet;

	CGSAutoMutex locker( &pPri->csMutex);

	if( pPri->bUnregister  )
	{
		MY_LOG_ERROR(g_pLog,  _GSTX("CVIOCPSimulator Sk:%d,pri:0x%p 异步From接收失败. 异步IO没有注册.\n"),
			(int)  pSocket->GetOSSocket() ,pPri ); 		
		return eERRNO_NET_EREG;
	}

	if( !IS_UDP_SOCKET(pSocket->SocketType( )) )
	{
		MY_LOG_ERROR(g_pLog,  _GSTX("CVIOCPSimulator Sk:%d,pri:0x%p 异步From接收失败. 非UDP 连接.\n"),
			(int)  pSocket->GetOSSocket() ,pPri ); 

		return eERRNO_NET_EREG;
	}
	if( bStart )
	{
		if( pPri->iEvtRequest&eEVT_SOCKET_MASK_RCVFROM )
		{
			pPri->csMutex.Unlock();
			MY_LOG_WARN(g_pLog,  _GSTX("CVIOCPSimulator Sk:%d,pri:0x%p异步From接收事件已经进行.\n"),
				(int)  pSocket->GetOSSocket() ); 
			return eERRNO_SUCCESS;
		}
		pPri->iEvtRequest |= eEVT_SOCKET_MASK_RCVFROM;
		eRet = AddWatch(pPri->sk, eEVT_SOCKET_MASK_READ, TRUE);
		if( eRet )
		{
			MY_LOG_ERROR(g_pLog,  _GSTX("CVIOCPSimulator Sk:%d,pri:0x%p  注册异步读消息事件失败.\n"),
				(int)  pPri->sk ,pPri ); 
			pPri->iEvtRequest &= ~eEVT_SOCKET_MASK_RCVFROM;
			return eRet;
		}
	}
	else  if( pPri->iEvtRequest&eEVT_SOCKET_MASK_RCVFROM )
	{
		CleanWatch(pPri->sk, eEVT_SOCKET_MASK_READ, TRUE);
		pPri->iEvtRequest &= ~eEVT_SOCKET_MASK_RCVFROM;
	}
	return eERRNO_SUCCESS;
}


EnumErrno CVIOCPSimulator::AsyncRcv(CISocket *pSocket,	BOOL bStart)
{

	GS_ASSERT_RET_VAL(pSocket->m_pEvtPri, eERRNO_NET_EREG); 
	CSkEvtClient *pPri = MY_PRI(pSocket->m_pEvtPri);
	EnumErrno eRet;

	CGSAutoMutex locker( &pPri->csMutex);

	if( pPri->bUnregister  )
	{
		MY_LOG_ERROR(g_pLog,  _GSTX("CVIOCPSimulator Sk:%d,pri:0x%p 异步接收失败. 异步IO没有注册.\n"),
			(int) pPri->sk ,pPri ); 

		return eERRNO_NET_EREG;
	}

	if( !IS_UDP_SOCKET(pSocket->SocketType( )) )
	{
		MY_LOG_ERROR(g_pLog,  _GSTX("CVIOCPSimulator Sk:%d,pri:0x%p 异步接收失败. 非UDP 连接.\n"),
			(int) pPri->sk ,pPri ); 

		return eERRNO_NET_EREG;
	}
	if( bStart )
	{
		if( pPri->iEvtRequest&eEVT_SOCKET_MASK_READ )
		{
			pPri->csMutex.Unlock();
			MY_LOG_WARN(g_pLog,  _GSTX("CVIOCPSimulator Sk:%d,pri:0x%p 异步接收事件已经进行.\n"),
				(int) pPri->sk , pPri); 
			return eERRNO_SUCCESS;
		}
		pPri->iEvtRequest |= eEVT_SOCKET_MASK_READ;
		eRet = AddWatch(pPri->sk, eEVT_SOCKET_MASK_READ, TRUE);
		if( eRet )
		{
			MY_LOG_ERROR(g_pLog,  _GSTX("CVIOCPSimulator Sk:%d,pri:0x%p  注册异步读消息事件失败.\n"),
				(int) pPri->sk ,pPri ); 
			pPri->iEvtRequest &= ~eEVT_SOCKET_MASK_READ;
			return eRet;
		}
	}
	else  if( pPri->iEvtRequest&eEVT_SOCKET_MASK_READ )
	{
		CleanWatch(pPri->sk, eEVT_SOCKET_MASK_READ, TRUE);
		pPri->iEvtRequest &= ~eEVT_SOCKET_MASK_READ;
	}
	return eERRNO_SUCCESS;
}

EnumErrno CVIOCPSimulator::AsyncAccept(CISocket *pSocket,BOOL bStart)
{
	GS_ASSERT(0);
	return eERRNO_SYS_EFUNC;
}

EnumErrno CVIOCPSimulator::AsyncSend(CISocket *pSocket, void *pKey, std::vector<CProPacket *> vProPacket )
{
	GS_ASSERT_RET_VAL(pSocket->m_pEvtPri, eERRNO_NET_EREG); 
	CSkEvtClient *pPri = MY_PRI(pSocket->m_pEvtPri);
	CGSAutoMutex locker( &pPri->csMutex);

	if( pPri->bUnregister  )
	{
		MY_LOG_ERROR(g_pLog,  _GSTX("CVIOCPSimulator Sk:%d,pri:0x%p 异步发送失败. 异步IO没有注册.\n"),
			(int) pPri->sk ,pPri ); 		
		return eERRNO_NET_EREG;
	}
	if( !pPri->vSendPackets.empty() )
	{
		//已经在发生
		MY_LOG_ERROR(g_pLog,  _GSTX("CVIOCPSimulator Sk:%d,pri:0x%p 异步发送失败. 数据已经存在.\n"),
			(int) pPri->sk ,pPri); 
		GS_ASSERT(0);	
		return eERRNO_SYS_EEXIST;
	}

	pPri->SetSendData(pKey, vProPacket);
	if( !pPri->PrepareSendData() )
	{
		//没有数据
		GS_ASSERT(0);
		pPri->ClearSendData();
		pPri->csMutex.Unlock();
		return eERRNO_SUCCESS;
	}
	if( pPri->csWTaskPool.RSUCCESS ==pPri->csWTaskPool.Task((void*)1) )
	{
		return eERRNO_SUCCESS;	
	}
	pPri->ClearSendData();
	return eERRNO_SYS_ENMEM;
}

EnumErrno CVIOCPSimulator::AsyncSend(CISocket *pSocket, void *pKey,std::vector<CProFrame *> vProFrame)
{
	GS_ASSERT_RET_VAL(pSocket->m_pEvtPri, eERRNO_NET_EREG); 
	CSkEvtClient *pPri = MY_PRI(pSocket->m_pEvtPri);
	CGSAutoMutex locker( &pPri->csMutex);
	if( pPri->bUnregister  )
	{
		MY_LOG_ERROR(g_pLog,  _GSTX("CVIOCPSimulator Sk:%d,pri:0x%p 异步发送失败. 异步IO没有注册.\n"),
			(int) pPri->sk ,pPri ); 		
		return eERRNO_NET_EREG;
	}
	if( !pPri->vSendPackets.empty() )
	{
		//已经在发生
		MY_LOG_ERROR(g_pLog,  _GSTX("CVIOCPSimulator Sk:%d,pri:0x%p 异步发送失败. 数据已经存在.\n"),
			(int) pPri->sk ,pPri); 
		GS_ASSERT(0);	
		return eERRNO_SYS_EEXIST;
	}

	pPri->SetSendData(pKey, vProFrame);
	if( !pPri->PrepareSendData() )
	{
		//没有数据
		GS_ASSERT(0);
		pPri->ClearSendData();
		pPri->csMutex.Unlock();
		return eERRNO_SUCCESS;
	}
	if( pPri->csWTaskPool.RSUCCESS ==pPri->csWTaskPool.Task((void*)1) )
	{
		return eERRNO_SUCCESS;	
	}
	pPri->ClearSendData();
	return eERRNO_SYS_ENMEM;
}


INT CVIOCPSimulator::GetEmptyCounts(void)
{	
	m_csWRMutexChn.LockReader();
	INT iRet = m_iMaxChannels-m_mapChannels.size();	
	m_csWRMutexChn.UnlockReader();
	return iRet;
}

SOCKET CVIOCPSimulator::GetPipeReadSocket(void) const
{
	return m_hPipe[PIPE_READ];
}

BOOL CVIOCPSimulator::Init(void)
{
	UINT iMax = m_iMaxChannels*3+1;
	if( iMax<100 ) 
	{
		iMax = 100;
	}
	if( !m_vEvtContains.Init(iMax) )
	{
		GS_ASSERT(0);
		return FALSE;
	}

	if( !CreatePipe() )
	{
		GS_ASSERT(0);
		return FALSE;
	}
	if( AddWatch(m_hPipe[PIPE_READ], eEVT_SOCKET_MASK_READ, FALSE) )
	{
		GS_ASSERT(0);
		return FALSE;
	}
	if( !m_csWatchTaskPool.IsInit() )
	{
		GS_ASSERT(0);
		return FALSE;
	}
	if( m_csWatchTaskPool.RSUCCESS !=m_csWatchTaskPool.Task((void*) 1 ) )
	{
		GS_ASSERT(0);
		return FALSE;
	}	
	return TRUE;

}

void CVIOCPSimulator::Uninit(void)
{
	m_bRunning = FALSE;
	Wakeup();	
	m_csWatchTaskPool.Uninit(FALSE);
	
}


void CVIOCPSimulator::OnWatchTaskEvent(CObjThreadPool *pThread, void *pArg )
{
	UINT iIndex;  
	UINT iCounts;
	EnumErrno iErr;
	StructSimEvents *pEvt;
	CNetError csError( eERRNO_NET_EUNK, 0, _GSTX("GIOCP fail.") );
	CISocket *pSocket;
	CMapOfSocket::iterator csIt;
	CSkEvtClient *pPri;  
	CFixedBuffer *pRcvBuffer;
	INT iRet;
	StruLenAndSocketAddr  stRcvFromAddr;

	while( m_bRunning && g_bModuleRunning )
	{

		m_vEvtContains.Reset();
		iErr = WaitEvent(m_vEvtContains);
		if( iErr != eERRNO_SUCCESS )
		{
			if( eERRNO_EFATAL == iErr )
			{
				MY_LOG_FATAL(g_pLog, _GSTX("Wait event err. %d\n"),iErr );     
				GS_ASSERT(0);
			}
			MSLEEP(10); //防止死循环
			continue;
		}
		iCounts = m_vEvtContains.Size();

		for(iIndex=0; iIndex<iCounts; iIndex++ )
		{
			pEvt = m_vEvtContains.Get(iIndex);
			GS_ASSERT(pEvt);
			if( pEvt->fd == m_hPipe[PIPE_READ] )
			{
				//PIPE 读事件
				if( m_bRunning )
				{

					while( m_bRunning && 0 < ::recv(m_hPipe[PIPE_READ],(char*)m_bPipeBuffer, SKEVT_PIPE_BUFF_SIZE, 0 ) )
					{  
					}					
				}
			}
			else
			{
				m_csWRMutexChn.LockReader();
				csIt=m_mapChannels.find(pEvt->fd);
				if( csIt==m_mapChannels.end() )
				{
					m_csWRMutexChn.UnlockReader();
					goto next_loop;
				}
				pSocket = csIt->second;
				pPri = MY_PRI(pSocket->m_pEvtPri);

				pPri->csMutex.Lock();
				pPri->iBusyRefs++;
				pPri->csMutex.Unlock();
				m_csWRMutexChn.UnlockReader();

				if( pEvt->iEvtMask&eEVT_SOCKET_MASK_READ )
				{
					//读数据
					pRcvBuffer =pRcvBuffer->Create();
					if( !pRcvBuffer )
					{
						MY_LOG_FATAL(g_pLog,  
							_GSTX("IOCPSim  处理连接 %s读取失败. 分配内存失败.\n"),
							pSocket->GetDescri().c_str() );
						GS_ASSERT(0);
						csError.m_eErrno = eERRNO_SYS_ENMEM;
						csError.m_iSysErrno = 0;
						csError.m_strError = _GSTX("分配内存失败");
						pSocket->OnAIOEventError(csError);
						UninitWatch(pPri->sk, FALSE);
						goto unref_object;
					}

					if(  eEVT_SOCKET_MASK_RCVFROM==(pPri->iEvtRequest&eEVT_SOCKET_MASK_RCVFROM) )
					{
						stRcvFromAddr.Reset();
						iRet = ::recvfrom(pEvt->fd, (char*) pRcvBuffer->m_bBuffer,
							pRcvBuffer->m_iBufferSize,0, &stRcvFromAddr.sn.sa, &stRcvFromAddr.len);
						if( iRet > 0 )
						{
							pRcvBuffer->m_iDataSize = iRet;
							pSocket->OnAIOEventRcvFrom(pRcvBuffer, &stRcvFromAddr);
						}						
					}
					else
					{
						iRet = ::recv(pEvt->fd,(char*) pRcvBuffer->m_bBuffer,
							pRcvBuffer->m_iBufferSize, 0);
						if( iRet > 0 )
						{
							pRcvBuffer->m_iDataSize = iRet;
							pSocket->OnAIOEventRcv(pRcvBuffer);
						}	
						else if( IS_TCP_SOCKET(pSocket->SocketType()) && pSocket->IsValid() )
						{
							//断开
							if( iRet == 0 )
							{
								csError.m_eErrno = eERRNO_NET_EDISCNN;
								csError.m_iSysErrno = 0;
								csError.m_strError = _GSTX("远程断开连接");
							}
							else
							{
								csError.m_eErrno = eERRNO_NET_ERCV;
								csError.m_iSysErrno = COSSocket::GetSocketErrno();
								csError.m_strError = _GSTX("读取数据失败");
							}
							pSocket->OnAIOEventError(csError);							
							pRcvBuffer->UnrefObject();
							UninitWatch(pPri->sk, FALSE);
							goto unref_object;
						}						
					}
					pRcvBuffer->UnrefObject();

					if( !pSocket->IsValid() || pPri->bUnregister ||
						0 == (pPri->iEvtRequest&eEVT_SOCKET_MASK_READ ) )
					{
						CleanWatch(pEvt->fd, eEVT_SOCKET_MASK_READ, FALSE);
					}
				}

				if( pEvt->iEvtMask&eEVT_SOCKET_MASK_WRITE )
				{
					pPri->csWTaskPool.Task((void*)1);		
					CleanWatch(pEvt->fd, eEVT_SOCKET_MASK_WRITE, FALSE);
				}
				goto unref_object;
			}			
		}
next_loop :
		continue;
unref_object :
		GS_ASSERT( pPri );

		pPri->csMutex.Lock();
		GS_ASSERT(pPri->iBusyRefs);
		pPri->iBusyRefs--;
		pPri->csMutex.Unlock();	
		continue;
	}
}




BOOL CVIOCPSimulator::Wakeup(void)
{
	if( m_hPipe[PIPE_WRITE]!=INVALID_SOCKET )
	{
		char c = 'a';
		return (1==send(m_hPipe[PIPE_WRITE],&c, 1,0));
	}
	return FALSE;
}

BOOL CVIOCPSimulator::CreatePipe(void)
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
