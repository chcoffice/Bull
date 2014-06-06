/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : CACHEBAK.H
Author :  zouyx
Version : 0.0.0.0
Date: 2011/9/8 15:51
Description: 
********************************************
*/

#ifndef _GS_H_CACHEBAK_H_
#define _GS_H_CACHEBAK_H_

#include "JouModule.h"
#include "DBMng.h"
#include <list>

namespace JOU
{

class CCacheBak :
	public CJouModule
{
private :
#define MAX_SQLITE_HANDLE 1
	void *m_pHCache[MAX_SQLITE_HANDLE];
	CGSWRMutex m_csMutex;	
	CGSThread m_csWatcher; //守护线程
	CGSCondEx m_csWCond;  //写线程
	CGSMutex m_csWMutex;
	BOOL m_bNormal;


	typedef std::list<StruJournalInfo*> CListJouInfoCache;
	CListJouInfoCache m_listCache;
	CListJouInfoCache m_listMemCache;
public:
	CCacheBak(void);
	~CCacheBak(void);

	virtual EnumJouErrno Init( CService *pServer );
	virtual void Uninit(void);

	virtual EnumJouErrno Start(void* pData);
	virtual void Stop(void);


// 	EnumJouErrno Add(const CGSString &stSql,
// 		INT32 iCliPmsID,INT32 iCliID, INT32 iCmdTag);

	EnumJouErrno JouUpdateOperation( const StruJouOperationUpdate *pData );

	EnumJouErrno Add(const StruJournalInfo &stInfo );
	
private :
	#define  eJOU_PRI_OPER_UPDATE ((EnumJournalType)-1)

	void WatchEntry(void);

	EnumJouErrno TestAndCreateSqliteDB(INT iHIdx,const CGSString &strDBName, 
						const CGSString &strTableName, 
						const CGSString &strCreateTableSql );

	EnumJouErrno ToMDBJouSql( CGSString &oStrSql, const StruJournalInfo &stInfo); //生成写入数据库的SQL

	EnumJouErrno ToSqliteJouSql( CGSString &oStrSql, const StruJournalInfo &stInfo); //生成写入数据库的SQL

	static void ThreadCallback(CGSThread *gsThreadHandle,void *pParam );

	//同步内存数据
	EnumJouErrno WriteJouToMDB( IConnection *pCnn,  CListJouInfoCache &listValue  );
	void WriteJouToLocalSqlite( CListJouInfoCache &listValue );


	StruJournalInfo* CreateJouInfo(void);
	void FreeJouInfo(StruJournalInfo*pInfo);

	EnumJouErrno LoadJouInfoFromLocalSqlite( CListJouInfoCache &listValue  );
	EnumJouErrno DeleteJouToForSqlite( CListJouInfoCache &listValue);
};



} //end namespace JOU

#endif //end _GS_H_CACHEBAK_H_
