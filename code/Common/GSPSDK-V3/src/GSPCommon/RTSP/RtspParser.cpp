#include "RtspParser.h"
#include "../crc.h"
#include "../md5.h"
#include "../Log.h"
#include "../GSPMemory.h"
#include "../OSThread.h"
#include "../StrFormater.h"


using namespace GSP;
using namespace GSP::RTSP;




/*
*********************************************************************
*
*@brief : CIRtspLineParser
*
*********************************************************************
*/
EnumRtspHeaderType CIRtspLineParser::GetRtspHeaderType( CGSString &strKey )
{

	if( strKey == "Request")
	{
		return eRTSP_H_TYPE_Request;
	}

	if( strKey == "Response" )
	{
		return eRTSP_H_TYPE_Response;
	}

	if( strKey == "CSeq" )
	{
		return eRTSP_H_TYPE_CSeq;
	}

	if( strKey == "Date" )
	{
		return eRTSP_H_TYPE_Date;
	}

	if( strKey == "Session" )
	{
		return eRTSP_H_TYPE_Session;
	}

	if( strKey == "Content-Type" )
	{
		return eRTSP_H_TYPE_ContenType;
	}



	if( strKey == "Content-Length" )
	{
		return eRTSP_H_TYPE_ContenLength;
	}

	if( strKey == "Range" )
	{
		return eRTSP_H_TYPE_Range;
	}

	if( strKey == "Connection" )
	{
		return eRTSP_H_TYPE_Connection;
	}

	if( strKey == "Keep-Alive" )
	{
		return eRTSP_H_TYPE_KeepAlive;
	}


	if( strKey == "Accept" )
	{
		return eRTSP_H_TYPE_Accept;
	}

	if( strKey == "Server" )
	{
		return eRTSP_H_TYPE_Server;
	}

	if( strKey == "User-Agent" )
	{
		return eRTSP_H_TYPE_UserAgent;
	}

	if( strKey == "RTP-Info" )
	{
		return eRTSP_H_TYPE_RTPInfo;
	}

	if( strKey == "Allow" )
	{
		return eRTSP_H_TYPE_Allow;
	}

	if( strKey == "Transport" )
	{
		return eRTSP_H_TYPE_Transport;
	}

	if( strKey == "Cache-Control" )
	{
		return eRTSP_H_TYPE_CacheControl;
	}
	if( strKey == "Scale")
	{
		return eRTSP_H_TYPE_Scale;
	}
	if( strKey == "Speed")
	{
		return eRTSP_H_TYPE_Speed;
	}
	if( strKey == "PauseTime")
	{
		return eRTSP_H_TYPE_PauseTime;
	}
	return eRTSP_H_TYPE_END;
}

const char *CIRtspLineParser::GetRtspCommandName( EnumRTSPComandMask eMask)
{
	switch( eMask )
	{
	case eRTSP_CMD_OPTIONS :
		{
			static const char szName[] = "OPTIONS";
			return szName;
		}
	break;
	case eRTSP_CMD_DESCRIBE :
		{
			static const char szName[] = "DESCRIBE";
			return szName;
		}
		break;
	case eRTSP_CMD_SETUP :
		{
			static const char szName[] = "SETUP";
			return szName;
		}
		break;
	case eRTSP_CMD_TEARDOWN :
		{
			static const char szName[] = "TEARDOWN";
			return szName;
		}
		break;
	case eRTSP_CMD_PLAY :
		{
			static const char szName[] = "PLAY";
			return szName;
		}
		break;
	case eRTSP_CMD_PAUSE :
		{
			static const char szName[] = "PAUSE";
			return szName;
		}
		break;
	case eRTSP_CMD_GET_PARAMETER :
		{
			static const char szName[] = "GET_PARAMETER";
			return szName;
		}
		break;
	case eRTSP_CMD_SET_PARAMETER :
		{
			static const char szName[] = "SET_PARAMETER";
			return szName;
		}
		break;
	}
	static const char szNone[]="";
	return szNone;
	
	
}

EnumRTSPComandMask CIRtspLineParser::GetRtspCommandMask(const char *szName )
{
	if( !strncasecmp(szName,"OPTIONS", 7) )
	{
		return  eRTSP_CMD_OPTIONS;
	}
	else if( !strncasecmp(szName,"DESCRIBE", 8) )
	{
		return  eRTSP_CMD_DESCRIBE;
	}
	else if( !strncasecmp(szName,"SETUP", 5) )
	{
		return  eRTSP_CMD_SETUP;
	}
	else if( !strncasecmp(szName,"TEARDOWN", 8) )
	{
		return  eRTSP_CMD_TEARDOWN;
	}
	else if( !strncasecmp(szName,"PLAY", 4) )
	{
		return  eRTSP_CMD_PLAY;
	}
	else if( !strncasecmp(szName,"PAUSE", 5) )
	{
		return  eRTSP_CMD_PAUSE;
	}
	else if( !strncasecmp(szName,"GET_PARAMETER", 13) )
	{
		return  eRTSP_CMD_GET_PARAMETER;
	}
	else if( !strncasecmp(szName,"SET_PARAMETER", 13) )
	{
		return  eRTSP_CMD_SET_PARAMETER;
	}
	return eRTSP_CMD_NONE;
}

// CGSString CIRtspLineParser::GetRtspHeaderTypeStr(EnumRtspHeaderType eType )
// {
// 
// }

