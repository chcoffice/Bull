#include "RecordManager.h"
#include "Service.h"
using namespace  JOU;


CRecordManager::CRecordManager(void)
:CJouModule("RecordManager")
,m_csWatcher()
{
}

CRecordManager::~CRecordManager(void)
{
	if( m_csWatcher.IsRunning() )
	{
		m_csWatcher.Join(500);
		MSLEEP(10);
	}
}


EnumJouErrno CRecordManager::Start(void* pData)
{
	if( m_eStatus == eST_RUNNING)
	{
		return eJOU_R_SUCCESS;
	}
	EnumJouErrno eRet = CJouModule::Start(pData);
	GS_ASSERT_RET_VAL(!eRet, eRet);
	if( !m_csWatcher.Start((GSThreadCallbackFunction)CRecordManager::ThreadCallback, this) )
	{
		m_eStatus = eST_INIT;		
		return eJOU_E_UNKNOWN;
	}
	return eJOU_R_SUCCESS;

}

void CRecordManager::Stop(void)
{
	BOOL bStart = m_eStatus==eST_RUNNING;
	CJouModule::Stop();	
	if( bStart )
	{
		m_csWatcher.Join(3000);
		MSLEEP(10);
	}
}


void CRecordManager::ThreadCallback(CGSThread *gsThreadHandle,void *pParam )
{
	CRecordManager *p = (CRecordManager*)pParam;
	p->WatchEntry();
}

void  CRecordManager::ExeDelSql( const char *czSql)
{
	// TODO
IConnection *pCnn;
	if( m_eStatus )
	{
		return;
	}

	pCnn = m_pSrv->m_csDB.GetConnection();
	if( !pCnn )
	{
		return;
	}
	if( !pCnn->ExecuteSql(czSql) )
	{
		MY_LOG_ERROR(g_pLog,"JouDB ExeSql '%s' fail.\n", czSql);
	}
	pCnn->ReleaseConnection();
}

void CRecordManager::ExeDelDays(char *pBuf, INT iBuf, 
								const char *czTableName,
								const char *czDataTime  )
{
	GS_SNPRINTF(pBuf, iBuf, "DELETE FROM %s WHERE HP_TM<%s", 
		czTableName, m_pSrv->m_csDB.ToDateTime(czDataTime).c_str() );
	ExeDelSql(pBuf);	
}

// void CRecordManager::ExeDelRows(char *pBuf, INT iBuf, 
// 								const char *czTableName,
// 								unsigned long iLeftRow  )
// {
// 	switch(m_pSrv->m_csDB.DBaseType() )
// 	{
// 	case ORACLE :
// 		{
// 			GS_SNPRINTF(pBuf, iBuf, 
// 				"DELETE %s WHERE ID IN (SELECT t.ID FROM (SELECT ROWNUM AS NO,ID FROM %s ORDER BY HP_TM DESC) t WHERE t.NO>%lu)",
// 				czTableName, czTableName, iLeftRow);
// 
// 			// 		GS_SNPRINTF(pBuf, iBuf, 
// 			// 			"DELETE %s WHERE ID IN (SELECT ID FROM (SELECT ID FROM %s ORDER BY HP_TM ASC) t WHERE ROWNUM<=(SELECT count(id)-%lu FROM %s))";
// 			// 			czTableName, czTableName, iLeftRow, czTableName);
// 		}
// 		break;
// 	case SQLSERVER :
// 		{
// 
// 			GS_SNPRINTF(pBuf, iBuf, 
// 				"DELETE %s WHERE ID IN (SELECT TOP (SELECT count(id)-%lu AS ii FROM %s) ID FROM %s ORDER BY HP_TM ASC)",
// 				czTableName, czTableName,czTableName, iLeftRow);
// 		}
// 		break;
// 	default : //MYSQL
// 		{
// 
// 			GS_SNPRINTF(pBuf, iBuf, 
// 				"DELETE %s WHERE ID IN (SELECT ID FROM ORDER BY HP_TM ASC LIMIT %lu)",
// 				czTableName, czTableName, iLeftRow, iLeftRow);
// 			break;
// 		}
// 	}
// 	ExeDelSql(pBuf);
// }

void CRecordManager::WatchEntry(void)
{
	time_t tv,tvtemp;
	struct tm tmNow;
	char czTemp[64];
	char czSql[512];

	while( !m_csWatcher.TestExit() && !m_eStatus )
	{
		//清除登陆日志
		tvtemp = time(NULL);


		if( m_pSrv->m_csCfg.m_iRcdLoginDays > 0 )
		{
			tv = tvtemp-m_pSrv->m_csCfg.m_iRcdLoginDays*86400;
#ifdef _WIN32
			localtime_s(&tmNow, &tv);
#else
			localtime_r(&tmNow, &tv);
#endif
			
			GS_SNPRINTF(czTemp, 64, "%04d-%02d-%02d %02d:%02d:%02d",
				tmNow.tm_year+1900, tmNow.tm_mon+1, tmNow.tm_mday,
				tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec );
			ExeDelDays(czSql, 512, "TB_JOU_LOGIN", czTemp );
			
		}

// 		if( m_pSrv->m_csCfg.m_iRcdLoginRows > 0 )
// 		{
// 			ExeDelRows(czSql, 512, "TB_JOU_LOGIN", (unsigned long) m_pSrv->m_csCfg.m_iRcdLoginRows );
// 			
// 		}


		//清除运行日志
		if( m_pSrv->m_csCfg.m_iRcdStatusDays > 0 )
		{
			tv = tvtemp-m_pSrv->m_csCfg.m_iRcdStatusDays*86400;
#ifdef _WIN32
			localtime_s(&tmNow, &tv);
#else
			localtime_r(&tmNow, &tv);
#endif
			GS_SNPRINTF(czTemp, 64, "%04d-%02d-%02d %02d:%02d:%02d",
				tmNow.tm_year+1900, tmNow.tm_mon+1, tmNow.tm_mday,
				tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec );
			ExeDelDays(czSql, 512, "TB_JOU_RUNSTATUS", czTemp );


		}

// 		if( m_pSrv->m_csCfg.m_iRcdStatusRows > 0 )
// 		{
// 			ExeDelRows(czSql, 512, "TB_JOU_RUNSTATUS", 
// 				(unsigned long) m_pSrv->m_csCfg.m_iRcdStatusRows );
// 		}

		//操作日志

		if( m_pSrv->m_csCfg.m_iRcdOperDays > 0 )
		{
			tv = tvtemp-m_pSrv->m_csCfg.m_iRcdOperDays*86400;
#ifdef _WIN32
			localtime_s(&tmNow, &tv);
#else
			localtime_r(&tmNow, &tv);
#endif
			GS_SNPRINTF(czTemp, 64, "%04d-%02d-%02d %02d:%02d:%02d",
				tmNow.tm_year+1900, tmNow.tm_mon+1, tmNow.tm_mday,
				tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec );
			ExeDelDays(czSql, 512, "TB_JOU_OPERATION", czTemp );

		}

// 		if( m_pSrv->m_csCfg.m_iRcdOperRows > 0 )
// 		{
// 			ExeDelRows(czSql, 512, "TB_JOU_OPERATION", 
// 				(unsigned long) m_pSrv->m_csCfg.m_iRcdOperRows );
// 			
// 		}


		for( int i =0; i<36000 && !m_eStatus; i++ )
		{
			MSLEEP(100);
		}
	}
}

