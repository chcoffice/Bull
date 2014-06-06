/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : RTSPPARSER.H
Author :  zouyx
Version : 0.0.0.0
Date: 2011/11/24 15:11
Description: 
********************************************
*/

#ifndef _GS_H_RTSPPARSER_H_
#define _GS_H_RTSPPARSER_H_

#include "RTSPStru.h"
#include "../GSPObject.h"

namespace GSP
{

	namespace RTSP
	{

		typedef enum 
		{
			eRTSP_H_TYPE_Request = 0,
			eRTSP_H_TYPE_Response,
			eRTSP_H_TYPE_Server,
			eRTSP_H_TYPE_CSeq,
			eRTSP_H_TYPE_Date,
			eRTSP_H_TYPE_Session,
			eRTSP_H_TYPE_ContenType,
			eRTSP_H_TYPE_ContenLength,
			eRTSP_H_TYPE_Scale,
			eRTSP_H_TYPE_Range,
			eRTSP_H_TYPE_Connection,
			eRTSP_H_TYPE_KeepAlive,
			eRTSP_H_TYPE_Accept,			
			eRTSP_H_TYPE_UserAgent,
			eRTSP_H_TYPE_RTPInfo,
			eRTSP_H_TYPE_Allow,
			eRTSP_H_TYPE_Transport,
			eRTSP_H_TYPE_CacheControl,			
			eRTSP_H_TYPE_Speed,
			eRTSP_H_TYPE_PauseTime,


			eRTSP_H_TYPE_END,
		}EnumRtspHeaderType;


		


		/*
		*********************************************************************
		*
		*@brief : 行分析器
		*
		*********************************************************************
		*/

		class CIRtspLineParser : public CGSPObject
		{
		public :
			CGSString m_strKey;			
			EnumRtspHeaderType m_eType;
			CIRtspLineParser(const CIRtspLineParser &csDest )
				:CGSPObject()
			{
				m_strKey = csDest.m_strKey;
				m_eType = GetRtspHeaderType(m_strKey);
			}

			CIRtspLineParser( const CGSString &strKey )
				:CGSPObject( )
			{
				m_strKey = strKey;
				m_eType = GetRtspHeaderType(m_strKey);				
			}

			virtual ~CIRtspLineParser(void)
			{

			}
			virtual CGSString Serial(void) = 0;
			virtual BOOL Parser(const char *szLine, INT iLength ) = 0;

			virtual void Reset(void)
			{

			}

			static EnumRtspHeaderType GetRtspHeaderType( CGSString &strKey );
			//static CGSString GetRtspHeaderTypeStr(EnumRtspHeaderType eType );

			static CIRtspLineParser *Create(EnumRtspHeaderType eType );

			static const char *GetRtspCommandName( EnumRTSPComandMask eMask);
			static EnumRTSPComandMask GetRtspCommandMask(const char *szName );
		};


		// 处理Request 的首行
		class CRtspRequestParser  : public  CIRtspLineParser
		{
		public :
			//OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, SCALE, GET_PARAMETER
			CGSString m_strMethod;
			CGSString m_strUrl;
			CGSString m_strVersion;
			CRtspRequestParser( const CGSString &strKey="UNKNOWN" )
				:CIRtspLineParser("Request")
			{
				Reset();
			}

			virtual CGSString Serial(void);
			virtual BOOL Parser(const char *szLine, INT iLength );
			virtual void Reset(void);
		};

		//处理Response 的首行
		class CRtspResponseParser : public CIRtspLineParser
		{
		public :

			EnumResponseStatus m_eResNo;
			CGSString m_strError;
			CGSString m_strVersion;
			CRtspResponseParser(void)
				:CIRtspLineParser("Response")
			{

				Reset();
			}
			virtual CGSString Serial(void);
			virtual BOOL Parser(const char *szLine, INT iLength );
			virtual void Reset(void);

		};



		//处理CSeq
		class CRtspCSeqParser : public CIRtspLineParser
		{
		public :
			CGSString m_strCSeq;
			CRtspCSeqParser(void)	
				:CIRtspLineParser("CSeq")
			{
				Reset();
			}
			virtual CGSString Serial(void);
			virtual BOOL Parser(const char *szLine, INT iLength );
			virtual void Reset(void);
		};

		//处理Server
		class CRtspServerParser : public  CIRtspLineParser
		{
		public :
			CGSString m_strServer;
			CRtspServerParser(void)	
				:CIRtspLineParser("Server")
			{

				Reset();
			}
			virtual CGSString Serial(void);
			virtual BOOL Parser(const char *szLine, INT iLength );
			virtual void Reset(void);

		};





