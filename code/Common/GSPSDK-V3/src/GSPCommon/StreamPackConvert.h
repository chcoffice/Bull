/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : STREAMPACKCONVERT.H
Author :  邹阳星
Version : 1.0.0.1
Date: 2013/5/8 16:59
Description: 流封装转换器
********************************************
*/

#ifndef _GS_H_STREAMPACKCONVERT_H_
#define _GS_H_STREAMPACKCONVERT_H_




#include "GSMediaDefs.h"
#include "MediaInfo.h"
#include "GSPMemory.h"

namespace GSP
{


class CStreamPackConvert : public CGSPObject
{
public :
	// bInsertGSFH 输出增加 GSFameHeader 头
	
	//bExistGSFH 存在 GSFameHeader, 输入纯码流
	virtual EnumErrno Conver( CFrameCache *pFrame, BOOL bExistGSFH ) = 0;
	
	virtual EnumStreamPackingType GetSrcPackType( void) const = 0;
	virtual EnumStreamPackingType GetDestPackType( void) const = 0;
	
	virtual CFrameCache *Get(void) = 0;	
	virtual EnumGSCodeID GetCodeId( UINT iChnNo ) = 0;
	virtual EnumGSMediaType GetMeidaType(UINT iChnNo ) = 0;
	virtual ~CStreamPackConvert(void)
	{

	}

	static CStreamPackConvert *Make( BOOL bRealPlay, BOOL bServer, EnumStreamPackingType eSrcPktType,
									EnumStreamPackingType eDestPktType, 
									const CMediaInfo &csSrcMediaInfo,BOOL bInsertGSFH);
protected :
	virtual EnumErrno Init( BOOL bInsertGSFH ) = 0;
	virtual void BindChannelInfo( const CMediaInfo &csSrcMediaInfo ) = 0;
	CStreamPackConvert(void) : CGSPObject()
	{

	}
};



} //end namespace GSP


#endif //end _GS_H_STREAMPACKCONVERT_H_