CIRtspLineParser *CIRtspLineParser::Create(EnumRtspHeaderType eType )
{
	switch( eType )
	{
	case eRTSP_H_TYPE_Request :
		return new CRtspRequestParser();
	break;
	case eRTSP_H_TYPE_Response :
		return new CRtspResponseParser();
		break;
	case eRTSP_H_TYPE_CSeq :
		return new CRtspCSeqParser();
		break;
	case eRTSP_H_TYPE_Date :
		return new CRtspDateParser();
		break;
	case eRTSP_H_TYPE_Session :
		return new CRtspSessionParser();
		break;
	case eRTSP_H_TYPE_ContenType :
		return new CRtspContenTypeParser();
		break;
	case eRTSP_H_TYPE_ContenLength :
		return new CRtspContenLengthParser();
		break;
	case eRTSP_H_TYPE_Range :
		return new CRtspRangeParser();
		break;
	case eRTSP_H_TYPE_Connection :
		return new CRtspConnectionParser();
		break;
	case eRTSP_H_TYPE_KeepAlive :
		return new CRtspKeepAliveParser();
		break;
	case eRTSP_H_TYPE_Accept :
		return new CRtspAcceptParser();
		break;
	case eRTSP_H_TYPE_Server :
		return new CRtspServerParser();
		break;
	case eRTSP_H_TYPE_UserAgent :
		return new CRtspUserAgentParser();
		break;
	case eRTSP_H_TYPE_RTPInfo :
		return new CRtspRTPInfoParser();
		break;
	case eRTSP_H_TYPE_Allow :
		return new CRtspAllowParser();
		break;
	case eRTSP_H_TYPE_Transport :
		return new CRtspTransportParser();
		break;
	case eRTSP_H_TYPE_CacheControl :
		return new CRtspCacheControlParser();
		break;
	case eRTSP_H_TYPE_Scale :
		return new CRtspScaleParser();
		break;
	case eRTSP_H_TYPE_Speed :
		return new CRtspSpeedParser();
	break;

	case eRTSP_H_TYPE_PauseTime :
		return new CRtspPauseTimeParser();
		break;

	default :
	break;

	}
	GS_ASSERT(0);
	return NULL;
}


/*
*********************************************************************
*
*@brief : CRtspRequestParser
*
*********************************************************************
*/
CGSString CRtspRequestParser::Serial(void)
{
	CGSString strRet;
	if( m_strVersion == CGSString(RTSP_VERSION))
	{

		GSStrUtil::Format(strRet, "%s: %s %s\r\n", 
			m_strMethod.c_str(),m_strUrl.c_str(),
			m_strVersion.c_str());
	}
	else
	{
		GSStrUtil::Format(strRet, "%s %s\r\n", 
			m_strMethod.c_str(),
			m_strVersion.c_str());
	}

	return strRet;
}

BOOL CRtspRequestParser::Parser(const char *szLine, INT iLength )
{
	CStrFormater::GetWord(m_strMethod, &szLine);
	CStrFormater::RightTrim(m_strMethod, ":");
	CStrFormater::GetWord(m_strUrl, &szLine);
	CStrFormater::GetWord(m_strVersion, &szLine);

	return !(m_strMethod.empty() && m_strUrl.empty() 
		&& m_strVersion.empty());
}

void CRtspRequestParser::Reset(void)
{
	m_strMethod.clear();
	m_strUrl.clear();
	m_strVersion = RTSP_VERSION;
}

/*
*********************************************************************
*
*@brief :  CRtspResponseParser
*
*********************************************************************
*/
CGSString CRtspResponseParser::Serial(void)
{
	CGSString strRet;
	GSStrUtil::Format(strRet, "%s %d %s\r\n", 					
		m_strVersion.c_str(), m_eResNo,
		m_strError.length() ? m_strError.c_str() : "");
	return strRet;
}

BOOL CRtspResponseParser::Parser(const char *szLine, INT iLength )
{


	CStrFormater::GetWord(m_strVersion, &szLine);
	CStrFormater::GetWord(m_strError, &szLine);
	if( m_strError.empty() )
	{
		Reset();
		return FALSE;
	}
	m_eResNo = eRTSP_INVALID_RST;
	m_eResNo = (EnumResponseStatus) GSStrUtil::ToNumber<INT>(m_strError);
	if( m_eResNo == eRTSP_OK )
	{
		m_strError  = "Success";
	}
	else
	{
		m_strError = szLine;					
	}			

	return (m_eResNo != eRTSP_INVALID_RST);
}

void CRtspResponseParser::Reset(void)
{
	m_eResNo = eRTSP_INVALID_RST;
	m_strError.clear();
	m_strVersion = RTSP_VERSION;
}


/*
*********************************************************************
*
*@brief : CRtspCSeqParser
*
*********************************************************************
*/
CGSString CRtspCSeqParser::Serial(void)
{
	CGSString strRet;
	GSStrUtil::Format(strRet, "%s: %s\r\n", 					
		m_strKey.c_str(), 
		m_strCSeq.length() ? m_strCSeq.c_str() : "");
	return strRet;
}

BOOL CRtspCSeqParser::Parser(const char *szLine, INT iLength )
{
	CGSString strKey;
	CStrFormater::GetWord(strKey, &szLine);
	CStrFormater::RightTrim(strKey, ":");
	CStrFormater::GetWord(m_strCSeq, &szLine);
	if( strKey!=m_strKey )
	{
		return FALSE;
	}
	return TRUE;
}

void CRtspCSeqParser::Reset(void)
{
	m_strCSeq.clear();
}

/*
*********************************************************************
*
*@brief :  CRtspServerParser
*
*********************************************************************
*/

CGSString CRtspServerParser::Serial(void)
{
	CGSString strRet;
	GSStrUtil::Format(strRet, "%s: %s\r\n", 					
		m_strKey.c_str(), 
		m_strServer.length() ? m_strServer.c_str() : "");
	return strRet;
}

BOOL CRtspServerParser::Parser(const char *szLine, INT iLength )
{
	CGSString strKey;
	CStrFormater::GetWord(strKey, &szLine);
	CStrFormater::RightTrim(strKey, ":");
	m_strServer = szLine;

	if( strKey!=m_strKey )
	{
		return FALSE;
	}
	return TRUE;
}

void CRtspServerParser::Reset(void)
{
	m_strServer.clear();
}

