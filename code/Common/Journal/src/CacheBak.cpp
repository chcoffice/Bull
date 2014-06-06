#include "CacheBak.h"
#include "Service.h"
#include "sqlite3.h"

using namespace JOU;


/*
*********************************************************************
*
*@brief : 本地缓冲设计
*
缓冲表
表名 : tb_caches
表结构 :
id :  主键， AUTOINCREMENT
csql :  缓冲的SQL 语句,  长度 varchar(2048)
itv :   记录的时间 采用 time(NULL),     INT(4)
bupdate : 释放已经上层到数据库 0 没有更新， 1 正在更新， 2 已经更新,			INT(1）
*********************************************************************
*/

#define MAX_SQL_LEN  2048


#define CACHE_DB_VERSION "v2.db"



#define iCacheSqlIdx 0


#define hCacheSqlDB ((sqlite3 *)m_pHCache[iCacheSqlIdx])


/* 检测输入的内容是否正确 */

static BOOL CheckContent(char *pStr, int iMaxLen)
{
	unsigned char *p = (unsigned char *) pStr;
	int i = iMaxLen;
	i--; //保留最后结束符
	BOOL bRet = TRUE;
	while( *p != '\0' && i>0 )
	{		
		if( *p >= 0x80 )
		{
			p++;
			i--;
			if( i<1 || *p=='\0' || *p < 0x80  )
			{
				//出错
				p--;
				bRet = FALSE;
				break;
			}
			else
			{
				p++;
				i--;
			}
		}
		else 
		{
			// 			if( *p=='?' || *p == '%' )
			// 			{
			// 				//不能有 '?'
			// 				*p = '^';
			// 				bRet = FALSE;
			// 			}
			p++;
			i--;
		}
	}
	*p = '\0';
	return bRet;
}

/*
*********************************************************************
*
*@brief : 
*
*********************************************************************
*/

CCacheBak::CCacheBak(void)
:CJouModule("CacheBak")
,m_csMutex()
,m_csWatcher()
,m_csWCond()
{

	
	m_bNormal = FALSE;
	for( int i = 0; i<MAX_SQLITE_HANDLE; i++ )
	{
		m_pHCache[i] = NULL;
	}

}

CCacheBak::~CCacheBak(void)
{
	m_bNormal = FALSE;
	m_csWatcher.Stop();
	if( m_csWatcher.IsRunning() )
	{
		m_csWatcher.Join(500);
		MSLEEP(10);
	}	

	for( int i = 0; i<MAX_SQLITE_HANDLE; i++ )
	{
		if( m_pHCache[i] )
		{
			sqlite3_close((sqlite3 *)m_pHCache[i]);
			m_pHCache[i] = NULL;
		}
	}
	
}

EnumJouErrno CCacheBak::Init( CService *pServer )
{
	CGSAutoWriterMutex wlocker(&m_csMutex);


	EnumJouErrno eRet =  CJouModule::Init(pServer);
	GS_ASSERT_RET_VAL(!eRet , eRet);


	CGSString strDB =  pServer->m_csCfg.m_strCachePath;
	CGSString strSql;

	//建立本地数据和表


	strDB =  pServer->m_csCfg.m_strCachePath;
	strDB += "JouCacheSql";
	strDB += CACHE_DB_VERSION;


	//操作日志
	strSql = "CREATE TABLE tb_cache_oper ("
		"id INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
		"StartTime VARCHAR(64) DEFAULT NULL,"
		"CliPmsID INTEGER,"
		"CliID INTEGER,"	
		"CmdSectionID INTEGER,"
		"OperatorID VARCHAR(256) DEFAULT NULL,"
		"HostName VARCHAR(256) DEFAULT NULL,"
		"ClientType INTEGER,"
		"PmsID INTEGER,"
		"DevID INTEGER,"	
		"Chn INTEGER,"
		"ChnType INTEGER,"
		"Content VARCHAR(512) DEFAULT NULL,"
		"Result INTEGER,"
		"Failure VARCHAR(512) DEFAULT NULL,"	
		"bupdate INTEGER DEFAULT 0"
		")";
	eRet = TestAndCreateSqliteDB( iCacheSqlIdx, strDB,"tb_cache_oper", strSql );
	if( eRet )
	{
		GS_ASSERT(0);
		return eRet;
	}

	//操作日志更新
	strSql = "CREATE TABLE tb_cache_oper_update ("
		"id INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
		"EndTime VARCHAR(64) DEFAULT NULL,"
		"CliPmsID INTEGER,"
		"CliID INTEGER,"	
		"CmdSectionID INTEGER,"		
		"Result INTEGER,"
		"Failure VARCHAR(512) DEFAULT NULL,"	
		"bupdate INTEGER DEFAULT 0"
		")";
	eRet = TestAndCreateSqliteDB( iCacheSqlIdx, strDB,"tb_cache_oper_update", strSql );
	if( eRet )
	{
		GS_ASSERT(0);
		return eRet;
	}

	//运行状态日志
	strSql = "CREATE TABLE tb_cache_RunSt ("
		"id INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
		"StartTime VARCHAR(64) DEFAULT NULL,"
		"ClientType INTEGER,"
		"PmsID INTEGER,"
		"DevID INTEGER,"
		"ChnID INTEGER,"
		"ChnType INTEGER,"
		"IsOnline INTEGER,"
		"Content VARCHAR(512) DEFAULT NULL,"	
		"Descri VARCHAR(512) DEFAULT NULL,"
		"bupdate INTEGER DEFAULT 0"
		")";
	eRet = TestAndCreateSqliteDB( iCacheSqlIdx, strDB,"tb_cache_RunSt", strSql );
	if( eRet )
	{
		GS_ASSERT(0);
		return eRet;
	}

	//登陆日志
	strSql = "CREATE TABLE tb_cache_Login ("
		"id INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
		"StartTime VARCHAR(64) DEFAULT NULL,"
		"OperatorID VARCHAR(256) DEFAULT NULL,"
		"HostName VARCHAR(256) DEFAULT NULL,"
		"Content VARCHAR(512) DEFAULT NULL,"	
		"Result INTEGER,"
		"Descri VARCHAR(512) DEFAULT NULL,"
		"Failure VARCHAR(512) DEFAULT NULL,"	
		"bupdate INTEGER DEFAULT 0"
		")";
	eRet = TestAndCreateSqliteDB( iCacheSqlIdx, strDB,"tb_cache_Login", strSql );
	if( eRet )
	{
		GS_ASSERT(0);
		return eRet;
	}
	m_bNormal = TRUE;
	return eJOU_R_SUCCESS;
}

