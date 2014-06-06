/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : JOUMODULE.H
Author :  zouyx
Version : 0.0.0.0
Date: 2011/9/15 15:04
Description: 
********************************************
*/

#ifndef _GS_H_JOUMODULE_H_
#define _GS_H_JOUMODULE_H_
#include "JouObj.h"
#include <string>



namespace JOU
{

class CService;

typedef std::string STRING;

class CJouModule :
	public CJouObj
{
public:
	CJouModule( const STRING &strModuleName );
	virtual ~CJouModule(void);

	virtual EnumJouErrno Init( CService *pServer );
	virtual void Uninit(void);

	virtual EnumJouErrno Start(void* pData);
	virtual void Stop(void);

	INLINE STRING &GetModuleName(void)
	{
		return m_strModuleName;
	}

protected :
	//状态定义
	typedef enum
	{
		eST_RUNNING = 0, //运行状态
		eST_UNINIT,  //未初始化
		eST_INIT,   //初始化
	} EnumStatus;

	CService *m_pSrv;
	STRING m_strModuleName;

	EnumStatus m_eStatus; //状态
};


} //end namespace JOU

#endif //end _GS_H_JOUMODULE_H_
