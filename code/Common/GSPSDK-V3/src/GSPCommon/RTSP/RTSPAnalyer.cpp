#include "RTSPAnalyer.h"
#include "../crc.h"
#include "../md5.h"
#include "../Log.h"
#include "../GSPMemory.h"
#include "../ISocket.h"
#include "RTSPProDebug.h"
#include "../StrFormater.h"

using namespace GSP;
using namespace GSP::RTSP;




/*
*********************************************************************
*
*@brief : CRtspProPacket 实现
*
*********************************************************************
*/


CRtspProPacket::CRtspProPacket(void)
:CProPacket()
{
	m_bRequest = TRUE;
	m_bPacketed = FALSE;
	m_eContentType = eCONT_TYPE_INVALID;
	m_iContentLenght = 0;
	m_pBinContent = NULL;

	m_eParserStatus = ePARSER_STATUS_BEGIN;
	m_czLineCache[0] =  '\0';
}





CRtspProPacket::~CRtspProPacket(void)
{
	if( m_pBinContent )
	{
		CMemoryPool::Free(m_pBinContent);
		m_pBinContent = NULL;
	}
}

EnumErrno CRtspProPacket::Analyse( const BYTE **ppData, UINT &iSize )
{
	if( m_eParserStatus == ePARSER_STATUS_END )
	{
		//已经完成
		return eERRNO_SUCCESS;
	}
	if( m_eParserStatus == ePARSER_STATUS_BEGIN ||
		m_eParserStatus == ePARSER_STATUS_HEADER )
	{
		//解析头
		return ParserHeader(ppData, iSize);
	}
	if( m_eParserStatus == ePARSER_STATUS_CONTENT )
	{
		//接受内容
		return ParserContent(ppData, iSize);
	}
	GS_ASSERT(0);
	return eERRNO_SYS_ESTATUS;	
}

// 1 Request
// 0 Respone
// -1 错误
INT KeyTestType(const CGSString &strKey)
{
	if( strKey.empty() )
	{
		return -1;
	}
	if( std::string::npos!=strKey.find("/") )
	{
		//回复
		return 1;
	}
	return 0;
	
}

EnumErrno CRtspProPacket::ParserHeader(const BYTE **pInput, UINT &iSize)
{
const BYTE *pp = *pInput;
char *pW;

CGSString strKey, strValue;
CIRtspLineParser *pLineParser;
	if( m_iWLinePos>MAX_RTSP_PROTOCOL_LINE_LEN)
	{
		GS_ASSERT(0);
		return eERRNO_SYS_EFLOWOUT;
	}

	pW = &m_czLineCache[m_iWLinePos];
	while( iSize>0 && *pp!='\0' )
	{
		*pW++ = *pp++;
		iSize --;
		m_iWLinePos ++;		
		if( *pW == '\n' )
		{
			//一行结束			
			pW[1] = '\0';
			break;
		}
	}
	*pInput = pp;
const char *p = m_czLineCache;

	CStrFormater::SkipSpaces(&p);

	if( m_eParserStatus == ePARSER_STATUS_BEGIN )
	{
		//首行
		CStrFormater::GetWord(strKey, &p);
		CStrFormater::RightTrim(strKey, ":");
		INT iTemp = KeyTestType(strKey);
		if( iTemp == -1 )
		{
			return eERRNO_SYS_EOPER;
		}
		m_bRequest = iTemp==1 ? TRUE : FALSE;

		if( m_bRequest )
		{
			pLineParser = m_csHeader.LineParser(eRTSP_H_TYPE_Request);
		
		}
		else
		{
			pLineParser = m_csHeader.LineParser(eRTSP_H_TYPE_Response);
		}
		GS_ASSERT(pLineParser);
	}

	return eERRNO_SYS_EOPER;
		
}



CRtspProPacket *CRtspProPacket::Create( const CRtspHeader &csHeader  )
{
	CRtspProPacket *pRet = Create();
	GS_ASSERT_RET_VAL(pRet, NULL);	
	if( pRet->SetHeader(csHeader) )
	{
		return pRet;
	}
	delete pRet;
	return NULL;
}

BOOL CRtspProPacket::SetHeader( const CRtspHeader &csHeader )
{
	m_csHeader = csHeader;
	return TRUE;
}



BOOL CRtspProPacket::SetContent( const CGSString &strContent )
{
	if( strContent.empty() )
	{
		m_strContent.clear();
	}
	else
	{
	m_strContent  = strContent;
	m_strContent +=  "\r\n";
	}
	return TRUE;
}


/*
****************************************
brief :   CRtspTcpDecoder 的实现
****************************************
*/

static void _OnClearListMember( CRtspProPacket *pObject )
{
	if( pObject )
	{
		pObject->UnrefObject();
	}
}

CRtspTcpDecoder::CRtspTcpDecoder( BOOL bRequest) 
: CGSPObject()
{   
	

}

CRtspTcpDecoder::~CRtspTcpDecoder(void)
{
	
}






namespace GSS
{

namespace RTSP
{

EnumErrno UrlString2MediaInfo( CMediaInfo &csResult, const CGSString &strDescri)
{
	//todo...
	csResult.Clear();
	return eERRNO_SUCCESS;
}

EnumErrno MediaInfo2UrlString( CGSString &strDescri,const CMediaInfo &csResult)
{
	//todo...
	strDescri.clear();
	return eERRNO_SUCCESS;
}	


} //end namespace RTSP

} //end namespace GSS
