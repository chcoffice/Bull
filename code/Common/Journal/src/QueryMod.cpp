#include "QueryMod.h"
#include "Service.h"
#include "JouXml.h"

using namespace JOU;


static void _FreeQueryArgs(void *pData )
{
	
	if( pData )
	{
		::free(pData);
	}
}

CQueryMod::CQueryMod(void)
:CJouModule("QueryMod")
,m_csPool()
,m_fnCallback(NULL)
,m_pUserContent(NULL)
{
	m_csPool.SetUserData(this);
	m_csPool.SetMaxWaitTask(100); //同时最大处理100 个数据
	m_csPool.SetFreedTaskDataFunction(_FreeQueryArgs);
}

CQueryMod::~CQueryMod(void)
{
	m_csPool.Uninit();
}



EnumJouErrno CQueryMod::Start(void* pData)
{
	EnumJouErrno eRet = CJouModule::Start(pData);
	GS_ASSERT_RET_VAL(!eRet, eRet);
	if( !m_csPool.Init((GSThreadPoolCallback)CQueryMod::OnThreadPoolEvent, 10, FALSE) )
	{
		MY_LOG_ERROR(g_pLog,"Init Thread Pool fail.\n");
		m_eStatus = eST_INIT;
		return eJOU_E_UNKNOWN;
	}
	return eJOU_R_SUCCESS;
}

void CQueryMod::Stop(void)
{
	CJouModule::Stop();
	m_csPool.Uninit();
}

EnumJouErrno  CQueryMod::JouAsyncQuery( const StruQueryArgs *pArgs )
{
	StruQueryArgs *pNew  = (StruQueryArgs*)::malloc( sizeof( StruQueryArgs) );
	GS_ASSERT_RET_VAL(pNew, eJOU_E_UNKNOWN);
	::memcpy(pNew, pArgs, sizeof(StruQueryArgs) );
	INT iRet = m_csPool.Task(pNew);
	
	if( iRet)
	{
		_FreeQueryArgs(pNew);
		if( iRet ==CGSThreadPool::EFLOWOUT  )
		{
			return eJOU_E_BUSY;
		}
		else
		{
			return eJOU_E_UNKNOWN;
		}
	}
	return eJOU_R_SUCCESS;
}


EnumJouErrno  CQueryMod::JouSetAsyncQueryCallback( JouFuncPtrAsyncQueryCallback fnCallback, 
												  void *pUserContent )
{
	m_pUserContent = pUserContent;
	m_fnCallback = fnCallback;	
	return eJOU_R_SUCCESS;
}

void CQueryMod::OnThreadPoolEvent( CGSThreadPool *pcsPool, 
								void *TaskData, 
								void *pDebugInfo)
{

CQueryMod *pMod = (CQueryMod*)pcsPool->GetUserData();
	pMod->TaskEntry((StruQueryArgs*)TaskData);
}

