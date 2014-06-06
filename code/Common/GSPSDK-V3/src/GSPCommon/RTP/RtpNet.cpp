#include "RtpNet.h"
#include "../GSPMemory.h"
#include "../OSThread.h"
#include "RTPPacket.h"
#include "RtpProFrame.h"

using namespace GSP;
using namespace GSP::RTP;


/*
*********************************************************************
*
*@brief : CRtpUdpNet
*
*********************************************************************
*/

CRtpUdpNet::CRtpUdpNet(BOOL bService) : CGSPObject()
,m_bService(bService)
{



	m_base_timestamp = (UINT32) COSThread::Milliseconds();

	m_num_frames = 0;

	/* rtcp sender statistics receive */
	m_last_rtcp_ntp_time = GetNTPTime();
	m_first_rtcp_ntp_time = m_last_rtcp_ntp_time;  

	/* rtcp sender statistics */
	m_packet_count = 0;
	m_octet_count = 0;      
	m_last_octet_count = 0; 
	m_first_packet = TRUE;

	m_iRcvRtcpCounts = 0;

	m_bConnected = FALSE;

	m_pRtpSocket = NULL;
	m_pRtcpSocket = NULL;

	m_fnEvtCb = NULL;
	m_pEvtReceiver= NULL;

	m_iChannelId = MAX_UINT16;
	m_iRtpPlayloadType = 96;
	m_iCurTimestamp = 0;
	m_iSSRC = 0;	
}


CRtpUdpNet::~CRtpUdpNet(void)
{
	Disconnect();
	if( m_pRtcpSocket )
	{
		m_pRtcpSocket->Release();
		m_pRtcpSocket = NULL;
	}
	if( m_pRtpSocket )
	{
		m_pRtpSocket->Release();
		m_pRtpSocket = NULL;
	}

}

void CRtpUdpNet::SetEventListener(CGSPObject *pEvtReceiver, FuncPtrRtpNetEvent fnEvtCb )
{
	if( pEvtReceiver )
	{
		m_fnEvtCb = fnEvtCb;
		m_pEvtReceiver = pEvtReceiver;
	}
	else
	{
		m_pEvtReceiver = NULL;
		m_fnEvtCb = NULL;
	}
}

EnumErrno CRtpUdpNet::SetRemoteAddress(  const CGSPString &strRemoteHost, int iRtpPort, int iRtcpPort)
{	
EnumErrno eRet = eERRNO_NET_ECLOSED;
	if( m_pRtpSocket )
	{
		eRet = m_pRtpSocket->SetRemoteAddress(strRemoteHost, iRtpPort);
		if( eERRNO_SUCCESS == eRet &&  m_pRtcpSocket)
		{
			eRet =  m_pRtcpSocket->SetRemoteAddress(strRemoteHost, 
				(iRtcpPort > 0 ? iRtcpPort : iRtpPort+1) );			
		}
	}
	return eRet;
}

void CRtpUdpNet::ClearRemoteAddress( void )
{
	if( m_pRtpSocket )
	{
		m_pRtpSocket->ClearRemoteAddress();
	}
	if( m_pRtcpSocket )
	{
		m_pRtcpSocket->ClearRemoteAddress();
	}
}


INT CRtpUdpNet::GetPlayloadType(void) const
{
	return m_iRtpPlayloadType;
	
}

INT CRtpUdpNet::GetChannelID( void) const
{
	return m_iChannelId;
}

EnumErrno CRtpUdpNet::SetPlayloadType( UINT8 iPlayloadType, UINT16 iChannelID)
{	
	m_iRtpPlayloadType = iPlayloadType;
	m_iChannelId = iChannelID;	
	m_csRtpHeader.SetPayloadType(m_iRtpPlayloadType);


	return eERRNO_SUCCESS;
}




EnumErrno CRtpUdpNet::JoinMulticast(const CGSPString &strRemoteHost )
{
	//todo..
	GS_ASSERT(0);
	return eERRNO_SYS_EFUNC;
}

EnumErrno CRtpUdpNet::LeaveMulticast(const CGSPString &strRemoteHost )
{
	//todo..
	GS_ASSERT(0);
	return eERRNO_SYS_EFUNC;
}

EnumErrno    CRtpUdpNet::SetTtl(UINT16 timeToLive)
{
	//todo..
	GS_ASSERT(0);
	return eERRNO_SYS_EFUNC;
}

