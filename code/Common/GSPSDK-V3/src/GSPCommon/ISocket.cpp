#define OBJECT_DEBUG
#include "ISocket.h"
#include "IAsyncIO.h"
#include "Log.h"
#include "OSThread.h"


using namespace GSP;
#include "MyObjectDebug.h"


static INT s_iErrTask = 0xFFFFFFFF;

#define IS_ERR_TASK_EVENT(pV) (((void*)(pV)) == (void*)&s_iErrTask)
#define TASK_EVENT   ((void*)&s_iErrTask)

DEFINE_OBJ_DEBUG(CISocket)

typedef struct _StruISkEventData
{
	EnumSocketEvent eEvt;
	union
	{
		 CGSPBuffer *pRcvBuffer;
		 StruAsyncSendEvent stSendEvt;
		 CNetError *pError;
		 CISocket *pSocket;
	}Args;
	StruLenAndSocketAddr stAddr;

	void Init(void)
	{
		bzero(&Args, sizeof(Args));
		stAddr.Reset();
	}

	void Clear(void)
	{
		switch(eEvt)
		{
		case eEVT_SOCKET_ARCV :
		case eEVT_SOCKET_ARCVFROM :
			{
				if( Args.pRcvBuffer  )
				{
					Args.pRcvBuffer->UnrefObject();
					Args.pRcvBuffer = NULL;
				}
			}
			break;
		case eEVT_SOCKET_ASEND :
			{

			}
			break;
		case eEVT_SOCKET_ERR :
			{
				if( Args.pError )
				{
					delete Args.pError;
					Args.pError = NULL;
				}
			}
			break;
		case eEVT_SOCKET_ACCEPT :
			{
				if( Args.pSocket )
				{
					Args.pSocket->Disconnect();
					Args.pSocket->Release();
					Args.pSocket = NULL;
				}
			}
			break;
		default:
			{

			}
		}
		Init();
	}
}StruISkEventData;


static void _FreeTaskData(StruISkEventData *p)
{
	if( p )
	{
		p->Clear();
		CMemoryPool::Free(p);
	}
}

static StruISkEventData *_CreateTaskData(void)
{
	StruISkEventData *p = (StruISkEventData *)CMemoryPool::Malloc(sizeof(StruISkEventData));
	if( p )
	{
		p->Init();
	}
	return p;
}

CISocket::CISocket(EnumSocketType eSkType, SOCKET hSocket  )
:COSSocket(eSkType, hSocket)
,m_csTaskPool("ISocket")
{
    OBJ_DEBUG_NEW(CISocket)
    m_bNoShutdown = TRUE;  
    m_pEvtPri = NULL;
    m_iRemotePort = 0;     
    m_pEventObject = NULL;
    m_pEventFun = NULL;
    m_pAIO = NULL;
    m_pUserData = NULL;  
	m_bRcvStart = FALSE;
	m_bAcceptStart = FALSE;

    if( !m_csTaskPool.Init(this, 
						(FuncPtrObjThreadPoolEvent)&CISocket::OnTaskPoolEvent, 
						1, FALSE) )
	{
		GS_ASSERT_EXIT( 0, -1); 
	}
	m_csTaskPool.SetFreedTaskDataFunction((FuncPtrFree)&_FreeTaskData);
    //m_csTaskPool.SetMaxWaitTask(1024);
	m_bRegistered = FALSE;
	m_fnSaftFreeEvtPri = NULL;
	m_pEvtPri = NULL;

	m_iUdpLastSendTime = 0;
	m_iUdpSendBytes = 0;
}

CISocket::~CISocket(void)
{   
	
    Disconnect();
	m_csTaskPool.Uninit();

	if( m_fnSaftFreeEvtPri )
	{
		m_fnSaftFreeEvtPri(&m_pEvtPri);
	}

    OBJ_DEBUG_DEL(CISocket)
}

void CISocket::SetListener(CGSPObject *pEvtFunOnwer, FuncPtrSocketEvent fnOnEvent)
{
    m_pEventObject = pEvtFunOnwer;
    m_pEventFun = fnOnEvent;
}

