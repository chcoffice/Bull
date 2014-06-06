/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : RTPCONTEXT.H
Author :  zouyx
Version : 0.0.0.0
Date: 2011/12/1 9:27
Description: 
********************************************
*/

#ifndef _GS_H_RTPCONTEXT_H_
#define _GS_H_RTPCONTEXT_H_

#include "RTSP/RTSPAnalyer.h"
#include "MySocket.h"



namespace GSP
{
	
class CSliceFrame;	
class CRefSource;
namespace RTP
{

class CRtpMyPairSocket;
}

namespace RTSP
{

	
	class CRtspSrvSession;


class CRtpContext :
    public CGSPObject
{     
private :
    static GSAtomicInter s_iAutoSSRC;

	#define iConstRtpBufLength  1500
	#define iConstRtcpBufLength 1500

	typedef struct _StruRtpInfo
	{
		UINT32 m_iTimestampInc; 
		StruRTPHeader m_stRTPHeader;
		UINT16 m_iSeq;
		UINT32 m_base_timestamp;
		UINT32 m_cur_timestamp;

		INT m_num_frames;

		/* rtcp sender statistics receive */
		INT64 m_last_rtcp_ntp_time;    
		INT64 m_first_rtcp_ntp_time;  

		/* rtcp sender statistics */
		unsigned int m_packet_count;     
		unsigned int m_octet_count;      
		unsigned int m_last_octet_count; 
		int m_first_packet;


		INT iInterleavedOfRtpChnID;
		INT iInterleavedOfRtcpChnID;

		//  int max_frames_per_packet;

		/**
		* Number of bytes used for H.264 NAL length, if the MP4 syntax is used
		* (1, 2 or 4)
		*/
		//  int nal_length_size;		
		char m_pRTCPBuffer[iConstRtcpBufLength];
		
		BOOL m_bPlay;

		//INT64 iPkts;
		
	}StruRtpInfo;


   


	/* RTP/UDP specific */
	RTP::CRtpMyPairSocket *m_ppRtphandles[GSP_MAX_MEDIA_CHANNELS];  
	StruRtpInfo *m_ppRtpBase[GSP_MAX_MEDIA_CHANNELS];


	/* RTP/TCP specific */
	CRtspSrvSession *m_pRtspC;

	CRefSource *m_pRefSrc;
	CGSWRMutex m_csWRMutex;
	EnumTransportMode m_eTransPort;
	BOOL m_bTeardown;

public :
    CRtpContext(void);
    virtual ~CRtpContext(void);

	void DestoryBefore(void);

	INLINE BOOL IsExistStream( INT iSteramId)
	{
		return (m_ppRtpBase[iSteramId]!=NULL);
	}

	BOOL RtpNewAVStream(CRtspSrvSession *pSection, CRefSource *pRefSrc, 
					int iStreamId, const char *czDestIP,
					const void *pPortField, /* ¿‡–ÕCRtspTransportParser::StruTransPortField* */
					const char *szLocalIP );
	EnumErrno Play(INT iStreamID);
	EnumErrno Pause(INT iStreamID);


	void Teardown(void);

	EnumErrno SendFrame( CSliceFrame *pFrame);

	void TimerTicks(void);


private :
	INLINE static void InitRtpInfo( StruRtpInfo &stInfo);
  

    BOOL Format(const UINT32 iTimeStamp, const CSliceFrame *pInFrame, CRtpSliceFrame *pOutFrame,
                    char **ppRtcpData, INT &iRtcpDataLen );

    void HandleRTCP(const  char *pRtcpData, INT iRtcpDataLen,
                     CRefBuffer **ppOutData,
                     char **ppORtcpData, INT &iORtcpDataLen ); 

	

  

	void *OnRtpPairSocketEvent(CGSPObject *pSender, 
		BOOL bRTPSocket, EnumSocketEvent iEvt,void *pParam);
    static UINT64 GetNTPTime(void);
 
	void SendRTP(INT iStreamID, CRtpSliceFrame *pFrame);
	void SendRTCP(INT iStreamID, const char  *pData, INT iLen );

    INT MakeRtcpSendSR(StruRtpInfo *pRtpInfo);

};




}
}




#endif //end _GS_H_RTPCONTEXT_H_