		class CRtspDateParser : public CIRtspLineParser
		{
		public :
			CGSString m_strGMT;
			CRtspDateParser(void)	
				:CIRtspLineParser("Date")
			{
				Reset();				
			}
			virtual CGSString Serial(void);
			virtual BOOL Parser(const char *szLine, INT iLength );
			virtual void Reset(void);

		};

		class CRtspAcceptParser : public CIRtspLineParser
		{
		public :
			EnumContentType m_eContentType;
			CRtspAcceptParser(void)	
				:CIRtspLineParser("Accept")
			{

				Reset();				
			}
			virtual CGSString Serial(void);
			virtual BOOL Parser(const char *szLine, INT iLength );
			virtual void Reset(void);
		};


		class CRtspContenTypeParser : public CIRtspLineParser
		{
		public :
			EnumContentType m_eContentType;
			CRtspContenTypeParser(void)	
				:CIRtspLineParser("Content-Type")
			{

				Reset();				
			}
			virtual CGSString Serial(void);
			virtual BOOL Parser(const char *szLine, INT iLength );
			virtual void Reset(void);

		};

		class CRtspContenLengthParser : public CIRtspLineParser
		{
		public :
			INT m_iLength;
			CRtspContenLengthParser(void)	
				:CIRtspLineParser("Content-Length")
			{

				Reset();				
			}
			virtual CGSString Serial(void);
			virtual BOOL Parser(const char *szLine, INT iLength );
			virtual void Reset(void);


		};

		class CRtspRangeParser : public CIRtspLineParser
		{
		public :
			double m_fBegin;
			double m_fEnd;
			CRtspRangeParser(void)	
				:CIRtspLineParser("Range")
			{

				Reset();				
			}
			virtual CGSString Serial(void);
			virtual BOOL Parser(const char *szLine, INT iLength );
			virtual void Reset(void);


		};

		class CRtspPauseTimeParser : public CIRtspLineParser
		{
		public :
			double m_fPauseTime;
			CRtspPauseTimeParser(void)	
				:CIRtspLineParser("PauseTime")
			{

				Reset();				
			}
			virtual CGSString Serial(void);
			virtual BOOL Parser(const char *szLine, INT iLength );
			virtual void Reset(void);


		};

		class CRtspConnectionParser : public CIRtspLineParser
		{
		public :
			CGSString m_strValue;
			CRtspConnectionParser(void)	
				:CIRtspLineParser("Connection")
			{

				Reset();				
			}
			virtual CGSString Serial(void);
			virtual BOOL Parser(const char *szLine, INT iLength );
			virtual void Reset(void);

		};


		class CRtspSessionParser : public CIRtspLineParser
		{
		public :
			int m_iTimeouts;
			CGSString  m_strSession;		
			
			CRtspSessionParser(void)	
				:CIRtspLineParser("Session")
			{

				Reset();				
			}
			virtual CGSString Serial(void);
			virtual BOOL Parser(const char *szLine, INT iLength );
			virtual void Reset(void);
			INLINE void SetSession( const CGSString &strSession  )
			{
				m_strSession = strSession;
			}


		};

		class CRtspKeepAliveParser : public CIRtspLineParser
		{
		public :
			CGSString m_strValue;
			CRtspKeepAliveParser(void)	
				:CIRtspLineParser("Keep-Alive")
			{

				Reset();				
			}
			virtual CGSString Serial(void);
			virtual BOOL Parser(const char *szLine, INT iLength );
			virtual void Reset(void);

		};



		class CRtspUserAgentParser : public CIRtspLineParser
		{
		public :
			CGSString m_strAgent;
			CRtspUserAgentParser(void)	
				:CIRtspLineParser("User-Agent")
			{

				Reset();				
			}
			virtual CGSString Serial(void);
			virtual BOOL Parser(const char *szLine, INT iLength );
			virtual void Reset(void);

		};


		class CRtspRTPInfoParser : public CIRtspLineParser
		{
		public :
			//url=trackID=0;seq=17040;rtptime=1467265309  
			CGSString m_strUrl;	
			INT m_iRtpSeq;
			INT m_iRtpTime;
			CRtspRTPInfoParser(void)	
				:CIRtspLineParser("RTP-Info")
			{
				Reset();
			}
			virtual CGSString Serial(void);
			virtual BOOL Parser(const char *szLine, INT iLength );
			virtual void Reset(void);

		};

