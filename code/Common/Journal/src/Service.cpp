#include "Service.h"
using namespace JOU;


/*
*********************************************************************
*
*@brief :  CService::CSqlFunction
*
*********************************************************************
*/
EnumJouErrno CService::CSqlFunction::Make( const char czArgs[JOU_ARG_LEN], CGSString &oStrVal)
{
std::vector<CGSString> vVArgs;
	oStrVal.clear();
	if( czArgs[0] )
	{
		GSStrUtil::Split(vVArgs, czArgs, ";" );
	}

	if( (1+vVArgs.size() ) != m_vBase.size() )
	{
		MY_LOG_WARN(g_pLog, "Args: [%s] Invalid of [%s]\n",
			czArgs, m_strFunSql.c_str() );
		return eJOU_E_INVALID;
	}
	oStrVal = m_vBase[0];
	for( UINT i = 0; i<vVArgs.size(); i++ )
	{
		oStrVal += vVArgs[i];
		oStrVal += m_vBase[i+1];
	}
	return eJOU_R_SUCCESS;
	
}


BOOL CService::CSqlFunction::ParserFunc( const CGSString &strFuncSql)
{
	m_vBase.clear();
	GSStrUtil::Split( m_vBase, strFuncSql,"?");
	if( m_vBase.size()==0 )
	{
		GS_ASSERT(0);
		return FALSE;
	}
	else
	{
		if( strFuncSql[strFuncSql.length()-1] == '?' )
		{
			m_vBase.push_back("");
		}
	}
	m_strFunSql = strFuncSql;
	return TRUE;
}


/*
*********************************************************************
*
*@brief : 
*
*********************************************************************
*/


CService::CService(void)
:CJouObj()
,m_vMoudles()
,m_csFuncSet()
,m_csWRMutex()
,m_csLog()
,m_csCfg()
,m_csDB()
,m_csWriter()
,m_csRcdMng()
,m_csQuery()
,m_csCache()

{
	m_vMoudles.push_back( &m_csDB );
	m_vMoudles.push_back( &m_csCache );
	m_vMoudles.push_back( &m_csRcdMng );
	m_vMoudles.push_back( &m_csWriter );
	m_vMoudles.push_back( &m_csQuery );

}

CService::~CService(void)
{
	std::map<CGSString, CSqlFunction *>::iterator csIt;
	for( csIt = m_csFuncSet.begin(); csIt!=m_csFuncSet.end();csIt++ )
	{
		delete (*csIt).second;
	}
	m_csFuncSet.clear();
}

EnumJouErrno CService::Init( const char *czConfFilename,void* pConnectionPoolArgs)
{
	//设置线程池
	CGSThreadPool::InitModule();


	//加载配置
	m_csCfg.LoadConfig(czConfFilename);

	//设置日志
	m_csLog.SetLogPath(m_csCfg.m_strLogDir.c_str() );
	m_csLog.SetLogLevel(CLog::DIR_CONSOLE, (CLog::EnumLogLevel)m_csCfg.m_iLogLevelConsole);
	m_csLog.SetLogLevel(CLog::DIR_FILE, (CLog::EnumLogLevel)m_csCfg.m_iLogLevelFile);

	EnumJouErrno eRet = eJOU_R_SUCCESS, eTemp;
	std::list<CJouModule *>::iterator csIt;
	CJouModule *pMod;
	for( csIt = m_vMoudles.begin(); csIt!=m_vMoudles.end(); csIt++ )
	{
		pMod = *csIt;
		if( (eTemp = pMod->Init(this)) )
		{
			eRet = eTemp;
			MY_LOG_FATAL(g_pLog, "Module %s Init fail. %s\n",
				pMod->GetModuleName().c_str(), JouGetError(eTemp) );
		}
		else
		{
			MY_LOG_DEBUG(g_pLog, "Module %s Init OK.\n",
				pMod->GetModuleName().c_str());
		}
	}
	//GS_ASSERT(eRet==eJOU_R_SUCCESS);


	for( csIt = m_vMoudles.begin(); csIt!=m_vMoudles.end(); csIt++ )
	{
		pMod = *csIt;
		
		if( (eTemp = pMod->Start(pConnectionPoolArgs)) )
		{

			MY_LOG_FATAL(g_pLog, "Module %s Start fail. %s\n",
				pMod->GetModuleName().c_str(), JouGetError(eTemp) );
		}
		else
		{
			MY_LOG_DEBUG(g_pLog, "Module %s Start OK.\n",
				pMod->GetModuleName().c_str());
		}
	}

	return eRet;


}


