#include "RtpContent.h"
#include "RtpSocket.h"
#include "../Log.h"


using namespace GSP;
using namespace GSP::RTSP;


#define TIMER_ID_SEND_NAL 2  //发送NAL 定时器

static void _FreeRtpRecvBuffer(RTP::CRtpRecvBuffer *p)
{
	p->UnrefObject();
}

CRtpContent::CRtpContent(void)
:CGSPObject()
{
	m_pSocket = NULL;
	m_stRemoteAddr.Reset();

	for( INT16 i = 0; i<ARRARY_SIZE(m_vRtpData); i++ )
	{
		m_vRtpData[i].Init((FuncPtrFree)_FreeRtpRecvBuffer);
		m_vRtpData[i].stRtpHeader.iPT = i&0x7F;
	}
	m_stCmdRtpData.Init((FuncPtrFree)_FreeRtpRecvBuffer);
	m_stCmdRtpData.stRtpHeader.iPT = RTP_CMD_CHANNEL_ID&0x7F; //用以CMD 通信
	m_stCmdRtpData.stRtpHeader.iM = 1;
	m_stCmdRtpData.stRtpHeader.iX = 1;
	m_iLocalSSRC = 0xFFFF;
	m_iRemoteSSRC = 0xFFFF;
	m_iSSRC = 0;
	m_stNalRemoteAddr.Reset();
	m_bNalEnable = TRUE;
	m_bConnect = FALSE; //当前是否连接
	 m_strRemoteIP = ""; // 对端IP
	 m_iRemotePort = 0; //对端通信端口
	 m_bNalInit = FALSE;

	m_pMyID = NULL;

	m_iNalInterval = 250;

	m_bTestConnect = FALSE;

	m_bServerType = TRUE;

	m_pFuncOwner = NULL;
	m_fnCallback = NULL;

	m_iSendNalTicks = 0xffff;
}

CRtpContent::~CRtpContent(void)
{
	Unint();
	m_csMutex.Lock();
	if( m_pMyID )
	{
		delete m_pMyID;
		m_pMyID = NULL;
	}	
	m_iLocalSSRC = 0xFFFF;
	m_csMutex.Unlock();

}

void CRtpContent::SetEventCallback( CGSPObject *pFuncOwner, FuncPtrEventCallback fnCallback )
{
	m_pFuncOwner = pFuncOwner;
	m_fnCallback = fnCallback;
}

void CRtpContent::Unint(void)
{
	m_csNalTimer.Stop();

	

	
	m_csMutex.Lock();
	m_bInit = FALSE;	
	m_bConnect = FALSE;
	while( m_bTestConnect )
	{
		m_csMutex.Unlock();
		MSLEEP(10);
		m_csMutex.Lock();
		m_csCond.BroadcastSignal();		
	}
	m_csMutex.Unlock();
	
	if( m_pSocket )
	{
		m_pSocket->RemoveListener(m_iLocalSSRC,(CGSPObject*) this );
		m_pSocket->UnrefObject();
		m_pSocket = NULL;
	}	

	for( INT16 i = 0; i<ARRARY_SIZE(m_vRtpData); i++ )
	{
		m_vRtpData[i].Clear();
	}
	m_stCmdRtpData.Clear();

	
}

EnumErrno CRtpContent::Init( CRtpSocket *pSocket )
{
	CGSAutoMutex locker(&m_csMutex);

	GS_ASSERT(m_pSocket==NULL);

	m_bServerType = pSocket->IsServerType();

	m_csNalTimer.Init(this, (FuncPtrTimerCallback)&CRtpContent::OnTimerEvent,
							TIMER_ID_SEND_NAL, 1000, FALSE);
	if( !m_csNalTimer.IsReady() )
	{
		return eERRNO_SYS_ENMEM;
	}	
	m_pMyID = new CMyID(&pSocket->IDContainer());
	if( m_pMyID == NULL )
	{
		return eERRNO_SYS_ENMEM;
	}
	
	m_pSocket = pSocket;
	m_pSocket->RefObject();
	m_iLocalSSRC = m_pMyID->m_iID;

	//MY_LOG_NOTICE(g_pLog, " ** LSSRC: *0x%04x*\n", m_iLocalSSRC);
	
	return pSocket->AddListener(m_iLocalSSRC, this, 
						(CRtpSocket::FuncPtrRtpDataRecvEvent)&CRtpContent::OnRtpRecvEvent);
}


