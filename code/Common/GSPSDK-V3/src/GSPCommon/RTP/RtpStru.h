/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : RTPSTRU.H
Author :  邹阳星
Version : 1.0.0.1
Date: 2012/11/1 10:59
Description: 
********************************************
*/

#ifndef _GS_H_RTPSTRU_H_
#define _GS_H_RTPSTRU_H_
#include "GSMediaDefs.h"

namespace GSP
{

namespace RTP
{

#ifdef _WIN32 
#pragma pack( push,1 )
#endif

	typedef enum  {
		eRTCP_SR     = 200,
		eRTCP_RR,   // 201
		eRTCP_SDES, // 202
		eRTCP_BYE,  // 203
		eRTCP_APP   // 204
	}EnumRTCPType;

	typedef struct _StruRTPHeader
	{

		UINT8 iCC : 4;    /* CSRC count */
		UINT8 iX : 1;     /* header extension flag */
		UINT8 iP : 1;     /* padding flag */
		UINT8 iVer : 2;    /* protocol version */        
		UINT8 iPT : 7;   /* payload type */
		UINT8 iM : 1;     /* marker bit */ 
		UINT16 iSeq;      /* sequence number */
		UINT32 iTS;       /* timestamp */
		UINT32 iSSRC;     /* synchronization source */
		// UINT32 csrc[0]; /* optional CSRC list */
	} GS_MEDIA_ATTRIBUTE_PACKED StruRTPHeader;


#define RTP_VERSION 2
#define DEFAULT_RTP_HEADER_VALUE {RTP_VERSION,0,0,0,0,0,0,0,0}
#define INIT_DEFAULT_RTH_HEADER(x) do{ memset(x,0,sizeof(GSP::RTP::StruRTPHeader)); (x)->iVer=RTP_VERSION; }while(0)

#define RTP_PACKET_HEADER_LENGTH sizeof(GSP::RTP::StruRTPHeader)


	typedef struct _StruRtcpCommon
	{           

		UINT8 count:5; /* varies by packet type */ 
		UINT8 p:1;      /* padding flag */ 
		UINT8 version:2; /* protocol version */ 
		UINT8 pt;    /* RTCP packet type */ 
		UINT16 length; /* pkt len in words, w/o this word */
	} GS_MEDIA_ATTRIBUTE_PACKED StruRtcpCommon;

	typedef struct { 
		UINT32 ssrc; /* data source being reported */ 
		UINT32 fraction: 8; /* fraction lost since last SR/RR */ 
		UINT32 lost: 24; /* cumul. no. pkts lost (signed!) */ 
		UINT32 last_seq; /* extended last seq. no. received */ 
		UINT32 jitter; /* interarrival jitter */ 
		UINT32 lsr; /* last SR packet from this source */ 
		UINT32 dlsr; /* delay since last SR packet */
	} GS_MEDIA_ATTRIBUTE_PACKED StruRtcpRR;

	typedef struct _StruRtcpSDESItem{ 
		UINT8 type; /* type of item (rtcp_sdes_type_t) */ 
		UINT8 length; /* length of item (in octets) */ 
		char data[1]; /* text, not null-terminated */
	} GS_MEDIA_ATTRIBUTE_PACKED StruRtcpSDESItem;

	typedef struct _StruRtcpPacket{ 
		StruRtcpCommon common; /* common header */ 
		union {
			/* sender report (SR) */
			struct 
			{ 
				UINT32 ssrc; /* sender generating this report */ 
				UINT32 ntp_sec; /* NTP timestamp */ 
				UINT32 ntp_frac; 
				UINT32 rtp_ts; /* RTP timestamp */ 
				UINT32 psent; /* packets sent */ 
				UINT32 osent; /* octets sent */ 
				StruRtcpRR rr[1]; /* variable-length list */
			} sr;
			/* reception report (RR) */
			struct 
			{ 
				UINT32 ssrc; /* receiver generating this report */ 
				StruRtcpRR rr[1]; /* variable-length list */
			} rr;
			/* source description (SDES) */
			struct rtcp_sdes 
			{ 
				UINT32 src; /* first SSRC/CSRC */ 
				StruRtcpSDESItem item[1]; /* list of SDES items */
			} sdes;
			/* BYE */
			struct 
			{ 
				UINT32 src[1]; /* list of sources */ 
				/* can't express trailing text for reason */
			} bye;
		} r;
	} GS_MEDIA_ATTRIBUTE_PACKED StruRtcpPacket;

