/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : RECORDMANAGER.H
Author :  zouyx
Version : 0.0.0.0
Date: 2011/9/15 17:35
Description: 日志记录维护管理模块
********************************************
*/

#ifndef _GS_H_RECORDMANAGER_H_
#define _GS_H_RECORDMANAGER_H_

#include "JouModule.h"


namespace JOU
{



class CRecordManager :
	public CJouModule
{
public:
	CRecordManager(void);
	~CRecordManager(void);

	virtual EnumJouErrno Start(void* pData);
	virtual void Stop(void);



private :
	void WatchEntry(void);
	static void ThreadCallback(CGSThread *gsThreadHandle,void *pParam );

	void ExeDelDays(char *pBuf, INT iBuf, const char *czTableName,const char *czDataTime );

	//void ExeDelRows(char *pBuf, INT iBuf, const char *czTableName, unsigned long iLeftRow );

	void ExeDelSql( const char *czSql);

	CGSThread m_csWatcher; //守护线程

};

} //end namespace JOU

#endif //end _GS_H_RECORDMANAGER_H_