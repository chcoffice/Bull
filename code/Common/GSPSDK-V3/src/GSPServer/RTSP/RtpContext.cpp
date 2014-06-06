#include "RtpContext.h"
#include "OSThread.h"
#include <time.h>
#include "MediaInfo.h"
#include "../IBaseSource.h"
#include "../RefSource.h"
#include "RTP/RTPUDPPairSocket.h"
#include "RtspSrvSession.h"
#include "RTSP/SDPFormater.h"
#include "RTSP/RTSPAnalyer.h"
#include "MyMemory.h"
#include "Log.h"

using namespace GSP;
using namespace GSP::RTSP;
using namespace GSP::RTP;

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


GSAtomicInter  CRtpContext::s_iAutoSSRC = 0;

INLINE  static INT16 RTP_W16(INT16 val)
{
	INT16 iVal = 0;
	unsigned char *p = (unsigned char *)&iVal;
	*p++ = val>>8;
	*p = (unsigned char)val;
	return iVal;
}

INLINE  static INT32 RTP_W32(INT32 val) 
{
	INT32 iVal = 0;
	unsigned char *p = (unsigned char *)&iVal;
	*p++ = val>>24;
	*p++ = val>>16;
	*p++ = val>>8;
	*p = val;
	return iVal;
}



CRtpContext::CRtpContext(void)
:CGSPObject()
{
    
	for( int i = 0; i<GSP_MAX_MEDIA_CHANNELS; i++)
	{

		m_ppRtphandles[i] = NULL;  
		m_ppRtpBase[i] = NULL;
		
	}

	m_pRtspC  = NULL;
	m_bTeardown = FALSE;
	m_pRefSrc = NULL;

}

CRtpContext::~CRtpContext(void)
{
	 m_bTeardown = TRUE;
	if( m_pRefSrc )
	{
		m_pRefSrc->Stop();		
		m_pRefSrc  =NULL;
	}
	for( int i = 0; i<GSP_MAX_MEDIA_CHANNELS; i++)
	{
		if( m_ppRtphandles[i] )
		{
			delete m_ppRtphandles[i];
			m_ppRtphandles[i] = NULL;
		}
		if( m_ppRtpBase[i] )
		{
			CMemory::Free(m_ppRtpBase[i]);
			m_ppRtpBase[i]= NULL;
		}
	}
	
}

void CRtpContext::DestoryBefore(void)
{
	CGSAutoWriterMutex rlocker(&m_csWRMutex);
	m_bTeardown = TRUE;
	if( m_pRefSrc )
	{
		m_pRefSrc->Stop();		
		m_pRefSrc  = NULL;
	}

	for( int i = 0; i<GSP_MAX_MEDIA_CHANNELS; i++)
	{
		if( m_ppRtphandles[i] )
		{
			delete m_ppRtphandles[i];
			m_ppRtphandles[i] = NULL;
		}		
	}
}

