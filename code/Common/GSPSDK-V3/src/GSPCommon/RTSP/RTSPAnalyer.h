/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : RTSPANALYER.H
Author :  zouyx
Version : 0.0.0.0
Date: 2011/11/24 9:32
Description: 
********************************************
*/

#ifndef _GS_H_RTSPANALYER_H_
#define _GS_H_RTSPANALYER_H_


/*
******************************************
Copyright (C), 2010-2011, GOSUN
Filename : ANALYER.H
Author :  zouyx
Version : 0.1.0.0
Date: 2010/6/13 8:43
Description: 进行协议分析
********************************************
*/
#include "../GSPObject.h"
#include "../List.h"
#include "../GSPMemory.h"
#include "../MediaInfo.h"
#include "RtspParser.h"



namespace GSP
{
	namespace RTSP
	{
		/*
		*********************************************************************
		*
		*@brief : class CRtspIPacket
		*
		*********************************************************************
		*/

		class CRtspProPacket : public CProPacket
		{	

		public :
			BOOL m_bRequest;
			CRtspHeader m_csHeader;		
			BOOL m_bPacketed;
			EnumContentType m_eContentType;
			UINT m_iContentLenght;
			CGSString m_strContent;
			BYTE *m_pBinContent;
		private :
			typedef enum
			{
				ePARSER_STATUS_BEGIN, //分析首行
				ePARSER_STATUS_HEADER, //分析头
				ePARSER_STATUS_CONTENT, //分析内容
				ePARSER_STATUS_END, //结束
			}EnumParserStatus;
			EnumParserStatus m_eParserStatus;
			char m_czLineCache[MAX_RTSP_PROTOCOL_LINE_LEN];
			UINT m_iWLinePos;  //写 m_czLineCahce 的位置, 或接收的内容长度

		public :

			INLINE static  CRtspProPacket *Create(void)
			{
				return new CRtspProPacket();
			}

			static CRtspProPacket *Create( const CRtspHeader &csHeader );

			/*
			*********************************************
			Function : Analyse
			Version : 1.0.0.1
			Author : 邹阳星
			DateTime : 2013/4/12 9:49
			Description :  分析数据
			Input :  ppData 被分析的数据， 出入参数， 返回使用后的指针
			Input :  iSize 数据长度 ， 出入参数， 返回使用后的大小
			Output : 
			Return : 返回 				
			Note :   
			*********************************************
			*/
			EnumErrno Analyse( const BYTE **ppData, UINT &iSize );

			/*
			*********************************************
			Function : AnalyseResult
			Version : 1.0.0.1
			Author : 邹阳星
			DateTime : 2013/4/12 9:53
			Description :  分析结果
			Input :  
			Output : 
			Return :  0 分析完成，已经是完整包,
			-1 没有数据
			-2 没有完整头
			-3 数据版本不对
			-4 效验不对
			-5 数据长度不对
			> 0 还需要的数据长度
			Note :   
			*********************************************
			*/			
			INT AnalyseResult(void);

			/*
			*********************************************
			Function : AnalyseReset
			Version : 1.0.0.1
			Author : 邹阳星
			DateTime : 2013/4/12 9:56
			Description :  重置
			Input :  
			Output : 
			Return : 
			Note :   
			*********************************************
			*/
			void Reset(void);


						
			BOOL SetHeader( const CRtspHeader &csHeader );
			BOOL SetContent( const CGSString &strContent );
			BOOL SetContent(const unsigned char *pContent, INT iSize, 
				BOOL bBase64 = TRUE);	


			UINT GetContentLenght(void) const
			{
				return m_iContentLenght;
			}

			INLINE CGSString &GetStrContent(void)
			{
				return m_strContent;
			}

			INLINE BYTE *GetBinContent(void)
			{
				return m_pBinContent;
			}

			virtual CRefObject *Clone(void) const
			{
				return NULL;
			}

			INLINE CRtspHeader &GetHeader( void )
			{
				return m_csHeader;
			}

			BOOL Packet(void);
		protected :
			CRtspProPacket(void);
			virtual ~CRtspProPacket(void);	
		private :
			EnumErrno ParserHeader(const BYTE **pInput, UINT &iSize);
			EnumErrno ParserContent(const BYTE **pInput, UINT &iSize);
		};

		//一个协议包等于一帧
		class CGspRtspFrame : public CProFrame
		{
		public :			
			static CGspRtspFrame *Create(void);
			virtual CRefObject *Clone(void) const
			{
				return NULL;
			}
		protected :
			CGspRtspFrame(void);
			virtual ~CGspRtspFrame(void);
		};
		

		/*
		*********************************************************************
		*
		*@brief : 
		*
		*********************************************************************
		*/


		class CRtspTcpDecoder : public CGSPObject
		{

		private :			
			

		public :
			CRtspTcpDecoder(BOOL bRequest);
			virtual ~CRtspTcpDecoder(void);

			EnumErrno Decode( CGSPBuffer *pInBuffer );
			CRtspProPacket *Pop( void );
			void Reset(void);
			void Clear(void);
		private :
			//检查并去除效验数据			

		};

		EnumErrno UrlString2MediaInfo( CMediaInfo &csResult, const CGSString &strDescri);
		EnumErrno MediaInfo2UrlString( CGSString &strDescri,const CMediaInfo &csResult);

	} // end namespace RTSP
} // end namespace GSP



#endif //end _GS_H_RTSPANALYER_H_
