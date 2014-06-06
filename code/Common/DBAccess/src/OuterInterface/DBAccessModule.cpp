// DBAccessModule.cpp : 定义 DLL 应用程序的导出函数。
//
#include "IDBAccessModule.h"
#include "../InnerRealize/CDBConnectionPool.h"

using	namespace	DBAccessModule;

/**************************************************************************************************
Function: StartDBAccessModule    
DateTime: 2010/5/23 10:51	
Description: 启动数据库模块
Input:       NULL
Output:      NULL
Return:      TRUE:成功  FALSE:失败
Note:		   提供启动数据库模块服务，该函数不提供外部调用者使用
　			 只提供底层服务启动的启动接口
 **************************************************************************************************/
BOOL	DBAccessModule::StartDBAccessModule()
{
//#ifdef _WIN32
#if  0
	// ADO数据库环境初始化
	::CoInitialize(NULL);
#else
// linux
#endif
	// 创建连接池
	return	CDBConnectionPool::Instance() == NULL ? FALSE : TRUE;

}

/**************************************************************************************************
Function:  GetConnectionPoolInstance
DateTime: 2010/5/23 10:15	
Description:  获取连接池对象
Input:        NULL
Output:       NULL
Return:       IConnectionPool*指针对象
Note:		    返回的连接池对象满足单例实例
前提条件：StartDBAccessModule成功，底层是单例，和StartDBAccessModule 的功能一样的
**************************************************************************************************/
IConnectionPool*	DBAccessModule::GetConnectionPoolInstance()
{
	// 返回连接池对象
	return	CDBConnectionPool::Instance() == NULL ? NULL : (IConnectionPool*)CDBConnectionPool::Instance();

}

IConnectionPool* DBAccessModule::CreateConnectionPool()
{
	return	CDBConnectionPool::Create();
}


/**************************************************************************************************
Function: StopDBAccessModule    
DateTime: 2010/5/23 10:51	
Description: 停止数据库模块服务，并内部释放所有的资源
Input:       NULL
Output:      NULL
Return:      TRUE : 成功，FALSE:失败
Note:		　该函数不提供外部调用者使用，只提供底层服务退出的退出接口
**************************************************************************************************/
BOOL	DBAccessModule::StopDBAccessModule()
{
	// 释放ADO库
	//#ifdef _WIN32
#if  0
	::CoUninitialize();
#else
	// linux
#endif
	return	CDBConnectionPool::Instance() != NULL ? CDBConnectionPool::Instance()->Release() : FALSE;

}

/**************************************************************************************************
Function		: ErrorLogInfo(SQLHENV		hEnv,SQLHDBC	hDbc,SQLHSTMT	hStmt)
DateTime		: 2010/11/30 13:49	
Description		: 输出错误信息的函数
Input			: hEnv:数据库环境句柄，hDbc：数据库链接句柄，hStmt：数据库查询语句句柄
Output			: NULL
Return			: TRUE
Note			:
**************************************************************************************************/
BOOL	DBAccessModule::ErrorLogInfo(SQLHENV hEnv,SQLHDBC hDbc,SQLHSTMT hStmt, INT *piErrno)
{
	SQLCHAR		szTEMP[1024]={0};
	SQLSMALLINT	ItEMP = 0;
    SQLINTEGER iErrno = 0;
	char szState[32];
	memset(szState,0,sizeof(szState));
	SQLError(hEnv,hDbc,hStmt,(SQLCHAR *)szState,&iErrno,szTEMP,1024,(SQLSMALLINT*)&ItEMP);
	std::cout <<"---> DBAccess " << " Errno:" << iErrno << ", Error： "<< szTEMP << "Status:"<< szState << endl;
    if( piErrno )
    {
       *piErrno = iErrno;


	   if(0== strncmp(szState, "01002",5)  ||
		   0== strncmp(szState, "08001",5)  ||
		   0== strncmp(szState, "08002",5)  ||
		   0== strncmp(szState, "08003",5)  ||
		   0== strncmp(szState, "08004",5)  || 
		   0== strncmp(szState, "08007",5)  ||
		   0== strncmp(szState, "S1T00",5)  )
	   {
		   printf("ODBC errno: %d,  error: %s\n", iErrno, szState);
		   *piErrno = 3114; //ERRNO_NETWORK_DISCNN;
	   }
	   
    }
	return TRUE;
}