EnumErrno    CRtpUdpNet::SetMulticastInterface(const CGSPString &strLocalIp )
{
	//todo..
	GS_ASSERT(0);
	return eERRNO_SYS_EFUNC;
}

void CRtpUdpNet::Disconnect(void)
{
	m_bConnected = FALSE;
	if( m_pRtpSocket)
	{
		m_pRtpSocket->Disconnect();
	}

	if( m_pRtcpSocket )
	{
		m_pRtcpSocket->Disconnect();
	}
}

EnumErrno CRtpUdpNet::Init( const char *czLocalIp, BOOL bSendOnly, BOOL bMulticast )
{
CNetError csError;
UINT i;
	GS_ASSERT(m_pRtpSocket==NULL);
	csError.m_eErrno = eERRNO_SUCCESS;
	m_bMulticast = bMulticast;
	m_bSendOnly = bSendOnly;
	m_pRtpSocket = m_pRtpSocket->Create(m_bService, !m_bService);
	if( m_pRtpSocket == NULL )
	{
		GS_ASSERT(0);
		goto fail_ret;
	}
	if( m_bService )
	{
		m_pRtpSocket->ResetBuffer( KBYTES*384, KBYTES);
	}
	else
	{
		m_pRtpSocket->ResetBuffer( KBYTES, KBYTES*384);
	}

	if( !m_bSendOnly )
	{
		m_pRtcpSocket = m_pRtcpSocket->Create(TRUE, TRUE);
		if( m_pRtcpSocket == NULL )
		{
			GS_ASSERT(0);
			goto fail_ret;
		}
		m_pRtcpSocket->ResetBuffer(KBYTES*32,KBYTES*32);
	}

	
	m_iLocalRtpPort = 0;

	for( i = 0; i<20000; i += 2 )
	{
		csError.m_eErrno = m_pRtpSocket->InitSocket(m_iLocalRtpPort+i, czLocalIp, bMulticast);
		if( csError.m_eErrno )
		{		
			if( m_iLocalRtpPort>0 )
			{
				continue;
			}
			else
			{
				GS_ASSERT(0);
				goto fail_ret;

			}
		}		
		m_strLocalIp = m_pRtpSocket->BindLocalIp();	
		m_iLocalRtpPort = m_pRtpSocket->LocalPort();

		if( m_pRtcpSocket )
		{
			csError.m_eErrno = m_pRtcpSocket->InitSocket(m_iLocalRtpPort+1, czLocalIp, bMulticast);
			if( !csError.m_eErrno )
			{
				m_iLocalRtcpPort = m_pRtpSocket->LocalPort();
				break;
			}			
		}	
		else
		{
			break;
		}
	}

	if( csError.m_eErrno )
	{
		GS_ASSERT(0);
		goto fail_ret;
	}
	m_bConnected = TRUE;
	return eERRNO_SUCCESS;
fail_ret:
	if( m_pRtcpSocket )
	{
		m_pRtcpSocket->Release();
		m_pRtcpSocket = NULL;
	}
	if( m_pRtpSocket )
	{
		m_pRtpSocket->Release();
		m_pRtpSocket = NULL;
	}
	return csError.m_eErrno;

}

