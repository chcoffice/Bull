/*
******************************************
Copyright (C), 2010-2011, GOSUN
Filename : GSPPRODEBUG.H
Author :  zouyx
Version : 0.1.0.0
Date: 2010/9/15 11:22
Description: gsp 协议的描述打印
********************************************
*/
#ifndef RTSP_GSPPRODEBUG_DEF_H
#define RTSP_GSPPRODEBUG_DEF_H
#include "RTSPStru.h"
#include "../GSPObject.h"

namespace GSP
{
	namespace RTSP
	{





		extern CGSString RtspCmdPrintString( const char * lpFormat, ... );

		const char *RtspGetResponseStatusName( EnumResponseStatus eSt );

		const char *RtspGetCodeIDOfQtTextName( EnumGSCodeID eCodeID );
		const char *RtspGetRtpPlayloadTypeName( EnumRTPPayloadType iPayloadType );


		EnumErrno RtspResponseStatusToLocalErrno( EnumResponseStatus eSt );

#if 1
#define RTSP_DEBUG_STEP
#define RTSP_CMD_DEBUG_PRINTF(szType, szContent) \
	do{ \
		CGSString _strDebug=RtspCmdPrintString(szContent); \
		MY_LOG_DEBUG(g_pLog, "\n**********\n<%s>\n%s\n**********\n", szType, _strDebug.c_str() ); \
	}while(0)
#else
#define RTSP_CMD_DEBUG_PRINTF(...) do{}while(0)
#endif

	} //end namespace RTSP
} //end namespace GSP

#endif