/*
*********************************************************************
*
*@brief : CRtspDateParser
*
*********************************************************************
*/

CGSString CRtspDateParser::Serial(void)
{
	CGSString strRet;
	GSStrUtil::Format(strRet, "%s: %s\r\n", 					
		m_strKey.c_str(), 
		m_strGMT.c_str() );
	return strRet;
}

BOOL CRtspDateParser::Parser(const char *szLine, INT iLength )
{
	CGSString strKey;			
	CStrFormater::GetWord(strKey, &szLine);
	CStrFormater::RightTrim(strKey, ":");
	m_strGMT = szLine;
	if( strKey!=m_strKey )
	{
		return FALSE;
	}
	return TRUE;
}

void CRtspDateParser::Reset(void)
{
	COSThread::GetCTimeOfGMT(m_strGMT);
}

static const char *_GetContentTypeName(EnumContentType eType )
{
static char szPkt[] = "application/pkt";
static char szSdp[] = "application/sdp";
static char szHtml[] = "application/html";
static char szBin[] = "application/bin";
static char szPara[] = "application/parameters";
static char szUnknow[] = "application/unknown";
	switch( eType )
	{
	case eCONT_TYPE_PACKET:
		return szPkt;		
	case eCONT_TYPE_SDP:
		return szSdp;
	case eCONT_TYPE_HTML:
		return szHtml;
	case eCONT_TYPE_BIN:
		return szBin;
		break;
	case eCONT_TYPE_PARAMS:
		return szPara;	
	}
	return szUnknow;

}

static EnumContentType _GetContentTypeOfName( const CGSString &strKey )
{
EnumContentType eRet = eCONT_TYPE_INVALID;
	if( strKey.find("/pkt") )
	{
		eRet = eCONT_TYPE_PACKET;
	}
	else if( strKey.find("/sdp") )
	{
		eRet = eCONT_TYPE_SDP;
	}
	else if( strKey.find("/html") )
	{
		eRet = eCONT_TYPE_HTML;
	}
	else if( strKey.find("/bin") )
	{
		eRet = eCONT_TYPE_BIN;
	}
	else if( strKey.find("/parameters") )
	{
		eRet = eCONT_TYPE_PARAMS;
	}
	return eRet;
}

/*
*********************************************************************
*
*@brief : CRtspAcceptParser
*
*********************************************************************
*/
CGSString CRtspAcceptParser::Serial(void)
{
	CGSString strRet;
	GSStrUtil::Format(strRet, "%s: %s\r\n", 					
		m_strKey.c_str(), 
		_GetContentTypeName(m_eContentType) );
	return strRet;
}

BOOL  CRtspAcceptParser::Parser(const char *szLine, INT iLength )
{
	CGSString strKey;
	CStrFormater::GetWord(strKey, &szLine);
	CStrFormater::RightTrim(strKey, ":");
	if( strKey!=m_strKey )
	{
		return FALSE;
	}
	strKey = szLine;
	m_eContentType = _GetContentTypeOfName(strKey);	
	return (m_eContentType != eCONT_TYPE_INVALID);
}

void  CRtspAcceptParser::Reset(void)
{
	m_eContentType = eCONT_TYPE_INVALID;
}



/*
*********************************************************************
*
*@brief : CRtspContenTypeParser
*
*********************************************************************
*/

CGSString CRtspContenTypeParser::Serial(void)
{
	CGSString strRet;
	GSStrUtil::Format(strRet, "%s: %s\r\n", 					
		m_strKey.c_str(), 
		_GetContentTypeName(m_eContentType) );
	return strRet;
}

BOOL  CRtspContenTypeParser::Parser(const char *szLine, INT iLength )
{
	CGSString strKey;
	CStrFormater::GetWord(strKey, &szLine);
	CStrFormater::RightTrim(strKey, ":");
	if( strKey!=m_strKey )
	{
		return FALSE;
	}
	strKey = szLine;
	m_eContentType = _GetContentTypeOfName(strKey);	
	return (m_eContentType != eCONT_TYPE_INVALID);
}

void  CRtspContenTypeParser::Reset(void)
{
	m_eContentType = eCONT_TYPE_INVALID;
}




/*
*********************************************************************
*
*@brief : CRtspContenLengthParser
*
*********************************************************************
*/

CGSString CRtspContenLengthParser::Serial(void)
{
	CGSString strRet;
	GSStrUtil::Format(strRet, "%s: %d\r\n", 					
		m_strKey.c_str(), 
		(int)m_iLength );
	return strRet;
}

BOOL CRtspContenLengthParser::Parser(const char *szLine, INT iLength )
{
	CGSString strKey;	
	CGSString strValue;
	CStrFormater::GetWord(strKey, &szLine);
	CStrFormater::RightTrim(strKey, ":");
	strValue = szLine;
	if( strKey!=m_strKey || strValue.length()<1 )
	{
		return FALSE;
	}
	m_iLength = GSStrUtil::ToNumber<int>(strValue);
	return TRUE;
}

void CRtspContenLengthParser::Reset(void)
{
	m_iLength = 0;
}

/*
*********************************************************************
*
*@brief : CRtspRangeLengthParser
*
*********************************************************************
*/

CGSString CRtspRangeParser::Serial(void)
{
	CGSString strRet;
	if( m_fEnd > 0.0 )
	{
// 		GSStrUtil::Format(strRet, "%s: npt=%lf-%lf\r\n", 				
// 			m_strKey.c_str(), m_fBegin, m_fEnd);	
		GSStrUtil::Format(strRet, "%s: npt=%d-%d\r\n", 				
			 			m_strKey.c_str(), (int)m_fBegin, (int)m_fEnd);	
	}
	else if( m_fBegin>0.0 )
	{
// 		GSStrUtil::Format(strRet, "%s: npt=%ld-\r\n", 				
// 			m_strKey.c_str(), m_fBegin);	
		GSStrUtil::Format(strRet, "%s: npt=%d-\r\n", 				
			m_strKey.c_str(), (int)m_fBegin);	
	}
	else
	{
// 		GSStrUtil::Format(strRet, "%s: npt=now-\r\n", 				
// 			m_strKey.c_str(), m_fBegin);	

		GSStrUtil::Format(strRet, "%s: npt=0-\r\n", 				
			m_strKey.c_str());	
	}
	return strRet;
}

