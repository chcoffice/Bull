/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : SERVICE.H
Author :  zouyx
Version : 0.0.0.0
Date: 2011/9/8 15:44
Description: 
********************************************
*/

#ifndef _GS_H_SERVICE_H_
#define _GS_H_SERVICE_H_

#include "JouObj.h"

#include <list>

#include "Config.h"
#include "Log.h"
#include "DBMng.h"
#include "JouWriter.h"
#include "RecordManager.h"
#include "QueryMod.h"
#include "CacheBak.h"

namespace JOU
{

class CService :
	public CJouObj
{
public:
	CService(void);
	virtual ~CService(void);


	EnumJouErrno Init( const char *czConfFilename,void* pConnectionPoolArgs);

	void Uninit(void);


	EnumJouErrno MakeFunctionSql(CGSString &oStrSql, 
							const CGSString &strFuncName, 
							const char czArgs[JOU_ARG_LEN] );
	
	

private :
	std::list<CJouModule *> m_vMoudles;

	class CSqlFunction
	{
	public :
		CSqlFunction(void)
			:m_vBase()
			,m_strFunSql()
		{

		}
		CSqlFunction(const CSqlFunction &csDest )
			:m_vBase()
		{
			m_vBase = csDest.m_vBase;
			m_strFunSql = csDest.m_strFunSql;
		}	
		~CSqlFunction(void)
		{

		}

		EnumJouErrno Make( const char czArgs[JOU_ARG_LEN], CGSString &oStrVal);
		BOOL ParserFunc( const CGSString &strFuncSql);
	private :		
		std::vector<CGSString> m_vBase;
		CGSString m_strFunSql;
	};
	std::map<CGSString, CSqlFunction *> m_csFuncSet;
	CGSWRMutex m_csWRMutex;
public :
	CLog m_csLog;
	CConfig m_csCfg;
	

	CDBManager m_csDB;
	CJouWriter m_csWriter;
	CRecordManager m_csRcdMng;
	CQueryMod m_csQuery;
	CCacheBak m_csCache;

	

};

} //end namespace JOU

#endif //end _GS_H_SERVICE_H_
