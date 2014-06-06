/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : STREAMPACKENCODER.H
Author :  邹阳星
Version : 1.0.0.1
Date: 2013/5/8 15:35
Description: 流封装格式打包
********************************************
*/

#ifndef _GS_H_STREAMPACKENCODER_H_
#define _GS_H_STREAMPACKENCODER_H_



#include "GSMediaDefs.h"
#include "MediaInfo.h"
#include "GSPMemory.h"

namespace GSP
{



class CStreamPackEncoder : public CGSPObject
{
public :
	// bInsertGSFH 输出增加 GSFameHeader 头
	virtual EnumErrno Init( BOOL bInsertGSFH ) = 0;
	//bExistGSFH 存在 GSFameHeader, 输入纯码流
	virtual EnumErrno Encode( CFrameCache *pFrame, BOOL bExistGSFH ) = 0;
	
	virtual CFrameCache *Get(void) = 0;
	virtual void BindChannelInfo( UINT iChnNo, EnumGSMediaType eMType,
								  EnumGSCodeID eGSCodeId, float fFrameRate ) = 0;
	virtual void BindChannelInfo( const CMediaInfo &csMediaInfo ) = 0;
	virtual EnumGSCodeID GetCodeId( UINT iChnNo ) = 0;
	virtual EnumGSMediaType GetMeidaType(UINT iChnNo ) = 0;
	virtual ~CStreamPackEncoder(void)
	{

	}

	static CStreamPackEncoder *Make(EnumStreamPackingType eDestPktType);
protected :
	CStreamPackEncoder(void) : CGSPObject()
	{

	}
};



} //end namespace GSP

#endif //end _GS_H_STREAMPACKENCODER_H_