#ifdef _WIN32
static WCHAR *mbcsToUnicode(const char *zFilename){
	int nByte;
	WCHAR *zMbcsFilename;
	int codepage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;

	nByte = MultiByteToWideChar(codepage, 0, zFilename, -1, NULL,0)*sizeof(WCHAR);
	zMbcsFilename = (WCHAR *)malloc( nByte*sizeof(zMbcsFilename[0]) );
	if( zMbcsFilename==0 ){
		return 0;
	}
	nByte = MultiByteToWideChar(codepage, 0, zFilename, -1, zMbcsFilename, nByte);
	if( nByte==0 ){
		free(zMbcsFilename);
		zMbcsFilename = 0;
	}
	return zMbcsFilename;
}

static char *unicodeToUtf8(const WCHAR *zWideFilename){
	int nByte;
	char *zFilename;

	nByte = WideCharToMultiByte(CP_UTF8, 0, zWideFilename, -1, 0, 0, 0, 0);

	zFilename = (char *)malloc( nByte );
	if( zFilename==0 ){
		return 0;
	}
	nByte = WideCharToMultiByte(CP_UTF8, 0, zWideFilename, -1, zFilename, nByte,
		0, 0);
	if( nByte == 0 ){
		free(zFilename);
		zFilename = 0;
	}
	return zFilename;
}


static CGSString mbcs_to_utf8(const char *zFilename){
	char *zFilenameUtf8;
	WCHAR *zTmpWide;

	zTmpWide = mbcsToUnicode(zFilename);
	if( zTmpWide==0 ){
		return CGSString();
	}
	zFilenameUtf8 = unicodeToUtf8(zTmpWide);
	free(zTmpWide);
	CGSString strUtf(zFilenameUtf8);
	free(zFilenameUtf8);
	return strUtf;
}
#endif

EnumJouErrno CCacheBak::TestAndCreateSqliteDB(INT iHIdx,const CGSString &strDBName, 
											  const CGSString &strTableName, 
											  const CGSString &strCreateTableSql )
{

	sqlite3 *pDb = NULL;
	int iRet;
#ifdef _WIN32
	// 把文件名转换为UTF8,sqlite3_open 只接受UTF8 文件名, 否则在有中文时出错误
	CGSString strUtf8 = mbcs_to_utf8(strDBName.c_str() );

	iRet  = sqlite3_open( strUtf8.c_str() , &pDb);
#else
	iRet  = sqlite3_open( strDBName.c_str() , &pDb);
#endif

	if( iRet!=SQLITE_OK )
	{
		GS_ASSERT(0);
		m_eStatus = eST_UNINIT;
		return eJOU_E_DB_GETCONN;
	}
	//sqlite3_exec(pDb, "PRAGMA synchronous=0;", NULL, NULL, NULL );
	//sqlite3_exec(pDb, "PRAGMA vdbe_trace=false;", NULL, NULL, NULL );
	//sqlite3_exec(pDb, "PRAGMA fullfsync=false;", NULL, NULL, NULL );


	CGSString strSql;

	GSStrUtil::Format(strSql,"SELECT name FROM sqlite_master WHERE type='table' AND name='%s'",
		strTableName.c_str() );

	char **ppResult = NULL;
	char *pErrMsg = NULL;
	int nRow = 0, nCol = 0;
	iRet = sqlite3_get_table(pDb, strSql.c_str() ,&ppResult,&nRow,&nCol, &pErrMsg);
	if( iRet!=SQLITE_OK ) {
		MY_LOG_ERROR(g_pLog, "Sqlite exe: '%s' failure. %s\n",strSql.c_str(), pErrMsg);
		::sqlite3_free(pErrMsg);
		m_eStatus = eST_UNINIT;
		::sqlite3_close(pDb);		
		return eJOU_E_DB_EXESQL;
	}
	sqlite3_free_table(ppResult);

	if( nCol == 1 )
	{
		//表已经存储在
	}
	else
	{
		//建立新表
		iRet = sqlite3_exec(pDb, strCreateTableSql.c_str(), NULL, NULL,  &pErrMsg );
		if( iRet!=SQLITE_OK ) {
			MY_LOG_ERROR(g_pLog, "Sqlite exe: '%s' failure. %s\n",
				strCreateTableSql.c_str(), pErrMsg);
			::sqlite3_free(pErrMsg);
			m_eStatus = eST_UNINIT;
			::sqlite3_close(pDb);

			return eJOU_E_DB_EXESQL;
		}
	}
	m_pHCache[iHIdx] = pDb;	
	return eJOU_R_SUCCESS;


}

void CCacheBak:: Uninit(void)
{
	m_bNormal = FALSE;
	CJouModule::Uninit();

	for( int i = 0; i<MAX_SQLITE_HANDLE; i++ )
	{
		if( m_pHCache[i] )
		{
			//sqlite3_exec((sqlite3 *)m_pHCache[i], "", NULL, NULL, NULL);
			//sqlite3_exec((sqlite3 *)m_pHCache[i], "END;", NULL, NULL, NULL );
			sqlite3_close((sqlite3 *)m_pHCache[i]);
			m_pHCache[i] = NULL;
		}
	}
	m_csWatcher.Stop();
}

EnumJouErrno CCacheBak::Start(void* pData)
{
	CGSAutoWriterMutex wlocker(&m_csMutex);
	if( m_eStatus == eST_RUNNING)
	{
		return eJOU_R_SUCCESS;
	}
	EnumJouErrno eRet = CJouModule::Start(pData);
	GS_ASSERT_RET_VAL(!eRet, eRet);
	if( !m_csWatcher.Start((GSThreadCallbackFunction)CCacheBak::ThreadCallback, this) )
	{
		m_eStatus = eST_INIT;		
		return eJOU_E_UNKNOWN;
	}
	return eJOU_R_SUCCESS;

}

void CCacheBak::Stop(void)
{
	m_csMutex.LockWrite();
	BOOL bStart = m_eStatus==eST_RUNNING;

	CJouModule::Stop();

	m_csWCond.Signal();

	m_csMutex.UnlockWrite();
	MSLEEP(1);
	m_csWMutex.Lock();	
	m_csWCond.BroadcastSignal();
	m_csWMutex.Unlock();
	if( bStart )
	{
		m_csWatcher.Join(3000);
		MSLEEP(10);
	}
	//把数据写入本地缓冲	
}

static std::string CnvSpChar( const char *p  ) //ConvertSpecialChar
{
	std::string strRet = "";
	if( p )
	{
		strRet.clear();
		while( *p!='\0' )
		{
			if( *p=='\'')
			{
				strRet += "'";
			}
			strRet += *p;
			p++;
		}
	}
	return strRet;
}