void CQueryMod::TaskEntry( StruQueryArgs *pArgs)
{
	
EnumJouErrno eRet = eJOU_R_SUCCESS;
StruQueryResult stResult;
CGSString strSql;
CGSString strTotalSql;
IConnection *pCnn = NULL;
IRecordSet *pRcdSet = NULL;
std::string strValue;
INT i, iCols;
CJouXmlMaker czXml;
std::vector<CGSString> vColNames;


	

	bzero(&stResult, sizeof(stResult) );

	stResult.iCliPmsID = pArgs->iCliID;
	stResult.iCliPmsID = pArgs->iCliPmsID;
	stResult.iCmdTag = pArgs->iCmdTag;
	stResult.pChn = pArgs->pChn;


	if( m_eStatus )
	{
		stResult.eResult = eJOU_E_NINIT;
		goto exit_func;
	}

	

	
	eRet = m_pSrv->MakeFunctionSql(strSql, pArgs->czFuncName, pArgs->czArgs );
	if( eRet )
	{
		stResult.eResult = eRet;
		goto exit_func;
	}

	//计算总条数
	
	strTotalSql = "SELECT COUNT(*) AS NROWTOTAL FROM (";

	strTotalSql += strSql;
	strTotalSql += ") TB_NTOTAL";


	pCnn  = m_pSrv->m_csDB.GetConnection();
	if( !pCnn )
	{
		MSLEEP(1000);
		pCnn = m_pSrv->m_csDB.GetConnection();
		if(!pCnn)
		{
			stResult.eResult = eJOU_E_DB_GETCONN;
			goto exit_func;
		}
	}
	pRcdSet = pCnn->ExecuteQuery(strTotalSql.c_str());
	if( pRcdSet )
	{
		if( pRcdSet->Eof() )
		{
			GS_ASSERT(0);
			stResult.eResult = eJOU_E_DB_ASSERT;
			goto exit_func;
		}
		else if( pRcdSet->GetCollect("NROWTOTAL", strValue ) && strValue.length() )
		{
				stResult.iTotals = atol( strValue.c_str() );
		}
		else
		{
			GS_ASSERT(0);
			stResult.eResult = eJOU_E_DB_ASSERT;
			goto exit_func;
		}

		pRcdSet->ReleaseRecordSet();
		pRcdSet = NULL;
	}
	else
	{
		MY_LOG_ERROR(g_pLog, "JouDB exesql: '%s' fail.\n", strTotalSql.c_str() );		
		stResult.eResult = eJOU_E_DB_EXESQL;
		goto exit_func;
	}
	
	//查询真正结果
	if( pArgs->iPageRows < 1  )
	{
		pArgs->iPageRows = 1000;
	}
	else if( pArgs->iPageRows > 100000  )
	{
		pArgs->iPageRows = 10000;
	}

	if( pArgs->iRowStart<1 )
	{
		pArgs->iRowStart = 1;
	}
	stResult.iRowStart = pArgs->iRowStart;

	pRcdSet = pCnn->ExecutePageQuery(strSql.c_str(), pArgs->iRowStart, pArgs->iPageRows  );
	if( !pRcdSet )
	{
		MY_LOG_ERROR(g_pLog, "JouDB ExecutePageQuery: '%s' fail.\n", strSql.c_str() );		
		stResult.eResult = eJOU_E_DB_EXESQL;
		goto exit_func;
	}

	//获取属性名称
	iCols = pRcdSet->GetColumnNumber();
	if( iCols<1 )
	{
		GS_ASSERT(0);
		stResult.eResult = eJOU_E_DB_ASSERT;
		goto exit_func;
	}
	
	for( i=0; i<iCols; i++ )
	{
		
		if( pRcdSet->GetCloumnName(i, strValue) )
		{
			if( strValue != "R" )
			{
				vColNames.push_back(strValue);
			}
		}
	}
	if( !czXml.Init(vColNames) )
	{
		GS_ASSERT(0);
		stResult.eResult = eJOU_E_UNKNOWN;
		goto exit_func;
	}

	for( ; !pRcdSet->Eof(); pRcdSet->MoveNext() )
	{
		if( !czXml.AddRow() )
		{
			GS_ASSERT(0);
			stResult.eResult = eJOU_E_NMEM;
			goto exit_func;
		}
		for(iCols = 0; iCols < (int) vColNames.size(); iCols++  )
		{
			if( pRcdSet->GetCollect(vColNames[iCols].c_str(), strValue) )
			{
				if(!czXml.PutRowValue(iCols, strValue ) )
				{
					GS_ASSERT(0);
					stResult.eResult = eJOU_E_UNKNOWN;
					goto exit_func;
				}
			}
			else
			{
				GS_ASSERT(0);
				stResult.eResult = eJOU_E_DB_ASSERT;
				goto exit_func;
			}
			
		}
		stResult.iRows++;
	}
	if( !czXml.SerialToXml(strValue) )
	{
		GS_ASSERT(0);
		stResult.eResult = eJOU_E_NMEM;
		goto exit_func;
	}
	stResult.iResultSize = strValue.length()+1;
	stResult.pResultData = (void*) strValue.c_str();


exit_func :
	if( pRcdSet )
	{
		pRcdSet->ReleaseRecordSet();
		pRcdSet = NULL;
	}
	if( pCnn )
	{
		pCnn->ReleaseConnection();
		pCnn = NULL;
	}
	_FreeQueryArgs(pArgs);
	if( m_fnCallback )
	{
		m_fnCallback(&stResult, m_pUserContent);
	}

}

