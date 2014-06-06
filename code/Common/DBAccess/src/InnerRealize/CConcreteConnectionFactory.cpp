// 头文件
#include "CConcreteConnectionFactory.h"
using	namespace	DBAccessModule;
CConcreteConnectionFactory::CConcreteConnectionFactory(void)
{
}

CConcreteConnectionFactory::~CConcreteConnectionFactory(void)
{
}

/**************************************************************************************************
  Function: CreateOracleConnection    
  DateTime: 2010/5/23 21:39	
  Description: 创建一个Oracle数据库连接对象
  Input:   	
		  szServer ：   数据库服务器地址
		  szDatabase：  数据库名称
		  szUser  ：    数据库用户名
		  szPass  ：    数据库密码
  Output:  NULL
  Return:  CConnection*
  Note:	   创建连接对象
**************************************************************************************************/
CConnection*		CConcreteConnectionFactory::CreateOracleConnection(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass)
{
	COracleConnection*	pOracleConnection = NULL;
	pOracleConnection = new COracleConnection();
	if (pOracleConnection != NULL)
	{
		// 设置连接ID
		pOracleConnection->SetConnectID(GenerateAutoConnectID());
		// 初始化信息
		pOracleConnection->Initial(szServer,szDatabase,szUser,szPass);
		// 打开数据库连接
		if (pOracleConnection->Open())
		{
			// 返回连接对象
			return (CConnection*)pOracleConnection;
		}		
		else
		{
			// 如果数据库打开失败，释放对象
			delete pOracleConnection;
			pOracleConnection = NULL;
		}
	}

	return	(CConnection*)NULL;

}


/**************************************************************************************************
  Function		: CreateSqlConnection(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass)    
  DateTime		: 2010/6/7 14:27	
  Description	: SqlServer 连接对象具体工厂创建方法
  Input			: 
				  szServer ：   数据库服务器地址
				  szDatabase：  数据库名称
				  szUser  ：    数据库用户名
				  szPass  ：    数据库密码
  Output		: NULL
  Return		: CConnection*
  Note			: Sql Server数据库连接对象
**************************************************************************************************/
CConnection*		CConcreteConnectionFactory::CreateSqlConnection(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass)
{

	CSqlServerConnection*	pSqlServerConnection = NULL;
	pSqlServerConnection = new CSqlServerConnection();
	if (pSqlServerConnection != NULL)
	{
		// 设置连接ID
		pSqlServerConnection->SetConnectID(GenerateAutoConnectID());
		// 初始化信息
		pSqlServerConnection->Initial(szServer,szDatabase,szUser,szPass);
		// 打开数据库连接
		if (pSqlServerConnection->Open())
		{
			// 返回连接对象
			return (CConnection*)pSqlServerConnection;
		}		
		else
		{
			delete	pSqlServerConnection;
			pSqlServerConnection = NULL;
		}
	}

	return	(CConnection*)NULL;
}


/**************************************************************************************************
Function		: CreateAccessConnection(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass)    
DateTime		: 2010/6/7 14:27	
Description		: Access 连接对象具体工厂创建方法
Input			: 
					szServer ：   数据库服务器地址
					szDatabase：  数据库名称
					szUser  ：    数据库用户名
					szPass  ：    数据库密码
Output			: NULL
Return			: CConnection*
Note			: Access数据库连接对象
**************************************************************************************************/
CConnection*		CConcreteConnectionFactory::CreateAccessConnection(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass)
{
	return NULL;
}

/**************************************************************************************************
  Function		: CreateMySQLConnection(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass)
  DateTime		: 2011/4/14 17:36	
  Author 		: yopo  
  Description	: MySQL 连接对象具体工厂创建方法
  Input			:
					szServer ：   数据库服务器地址
					szDatabase：  数据库名称
					szUser  ：    数据库用户名
					szPass  ：    数据库密码
  Output		: NULL
  Return		: CConnection*
  Note			: MySQL数据库连接对象
**************************************************************************************************/
CConnection*		CConcreteConnectionFactory::CreateMySQLConnection(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass)
{
	CMySQLConnection*	pMySQLConnection = NULL;
	pMySQLConnection = new CMySQLConnection();
	if (pMySQLConnection != NULL)
	{
		// 设置连接ID
		pMySQLConnection->SetConnectID(GenerateAutoConnectID());
		// 初始化信息
		pMySQLConnection->Initial(szServer,szDatabase,szUser,szPass);
		// 打开数据库连接
		if (pMySQLConnection->Open())
		{
			// 返回连接对象
			return (CConnection*)pMySQLConnection;
		}		
		else
		{
			// 如果数据库打开失败，释放对象
			delete pMySQLConnection;
			pMySQLConnection = NULL;
		}
	}
	return	(CConnection*)NULL;
}
/**************************************************************************************************
  Function: CreateOciConnection    
  DateTime: 2010/5/23 21:39	
  Description: 创建一个Oci数据库连接对象
  Input:   	
		  szServer ：   数据库服务器地址
		  szDatabase：  数据库名称
		  szUser  ：    数据库用户名
		  szPass  ：    数据库密码
  Output:  NULL
  Return:  CConnection*
  Note:	   创建连接对象
**************************************************************************************************/
CConnection*		CConcreteConnectionFactory::CreateOciConnection(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass)
{
	COciConnection*	pOciConnection = NULL;
	pOciConnection = new COciConnection();
	if (pOciConnection != NULL)
	{
		// 设置连接ID
		pOciConnection->SetConnectID(GenerateAutoConnectID());
		// 初始化信息
		pOciConnection->Initial(szServer,szDatabase,szUser,szPass);
		// 打开数据库连接
		if (pOciConnection->Open())
		{
			// 返回连接对象
			return (CConnection*)pOciConnection;
		}		
		else
		{
			// 如果数据库打开失败，释放对象
			delete pOciConnection;
			pOciConnection = NULL;
		}
	}

	return	(CConnection*)NULL;

}