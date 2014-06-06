#include "SIPParser.h"
#include <sstream>
#include "../RTSP/RtspParser.h"
#include "../StrFormater.h"
using namespace GSP;
using namespace GSP::SIP;
using namespace GSP::RTSP;

CSIPParser::CSIPParser(void)
{
}

CSIPParser::~CSIPParser(void)
{
}


/*
*********************************************************************
*
*@brief : C28181ParserSubject
*
*********************************************************************
*/

C28181ParserSubject::C28181ParserSubject(const CGSString &strStr  )
{
	Decode(strStr);
}

C28181ParserSubject::C28181ParserSubject(void)
{

}

BOOL C28181ParserSubject::Decode( const CGSString &strStr )
{
	 m_strSendDevID.clear();
	 m_strSendStreamSeq.clear(); //发送端媒体流序列号
	 m_strSendDevID.clear();  //媒体流接收者设备编码
	 m_strSendStreamSeq.clear(); //接收端媒体流系列号
	 std::vector<std::string> vString;
	 GSStrUtil::Split(vString, strStr, "," );
	 if( vString.size()==2)
	 {	
		std::vector<std::string> vTemp;
		GSStrUtil::Split(vTemp, vString[0], ":" );
		if( vTemp.size()!=2)
		{
			GS_ASSERT(0);
			return FALSE;
		}
		m_strSendDevID = vTemp[0];
		m_strSendStreamSeq = vTemp[1];

		vTemp.clear();

		GSStrUtil::Split(vTemp, vString[1], ":" );
		if( vTemp.size()!=2)
		{
			GS_ASSERT(0);
			return FALSE;
		}
		m_strRcvDevID = vTemp[0];
		m_strRcvStreamSeq = vTemp[1];

	 }
	 else
	 {
		 GS_ASSERT(0);
		 return FALSE;
	 }
	 return TRUE;
}

CGSString C28181ParserSubject::Encode(void)
{
	stringstream sstr;
	sstr << m_strSendDevID << ":" << m_strSendStreamSeq;
	sstr << "," << m_strRcvDevID << ":" << m_strRcvStreamSeq;
	return sstr.str();
}



/*
*********************************************************************
*
*@brief : CMANSCDP_XMLParser
*
*********************************************************************
*/
CMANSCDP_XMLParser::FuncPtrMXmlParser CMANSCDP_XMLParser::s_vDecoders[] = 
{
	(CMANSCDP_XMLParser::FuncPtrMXmlParser)&CMXmlMediaStatusParser::Create,
	//在下面添加函数

	NULL //放在最后
};

CMANSCDP_XMLParser::CMANSCDP_XMLParser(void)
{
	m_strCmdType.clear();
}

CMANSCDP_XMLParser::~CMANSCDP_XMLParser(void)
{

}


CMANSCDP_XMLParser *CMANSCDP_XMLParser::GuessParser(const CGSString &strVal)
{
CMANSCDP_XMLParser *pRet = NULL;
XMLNode stRootXmlNode,stXmlNode, stXmlTempNode;
CGSString strCmdType;

	stRootXmlNode = stXmlNode.parseString(strVal.c_str());

	
	if( stRootXmlNode.isEmpty() )
	{
		GS_ASSERT(0);
		return NULL;
	}

	for( int i = 0; i<stRootXmlNode.nChildNode(); i++ )
	{
		stXmlNode = stRootXmlNode.getChildNode(i);
		if( stRootXmlNode.isEmpty() )
		{
			GS_ASSERT(0);
			return NULL;
		}
		if( !stXmlNode.isDeclaration()  )
		{
			stXmlTempNode = stXmlNode.getChildNode("CmdType");
			if( stXmlTempNode.isEmpty() )
			{
				GS_ASSERT(0);
				continue;
			}
			strCmdType = stXmlTempNode.getText();
			break;
		}
	}

	if(strCmdType.empty() )
	{
		GS_ASSERT(0);
		return NULL;
	}

	for( int i = 0;  s_vDecoders[i]; i++ )
	{
		if( (pRet= (*s_vDecoders[i])(strCmdType, stXmlNode ) ))
		{
			break;
		}
	}
	return pRet;
}

