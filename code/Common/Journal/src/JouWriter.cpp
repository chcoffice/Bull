#include "JouWriter.h"
#include "Service.h"

using namespace JOU;

CJouWriter::CJouWriter(void)
:CJouModule("JouWriter")
{
}

CJouWriter::~CJouWriter(void)
{
}

EnumJouErrno CJouWriter::Start(void* pData)
{
	return CJouModule::Start(pData);
}

void CJouWriter::Stop(void)
{
	CJouModule::Stop();
}

EnumJouErrno CJouWriter::JouAdd( const StruJournalInfo *pLog)
{
	if( m_eStatus != eST_RUNNING )
	{
		return eJOU_E_NINIT;
	}	
EnumJouErrno eRet;
	eRet =  m_pSrv->m_csCache.Add(*pLog);
	return eRet;

}


EnumJouErrno  CJouWriter::JouDelete(  EnumJournalType eType, INT64 iKeyID)
{
	if( m_eStatus )
	{
		return eJOU_E_NINIT;
	}
	char czTemp[128];
	switch( eType )
	{
	case eJOU_TYPE_OPERATOR :
		{
			//操作日志

			GS_SNPRINTF(czTemp, 128, "DELETE TB_JOU_OPERATION  WHERE ID=%lld\n",
				(long long ) iKeyID);


		}
		break;
	case eJOU_TYPE_RUNSTATUS :
		{
			//运行日志
			GS_SNPRINTF(czTemp, 128, "DELETE TB_JOU_RUNSTATUS  WHERE ID=%lld\n",
				(long long ) iKeyID);
		}
		break;
	case eJOU_TYPE_LOGIN :
		{
			//登陆日志
			GS_SNPRINTF(czTemp, 128, "DELETE TB_JOU_LOGIN  WHERE ID=%lld\n",
				(long long ) iKeyID);
		}		
		break;
	default :
		GS_ASSERT(0);
		return eJOU_E_NJOUTYPE;
		break;
	}	

	IConnection *pCnn = m_pSrv->m_csDB.GetConnection();
	if( !pCnn )
	{
		MY_LOG_ERROR(g_pLog, "Get JouDB Connection fail.\n");
		return eJOU_E_DB_GETCONN;
	}
	EnumJouErrno eRet = eJOU_R_SUCCESS;
	if( !pCnn->ExecuteSql(czTemp) )
	{
		MY_LOG_ERROR(g_pLog, "JouDB exe: %s fail.\n", czTemp);
		eRet = eJOU_E_DB_EXESQL;
	}
	pCnn->ReleaseConnection();
	return eRet;
}


EnumJouErrno  CJouWriter::JouDeleteExt( const char czFuncName[JOU_FUNC_NAME_LEN],
										   const char czArgs[JOU_ARG_LEN] )
{
	if( m_eStatus )
	{
		return eJOU_E_NINIT;
	}
	CGSString strSql;
	EnumJouErrno eRet;
	eRet = m_pSrv->MakeFunctionSql(strSql, czFuncName, czArgs );
	if( eRet )
	{
		return eRet;
	}
	IConnection *pCnn = m_pSrv->m_csDB.GetConnection();
	if( !pCnn )
	{
		MY_LOG_ERROR(g_pLog, "Get JouDB Connection fail.\n");
		return eJOU_E_DB_GETCONN;
	}

	if( !pCnn->ExecuteSql( strSql.c_str() ) )
	{
		MY_LOG_ERROR(g_pLog, "JouDB exe: %s fail.\n", strSql.c_str() );
		eRet = eJOU_E_DB_EXESQL;
	}
	pCnn->ReleaseConnection();
	return eRet;
	return eJOU_E_UNKNOWN;
}


EnumJouErrno  CJouWriter::JouUpdateOperation( const StruJouOperationUpdate *pData )
{


	if( m_eStatus != eST_RUNNING )
	{
		return eJOU_E_NINIT;
	}
	m_pSrv->m_csCache.JouUpdateOperation(pData); //更新缓存区
	return eJOU_R_SUCCESS;
	

}


