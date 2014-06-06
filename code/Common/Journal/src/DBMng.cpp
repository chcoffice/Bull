/*******************************************************
Copyright (C), 2010-2011, GOSUN 
File name   : DATABASEMANAGER.CPP      
Author      : jiangshx     
Version     : Vx.xx        
DateTime    : 2010/11/12 9:39
Description :  数据库范围接口操作对象
*******************************************************/
#include "DBMng.h"
#include "Service.h"

using namespace JOU;
using namespace std;



/**************************************************************************
Function    : CDBManager::CDBManager    
DateTime    : 2010/11/12 10:26	
Description : 构造函数
Input       : 
Output      : 
Return      : 
Note        :	
**************************************************************************/
CDBManager::CDBManager(void)
:CJouModule("DBManager")
{

    m_pConnPool = NULL;
	m_pOutConnPool = NULL;

}

/**************************************************************************
Function    : CDBManager::~CDBManager    
DateTime    : 2010/11/12 10:26	
Description : 析构函数
Input       : 
Output      : 
Return      : 
Note        :	
**************************************************************************/
CDBManager::~CDBManager(void)
{
    m_strServer = "";
    m_strDatabase = "";
    m_strUser = "";
    m_strPWD = "";
    m_eDbaseType = ORACLE;
}



EnumJouErrno CDBManager::Init( CService *pServer )
{
EnumJouErrno eErrno;
	eErrno = CJouModule::Init(pServer);
	GS_ASSERT_RET_VAL(!eErrno, eErrno );


	
	m_strServer = pServer->m_csCfg.m_strDBHostname;
	m_strDatabase = pServer->m_csCfg.m_strDBName;
	m_strUser =  pServer->m_csCfg.m_strDBUser;
	m_strPWD =  pServer->m_csCfg.m_strDBPWD;
	m_eDbaseType =  (EnumDatabaseType)pServer->m_csCfg.m_eDbaseType;
	return eJOU_R_SUCCESS;

}




EnumJouErrno CDBManager::Start(void* pData)
{
	EnumJouErrno eRet = CJouModule::Start(pData);
	GS_ASSERT_RET_VAL(!eRet, eRet );

	if (pData != NULL  )
	{
		StruConnectPoolArgs *pArgs = (StruConnectPoolArgs*)pData;
		if( pArgs->pConnectPool )
		{
			m_pConnPool = (IConnectionPool*) pArgs->pConnectPool;
			m_pOutConnPool = m_pConnPool;
			m_strServer = pArgs->szServer;
			m_strDatabase = pArgs->szDatabase;
			m_strUser =  pArgs->szUser;
			m_strPWD =  pArgs->szPWD;
			m_eDbaseType =  (EnumDatabaseType)pArgs->eDbaseType;
		}
	}
	else 
	{		
		//加载数据库模块
		if (!StartDBAccessModule())
		{
			return eJOU_E_DB_MODULE;
		}
		//获取连接池,注:此接口

		m_pConnPool = GetConnectionPoolInstance();
		if(m_pConnPool == NULL)
		{
			StopDBAccessModule();
			return eJOU_E_DB_CONNPOOL;
		}

	}
	
    IConnection *pCnn  = GetConnection();
    if( pCnn )
    {		
        pCnn->ReleaseConnection();
        return eJOU_R_SUCCESS;
    }
	MY_LOG_ERROR( g_pLog, "Connect DB:'%s@%s' USER:'%s' PWD:'%s' Type:%d FAIL.\n",
			 m_strDatabase.c_str() ,m_strServer.c_str(),
			 m_strUser.c_str(), m_strPWD.c_str(), 
			 (INT)m_eDbaseType );
    return eJOU_E_DB_GETCONN;
}

/********************************************************************
Function	:	CDBManager::StopRoutine
DateTime 	:	2010/18/8  19:03     
Description :   数据库服务停止  
Input	 	:	
Output 		:	
Return		: 	[EnumRetCode] 参见返回值类型定义
Note		:		
*********************************************************************/
void CDBManager::Stop(void)
{
    //卸载数据库模块
	if( m_eDbaseType )
	{
		if( m_pOutConnPool != m_pConnPool )
		{
			
			if( m_pConnPool )
			{
				delete m_pConnPool;
				m_pConnPool = NULL;
			}
			StopDBAccessModule();
		}
	}
	CJouModule::Stop();
}


/********************************************************************
Function	: CDBManager::ConnectDB
DateTime 	: 2010/18/8  19:10     
Description : 连接数据库  
Input       : szServer :   数据库服务器地址
szDatabase:  数据库名称
szUser  :    数据库用户名
szPass  :    数据库密码
iDbType :    数据库类型[sql ,oracle 等]
ORACLE		 0		// oracle 
SQLSERVER	 1		// SqlServer
MYSQL		 2		// MySql 
ACCESS		 3		// Access
EXCEL		 4		// Excel数据库
Output 		:	
Return		: [EnumRetCode] 参见返回值类型定义
Note		:		
*********************************************************************/
IConnection *CDBManager::GetConnection(void)
{

    return m_pConnPool->GetConnection(m_strServer.c_str(),
        m_strDatabase.c_str(),
        m_strUser.c_str(),
        m_strPWD.c_str(),
        m_eDbaseType);
}