void CService::Uninit(void)
{
	std::list<CJouModule *>::iterator csIt;
	for( csIt = m_vMoudles.begin(); csIt!=m_vMoudles.end(); csIt++ )
	{
		(*csIt)->Stop();		
		MY_LOG_DEBUG(g_pLog, "Module %s Stop OK.\n",
			(*csIt)->GetModuleName().c_str());	
	}

	for( csIt = m_vMoudles.begin(); csIt!=m_vMoudles.end(); csIt++ )
	{
		(*csIt)->Uninit();		
		MY_LOG_DEBUG(g_pLog, "Module %s Stop OK.\n",
			(*csIt)->GetModuleName().c_str());	
	}

	CGSThreadPool::UninitModule();
}


EnumJouErrno CService::MakeFunctionSql(CGSString &oStrSql, 
							 const CGSString &strFuncName, 
							 const char czArgs[JOU_ARG_LEN] )
{
	CGSAutoReaderMutex rlocker( &m_csWRMutex);

CSqlFunction *pSqlFunc = NULL;
std::map<CGSString, CSqlFunction *>::iterator csIt;
	csIt = m_csFuncSet.find( strFuncName );
	if( csIt != m_csFuncSet.end() )
	{
		pSqlFunc = (*csIt).second;
	}
	else
	{
		m_csWRMutex.UnlockReader();
		m_csWRMutex.LockWrite();
		csIt = m_csFuncSet.find( strFuncName );
		if( csIt != m_csFuncSet.end() )
		{
			pSqlFunc = (*csIt).second;
		}
		else
		{


			IConnection *pCnn = NULL;
			IRecordSet *pSet = NULL;
			CGSString strSql = "SELECT STR_SQL FROM TB_JOU_FUNC t WHERE t.FUNC_NAME='";
			EnumJouErrno eRet = eJOU_R_SUCCESS;
			std::string strValue;
			oStrSql.clear();
			strSql += strFuncName;
			strSql += "'";

			do {

				pCnn = m_csDB.GetConnection();
				if( !pCnn )
				{
					eRet =  eJOU_E_DB_GETCONN;
					break;
				}
				pSet = pCnn->ExecuteQuery(strSql.c_str() );
				if( !pSet )
				{
					GS_ASSERT(0);
					eRet = eJOU_E_DB_EXESQL;
					break;
				}
				if( pSet->Eof() )
				{
					eRet =  eJOU_E_INVALID;
					break;
				}

				if( !pSet->GetCollect("STR_SQL", strValue ) )
				{
					GS_ASSERT(0);
					eRet =  eJOU_E_DB_ASSERT;
					break;
				}
				CSqlFunction *pt = new CSqlFunction();
				if( !pt )
				{
					GS_ASSERT(0);
					eRet =  eJOU_E_NMEM;
					break;	
				}
				if( !pt->ParserFunc(strValue ) )
				{
					GS_ASSERT(0);
					eRet =  eJOU_E_UNKNOWN;					
					delete pt;
					break;	
				}
				m_csFuncSet[strFuncName] =  pt;
				pSqlFunc = pt;
			}while(0);

			if( pSet )
			{
				pSet->ReleaseRecordSet();
			}
			if( pCnn )
			{
				pCnn->ReleaseConnection();
			}
		}
		m_csWRMutex.UnlockWrite();
		m_csWRMutex.LockReader();
	}
	if( !pSqlFunc )
	{
		return eJOU_E_INVALID;
	}

	return pSqlFunc->Make(czArgs, oStrSql);
}
