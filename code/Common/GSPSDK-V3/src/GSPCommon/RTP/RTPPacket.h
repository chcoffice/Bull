/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : RTPPACKET.H
Author :  邹阳星
Version : 1.0.0.1
Date: 2013/4/10 16:46
Description: 
********************************************
*/

#ifndef _GS_H_RTPPACKET_H_
#define _GS_H_RTPPACKET_H_

#include "../GSPObject.h"
#include "RtpStru.h"
#include "../OSSocket.h"

namespace GSP
{

namespace RTP
{


class CRtpHeader : CGSPObject
{
private :
	union
	{
		BYTE bBuffer[MAX_RTP_HEADER_SIZE];
		StruRTPHeader stHeader;
	}m_uData;

public :
	CRtpHeader(void);
	~CRtpHeader(void);
	 
	 //返回头使用的数据大小
	 INLINE INT Init(const BYTE *pData, UINT iLength )
	 {
		Init();
		memcpy(m_uData.bBuffer, pData, MIN(iLength, MAX_RTP_HEADER_SIZE));
		return GetHeaderSize();
	 }

	 INLINE INT Init(const StruRTPHeader &stHeader  )
	 {
		 Init();
		 memcpy(m_uData.bBuffer,  &stHeader, MIN(sizeof(stHeader), MAX_RTP_HEADER_SIZE));
		 return GetHeaderSize();
	 }


	 INLINE void Init(void)
	 {
		 bzero(&m_uData, sizeof(m_uData));
		 INIT_DEFAULT_RTH_HEADER(&m_uData.stHeader);
		 SetVersion(RTP_VERSION);
		 SetPadding(FALSE);
		 SetExtension(FALSE);
		 SetContribSrcCount(0);		
	 }

	 INLINE const BYTE *GetHeaderBuffer(void)
	 {
		 return m_uData.bBuffer;
	 }

	 INLINE  UINT GetHeaderSize(void) const
	 {
		 if(  GetExtension() )
		 {
			return  (12 + 4 * GetContribSrcCount() + ( 4 * GetExtensionSize() + 4) );
		 }
		 return (12 + 4 * GetContribSrcCount());		 
	 }


	 INLINE INT	GetVersion(void) const
	 {
			return m_uData.stHeader.iVer;
	 }

	 INLINE void SetVersion(int iVersion)
	 {
		 m_uData.stHeader.iVer = iVersion&0x3;
	 }

	 // 填充标识
	 INLINE BOOL GetPadding(void) const
	 {
		return m_uData.stHeader.iP; 
	 }

	 INLINE void SetPadding(BOOL bPadding)
	 {
		 m_uData.stHeader.iP = bPadding ? 1 : 0; 
	 }

	 // 头部扩展标识
	 INLINE BOOL GetExtension(void) const
	 {
			return m_uData.stHeader.iX; 
	 }

	 INLINE  void	SetExtension(BOOL bExtension)
	 {
		 m_uData.stHeader.iX = bExtension ? 1 : 0;
	 }

	 // CSRC数量
	 INLINE INT	GetContribSrcCount(void) const
	 {
		  return m_uData.stHeader.iCC; 
	 }

	 INLINE void SetContribSrcCount(int iCSRCNum)
	 {
		m_uData.stHeader.iCC = iCSRCNum&0xF;
	 }

	 // mark标识，一帧数据最后一个包为1
	 INLINE BOOL GetMarker(void) const
	 {
		return m_uData.stHeader.iM;
	 }

	 INLINE void SetMarker(BOOL bMarker)
	 {
		 m_uData.stHeader.iM = bMarker ? 1 : 0;
	 }

	 // 负载类型，自定义从96开始
	 INLINE INT GetPayloadType(void) const
	 {
		 return m_uData.stHeader.iPT;
	 }

	 INLINE void	SetPayloadType(int iType)
	 {
			m_uData.stHeader.iPT = iType&0x7F;
	 }

	 // 包序号
	 INLINE UINT16 GetSequenceNumber(void) const
	 {
		 return COSSocket::Int16N2H(m_uData.stHeader.iSeq);
	 }

