
#include "SdpParser.h"
#include "osipparser2/sdp_message.h"
#include "../StrFormater.h"
namespace GSP
{

namespace RTP
{






 CGSString StruSdpAddr::NetTypeI2Str(EnumIPType eNetType)
{
	if( eNetType== eIP_TYPE_IPV4 )
	{
		return CGSString("IP4" );
	}
	return CGSString("IP6");
}

 EnumIPType StruSdpAddr::NetTypeStr2I(const CGSString &strName)
{
	if( GSStrUtil::EqualsIgnoreCase("IP4", GSStrUtil::Trim(strName)) )
	{
		return eIP_TYPE_IPV4;
	}
	return eIP_TYPE_IPV6;
}

/*
*********************************************************************
*
*@brief : CSdpParser
*
*********************************************************************
*/

CSdpParser::CSdpParser(void)
{
	m_iTStart = 0;
	m_iTStop = 0;
	m_strVVersion = "0";
}

CSdpParser::~CSdpParser(void)
{
}

CSdpParser::CSdpParser(const CGSString &strSdp)
{
	m_iTStart = 0;
	m_iTStop = 0;
	Parser(strSdp);
}

void CSdpParser::Clear(void)
{
	CSdpParser csSdp;
	*this = csSdp;
}

EnumErrno CSdpParser::DecodeYAttri(const CGSString &strY, INT &iPlayType, UINT32  &iSSRC)
{
	CGSString str = GSStrUtil::Trim(strY);
	if( strY.length() != 10 )
	{
		GS_ASSERT(0);
		return eERRNO_SYS_EINVALID;
	}
	const char *czStr = strY.c_str();
	iPlayType = *czStr == '0' ? 0 : 1;
	czStr++;
	iSSRC = GSStrUtil::ToNumber<UINT32>(czStr);
	return eERRNO_SUCCESS;
}

CGSString CSdpParser::EncodeYAtri( INT iPlayType, UINT32 iSSRC)
{	
	char czITemp[30];
	GS_SNPRINTF(czITemp, 30, "%d%09d", iPlayType ? 1 : 0, iSSRC );
	return CGSString(czITemp);
}





//解析key value 模式
static EnumErrno KeyValuePasrser(const char **ppInput,
								  char cSlit, //分割符 
								 const char *czEnd,
								 StruSdpAtribute &stResult )
{
	stResult.strName.clear();
	stResult.strValue.clear();
	 char *p = (char*)*ppInput;
	 char *temp;
	CStrFormater::SkipSpaces((const char**)&p);	
	
	if( *p=='\0') 
	{
		GS_ASSERT(0);
		*ppInput = p;
		return eERRNO_SYS_EINVALID;
	}
	temp = p;
	while( *p!='\0' && *p!=cSlit )
	{
		p++;
	}

	if( *p=='\0') 
	{		
		*ppInput = p;
		stResult.strName = GSStrUtil::Trim(temp); //没有VALUE
		return eERRNO_SUCCESS;
	}
	*p ='\0';
	stResult.strName = GSStrUtil::Trim(temp);
	*p = cSlit;

	if( stResult.strName.empty() )
	{
		GS_ASSERT(0);
		*ppInput = p;
		return eERRNO_SYS_EINVALID;
	}

	p++;
	temp = p;
	while( *p!='\0' && NULL==strchr(czEnd,*p) )
	{
		p++;
	}
	cSlit =  *p;
	*p ='\0';
	if( temp == p )
	{
		stResult.strValue.clear();
	}
	else
	{
		stResult.strValue = GSStrUtil::Trim(temp);
	}

	*p = cSlit;
	if( *p!='\0' )
	{
		p++;
	}
	*ppInput = p;
	return eERRNO_SUCCESS;

}

typedef struct _StruSdp_o
{
	// o 选项 列
	// o=123 0 0 IN IP4 127.0.0.1 (owner and session identifier )
	//    Username SessionId SessionVer NetType AddrType AddrIp
	CGSString strUsername; /**< Username */
	CGSString strSessionId; /**< Identifier for session */
	CGSString strSessionVer;   /**< Version of session */
	CGSString strNetType;  /**< Network type */
	CGSString strAddrType;   /**< Address type */
	CGSString strAddrIp;  /**< Address */
	_StruSdp_o(void)
	{
		strUsername = "#";
			strSessionId = "0";
			strSessionVer = "0";
	}
}StruSdp_o;

static EnumErrno ParserSdp_o(const char *czValue, StruSdp_o &stSdp_o )
{
	// o=123 0 0 IN IP4 127.0.0.1 (owner and session identifier )
	const char *p = (const char*)czValue;
	
	CStrFormater::GetWord(stSdp_o.strUsername, &p); //  get Username
	if( *p=='\0' )
	{
		GS_ASSERT(0);
		return eERRNO_SYS_EINVALID;
	}

	CStrFormater::GetWord(stSdp_o.strSessionId, &p); //  get SessionId
	if( stSdp_o.strSessionId.empty() )
	{
		GS_ASSERT(0);
		return eERRNO_SYS_EINVALID;
	}

	CStrFormater::GetWord(stSdp_o.strSessionVer, &p); //  get SessionVer
	if( stSdp_o.strSessionVer.empty() )
	{
		GS_ASSERT(0);
		return eERRNO_SYS_EINVALID;
	}

	CStrFormater::GetWord(stSdp_o.strNetType, &p); //  get NetType
	if( stSdp_o.strNetType.empty() )
	{
		GS_ASSERT(0);
		return eERRNO_SYS_EINVALID;
	}

	CStrFormater::GetWord(stSdp_o.strAddrType, &p); //  get AddrType
	if( stSdp_o.strAddrType.empty() )
	{
		GS_ASSERT(0);
		return eERRNO_SYS_EINVALID;
	}

	CStrFormater::GetWord(stSdp_o.strAddrIp, &p); //  get AddrIp
	if( stSdp_o.strAddrIp.empty() )
	{
		GS_ASSERT(0);
		return eERRNO_SYS_EINVALID;
	}
	return eERRNO_SUCCESS;
}


typedef struct _StruSdp_c
{
	// c 选项 列
	// c=IN IP4 127.0.0.1 (connection information)
	//    NetType AddrType AddrIp
	CGSString strNetType;  /**< Network type */
	CGSString strAddrType;   /**< Address type */
	CGSString strAddrIp;  /**< Address */
	_StruSdp_c(void)
	{

	}
}StruSdp_c;

static EnumErrno ParserSdp_c(const char *czValue, StruSdp_c &stSdp_c )
{
// c=IN IP4 127.0.0.1
	const char *p = czValue;
	CStrFormater::GetWord(stSdp_c.strNetType, &p); //  get NetType
	if( stSdp_c.strNetType.empty() )
	{
		GS_ASSERT(0);
		return eERRNO_SYS_EINVALID;
	}

	CStrFormater::GetWord(stSdp_c.strAddrType, &p); //  get AddrType
	
	if( stSdp_c.strAddrType.empty() )
	{
		GS_ASSERT(0);
		return eERRNO_SYS_EINVALID;
	}

	CStrFormater::GetWord(stSdp_c.strAddrIp, &p); //  get AddrIp
	if( stSdp_c.strAddrIp.empty())
	{
		GS_ASSERT(0);
		return eERRNO_SYS_EINVALID;
	}
	return eERRNO_SUCCESS;
}


typedef struct _StruSdp_t
{
	// t 选项 列
	// t=0 0 (time the session is active) 
	//    格式  RFC4566-5.9 采用 NTP 如果要转换为UNIX时间要减去 2208988800 为 INT64
	INT64 iStart;
	INT64 iStop;
	_StruSdp_t(void) : iStart(0), iStop(0)
	{

	}
}StruSdp_t;

static EnumErrno ParserSdp_t(const char *czValue, StruSdp_t &stSdp_t )
{
	// c=IN IP4 127.0.0.1
	CGSString strTemp;
	const char *p = czValue;
	CStrFormater::GetWord(strTemp, &p);  //get 开始时间 
	if( *p=='\0' || strTemp.empty() )
	{
		GS_ASSERT(0);
		return eERRNO_SYS_EINVALID;
	}
	stSdp_t.iStart  = GSStrUtil::ToNumber<INT64>(strTemp);

	CStrFormater::GetWord(strTemp, &p);  // get 结束时间
	if( strTemp.empty() )
	{
		GS_ASSERT(0);
		return eERRNO_SYS_EINVALID;
	}
	stSdp_t.iStop  = GSStrUtil::ToNumber<INT64>(strTemp);
	
	return eERRNO_SUCCESS;
}

typedef struct _StruSdp_m
{
	// m 选项 列
	// m=video 6000 RTP/AVP 96 97 98 ** (media name and transprot address)
	//    媒体名 传输方式
	CGSString strMediaName;
	INT iPort;
	CGSString strTransportType;
	std::vector<INT> vPlayloadType;
	_StruSdp_m(void) 
	{
		iPort = -1;
	}
}StruSdp_m;

static EnumErrno ParserSdp_m(const char *czValue, StruSdp_m &stSdp_m )
{
	//m=video 6000 RTP/AVP 96 97 98 **
	CGSString strTemp;
	const char *p = czValue;
	CStrFormater::GetWord(stSdp_m.strMediaName, &p); //  get MediaName
	if( stSdp_m.strMediaName.empty() )
	{
		GS_ASSERT(0);
		return eERRNO_SYS_EINVALID;
	}
	

	CStrFormater::GetWord(strTemp, &p); //  get 端口
	if( *p=='\0' || strTemp.empty() )
	{
		GS_ASSERT(0);
		return eERRNO_SYS_EINVALID;
	}
	stSdp_m.iPort  = GSStrUtil::ToNumber<INT>(strTemp);

	CStrFormater::GetWord(stSdp_m.strTransportType, &p); //  get TransportType
	if( stSdp_m.strMediaName.empty() )
	{
		GS_ASSERT(0);
		return eERRNO_SYS_EINVALID;
	}

	INT i = 0;
	while(1) 
	{
		CStrFormater::GetWord(strTemp, &p); //  get 获取PlayloadType
		if( strTemp.empty() )
		{
			break;
		}
		i = GSStrUtil::ToNumber<INT>(strTemp);
		stSdp_m.vPlayloadType.push_back(i);		
	}

	return eERRNO_SUCCESS;
}

#define  StruSdp_a StruSdpAtribute
static EnumErrno ParserSdp_a(const char *czValue, StruSdp_a &stSdp_a )
{
	// a 选项 列
	// a=rtpmap:96 H264/9000 ** (zoero or more media attribute lines)
	//    属性名 参数
	
	return KeyValuePasrser(&czValue, ':', "\r\n", stSdp_a);
}


static EnumErrno ParserSdp_a_rtpmap( const char *czValue, StruSdpRtpmap &fmt )
{
	//rtpmap:96 H264/9000
	CGSString strTemp;
	const char *p = czValue;
	CStrFormater::GetWord(strTemp, &p); //  get PlayloadType
	if( strTemp.empty() )
	{
		GS_ASSERT(0);
		return eERRNO_SYS_EINVALID;
	}
	fmt.eRtpPlayloadType = (EnumRTPPayloadType)GSStrUtil::ToNumber<INT>(strTemp);

	CStrFormater::GetWordUntilChars(fmt.strCodeName,"/\r\n\t ", &p );	
	if(fmt.strCodeName.empty() )
	{
		GS_ASSERT(0);
		return eERRNO_SYS_EINVALID;
	}

	if( *p=='/' )
	{
		p++;
	}

	CStrFormater::GetWord(strTemp, &p); //  get PlayloadType
	if( strTemp.empty() )
	{
		GS_ASSERT(0);
		return eERRNO_SYS_EINVALID;
	}
	fmt.iRate=GSStrUtil::ToNumber<INT>(strTemp);	
	return eERRNO_SUCCESS;	
}




EnumErrno CSdpParser::Parser( const CGSString &strSdp)
{

	Clear();
	const char *p =  strSdp.c_str();
	const char *temp;
	StruSdpAtribute stAt;
	CGSString strTemp;
	

	

	CStrFormater::SkipSpaces(&p);
	if( *p=='\0') 
	{
		GS_ASSERT(0);
		return eERRNO_SYS_EINVALID;
	}

	// sdp  v  解析 v
	if( *p != 'v' )
	{
		//错误
		GS_ASSERT(0);
		return eERRNO_SYS_EINVALID;
	}

	EnumErrno eErrno;
	INT iBreak = 1;
	CStrFormater::SkipSpaces(&p);
	while( iBreak && *p!='\0' )
	{

		eErrno = KeyValuePasrser(&p, '=', "\r\n", stAt );
		if( eErrno  )
		{
			GS_ASSERT(0);
			return eErrno;
		}
		if( stAt.strName.length()!=1 )
		{	
			GS_ASSERT(0);	
			return eERRNO_SYS_EINVALID;
		}
		temp = stAt.strName.c_str();
		switch( temp[0] )
		{
		case 'v' :
			{
				GS_ASSERT(!m_strVVersion.empty() );
				m_strVVersion = stAt.strValue;		
			}
		break;
		case 'o' :
			{
				StruSdp_o stSdp_o;
				eErrno = ParserSdp_o( stAt.strValue.c_str(), stSdp_o );
				GS_ASSERT_RET_VAL(eErrno==eERRNO_SUCCESS, eErrno);
				m_strOUsername = stSdp_o.strUsername;
				m_stOAddr.eNetType = m_stOAddr.NetTypeStr2I(stSdp_o.strAddrType);
				m_stOAddr.strIp = stSdp_o.strAddrIp;
			}
		break;
		case 's' :
			{
				temp = stAt.strValue.c_str();
				CStrFormater::GetWord(m_strSName, &temp); 
			}
		break;
		case 'u' : // URI
			{
				m_strUUri = stAt.strValue;
				
			}
		break;
		case 'c' :
			{
				StruSdp_c stSdp_c;
				eErrno = ParserSdp_c( stAt.strValue.c_str(), stSdp_c );
				GS_ASSERT_RET_VAL(eErrno==eERRNO_SUCCESS, eErrno);
				m_stCAddr.eNetType = m_stOAddr.NetTypeStr2I(stSdp_c.strAddrType);
				m_stCAddr.strIp = stSdp_c.strAddrIp;
			}
		break;
		case 't' :
		{
			StruSdp_t stSdp_t;
			eErrno = ParserSdp_t( stAt.strValue.c_str(), stSdp_t );
			GS_ASSERT_RET_VAL(eErrno==eERRNO_SUCCESS, eErrno);
			m_iTStart = stSdp_t.iStart;
			m_iTStop = stSdp_t.iStop;
		}
		break;
		case 'm' :
			{
				iBreak = 0; //跳出 while 循环
			}
		break;	
		default :
			{
				// TODO...
			}
		break;

		}
		CStrFormater::SkipSpaces(&p);
	};

	CStrFormater::SkipSpaces(&p);
	while( iBreak == 0  && *p!='\0' )
	{
		StruSdp_m stSdp_m;
		eErrno = ParserSdp_m( stAt.strValue.c_str(), stSdp_m );
		GS_ASSERT_RET_VAL(eErrno==eERRNO_SUCCESS, eErrno);

		m_vMedia.push_back(StruSdpMedia());
		StruSdpMedia &stM =  m_vMedia[m_vMedia.size()-1];
		stM.eGsMediaType = MediaTypeName2I(stSdp_m.strMediaName.c_str());

		stM.stPort.vPort[0] = stSdp_m.iPort;
		stM.eTransType = TransportModeName2I(stSdp_m.strTransportType.c_str());
		for( UINT i = 0; i<stSdp_m.vPlayloadType.size(); i++ )
		{
			StruSdpRtpmap stRtpmap;
			stRtpmap.eRtpPlayloadType = (EnumRTPPayloadType) stSdp_m.vPlayloadType[i];
			stM.vRtpmap.push_back(stRtpmap);
		}
		iBreak = 1;
		CStrFormater::SkipSpaces(&p);
		while(  *p!='\0' && iBreak )
		{
			//获取媒体属性
			eErrno = KeyValuePasrser(&p, '=', "\r\n", stAt );
			if( eErrno  )
			{
				GS_ASSERT(0);
				return eErrno;
			}
			if( stAt.strName.length()!=1 )
			{	
				GS_ASSERT(0);	
				return eERRNO_SYS_EINVALID;
			}
			temp = stAt.strName.c_str();
			switch( temp[0] )
			{			
			case 'c' :
				{
					StruSdp_c stSdp_c;
					StruSdpAddr stAddr;
					eErrno = ParserSdp_c( stAt.strValue.c_str(), stSdp_c );
					GS_ASSERT_RET_VAL(eErrno==eERRNO_SUCCESS, eErrno);
					stAddr.eNetType = m_stOAddr.NetTypeStr2I(stSdp_c.strAddrType);
					stAddr.strIp = stSdp_c.strAddrIp;
					stM.vCAddr.push_back(stAddr);
				}
				break;
			case 'a' :
				{
					//属性
					StruSdp_a stSdp_a;
					eErrno = ParserSdp_a( stAt.strValue.c_str(), stSdp_a );
					GS_ASSERT_RET_VAL(eErrno==eERRNO_SUCCESS && !stSdp_a.strName.empty(), eErrno);

					if(GSStrUtil::EqualsIgnoreCase(stSdp_a.strName, "rtpmap") )
					{
						StruSdpRtpmap stRtpmap;
						GS_ASSERT_RET_VAL(!stSdp_a.strValue.empty(), eErrno);

						eErrno = ParserSdp_a_rtpmap(stSdp_a.strValue.c_str(), stRtpmap);
						GS_ASSERT_RET_VAL(eErrno==eERRNO_SUCCESS, eErrno);
						BOOL bNotExist = TRUE;
						for( UINT k = 0; k<stM.vRtpmap.size(); k++ )
						{
							if( stM.vRtpmap[k].eRtpPlayloadType == stRtpmap.eRtpPlayloadType )
							{
								stM.vRtpmap[k] = stRtpmap; //覆盖已经存在的媒体
								bNotExist = FALSE;
								break;
							}
						}
						if( bNotExist )
						{
							stM.vRtpmap.push_back(stRtpmap);
						}
					}
					else if(  GSStrUtil::EqualsIgnoreCase(stSdp_a.strName, "fmtp") )
					{
						GS_ASSERT_RET_VAL(!stSdp_a.strValue.empty(), eErrno);
						stM.strFMTP = stSdp_a.strValue;
					}
					else if(  GSStrUtil::EqualsIgnoreCase(stSdp_a.strName, "control") )
					{
						GS_ASSERT_RET_VAL(!stSdp_a.strValue.empty(), eErrno);
						stM.strCtrl = stSdp_a.strValue;
					}
					else if( GSStrUtil::EqualsIgnoreCase(stSdp_a.strName, "rtcp"))
					{
						GS_ASSERT_RET_VAL(!stSdp_a.strValue.empty(), eErrno);
						stM.stPort.vPort[RTCP_PORT_IDX] = GSStrUtil::ToNumber<INT>(stSdp_a.strValue);
					}
					else
					{
						stM.vAtribute.push_back(StruSdpAtribute(stSdp_a.strName,stSdp_a.strValue));
					}

					
				}
				break;
			case 'y' :
				{
					// y=ssrc
					stM.strYValue = stAt.strValue;
				}
			break;
			case 'b' :
				{
					//带宽 TODO
				}
			break;
			case 'f' :
				{
					//媒体描述

				}
			break;
			case 'm' :
				{
					iBreak = 0; //跳出 while 循环
				}
				break;	
			default :
				{
					// TODO...
				}
				break;
			}
			CStrFormater::SkipSpaces(&p);
		}
		CStrFormater::SkipSpaces(&p);
	}
	
	return eERRNO_SUCCESS;

}

EnumErrno CSdpParser::ToGSMediaInfo( CMediaInfo &csInfo )
{
	csInfo.Clear();

	if( m_vMedia.empty() || m_vMedia[0].vRtpmap.empty()   )
	{
		GS_ASSERT(0);
		return eERRNO_SYS_EINVALID;
	}

	
	for( UINT i = 0; i< m_vMedia[0].vRtpmap.size(); i++ )
	{

		EnumGSCodeID eCodeID = GetGsCodeId4RtpPtName(m_vMedia[0].vRtpmap[i].strCodeName);
		if( eCodeID == GS_CODEID_PS )
		{		
			csInfo.Clear();
			StruGSMediaDescri stDescri;
			bzero(&stDescri, sizeof(stDescri));
			stDescri.eMediaType = GS_MEDIA_TYPE_VIDEO;
			stDescri.unDescri.struVideo.eCodeID = GS_CODEID_PS;		
			csInfo.AddChannel( &stDescri, 0, NULL);	

			bzero(&stDescri, sizeof(stDescri));
			stDescri.eMediaType = GS_MEDIA_TYPE_AUDIO;
			stDescri.unDescri.struAudio.eCodeID = GS_CODEID_AUDIO_ST_G711A;		
			stDescri.unDescri.struAudio.iChannels = 1;		
			stDescri.unDescri.struAudio.iBits = 16;
			stDescri.unDescri.struAudio.iSample = 16000;
			csInfo.AddChannel( &stDescri, 1, NULL);	
			return eERRNO_SUCCESS;
		}
		else if( eCodeID == GS_CODEID_GSC3MVIDEO )
		{
			//获取编码信息
			csInfo.Clear();
			StruSdpAtribute *pAtr;
			CGSString strGsfmt("gsfmt");
			for(UINT j = 0; j<m_vMedia[0].vAtribute.size(); j++ )
			{
				pAtr = &m_vMedia[0].vAtribute[j];
				if( pAtr->strName==strGsfmt )
				{
					//信息格式  DuHua InfoBinary InfoBinary
					CGSString strValue = pAtr->strValue;

					if( strValue.empty() )
					{
						GS_ASSERT(0);
						return eERRNO_SYS_EINVALID;

					}
					const char *p = strValue.c_str();
					CGSString strTemp;

					//厂商
					CStrFormater::GetWord(strTemp, &p);
					if( strTemp.empty() )
					{
						GS_ASSERT(0);
						return eERRNO_SYS_EINVALID;
					}
					int iChnIdx = 0;
					StruGSMediaDescri stDescri;
					while(1)
					{
						CStrFormater::GetWord(strTemp, &p);
						if( strTemp.empty() )
						{
							break;
						}
						bzero(&stDescri, sizeof(stDescri));
						int iLen = sizeof(stDescri);
						if( !CStrFormater::StringToBinary(strTemp, (BYTE*) &stDescri, iLen ) )
						{
							GS_ASSERT(0);
							return eERRNO_SYS_EINVALID;
						}
						csInfo.AddChannel( &stDescri, iChnIdx++, NULL);	
					}
					if( iChnIdx==0 )
					{
						GS_ASSERT(0);
						return eERRNO_SYS_EINVALID;
					}
				}
			}
			return eERRNO_SUCCESS;
		}
	}


// 	StruGSMediaDescri stDescri;
// 	bzero(&stDescri, sizeof(stDescri));
// 	stDescri.eMediaType = GS_MEDIA_TYPE_VIDEO;
// 	stDescri.unDescri.struVideo.eCodeID = GS_CODEID_PS;		
// 	csInfo.AddChannel( &stDescri, 0, NULL);			
	

// 	bzero(&stDescri, sizeof(stDescri));
// 	stDescri.eMediaType = GS_MEDIA_TYPE_AUDIO;
// 	stDescri.unDescri.struAudio.eCodeID = GS_CODEID_AUDIO_ST_G711A;		
// 	stDescri.unDescri.struAudio.iSample = 8000;
// 	stDescri.unDescri.struAudio.iBits = 16;
// 	stDescri.unDescri.struAudio.iChannels = 1;
// 	csInfo.AddChannel( &stDescri, 1, NULL);	


	



	if( csInfo.GetChannelNums() == 0 )
	{		
		CIMediaInfo::StruMediaChannelInfo stInfo;
		bzero(&stInfo, sizeof(stInfo));
		stInfo.stDescri.eMediaType = GS_MEDIA_TYPE_VIDEO;
		stInfo.stDescri.unDescri.struVideo.eCodeID = GetGsCodeId4RtpPtName(m_vMedia[0].vRtpmap[0].strCodeName);
		if( stInfo.stDescri.unDescri.struVideo.eCodeID == GS_CODEID_HK_COMPLEX )
		{
			bzero(&stInfo, sizeof(stInfo));
			stInfo.stDescri.eMediaType = GS_MEDIA_TYPE_SYSHEADER;
			csInfo.AddChannel( &stInfo.stDescri, 0, NULL);	

			bzero(&stInfo, sizeof(stInfo));
			stInfo.stDescri.eMediaType = GS_MEDIA_TYPE_VIDEO;
			stInfo.stDescri.unDescri.struVideo.eCodeID = GS_CODEID_HK_COMPLEX;
			csInfo.AddChannel( &stInfo.stDescri, 1, NULL);	

			bzero(&stInfo, sizeof(stInfo));
			stInfo.stDescri.eMediaType = GS_MEDIA_TYPE_AUDIO;
			stInfo.stDescri.unDescri.struVideo.eCodeID = GS_CODEID_HK_COMPLEX;
			csInfo.AddChannel( &stInfo.stDescri, 2, NULL);	
		}
		else if( stInfo.stDescri.unDescri.struVideo.eCodeID != GS_CODEID_PS )
		{			
			csInfo.AddChannel( &stInfo.stDescri, 0, NULL);			
		}

		
	}
	
	return eERRNO_SUCCESS;
}

CGSString CSdpParser::Serial(void)
{
#define SDPSPC " "
#define SDPLE "\r\n"

	CGSString strRet;
	std::stringstream sstr;
	sstr << "v=" << m_strVVersion << SDPLE;
	sstr << "o=" << m_strOUsername << " 0 0" << " IN " << m_stOAddr.NetTypeI2Str(m_stOAddr.eNetType);
			sstr << SDPSPC << m_stOAddr.strIp << SDPLE;
	if( !m_strSName.empty() )
	{
		sstr << "s=" << m_strSName << SDPLE;
	}
	if( !m_strUUri.empty() )
	{
		sstr << "u=" << m_strUUri << SDPLE;
	}
	if( !m_stCAddr.strIp.empty() )
	{
		sstr << "c=" << "IN " << m_stCAddr.NetTypeI2Str(m_stCAddr.eNetType) << SDPSPC << m_stCAddr.strIp << SDPLE;
	}
	sstr << "t=" << m_iTStart << SDPSPC << m_iTStop << SDPLE;

	for(UINT iM = 0; iM<m_vMedia.size(); iM++)
	{
		sstr << "m=" << MediaTypeI2Name(m_vMedia[iM].eGsMediaType) << SDPSPC << m_vMedia[iM].stPort.vPort[RTP_PORT_IDX];
				sstr << SDPSPC	<< TransportModeI2Name(m_vMedia[iM].eTransType);
			for( UINT i = 0; i<m_vMedia[iM].vRtpmap.size(); i++ )
			{
				sstr << SDPSPC << (INT)m_vMedia[iM].vRtpmap[i].eRtpPlayloadType;
			}
			sstr << SDPLE;


			for( UINT i = 0; i<m_vMedia[iM].vRtpmap.size(); i++ )
			{
				sstr << "a=rtpmap:" <<  (INT)m_vMedia[iM].vRtpmap[i].eRtpPlayloadType;
				sstr << SDPSPC << m_vMedia[iM].vRtpmap[i].strCodeName << "/90000";
				sstr << SDPLE;
			}

		for( UINT i = 0; i<m_vMedia[iM].vCAddr.size(); i++ )
		{
			sstr << "c=" << " IN " << m_stCAddr.NetTypeI2Str(m_vMedia[iM].vCAddr[i].eNetType) 
					<< SDPSPC << m_vMedia[iM].vCAddr[i].strIp << SDPLE;
		}

		if( !m_vMedia[iM].strFMTP.empty() )
		{
			sstr << "a=fmtp:" << m_vMedia[iM].strFMTP << SDPLE;
		}

		if( !m_vMedia[iM].strCtrl.empty() )
		{
			sstr << "a=control:" << m_vMedia[iM].strCtrl << SDPLE;
		}

		for( UINT i = 0; i<m_vMedia[iM].vAtribute.size(); i++ )
		{
			if( m_vMedia[iM].vAtribute[i].strValue.empty() )
			{
				sstr << "a=" << m_vMedia[iM].vAtribute[i].strName << SDPLE;
			}
			else
			{
				sstr << "a=" << m_vMedia[iM].vAtribute[i].strName << ":" <<m_vMedia[iM].vAtribute[i].strValue  << SDPLE;
			}
		}

		if(m_vMedia[iM].stPort.vPort[RTCP_PORT_IDX]!=0 &&
			(m_vMedia[iM].stPort.vPort[RTP_PORT_IDX]+1) != m_vMedia[iM].stPort.vPort[RTCP_PORT_IDX] )
		{
			sstr << "a=rtcp:" << m_vMedia[iM].stPort.vPort[RTCP_PORT_IDX] << SDPSPC;
		}

		if( !m_vMedia[iM].strYValue.empty() )
		{
			sstr << "y=" << m_vMedia[iM].strYValue << SDPLE;
		}
		if( !m_vMedia[iM].strFValue.empty() )
		{
			sstr << "f=" << m_vMedia[iM].strYValue << SDPLE;
		}

	}

	return sstr.str();
}



} //end namespace RTP

} //end namespace GSP