#include "RTSPProDebug.h"

using namespace GSP;

namespace GSP
{
	namespace RTSP
	{
		typedef  struct _StruGSPCmdDescri
		{
			INT eID;
			const char *czName;
		}StruGSPCmdDescri;


		CGSString RtspCmdPrintString (const char * lpFormat, ...)
		{
			CGSString csStr;
			CGSString strRet;

			//    char *p;
			va_list marker;
			va_start( marker, lpFormat );    
			GSStrUtil::VFormat(csStr,lpFormat, marker);
			va_end( marker );

			if( csStr.empty() )
			{
				return strRet;
			}
			else
			{
				const char *p = csStr.c_str();
				while(p &&  *p!='\0')
				{
					if(*p=='\r' )
					{
						strRet += "\\r";
					}
					else if( *p=='\n')
					{
						strRet += "\\n";
						strRet += "\n";
					}
					else
					{
						strRet += *p;
					}
					p++;
				}
			}
			return strRet;
		}
		#define  INFO_NAME_NODE_VALUE( t, stv ) { (INT)(t), stv }
		const char *RtspGetResponseStatusName( EnumResponseStatus eSt )
		{
			struct _StrucInfoNameNode
			{
				INT iType;
				char *czName;
			};
			static const struct _StrucInfoNameNode _s_stResStatV[] = 
			{

				{-1, "???"},  //          = -1
				INFO_NAME_NODE_VALUE( eRTSP_CONTINUE, "Continue"),  //          = 100

				INFO_NAME_NODE_VALUE(eRTSP_OK, "OK"),  //  									= 200,
				INFO_NAME_NODE_VALUE(eRTSP_CREATED, "Created"),  //  							= 201,
				INFO_NAME_NODE_VALUE(eRTSP_LOW_ON_STORAGE_SPACE, "Low on Storage Space"),  //  				= 250,

				INFO_NAME_NODE_VALUE(eRTSP_MULTIPLE_CHOICES, "Multiple Choices"),  //  					= 300,
				INFO_NAME_NODE_VALUE(eRTSP_MOVED_PERMANENTLY, "Moved Permanently"),  //  					= 301,
				INFO_NAME_NODE_VALUE(eRTSP_MOVED_TEMPORARILY, "Moved Temporarily"),  //  					= 302,
				INFO_NAME_NODE_VALUE(eRTSP_SEE_OTHER	, "See Other"),  //  						= 303,
				INFO_NAME_NODE_VALUE(eRTSP_USE_PROXY, "Use Proxy"),  //  							= 305,

				INFO_NAME_NODE_VALUE(eRTSP_BAD_REQUEST, "Bad Request"),  //  						= 400,
				INFO_NAME_NODE_VALUE(eRTSP_UNAUTHORIZED, "Unauthorized"),  //  						= 401,
				INFO_NAME_NODE_VALUE(eRTSP_PAYMENT_REQUIRED, "Payment Required"),  //  					= 402,
				INFO_NAME_NODE_VALUE(eRTSP_FORBIDDEN, "Forbidden"),  //  							= 403,
				INFO_NAME_NODE_VALUE(eRTSP_NOT_FOUND	, "Not Found"),  //  						= 404,
				INFO_NAME_NODE_VALUE(eRTSP_METHOD_NOT_ALLOWED, "Method Not Allowed"),  //  					= 405,
				INFO_NAME_NODE_VALUE(eRTSP_NOT_ACCEPTABLE, "Not Acceptable"),  //  						= 406,
				INFO_NAME_NODE_VALUE(eRTSP_PROXY_AUTHENTICATION_REQUIRED, "Proxy Authentication Required"),  //  		= 407,
				INFO_NAME_NODE_VALUE(eRTSP_REQUEST_TIMEOUT, "Request Timeout"),  //  					= 408,
				INFO_NAME_NODE_VALUE(eRTSP_GONE	, "Gone"),  //  							= 410,
				INFO_NAME_NODE_VALUE(eRTSP_LENGTH_REQUIRED, "Length Required"),  //  					= 411,
				INFO_NAME_NODE_VALUE(eRTSP_PRECONDITION_FAILED, "Precondition Failed"),  //  				= 412,
				INFO_NAME_NODE_VALUE(eRTSP_REQUEST_ENTITY_TOO_LARGE	, "Request Entity Too Large"),  //  		= 413,
				INFO_NAME_NODE_VALUE(eRTSP_REQUEST_URI_TOO_LONG, "Request-URI Too Long"),  //  				= 414,
				INFO_NAME_NODE_VALUE(eRTSP_UNSUPPORTED_MEDIA_TYPE, "Unsupported Media Type"),  //  				= 415,
				INFO_NAME_NODE_VALUE(eRTSP_INVALID_PARAMETER	, "Invalid Parameter"),  //  				= 451,
				INFO_NAME_NODE_VALUE(eRTSP_ILLEGAL_CONFERENCE_IDENTIFIER	, "Illegal Conference Identifier"),  //  	= 452,
				INFO_NAME_NODE_VALUE(eRTSP_NOT_ENOUGH_BANDWIDTH, "Not Enough Bandwidth"),  //  				= 453,
				INFO_NAME_NODE_VALUE(eRTSP_SESSION_NOT_FOUND	, "Session Not Found"),  //  				= 454,
				INFO_NAME_NODE_VALUE(eRTSP_METHOD_NOT_VALID_IN_THIS_STATE, "Method Not Valid In This State"),  //  		= 455,
				INFO_NAME_NODE_VALUE(eRTSP_HEADER_FIELD_NOT_VALID, "Header Field Not Valid"),  //  				= 456,
				INFO_NAME_NODE_VALUE(eRTSP_INVALID_RANGE	, "Invalid Range"),  //  					= 457,
				INFO_NAME_NODE_VALUE(eRTSP_PARAMETER_IS_READ_ONLY, "Parameter Is Read-Only"),  //  				= 458,
				INFO_NAME_NODE_VALUE(eRTSP_AGGREGATE_OPERATION_NOT_ALLOWED, "Aggregate Oper Not Allowed"),  //  	= 459,
				INFO_NAME_NODE_VALUE(eRTSP_ONLY_AGGREGATE_OPERATION_ALLOWED, "Only Aggregate Oper Allowed"),  //  	= 460,
				INFO_NAME_NODE_VALUE(eRTSP_UNSUPPORTED_TRANSPORT, "Unsupported Transport"),  //  				= 461,
				INFO_NAME_NODE_VALUE(eRTSP_DESTINATION_UNREACHABLE, "Destination Unreachable"),  //  			= 462,

				INFO_NAME_NODE_VALUE(eRTSP_INTERNAL_SERVER_ERROR, "Internal Server Error"),  //  				= 500,
				INFO_NAME_NODE_VALUE(eRTSP_NOT_IMPLEMENTED, "Not Implemented"),  //  					= 501,
				INFO_NAME_NODE_VALUE(eRTSP_BAD_GATEWAY, "Bad Gateway"),  //  						= 502,
				INFO_NAME_NODE_VALUE(eRTSP_SERVICE_UNAVAILABLE, "Service Unavailabe"),  //  				= 503,
				INFO_NAME_NODE_VALUE(eRTSP_GATEWAY_TIMEOUT, "Gateway Timeout"),  //  					= 504,
				INFO_NAME_NODE_VALUE(eRTSP_VERSION_NOT_SUPPORTED	, "RTSP Version Not Supported"),  //  			= 505,
				INFO_NAME_NODE_VALUE(eRTSP_OPTION_NOT_SUPPORTED, "Option Not Supported"),  //  				= 551,


			};

			for( UINT i =1; i<ARRARY_SIZE(_s_stResStatV); i++ )
			{
				if( _s_stResStatV[i].iType == (INT)eSt )
				{
					return _s_stResStatV[i].czName;
				}
			}
			return _s_stResStatV[0].czName;
		}



	const char *RtspGetRtpPlayloadTypeName( EnumRTPPayloadType iPayloadType )
	{
		switch(iPayloadType)
		{
		case eRTP_PT_H264 :
			{
				static char szName[]="H264";
				return szName;
			}
		break;
		case eRTP_PT_JPEG :
		case eRTP_PT_MP4 :
		case eRTP_PT_MJPEG :
		
			{
				static char szName[]="MP4V-ES";
				return szName;
			}
			break;
		case eRTP_PT_GXX :
			{
				static char szName[]="X-GSMedia";
				return szName;
			}
			break;
		default :
			{
				{
					static char szName[]="X-Media";
					return szName;
				}
				break;
			}
		break;
		}

		return NULL;
	}

	} //end namespace RTSP
}