/*
*********************************************************************
*
*@brief : CMXmlMediaStatusParser
*
*********************************************************************
*/
const char CMXmlMediaStatusParser::s_czCmdType[] = "MediaStatus";

CMXmlMediaStatusParser::CMXmlMediaStatusParser(void)
{
	m_strCmdType = s_czCmdType;
	m_strSn = "1";
	m_strDevId.clear();
	m_strNotifyType = "121";
}

CMXmlMediaStatusParser::~CMXmlMediaStatusParser(void)
{

}

CGSString CMXmlMediaStatusParser::Encode(void)
{
	XMLNode stXmlNotify, stXmlNode, stXmlRoot;

	
	stXmlNotify =  stXmlNotify.createXMLTopNode("Notify");
	stXmlNode = stXmlNotify.addChild("CmdType");
	stXmlNode.addText(m_strCmdType.c_str());

	if( !m_strSn.empty() )
	{
		stXmlNode = stXmlNotify.addChild("SN");
		stXmlNode.addText(m_strSn.c_str());
	}

	if( !m_strDevId.empty() )
	{
		stXmlNode = stXmlNotify.addChild("DeviceID");
		stXmlNode.addText(m_strDevId.c_str());
	}

	stXmlNode = stXmlNotify.addChild("NotifyType");
	stXmlNode.addText(m_strNotifyType.c_str());

	XMLSTR szXml = stXmlNotify.createXMLString();
	CGSString strRet = "<?xml version=\"1.0\"?>\n";
	strRet += szXml;
	freeXMLString(szXml);

	return strRet;
}


CMANSCDP_XMLParser *CMXmlMediaStatusParser::Create( const CGSString &strCmdType,
								  XMLNode &stRootNode)
{
	if( CGSString("MediaStatus") != strCmdType )
	{
		return NULL;
	}
CMXmlMediaStatusParser *pRet = new CMXmlMediaStatusParser();
	GS_ASSERT_RET_VAL(pRet, NULL);

	XMLNode stXmlNode;	
	stXmlNode = stRootNode.getChildNode("SN");
	if( stXmlNode.isEmpty() )
	{
		GS_ASSERT(0);
		pRet->m_strSn.clear();
	}
	else
	{
		pRet->m_strSn = stXmlNode.getText();
	}
	
	stXmlNode = stRootNode.getChildNode("DeviceID");
	if( stXmlNode.isEmpty() )
	{
		GS_ASSERT(0);
		pRet->m_strDevId.clear();
	}
	else
	{
		pRet->m_strDevId = stXmlNode.getText();
	}
	
	stXmlNode = stRootNode.getChildNode("NotifyType");
	if( stXmlNode.isEmpty() )
	{
		GS_ASSERT(0);
		pRet->m_strNotifyType.clear();
	}
	else
	{
		pRet->m_strNotifyType = stXmlNode.getText();
	}
	return pRet;

}


/*
*********************************************************************
*
*@brief : CMansRtspPaser
*
*********************************************************************
*/
CMansRtspPaser::CMansRtspPaser(void)
{
	bzero(&m_stGspCtrl, sizeof(m_stGspCtrl));
	m_fSpeed = 1.0;
}

CMansRtspPaser::~CMansRtspPaser(void)
{

}