	 INLINE void SetSequenceNumber(UINT16 iSequenceNum)
	 {
		m_uData.stHeader.iSeq = COSSocket::Int16H2N(iSequenceNum);
	 }

	 // 时间戳
	 INLINE  UINT32		GetTimestamp(void) const
	 {
			 return COSSocket::Int32N2H(m_uData.stHeader.iTS);
	 }

	 INLINE void	SetTimestamp(UINT32 iTimestamp)
	 {
		m_uData.stHeader.iTS = COSSocket::Int32H2N(iTimestamp);
	 }

	 // 同步源SSRC
	 INLINE UINT32	GetSyncSource(void) const
	 {
		 UINT32 iV;
		 ::memcpy(&iV, &m_uData.bBuffer[8], sizeof(iV));
		return COSSocket::Int32N2H(iV);
	 }

	 INLINE void	SetSyncSource(UINT32 iSSRC)
	 {
		 iSSRC = COSSocket::Int32H2N(iSSRC);
		 ::memcpy(&m_uData.bBuffer[8], &iSSRC, sizeof(iSSRC) );
	 }

	 // 贡献源CSRC
	 INLINE UINT32	GetContribSource(int iIndex) const
	 {
		 UINT32 iV;
		 ::memcpy(&iV, &m_uData.bBuffer[12+iIndex*sizeof(iV)], sizeof(iV));
		 return COSSocket::Int32N2H(iV);
	 }

	 INLINE void	SetContribSource(int iIndex, UINT32 iSSRC)
	 {
		 if( GetContribSrcCount() > iIndex )
		 {
			 iSSRC = COSSocket::Int32H2N(iSSRC);
			 ::memcpy( &m_uData.bBuffer[12+iIndex*sizeof(iSSRC)],  &iSSRC, sizeof(iSSRC));
		 }
	 }

	 // 头部扩展
	 INLINE UINT16	GetExtensionType(void ) const
	 {
		  if( GetExtension() )
		  {
			  UINT16 iV;
			  ::memcpy( &iV, &m_uData.bBuffer[12 + 4 * GetContribSrcCount()], sizeof(iV));
			  return COSSocket::Int16N2H(iV);
		  }
		  return MAX_UINT16;		
	 }

	 INLINE void	SetExtensionType(UINT16 iType)
	 {
		 if( GetExtension() )
		 {
			 iType = COSSocket::Int16H2N(iType);
			 ::memcpy( &m_uData.bBuffer[12 + 4 * GetContribSrcCount()],&iType, sizeof(iType));			
		 }
	 }

	 INLINE UINT16	GetExtensionSize(void) const
	 {
		 if( GetExtension() )
		 {
			 UINT16 iV;
			 ::memcpy( &iV, &m_uData.bBuffer[12 + 4 * GetContribSrcCount() + 2], sizeof(iV));
			 return COSSocket::Int16N2H(iV)*4;
		 }
		 return MAX_UINT16;		
	 }

	 INLINE void	SetExtensionSize(UINT16 iSize)
	 {
		 if( GetExtension() )
		 {
			 iSize = COSSocket::Int16H2N(iSize);
			 ::memcpy( &m_uData.bBuffer[12 + 4 * GetContribSrcCount()+2],&iSize, sizeof(iSize));			
		 }
	 }

	 INLINE const BYTE *GetExtensionHead(void) const
	 {
		 if( GetExtension() )
		 {
			return &m_uData.bBuffer[12 + 4 * GetContribSrcCount()+4];
		 }
		 return NULL;
	 }

	INLINE  void  SetExtensionHead(const BYTE* pData, int iLen)
	 {
		 
		 if( GetExtension()  && (12 + 4 * GetContribSrcCount()+4+iLen) <= MAX_RTP_HEADER_SIZE )
		 {
			 ::memcpy( &m_uData.bBuffer[12 + 4 * GetContribSrcCount()+4] , pData, iLen);
			 SetExtensionSize(iLen/4);
		 }		
	 }
};

} //end namespace RTP

} //end namespace GSP

#endif //end _GS_H_RTPPACKET_H_