EnumErrno CRtpUdpNet::Init(int iMinPort, int iMaxPort, const char *czLocalIp, 
						   BOOL bSendOnly, BOOL bMulticast )
{
	CNetError csError;
	int i;
	GS_ASSERT(m_pRtpSocket==NULL);
	csError.m_eErrno = eERRNO_SYS_EINVALID;
	m_bMulticast = bMulticast;
	m_bSendOnly = bSendOnly;
	m_pRtpSocket = CUDPSocket::Create(m_bService, !m_bService);
	if( m_pRtpSocket == NULL )
	{
		GS_ASSERT(0);
		csError.m_eErrno = eERRNO_SYS_ENMEM;
		goto fail_ret;
	}
	if( m_bService )
	{
		m_pRtpSocket->ResetBuffer( KBYTES*384, KBYTES);
	}
	else
	{
		m_pRtpSocket->ResetBuffer( KBYTES, KBYTES*384);
	}
	if( !m_bSendOnly )
	{
		m_pRtcpSocket =  CUDPSocket::Create(TRUE, TRUE);
		if( m_pRtcpSocket == NULL )
		{
			GS_ASSERT(0);
			csError.m_eErrno = eERRNO_SYS_ENMEM;
			goto fail_ret;
		}
		m_pRtcpSocket->ResetBuffer(KBYTES*32,KBYTES*32);
	}

	for( i = iMinPort; i <= iMaxPort; i+=2 )
	{

		csError.m_eErrno = m_pRtpSocket->InitSocket(i, czLocalIp, bMulticast);
		if( csError.m_eErrno == eERRNO_SUCCESS )
		{	
			m_iLocalRtpPort = m_pRtpSocket->LocalPort();
			m_strLocalIp = m_pRtpSocket->BindLocalIp();
			if( m_pRtcpSocket )
			{			
				csError =  m_pRtcpSocket->InitSocket(m_iLocalRtpPort+1, czLocalIp, bMulticast);		
				if( csError.m_eErrno == eERRNO_SUCCESS )
				{
					m_iLocalRtcpPort = m_pRtpSocket->LocalPort();
					m_bConnected = TRUE;
					return eERRNO_SUCCESS;
				}				
			}
			else
			{
				m_bConnected = TRUE;
				return eERRNO_SUCCESS;
			}
		}

	}


fail_ret :

	if( m_pRtcpSocket )
	{
		m_pRtcpSocket->Release();
		m_pRtcpSocket = NULL;
	}
	if( m_pRtpSocket )
	{
		m_pRtpSocket->Release();
		m_pRtpSocket = NULL;
	}
	
	return csError.m_eErrno;
}


/*
*********************************************************************
*
*@brief : CRtpSender
*
*********************************************************************
*/
#define RTCP_TX_RATIO_NUM 5
#define RTCP_TX_RATIO_DEN 100
#define RTCP_SR_SIZE 5


typedef enum  {
	eAV_ROUND_ZERO     = 0, ///< Round toward zero.
	eAV_ROUND_INF      = 1, ///< Round away from zero.
	eAV_ROUND_DOWN     = 2, ///< Round toward -infinity.
	eAV_ROUND_UP       = 3, ///< Round toward +infinity.
	eAV_ROUND_NEAR_INF = 5, ///< Round to nearest and halfway cases away from zero.
}EnumAVRounding;


static INT64 RescaleRnd(INT64 a, INT64 b, INT64 c, EnumAVRounding rnd )
{
	INT64 r=0;
	GS_ASSERT_RET_VAL(c > 0, MAX_INT64);
	GS_ASSERT_RET_VAL(b >=0, MAX_INT64);
	GS_ASSERT_RET_VAL((unsigned)rnd<=5 && rnd!=4, MAX_INT64);

	if(a<0 && a != MIN_INT64) return -RescaleRnd(-a, b, c,(EnumAVRounding) (rnd ^ ((rnd>>1)&1)));

	if(rnd==eAV_ROUND_NEAR_INF) r= c/2;
	else if(rnd&1)             r= c-1;

	if(b<=INT_MAX && c<=INT_MAX){
		if(a<=INT_MAX)
			return (a * b + r)/c;
		else
			return a/c*b + (a%c*b + r)/c;
	}else{  
		INT64 a0= a&0xFFFFFFFF;
		INT64 a1= a>>32;
		INT64 b0= b&0xFFFFFFFF;
		INT64 b1= b>>32;
		INT64 t1= a0*b1 + a1*b0;
		INT64 t1a= t1<<32;
		int i;

		a0 = a0*b0 + t1a;
		a1 = a1*b1 + (t1>>32) + (a0<t1a);
		a0 += r;
		a1 += a0<r;

		for(i=63; i>=0; i--){
			//            int o= a1 & 0x8000000000000000ULL;
			a1+= a1 + ((a0>>i)&1);
			t1+=t1;
			if(/*o || */c <= a1){
				a1 -= c;
				t1++;
			}
		}
		return t1;
	}
	return MAX_INT64;
}











CRtpUdpSender::CRtpUdpSender(void)
:CRtpUdpNet(TRUE)
{
	

	m_iTimestampInterval = 3600;

	
	
}

CRtpUdpSender::~CRtpUdpSender(void)
{
	m_csSendRtcpTimer.Stop();
	Disconnect();
}

