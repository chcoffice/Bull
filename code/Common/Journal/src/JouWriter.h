/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : JOUWRITER.H
Author :  zouyx
Version : 0.0.0.0
Date: 2011/9/15 17:40
Description: 日志写入模块, 处理日志的写入
********************************************
*/

#ifndef _GS_H_JOUWRITER_H_
#define _GS_H_JOUWRITER_H_
#include "JouModule.h"

namespace JOU
{


class CJouWriter :
	public CJouModule
{
public:
	CJouWriter(void);
	virtual ~CJouWriter(void);

	virtual EnumJouErrno Start(void* pData);
	virtual void Stop(void);

	EnumJouErrno JouAdd( const StruJournalInfo *pLog);

	EnumJouErrno  JouDelete(  EnumJournalType eType, INT64 iKeyID);
	EnumJouErrno  JouDeleteExt( const char czFuncName[JOU_FUNC_NAME_LEN],
		const char czArgs[JOU_ARG_LEN] );
	EnumJouErrno  JouUpdateOperation( const StruJouOperationUpdate *pData );

};

} //end namespace GSP

#endif //end _GS_H_JOUWRITER_H_