EnumErrno CISocket::AsyncSend( void *pKey, std::vector<CProPacket *> vProPacket  )
{
	GS_ASSERT_RET_VAL(!vProPacket.empty(), eERRNO_SYS_EINVALID);
    if( !IsValid() )
    {
        return eERRNO_NET_ECLOSED; 
    }

    EnumErrno eErrno  =  m_pAIO->AsyncSend(this, pKey, vProPacket);
    if( eErrno )
    {
		 m_bConnect=FALSE;
        CNetError csError(eErrno, 0, _GSTX("异步发送失败"));
		OnAIOEventError(csError);
    }
	return eErrno;
}

EnumErrno CISocket::AsyncSend( void *pKey,  std::vector<CProFrame *> vProFrame )
{

	GS_ASSERT_RET_VAL(!vProFrame.empty() , eERRNO_SYS_EINVALID);
	if( !IsValid() )
	{
		return eERRNO_NET_ECLOSED; 
	}

	EnumErrno eErrno  =  m_pAIO->AsyncSend(this, pKey, vProFrame);
	if( eErrno )
	{
		m_bConnect=FALSE;
		CNetError csError(eErrno, 0, _GSTX("异步发送失败"));
		OnAIOEventError(csError);
	}
	return eErrno;
}


#ifdef _WIN32
long OSUSleep( long iUSecond )
{

	LARGE_INTEGER litmp; LONGLONG QPart1,QPart2;
	double dfMinus, dfFreq, dfTim; 
	QueryPerformanceFrequency(&litmp);
	dfFreq = (double)litmp.QuadPart;
	// 获得计数器的时钟频率
	QueryPerformanceCounter(&litmp);
	QPart1 = litmp.QuadPart;
	// 获得初始值
	long iUS = 0;
	do
	{
		QueryPerformanceCounter(&litmp);
		QPart2 = litmp.QuadPart;  //获得中止值
		dfMinus = (double)(QPart2-QPart1);
		dfTim = dfMinus / dfFreq; // 获得对应的时间值，单位为秒
		iUS = dfTim*1000000;
	}while(iUS<iUSecond);
	return iUS;
}
#else
#define OSUSleep(x) usleep(x)
#endif

INT CISocket::SendTo(const CProPacket *pPacket, const StruLenAndSocketAddr *pRemoteAddr)
{	
	

	
	if( m_iUdpSendBytes > KBYTES*32 )
	{
		INT64 iCur = DoGetTickCount();
		int iInterval = iCur-m_iUdpLastSendTime;
		if( iInterval< 10  && iInterval>0 )
		{
			OSUSleep(500);			
		}
		m_iUdpLastSendTime = iCur;		
		m_iUdpSendBytes = 0;		
	}
	


	struct iovec vIO[3];
	UINT iCounts = 0;
	const StruPktInfo &stInfo = pPacket->GetParser();
	if( stInfo.iHeaderSize > 0 )
	{
		vIO[iCounts].iov_base = (char*) stInfo.bHeader;
		vIO[iCounts].iov_len = stInfo.iHeaderSize;
		iCounts++;
		m_iUdpSendBytes += stInfo.iHeaderSize;
	}
	if( stInfo.iPlayloadSize > 0 )
	{
		vIO[iCounts].iov_base = (char*) stInfo.bPlayload;
		vIO[iCounts].iov_len = stInfo.iPlayloadSize;
		iCounts++;
		m_iUdpSendBytes += stInfo.iPlayloadSize;
	}
	if( stInfo.iTailerSize > 0 )
	{
		vIO[iCounts].iov_base = (char*) stInfo.bTailer;
		vIO[iCounts].iov_len = stInfo.iTailerSize;
		iCounts++;
		m_iUdpSendBytes += stInfo.iTailerSize;
	}
	if( iCounts >0 )
	{

		return COSSocket::OSSendTo(vIO, iCounts, pRemoteAddr);
	}
	else
	{
		return 0;
	}
}

