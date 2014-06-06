/*
******************************************
Copyright (C), 2010-2011, GOSUN
Filename : GSPPRODEBUG.H
Author :  zouyx
Version : 0.1.0.0
Date: 2010/9/15 11:22
Description: gsp 协议的描述打印
********************************************
*/
#ifndef GSP_GSPPRODEBUG_DEF_H
#define GSP_GSPPRODEBUG_DEF_H
#include "GSPStru.h"
#include "GSMediaDefs.h"
#include "GSPConfig.h"
#include "GSPErrno.h"


namespace GSP
{

extern const char *GSPCommandName(EnumGSPCommandID eCmdID);

extern const char *GSPError(INT iGSPErrno);

extern const char *GSPCtrlName( INT iCtrlID);

extern const char *GetMediaName( EnumGSMediaType eType);

extern const char *GetError( EnumErrno eErrno );

extern  const char *GetProtocolName( EnumProtocol ePro );

extern   EnumProtocol GetProtocolType( const char *strName);


};

#endif