CRtpUdpSender *CRtpUdpSender::Create( const char *czLocalIp, BOOL bSendOnly, BOOL bMulticast )
{

	
	CRtpUdpSender *pRet = new CRtpUdpSender();
	GS_ASSERT(pRet);
	if( pRet )
	{
		if( !bSendOnly )
		{
			pRet->m_csSendRtcpTimer.Init( pRet, (FuncPtrTimerCallback) &CRtpUdpSender::OnTimerEvent, 
				0, 100, FALSE, NULL );
			if( !pRet->m_csSendRtcpTimer.IsReady() )
			{
				GS_ASSERT(0);
				delete pRet;
				return NULL;
			}
		}

		if( pRet->Init(czLocalIp, bSendOnly, bMulticast) )
		{			
			GS_ASSERT(0);
			delete pRet;
			pRet = NULL;
			
		}
	}
	return pRet;

}

CRtpUdpSender *CRtpUdpSender::Create( int iMinPort, int iMaxPort, const char *czLocalIp, 
						BOOL bSendOnly, BOOL bMulticast )
{
	
	CRtpUdpSender *pRet = new CRtpUdpSender();
	GS_ASSERT(pRet);
	if( pRet )
	{
		if( !bSendOnly )
		{
			pRet->m_csSendRtcpTimer.Init(pRet,(FuncPtrTimerCallback)&CRtpUdpSender::OnTimerEvent, 
				0, 100, FALSE, NULL );
			if( !pRet->m_csSendRtcpTimer.IsReady() )
			{
				GS_ASSERT(0);
				delete pRet;
				return NULL;
			}
		}
		if( pRet->Init(iMinPort, iMaxPort, czLocalIp, bSendOnly, bMulticast) )
		{			
			GS_ASSERT(0);
			delete pRet;
			pRet = NULL;
			
		}
	}
	return pRet;

}



EnumErrno CRtpUdpSender::Start(void)
{
	if( !m_bSendOnly )
	{
		m_pRtcpSocket->SetListener(this, (FuncPtrSocketEvent)&CRtpUdpSender::OnRtcpSocketEvent);
		
		EnumErrno eRet = m_pRtcpSocket->AsyncRcvFrom(TRUE);
		if( eRet ==eERRNO_SUCCESS )
		{
			m_csSendRtcpTimer.Start();
		}
		return eRet;
	}
	return eERRNO_SUCCESS;
}

void CRtpUdpSender::Stop(void)
{
	m_csSendRtcpTimer.Stop();
	Disconnect();
}

EnumErrno CRtpUdpSender::Send(CFrameCache *pFrame, UINT32 iTimestamp)
{
	if( !m_bConnected  || !m_pRtpSocket || !m_pRtpSocket->HadRemoteAddr() )
	{
		return eERRNO_SRC_EUNUSED;
	}
	m_csRtpHeader.SetTimestamp(iTimestamp);
	CRtpProFrame *pProFrame = CRtpProFrame::Create(pFrame, m_csRtpHeader );
	
	if( !pProFrame )
	{	
		GS_ASSERT(0);
		return eERRNO_SYS_ENMEM;
	}

	RtcpTimerTicks();
	m_iCurTimestamp = iTimestamp;

	UINT iPkts;
	CProPacket **ppPkt;
	iPkts = pProFrame->GetPackets( &ppPkt );
	m_packet_count += iPkts;
	m_octet_count = m_packet_count;
	m_pRtpSocket->SendTo(pProFrame);
	pProFrame->UnrefObject();
	return eERRNO_SUCCESS;
}

EnumErrno CRtpUdpSender::Send(CFrameCache *pFrame)
{
	return Send(pFrame, m_iCurTimestamp+m_iTimestampInterval );
}