BOOL CRtpContext::RtpNewAVStream(CRtspSrvSession *pSection, CRefSource *pRefSrc, 
					int iStreamId, const char *czDestIP,
					const void *pPortField, /* 类型CRtspTransportParser::StruTransPortField* */
					const char *szLocalIP )
{
	CRtspTransportParser::StruTransPortField *pField = (CRtspTransportParser::StruTransPortField *)pPortField;
	CRtpMyPairSocket *h = NULL;
	UINT iInc = 3600;
	double fRate; 
//	int  iSize;
	EnumGSCodeID eCodeID = GS_CODEID_NONE;
	int iPlayloadType;
	const CIMediaInfo &csInfo = pRefSrc->Source()->MediaInfo();
	const CIMediaInfo::StruMediaChannelInfo *pMediaInfo; 

	CGSWRMutexAutoWrite alocker( &m_csWRMutex );

	m_pRtspC = pSection;

	GS_ASSERT_RET_VAL(m_pRefSrc==NULL, FALSE);

	if( m_ppRtpBase[iStreamId] )
	{
		if( m_eTransPort==pField->eTrType &&
			m_pRefSrc==pRefSrc )
		{
			return TRUE;
		}
		GS_ASSERT(0);
		return FALSE;
	}
	m_ppRtpBase[iStreamId] = (StruRtpInfo*) CMemory::Malloc(sizeof(StruRtpInfo));

	if( !m_ppRtpBase[iStreamId] )
	{
		MY_LOG_FATAL(g_pLog, _GSTX("Create StruRtpInfo object fail.\n"));
		GS_ASSERT(0);
		goto fail;

	}

	InitRtpInfo(*m_ppRtpBase[iStreamId]);


	
	pMediaInfo = csInfo.GetChannel( iStreamId );
	if( !pMediaInfo )
	{
		MY_LOG_FATAL(g_pLog, _GSTX("Get channel info fail.\n"));
		GS_ASSERT(0);
		goto fail;

	}
	
	switch( pMediaInfo->stDescri.eMediaType ) {
		case GS_MEDIA_TYPE_AUDIO :
			eCodeID = (EnumGSCodeID) pMediaInfo->stDescri.unDescri.struAudio.eCodeID;
			iInc = 3200;
			break;
		case GS_MEDIA_TYPE_PICTURE :
			eCodeID = (EnumGSCodeID)pMediaInfo->stDescri.unDescri.struPicture.eCodeID; 
			break;
		case GS_MEDIA_TYPE_VIDEO :
			eCodeID = (EnumGSCodeID) pMediaInfo->stDescri.unDescri.struVideo.eCodeID; 
			fRate = 25.0;
			if( pMediaInfo->stDescri.unDescri.struVideo.iFrameRate2 )
			{

				fRate =  pMediaInfo->stDescri.unDescri.struVideo.iFrameRate2/10.0;
				while( fRate > 1.0 )
				{
					fRate = fRate/10.0;
				}
			} 
			else if( pMediaInfo->stDescri.unDescri.struVideo.iFrameRate )
			{
				fRate = 0.0;
			}
			fRate += pMediaInfo->stDescri.unDescri.struVideo.iFrameRate;   

			iInc =(UINT) (1000000.0/fRate);
			break; 
		case GS_MEDIA_TYPE_OSD :
			eCodeID = GS_CODEID_BINARY;
			break;        
		default :
			eCodeID = GS_CODEID_NONE;
			break;
	}
	//转换为RTP PT
	iPlayloadType =  CSDPFormater::GSCodecID2RTPPlayType(eCodeID);
	if( iPlayloadType == eRTP_PT_GXX )
	{
		iPlayloadType += pMediaInfo->iSubChannel;
	}
	m_eTransPort = pField->eTrType;
	if( m_eTransPort==eTRANSPORT_RTP_UDP )
	{

	h = new CRtpMyPairSocket(NULL);
	if( !h )
	{
		MY_LOG_FATAL(g_pLog , _GSTX("Crate CRtpMyPairSocket Object fail.\n") );
		GS_ASSERT(0);
		goto fail;
	}

	if( !h->Connect(czDestIP,pField->stCliPort.vPort[RTP_PORT_IDX],
		pField->stCliPort.vPort[RTCP_PORT_IDX] ) )
	{
		MY_LOG_FATAL(g_pLog, _GSTX("Connect to rtp %s:(%d,%d) fail.\n"), czDestIP, pField->stCliPort.vPort[RTP_PORT_IDX],pField->stCliPort.vPort[RTCP_PORT_IDX]  );
		GS_ASSERT(0);
		goto fail;
	}
	//  Transport: RTP/AVP;unicast;destination=192.168.15.100;source=192.168.15.140;client_port=11080-11081;server_port=6970-6971\r\n

	pField->stSrvPort.vPort[RTP_PORT_IDX] = h->GetPair()->GetRTPSocket()->LocalPort();
	pField->stSrvPort.vPort[RTCP_PORT_IDX] = h->GetPair()->GetRTCPSocket()->LocalPort();
	if( szLocalIP )
	{
	  pField->strSrvAddr = szLocalIP;
	}
	pField->strDestAddr = czDestIP;

	m_ppRtpBase[iStreamId]->m_stRTPHeader.iPT = iPlayloadType&0x7F;	
	m_ppRtpBase[iStreamId]->m_stRTPHeader.iSSRC = COSSocket::Int32H2N(AtomicInterInc(s_iAutoSSRC));
	m_ppRtpBase[iStreamId]->m_iTimestampInc = iInc;
	m_ppRtpBase[iStreamId]->m_iSeq = 0;
	m_ppRtphandles[iStreamId] = h;
	h->SetUserData((void*)iStreamId);
	h->SetEventListener(this, (CRtpPairSocket::PairSocketEventFunctionPtr) &CRtpContext::OnRtpPairSocketEvent);
	}	
	if( !m_pRefSrc)
	{
		m_pRefSrc = pRefSrc;		
	}

	return TRUE;
fail :
	if( h )
	{
		delete h;
	}
	if( m_ppRtpBase[iStreamId] )
	{
		CMemory::Free(m_ppRtpBase[iStreamId]);
		m_ppRtpBase[iStreamId]  = NULL;
	}
	return FALSE;

}

