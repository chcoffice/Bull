/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : SDPPARSER.H
Author :  邹阳星
Version : 1.0.0.1
Date: 2012/11/8 10:01
Description: sdp 解析
********************************************
*/

#ifndef _GS_H_SDPPARSER_H_
#define _GS_H_SDPPARSER_H_
#include <vector>
#include "../ISocket.h"
#include "RtpStru.h"
#include "../MediaInfo.h"



namespace GSP
{

namespace RTP
{


	typedef struct _StruSdpAtribute
	{
		CGSString strName;
		CGSString strValue;
		_StruSdpAtribute( const  CGSString &name, const CGSString &val )
		{
			strName = name;
			strValue = val;

		}

		_StruSdpAtribute(void) : strName(),strValue()
		{

		}
		void Set(const  CGSString &name )
		{
			strName = name;
			strValue.clear();
		}

		void Set( const  CGSString &name, const CGSString &val )
		{
			strName = name;
			strValue = val;

		}

	}StruSdpAtribute;

	typedef struct _StruSdpAddr
	{
		CGSString strIp;
		EnumIPType eNetType;
		_StruSdpAddr(void) :
		strIp()		
			,eNetType(eIP_TYPE_IPV4)
		{

		}
		static CGSString NetTypeI2Str(EnumIPType eNetType);
		static EnumIPType NetTypeStr2I(const CGSString &strName);

	}StruSdpAddr;

	typedef std::vector<StruSdpAddr> CVectorSdpAddr;

	typedef struct _StruSdpRtpmap
	{
		EnumRTPPayloadType eRtpPlayloadType; 
		CGSString strCodeName;
		int iRate;	
		
		_StruSdpRtpmap(void) :
		eRtpPlayloadType(eRTP_PT_GXX)
			,strCodeName()
			,iRate(90000)			
			
		{

		}
		
	}StruSdpRtpmap;

	typedef std::vector<StruSdpRtpmap> CVectorSdpRtpmap;

	typedef struct _StruSdpMedia
	{
		EnumGSMediaType  eGsMediaType;
		EnumTransportMode eTransType;	
		StruUdpPortPair stPort;	
		CVectorSdpRtpmap vRtpmap;
		
		CVectorSdpAddr vCAddr;
		CGSString strFMTP;
		CGSString strCtrl;   // a=control
		std::vector<StruSdpAtribute> vAtribute; //除了 a=rtpmap, a=fmtp 外的其他属性
		CGSString strYValue; // y=ssrc
		CGSString strFValue;  // f=
		_StruSdpMedia() :
		eGsMediaType(GS_MEDIA_TYPE_NONE)
			,eTransType(eTRANSPORT_RTP_UDP)			
			,vRtpmap()				
			,vCAddr()
			,strFMTP()
			,strCtrl()
			,vAtribute()
			,strYValue()
			,strFValue()
		{
			stPort.vPort[0] = 0;
			stPort.vPort[1] = 0;
		}

				
	}StruSdpMedia;

	typedef std::vector<StruSdpMedia> CVectorSdpMedia;




	class CSdpParser : public CGSPObject
	{
	public :
		CGSString m_strVVersion;
		INT64 m_iTStart;
		INT64 m_iTStop;
		CGSString m_strOUsername;
		StruSdpAddr m_stOAddr;
		CGSString m_strSName;
		StruSdpAddr m_stCAddr;
		CVectorSdpMedia m_vMedia; 
		CGSString m_strUUri;

		CSdpParser(void);
		CSdpParser(const CGSString &strSdp);
		~CSdpParser(void);

		EnumErrno Parser( const CGSString &strSdp);
		CGSString Serial(void);

		EnumErrno ToGSMediaInfo( CMediaInfo &csInfo );


		void Clear(void);


		static EnumErrno DecodeYAttri(const CGSString &strY, INT &iPlayType, UINT32  &iSSRC);
		static CGSString EncodeYAtri( INT iPlayType, UINT32  iSSRC);
	};




	//CGSString BinaryByte2HexString( const void *pBuf, int iSize );
	// EnumErrno BinaryHexString2Byte(const CGSString &strVal,  void *pBuf, int &iSize );

} //end namespace RTP

} //end namespace GSP

#endif //end _GS_H_SDPPARSER_H_