	typedef enum  {
		eRTP_PT_ULAW = 0,
		eRTP_PT_GSM = 3,
		eRTP_PT_G723 = 4,
		eRTP_PT_ALAW = 8,
		eRTP_PT_S16BE_STEREO = 10,
		eRTP_PT_S16BE_MONO = 11,
		eRTP_PT_MPEGAUDIO = 14,
		eRTP_PT_JPEG = 26,
		eRTP_PT_H261 = 31,
		eRTP_PT_MPEGVIDEO = 32,
		eRTP_PT_MPEG2TS = 33,
		eRTP_PT_H263 = 34, /* old H263 encapsulation */

		//eRTP_PT_MPEG4VIDEO = 96,     //added by ivan
		eRTP_PT_ADPCM = 36,          //added by ivan
		eRTP_PT_MJPEG = 37,          //added by ivan
		/*eRTP_PT_H264 = 38,          //added by ivan*/

		eRTP_PT_PS = 96, // add by zouyx of 28181
		eRTP_PT_MP4 = 97, //add by zouyx of 28181
		eRTP_PT_H264 = 98, //add by zouyx of 28181

		eRTP_PT_GXX = 99,  //added by zouyx

		
		

		eRTP_PT_HIK = 100,  //added by zouyx HK


		eRTP_PT_C3MVIDEO = 101, // added by zouyx c3mvideo

	} EnumRTPPayloadType;




	typedef enum
	{
		eTRANSPORT_RTP_UDP = 0,           /**< UDP/unicast */
		eTRANSPORT_RTP_TCP = 1,           /**< TCP; interleaved in RTSP */
		eTRANSPORT_RTP_UDP_MULTICAST = 2, /**< UDP/multicast */
		eTRANSPORT_RAW_UDP = 3, // RAW/UDP
	}EnumTransportMode;









	typedef struct _struDTS
	{
		UINT8 iMarkerBit1 : 1;
		UINT8  iPTS1 : 3; // [32..30]
		UINT8 iLabel : 4; // == "0010"



		UINT16 iMarkerBit2 : 1;
		UINT16 	PTS2 : 15; // [29..15]

		UINT16 iMarkerBit3 : 1;
		UINT16 	PTS3 : 15; // [14..0]


	} GS_MEDIA_ATTRIBUTE_PACKED StruDTS;

	// 		typedef struct _struDTS11
	// 		{
	// 			UINT8 iLabel : 4; // == "0011"
	// 			UINT8  iPTS1 : 3; // [32..30]
	// 			UINT8 iMarkerBit1 : 1;
	// 			UINT16 	PTS2 : 15; // [29..15]
	// 			UINT16 iMarkerBit2 : 1;
	// 			UINT16 	PTS3 : 15; // [14..0]
	// 			UINT16 iMarkerBit3 : 1;
	// 
	// 
	// 			UINT8 iLabel2 : 4; // == "0001"
	// 			UINT8  iPTS12 : 3; // [32..30]
	// 			UINT8 iMarkerBit12 : 1;
	// 			UINT16 	PTS22 : 15; // [29..15]
	// 			UINT16 iMarkerBit22 : 1;
	// 			UINT16 	PTS32 : 15; // [14..0]
	// 			UINT16 iMarkerBit32 : 1;
	// 
	// 		}StruDTS11;



	typedef struct _StruPESPacket
	{
		UINT32 iHeaderMark;
		UINT8 iStreamID;
		UINT16 iPacketLen;

		UINT8 iScramblingControl : 2;
		UINT8 iPriority : 1;
		UINT8 iDataAlignmentIndicator : 1;
		UINT8 iCopyright : 1;
		UINT8 iOriginalOrCopy : 1;
		UINT8 iLabel : 2; // == "01"	


		UINT8 iESCR_flag : 1;
		UINT8 iES_rate_flag : 1;
		UINT8 iDSM_trick_mode_flag :1;
		UINT8 iAdditional_copy_info_flag :1;
		UINT8 iPES_CRC_flag : 1;
		UINT8 iPES_extension_flag : 1;
		UINT8 iPTS_DTS_flags : 2;




		UINT8 PES_header_data_length : 8;   // == 5
		StruDTS stDTS10;  //if (PTS_DTS_flags =='10' ) // iLabel = "0010"
	} GS_MEDIA_ATTRIBUTE_PACKED StruPESPacket;