EnumJouErrno CCacheBak::ToSqliteJouSql( CGSString &oStrSql, const StruJournalInfo &stInfo)
{
	//转换为Sqlite 的SQL
	std::stringstream strStream;
	char czDateTime[20];


	//格式化时间
	GS_SNPRINTF(czDateTime,20, "%04d-%02d-%02d %02d:%02d:%02d",
		stInfo.stTime.iYear, stInfo.stTime.iMonth, stInfo.stTime.iDay,
		stInfo.stTime.iHour, stInfo.stTime.iMinute, stInfo.stTime.iSecond );

	switch( stInfo.eType )
	{
	case eJOU_TYPE_OPERATOR :
		{
			//操作日志

			//"CREATE TABLE tb_cache_oper ("
			//	"id INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
			//	"StartTime VARCHAR(64) DEFAULT NULL,"
			//	"CliPmsID INTEGER,"
			//	"CliID INTEGER,"	
			//	"CmdSectionID INTEGER,"
			//	"OperatorID VARCHAR(256) DEFAULT NULL,"
			//  "ClientType INTEGER,"
			//	"HostName VARCHAR(256) DEFAULT NULL,"
			//	"PmsID INTEGER,"
			//	"DevID INTEGER,"	
			//	"Chn INTEGER,"
			//	"ChnType INTEGER,"
			//	"Content VARCHAR(512) DEFAULT NULL,"
			//	"Result INTEGER,"
			//	"Failure VARCHAR(512) DEFAULT NULL,"	
			//	"bupdate INTEGER DEFAULT 0"
			//	")";

			strStream << "INSERT INTO tb_cache_oper (";
			strStream <<"StartTime,CliPmsID,CliID,CmdSectionID,";
			strStream << "OperatorID,HostName,ClientType,";
			strStream << "PmsID,DevID,Chn,ChnType,Content,Result,Failure,";
		    strStream << "bupdate";
			strStream << ") VALUES (";

			strStream << "'" << czDateTime << "',";
			strStream << (int) stInfo.unLog.stOperation.iCliPmsID << ",";
			strStream << (int) stInfo.unLog.stOperation.iCliID << ",";
			strStream << (int) stInfo.unLog.stOperation.iCmdSectionID << ",";


			strStream << "'" << CnvSpChar(stInfo.unLog.stOperation.OperatorID) << "',";
			strStream << "'" <<  CnvSpChar(stInfo.unLog.stOperation.czHostName) << "',";
			strStream << (int) stInfo.unLog.stOperation.eClientType << ",";

			strStream << (int) stInfo.unLog.stOperation.iPmsID << ",";
			strStream << (int) stInfo.unLog.stOperation.iDevID << ",";
			strStream << (int) stInfo.unLog.stOperation.iChn << ",";
			strStream << (int) stInfo.unLog.stOperation.eChnType << ",";
			strStream << "'" << CnvSpChar(stInfo.unLog.stOperation.czContent) << "',";
			strStream << (int) stInfo.unLog.stOperation.eResult << ",";
			strStream << "'" << CnvSpChar(stInfo.unLog.stOperation.czFailure) << "',";

			strStream << (long)time(NULL);

			strStream << ")";
		}
		break;
	case eJOU_TYPE_RUNSTATUS :
		{
			//运行日志
			//"CREATE TABLE tb_cache_RunSt ("
			//	"id INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
			//	"StartTime VARCHAR(64) DEFAULT NULL,"
			//	"ClientType INTEGER,"
			//	"PmsID INTEGER,"
			//	"DevID INTEGER,"	
			//	"Content VARCHAR(512) DEFAULT NULL,"	
			//	"Descri VARCHAR(512) DEFAULT NULL,"
			//	"bupdate INTEGER DEFAULT 0"
			//	")";

			strStream << "INSERT INTO tb_cache_RunSt ";
			strStream << "(StartTime,ClientType,PmsID,DevID,ChnID,ChnType,IsOnline,Content,Descri,bupdate) VALUES (";
			strStream << "'" << czDateTime << "',";
			strStream << (int) stInfo.unLog.stRunStatus.eClientType << ",";
			strStream << (int) stInfo.unLog.stRunStatus.iPmsID << ",";
			strStream << (int) stInfo.unLog.stRunStatus.iDevID << ",";
			strStream << (int) stInfo.unLog.stRunStatus.iChnID << ",";
			strStream << (int) stInfo.unLog.stRunStatus.iChnType << ",";
			strStream << (int) stInfo.unLog.stRunStatus.iOnline << ",";
			strStream << "'" << CnvSpChar(stInfo.unLog.stRunStatus.czContent) << "',";
			strStream << "'" << CnvSpChar(stInfo.unLog.stRunStatus.czDescri) << "',";
			strStream << (long)time(NULL);
			strStream << ")";

		}
		break;
	case eJOU_TYPE_LOGIN :
		{
			//登陆日志
			//登陆日志
			//"CREATE TABLE tb_cache_Login ("
			//	"id INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
			//	"StartTime VARCHAR(64) DEFAULT NULL,"
			//	"OperatorID VARCHAR(256) DEFAULT NULL,"
			//	"HostName VARCHAR(256) DEFAULT NULL,"
			//	"Content VARCHAR(512) DEFAULT NULL,"	
			//	"Result INTEGER,"
			//	"Descri VARCHAR(512) DEFAULT NULL,"
			//	"Failure VARCHAR(512) DEFAULT NULL,"	
			//	"bupdate INTEGER DEFAULT 0"
			//	")";

			strStream << "INSERT INTO tb_cache_Login ";
			strStream << "(StartTime,OperatorID,HostName,Content,Result,Descri,Failure,bupdate)";
			strStream << " VALUES (";
			strStream << "'" << czDateTime << "',";
			strStream << "'" << CnvSpChar(stInfo.unLog.stLogin.OperatorID) << "',";
			strStream << "'" << CnvSpChar(stInfo.unLog.stLogin.czHostName) << "',";
			strStream << "'" << CnvSpChar(stInfo.unLog.stLogin.czContent) << "',";
			strStream << (int) stInfo.unLog.stLogin.eResult << ",";
			strStream << "'" << CnvSpChar(stInfo.unLog.stLogin.czDescri) << "',";
			strStream << "'" << CnvSpChar(stInfo.unLog.stLogin.czFailure) << "',";	
			strStream << (long)time(NULL);
			strStream << ")";
		}
		break;
	case eJOU_PRI_OPER_UPDATE :
		{
			//更新操作日志结果

			//CREATE TABLE tb_cache_oper_update ("
			//	"id INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
			//	"EndTime VARCHAR(64) DEFAULT NULL,"
			//	"CliPmsID INTEGER,"
			//	"CliID INTEGER,"	
			//	"CmdSectionID INTEGER,"		
			//	"Result INTEGER,"
			//	"Failure VARCHAR(512) DEFAULT NULL,"	
			//	"bupdate INTEGER DEFAULT 0"
			//	")";


			strStream << "INSERT INTO tb_cache_oper_update (";
			strStream <<"EndTime,CliPmsID,CliID,CmdSectionID,";		
			strStream << "Result";
			if( stInfo.unLog.stOperation.czFailure[0]!='\0')
			{
				strStream << ",Failure";
			}
			strStream << ",bupdate";

			strStream << ") VALUES (";


			strStream << "'" << czDateTime << "',";
			strStream << (int) stInfo.unLog.stOperation.iCliPmsID << ",";
			strStream << (int) stInfo.unLog.stOperation.iCliID << ",";
			strStream << (int) stInfo.unLog.stOperation.iCmdSectionID << ",";

			strStream << (int) stInfo.unLog.stOperation.eResult << ",";
			if( stInfo.unLog.stOperation.czFailure[0]!='\0')
			{
				strStream << "'" << CnvSpChar(stInfo.unLog.stOperation.czFailure) << "',";
			}
			strStream << (long)time(NULL);
			strStream << ")";
		}
		break;
	default :
		GS_ASSERT(0);
		return eJOU_E_NJOUTYPE;
		break;
	}
	oStrSql = strStream.str();
	return eJOU_R_SUCCESS;


}