EnumErrno CRtpContent::Connect(const char *szRemoteIP, UINT16 iRemotePort,UINT16 iRemoteSSRC, 
				  const char *szNalIP, INT iNalPort )
{
	CGSAutoMutex locker(&m_csMutex);
	if(m_pSocket == NULL )
	{
		return eERRNO_SYS_ESTATUS;
	}

	if( !COSSocket::Host2Addr(szRemoteIP, iRemotePort, m_stRemoteAddr ) )
	{
		return eERRNO_SYS_EINVALID;
	}
	m_iRemoteSSRC = iRemoteSSRC;
	if( m_bServerType )
	{
		m_iSSRC = (((UINT32)m_iLocalSSRC)<<16)|m_iRemoteSSRC; //服务器端在高位
	}
	else
	{	
		m_iSSRC = (((UINT32)m_iRemoteSSRC)<<16)|m_iLocalSSRC; //服务器端在高位
	}
	m_stCmdRtpData.stRtpHeader.iSSRC =  COSSocket::Int32H2N(m_iSSRC);
	for( int i = 0; i<GSP_MAX_MEDIA_CHANNELS; i++ )
	{
		m_vRtpData[i].stRtpHeader.iSSRC = m_stCmdRtpData.stRtpHeader.iSSRC;
	}
	
	if( szNalIP != NULL && iNalPort>0 )
	{
		
		if( COSSocket::Host2Addr(szNalIP, iNalPort, m_stNalRemoteAddr ) )
		{
			m_bNalInit = TRUE;
			if( m_bNalEnable )
			{
				m_csNalTimer.Start();
			}

			m_csMutex.Lock();
			if( m_pSocket )
			{
				//打穿连接
			   INT iVal = 0;	
				m_pSocket->SendTo(&iVal,sizeof(iVal), &m_stRemoteAddr);
			}
			m_csMutex.Unlock();
		}
		else
		{
			GS_ASSERT(0);
		}
	}
	m_iRemotePort = iRemotePort;
	m_strRemoteIP = szRemoteIP;
	m_bInit = TRUE;
	return eERRNO_SUCCESS;	
}

BOOL CRtpContent::EnableNal(BOOL bEnable, INT iInterval )
{
	m_bNalEnable = bEnable;	
	if( m_bNalEnable )
	{
		m_iNalInterval = iInterval;
		if( m_bNalInit )
		{
			m_csNalTimer.Start();
		}
	}
	return TRUE;
}

EnumErrno CRtpContent::SendFrame(CSliceFrame *pFrame )
{

	if( !m_bConnect )
	{
		return eERRNO_SRC_EUNUSED;
	}
	INT iChn = pFrame->m_iChnID;
	GS_ASSERT_RET_VAL( iChn>=0 && iChn<GSP_MAX_MEDIA_CHANNELS, eERRNO_SYS_EINVALID );

	m_csMutex.Lock();
	m_vRtpData[iChn].iTimestamp += 40;
	m_vRtpData[iChn].iRtpSeq = 0;
	m_vRtpData[iChn].stRtpHeader.iTS = COSSocket::Int32H2N(m_vRtpData[iChn].iTimestamp);
	if( pFrame->m_bKey )
	{
		m_vRtpData[iChn].stRtpHeader.iP = 1;
	}
	else
	{
		m_vRtpData[iChn].stRtpHeader.iP = 0;
	}
	CRtpSliceFrame *pRtpFrame;
	
	m_vRtpData[iChn].iCacheW++;
	if( m_vRtpData[iChn].iCacheW == m_vRtpData[iChn].iMaxCaches )
	{
		m_vRtpData[iChn].iCacheW = 0;
	}
	pRtpFrame = m_vRtpData[iChn].vFrameCache[m_vRtpData[iChn].iCacheW];	
	pRtpFrame->Clear();
	if( eERRNO_SUCCESS!=pRtpFrame->Build(pFrame, m_vRtpData[iChn].stRtpHeader, m_vRtpData[iChn].iRtpSeq ) )
	{
		m_csMutex.Unlock();
		return eERRNO_SYS_ENMEM;
	}
	m_csMutex.Unlock();
	m_iSendNalTicks = 0;
	m_pSocket->SendTo(pRtpFrame, &m_stRemoteAddr);
	return eERRNO_SUCCESS;
}

