/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : STREAMPACKDECODER.H
Author :  邹阳星
Version : 1.0.0.1
Date: 2013/5/8 10:12
Description: 把各个厂商的码率解析为标准裸码流
********************************************
*/

#ifndef _GS_H_STREAMPACKDECODER_H_
#define _GS_H_STREAMPACKDECODER_H_

#include "GSMediaDefs.h"
#include "MediaInfo.h"
#include "GSPMemory.h"
#include "List.h"
namespace GSP
{



class CStreamPackDecoder : public CGSPObject
{
public :
	//bOutFactorStream 输出厂商码流， 否则输出标准码流
	virtual EnumErrno Init( BOOL bOutFactorStream ) = 0;
	//bExistGSFH 存在 GSFameHeader
	virtual EnumErrno Decode( CFrameCache *pFrame, BOOL bExistGSFH ) = 0;
	virtual CFrameCache *Get(void) = 0;
	virtual void BindChannelInfo( UINT iChnNo, EnumGSMediaType eMType ) = 0;
	virtual void BindChannelInfo( const CMediaInfo &csMediaInfo ) = 0;
	virtual EnumGSCodeID GetCodeId( UINT iChnNo ) = 0;
	virtual EnumGSMediaType GetMeidaType(UINT iChnNo ) = 0;
	virtual ~CStreamPackDecoder(void)
	{

	}
	static CStreamPackDecoder *Make(EnumStreamPackingType eSrcPktType);
	


	static void ResigterExtDecoder(EnumStreamPackingType ePktType, 
									CStreamPackDecoder*(*CreateFunc)(EnumStreamPackingType ePktType) );
	
protected :
	CStreamPackDecoder(void) : CGSPObject()
	{

	}
};

/*
*********************************************************************
*
*@brief : CStPkDdBase CStreamPackDecoder 的基本实现
*
*********************************************************************
*/
class CStPkDdBase : public CStreamPackDecoder
{
protected :
	CList m_listFrameCache; // 存储 CFrameCache * 分析完的结果
	typedef struct _StruInfoCtx
	{
		EnumGSMediaType eMType;
		EnumGSCodeID eCodeId;
		UINT iChnNo;
	}StruInfoCtx;
	StruInfoCtx m_vInfoCtx[GSP_MAX_MEDIA_CHANNELS];
	EnumStreamPackingType m_eSrcPktType;
	BOOL m_bOutFactorStream;
public :

	virtual EnumErrno Init( BOOL bOutFactorStream );
	virtual CFrameCache *Get(void);
	virtual void BindChannelInfo( UINT iChnNo, EnumGSMediaType eMType);

	virtual void BindChannelInfo( const CMediaInfo &csMediaInfo );
	virtual EnumGSCodeID GetCodeId( UINT iChnNo );

	virtual EnumGSMediaType GetMeidaType(UINT iChnNo );
protected :
	CStPkDdBase(EnumStreamPackingType eSrcPktType);
	virtual ~CStPkDdBase(void);
	UINT16 GetMediaChannel(EnumGSMediaType eMType );
	void SetMediaCodeId(EnumGSMediaType eMType, EnumGSCodeID eCodeId );
};

} //end namespace GSP

#endif //end _GS_H_STREAMPACKDECODER_H_