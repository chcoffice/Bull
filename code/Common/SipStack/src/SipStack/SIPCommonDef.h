/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : SIPCOMMONDEF.H
Author :  邹阳星
Version : 1.0.0.1
Date: 2012/8/16 14:59
Description: 全局定义
********************************************
*/

#ifndef _GS_H_SIPCOMMONDEF_H_
#define _GS_H_SIPCOMMONDEF_H_

#include <GSType.h>
#include <GSCommon.h> 
#include <map>
#include <string>
#include <eXosip2/eXosip.h>

namespace GSSIP
{



#define MAX_STRING_NUM	256
#define MAX_ADDRESS_NUM	128
#define	STR_PARAM_LEN	64


	//SIP 返回值的错误码
	typedef enum
	{
		eSIP_RESPONSE_OK = 200,
		eSIP_RESPONSE_ERROR = 400,

		eSIP_RESPONSE_BUSY = 486,
		
	}EnumSipResponseCode;


//////////////////////////////////////////////////////////////////////////






#define	SIP_CONTENT_TYPE		"Application/DDCP"
#define SDP_CONTENT_TYPE		"Application/sdp"
#define MANSCDP_XML_CONTENT_TYPE "Application/MANSCDP+xml"
#define MANSRTSP_CONTENT_TYPE    "Application/MANSRTSP"
#define RTSP_CONTENT_TYPE    "Application/RTSP"

#define REGISTER_CODE			"REGISTER"
#define INVITE_CODE				"INVITE"
#define SUBSCRIBE_CODE			"SUBSCRIBE"
#define	NOTIFY_CODE				"NOTIFY"
#define INFO_CODE				"INFO"
//#define INFO_CODE				"DO"
#define MESSAGE_CODE			"MESSAGE"
#define BYE_CODE				"BYE"
#define CANCEL_CODE				"CANCEL"
#define RESPONSE_CODE				"RESPONSE"
#define INFOEX_CODE				"INFO"
#define ACK_CODE				"ACK"

#ifndef IN

#define IN

#endif

#ifndef OUT

#define OUT

#endif

#ifndef IN_OUT

#define IN_OUT

#endif

#if defined(DEBUG) || defined(_DEBUG)
#define SIP_DEBUG_PRINT  printf
#else
#define SIP_DEBUG_PRINT(...)
#endif


#define MAKE_OSIP_ERRNO( x ) ((EnumSipErrorCode)((int)eSIP_RET_OSIP_E_OPER+x))

} //end namespace GSS

#endif //end _GS_H_SIPCOMMONDEF_H_