EnumJouErrno CCacheBak::ToMDBJouSql( CGSString &oStrSql,const StruJournalInfo &stInfo)
{
	//转换为主数据库的SQL
	std::stringstream strStream;
	char czDateTime[20];


	//格式化时间
	GS_SNPRINTF(czDateTime,20, "%04d-%02d-%02d %02d:%02d:%02d",
		stInfo.stTime.iYear, stInfo.stTime.iMonth, stInfo.stTime.iDay,
		stInfo.stTime.iHour, stInfo.stTime.iMinute, stInfo.stTime.iSecond );

	switch( stInfo.eType )
	{
	case eJOU_TYPE_OPERATOR :
		{
			//操作日志
			strStream << "INSERT INTO TB_JOU_OPERATION (";
			strStream <<"CLI_PMSID,CLI_ID,CMD_SECTIONID,";
			strStream << "HP_TM,OPER_ID,USER_HOSTNAME,CLIENT_TYPE,";
			strStream << "PMS_ID,DEV_ID,CHN_ID,CHN_TYPE,CONTENT,RESULT,ERROR";
			strStream << ") VALUES (";

			strStream << (int) stInfo.unLog.stOperation.iCliPmsID << ",";
			strStream << (int) stInfo.unLog.stOperation.iCliID << ",";
			strStream << (int) stInfo.unLog.stOperation.iCmdSectionID << ",";

			strStream << m_pSrv->m_csDB.ToDateTime(czDateTime) << ",";
			strStream << "'" << CnvSpChar(stInfo.unLog.stOperation.OperatorID) << "',";
			strStream << "'" <<  CnvSpChar(stInfo.unLog.stOperation.czHostName) << "',";
			strStream << (int) stInfo.unLog.stOperation.eClientType << ",";

			strStream << (int) stInfo.unLog.stOperation.iPmsID << ",";
			strStream << (int) stInfo.unLog.stOperation.iDevID << ",";
			strStream << (int) stInfo.unLog.stOperation.iChn << ",";
			strStream << (int) stInfo.unLog.stOperation.eChnType << ",";
			strStream << "'" << CnvSpChar(stInfo.unLog.stOperation.czContent) << "',";
			strStream << (int) stInfo.unLog.stOperation.eResult << ",";
			strStream << "'" << CnvSpChar(stInfo.unLog.stOperation.czFailure) << "'";
			strStream << ")";
		}
		break;
	case eJOU_TYPE_RUNSTATUS :
		{
			//运行日志
			strStream << "INSERT INTO TB_JOU_RUNSTATUS (HP_TM,CLIENT_TYPE,PMS_ID,DEV_ID,CHN_ID,CHN_TYPE,ISONLINE,CONTENT,DESCRI) VALUES (";
			strStream << m_pSrv->m_csDB.ToDateTime(czDateTime) << ",";
			strStream << (int) stInfo.unLog.stRunStatus.eClientType << ",";
			strStream << (int) stInfo.unLog.stRunStatus.iPmsID << ",";
			strStream << (int) stInfo.unLog.stRunStatus.iDevID << ",";
			strStream << (int) stInfo.unLog.stRunStatus.iChnID << ",";
			strStream << (int) stInfo.unLog.stRunStatus.iChnType << ",";
			strStream << (int) stInfo.unLog.stRunStatus.iOnline << ",";
			strStream << "'" << CnvSpChar(stInfo.unLog.stRunStatus.czContent) << "',";
			strStream << "'" << CnvSpChar(stInfo.unLog.stRunStatus.czDescri) << "'";
			strStream << ")";

		}
		break;
	case eJOU_TYPE_LOGIN :
		{



			strStream << "INSERT INTO TB_JOU_LOGIN (HP_TM,OPER_ID,USER_HOSTNAME,CONTENT,RESULT,DESCRI,ERROR) VALUES (";
			strStream <<  m_pSrv->m_csDB.ToDateTime(czDateTime) << ",";
			strStream << "'" << CnvSpChar(stInfo.unLog.stLogin.OperatorID) << "',";
			strStream << "'" << CnvSpChar(stInfo.unLog.stLogin.czHostName) << "',";
			strStream << "'" << CnvSpChar(stInfo.unLog.stLogin.czContent) << "',";
			strStream << (int) stInfo.unLog.stLogin.eResult << ",";
			strStream << "'" << CnvSpChar(stInfo.unLog.stLogin.czDescri) << "',";
			strStream << "'" << CnvSpChar(stInfo.unLog.stLogin.czFailure) << "'";	
			strStream << ")";
		}
		break;
	case eJOU_PRI_OPER_UPDATE :
		{
			std::stringstream strSubSelect; //子查询



			strSubSelect << "(SELECT MAX(ID) as ID FROM TB_JOU_OPERATION";
			strSubSelect <<  " WHERE CLI_PMSID=" << (int)stInfo.unLog.stOperation.iCliPmsID;
			strSubSelect << " AND CLI_ID=" << stInfo.unLog.stOperation.iCliID;
			strSubSelect << " AND CMD_SECTIONID=" << stInfo.unLog.stOperation.iCmdSectionID;
	

			struct tm stTm;
			bzero(&stTm, sizeof(stTm) );
			stTm.tm_year = stInfo.stTime.iYear-1900;
			stTm.tm_mon = stInfo.stTime.iMonth-1;
			stTm.tm_mday = stInfo.stTime.iDay;
			stTm.tm_hour = stInfo.stTime.iHour;
			stTm.tm_min = stInfo.stTime.iMinute;
			stTm.tm_sec = stInfo.stTime.iSecond;

			time_t t = mktime(&stTm);
			t -= m_pSrv->m_csCfg.m_iCacheInterval;

			localtime_s(&stTm,&t);

			char czDateTimeBegin[20];
			GS_SNPRINTF(czDateTimeBegin,20, "%04d-%02d-%02d %02d:%02d:%02d",
				stTm.tm_year+1900, stTm.tm_mon+1, stTm.tm_mday,
				stTm.tm_hour, stTm.tm_min, stTm.tm_sec);

			strSubSelect << " AND HP_TM>=" << m_pSrv->m_csDB.ToDateTime(czDateTimeBegin);
			strSubSelect << " AND HP_TM<" << m_pSrv->m_csDB.ToDateTime(czDateTime);
			strSubSelect << ")";

			if( m_pSrv->m_csDB.DBaseType()==MYSQL )
			{
				strStream << "UPDATE TB_JOU_OPERATION a,";
				strStream << strSubSelect.str() << " b";

				strStream << " SET a.RESULT=" << (int)stInfo.unLog.stOperation.eResult;
				if( stInfo.unLog.stOperation.czFailure[0] != '\0')
				{
					strStream << ",a.ERROR='" << CnvSpChar(stInfo.unLog.stOperation.czFailure) << "'";
				}

				strStream << " WHERE a.ID=b.ID";				
			}
			else
			{


				strStream << "UPDATE TB_JOU_OPERATION";
				strStream << " SET RESULT=" << (int)stInfo.unLog.stOperation.eResult;
				if( stInfo.unLog.stOperation.czFailure[0] != '\0')
				{
					strStream << ",ERROR='" << CnvSpChar(stInfo.unLog.stOperation.czFailure) << "'";
				}

				strStream << " WHERE ID IN ";
				strStream << strSubSelect.str();
			}


		}
		break;
	default :
		GS_ASSERT(0);
		return eJOU_E_NJOUTYPE;
		break;
	}
	oStrSql = strStream.str();
	return eJOU_R_SUCCESS;
}

