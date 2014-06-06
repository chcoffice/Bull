/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : RTSPSTRU.H
Author :  zouyx
Version : 0.0.0.0
Date: 2011/11/24 9:34
Description: RTSP/RTP/RTCP 用到的结构
********************************************
*/

#ifndef _GS_H_RTSPSTRU_H_
#define _GS_H_RTSPSTRU_H_
#include "GSMediaDefs.h"
#include "../RTP/RtpStru.h"

using namespace GSP::RTP;

namespace GSP
{
namespace RTSP
{

	typedef enum EnumHTTPState {
		eRTSPSTATE_NONE = 0,
		eRTSPSTATE_INIT,
		eRTSPSTATE_ASSERT, 

		eHTTPSTATE_WAIT_REQUEST,
		eHTTPSTATE_SEND_HEADER,
		eHTTPSTATE_SEND_DATA_HEADER,
		eHTTPSTATE_SEND_DATA,          /* sending TCP or UDP data */
		eHTTPSTATE_SEND_DATA_TRAILER,
		eHTTPSTATE_RECEIVE_DATA,       
		eHTTPSTATE_WAIT_FEED,          /* wait for data from the feed */
		eHTTPSTATE_WAIT,               /* wait before sending next packets */
		eHTTPSTATE_WAIT_SHORT,         /* short wait for short term 
									  bandwidth limitation */
		eHTTPSTATE_READY,

		eRTSPSTATE_WAIT_REQUEST,
		eRTSPSTATE_SEND_REPLY,
		eRTSPSTATE_SEND_PACKET,

		
	}EnumHTTPState;

#define MAX_HTTP_PROTOCOL_LINE_LEN  (KBYTES*2)
#define MAX_HTTP_RTP_TCP_LINE   (KBYTES*512)
#define MAX_HTTP_RTP_UDP_LINE   (1500)

#define MAX_RTSP_PROTOCOL_LINE_LEN MAX_HTTP_PROTOCOL_LINE_LEN

#define MAX_RTP_UDP_PACKET_LEN  MAX_HTTP_RTP_UDP_LINE


#define HTTP_REQUEST_TIMEOUT (15 * 1000)
#define RTSP_REQUEST_TIMEOUT (3600 * 24 * 1000)


#define RTSP_GXX_USERAGENT  "GoSunCn media player (V2013.8.1)"

#define RTSP_IS_GXX_USERAGENT(x)   (x!=NULL && strstr(x,"GoSunCn ") != NULL)



	// RTP / RTCP 的有关定义
#ifdef _WIN32 
#pragma pack( push,1 )
#endif

	

	#define GSP_RTP_CMD_STRUCT_PLAYLOAD_SIZE 511
	typedef struct _StruRtpGspCommandPacket
	{
		RTP::StruRTPHeader stRtpHeader;
		unsigned char iCommandID;
		unsigned char bPlayload[GSP_RTP_CMD_STRUCT_PLAYLOAD_SIZE];
	} GS_MEDIA_ATTRIBUTE_PACKED StruGspCommandPacket;


	typedef struct _StruRtpGspCmddRetrySend
	{
		//重传请求包
		UINT32 iTS;
		UINT16 iSeq;
		UINT8 iPT;
	}GS_MEDIA_ATTRIBUTE_PACKED StruRtpGspCmddRetrySend;

#define GSP_RTP_CMD_PACKET_MIN_LENGTH (sizeof(StruRTPHeader)+1)




#define RTCP_VALID_MASK (0xc000 | 0x2000 | 0xfe) 
#define RTCP_VALID_VALUE ((GSS_RTP_VERSION << 14) | eRTCP_SR)





	typedef  enum
	{
		eCONT_TYPE_INVALID = -1,
		eCONT_TYPE_SDP = 0,
		eCONT_TYPE_BIN,
		eCONT_TYPE_HTML,
		eCONT_TYPE_PACKET,
		eCONT_TYPE_PARAMS,
	}EnumContentType;