		class CRtspCacheControlParser : public CIRtspLineParser
		{
		public :
			CGSString m_strType;
			CRtspCacheControlParser(void)	
				:CIRtspLineParser("Cache-Control")
			{


				Reset();				
			}
			virtual CGSString Serial(void);
			virtual BOOL Parser(const char *szLine, INT iLength );
			virtual void Reset(void);

		};


		class CRtspAllowParser : public CIRtspLineParser
		{
		public :
			// OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, GET_PARAMETER, SET_PARAMETER
			EnumRTSPComandMask m_eAllowMask;


			CRtspAllowParser(void)	
				:CIRtspLineParser("Allow")
			{

				Reset();				
			}
			virtual CGSString Serial(void);
			virtual BOOL Parser(const char *szLine, INT iLength );
			virtual void Reset(void);

		};

		




		class CRtspTransportParser : public CIRtspLineParser
		{
		public :
			typedef struct _StruTransPortField
			{
				EnumTransportMode eTrType;			
				StruUdpPortPair stCliPort;
				StruUdpPortPair stSrvPort;
				INT iInterleavedOfRtpChnID;
				INT iInterleavedOfRtcpChnID;
				CGSString strDestAddr;
				LONG iDestTTL;
				BOOL bMulticast;
				CGSString strSrvAddr;

				_StruTransPortField(void)
				{
					Reset();
				}
				void Reset(void)
				{
					eTrType = eTRANSPORT_RTP_UDP;	
					bzero( &stCliPort, sizeof(stCliPort) );
					bzero( &stSrvPort, sizeof(stSrvPort) );
					iInterleavedOfRtpChnID = 0;
					iInterleavedOfRtcpChnID = 1;
					strDestAddr.clear();
					iDestTTL = -1;
					bMulticast = FALSE;
					strSrvAddr.clear();
				}
			}StruTransPortField;

			std::vector<StruTransPortField> m_vFields;


			CRtspTransportParser(void)	
				:CIRtspLineParser("Transport")
			{

			}
			virtual CGSString Serial(void);
			virtual BOOL Parser(const char *szLine, INT iLength );
			virtual void Reset(void);

		};



		class CRtspScaleParser : public CIRtspLineParser
		{
		public :
			float m_iScale;
			CRtspScaleParser(void)	
				:CIRtspLineParser("Scale")
			{

				Reset();				
			}
			virtual CGSString Serial(void);
			virtual BOOL Parser(const char *szLine, INT iLength );
			virtual void Reset(void);


		};


		class CRtspSpeedParser : public CIRtspLineParser
		{
		public :
			float m_iSpeed;
			CRtspSpeedParser(void)	
				:CIRtspLineParser("Speed")
			{

				Reset();				
			}
			virtual CGSString Serial(void);
			virtual BOOL Parser(const char *szLine, INT iLength );
			virtual void Reset(void);


		};




		/*
		*********************************************************************
		*
		*@brief : 
		*
		*********************************************************************
		*/
		class  CRtspHeader : public CGSPObject
		{
		public :
			CRtspRequestParser m_csRequest;
			CRtspResponseParser m_csResponse;
			CRtspCSeqParser m_csCSeq;
			CRtspDateParser m_csDate;
			CRtspSessionParser m_csSession;
			CRtspContenTypeParser m_csContentType;
			CRtspContenLengthParser m_csContenLength;
			CRtspRangeParser m_csRange;
			CRtspConnectionParser m_csConnect;
			CRtspKeepAliveParser m_csKeepAlive;
			CRtspAcceptParser m_csAccept;
			CRtspServerParser m_csServer;
			CRtspUserAgentParser m_csUserAgent;
			CRtspRTPInfoParser m_csRtpInfo;
			CRtspAllowParser m_csAllow;
			CRtspTransportParser m_csTransPort;
			CRtspCacheControlParser m_csCacheControl;
			CRtspScaleParser m_csScale;
			CRtspSpeedParser m_csSpeed;
			CRtspPauseTimeParser m_csPauseTime;


			CIRtspLineParser *m_vParser[eRTSP_H_TYPE_END];

			
			CRtspHeader(void);
			CRtspHeader(const CRtspHeader &csDest);
			virtual ~CRtspHeader(void);

			CRtspHeader& operator=(const CRtspHeader &csDest);

			CIRtspLineParser *LineParser(EnumRtspHeaderType eType);
			CGSString Serial(void);

			INLINE CIRtspLineParser *GetExistParser( EnumRtspHeaderType eType ) const
			{
				GS_ASSERT((UINT)eType<(UINT)eRTSP_H_TYPE_END);
				return m_vParser[(UINT)eType];
			}

		};


	} //end namespace RTSP

} //end namespace GSS

#endif //end _GS_H_RTSPPARSER_H_