StruJournalInfo* CCacheBak::CreateJouInfo(void)
{
StruJournalInfo* pRet = NULL;
	if( !m_listMemCache.empty() )
	{
		CListJouInfoCache::iterator csIt = m_listMemCache.begin();
		pRet  = *csIt;
		m_listMemCache.erase(csIt);
	
	}
	else
	{
		pRet = (StruJournalInfo*)::malloc(sizeof(StruJournalInfo)); 
	}
	return pRet;
}

void CCacheBak::FreeJouInfo(StruJournalInfo*pInfo)
{
	if( pInfo )
	{
		if( m_listMemCache.size() > 50000 )
		{
			::free(pInfo);
		}
		else
		{
			m_listMemCache.push_front(pInfo);
		}
	}
}



EnumJouErrno CCacheBak::Add(const StruJournalInfo &stInfo )
{
	CGSAutoReaderMutex rlocker(&m_csMutex);
	if( m_eStatus )
	{
		MY_LOG_WARN(g_pLog, "Module status invalid: 0x%x.\n", m_eStatus );
		return eJOU_E_NINIT;
	}
	
	m_csWMutex.Lock();
	StruJournalInfo *pCache = CreateJouInfo();
	if( !pCache )
	{
		m_csWMutex.Unlock();
		GS_ASSERT(0);
		MY_LOG_FATAL(g_pLog, "Jou malloc  StruJournalInfo fail.\n");
		return eJOU_E_NMEM;
	} 

	memcpy(pCache, &stInfo, sizeof(stInfo));

	switch( pCache->eType )
	{
	case eJOU_PRI_OPER_UPDATE :
	case eJOU_TYPE_OPERATOR :
		{
			CheckContent(pCache->unLog.stOperation.OperatorID, 
				  sizeof(pCache->unLog.stOperation.OperatorID));
			CheckContent(pCache->unLog.stOperation.czHostName, 
				  sizeof(pCache->unLog.stOperation.czHostName));
			CheckContent(pCache->unLog.stOperation.czContent, 
				  sizeof(pCache->unLog.stOperation.czContent));
			CheckContent(pCache->unLog.stOperation.czFailure, 
				  sizeof(pCache->unLog.stOperation.czFailure));
		}
	break;
	case eJOU_TYPE_RUNSTATUS :
		{
			
			CheckContent(pCache->unLog.stRunStatus.czContent, 
				  sizeof(pCache->unLog.stRunStatus.czContent));
			CheckContent(pCache->unLog.stRunStatus.czDescri, 
				  sizeof(pCache->unLog.stRunStatus.czDescri));
		}
		break;
	case eJOU_TYPE_LOGIN :
		{

			CheckContent(pCache->unLog.stLogin.OperatorID, 
				  sizeof(pCache->unLog.stLogin.OperatorID));
			CheckContent(pCache->unLog.stLogin.czHostName, 
				 sizeof(pCache->unLog.stLogin.czHostName));
			CheckContent(pCache->unLog.stLogin.czContent, 
				 sizeof(pCache->unLog.stLogin.czContent));
			CheckContent(pCache->unLog.stLogin.czFailure, 
				 sizeof(pCache->unLog.stLogin.czFailure));
			CheckContent(pCache->unLog.stLogin.czDescri, 
				 sizeof(pCache->unLog.stLogin.czDescri));
		}
		break;


	}

	

	
	m_listCache.push_back(pCache);
	UINT iSize = m_listCache.size();
	m_csWCond.Signal();
	m_csWMutex.Unlock();

	if( iSize>50000 )
	{
		//降低写入的速度
		MSLEEP(10);
	}

// 	if( m_listCache.size() )
// 
// 	
// 	CGSString strSql;
// 	EnumJouErrno eRet = ToSqliteJouSql(strSql, stInfo);
// 	if( eRet )
// 	{
// 		GS_ASSERT(0);
// 		MY_LOG_ERROR(g_pLog, "Jou Type %d to sqlite sql fail.\n", stInfo.eType );
// 		return eRet;
// 	}
// 	char *czError = NULL;
// 
// 	if( SQLITE_OK != sqlite3_exec(strSql.c_str(), NULL,NULL, &czError))
// 	{
// 		
// 		MY_LOG_ERROR(g_pLog, "Jou sqltie exce '%s' fail: %s.\n", strSql.c_str(), czError );
// 		sqlite3_free(czError);
// 		GS_ASSERT(0);
// 		return eRet;
// 	}
// 	m_csWCond.Signal();
	return eJOU_R_SUCCESS;
}

EnumJouErrno CCacheBak::JouUpdateOperation( const StruJouOperationUpdate *pData )
{
	StruJournalInfo stInfo;
	StruSysTime stTm;
	bzero(&stInfo, sizeof(stInfo) );
	stInfo.eType = eJOU_PRI_OPER_UPDATE;
	DoGetLocalTime(&stTm);
	stInfo.stTime.iYear = stTm.wYear;
	stInfo.stTime.iMonth = stTm.wMonth;
	stInfo.stTime.iDay = stTm.wDay;
	stInfo.stTime.iHour = stTm.wHour;
	stInfo.stTime.iMinute = stTm.wMinute;
	stInfo.stTime.iSecond = stTm.wSecond;

	strncpy(stInfo.unLog.stOperation.czFailure,pData->czFailure, JOU_DESCRI_LEN );
	stInfo.unLog.stOperation.czFailure[JOU_DESCRI_LEN-1] = '\0';
	stInfo.unLog.stOperation.iCliPmsID = pData->iCliPmsID;
	stInfo.unLog.stOperation.iCliID = pData->iCliID;
	stInfo.unLog.stOperation.iCmdSectionID = pData->iCmdSectionID;
	stInfo.unLog.stOperation.eResult = pData->eResult;
	return Add(stInfo);

}

