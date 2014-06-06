/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : RTPANALYER.H
Author :  邹阳星
Version : 1.0.0.1
Date: 2013/5/7 15:27
Description: RTP 包解析、分析器
********************************************
*/

#ifndef _GS_H_RTPANALYER_H_
#define _GS_H_RTPANALYER_H_
#include "../GSPMemory.h"
#include "../List.h"
#include "RtpProFrame.h"

namespace GSP
{

namespace RTP
{

	typedef struct _StruRtpPktSn
	{
		UINT8 iPT;   /* payload type */	
		UINT16 iSeq;      /* sequence number */
		UINT32 iSSRC;     /* synchronization source */
	}StruRtpPktSn;


	

class CRtpPacketMerger : public CGSPObject
{
private :
	typedef struct _StruCore
	{	
		//	RTSP::StruRTPHeader stRtpHeader; //发送数据的Rtp头			
		CList lRcvPacket; //接受到得数据 存储对象为 // CRtpProPacket *
		UINT64 iLastTs; //最后失效帧的时间
		UINT32 iLastSeq; //最后一个SEQ
		UINT32 iPt; //Playload 类型
		UINT32 iSSRC;




		void Init( FuncPtrFree fnFreeRcvPacket)
		{

			//INIT_DEFAULT_RTH_HEADER(&stRtpHeader);			
			lRcvPacket.Clear();
			iLastTs = MAX_UINT64;				
			iPt = MAX_UINT32;
			iLastSeq = MAX_UINT32;
			iSSRC = MAX_UINT32;

			if( fnFreeRcvPacket )
			{
				lRcvPacket.SetFreeCallback(fnFreeRcvPacket);
			}

		}
		void Clear(void)
		{				
			lRcvPacket.Clear();
			iLastTs = MAX_UINT64;				
			iPt = MAX_UINT32;
			iLastSeq = MAX_UINT32;
		}
	}StruCore;

	CList m_listFinish; //完成的帧 CRtpProFrame *
	StruCore m_vChannel[GSP_MAX_MEDIA_CHANNELS]; //RTP 头使用的数据

	
public :
	CRtpPacketMerger(void);
	virtual ~CRtpPacketMerger(void);

	//有数据返回 eERRNO_SUCCESS 
	// 返回 eERRNO_SUCCESS 调用 Get 得到完整的正数据
	// 返回 eERRNO_SYS_ELOSE 调用 vLoser 返回丢失的信息
	EnumErrno Rcv( CGSPBuffer *pBuffer, std::vector<StruRtpPktSn> &vLoser ); //添加数据包

	INLINE CRtpProFrame *Get(void)
	{
		void *p = NULL;
		m_listFinish.RemoveFront(&p);
		return (CRtpProFrame *)p;
	}
};


class CRtpDecoder : public CGSPObject
{
private :	
	CRtpPacketMerger m_csMeger;
public :
	CRtpDecoder(void);
	virtual ~CRtpDecoder(void);
	EnumErrno Decode( CGSPBuffer *pBuffer, std::vector<StruRtpPktSn> &vLoser ); //添加数据包

	/*
	媒体信息中的
	iChn 为RTP 的 PT
	TimeStamp 为  RTP 的 timestamp
	eMediaType 为 RTP SSRC
	*/
	CFrameCache *Get(void);
};

} //end namespace RTP

} //end namespace GSP

#endif //end _GS_H_RTPANALYER_H_