void CRtpContext::InitRtpInfo( StruRtpInfo &stInfo)
{
	bzero(&stInfo, sizeof(stInfo));

	stInfo.m_iTimestampInc = 3600;
	INIT_DEFAULT_RTH_HEADER(&stInfo.m_stRTPHeader);

	stInfo.m_base_timestamp = (UINT32) COSThread::Milliseconds();
	stInfo.m_cur_timestamp = stInfo.m_base_timestamp;

	stInfo.m_num_frames = 0;

	/* rtcp sender statistics receive */
	stInfo.m_last_rtcp_ntp_time = GetNTPTime();
	stInfo.m_first_rtcp_ntp_time = stInfo.m_last_rtcp_ntp_time;  

	/* rtcp sender statistics */
	stInfo.m_packet_count = 0;
	stInfo.m_octet_count = 0;      
	stInfo.m_last_octet_count = 0; 
	stInfo.m_first_packet = TRUE;

}

EnumErrno CRtpContext::Play(INT iStreamID)
{
	CGSAutoReaderMutex rlocker(&m_csWRMutex);
	if( iStreamID<0 )
	{
		for( iStreamID  = 0; iStreamID<GSP_MAX_MEDIA_CHANNELS; iStreamID++ )
		{
			if( m_ppRtpBase[iStreamID] )
			{
				m_ppRtpBase[iStreamID]->m_bPlay = TRUE;
			}
		}
	}
	else
	{

		GS_ASSERT_RET_VAL(m_ppRtpBase[iStreamID], eERRNO_SYS_ESTATUS);
		m_ppRtpBase[iStreamID]->m_bPlay = TRUE;
	}
	return eERRNO_SUCCESS;
}

EnumErrno CRtpContext::Pause(INT iStreamID)
{
	CGSAutoReaderMutex rlocker(&m_csWRMutex);
	if( iStreamID<0 )
	{
		for( iStreamID = 0; iStreamID<GSP_MAX_MEDIA_CHANNELS; iStreamID++ )
		{
			if( m_ppRtpBase[iStreamID] )
			{
				m_ppRtpBase[iStreamID]->m_bPlay = FALSE;
			}
		}
	}
	else
	{

		GS_ASSERT_RET_VAL(m_ppRtpBase[iStreamID], eERRNO_SYS_ESTATUS);
		m_ppRtpBase[iStreamID]->m_bPlay = FALSE;
	}
	return eERRNO_SUCCESS;
}

void CRtpContext::Teardown(void)
{
	if( m_pRefSrc) 
	{
		m_pRefSrc->Stop();
	}
	CGSAutoReaderMutex rlocker(&m_csWRMutex);
	m_bTeardown = TRUE;
	
	for(int i = 0; i<GSP_MAX_MEDIA_CHANNELS; i++ )
	{
		if(m_ppRtpBase[i] )
		{
			m_ppRtpBase[i]->m_bPlay = FALSE;	
		}
	}

	
}