void CRtpUdpSender::RtcpTimerTicks(void)
{
	//发送RTCP

	if( !m_pRtcpSocket || !m_pRtcpSocket->HadRemoteAddr())
	{
		return;
	}

	int rtcp_bytes;
	UINT64 iv = GetNTPTime();
	rtcp_bytes = ((m_octet_count - m_last_octet_count) * RTCP_TX_RATIO_NUM) /RTCP_TX_RATIO_DEN;

	if ( m_first_packet ||
		( rtcp_bytes >= RTCP_SR_SIZE &&	(iv - m_last_rtcp_ntp_time) > 5000000) ) 
	{
		m_csMutex.Lock();
		rtcp_bytes = ((m_octet_count - m_last_octet_count) * RTCP_TX_RATIO_NUM) /RTCP_TX_RATIO_DEN;
		if (  m_first_packet ||
			( rtcp_bytes >= RTCP_SR_SIZE &&	(iv - m_last_rtcp_ntp_time) > 5000000) ) 
		{
		
			m_last_rtcp_ntp_time = GetNTPTime();
			rtcp_bytes =  MakeRtcpSendSR();
			m_last_octet_count = m_octet_count;
			m_first_packet = 0;					
			if( rtcp_bytes > 0 )
			{
			//	printf("send rtcp: **%d %d  %lld-%lld = %ld  %d  %d\n",
			 //          m_first_packet, rtcp_bytes,iv, m_last_rtcp_ntp_time, 
			//		   (long) (iv - m_last_rtcp_ntp_time) , rtcp_bytes, COSSocket::Int16N2H(m_stRemoteRtcpAddr.sn.sin.sin_port) );
				struct iovec vIO;
				vIO.iov_base = m_unionRtcpBuf.pRtcpBuffer;
				vIO.iov_len = rtcp_bytes;
				m_pRtcpSocket->SendTo(&vIO, 1 );
			}
		}
		m_csMutex.Unlock();
	}

}


INT CRtpUdpSender::MakeRtcpSendSR(void)
{
	UINT32 rtp_ts;
	INT64 b;
	INT64 c;
	INT iContents = 6;
	b = 1L*m_iTimestampInterval;
	c = 1000000LL*1;

	rtp_ts = (UINT32) RescaleRnd(m_last_rtcp_ntp_time - m_first_rtcp_ntp_time, b,c, eAV_ROUND_NEAR_INF) +		m_base_timestamp;
	StruRtcpPacket &stPacket = m_unionRtcpBuf.stPacket;
	bzero(&stPacket, sizeof(stPacket) );
	stPacket.common.version = RTP_VERSION;
	stPacket.common.pt = eRTCP_SR;
	stPacket.common.length = COSSocket::Int16H2N(iContents);
	stPacket.r.sr.ssrc = COSSocket::Int32H2N(m_iSSRC);
	stPacket.r.sr.ntp_sec =  COSSocket::Int32H2N((INT32) (m_last_rtcp_ntp_time/1000000) ) ;
	stPacket.r.sr.ntp_frac = COSSocket::Int32H2N( (((m_last_rtcp_ntp_time % 1000000) << 32) / 1000000));
	stPacket.r.sr.rtp_ts = COSSocket::Int32H2N(m_iCurTimestamp);

	stPacket.r.sr.psent = COSSocket::Int32H2N( m_packet_count);
	stPacket.r.sr.osent = COSSocket::Int32H2N( m_octet_count);
	INT iLen = (iContents+1)*sizeof(UINT32);	
	return iLen;    
}

void CRtpUdpSender::RtcpSendBye(void)
{
	if( m_packet_count  && m_pRtcpSocket && m_pRtcpSocket->HadRemoteAddr())
	{
		StruRtcpPacket stPacket;
		INT iContents = 1;
		bzero(&stPacket, sizeof(stPacket) );
		stPacket.common.version = RTP_VERSION;
		stPacket.common.pt = eRTCP_BYE;		
		stPacket.common.length = COSSocket::Int16H2N(iContents);
		stPacket.r.bye.src[0] = COSSocket::Int32H2N(m_iSSRC);
		INT iLen = (iContents+1)*sizeof(UINT32);	
		if(m_pRtcpSocket )
		{
			struct iovec vIO;
			vIO.iov_base = m_unionRtcpBuf.pRtcpBuffer;
			vIO.iov_len = iLen;
			m_pRtcpSocket->SendTo(&vIO, 1 );
		}

	}
}