INT CISocket::SendTo(const CProFrame *pFrame, const StruLenAndSocketAddr *pRemoteAddr )
{
	CProPacket **vPkts = NULL;
	UINT iCnts = pFrame->GetPackets(&vPkts);
	INT iRet = 0;
	INT iTemp;
	for( UINT i = 0; i<iCnts; i++ )
	{
		iTemp = SendTo( vPkts[i], pRemoteAddr );
		if( iTemp<0 )
		{
			return iTemp;
		}
		else
		{
			iRet+=iTemp;
		}
	}
	return iRet;
}




EnumErrno CISocket::AsyncRcv(  BOOL bStart )
{
	
	if( !IsValid() )
	{
		return eERRNO_NET_ECLOSED; 
	}
	m_bRcvStart = bStart;
	EnumErrno eErrno  =  m_pAIO->AsyncRcv(this, bStart);
	if( eErrno )
	{
		m_bRcvStart = FALSE;
		m_bConnect = FALSE;
		CNetError csError(eErrno, 0, _GSTX("异步接收操作失败"));
		OnAIOEventError(csError);
	}
	return eErrno;
}

EnumErrno CISocket::AsyncAccept( BOOL bStart )
{

	if( !IsValid() )
	{
		return eERRNO_NET_ECLOSED; 
	}
	m_bAcceptStart = bStart;
	EnumErrno eErrno  =  m_pAIO->AsyncAccept(this, bStart);
	if( eErrno )
	{
		m_bAcceptStart = FALSE;
		m_bConnect = FALSE;
		CNetError csError(eErrno, 0, _GSTX("异步Accept操作失败"));
		OnAIOEventError(csError);
	}
	return eErrno;
}

EnumErrno CISocket::AsyncRcvFrom(  BOOL bStart )
{
	
	if( !IsValid() )
	{
		return eERRNO_NET_ECLOSED; 
	}
	m_bRcvStart = bStart;
	EnumErrno eErrno  =  m_pAIO->AsyncRcvFrom(this, bStart);
	if( eErrno )
	{
		m_bRcvStart = FALSE;
		m_bConnect = FALSE;
		CNetError csError(eErrno, 0, _GSTX("异步From接收操作失败"));
		OnAIOEventError(csError);
	}	
	return eErrno;

}



void CISocket::Release(void)
{
    Disconnect();
    UnrefObject();
}

void CISocket::Disconnect(void)
{
	g_csMutex.Lock();
	m_bConnect = FALSE;
	g_csMutex.Unlock();
	m_csTaskPool.Uninit(FALSE);
   Shutdown();
   CloseSocket();
   if( m_pAIO && m_bRegistered )
   {	
		m_bRegistered = FALSE;
		m_pAIO->Unregister(this);        
   } 	
   m_bRegistered = FALSE;
	
	
}





BOOL CISocket::BindAsyncIO( CNetError &csError )
{
     if( m_pAIO )
     {
         GS_ASSERT(0);
         return TRUE;
     }
	 m_pAIO = CNetAsyncIOGlobal::Intance().Register(this, csError );
     if( m_pAIO )
     {
		 m_bRegistered = TRUE;
         return TRUE;
     }
     GS_ASSERT(0);
     return FALSE;
}