BOOL CRtspRangeParser::Parser(const char *szLine, INT iLength )
{
	CGSString strKey;	
	CGSString strValue;
	CStrFormater::GetWord(strKey, &szLine);
	CStrFormater::RightTrim(strKey, ":");
	strValue = szLine;
	if( strKey!=m_strKey || strValue.length()<1 )
	{
		return FALSE;
	}
	const char *p,*pValue;
	pValue = strValue.c_str();
	CStrFormater::SkipSpaces(&pValue);
	if( CStrFormater::StrIsStart(pValue,"npt=", &p) )
	{
		//npt
		CStrFormater::SkipSpaces(&p);
		strKey = "0.00";
		CStrFormater::GetWordSep(strKey, "-",&p);

		if( strKey!="now")
		{
			m_fBegin = GSStrUtil::ToNumber<double>(strKey);		
		}
		if( '-'== *p )
		{
			p++;
			CStrFormater::SkipSpaces(&pValue);
			strKey=p;
			if( strKey.length()>1 )
			{
				m_fEnd = m_fBegin = GSStrUtil::ToNumber<double>(strKey);
			}
		}

	}
	else if( CStrFormater::StrIsStart(pValue,"clock=", &p) )
	{

	}
	else if( CStrFormater::StrIsStart(pValue,"smtpe=", &p) )
	{

	} else {
		return FALSE; // The header is malformed
	}

	return TRUE;
}

void CRtspRangeParser::Reset(void)
{
	m_fBegin = 0.0;
	m_fEnd = 0.0;
}

/*
*********************************************************************
*
*@brief : CRtspPauseTimeParser
*
*********************************************************************
*/

CGSString CRtspPauseTimeParser::Serial(void)
{
	CGSString strRet;
	if( m_fPauseTime > 0.0 )
	{
		
		GSStrUtil::Format(strRet, "%s: %.2f\r\n", 				
			m_strKey.c_str(), m_fPauseTime );	
	}	
	else
	{
		GSStrUtil::Format(strRet, "%s: -\r\n", 				
			m_strKey.c_str());	
	}
	return strRet;
}

BOOL CRtspPauseTimeParser::Parser(const char *szLine, INT iLength )
{
	CGSString strKey;	
	CGSString strValue;
	CStrFormater::GetWord(strKey, &szLine);
	CStrFormater::RightTrim(strKey, ":");
	strValue = szLine;
	if( strKey!=m_strKey || strValue.length()<1 )
	{
		return FALSE;
	}
	const char *pValue;
	pValue = strValue.c_str();
	CStrFormater::SkipSpaces(&pValue);	
	strKey.clear();
	CStrFormater::GetWord(strKey,&pValue);
	if(strKey.empty() || strKey[0] == '-' )
	{
		//npt
		m_fPauseTime  = 0.0;
	}
	else 
	{
		m_fPauseTime = GSStrUtil::ToNumber<double>(strKey);

	} 
	return TRUE;
}

void CRtspPauseTimeParser::Reset(void)
{
	m_fPauseTime = 0.0;	
}



/*
*********************************************************************
*
*@brief : CRtspConnectionParser
*
*********************************************************************
*/

CGSString CRtspConnectionParser::Serial(void)
{
	CGSString strRet;
	GSStrUtil::Format(strRet, "%s: %s\r\n", 					
		m_strKey.c_str(), 
		m_strValue.c_str() );
	return strRet;
}

BOOL CRtspConnectionParser::Parser(const char *szLine, INT iLength )
{
	CGSString strKey;			
	CStrFormater::GetWord(strKey, &szLine);
	CStrFormater::RightTrim(strKey, ":");
	m_strValue = szLine;
	if( strKey!=m_strKey )
	{
		return FALSE;
	}
	return TRUE;
}

void CRtspConnectionParser::Reset(void)
{
	m_strValue ="Close";
}


/*
*********************************************************************
*
*@brief : CRtspSessionParser
*
*********************************************************************
*/

CGSString CRtspSessionParser::Serial(void)
{
	CGSString strRet;
	if(m_iTimeouts==-1)
	{

		GSStrUtil::Format(strRet, "%s: %s\r\n", 					
			m_strKey.c_str(), m_strSession.c_str());
	}
	else
	{
		GSStrUtil::Format(strRet, "%s: %s; timeout=%d\r\n", 				
			m_strKey.c_str(),m_strSession.c_str(), m_iTimeouts );
	}
	return strRet;
}

BOOL CRtspSessionParser::Parser(const char *szLine, INT iLength )
{
	m_strSession.clear();
	CGSString strKey;			
	CStrFormater::GetWord(strKey, &szLine);
	CStrFormater::RightTrim(strKey, ":");
	CGSString strValue = szLine;
	if( strKey!=m_strKey || strValue.length()<1 )
	{
		return FALSE;
	}
	CStrFormater::GetWordSep(m_strSession,  ";", &szLine);
	GSStrUtil::Trim(m_strSession);				
	if( CStrFormater::StrIsStart(szLine, ";timeout=", &szLine) )
	{
		strValue = szLine;
		m_iTimeouts  = GSStrUtil::ToNumber<int>(strValue);
	} 
	return (m_strSession.length()>0 );
}

void CRtspSessionParser::Reset(void)
{
	m_iTimeouts =  -1;
	m_strSession.clear();
}


/*
*********************************************************************
*
*@brief : CRtspKeepAliveParser
*
*********************************************************************
*/