EnumErrno CMansRtspPaser::Decode( const CGSString &strVal)
{
	/*
	PLAY MANSRTSP/1.0
	CSeq:5
	Scale:1.0
	Range:ntp=196-
	*/

const char *p = strVal.c_str();
CGSString strTemp;
CGSString strLine;


	bzero(&m_stGspCtrl, sizeof(m_stGspCtrl));
	m_fSpeed = 1.0;


	CStrFormater::GetWordUntilChars(strLine,"\n", &p); //获取一行
	
	if( strLine.empty() )
	{
		GS_ASSERT(0);
		return eERRNO_SYS_EINVALID;
	}
	CStrFormater::SkipSpaces(&p);

	CRtspHeader csRtspHeader;

	//解析请求方法
	csRtspHeader.LineParser(eRTSP_H_TYPE_Request);

	if( !csRtspHeader.m_csRequest.Parser(strLine.c_str(), strLine.length()) )
	{
		GS_ASSERT(0);
		return eERRNO_SYS_EINVALID;
	}

	EnumRTSPComandMask eRtspCmd = csRtspHeader.m_csRequest.GetRtspCommandMask(csRtspHeader.m_csRequest.m_strMethod.c_str());
	bzero( &m_stGspCtrl, sizeof(m_stGspCtrl));
	switch( eRtspCmd )
	{
	case eRTSP_CMD_TEARDOWN:
		{
			m_stGspCtrl.iCtrlID = GSP_CTRL_STOP;
		}
	break;
	case eRTSP_CMD_PLAY :
		{
			m_stGspCtrl.iCtrlID = GSP_CTRL_PLAY;
		}
	break;
	case eRTSP_CMD_PAUSE :
	
		{
			m_stGspCtrl.iCtrlID = GSP_CTRL_PAUSE;
		}
	break;
	default :
		GS_ASSERT(0);
	break;
	}

	const char *pTemp;
	EnumRtspHeaderType eType;
	CIRtspLineParser *pLineParser;
	while(1)
	{

		CStrFormater::GetWordUntilChars(strLine,"\n", &p); //获取一行

		if( strLine.empty() )
		{
			break;
		}
		CStrFormater::SkipSpaces(&p);
		pTemp = strLine.c_str();

		CStrFormater::GetWordUntilChars(strTemp, RTP_SPACE_CHARS ":", &pTemp);
		CStrFormater::SkipSpaces(&p);
		if( *pTemp == ':' )
		{
			pTemp++;
		}
		CStrFormater::RightTrim(strTemp, ":");

		eType = CIRtspLineParser::GetRtspHeaderType(strTemp);

		pLineParser = csRtspHeader.LineParser(eType);		
		if( !pLineParser )
		{
			//未知的类型
			GS_ASSERT(0);
			continue;
		}
		
		if( !pLineParser->Parser(strLine.c_str() ,strLine.length() ) )
		{
			GS_ASSERT(0);
			return eERRNO_SYS_EINVALID;
		}

		switch( pLineParser->m_eType )
		{
		case eRTSP_H_TYPE_Scale :
			{
				//速度
				CRtspScaleParser *pParser = dynamic_cast<CRtspScaleParser *>(pLineParser);	
				double fRate = pParser->m_iScale;

				m_fSpeed = pParser->m_iScale;

				if( m_stGspCtrl.iCtrlID == 0 )
				{
					break;
				}


				if( pParser->m_iScale<0 )
				{
					fRate = - pParser->m_iScale;
				}

				if( eRtspCmd==eRTSP_CMD_PLAY && fRate != 0.00 && (fRate<0.95 || fRate>1.05 ))
				{					
					if( pParser->m_iScale<0 )
					{
						//后退放
						m_stGspCtrl.iCtrlID = fRate < 1.0 ? GSP_CTRL_BSLOW  : GSP_CTRL_BFAST; 
					}
					else
					{
						//进放
						m_stGspCtrl.iCtrlID =   fRate < 1.0 ? GSP_CTRL_SLOW  : GSP_CTRL_FAST; 
					}
					if( fRate < 1.0 )
					{
						m_stGspCtrl.iArgs1 =  (INT)(1.0/fRate);
					}
					else
					{
						m_stGspCtrl.iArgs1 = ROUND(fRate);
					}					
				}
			}
		break;
		case eRTSP_H_TYPE_Range :
			{
				CRtspRangeParser *pParser = dynamic_cast<CRtspRangeParser *>(pLineParser);	
				if( eRtspCmd==eRTSP_CMD_PLAY  )
				{
					//拖动
					if( pParser->m_fBegin > 0.0 )
					{
						m_stGspCtrl.iCtrlID = GSP_CTRL_SETPOINT;
						m_stGspCtrl.iArgs1 = GSP_OFFSET_TYPE_SECS;
						m_stGspCtrl.iArgs2 = (INT32) pParser->m_fBegin;
					}

				}
			}
		break;
		}
	}

	return eERRNO_SUCCESS;

}