void CISocket::OnTaskPoolEvent(CObjThreadPool *pTkPool, void *pEvtPara )
{
	StruISkEventData *p = (StruISkEventData*)pEvtPara;
	void *pParam = NULL, *ParamExt = NULL;
	switch(p->eEvt)
	{
	case eEVT_SOCKET_ARCVFROM :
		{
			if( m_bRcvStart )
			{
				pParam = p->Args.pRcvBuffer;		
				ParamExt = &p->stAddr; 
				m_bRcvStart = (BOOL) SendCallbackEvent(p->eEvt, pParam, ParamExt);
			}
			_FreeTaskData(p);
			return;
		}
	case eEVT_SOCKET_ARCV :	
		{
			if( m_bRcvStart )
			{
				pParam = p->Args.pRcvBuffer;			
				m_bRcvStart = (BOOL) SendCallbackEvent(p->eEvt, pParam, ParamExt);
			}
			_FreeTaskData(p);
			return;
		}
		break;
	case eEVT_SOCKET_ASEND :
		{
			pParam = &p->Args.stSendEvt;
		}
		break;
	case eEVT_SOCKET_ERR :
		{
			pParam = p->Args.pError;
		}
		break;
	case eEVT_SOCKET_ACCEPT :
		{
			if( m_bAcceptStart && m_pEventFun )
			{
				pParam = p->Args.pSocket;
				p->Args.pSocket = NULL;
				m_bAcceptStart = (BOOL) SendCallbackEvent(p->eEvt, pParam, ParamExt);
			}
			_FreeTaskData(p);
			return;
		}
		break;
	default:
		{
			GS_ASSERT(0);	
			_FreeTaskData(p);
			return;
		}
	break;
	}
	SendCallbackEvent(p->eEvt, pParam, ParamExt);
	
	_FreeTaskData(p);
}



BOOL CISocket::OnAIOEventRcv( CGSPBuffer *pBuffer )
{
	//    CGSWRMutexAutoRead rlocker(&m_csWRMutex);

	 CountAddRcvBytes(pBuffer->m_iDataSize);
     if( !m_bConnect )
     {
         return FALSE;
     }
	 StruISkEventData *p = _CreateTaskData();
	 if( !p )
	 {
		 GS_ASSERT(0);
		 CNetError csError(eERRNO_SYS_ENMEM, 0, _GSTX("没有内存!"));
		// SendCallbackEvent(eEVT_SOCKET_ERR, &csError, NULL );
		 OnAIOEventError(csError);
		 //TODO....
		 return FALSE;
	 }
	 pBuffer->RefObject();
	 p->eEvt = eEVT_SOCKET_ARCV;
	 p->Args.pRcvBuffer = pBuffer;
	 
	 if( m_csTaskPool.RSUCCESS != m_csTaskPool.Task(p))
	 {
		// GS_ASSERT(0);
		 _FreeTaskData(p);
		 CNetError csError(eERRNO_SYS_ETKP, 0, _GSTX("线程池失败!"));
		 //SendCallbackEvent(eEVT_SOCKET_ERR, &csError, NULL );
		  OnAIOEventError(csError);
		 //TODO....
		 return FALSE;
	 }
     return TRUE;
}

void CISocket::OnAIOEventSend( StruAsyncSendEvent &stEvt  )
{
	CountAddSendBytes( (long) stEvt.iSends);
    //  CGSWRMutexAutoRead rlocker(&m_csWRMutex);
	if( !m_bConnect )
	{
	//	GS_ASSERT(0);
		return;
	}
	StruISkEventData *p = _CreateTaskData();
	if( !p )
	{
		GS_ASSERT(0);
		CNetError csError(eERRNO_SYS_ENMEM, 0, _GSTX("没有内存!"));
		//SendCallbackEvent(eEVT_SOCKET_ERR, &csError, NULL );		
		 OnAIOEventError(csError);
	}
	
	p->eEvt = eEVT_SOCKET_ASEND;
	memcpy( &p->Args.stSendEvt,&stEvt, sizeof(stEvt));

	if( m_csTaskPool.RSUCCESS != m_csTaskPool.Task(p))
	{
		//GS_ASSERT(0);
		_FreeTaskData(p);
		CNetError csError(eERRNO_SYS_ETKP, 0, _GSTX("线程池失败!"));
		//SendCallbackEvent(eEVT_SOCKET_ERR, &csError, NULL );		
		 OnAIOEventError(csError);
	}
	
}