CGSString CRtspKeepAliveParser::Serial(void)
{
	CGSString strRet;
	GSStrUtil::Format(strRet, "%s: %s\r\n", 					
		m_strKey.c_str(), 
		m_strValue.c_str() );
	return strRet;
}
BOOL CRtspKeepAliveParser::Parser(const char *szLine, INT iLength )
{
	CGSString strKey;			
	CStrFormater::GetWord(strKey, &szLine);
	CStrFormater::RightTrim(strKey, ":");
	m_strValue = szLine;
	if( strKey!=m_strKey )
	{
		return FALSE;
	}
	return TRUE;
}

void CRtspKeepAliveParser::Reset(void)
{
	m_strValue ="1";
}


/*
*********************************************************************
*
*@brief : CRtspUserAgentParser
*
*********************************************************************
*/

CGSString CRtspUserAgentParser::Serial(void)
{
	CGSString strRet;
	GSStrUtil::Format(strRet, "%s: %s\r\n", 					
		m_strKey.c_str(), 
		m_strAgent.c_str() );
	return strRet;
}
BOOL CRtspUserAgentParser::Parser(const char *szLine, INT iLength )
{
	CGSString strKey;			
	CStrFormater::GetWord(strKey, &szLine);
	CStrFormater::RightTrim(strKey, ":");
	m_strAgent = szLine;
	if( strKey!=m_strKey )
	{
		return FALSE;
	}
	return TRUE;
}

void CRtspUserAgentParser::Reset(void)
{
	m_strAgent =RTSP_GXX_USERAGENT;
}


/*
*********************************************************************
*
*@brief : CRtspRTPInfoParser
*
*********************************************************************
*/
CGSString CRtspRTPInfoParser::Serial(void)
{
	CGSString strRet;
	GSStrUtil::Format(strRet, "%s: url=%s; seq=%d; rtptime=%d\r\n",
		m_strKey.c_str(), m_strUrl.c_str(),
		m_iRtpSeq, m_iRtpTime);
	return strRet;
}
BOOL CRtspRTPInfoParser::Parser(const char *szLine, INT iLength )
{
	Reset();
	CGSString strKey;			
	CStrFormater::GetWord(strKey, &szLine);
	CStrFormater::RightTrim(strKey, ":");
	CGSString strValue = szLine;
	if( strKey!=m_strKey || strValue.length()<1 )
	{
		return FALSE;
	}
	std::vector<CGSString> vSlipt;
	GSStrUtil::Split(vSlipt, strValue, ";"); //·Ö¸î
	for( UINT i = 0; i<vSlipt.size(); i++ )
	{
		const char *p = vSlipt[i].c_str();
		CStrFormater::GetWordSep(strKey,"=", &p );
		strValue = p;
		strKey = GSStrUtil::Trim(strKey);
		if( strKey == "url" )
		{
			m_strUrl = strValue;
		}
		else if( strKey == "seq" )
		{
			m_iRtpSeq = GSStrUtil::ToNumber<int>(strValue);
		}
		else if( strKey == "rtptime" )
		{
			m_iRtpTime = GSStrUtil::ToNumber<int>(strValue);
		}
		else
		{
			GS_ASSERT(0);
		}					
	}
	return TRUE;
}

void CRtspRTPInfoParser::Reset(void)
{
	m_strUrl = "";	
	m_iRtpSeq = -1;
	m_iRtpTime = -1;
}


/*
*********************************************************************
*
*@brief : CRtspCacheControlParser
*
*********************************************************************
*/

CGSString CRtspCacheControlParser::Serial(void)
{
	CGSString strRet;
	GSStrUtil::Format(strRet, "%s: %s\r\n", 					
		m_strKey.c_str(), 
		m_strType.c_str() );
	return strRet;
}

BOOL CRtspCacheControlParser::Parser(const char *szLine, INT iLength )
{
	CGSString strKey;			
	CStrFormater::GetWord(strKey, &szLine);
	CStrFormater::RightTrim(strKey, ":");
	m_strType = szLine;
	if( strKey!=m_strKey )
	{
		return FALSE;
	}
	return TRUE;
}

void CRtspCacheControlParser::Reset(void)
{
	m_strType ="no-cache";
}


/*
*********************************************************************
*
*@brief : CRtspPublicParser
*
*********************************************************************
*/
CGSString CRtspAllowParser::Serial(void)
{
	CGSString strRet;
	strRet = m_strKey; 
	strRet += ": ";
	BOOL bNFirst = FALSE;
	if( m_eAllowMask&eRTSP_CMD_OPTIONS )
	{
		if( bNFirst )
		{
			strRet += ",";
		}
		else
		{
			bNFirst = TRUE;
		}
		strRet += "OPTIONS";
	}
	if( m_eAllowMask&eRTSP_CMD_DESCRIBE )
	{
		if( bNFirst )
		{
			strRet += ",";
		}
		else
		{
			bNFirst = TRUE;
		}
		strRet += "DESCRIBE";
	}
	if( m_eAllowMask&eRTSP_CMD_SETUP )
	{
		if( bNFirst )
		{
			strRet += ",";
		}
		else
		{
			bNFirst = TRUE;
		}
		strRet += "SETUP";
	}

	if( m_eAllowMask&eRTSP_CMD_TEARDOWN )
	{
		if( bNFirst )
		{
			strRet += ",";
		}
		else
		{
			bNFirst = TRUE;
		}
		strRet += "TEARDOWN";
	}

	if( m_eAllowMask&eRTSP_CMD_PLAY )
	{
		if( bNFirst )
		{
			strRet += ",";
		}
		else
		{
			bNFirst = TRUE;
		}
		strRet += "PLAY";
	}

	if( m_eAllowMask&eRTSP_CMD_PAUSE )
	{
		if( bNFirst )
		{
			strRet += ",";
		}
		else
		{
			bNFirst = TRUE;
		}
		strRet += "PAUSE";
	}


	if( m_eAllowMask&eRTSP_CMD_GET_PARAMETER )
	{
		if( bNFirst )
		{
			strRet += ",";
		}
		else
		{
			bNFirst = TRUE;
		}
		strRet += "GET_PARAMETER";
	}

	if( m_eAllowMask&eRTSP_CMD_SET_PARAMETER )
	{
		if( bNFirst )
		{
			strRet += ",";
		}
		else
		{
			bNFirst = TRUE;
		}
		strRet += "SET_PARAMETER";
	}
	strRet += "\r\n";
	return strRet;
}