	typedef struct _StruPSHeader
	{


		UINT32 pack_start_code;  //0x000001BA

		UINT8  iLabel : 2 ;   // ==	  '01'
		UINT8  system_clock_reference_base : 3; // [32..30]
		UINT8  marker_bit1 : 1;

		UINT8  a : 2;
		UINT8  b;
		UINT8  c : 5;

		//UINT16 system_clock_reference_base : 15 ; // [29..15]

		UINT8 marker_bit2 : 1;


		UINT8  d : 2;
		UINT8  e;

		UINT16  h: 5;
		//UINT16 system_clock_reference_base : 15; // [14..0] 15 bslbf
		UINT16 marker_bit3 : 1;
		UINT16 system_clock_reference_extension : 9;
		UINT16  marker_bit4 :1;


		UINT32 program_mux_rate:22;
		UINT32 marker_bit5 : 1;
		UINT32 marker_bit6 : 1;
		UINT32  reserved : 5;
		UINT32 pack_stuffing_length : 3;
	}StruPSHeader;


	#define PS_HEADER_SIZE 14



	typedef struct _StruPSSystemHeader
	{
		UINT32 system_header_start_code; // 0x000001BB
		UINT16 header_length;
		UINT8  marker_bit1 : 1;
		UINT32 rate_bound : 22;
		UINT8  marker_bit2 : 1;
		UINT8 audio_bound : 6;
		UINT8 fixed_flag : 1;
		UINT8 CSPS_flag : 1;
		UINT8 system_audio_lock_flag : 1;
		UINT8 system_video_lock_flag :1;
		UINT8  marker_bit3 : 1;
		UINT8 video_bound : 5;
		UINT8 packet_rate_restriction_flag : 1;
		UINT8 reserved_byte : 1;

	}StruPSSystemHeader;


// 	typedef struct _StruPSPSM
// 	{
// 		UINT32 system_header_start_code : 24; // 0x000001
// 
// 	};


#ifdef _WIN32
#pragma pack( pop )
#endif

	typedef struct _struUdpPortPair
	{
		UINT16 vPort[2];
		_struUdpPortPair(void)

		{
			vPort[0] = 0;
			vPort[1] = 1;
		}
	}StruUdpPortPair;


	#define RTP_PORT_IDX 0
	#define RTCP_PORT_IDX 1

#define MY_EXT_CODEID_START  0x20000000
	#define GS_CODEID_PS_GXX	    ((EnumGSCodeID)(MY_EXT_CODEID_START+1))
	#define GS_CODEID_UNKNOW	     ((EnumGSCodeID)(MY_EXT_CODEID_START+2))
	#define GS_CODEID_GSC3MVIDEO	((EnumGSCodeID)(MY_EXT_CODEID_START+3))

	EnumGSCodeID GetGsCodeId4RtpPtName(const CGSString &strRtpPtName);
	EnumRTPPayloadType MakeRtpPtInfo4GsCodeId(const EnumGSCodeID eCodeId, CGSString &strRtpPtName);

	
	

	EnumGSMediaType MediaTypeName2I( const CGSString &strName );
	const char *MediaTypeI2Name( EnumGSMediaType eType );


	//是否允许PS 封装
	BOOL GsCodeIDTestPT_PS(EnumGSCodeID eCodeId );
	//是否允许H264 封装
	BOOL GsCodeIDTestPT_H264(EnumGSCodeID eCodeId );
	//是否允许MPEG4 封装
	BOOL GsCodeIDTestPT_MP4(EnumGSCodeID eCodeId );



	const char *TransportModeI2Name(EnumTransportMode eType);
	EnumTransportMode TransportModeName2I( const char *czName);



	UINT64 GetNTPTime(void);


	
	
	#define GS_MKTAG(a,b,c,d) (a | (b << 8) | (c << 16) | (d << 24))
	#define GS_MKBETAG(a,b,c,d) (d | (c << 8) | (b << 16) | (a << 24))



#define MAX_RTP_SIZE			1548				// RTP包最大长度
#define MAX_RTP_PAYLOAD_SIZE		1400			// RTP负载最大长度
#define MAX_RTP_HEADER_SIZE			128                //最大RTP 头大小

} //end namespace RTP


} //end namespace GSP

#endif //end _GS_H_RTPSTRU_H_