void CCacheBak::ThreadCallback(CGSThread *gsThreadHandle,void *pParam )
{
	CCacheBak *p = (CCacheBak*)pParam;
	p->WatchEntry();
}


static void StringDatetimeToStruct(const char *czStr, CmdProtocolDef::StruDateTime &stResult)
{
	GS_ASSERT(czStr);
	bzero(&stResult, sizeof(stResult));
	if( czStr )
	{
		int iYear,iMonth,iDay, iHour, iMinute, iSecond;
		if( 6==sscanf(czStr, "%04d-%02d-%02d %02d:%02d:%02d",
				&iYear, &iMonth, &iDay, &iHour, &iMinute, &iSecond)	)
		{
			stResult.iYear = iYear;
			stResult.iMonth = iMonth;
			stResult.iDay = iDay;
			stResult.iHour = iHour;
			stResult.iMinute = iMinute;
			stResult.iSecond = iSecond;
			return;
		}
	}
	GS_ASSERT(0);
}

EnumJouErrno CCacheBak::LoadJouInfoFromLocalSqlite( CListJouInfoCache &listValue  )
{
	int iRet;

	CGSString strSql;
	sqlite3_stmt *pStmt = NULL;
	StruJournalInfo *p;


#define LIMIT_ROWS  " 400"

	////操作日志
	strSql = "SELECT id,StartTime,CliPmsID,CliID,CmdSectionID,OperatorID,HostName,ClientType,"
			"PmsID,DevID,Chn,ChnType,Content,Result,Failure,bupdate"
			" FROM tb_cache_oper ORDER BY id ASC LIMIT " LIMIT_ROWS;
	
	iRet = sqlite3_prepare(hCacheSqlDB,strSql.c_str(), strSql.length(), &pStmt, NULL);
	if( iRet != SQLITE_OK )
	{	
		MY_LOG_FATAL(g_pLog, "Sqlite exe: '%s' failure. %s\n",
			strSql.c_str(),  sqlite3_errmsg(hCacheSqlDB));
		GS_ASSERT_EXIT(0, -1);		
		return eJOU_E_DB_EXESQL;
	}

	
	while( SQLITE_ROW==(iRet=sqlite3_step(pStmt)) )
	{
		m_csWMutex.Lock();
		p = CreateJouInfo();
		m_csWMutex.Unlock();
		if( !p )
		{
			GS_ASSERT(0);
			break;
		}
		bzero(p, sizeof(*p));

		p->eType = eJOU_TYPE_OPERATOR;
		p->iSeqID = (INT64) sqlite3_column_int64(pStmt,0);		
		StringDatetimeToStruct((const char *)sqlite3_column_text(pStmt, 1), p->stTime);
		p->unLog.stOperation.iCliPmsID = sqlite3_column_int(pStmt,2);		
		p->unLog.stOperation.iCliID = sqlite3_column_int(pStmt,3);	
		p->unLog.stOperation.iCmdSectionID = sqlite3_column_int(pStmt,4);	

		//OperatorID,HostName,ClientType,
		if( sqlite3_column_text(pStmt,5) )
		{
			strncpy(p->unLog.stOperation.OperatorID,
				(const char*) sqlite3_column_text(pStmt,5),
				JOU_OPERATOR_ID_LEN-1 );
		}

		if( sqlite3_column_text(pStmt,6) )
		{
			strncpy(p->unLog.stOperation.czHostName,
				(const char*)sqlite3_column_text(pStmt,6),
				JOU_HOSTNAME_LEN-1 );
		}
		p->unLog.stOperation.eClientType = (CmdProtocolDef::EnumClientType) sqlite3_column_int(pStmt,7);	

		//PmsID,DevID,Chn,ChnType,

		p->unLog.stOperation.iPmsID = sqlite3_column_int(pStmt,8);		
		p->unLog.stOperation.iDevID = sqlite3_column_int(pStmt,9);	
		p->unLog.stOperation.iChn = sqlite3_column_int(pStmt,10);	
		p->unLog.stOperation.eChnType = (CmdProtocolDef::EnumChannelType)sqlite3_column_int(pStmt,11);	

		//Content,Result,Failure,
		if( sqlite3_column_text(pStmt,12) )
		{
			strncpy(p->unLog.stOperation.czContent,
				(const char*)sqlite3_column_text(pStmt,12),
				127 );
		}

		p->unLog.stOperation.eResult = sqlite3_column_int(pStmt,13);	

		if( sqlite3_column_text(pStmt,14) )
		{
			strncpy(p->unLog.stOperation.czFailure,
				(const char*)sqlite3_column_text(pStmt,14),
				JOU_DESCRI_LEN-1 );
		}
		listValue.push_back(p);
	}
	sqlite3_finalize(pStmt); //关闭SQLITE 句柄
	if( listValue.size()>100 )
	{
		return eJOU_R_SUCCESS;
	}



	////操作日志更新

	strSql = "SELECT id,EndTime,CliPmsID,CliID,CmdSectionID"
		",Result,Failure,bupdate"
		" FROM tb_cache_oper_update ORDER BY id ASC LIMIT " LIMIT_ROWS;
	pStmt = NULL;
	iRet = sqlite3_prepare(hCacheSqlDB,strSql.c_str(), strSql.length(), &pStmt, NULL);
	if( iRet != SQLITE_OK )
	{	
		MY_LOG_FATAL(g_pLog, "Sqlite exe: '%s' failure. %s\n",
			strSql.c_str(),  sqlite3_errmsg(hCacheSqlDB));
		GS_ASSERT_EXIT(0, -1);		
		return eJOU_E_DB_EXESQL;
	}

	while( SQLITE_ROW==(iRet=sqlite3_step(pStmt)) )
	{
		m_csWMutex.Lock();
		p = CreateJouInfo();
		m_csWMutex.Unlock();
		if( !p )
		{
			GS_ASSERT(0);
			break;
		}
		bzero(p, sizeof(*p));

		p->eType = eJOU_PRI_OPER_UPDATE;
		p->iSeqID = (INT64) sqlite3_column_int64(pStmt,0);		
		StringDatetimeToStruct((const char *)sqlite3_column_text(pStmt, 1), p->stTime);
		p->unLog.stOperation.iCliPmsID = sqlite3_column_int(pStmt,2);		
		p->unLog.stOperation.iCliID = sqlite3_column_int(pStmt,3);	
		p->unLog.stOperation.iCmdSectionID = sqlite3_column_int(pStmt,4);	

		
		//Result,Failure,		
		p->unLog.stOperation.eResult = sqlite3_column_int(pStmt,5);	

		if( sqlite3_column_text(pStmt,6) )
		{
			strncpy(p->unLog.stOperation.czFailure,
				(const char*)sqlite3_column_text(pStmt,6),
				JOU_DESCRI_LEN-1 );
		}
		listValue.push_back(p);
	}
	sqlite3_finalize(pStmt); //关闭SQLITE 句柄
	if( listValue.size()>100 )
	{
		return eJOU_R_SUCCESS;
	}


	////运行状态

	strSql = "SELECT id,StartTime,ClientType,PmsID,DevID,ChnID,ChnType,IsOnline,Content,Descri,bupdate"
		" FROM tb_cache_RunSt ORDER BY id ASC LIMIT " LIMIT_ROWS;

	iRet = sqlite3_prepare(hCacheSqlDB,strSql.c_str(), strSql.length(), &pStmt, NULL);
	if( iRet != SQLITE_OK )
	{	
		MY_LOG_FATAL(g_pLog, "Sqlite exe: '%s' failure. %s\n",
			strSql.c_str(),  sqlite3_errmsg(hCacheSqlDB));
		GS_ASSERT_EXIT(0, -1);		
		return eJOU_E_DB_EXESQL;
	}


	while( SQLITE_ROW==(iRet=sqlite3_step(pStmt)) )
	{
		m_csWMutex.Lock();
		p = CreateJouInfo();
		m_csWMutex.Unlock();
		if( !p )
		{
			GS_ASSERT(0);
			break;
		}
		bzero(p, sizeof(*p));

		p->eType = eJOU_TYPE_RUNSTATUS;
		p->iSeqID = (INT64) sqlite3_column_int64(pStmt,0);		
		StringDatetimeToStruct((const char *)sqlite3_column_text(pStmt, 1), p->stTime);
		p->unLog.stRunStatus.eClientType =(CmdProtocolDef::EnumClientType)sqlite3_column_int(pStmt,2);		
		p->unLog.stRunStatus.iPmsID = sqlite3_column_int(pStmt,3);	
		p->unLog.stRunStatus.iDevID = sqlite3_column_int(pStmt,4);
		p->unLog.stRunStatus.iChnID = sqlite3_column_int(pStmt,5);
		p->unLog.stRunStatus.iChnType = sqlite3_column_int(pStmt,6);
		p->unLog.stRunStatus.iOnline = sqlite3_column_int(pStmt,7);

		//Content,Descri,,
		if( sqlite3_column_text(pStmt,8) )
		{
			strncpy(p->unLog.stRunStatus.czContent,
				(const char*)sqlite3_column_text(pStmt,8),
				127 );
		}
		if( sqlite3_column_text(pStmt,9) )
		{
			strncpy(p->unLog.stRunStatus.czDescri,
				(const char*)sqlite3_column_text(pStmt,9),
				JOU_DESCRI_LEN-1 );
		}
		listValue.push_back(p);
	}
	sqlite3_finalize(pStmt); //关闭SQLITE 句柄
	if( listValue.size()>100 )
	{
		return eJOU_R_SUCCESS;
	}

	//登陆日志
	strSql = "SELECT id,StartTime,OperatorID,HostName,Content,Result,Descri,Failure,bupdate"
		" FROM tb_cache_Login ORDER BY id ASC LIMIT " LIMIT_ROWS;

	iRet = sqlite3_prepare(hCacheSqlDB,strSql.c_str(), strSql.length(), &pStmt, NULL);
	if( iRet != SQLITE_OK )
	{	
		MY_LOG_FATAL(g_pLog, "Sqlite exe: '%s' failure. %s\n",
			strSql.c_str(),  sqlite3_errmsg(hCacheSqlDB));
		GS_ASSERT_EXIT(0, -1);		
		return eJOU_E_DB_EXESQL;
	}


	while( SQLITE_ROW==(iRet=sqlite3_step(pStmt)) )
	{
		m_csWMutex.Lock();
		p = CreateJouInfo();
		m_csWMutex.Unlock();
		if( !p )
		{
			GS_ASSERT(0);
			break;
		}
		bzero(p, sizeof(*p));

		p->eType = eJOU_TYPE_LOGIN;
		p->iSeqID = (INT64) sqlite3_column_int64(pStmt,0);		
		StringDatetimeToStruct((const char *)sqlite3_column_text(pStmt, 1), p->stTime);

		

		//OperatorID,HostName
		if( sqlite3_column_text(pStmt,2) )
		{
			strncpy(p->unLog.stLogin.OperatorID,
				(const char*) sqlite3_column_text(pStmt,2),
				JOU_OPERATOR_ID_LEN-1 );
		}

		if( sqlite3_column_text(pStmt,3) )
		{
			strncpy(p->unLog.stLogin.czHostName,
				(const char*)sqlite3_column_text(pStmt,3),
				JOU_HOSTNAME_LEN-1 );
		}		

		//Content,Result, Descri,Failure
		if( sqlite3_column_text(pStmt,4) )
		{
			strncpy(p->unLog.stLogin.czContent,
				(const char*)sqlite3_column_text(pStmt,4),
				127 );
		}
		p->unLog.stLogin.eResult = sqlite3_column_int(pStmt,5);	

		if( sqlite3_column_text(pStmt,6) )
		{
			strncpy(p->unLog.stLogin.czDescri,
				(const char*)sqlite3_column_text(pStmt,6),
				JOU_DESCRI_LEN-1 );
		}

		if( sqlite3_column_text(pStmt,7) )
		{
			strncpy(p->unLog.stLogin.czFailure,
				(const char*)sqlite3_column_text(pStmt,7),
				JOU_DESCRI_LEN-1 );
		}
		listValue.push_back(p);
	}
	sqlite3_finalize(pStmt); //关闭SQLITE 句柄
	return eJOU_R_SUCCESS;
	
}

