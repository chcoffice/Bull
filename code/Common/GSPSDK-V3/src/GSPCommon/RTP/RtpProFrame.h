/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : RTPPROFRAME.H
Author :  邹阳星
Version : 1.0.0.1
Date: 2013/4/3 9:17
Description: RTP 协议帧
********************************************
*/

#ifndef _GS_H_RTPPROFRAME_H_
#define _GS_H_RTPPROFRAME_H_

#include "../GSPMemory.h"
#include "RtpStru.h"
#include "RTPPacket.h"

namespace GSP
{


	namespace RTP
	{

		class CRtpProPacket : public CProPacket
		{
		public :		
			static const UINT iDEFAULT_PACKET_SIZE = 1400;
		private :
			//最大的包长度
			BYTE *m_bPriBuffer;
			UINT m_iPriMaxBuf;
			CRtpHeader m_csRtpHeader;
			UINT m_iPriBufDataSize;
			CGSPBuffer *m_pRefBuf;
		public :
			static CRtpProPacket *Create( const BYTE *pPacket, int iPacketLen );
			static CRtpProPacket *Create( CGSPBuffer *pBuf );

		//	static CRtpProPacket *Create( const StruRTPHeader &stHeader );
			static CRtpProPacket *Create( const CRtpHeader &csHeader , UINT iMaxBuffer);
			//出错返回 -1， 其他返回使用的 数据大小, < iSize 表示缓冲区已经满了
			INT MaxAppendPlayload(const BYTE *pPlayload, UINT iSize );

			INLINE CRtpHeader &GetHeader(void)
			{
				return m_csRtpHeader;
			}
		protected :
			CRtpProPacket(void);
			virtual ~CRtpProPacket(void);
		};


		class CRtpProFrame :
			public CProFrame
		{
		public:
			
			// bEnableMark 非 TRUE 每帧结束 把 iMark 设为 1
			static CRtpProFrame *Create(CFrameCache *pFrameData, 
									CRtpHeader &csHeader, BOOL bEnableMark = TRUE);
			static CRtpProFrame *Create(void)
			{
				return new CRtpProFrame();
			}


			CRtpHeader *GetFirstHeader(void);
		private :
			CRtpProFrame(void);
			virtual ~CRtpProFrame(void);
		};
	}
} //end namespace GSP

#endif //end _GS_H_RTPPROFRAME_H_