BOOL CRtspAllowParser::Parser(const char *szLine, INT iLength )
{
	m_eAllowMask = eRTSP_CMD_NONE;
	CGSString strKey;			
	CStrFormater::GetWord(strKey, &szLine);
	CStrFormater::RightTrim(strKey, ":");
	CGSString strValue = szLine;
	if( strKey!=m_strKey )
	{
		return FALSE;
	}

	std::vector<CGSString> vSlipt;
	GSStrUtil::Split(vSlipt, strValue, ","); //·Ö¸î
	for( UINT i = 0; i<vSlipt.size(); i++ )
	{
		// , , , , , , , 
		strValue = GSStrUtil::Trim(vSlipt[i]);
		m_eAllowMask |= GetRtspCommandMask(strValue.c_str());
	}
	GS_ASSERT(m_eAllowMask);
	return TRUE;
}

void CRtspAllowParser::Reset(void)
{
	m_eAllowMask = eRTSP_CMD_NONE;
}


/*
*********************************************************************
*
*@brief : CRtspTransportParser
*
*********************************************************************
*/

CGSString CRtspTransportParser::Serial(void)
{
	CGSString strRet;
	strRet = m_strKey;
	strRet += ": ";
	for( UINT i = 0; i<m_vFields.size(); i++ )
	{
		if( i!=0 )
		{
			strRet += ";";
		}
		switch( m_vFields[i].eTrType)
		{
		case eTRANSPORT_RTP_UDP :
			{
				if( m_vFields[i].bMulticast )
				{
					strRet += "RTP/AVP;multicast";	 /*RTP/AVP/UDP	*/				
				}
				else
				{
					strRet += "RTP/AVP;unicast";	
				}
				if( m_vFields[i].stCliPort.vPort[RTP_PORT_IDX])
				{
					GSStrUtil::AppendWithFormat(strRet,";client_port=%hu",
						m_vFields[i].stCliPort.vPort[RTP_PORT_IDX]);
					if( m_vFields[i].stCliPort.vPort[RTCP_PORT_IDX])
					{
						GSStrUtil::AppendWithFormat(strRet,"-%hu",
							m_vFields[i].stCliPort.vPort[RTCP_PORT_IDX]);
					}									
				}
				if( m_vFields[i].stSrvPort.vPort[RTP_PORT_IDX])
				{
					GSStrUtil::AppendWithFormat(strRet,";server_port=%hu",
						m_vFields[i].stSrvPort.vPort[RTP_PORT_IDX]);
					if( m_vFields[i].stSrvPort.vPort[RTCP_PORT_IDX])
					{
						GSStrUtil::AppendWithFormat(strRet,"-%hu",
							m_vFields[i].stSrvPort.vPort[RTCP_PORT_IDX]);
					}									
				}							
			}
			break;
		case eTRANSPORT_RTP_TCP :
			{
				GSStrUtil::AppendWithFormat(strRet,
					"RTP/AVP/TCP;interleaved=%d-%d",
					m_vFields[i].iInterleavedOfRtpChnID,
					m_vFields[i].iInterleavedOfRtcpChnID);					
			}
			break;
		case eTRANSPORT_RAW_UDP :
			{
				strRet += "RAW/RAW/UDP";
				if( m_vFields[i].stCliPort.vPort[RTP_PORT_IDX])
				{
					GSStrUtil::AppendWithFormat(strRet,
						";client_port=%hu",
						m_vFields[i].stCliPort.vPort[RTP_PORT_IDX]);																
				}
				if( m_vFields[i].stSrvPort.vPort[RTP_PORT_IDX])
				{
					GSStrUtil::AppendWithFormat(strRet,
						";server_port=%hu",
						m_vFields[i].stSrvPort.vPort[RTP_PORT_IDX]);															
				}			
			}
			break;
		default :
			break;
		} //end switch
		if( m_vFields[i].strSrvAddr.length()>0 )
		{
			GSStrUtil::AppendWithFormat(strRet,
				";source=%s",m_vFields[i].strSrvAddr.c_str());		
		}
		if( m_vFields[i].strDestAddr.length()>0 )
		{
			GSStrUtil::AppendWithFormat(strRet,
				";destination=%s",m_vFields[i].strDestAddr.c_str());		
		}
		if( m_vFields[i].iDestTTL != -1 )
		{
			GSStrUtil::AppendWithFormat(strRet,
				";ttl=%ld",m_vFields[i].iDestTTL);	
		}
	}
	strRet += "\r\n";
	return strRet;
}