void CRtpContext::TimerTicks(void)
{
	CGSAutoWriterMutex wlocker(&m_csWRMutex);

	for(INT iStreamID  = 0; iStreamID<GSP_MAX_MEDIA_CHANNELS; iStreamID++ )
	{
		if( m_ppRtpBase[iStreamID] && !m_ppRtpBase[iStreamID]->m_bPlay )
		{
			StruRtpInfo *pInfo;	
			int rtcp_bytes;
			UINT64 iv = GetNTPTime();

			pInfo = m_ppRtpBase[iStreamID];
			rtcp_bytes = ((pInfo->m_octet_count - pInfo->m_last_octet_count) * RTCP_TX_RATIO_NUM) /RTCP_TX_RATIO_DEN;

			// MY_PRINTF("**%d %d  %lld-%lld = %ld\n",
			//           pInfo->m_first_packet, rtcp_bytes,iv, pInfo->m_last_rtcp_ntp_time, 
			// 		  (long) (iv - pInfo->m_last_rtcp_ntp_time)  );
			if ( ((rtcp_bytes >= RTCP_SR_SIZE) &&
				( (iv - pInfo->m_last_rtcp_ntp_time) > 5000000))) 
			{

				/*   MY_PRINTF( "%ld ********send rtcp.\n", (unsigned long) time(NULL) ); */
				pInfo->m_last_rtcp_ntp_time = GetNTPTime();
				rtcp_bytes =  MakeRtcpSendSR(pInfo);
				if( rtcp_bytes > 0 )
				{
					SendRTCP(iStreamID, pInfo->m_pRTCPBuffer, rtcp_bytes );
				}
				pInfo->m_last_octet_count = pInfo->m_octet_count;
				pInfo->m_first_packet = 0;
			}

		}
	}

}

EnumErrno CRtpContext::SendFrame( CSliceFrame *pFrame)
{
	CGSAutoReaderMutex rlocker(&m_csWRMutex);

	if( m_bTeardown )
	{
		return eERRNO_SRC_EUNUSED;
	}

	INT iChn = pFrame->m_iChnID;

	GS_ASSERT_RET_VAL( iChn>=0 && iChn<GSP_MAX_MEDIA_CHANNELS, eERRNO_SYS_EINVALID );
	GS_ASSERT_RET_VAL( m_ppRtpBase[iChn], eERRNO_SRC_EUNUSED);
	//GSP_ASSERT_RET_VAL(rtp_handles[iChn],CISource::SRC_RET_EINVALID);

	if( !m_ppRtpBase[iChn]->m_bPlay || m_bTeardown )
	{
		return eERRNO_SRC_EUNUSED;
	}
	INT64 iCur = COSThread::Milliseconds();


	CRtpSliceFrame csFrame;	
	char *pRTCPData = NULL;
	INT iLen = 0;
	if( Format((UINT32)iCur, pFrame,  &csFrame,&pRTCPData, iLen  ) )
	{
		if( csFrame.m_lMember.Size() )
		{
			SendRTP(iChn, &csFrame);			
		}
		if( pRTCPData )
		{
			SendRTCP(iChn, pRTCPData, iLen);
		}
	}
	else
	{
		GS_ASSERT(0);
		return eERRNO_SYS_ENMEM;
	}
	return eERRNO_SUCCESS;
}


BOOL CRtpContext::Format(const UINT32 iTimeStamp,const CSliceFrame *pInFrame, CRtpSliceFrame *pOutFrame,
                            char **ppRtcpData, INT &iRtcpDataLen )
{

StruRtpInfo *pInfo;	
int rtcp_bytes;
  
UINT64 iv = GetNTPTime();

	pInfo = m_ppRtpBase[pInFrame->m_iChnID];
	 rtcp_bytes = ((pInfo->m_octet_count - pInfo->m_last_octet_count) * RTCP_TX_RATIO_NUM) /RTCP_TX_RATIO_DEN;

// MY_PRINTF("**%d %d  %lld-%lld = %ld\n",
//           pInfo->m_first_packet, rtcp_bytes,iv, pInfo->m_last_rtcp_ntp_time, 
// 		  (long) (iv - pInfo->m_last_rtcp_ntp_time)  );
    if (pInfo->m_first_packet || ((rtcp_bytes >= RTCP_SR_SIZE) &&
        ( (iv - pInfo->m_last_rtcp_ntp_time) > 5000000))) 
    {

         /*   MY_PRINTF( "%ld ********send rtcp.\n", (unsigned long) time(NULL) ); */
            pInfo->m_last_rtcp_ntp_time = GetNTPTime();
            iRtcpDataLen =  MakeRtcpSendSR(pInfo);
            if( iRtcpDataLen > 0 )
            {
                *ppRtcpData = pInfo->m_pRTCPBuffer;
            }
            pInfo->m_last_octet_count = pInfo->m_octet_count;
            pInfo->m_first_packet = 0;
    }
	pInfo->m_cur_timestamp = (UINT32) COSThread::Milliseconds();
    pInfo->m_stRTPHeader.iTS = RTP_W32(iTimeStamp);

	if(eERRNO_SUCCESS==pOutFrame->Build(pInFrame,pInfo->m_stRTPHeader,pInfo->m_iSeq ) )
	{
		pInfo->m_packet_count += pOutFrame->m_lMember.Size();
		pInfo->m_octet_count = pInfo->m_packet_count;
		
	}
	else
	{
		return FALSE;
	}
    
    return TRUE;
}






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