EnumJouErrno CCacheBak::DeleteJouToForSqlite( CListJouInfoCache &listValue)
{
	int iRet;
	char *pErrMsg;
	StruJournalInfo *p;
	char czSql[256];
	sqlite3_exec(hCacheSqlDB, "BEGIN;", NULL, NULL, NULL );	
	CListJouInfoCache::iterator csIt;
	czSql[255] = '\0';
	for( csIt = listValue.begin(); csIt!=listValue.end(); ++csIt )
	{
		p  = *csIt;
		if( p->eType == eJOU_TYPE_OPERATOR )
		{
			GS_SNPRINTF(czSql, 255, "DELETE FROM tb_cache_oper WHERE id=%lld",
				(long long ) p->iSeqID);
		} 
		else if( p->eType == eJOU_PRI_OPER_UPDATE )
		{
			GS_SNPRINTF(czSql, 255, "DELETE FROM tb_cache_oper_update WHERE id=%lld",
				(long long ) p->iSeqID);
		}
		else if( p->eType == eJOU_TYPE_RUNSTATUS )
		{
			GS_SNPRINTF(czSql, 255, "DELETE FROM tb_cache_RunSt WHERE id=%lld",
				(long long ) p->iSeqID);
			
		}
		else if( p->eType == eJOU_TYPE_LOGIN )
		{
			GS_SNPRINTF(czSql, 255, "DELETE FROM tb_cache_Login WHERE id=%lld",
				(long long ) p->iSeqID);
		}
		else
		{
			GS_ASSERT(0);
			continue;
		}

		iRet = sqlite3_exec(hCacheSqlDB, czSql, NULL, NULL,  &pErrMsg );
		if( iRet!=SQLITE_OK ) 
		{
			MY_LOG_ERROR(g_pLog, "Sqlite exe: '%s' failure. %s\n",
				czSql, pErrMsg);
			::sqlite3_free(pErrMsg);			
		}
	}
	sqlite3_exec(hCacheSqlDB, "END;", NULL, NULL, NULL );
	return eJOU_R_SUCCESS;

}