	typedef enum 
	{
		eRTSP_CONTINUE							= 100,

		eRTSP_OK									= 200,
		eRTSP_CREATED							= 201,
		eRTSP_LOW_ON_STORAGE_SPACE				= 250,

		eRTSP_MULTIPLE_CHOICES					= 300,
		eRTSP_MOVED_PERMANENTLY					= 301,
		eRTSP_MOVED_TEMPORARILY					= 302,
		eRTSP_SEE_OTHER							= 303,
		eRTSP_USE_PROXY							= 305,

		eRTSP_BAD_REQUEST						= 400,
		eRTSP_UNAUTHORIZED						= 401,
		eRTSP_PAYMENT_REQUIRED					= 402,
		eRTSP_FORBIDDEN							= 403,
		eRTSP_NOT_FOUND							= 404,
		eRTSP_METHOD_NOT_ALLOWED					= 405,
		eRTSP_NOT_ACCEPTABLE						= 406,
		eRTSP_PROXY_AUTHENTICATION_REQUIRED		= 407,
		eRTSP_REQUEST_TIMEOUT					= 408,
		eRTSP_GONE								= 410,
		eRTSP_LENGTH_REQUIRED					= 411,
		eRTSP_PRECONDITION_FAILED				= 412,
		eRTSP_REQUEST_ENTITY_TOO_LARGE			= 413,
		eRTSP_REQUEST_URI_TOO_LONG				= 414,
		eRTSP_UNSUPPORTED_MEDIA_TYPE				= 415,
		eRTSP_INVALID_PARAMETER					= 451,
		eRTSP_ILLEGAL_CONFERENCE_IDENTIFIER		= 452,
		eRTSP_NOT_ENOUGH_BANDWIDTH				= 453,
		eRTSP_SESSION_NOT_FOUND					= 454,
		eRTSP_METHOD_NOT_VALID_IN_THIS_STATE	= 455,
		eRTSP_HEADER_FIELD_NOT_VALID			= 456,
		eRTSP_INVALID_RANGE						= 457,
		eRTSP_PARAMETER_IS_READ_ONLY				= 458,
		eRTSP_AGGREGATE_OPERATION_NOT_ALLOWED	= 459,
		eRTSP_ONLY_AGGREGATE_OPERATION_ALLOWED	= 460,
		eRTSP_UNSUPPORTED_TRANSPORT				= 461,
		eRTSP_DESTINATION_UNREACHABLE			= 462,

		eRTSP_INTERNAL_SERVER_ERROR				= 500,
		eRTSP_NOT_IMPLEMENTED					= 501,
		eRTSP_BAD_GATEWAY						= 502,
		eRTSP_SERVICE_UNAVAILABLE				= 503,
		eRTSP_GATEWAY_TIMEOUT					= 504,
		eRTSP_VERSION_NOT_SUPPORTED				= 505,
		eRTSP_OPTION_NOT_SUPPORTED				= 551,

		eRTSP_INVALID_RST                      = -1,
	}EnumResponseStatus;


	

	extern int  g_bRtspCmdPrint;



	



typedef LONG EnumRTSPComandMask;

#define		eRTSP_CMD_NONE  0
#define		eRTSP_CMD_OPTIONS  (1L)
#define		eRTSP_CMD_DESCRIBE  (1L<<1)
#define		eRTSP_CMD_TEARDOWN  (1L<<2)
#define		eRTSP_CMD_SETUP  (1L<<3)
#define		eRTSP_CMD_PLAY  (1L<<4)
#define		eRTSP_CMD_PAUSE  (1L<<5)
#define		eRTSP_CMD_GET_PARAMETER  (1L<<6)
#define		eRTSP_CMD_SET_PARAMETER  (1L<<7)








#define RTSP_VERSION "RTSP/1.0"
#define HTTP_VERSION "HTTP/1.0"
#ifdef _WIN32
#pragma pack( pop )
#endif

#include <GSType.h>


} //end namespace RTSP
} //end namespace GSP

#endif //end _GS_H_RTSPSTRU_H_