/* send an rtcp sender report packet */
INT CRtpContext::MakeRtcpSendSR( StruRtpInfo *pRtpInfo )
{
    UINT32 rtp_ts;
    INT64 b;
    INT64 c;
     INT iContents = 6;
    b = 1L*pRtpInfo->m_iTimestampInc;
    c = 1000000LL*1;
   
    rtp_ts = (UINT32) RescaleRnd(pRtpInfo->m_last_rtcp_ntp_time - pRtpInfo->m_first_rtcp_ntp_time, b,c, eAV_ROUND_NEAR_INF) + pRtpInfo->m_base_timestamp;
    StruRtcpPacket stPacket;
    bzero(&stPacket, sizeof(stPacket) );
    stPacket.common.version = GSS_RTP_VERSION;
    stPacket.common.pt = eRTCP_SR;
	stPacket.common.length = COSSocket::Int16H2N(iContents);
    stPacket.r.sr.ssrc = pRtpInfo->m_stRTPHeader.iSSRC;
    stPacket.r.sr.ntp_sec =  COSSocket::Int32H2N((INT32) (pRtpInfo->m_last_rtcp_ntp_time/1000000) ) ;
    stPacket.r.sr.ntp_frac = COSSocket::Int32H2N( (((pRtpInfo->m_last_rtcp_ntp_time % 1000000) << 32) / 1000000));
    stPacket.r.sr.rtp_ts =COSSocket::Int32H2N(rtp_ts);

    stPacket.r.sr.psent = COSSocket::Int32H2N( pRtpInfo->m_packet_count);
    stPacket.r.sr.osent = COSSocket::Int32H2N( pRtpInfo->m_octet_count);

    INT iLen = (iContents+1)*sizeof(UINT32);

    ::memcpy(pRtpInfo->m_pRTCPBuffer, &stPacket, iLen);
    return iLen;    
}


void CRtpContext::HandleRTCP(const  char *pRtcpData, INT iRtcpDataLen,
                CRefBuffer **ppOutData,
                char **ppORtcpData, INT &iORtcpDataLen )
{
   /* MY_PRINTF( "Rcv RTCP packet:%ld\n", time(NULL) );*/
}


void CRtpContext::SendRTP(INT iStreamID, CRtpSliceFrame *pFrame)
{
	if(  m_ppRtphandles[iStreamID] )
	{
		 m_ppRtphandles[iStreamID]->SendRtp(pFrame);
	}
}


void CRtpContext::SendRTCP(INT iStreamID, const char  *pData, INT iLen )
{
	if( m_ppRtphandles[iStreamID] )
	{

	  INT iRet =  m_ppRtphandles[iStreamID]->Send(pData, iLen, FALSE);
	   MY_DEBUG("RTCP Send: %d\n", iRet );
	}
}

void *CRtpContext::OnRtpPairSocketEvent(CGSPObject *pSender, 
						   BOOL bRTPSocket, EnumSocketEvent iEvt,void *pParam)
{
	MY_DEBUG("RTCP Rcv..\n" );
	if( m_bTeardown )
	{
		return FALSE;
	}
	if( m_pRtspC )
	{
		m_pRtspC->OnRtpContentKeepalive();
	}
	if( bRTPSocket )
	{
		return (void*)FALSE;
	}
	
	return (void*)TRUE;
}