BOOL CRtspTransportParser::Parser(const char *szLine, INT iLength )
{
	Reset();
	char *pFiled = strdup(szLine);
	INT iIdx = -1;
	UINT ttl,rtpCid, rtcpCid;
	UINT16 p1, p2;
	CGSString strKey;			
	CStrFormater::GetWord(strKey, &szLine);
	CStrFormater::RightTrim(strKey, ":");	
	const char *pTemp = NULL, *pValue;
	GS_ASSERT_RET_VAL(pFiled, FALSE);
	if( strKey!=m_strKey )
	{
		goto fail_ret;
	}
	
	while (sscanf(szLine, "%[^;]", pFiled) == 1) 
	{
		pValue = pFiled;
		CStrFormater::SkipSpaces(&pValue);

		if( CStrFormater::StrIsStart(pValue, "RTP/AVP", &pTemp) )
		{
			iIdx = m_vFields.size();
			m_vFields.push_back(StruTransPortField());
			//RTP
			if( strstr(pValue, "TCP") )
			{
				m_vFields[iIdx].eTrType = eTRANSPORT_RTP_TCP;
			}
			else
			{
				m_vFields[iIdx].eTrType = eTRANSPORT_RTP_UDP;
			}
		}		
		else if (strcmp(pValue, "RAW/RAW/UDP") == 0 ||
			strcmp(pValue, "MP2T/H2221/UDP") == 0) 
		{
			iIdx = m_vFields.size();
			m_vFields.push_back(StruTransPortField());			
			m_vFields[iIdx].eTrType = eTRANSPORT_RAW_UDP;
		} 
		else if( iIdx<0 )
		{
			GS_ASSERT(0);
			goto fail_ret;
		} 
		else if (strncasecmp(pValue, "destination=", 12) == 0) 
		{
			m_vFields[iIdx].strDestAddr = (pValue+12);
		} 
		else if (strncasecmp(pValue, "source=", 7) == 0) 
		{

			m_vFields[iIdx].strSrvAddr = (pValue+7);
		} 
		else if( strcmp(pValue, "multicast")  == 0)
		{
			if( iIdx<0 )
			{
				GS_ASSERT(0);
				goto fail_ret;
			}
			m_vFields[iIdx].bMulticast = TRUE;
		}
		else if (sscanf(pValue, "ttl%u", &ttl) == 1) 
		{

			m_vFields[iIdx].iDestTTL = ttl;

		} 
		else if (sscanf(pValue, "client_port=%hu-%hu", &p1, &p2) == 2) 
		{

			m_vFields[iIdx].stCliPort.vPort[RTP_PORT_IDX] = p1;

			if( m_vFields[iIdx].eTrType == eTRANSPORT_RAW_UDP )
			{
				m_vFields[iIdx].stCliPort.vPort[RTCP_PORT_IDX] = 0;
			}
			else
			{
				m_vFields[iIdx].stCliPort.vPort[RTCP_PORT_IDX] = p2;
			}

		} 
		else if(sscanf(pValue, "client_port=%hu", &p1) == 1) 
		{

			m_vFields[iIdx].stCliPort.vPort[RTP_PORT_IDX] = p1;
			if( m_vFields[iIdx].eTrType == eTRANSPORT_RAW_UDP )
			{
				m_vFields[iIdx].stCliPort.vPort[RTCP_PORT_IDX] = 0;
			}
			else
			{
				m_vFields[iIdx].stCliPort.vPort[RTCP_PORT_IDX] = p1+1;
			}
		} 
		else if (sscanf(pValue, "server_port=%hu-%hu", &p1, &p2) == 2) 
		{

			m_vFields[iIdx].stSrvPort.vPort[RTP_PORT_IDX] = p1;

			if( m_vFields[iIdx].eTrType == eTRANSPORT_RAW_UDP )
			{
				m_vFields[iIdx].stSrvPort.vPort[RTCP_PORT_IDX] = 0;
			}
			else
			{
				m_vFields[iIdx].stSrvPort.vPort[RTCP_PORT_IDX] = p2;
			}

		} 
		else if(sscanf(pValue, "server_port=%hu", &p1) == 1) 
		{

			m_vFields[iIdx].stSrvPort.vPort[RTP_PORT_IDX] = p1;
			if( m_vFields[iIdx].eTrType == eTRANSPORT_RAW_UDP )
			{
				m_vFields[iIdx].stSrvPort.vPort[RTCP_PORT_IDX] = 0;
			}
			else
			{
				m_vFields[iIdx].stSrvPort.vPort[RTCP_PORT_IDX] = p1+1;
			}
		} 

		else if (sscanf(pValue, "interleaved=%u-%u", &rtpCid, &rtcpCid) == 2)
		{			
			m_vFields[iIdx].iInterleavedOfRtpChnID = rtpCid;
			m_vFields[iIdx].iInterleavedOfRtcpChnID = rtcpCid;
		}


		szLine += strlen(pFiled);
		CStrFormater::SkipSpaces(&szLine);
		while (*szLine == ';') ++szLine; // skip over separating ';' chars
		CStrFormater::SkipSpaces(&szLine);
		if (*szLine == '\0' || *szLine == '\r' || *szLine == '\n') break;
	}
	::free(pFiled);
	return  (m_vFields.size()>0);
fail_ret :
	::free(pFiled);	
	return FALSE;
}

void CRtspTransportParser::Reset(void)
{
	m_vFields.clear();
}

/*
*********************************************************************
*
*@brief : CRtspRangeParser
*
*********************************************************************
*/

CGSString CRtspScaleParser::Serial(void)
{
	CGSString strRet;
	GSStrUtil::Format(strRet, "%s: %.2f\r\n", 				
			m_strKey.c_str(), m_iScale);	
	
	return strRet;
}

BOOL CRtspScaleParser::Parser(const char *szLine, INT iLength )
{
	CGSString strKey;	
	CGSString strValue;
	CStrFormater::GetWord(strKey, &szLine);
	CStrFormater::RightTrim(strKey, ":");
	strValue = szLine;
	if( strKey!=m_strKey || strValue.length()<1 )
	{
		return FALSE;
	}
	m_iScale = GSStrUtil::ToNumber<float>(strValue);
	return TRUE;
}

void CRtspScaleParser::Reset(void)
{
	m_iScale = 0;
}

/*
*********************************************************************
*
*@brief : CRtspSpeedParser
*
*********************************************************************
*/

CGSString CRtspSpeedParser::Serial(void)
{
	CGSString strRet;
	GSStrUtil::Format(strRet, "%s: %.2f\r\n", 				
		m_strKey.c_str(), m_iSpeed);	

	return strRet;
}