BOOL CISocket::OnAIOEventAccept(CISocket *pSocket )
{
	if( !m_bConnect )
	{
		return FALSE;
	}
	StruISkEventData *p = _CreateTaskData();
	if( !p )
	{
		CNetError csError(eERRNO_SYS_ENMEM, 0, _GSTX("没有内存!"));
		//SendCallbackEvent(eEVT_SOCKET_ERR, &csError, NULL );
		 OnAIOEventError(csError);
		//TODO....
		return FALSE;
	}
	p->eEvt = eEVT_SOCKET_ACCEPT;
	p->Args.pSocket = pSocket;	
	INT iRet = m_csTaskPool.Task(p);
	if( m_csTaskPool.RSUCCESS != iRet )
	{
		if( iRet = m_csTaskPool.EFLOWOUT )
		{

		GS_ASSERT(0);
	
		MY_LOG_WARN(g_pLog,  "CISocket 服务器 %s:%d 线程忙，%d连接等待处理. 连接 %s:%d 被拒绝.\n",
			LocalIP(),LocalPort(), pSocket->RemoteIP(),  m_csTaskPool.GetWaitTask(),
			pSocket->RemotePort()); 	
		}
		_FreeTaskData(p);
	}
	return TRUE;
}

BOOL CISocket::OnAIOEventRcvFrom( CGSPBuffer *pBuffer, 
								 const StruLenAndSocketAddr *pRemoteAddr )
{
	if( !m_bConnect )
	{
		return FALSE;
	}
	StruISkEventData *p = _CreateTaskData();
	if( !p )
	{
		CNetError csError(eERRNO_SYS_ENMEM, 0, _GSTX("没有内存!"));
		//SendCallbackEvent(eEVT_SOCKET_ERR, &csError, NULL );
		 OnAIOEventError(csError);
		//TODO....
		return FALSE;
	}
	pBuffer->RefObject();
	p->eEvt = eEVT_SOCKET_ARCVFROM;
	p->Args.pRcvBuffer = pBuffer;
	if( pRemoteAddr )
	{
		memcpy(&p->stAddr, pRemoteAddr, sizeof(*pRemoteAddr));
	}

	if( m_csTaskPool.RSUCCESS != m_csTaskPool.Task(p))
	{
	//	GS_ASSERT(0);
		_FreeTaskData(p);
		CNetError csError(eERRNO_SYS_ETKP, 0, _GSTX("线程池失败!"));
		//SendCallbackEvent(eEVT_SOCKET_ERR, &csError, NULL );
		 OnAIOEventError(csError);
		//TODO....
		return FALSE;
	}
	return TRUE;

}

void CISocket::OnAIOEventError( CNetError &csError )
{   
	
	StruISkEventData *p = _CreateTaskData();
	if( !p )
	{
		CNetError csError(eERRNO_SYS_ENMEM, 0, _GSTX("没有内存!"));
		SendCallbackEvent(eEVT_SOCKET_ERR, &csError, NULL );
		//TODO....
		return ;
	}
	p->Args.pError = new CNetError(csError);
	if( !p->Args.pError )
	{
		GS_ASSERT(0);
		_FreeTaskData(p);
		SendCallbackEvent(eEVT_SOCKET_ERR, &csError, NULL );
		return ;
		
	}
	p->eEvt = eEVT_SOCKET_ERR;
	int iRet = m_csTaskPool.Task(p);
	if( iRet != m_csTaskPool.RSUCCESS )
	{
		_FreeTaskData(p);
		if( iRet == m_csTaskPool.EFLOWOUT )
		{
			GS_ASSERT(0);		
			SendCallbackEvent(eEVT_SOCKET_ERR, &csError, NULL );
		}	
	}
}



void CISocket::SetTCPConnectOptions(void)
{

    if( eERRNO_SUCCESS == NonBlocking(TRUE).m_eErrno )
    {      
        if( IsInvalidSocket() )
        {
            MY_DEBUG("Socket is invalid.\n");
            return ;
        }

		NoDelay(TRUE);

		SetKeepalive(1);

		SetSendBuffer(KBYTES*96);
		//SetLinger(2); // (在closesocket()调用,但是还有数据没发送完毕的时候容许逗留)      
    } 
    else
    {
        GS_ASSERT(0);
    }

	
}