void* CRtpUdpSender::OnRtcpSocketEvent(CISocket *pSocket, 
					   EnumSocketEvent iEvt,void *pParam, void *pParamExt )
{
	if( !m_bConnected )
	{
		return (void*) FALSE;
	}
	GS_ASSERT_RET_VAL( pSocket == dynamic_cast<CISocket*>(m_pRtcpSocket), FALSE );
	BOOL bRet = TRUE;
	if( iEvt == eEVT_SOCKET_ARCVFROM  )
	{
		const StruLenAndSocketAddr *pRemoteAddr = (StruLenAndSocketAddr *)pParamExt;
		CGSPBuffer *pBuf = (CGSPBuffer *)pParam;
		GS_ASSERT(pRemoteAddr);
		GS_ASSERT(pBuf);

		StruRtcpPacket stRtcpPkt;
		bzero(&stRtcpPkt, sizeof(stRtcpPkt));

		memcpy(&stRtcpPkt, pBuf->m_bBuffer, MIN(pBuf->m_iDataSize, sizeof(stRtcpPkt)));
		RunEventCallback(eEVT_RTPNET_RTCP_PULSE, NULL);
	
			switch( stRtcpPkt.common.pt )
			{
			case RTP::eRTCP_RR :
				{
					
					if( m_iSSRC == COSSocket::Int32N2H(stRtcpPkt.r.sr.ssrc ) )
					{
						//是同步源
						m_iRcvRtcpCounts++;
					}				
				}			
			break;
			case RTP::eRTCP_BYE:
				{
					RunEventCallback(eEVT_RTPNET_RTCP_BYE, NULL);
					bRet =FALSE;
				}
			break;

			}

	}

	return (void*) bRet;

}


void CRtpUdpSender::OnTimerEvent(CWatchTimer *pTimer)
{
	
	if( !m_bSendOnly )
	{
		RtcpTimerTicks();
	}
}


/*
*********************************************************************
*
*@brief : CRtpUdpReader
*
*********************************************************************
*/
CRtpUdpReader::CRtpUdpReader(void) 
: CRtpUdpNet(FALSE)
{
	m_stLimitRemoteAddr.Reset();
	m_stLimitRemoteAddr.len = 0;
	m_iLimitePort = 0;

}

CRtpUdpReader::~CRtpUdpReader(void)
{
	m_csSendRtcpTimer.Stop();
}

void CRtpUdpReader::BindChannelMediaType(UINT iChnId,EnumGSMediaType eType, UINT8 iRtpPt )
{
	GS_ASSERT_RET(iChnId<GSP_MAX_MEDIA_CHANNELS);
	SetPlayloadType(iRtpPt, iChnId);
	m_vBindMediaType[iChnId] = eType;

}


void CRtpUdpReader::SetLimitAddr( const CGSPString &strRemoteIp, UINT iPort )
{
	m_stLimitRemoteAddr.Reset();
	if( !strRemoteIp.empty() )
	{
		COSSocket::Host2Addr(strRemoteIp.c_str(), iPort, m_stLimitRemoteAddr);
	}
	else
	{
		m_stLimitRemoteAddr.len = 0;
	}
	m_iLimitePort = iPort;
}



CRtpUdpReader *CRtpUdpReader::Create( const char *czLocalIp, BOOL bRcvOnly, BOOL bMulticast )
{


	CRtpUdpReader *pRet = new CRtpUdpReader();
	GS_ASSERT(pRet);
	if( pRet )
	{
		if( !bRcvOnly )
		{
			pRet->m_csSendRtcpTimer.Init(pRet,(FuncPtrTimerCallback)&CRtpUdpReader::OnTimerEvent, 
				0, 100, FALSE, NULL );
			if( !pRet->m_csSendRtcpTimer.IsReady() )
			{
				GS_ASSERT(0);
				delete pRet;
				return NULL;
			}
		}
		if( pRet->Init(czLocalIp, bRcvOnly, bMulticast) )
		{			
			GS_ASSERT(0);
			delete pRet;
			pRet = NULL;

		}
	}
	
	return pRet;

}

CRtpUdpReader *CRtpUdpReader::Create( int iMinPort, int iMaxPort, const char *czLocalIp, 
									 BOOL bRcvOnly, BOOL bMulticast )
{

	CRtpUdpReader *pRet = new CRtpUdpReader();
	GS_ASSERT(pRet);
	if( pRet )
	{		
		if( !bRcvOnly )
		{
			pRet->m_csSendRtcpTimer.Init(pRet,(FuncPtrTimerCallback)&CRtpUdpReader::OnTimerEvent, 
				0, 100, FALSE, NULL );
			if( !pRet->m_csSendRtcpTimer.IsReady() )
			{
				GS_ASSERT(0);
				delete pRet;
				return NULL;
			}
		}
		if( pRet->Init(iMinPort, iMaxPort, czLocalIp, bRcvOnly, bMulticast) )
		{			
			GS_ASSERT(0);
			delete pRet;
			pRet = NULL;

		}
	}	
	return pRet;
}