BOOL CRtspSpeedParser::Parser(const char *szLine, INT iLength )
{
	CGSString strKey;	
	CGSString strValue;
	CStrFormater::GetWord(strKey, &szLine);
	CStrFormater::RightTrim(strKey, ":");
	strValue = szLine;
	if( strKey!=m_strKey || strValue.length()<1 )
	{
		return FALSE;
	}
	m_iSpeed = GSStrUtil::ToNumber<float>(strValue);
	return TRUE;
}

void CRtspSpeedParser::Reset(void)
{
	m_iSpeed = 0;
}

/*
*********************************************************************
*
*@brief : CRtspHeader
*
*********************************************************************
*/

CRtspHeader::CRtspHeader(void)
:CGSPObject()
{
	for( INT i = 0; i<eRTSP_H_TYPE_END; i++ )
	{
		m_vParser[i] = NULL;
	}

}

CRtspHeader::CRtspHeader(const CRtspHeader &csDest)
:CGSPObject()
{
	*this = csDest;
}


 CRtspHeader::~CRtspHeader(void)
 {
	 for( INT i = 0; i<eRTSP_H_TYPE_END; i++ )
	 {
		 if( m_vParser[i] )
		 {			
			 m_vParser[i] = NULL;
		 }
	 }
 }

 CRtspHeader&  CRtspHeader::operator=(const CRtspHeader &csDest)
 {
	if( this != &csDest )
	{
		 m_csRequest = csDest.m_csRequest;
		 m_csResponse = csDest.m_csResponse;
		 m_csCSeq = csDest.m_csCSeq;
		 m_csDate = csDest.m_csDate;
		 m_csSession = csDest.m_csSession;
		 m_csContentType = csDest.m_csContentType;
		 m_csContenLength = csDest.m_csContenLength;
		 m_csRange = csDest.m_csRange;
		 m_csConnect = csDest.m_csConnect;
		 m_csKeepAlive = csDest.m_csKeepAlive;
		 m_csAccept = csDest.m_csAccept;
		 m_csServer = csDest.m_csServer;
		 m_csUserAgent = csDest.m_csUserAgent;
		 m_csRtpInfo = csDest.m_csRtpInfo;
		 m_csAllow = csDest.m_csAllow;
		 m_csTransPort = csDest.m_csTransPort;
		 m_csCacheControl = csDest.m_csCacheControl;
		 m_csScale = csDest.m_csScale;
		 m_csSpeed = csDest.m_csSpeed;
		 m_csPauseTime = csDest.m_csPauseTime;
		 m_csUserAgent = csDest.m_csUserAgent;

		for( INT i = 0; i<eRTSP_H_TYPE_END; i++ )
		{
			if( csDest.m_vParser[i] )
			{
				m_vParser[i] = LineParser((EnumRtspHeaderType)i);
			}
		}
	}	
	return *this;
 }

 CIRtspLineParser *CRtspHeader::LineParser(EnumRtspHeaderType eType)
 {
	 GS_ASSERT_RET_VAL( (UINT)eType<(UINT) eRTSP_H_TYPE_END, NULL);

	 if( m_vParser[(INT)eType ])
	 {
		 return m_vParser[(INT)eType];
	 }
 CIRtspLineParser *p = NULL;
	 switch( eType )
	 {
	 case eRTSP_H_TYPE_Request :
		 p = &m_csRequest;
		 break;
	 case eRTSP_H_TYPE_Response :
		 p = &m_csResponse;
		 break;
	 case eRTSP_H_TYPE_CSeq :
		 p = &m_csCSeq;
		 break;
	 case eRTSP_H_TYPE_Date :
		 p = &m_csDate;
		 break;
	 case eRTSP_H_TYPE_Session :
		 p = &m_csSession;
		 break;
	 case eRTSP_H_TYPE_ContenType :
		 p = &m_csContentType;
		 break;
	 case eRTSP_H_TYPE_ContenLength :
		 p = &m_csContenLength;
		 break;
	 case eRTSP_H_TYPE_Range :
		 p = &m_csRange;
		 break;
	 case eRTSP_H_TYPE_Connection :
		 p = &m_csConnect;
		 break;
	 case eRTSP_H_TYPE_KeepAlive :
		 p = &m_csKeepAlive;
		 break;
	 case eRTSP_H_TYPE_Accept :
		 p = &m_csAccept;;
		 break;
	 case eRTSP_H_TYPE_Server :
		 p = &m_csServer;
		 break;
	 case eRTSP_H_TYPE_UserAgent :
		 p = &m_csUserAgent;
		 break;
	 case eRTSP_H_TYPE_RTPInfo :
		 p = &m_csRtpInfo;
		 break;
	 case eRTSP_H_TYPE_Allow :
		 p = &m_csAllow;
		 break;
	 case eRTSP_H_TYPE_Transport :
		 p = &m_csTransPort;
		 break;
	 case eRTSP_H_TYPE_CacheControl :
		 p = &m_csCacheControl;
		 break;
	 case eRTSP_H_TYPE_Scale :
		 {
			 p = &m_csScale;
		 }
	 break;
	 case eRTSP_H_TYPE_Speed :
		 {
			 p = &m_csSpeed;
		 }
		 break;
	 case eRTSP_H_TYPE_PauseTime :
		 {
			 p = &m_csPauseTime;
		 }
		 break;
	 default :
		 break;

	 }
	 GS_ASSERT_RET_VAL(p, NULL);
	 m_vParser[(INT)eType] = p;
	 return p;
 }

 CGSString CRtspHeader::Serial(void)
 {
CGSString strRet("");

	for( INT i = 0; i<eRTSP_H_TYPE_END; i++ )
	{
		if( m_vParser[i] )
		{
			strRet += m_vParser[i]->Serial();
		}
	}
	if( strRet.length()<2)
	{
		GS_ASSERT(0);
		strRet += "\r\n";
	}

	strRet += "\r\n";
	return strRet;
 }