CGSString CMansRtspPaser::Encode(UINT32 iCSeq)
{
	CRtspHeader csRtspHeader;
	csRtspHeader.LineParser(eRTSP_H_TYPE_Request);

	csRtspHeader.m_csCSeq.m_strCSeq = GSStrUtil::ToString(iCSeq);




	switch(m_stGspCtrl.iCtrlID )
	{
	case GSP_CTRL_PLAY :
	case GSP_CTRL_FAST :
	case GSP_CTRL_SLOW :
		{
			csRtspHeader.m_csRequest.m_strMethod = "PLAY";
			csRtspHeader.LineParser(eRTSP_H_TYPE_CSeq);
			csRtspHeader.LineParser(eRTSP_H_TYPE_Scale);
			csRtspHeader.m_csScale.m_iScale = (float)m_fSpeed;		
			csRtspHeader.LineParser(eRTSP_H_TYPE_Range);
			csRtspHeader.m_csRange.m_fBegin = 0.0;
			csRtspHeader.m_csRange.m_fEnd = 0.0;
		}
		break;	
		// 	case GSP_CTRL_STEP :
		// 		{
		//         没有单帧
		// 		}
		// 		break;
	case GSP_CTRL_SETPOINT :
		{
			csRtspHeader.m_csRequest.m_strMethod = "PLAY";
			csRtspHeader.LineParser(eRTSP_H_TYPE_CSeq);
			csRtspHeader.LineParser(eRTSP_H_TYPE_Scale);
			csRtspHeader.LineParser(eRTSP_H_TYPE_Range);
			csRtspHeader.m_csScale.m_iScale = (float)m_fSpeed;
			csRtspHeader.m_csRange.m_fBegin = m_stGspCtrl.iArgs2;
			csRtspHeader.m_csRange.m_fEnd = 0.0;
		}
		break;
	case GSP_CTRL_PAUSE :
		{
			csRtspHeader.LineParser(eRTSP_H_TYPE_CSeq);
			csRtspHeader.LineParser(eRTSP_H_TYPE_PauseTime);
			csRtspHeader.m_csRequest.m_strMethod = "PAUSE";			
		}
		break;
	case GSP_CTRL_STOP :
		{
			csRtspHeader.LineParser(eRTSP_H_TYPE_CSeq);
			csRtspHeader.m_csRequest.m_strMethod = "TEARDOWN";
		}
		break;
	}
	csRtspHeader.m_csRequest.m_strVersion = "MANSRTSP/1.0";
	if( csRtspHeader.m_csScale.m_iScale<0.1)
	{
		csRtspHeader.m_csScale.m_iScale =(float) 0.1;
	}
	CGSString strRet = csRtspHeader.Serial();

	
	return strRet;
	
}










/*
*********************************************************************
*
*@brief : other function
*
*********************************************************************
*/
namespace GSP
{


namespace SIP
{

struct _StruKeyType
{
	INT iTrans;
	const char *czName;
};

static struct _StruKeyType s_vTransKeyV[] = 
{
	{GSP_TRAN_RTPLAY, "Play"},
	{GSP_TRAN_DOWNLOAD, "Download"},
	{GSP_TRAN_REPLAY, "Playback"},

};


INT TransModel28181SSName2I( const CGSString &strSName )
{
	for( int i = 0; i<ARRARY_SIZE(s_vTransKeyV); i++ )
	{
		if( GSStrUtil::EqualsIgnoreCase(s_vTransKeyV[i].czName, strSName))
		{
			return s_vTransKeyV[i].iTrans;
		}
	}
	return s_vTransKeyV[0].iTrans;
}

const char *TransModel28181I2SName( INT iModel )
{
	for( int i = 0; i<ARRARY_SIZE(s_vTransKeyV); i++ )
	{
		if( s_vTransKeyV[i].iTrans==iModel)
		{
			return s_vTransKeyV[i].czName;
		}
	}
	return s_vTransKeyV[0].czName;
}

} //end namespace GSS

} //end namespace SIP