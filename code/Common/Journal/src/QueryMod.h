/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : QUERYMOD.H
Author :  zouyx
Version : 0.0.0.0
Date: 2011/9/15 17:48
Description: 日志查询模块
********************************************
*/

#ifndef _GS_H_QUERYMOD_H_
#define _GS_H_QUERYMOD_H_

#include "JouModule.h"

namespace JOU
{



class CQueryMod :
	public CJouModule
{
public:
	CQueryMod(void);
	virtual ~CQueryMod(void);

	virtual EnumJouErrno Start(void* pData);
	virtual void Stop(void);

	EnumJouErrno  JouAsyncQuery( const StruQueryArgs *pArgs );

	EnumJouErrno  JouSetAsyncQueryCallback( JouFuncPtrAsyncQueryCallback fnCallback, 
		void *pUserContent );
private :
	static  void OnThreadPoolEvent( CGSThreadPool *pcsPool, 
									void *TaskData, 
									void *pDebugInfo);
	void TaskEntry( StruQueryArgs *pArgs);
	CGSThreadPool m_csPool; //人员线程池
	JouFuncPtrAsyncQueryCallback *m_fnCallback;
	void *m_pUserContent;
};



} //end namespace JOU

#endif //end _GS_H_QUERYMOD_H_