EnumErrno CRtpContent::ConnectTest(INT iTimeouts)
{
	CGSAutoMutex locker( &m_csMutex);
	GS_ASSERT_RET_VAL(m_pSocket != NULL, eERRNO_SYS_ESTATUS);
	
	if( m_bConnect  )
	{
		return eERRNO_SUCCESS;
	}
	if( !m_bInit )
	{
	
		return eERRNO_SYS_ESTATUS;
	}
	m_bTestConnect = TRUE;
	if( ! SendTestConnectCommand()  ) //发送测试命令
	{
		m_bTestConnect = FALSE;
		return eERRNO_NET_EWEVT;
	}
	m_csCond.WaitTimeout(&m_csMutex, iTimeouts);
	m_bTestConnect = FALSE;	
	return m_bConnect ? eERRNO_SUCCESS : eERRNO_SYS_ETIMEOUT;

}

BOOL CRtpContent::SendTestConnectCommand(void)
{
	RTSP::StruGspCommandPacket stCmd;

	
	m_stCmdRtpData.iTimestamp += 40;
	m_stCmdRtpData.stRtpHeader.iTS = COSSocket::Int32H2N(m_stCmdRtpData.iTimestamp);
	::memcpy(&stCmd.stRtpHeader,&m_stCmdRtpData.stRtpHeader, sizeof(RTP::StruRTPHeader) );
	stCmd.iCommandID = GSP_RTP_CMD_TEST_CONNECT_REQUEST;
	::memcpy(stCmd.bPlayload ,&m_stCmdRtpData.stRtpHeader.iSSRC, sizeof(UINT32));
	for( int i = 0; i<3; i++ )
	{
		//发送三次
		if( m_pSocket->SendTo(&stCmd,
						GSP_RTP_CMD_PACKET_MIN_LENGTH+sizeof(UINT32), &m_stRemoteAddr) < 0 )
		{
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CRtpContent::OnRtpRecvEvent(const StruLenAndSocketAddr &stRemoteAddr, 
								 RTP::CRtpRecvBuffer *pPacket )
{
	UINT iChn = pPacket->m_stHeader.iPT;

	

	if( !m_bInit )
	{
		return TRUE;
	}

	GS_ASSERT(pPacket->m_stHeader.iSSRC == m_iSSRC );

	if( iChn == RTP_CMD_CHANNEL_ID )
	{
		//通道命令
		
		return HandleCommand(stRemoteAddr, pPacket);
	}
	//数据流
	if( m_bServerType )
	{
		GS_ASSERT(0); //服务器端为什么会收到流数据
		return TRUE;
	}
	return HandleStream(stRemoteAddr, pPacket);
}

BOOL CRtpContent::HandleCommand(const StruLenAndSocketAddr &stRemoteAddr, 
				   RTP::CRtpRecvBuffer *pPacket)
{
	//处理命令
	const BYTE *p = pPacket->Data();
	INT iSize = pPacket->Size();
	if( iSize<1 )
	{
		GS_ASSERT(0); //达不到最小长度
		return TRUE;
	}
	iSize -= 1;
	switch( *p )
	{
	case GSP_RTP_CMD_RETRY_SEND_REQUEST :
		{
			//重传强求
			m_csMutex.Lock();
			p++;

			StruRtpGspCmddRetrySend stResend;
			CRtpSliceFrame::CMyIterator csIt;
			CRtpPacket *pFind;
			INT iNums = (INT) iSize/sizeof(StruRtpGspCmddRetrySend);
			SendEvent(eEVT_RCV_RETRY_RESEND, (void*)iNums);
			for( INT i = 0; i<iNums; i++ )
			{
				::memcpy(&stResend, p, sizeof(stResend));
				p += sizeof(stResend);
				if( stResend.iPT >= GSP_MAX_MEDIA_CHANNELS )
				{
					GS_ASSERT(0);
					continue;
				}
				for( INT iR = 0; iR< m_vRtpData[stResend.iPT].iMaxCaches; iR++ )
				{
					//查找
					csIt = m_vRtpData[stResend.iPT].vFrameCache[iR]->m_lMember.First<CRtpPacket*>();
					for( ;csIt.IsOk(); csIt.Next() )
					{
						pFind = csIt.Data();
						if( pFind->m_stHeader.iTS == stResend.iTS &&
							pFind->m_stHeader.iSeq == stResend.iSeq )
						{
							//查找到
// 							MY_PRINTF("Resend: %d, %ld-%d\n",
// 								m_iLocalSSRC,
// 								COSSocket::Int32N2H(stResend.iTS),
// 								COSSocket::Int16N2H(stResend.iSeq) );
							m_pSocket->SendTo(pFind, &m_stRemoteAddr);
							iR = m_vRtpData[stResend.iPT].iMaxCaches; //中断上一级for 
							break;
						}
					}
				}
			}
			m_csMutex.Unlock();
			 
		}
	break;
	case GSP_RTP_CMD_NAL_HOLE :
		{
			//NAL 大同名
			//MY_DEBUG("%u Rcv nal packet.\n", m_iSSRC);
			SendEvent( eEVT_RCV_NAL_PACKET, NULL);
		}
	break;
	case GSP_RTP_CMD_TEST_CONNECT_REQUEST :
		{
			//处理连接测试强求
			if( iSize<sizeof(UINT32) )
			{
				GS_ASSERT(0);
				return TRUE;
			}

			m_csMutex.Lock();
			::memcpy(&m_stRemoteAddr, &stRemoteAddr, sizeof(stRemoteAddr) ); //设置对端网络信息
			::memcpy(&m_stNalRemoteAddr, &stRemoteAddr, sizeof(stRemoteAddr) ); //设置对端网络信息
			CGSString strHost;
			INT iNewPort = COSSocket::AddrToString(stRemoteAddr, strHost);

			

			StruGspCommandPacket stCmd;
			m_stCmdRtpData.iTimestamp += 40;
			m_stCmdRtpData.stRtpHeader.iTS = COSSocket::Int32H2N(m_stCmdRtpData.iTimestamp);
			::memcpy(&stCmd.stRtpHeader,&m_stCmdRtpData.stRtpHeader, sizeof(StruRTPHeader) );
			stCmd.iCommandID = GSP_RTP_CMD_TEST_CONNECT_RESPONSE;
			::memcpy(stCmd.bPlayload ,&p[1], sizeof(UINT32));
			for( int i = 0; i<3; i++ )
			{
				//发送三次
				m_pSocket->SendTo((void*) &stCmd,GSP_RTP_CMD_PACKET_MIN_LENGTH+sizeof(UINT32), &m_stRemoteAddr);
			}
			m_csMutex.Unlock();
			if( !m_bConnect )
			{
				SendTestConnectCommand();
			}
		}
	break;
	case GSP_RTP_CMD_TEST_CONNECT_RESPONSE :
		{
			//连接强求回复
			//处理连接测试强求
			if( iSize<sizeof(UINT32) )
			{
				GS_ASSERT(0);
				return TRUE;
			}
			if( m_bConnect )
			{
				//已经连接
				return TRUE;
			}
			CGSAutoMutex locker(&m_csMutex);
			if( m_bConnect )
			{
				//已经连接
				return TRUE;
			}
			UINT32 iSSRC;
			::memcpy(&m_stRemoteAddr, &stRemoteAddr, sizeof(stRemoteAddr) ); //设置对端网络信息
			::memcpy(&m_stNalRemoteAddr, &stRemoteAddr, sizeof(stRemoteAddr) ); //设置对端网络信息
			CGSString strHost;
			INT iNewPort = COSSocket::AddrToString(stRemoteAddr, strHost);

			::memcpy(&iSSRC, &p[1], sizeof(UINT32));
			iSSRC = COSSocket::Int32N2H(iSSRC);
			GS_ASSERT(iSSRC == m_iSSRC);
			if( iSSRC == m_iSSRC )
			{
				m_bConnect = TRUE;
				m_csCond.Signal();
			}
		}
	break;
	default :
	break;
	}
	return TRUE;

}

INLINE static  INT _CmpTimestamp(UINT32 iNew, UINT32 iOld )
{
	if( iNew==iOld )
	{
		return 0;
	}
	if( iNew>iOld )
	{
		return iNew-iOld;
	}
	if( (iOld-iNew) > 0x7fffffff )
	{
		//已经循环
		return 1;
	}
	return -1;
}

INLINE static INT _CountSeq(UINT16 iNew, UINT16 iOld )
{
	if( iNew==iOld )
	{
		return 0;
	}
	if( iNew>iOld )
	{
		return iNew-iOld;
	}
	return iNew+0xffff-iOld+1;
}

BOOL CRtpContent::HandleStream(const StruLenAndSocketAddr &stRemoteAddr, 
				  RTP::CRtpRecvBuffer *pPacket)
{
INT iChn = pPacket->m_stHeader.iPT; //通道号
	if( iChn<0 || iChn>=GSP_MAX_MEDIA_CHANNELS )
	{
		//不合法的通道号
		GS_ASSERT(0);
		return TRUE;
	}

// 	if( pPacket->m_stHeader.iSeq == 2 )
// 	{
// 		MY_PRINTF("Rcv TS:%ld, Seq:%d, iM:%d \n",
// 					pPacket->m_stHeader.iTS, pPacket->m_stHeader.iSeq, pPacket->m_stHeader.iM );
// 		INT i = 0;
// 	}

// 	MY_DEBUG("Rcv TS:%ld, Seq:%d, iM:%d \n",
// 		pPacket->m_stHeader.iTS, pPacket->m_stHeader.iSeq, pPacket->m_stHeader.iM );


	
	//按顺序插入
	CList::CIterator<RTP::CRtpRecvBuffer *> csIt;
	csIt = m_vRtpData[iChn].lRcvPacket.First<RTP::CRtpRecvBuffer *>();
	INT iCmp;
	
	RTP::CRtpRecvBuffer *pBuf;

	BOOL bAddNew = TRUE;

	if( csIt.IsOk() )
	{
		iCmp = _CmpTimestamp(pPacket->m_stHeader.iTS, m_vRtpData[iChn].iRcvDiscardTimestamp );
		if( iCmp<=0 )
		{
			//已经过时的数据
// 			MY_PRINTF("Discard TS:%d, SEQ:%d,  OTS:%d\n",
//  					pPacket->m_stHeader.iTS, pPacket->m_stHeader.iSeq ,
// 					m_vRtpData[iChn].iRcvDiscardTimestamp );
			return TRUE;
		}

		
	}
	else
	{
		m_vRtpData[iChn].iRcvDiscardTimestamp = pPacket->m_stHeader.iTS-40;
	}
	

	EnumErrno  eRet = eERRNO_ENONE;
	csIt = m_vRtpData[iChn].lRcvPacket.First<RTP::CRtpRecvBuffer *>();
	
	for(BOOL bNBreak=TRUE; bNBreak && csIt.IsOk(); csIt.Next() )
	{		
		pBuf = csIt.Data();
		iCmp = _CmpTimestamp(pPacket->m_stHeader.iTS, pBuf->m_stHeader.iTS );
		if( iCmp==0 )
		{
			//相等 比较 Seq
			bAddNew = FALSE;

			bNBreak = FALSE; //中断外部循环
			for( ;csIt.IsOk(); csIt.Next() )
			{
				pBuf = csIt.Data();
				if( pPacket->m_stHeader.iSeq==pBuf->m_stHeader.iSeq )
				{
					//已经存在
					return TRUE;
				}

				if( pPacket->m_stHeader.iSeq<pBuf->m_stHeader.iSeq 
					|| pPacket->m_stHeader.iTS != pBuf->m_stHeader.iTS )
				{
					//加入到前面
					eRet = m_vRtpData[iChn].lRcvPacket.Prepend(csIt.Node(), pPacket);
					break;
				}
			}
		}
		else if( iCmp<0 )
		{
			//加入到前面
			eRet = m_vRtpData[iChn].lRcvPacket.Prepend(csIt.Node(), pPacket);
			bAddNew = FALSE;
			break;
		}
	}

	if( eRet == eERRNO_ENONE)
	{
		//加到后面
		eRet = m_vRtpData[iChn].lRcvPacket.AddTail(pPacket);
	}

	if(eERRNO_SUCCESS!=eRet )
	{
		//丢帧	
		GS_ASSERT(0);
		csIt = m_vRtpData[iChn].lRcvPacket.First<RTP::CRtpRecvBuffer *>();
		for( ;csIt.IsOk(); csIt.Next() )
		{
			pBuf = csIt.Data();
			if( pPacket->m_stHeader.iTS==pBuf->m_stHeader.iTS  )
			{
				
				if( pBuf->m_stHeader.iM )
				{
					GS_ASSERT(m_vRtpData[iChn].iCompletes>0);
					m_vRtpData[iChn].iCompletes--;
				}
				csIt = m_vRtpData[iChn].lRcvPacket.Erase<RTP::CRtpRecvBuffer *>(csIt);
			}
		}
		SendEvent( eEVT_RCV_LOST_FRAME , NULL);
		return TRUE;
	}

	pPacket->RefObject();
	
	csIt = m_vRtpData[iChn].lRcvPacket.First<RTP::CRtpRecvBuffer *>();
	UINT32 iFirstTS = csIt.Data()->m_stHeader.iTS;
	UINT16 iOldSeq = 0xffff;
	UINT32 iTemp = iFirstTS;
	UINT32 iFrameCounts = 1; //当前收到的帧数


	
	//统计帧数据
	for( ;csIt.IsOk(); csIt.Next() )
	{
		if( iTemp!=csIt.Data()->m_stHeader.iTS  )
		{			
			iFrameCounts++;
			iTemp =csIt.Data()->m_stHeader.iTS;
		}
	}
	
	if( pPacket->m_stHeader.iM )
	{
		m_vRtpData[iChn].iCompletes++;		
	}

	BOOL bOk = FALSE;
	if( iFrameCounts > 2  && m_vRtpData[iChn].iCompletes > 1 )
	{
		//检查第一帧是否完成


		
		//检查第一帧是否为结束包
		csIt = m_vRtpData[iChn].lRcvPacket.First<RTP::CRtpRecvBuffer *>();
		iFirstTS = csIt.Data()->m_stHeader.iTS;
		iOldSeq = 0xffff;
		for( ;csIt.IsOk(); csIt.Next() )
		{		
			pBuf = csIt.Data();
			iCmp = _CountSeq(pBuf->m_stHeader.iSeq, iOldSeq);
			if( iCmp != 1 || iFirstTS != pBuf->m_stHeader.iTS )
			{
				//丢帧
				break;
			}
			
			if( pBuf->m_stHeader.iM )
			{
				//第一帧为结束包
				iCmp=	_CmpTimestamp( pBuf->m_stHeader.iTS,
									m_vRtpData[iChn].iRcvDiscardTimestamp);
// 				if( 40!=iCmp )
// 				{
// 					MY_PRINTF("Lost TS:%d, SEQ:%d,  OTS:%d, Less:%d\n",
// 						pPacket->m_stHeader.iTS, pPacket->m_stHeader.iSeq ,
// 						m_vRtpData[iChn].iRcvDiscardTimestamp, iCmp );
// 				}

				m_vRtpData[iChn].iRcvDiscardTimestamp = pBuf->m_stHeader.iTS;
				bOk = TRUE;		
				break;
			}
			iOldSeq = pBuf->m_stHeader.iSeq;
		}

		if( bOk )
		{
			CSliceFrame *pFrame;
			pFrame = CSliceFrame::Create(iChn);
			if( pFrame )
			{
				iFrameCounts--;
				m_vRtpData[iChn].iCompletes--;
				RTP::CRtpRecvBuffer *p = NULL;
				BOOL bCreated = TRUE;
				INT iM;
				do 
				{
					p = NULL;
					m_vRtpData[iChn].lRcvPacket.RemoveFront((void**)&p);
					if(!p)
					{
						GS_ASSERT(0);
						break;
					}
					iM = p->m_stHeader.iM;
					if( iM )
					{
						pFrame->m_bKey = p->m_stHeader.iP != 0;
					}

					if( eERRNO_SUCCESS != pFrame->Add(p) )
					{
						GS_ASSERT(0);
						//丢帧
						bCreated = FALSE;						
					}
					p->UnrefObject();
				} while ( iM == 0 );	

				if( bCreated )
				{
					//一帧数据完成
					SendEvent( eEVT_RCV_FRAME, (CSliceFrame*)pFrame);
				}
				else
				{
					//丢帧
					SendEvent( eEVT_RCV_LOST_FRAME , NULL);
				}
				pFrame->UnrefObject();
			} 
			else
			{
				//丢帧
				GS_ASSERT(0); //todo
				iFrameCounts--;
				m_vRtpData[iChn].iCompletes--;
				RTP::CRtpRecvBuffer *p = NULL;
				INT iM;
				do 
				{
					p = NULL;
					m_vRtpData[iChn].lRcvPacket.RemoveFront((void**)&p);
					if(!p)
					{
						GS_ASSERT(0);
						break;
					}
					iM = p->m_stHeader.iM;
					p->UnrefObject();
				} while ( iM == 0 );	
			}
		}			
	}
	
	while(iFrameCounts > 5 ) 
	{
			//丢帧 
		csIt = m_vRtpData[iChn].lRcvPacket.First<RTP::CRtpRecvBuffer *>();
		UINT32 iFirstTS = csIt.Data()->m_stHeader.iTS;
		m_vRtpData[iChn].iRcvDiscardTimestamp = iFirstTS;
		for( ;csIt.IsOk(); csIt.Next() )
		{		
				//移除一帧
			if( iFirstTS!=csIt.Data()->m_stHeader.iTS )
			{					
				break;
			}
			pBuf = csIt.Data();

// 			 	MY_PRINTF("Rm %d TS:%ld, Seq:%d, iM:%d \n", m_iRemoteSSRC,
// 				 		pBuf->m_stHeader.iTS, pBuf->m_stHeader.iSeq, pBuf->m_stHeader.iM );

			csIt = m_vRtpData[iChn].lRcvPacket.Erase<RTP::CRtpRecvBuffer *>(csIt);
		}
		iFrameCounts--;		
		SendEvent( eEVT_RCV_LOST_FRAME , NULL);
	}


	if( iFrameCounts>2 &&  !bOk && bAddNew )
	{
		//重传


		csIt = m_vRtpData[iChn].lRcvPacket.First<RTP::CRtpRecvBuffer *>();
		iFirstTS = csIt.Data()->m_stHeader.iTS;
		iOldSeq = 0xffff;

		StruGspCommandPacket stCmd;
		StruRtpGspCmddRetrySend stResend;
		INT iResendNum = 0;
		INT iMaxCnts = (GSP_RTP_CMD_STRUCT_PLAYLOAD_SIZE/sizeof(stResend))-1;
		BYTE *p = (BYTE*)&stCmd.bPlayload[0];

		StruRTPHeader *pPreHeader = NULL;
		
		for( INT i = 0 ;csIt.IsOk() && i<2 && iResendNum<iMaxCnts ; csIt.Next() )
		{		
			pBuf = csIt.Data();
			
			if( iFirstTS != pBuf->m_stHeader.iTS )
			{
				if( pPreHeader->iM == 0 )
				{
					//增加最后结束
					if( iResendNum<iMaxCnts )
					{
						//MY_DEBUG("Lost: Seq:%d , Cnts: %d\n", iOldSeq, iCmp );
						iResendNum++;
						iOldSeq++;
						stResend.iTS = COSSocket::Int32H2N(pPreHeader->iTS);
						stResend.iSeq = COSSocket::Int16H2N(iOldSeq);
						stResend.iPT = pPreHeader->iPT;
						GS_ASSERT( stResend.iPT<GSP_MAX_MEDIA_CHANNELS);
						::memcpy(p, &stResend, sizeof(stResend));		
						p += sizeof(stResend);
					}
				}

				iOldSeq = 0xffff;
				i++;
				iFirstTS = pBuf->m_stHeader.iTS;
			}

			iCmp = _CountSeq(pBuf->m_stHeader.iSeq, iOldSeq);
			
			if( iCmp>0 )
			{				
				for( ;pBuf->m_stHeader.iSeq!=iOldSeq && iResendNum<iMaxCnts; iResendNum++,iOldSeq++ )
				{
					//MY_DEBUG("Lost: Seq:%d , Cnts: %d\n", iOldSeq, iCmp );
					
					stResend.iTS = COSSocket::Int32H2N(pBuf->m_stHeader.iTS);
					stResend.iSeq = COSSocket::Int16H2N(iOldSeq);
					stResend.iPT = pBuf->m_stHeader.iPT;
					GS_ASSERT( stResend.iPT<GSP_MAX_MEDIA_CHANNELS);
					::memcpy(p, &stResend, sizeof(stResend));		
					p += sizeof(stResend);
				}
			}
			iOldSeq = pBuf->m_stHeader.iSeq;
			pPreHeader = &pBuf->m_stHeader;
		}
	
		if( iResendNum > 0 )
		{
			//发送重传强求	

			m_csMutex.Lock();
			if( !m_pSocket )
			{
				m_csMutex.Unlock();
				return FALSE;
			}
			::memcpy(&m_stRemoteAddr, &stRemoteAddr, sizeof(stRemoteAddr) ); //设置对端网络信息		
			m_stCmdRtpData.iTimestamp += 40;
			m_stCmdRtpData.stRtpHeader.iTS = COSSocket::Int32H2N(m_stCmdRtpData.iTimestamp);
			::memcpy(&stCmd.stRtpHeader,&m_stCmdRtpData.stRtpHeader, sizeof(StruRTPHeader) );
			stCmd.iCommandID = GSP_RTP_CMD_RETRY_SEND_REQUEST;			
			m_pSocket->SendTo( (void*) &stCmd,
								GSP_RTP_CMD_PACKET_MIN_LENGTH+sizeof(stResend)*iResendNum,
								&m_stRemoteAddr);	
			m_csMutex.Unlock();

		}
	}

	return TRUE;
}


void CRtpContent::OnTimerEvent( CWatchTimer *pTimer )
{
	switch(pTimer->GetID() )
	{
	case TIMER_ID_SEND_NAL :
		{
			//发送NAL
			m_iSendNalTicks++;
			if( m_iSendNalTicks>=m_iNalInterval )
			{
				
				m_csMutex.Lock();
				if( !m_pSocket )
				{
					m_csMutex.Unlock();
					return ;
				}
				StruGspCommandPacket stCmd;
				m_iSendNalTicks = 0;
				m_stCmdRtpData.iTimestamp += 40;
				m_stCmdRtpData.stRtpHeader.iTS = COSSocket::Int32H2N(m_stCmdRtpData.iTimestamp);
				::memcpy(&stCmd.stRtpHeader,&m_stCmdRtpData.stRtpHeader, sizeof(StruRTPHeader) );
				stCmd.iCommandID = GSP_RTP_CMD_NAL_HOLE;
				::memcpy(stCmd.bPlayload ,&m_stCmdRtpData.stRtpHeader.iSSRC, sizeof(UINT32));			
				m_pSocket->SendTo(&stCmd,GSP_RTP_CMD_PACKET_MIN_LENGTH+sizeof(UINT32),
									&m_stRemoteAddr);
				m_csMutex.Unlock();
			}
		}
	break;
	}
}