void CCacheBak::WriteJouToLocalSqlite( CListJouInfoCache &listValue )
{	
	int iRet;
	char *pErrMsg;
	CGSString strSql;

	sqlite3_exec(hCacheSqlDB, "BEGIN;", NULL, NULL, NULL );	
	CListJouInfoCache::iterator csIt;
	for( csIt = listValue.begin(); csIt!=listValue.end(); ++csIt )
	{
		if( eJOU_R_SUCCESS==ToSqliteJouSql(strSql, *(*csIt) ))
		{
			iRet = sqlite3_exec(hCacheSqlDB, strSql.c_str(), NULL, NULL,  &pErrMsg );
			if( iRet!=SQLITE_OK ) 
			{
				MY_LOG_ERROR(g_pLog, "Sqlite exe: '%s' failure. %s\n",
					strSql.c_str(), pErrMsg);
				::sqlite3_free(pErrMsg);					
			}
		}
		else
		{
			MY_LOG_FATAL(g_pLog, "Jou type %d to sqlite sql fail.\n",
				(*csIt)->eType);
			GS_ASSERT(0);
		}
	}
	sqlite3_exec(hCacheSqlDB, "END;", NULL, NULL, NULL );


	
}

EnumJouErrno CCacheBak::WriteJouToMDB( IConnection *pCnn, CListJouInfoCache &listValue  )
{
	int iRet;
	CGSString strSql;

	pCnn->BeginTrans();

	CListJouInfoCache::iterator csIt;
	for( csIt = listValue.begin(); csIt!=listValue.end(); ++csIt )
	{
		if( eJOU_R_SUCCESS==ToMDBJouSql(strSql, *(*csIt)))
		{		
			iRet = pCnn->ExecuteSql(strSql.c_str());
			if( !iRet ) 
			{
				GS_ASSERT(0);
				MY_LOG_ERROR(g_pLog, "MDB exec '%s' fail.\n",strSql.c_str() );	
				break; //出错是也保证下一条可以写入 ???				
				pCnn->RollbackTrans();
				return eJOU_E_DB_EXESQL; 
			}
		}
		else
		{
			MY_LOG_FATAL(g_pLog, "Jou type %d to MDB sql fail.\n",
				(*csIt)->eType);
			GS_ASSERT(0);
		}
	}
	pCnn->CommitTrans();

	return eJOU_R_SUCCESS;
}


void CCacheBak::WatchEntry(void)
{
	IConnection *pCnn = NULL;
	int iDeleteCnts[MAX_SQLITE_HANDLE]; //首次先收缩数据库
	bzero(iDeleteCnts, sizeof(iDeleteCnts) );
	iDeleteCnts[iCacheSqlIdx] =  1000000;		
	CListJouInfoCache listTemp;
	CListJouInfoCache::iterator csIt;
	INT iRet;
	UINT iCCount; //连续写入统计
	BOOL bVACCUM = FALSE;
	m_csMutex.LockReader();
	while( !m_csWatcher.TestExit() && !m_eStatus )
	{		
		//把内存数据写到本地数据库		
		iCCount = 0;
		bVACCUM = FALSE;
faster_write :		
		listTemp.clear();
		m_csWMutex.Lock();
		listTemp = m_listCache;
		m_listCache.clear();
		m_csWMutex.Unlock();
		if( !listTemp.empty() )
		{
			WriteJouToLocalSqlite(listTemp);
			m_csWMutex.Lock();
			for( csIt = listTemp.begin(); csIt!=listTemp.end(); ++csIt )
			{
				FreeJouInfo(*csIt);
			}
			m_csWMutex.Unlock();
			listTemp.clear();
		}

		//从本地数据库把数据库写到主数据库
		if( !pCnn )
		{
			pCnn = m_pSrv->m_csDB.GetConnection();					
		}

		while(pCnn && hCacheSqlDB && !m_eStatus )
		{
			//重数据库中加载数据


			m_csWMutex.Lock();
			if( m_listCache.size() > 100 )
			{
				//优先把内存数据写到到本地数据库
				m_csWMutex.Unlock();
				goto faster_write;

			}
			m_csWMutex.Unlock();

			listTemp.clear();
			if( eJOU_R_SUCCESS != LoadJouInfoFromLocalSqlite(listTemp) )
			{
				break;
			}

			if( listTemp.empty() )
			{		
				bVACCUM = TRUE;
				break;
			}
			else if( iCCount > 30000 )
			{				
				//连续写入过多， 中断一下
				MSLEEP(10);		
				iCCount = 0;
			}
			iRet = WriteJouToMDB(pCnn, listTemp);

			if( eJOU_R_SUCCESS == iRet )
			{
				DeleteJouToForSqlite(listTemp);
				iDeleteCnts[iCacheSqlIdx] += listTemp.size();
				iCCount += listTemp.size();
			}	
			
			m_csWMutex.Lock();
			for( csIt = listTemp.begin(); csIt!=listTemp.end(); ++csIt )
			{
				FreeJouInfo(*csIt);
			}
			m_csWMutex.Unlock();
			listTemp.clear();

			if( eJOU_R_SUCCESS != iRet )
			{
				//主数据库失败, 休眠一下
				break;
			}
		} //end pCnn

		if( iDeleteCnts[iCacheSqlIdx] > 1000 && bVACCUM )
		{			
			//收缩数据库
			if(hCacheSqlDB && SQLITE_OK == sqlite3_exec(hCacheSqlDB, "VACCUM", NULL, NULL,NULL) )
			{
				iDeleteCnts[iCacheSqlIdx] = 0;			
			}
		}
		m_csMutex.UnlockReader();

		m_csWMutex.Lock();
		if( !m_eStatus )
		{
			if( pCnn )
			{
				pCnn->ReleaseConnection();
				pCnn = NULL;
			}
			m_csWCond.WaitTimeout(&m_csWMutex, 100 );
		}
		m_csWMutex.Unlock();
		m_csMutex.LockReader();
	}

	m_csMutex.UnlockReader();
	if(pCnn)
	{
		pCnn->ReleaseConnection();
		pCnn = NULL;
	}
}