EnumErrno CRtpUdpReader::Start(void)
{
	EnumErrno eRet = eERRNO_SUCCESS;
	if( m_pRtcpSocket )
	{
		m_pRtcpSocket->SetListener(this, (FuncPtrSocketEvent)&CRtpUdpReader::OnRtcpSocketEvent);
		eRet = m_pRtcpSocket->AsyncRcvFrom(TRUE);
	}

	if( eRet == eERRNO_SUCCESS )
	{
		m_pRtpSocket->SetListener(this, (FuncPtrSocketEvent)&CRtpUdpReader::OnRtpSocketEvent);
		eRet = m_pRtpSocket->AsyncRcvFrom(TRUE);
	}

	if( eRet )
	{
		if( m_pRtcpSocket )
		{
			m_pRtcpSocket->Disconnect();
		}
		m_pRtpSocket->Disconnect();
	}
	else
	{
		m_csSendRtcpTimer.Start();
	}
	return eERRNO_SUCCESS;
}

void CRtpUdpReader::Stop(void)
{
	m_csSendRtcpTimer.Stop();
	Disconnect();
}


void CRtpUdpReader::OnTimerEvent(CWatchTimer *pTimer)
{

	if( !m_first_packet && !m_bSendOnly )
	{
		RtcpTimerTicks();
	}
}


void CRtpUdpReader::RtcpTimerTicks(void)
{
	//发送RTCP

	if( !m_pRtcpSocket || !m_pRtcpSocket->HadRemoteAddr())
	{
		return;
	}

	int rtcp_bytes;
	UINT64 iv = GetNTPTime();
	rtcp_bytes = ((m_octet_count - m_last_octet_count) * RTCP_TX_RATIO_NUM) /RTCP_TX_RATIO_DEN;

	if ( m_first_packet ||
		( rtcp_bytes >= RTCP_SR_SIZE &&	(iv - m_last_rtcp_ntp_time) > 5000000) ) 
	{
		m_csMutex.Lock();
		rtcp_bytes = ((m_octet_count - m_last_octet_count) * RTCP_TX_RATIO_NUM) /RTCP_TX_RATIO_DEN;
		if (  m_first_packet ||
			( rtcp_bytes >= RTCP_SR_SIZE &&	(iv - m_last_rtcp_ntp_time) > 5000000) ) 
		{

			m_last_rtcp_ntp_time = GetNTPTime();
			rtcp_bytes =  MakeRtcpRcvSR();
			m_last_octet_count = m_octet_count;
			m_first_packet = 0;					
			if( rtcp_bytes > 0 )
			{
				struct iovec vIO;
				vIO.iov_base = m_unionRtcpBuf.pRtcpBuffer;
				vIO.iov_len = rtcp_bytes;
				m_pRtcpSocket->SendTo(&vIO, 1 );
			}
		}
		m_csMutex.Unlock();
	}

}


INT CRtpUdpReader::MakeRtcpRcvSR(void)
{
	UINT32 rtp_ts;
	INT64 b;
	INT64 c;
	INT iContents = 6;
	b = 1L*m_iTimestampInterval;
	c = 1000000LL*1;

	rtp_ts = (UINT32) RescaleRnd(m_last_rtcp_ntp_time - m_first_rtcp_ntp_time, b,c, eAV_ROUND_NEAR_INF) +		m_base_timestamp;
	StruRtcpPacket &stPacket = m_unionRtcpBuf.stPacket;
	bzero(&stPacket, sizeof(stPacket) );
	stPacket.common.version = RTP_VERSION;
	stPacket.common.pt = eRTCP_SR;
	stPacket.common.length = COSSocket::Int16H2N(iContents);
	stPacket.r.sr.ssrc = COSSocket::Int32H2N(m_iSSRC);
	stPacket.r.sr.ntp_sec =  COSSocket::Int32H2N((INT32) (m_last_rtcp_ntp_time/1000000) ) ;
	stPacket.r.sr.ntp_frac = COSSocket::Int32H2N( (((m_last_rtcp_ntp_time % 1000000) << 32) / 1000000));
	stPacket.r.sr.rtp_ts = COSSocket::Int32H2N(m_iCurTimestamp);

	stPacket.r.sr.psent = COSSocket::Int32H2N( m_packet_count);
	stPacket.r.sr.osent = COSSocket::Int32H2N( m_octet_count);
	INT iLen = (iContents+1)*sizeof(UINT32);	
	return iLen;    
}

void CRtpUdpReader::RtcpSendBye(void)
{
	if( m_packet_count  && m_pRtcpSocket && m_pRtcpSocket->HadRemoteAddr())
	{
		StruRtcpPacket stPacket;
		INT iContents = 1;
		bzero(&stPacket, sizeof(stPacket) );
		stPacket.common.version = RTP_VERSION;
		stPacket.common.pt = eRTCP_BYE;		
		stPacket.common.length = COSSocket::Int16H2N(iContents);
		stPacket.r.bye.src[0] = COSSocket::Int32H2N(m_iSSRC);
		INT iLen = (iContents+1)*sizeof(UINT32);	
		if(m_pRtcpSocket )
		{
			struct iovec vIO;
			vIO.iov_base = m_unionRtcpBuf.pRtcpBuffer;
			vIO.iov_len = iLen;
			m_pRtcpSocket->SendTo(&vIO, 1 );
		}

	}
}


void* CRtpUdpReader::OnRtcpSocketEvent(CISocket *pSocket, 
									   EnumSocketEvent iEvt,void *pParam, void *pParamExt )
{
	if( !m_bConnected )
	{
		return (void*) FALSE;
	}
	GS_ASSERT_RET_VAL( pSocket == dynamic_cast<CISocket*>(m_pRtcpSocket), FALSE );
	BOOL bRet = TRUE;
	if( iEvt == eEVT_SOCKET_ARCVFROM  )
	{
		m_first_packet = 0;

		const StruLenAndSocketAddr *pRemoteAddr = (StruLenAndSocketAddr *)pParamExt;
		CGSPBuffer *pBuf = (CGSPBuffer *)pParam;
		GS_ASSERT(pRemoteAddr);
		GS_ASSERT(pBuf);

		StruRtcpPacket stRtcpPkt;
		bzero(&stRtcpPkt, sizeof(stRtcpPkt));

		memcpy(&stRtcpPkt, pBuf->m_bBuffer, MIN(pBuf->m_iDataSize, sizeof(stRtcpPkt)));
		RunEventCallback(eEVT_RTPNET_RTCP_PULSE, NULL);

		switch( stRtcpPkt.common.pt )
		{
		case RTP::eRTCP_RR :
			{

				if( m_iSSRC == COSSocket::Int32N2H(stRtcpPkt.r.sr.ssrc ) )
				{
					//是同步源
					m_iRcvRtcpCounts++;
				}				
			}
		break;
		case RTP::eRTCP_BYE:
			{
				RunEventCallback(eEVT_RTPNET_RTCP_BYE, NULL);
				bRet =FALSE;
			}
			break;

		}
	}
	return (void*) bRet;
}

void* CRtpUdpReader::OnRtpSocketEvent(CISocket *pSocket, 
									   EnumSocketEvent iEvt,void *pParam, void *pParamExt )
{
	if( !m_bConnected )
	{
		return (void*) FALSE;
	}
	GS_ASSERT_RET_VAL( pSocket == dynamic_cast<CISocket*>(m_pRtpSocket), FALSE );
	BOOL bRet = TRUE;
	if( iEvt == eEVT_SOCKET_ARCVFROM  )
	{
		m_first_packet = 0;
		m_packet_count++;
		m_octet_count = m_packet_count;
		
		const StruLenAndSocketAddr *pRemoteAddr = (StruLenAndSocketAddr *)pParamExt;
		CGSPBuffer *pBuf = (CGSPBuffer *)pParam;
		GS_ASSERT(pRemoteAddr);
		GS_ASSERT(pBuf);
		if( pBuf->m_iDataSize < sizeof(StruRTPHeader))
		{
			//????
			return (void*)TRUE;
		}
		std::vector<StruRtpPktSn> vLoser;
		m_rtpDecoder.Decode(pBuf,vLoser);
		CFrameCache *pFrame;
		while(m_bConnected &&  (pFrame =m_rtpDecoder.Get() ))
		{
			INT iChn = GetChannelID();
			if( iChn >-1 && iChn>=GSP_MAX_MEDIA_CHANNELS )
			{
				/*GS_ASSERT(0);*/
				iChn = 0;
			}
			pFrame->m_stFrameInfo.iChnNo = iChn;
			pFrame->m_stFrameInfo.eMediaType = m_vBindMediaType[iChn];
			RunEventCallback(eEVT_RTPNET_STREAM_FRAME, pFrame);
			pFrame->UnrefObject();
		}
		
	}
	return (void*) m_bConnected;
}