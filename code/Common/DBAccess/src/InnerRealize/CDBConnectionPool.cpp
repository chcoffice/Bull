#include "CDBConnectionPool.h"

using	namespace	DBAccessModule;



#define OCI_DLL32_DIR _T("ociliba.dll")
#define OCI_DLL64_DIR _T("ociliba.dll")
#define CLIENT_DIR "ocilib_client"


// 底层模块库的加载,在外层中添加
//#ifdef _DEBUG
//#pragma  comment(lib, "CommonLibsD.lib")
//#else
//#pragma	 comment(lib,"CommonLibs.lib")
//#endif


CDBConnectionPool*		CDBConnectionPool::m_pInstance = NULL;
CGSMutex				CDBConnectionPool::m_CsSingleton;
OciDllDeal*				DBAccessModule::m_pOciDeal = NULL;

CDBConnectionPool::CDBConnectionPool(void)
{
	m_pConnectionFactory = NULL;
}

// 析构函数
CDBConnectionPool::~CDBConnectionPool(void)
{
	// 释放对象
	if (m_pConnectionFactory!=NULL)
	{
		delete	m_pConnectionFactory;
		m_pConnectionFactory = NULL;
	}

	Release();
	
	if(m_pOciDeal != NULL)
	{
		m_pOciDeal->UnLoad_Oci();
		delete m_pOciDeal;
		m_pOciDeal = NULL;
	}
}

//---------------------------------------------------------------------------------------------
// 单例的实现
//---------------------------------------------------------------------------------------------
/**************************************************************************************************
  Function: Instance    
  DateTime: 2010/5/25 22:15	
  Description:    	创建单例对象
  Input:          	NULL
  Output:         	NULL
  Return:         	CDBConnectionPool*类型的指针变量
  Note:				
**************************************************************************************************/
CDBConnectionPool*	CDBConnectionPool::Instance()
{
	if (m_pInstance == NULL)
	{
		CGSAutoMutex	AutoLock(&m_CsSingleton);
		if (m_pInstance == NULL)
		{
			m_pInstance = new CDBConnectionPool();
		}
	}

	return	m_pInstance;

}

/**************************************************************************************************
  Function: Release    
  DateTime: 2010/5/25 22:14	
  Description:    	释放单例资源
  Input:          	NULL
  Output:         	NULL
  Return:         	
					TRUE:操作成功
  Note:				
**************************************************************************************************/
BOOL	CDBConnectionPool::Release()
{
	m_CsConnectionPool.Lock();
	DeleteAllConnection();
	m_CsConnectionPool.Unlock();

	if(this == m_pInstance )
	{
		// 关闭所有的连接对象，释放连接对象
		

		//delete m_pInstance;
		m_pInstance = NULL;
	}

	return TRUE;	
}

//---------------------------------------------------------------------------------------------
// OuterInterface [5/22/2010 liujs] 
//---------------------------------------------------------------------------------------------

/**************************************************************************************************
  Function:  GetConnection   
  DateTime: 2010/5/25 22:12	
  Description:      从连接池中取出一个连接对象，供外部使用
  Input:          	
					  szServer ：   数据库服务器地址
					  szDatabase：  数据库名称
					  szUser  ：    数据库用户名
					  szPass  ：    数据库密码
					  iDbType ：     数据库类型[sql ,oracle 等]

  Output:         	NULL
  Return:         	连接对象指针IConnection*
  Note:				
**************************************************************************************************/
IConnection*	CDBConnectionPool::GetConnection(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass,const INT iDbType)
{
	CGSAutoMutex	AutoLock(&m_CsConnectionPool);

	IConnection *p = (IConnection*)GetOneConnectionFromPool();
	if( p )
	{
		return p;
	}

	// 判断连接池是否有连接存在
	if (InputDataVerify(szServer,szDatabase,szUser,szPass,iDbType) == FALSE)
	{
		return	NULL;
	}

	// 没有创建，就创建连接对象，或者都在使用且没有到达最大数目就在创建5个
	if ( GetConnectionPoolNum()==0  || GetConnectionPoolNum() < coniConnectionPoolMaxMum )
	{
		// 批量创建连接对象，存入连接池中,每次创建 coniConnectionBatchNum 个，创建失败，返回NULL
		BatchCreateConnHandle(szServer,szDatabase,szUser,szPass,iDbType,coniConnectionBatchNum);		
		
	}

	// 从连接池中获取连接对象
	return	(IConnection*)GetOneConnectionFromPool();
}


/**************************************************************************************************
  Function: InputDataVerify    
  DateTime: 2010/5/25 20:48	
  Description:    	判断用户输入条件是否准确
  Input:          	
					  szServer ：   数据库服务器地址
					  szDatabase：  数据库名称
					  szUser  ：    数据库用户名
					  szPass  ：    数据库密码
					  iDbType ：     数据库类型[sql ,oracle 等]
  Output:         	NULL
  Return:         	TRUE : 输入合法   FALSE :输出错误
  Note:				
**************************************************************************************************/
BOOL			CDBConnectionPool::InputDataVerify(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass,const INT iDbType)
{
	// 校验信息
	if (strlen(szServer) > coniDataLen || strlen(szServer) <= coniZero)
	{
		return	FALSE;
	}

	if( strlen(szDatabase) > coniDataLen || strlen(szDatabase) <= coniZero)
	{
		return	FALSE;
	}

	if( strlen(szUser) > coniDataLen || strlen(szUser) <= coniZero)
	{
		return	FALSE;
	}

	if( strlen(szPass) > coniDataLen || strlen(szPass) <= coniZero)
	{
		return	FALSE;
	}

	if (iDbType == ORACLE  || iDbType == SQLSERVER || iDbType == MYSQL || iDbType == OCI)
	{
		return	TRUE;
	}

	return FALSE;

}


//---------------------------------------------------------------------------------------------
// 创建对象
//---------------------------------------------------------------------------------------------
/**************************************************************************************************
Function: BatchCreateConnHandle    
DateTime: 2010/5/25 20:32	
Description:    	   创建当个连接对象,利用具体的工厂对象创建
Input:          	
				       szServer ：   数据库服务器地址
					   szDatabase：  数据库名称
					   szUser  ：    数据库用户名
					   szPass  ：    数据库密码
					   iDbType ：     数据库类型[sql ,oracle 等]
Output:         	   NULL
Return:         	  
				       TRUE:成功
	                   FALSE:失败
Note:				// 备注
**************************************************************************************************/
CConnection*    CDBConnectionPool::CreateConnectionHandle(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass,const INT iDbType)
{
	// 数据库连接对象
	CConnection*	pConnection = NULL;
	switch (iDbType)
	{
	case ORACLE:
		// ORACLE 数据库
		{	
			if (m_pConnectionFactory == NULL)
			{	
				// 创建工厂
				m_pConnectionFactory	=	new CConcreteConnectionFactory();		
			}
			// 创建对象
			pConnection	=	m_pConnectionFactory->CreateOracleConnection(szServer,szDatabase,szUser,szPass);
		}		
		break;

	case MYSQL:
		// MySQL 数据库
		{	
			if (m_pConnectionFactory == NULL)
			{	
				// 创建工厂
				m_pConnectionFactory	=	new CConcreteConnectionFactory();		
			}
			// 创建对象
			pConnection	=	m_pConnectionFactory->CreateMySQLConnection(szServer,szDatabase,szUser,szPass);
		}		
		break;

	case SQLSERVER:
		// Sql Server 数据库
		{
			if (m_pConnectionFactory == NULL)
			{	
				// 创建工厂
				m_pConnectionFactory	=	new CConcreteConnectionFactory();		
			}
			// 创建对象
			pConnection	=	m_pConnectionFactory->CreateSqlConnection(szServer,szDatabase,szUser,szPass);

		}
		break;
	case ACCESS:
		// Access 数据库
		{
			if (m_pConnectionFactory == NULL)
			{	
				// 创建工厂
				m_pConnectionFactory	=	new CConcreteConnectionFactory();		
			}
			// 创建对象
			pConnection	=	m_pConnectionFactory->CreateAccessConnection(szServer,szDatabase,szUser,szPass);

		}
		break;
	case OCI:
		// OCI
		{	
			//OCI环境初始化
			if(m_pOciDeal == NULL)
			{
				m_pOciDeal = new OciDllDeal;
				if(!m_pOciDeal->Load_Oci(OCI_DLL32_DIR,OCI_DLL64_DIR))
				{
					delete m_pOciDeal;
					m_pOciDeal = NULL;
					return NULL;
				}
			}
			if (!m_pOciDeal->pOCI_Initialize(NULL, NULL, OCI_ENV_DEFAULT)) 
			{
				delete m_pOciDeal;
				m_pOciDeal = NULL;
				return NULL;
			}
	
			if (m_pConnectionFactory == NULL)
			{	
				// 创建工厂
				m_pConnectionFactory	=	new CConcreteConnectionFactory();		
			}
			// 创建对象
			pConnection	=	m_pConnectionFactory->CreateOciConnection(szServer,szDatabase,szUser,szPass);
		}		
		break;
	
		// 别的数据库，只需在此处创建就可以
	default:
		break;
	}

	// 返回创建成功的对象
	return  pConnection;

}


/**************************************************************************************************
  Function: BatchCreateConnHandle    
  DateTime: 2010/5/25 20:32	
  Description:    	  批量创建对象,添加到连接池中
  Input:          	
					  szServer ：   数据库服务器地址
					  szDatabase：  数据库名称
					  szUser  ：    数据库用户名
					  szPass  ：    数据库密码
					  iDbType ：     数据库类型[sql ,oracle 等]
  Output:         	  NULL
  Return:         	  
					  TRUE:成功
					  FALSE:失败
  Note:				// 备注
**************************************************************************************************/
BOOL            CDBConnectionPool::BatchCreateConnHandle(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass,const INT iDbType,const INT iConnNum)
{
	BOOL  bRet = FALSE;
	// 创建20个连接对象
	for (int i=0;i< iConnNum ;i++)
	{
		CConnection*	pConnection = NULL;
		// 功过工厂创建对象
		pConnection = CreateConnectionHandle(szServer,szDatabase,szUser,szPass,iDbType);

		if(pConnection != NULL)
		{
			bRet = TRUE;
			// 添加到队列
			AddConnecttion(pConnection);
		}
		else
		{
			bRet = FALSE;
			break;
		}
	}

	return bRet;

}





//---------------------------------------------------------------------------------------------
// 连接池队列的相关操作
//---------------------------------------------------------------------------------------------

/**************************************************************************************************
Function: AddConnecttion    
DateTime: 2010/5/25 19:29	
Description:    	添加连接对象到队列中
Input:          	pConnection:连接对象
Output:         	NULL
Return:         	
					TRUE: 操作成功
					FALSE: 操作失败
Note:				// 备注
**************************************************************************************************/
BOOL            CDBConnectionPool::AddConnecttion(CConnection*   pConnection)
{
	if (pConnection != NULL)
	{		
		m_ConnectionPool.push_back(pConnection);
		return	TRUE;
	}

	return  FALSE;

}

/**************************************************************************************************
  Function: DeleteConnection    
  DateTime: 2010/5/25 19:29	
  Description:    	删除队列中的连接对象
  Input:          	pConnection:连接对象
  Output:         	NULL
  Return:         	
					TRUE: 操作成功
					FALSE: 操作失败
  Note:				// 备注
**************************************************************************************************/
BOOL            CDBConnectionPool::DeleteConnection(CConnection* pConnection)
{
	if (pConnection!=NULL)
	{	
		for (Connection_Vector_Iterator	iter = m_ConnectionPool.begin();iter != m_ConnectionPool.end();iter++)
		{
			// 连接号相等就表示相等
			if (pConnection->GetConnectID() == (*iter)->GetConnectID())
			{
				CConnection*	pCsConnnection = NULL;
				pCsConnnection = (*iter);
				if (pCsConnnection != NULL)
				{
					pCsConnnection->Close();
					delete pCsConnnection;
					pCsConnnection = NULL;
				}

				// 从容器中删除
				m_ConnectionPool.erase(iter);
				break;
			}
		}
	}
	return  FALSE;

}

/**************************************************************************************************
  Function: DeleteAllConnection    
  DateTime: 2010/5/25 19:28	
  Description:    	删除连接池中所有的连接
  Input:          	NULL
  Output:         	NULL
  Return:         	
					TRUE:成功
					FALSE:失败
  Note:				
**************************************************************************************************/
BOOL			CDBConnectionPool::DeleteAllConnection()
{
	for (Connection_Vector_Iterator	iter = m_ConnectionPool.begin();iter != m_ConnectionPool.end();iter++)
	{
		CConnection*	pConnectionObj = NULL;
		pConnectionObj = (*iter);
		if (pConnectionObj != NULL)
		{
			// 关闭
			pConnectionObj->Close();

			delete pConnectionObj;
			pConnectionObj = NULL;
		}
	}

	// 清空队列
	m_ConnectionPool.erase(m_ConnectionPool.begin(),m_ConnectionPool.end());
	return	TRUE;

}


/**************************************************************************************************
  Function: GetOneConnectionFromPool    
  DateTime: 2010/5/25 22:03	
  Description:    	获取一个有效的连接
  Input:          	NULL
  Output:         	NULL
  Return:         	返回一个有效的连接对象
  Note:				
**************************************************************************************************/
CConnection*	CDBConnectionPool::GetOneConnectionFromPool()
{
	
	for (Connection_Vector_Iterator	iter = m_ConnectionPool.begin();iter != m_ConnectionPool.end();iter++)
	{
		CConnection*	pConnectionObj = NULL;
		pConnectionObj = (*iter);
		if (pConnectionObj != NULL)
		{
			// 空闲的
			if (pConnectionObj->GetConnectionUseFlag() == CONNECTION_IDLESSE)
			{
				// 设为使用
				pConnectionObj->SetConnectionUseFlag(CONNECTION_OCCUPY);
				pConnectionObj->m_pCnnPool = (CDBConnectionPool*)this;
				return	pConnectionObj;
			}
		}
	}
	return NULL ;

}
/**************************************************************************************************
  Function: GetConnectionPoolNum
  DateTime: 2010/5/25 20:43	
  Description:    	获取连接池元素个数    
  Input:          	NULL
  Output:         	NULL
  Return:           连接池元素个数
  Note:				
**************************************************************************************************/
INT				CDBConnectionPool::GetConnectionPoolNum()
{	
	return (INT)m_ConnectionPool.size();

}

/**************************************************************************************************
  Function: ReleaseConnection     
  DateTime: 2010/5/25 19:26	
  Description:    	释放连接，将连接对象放回连接池
  Input:          	pConnection: 连接对象
  Output:         	NULL
  Return:         	
					TRUE: 释放成功
					FALSE:释放失败
  Note:				
**************************************************************************************************/
BOOL			CDBConnectionPool::ReleaseConnection(CConnection*	pConnection)
{
	CGSAutoMutex	AutoLock(&m_CsConnectionPool);
	for (Connection_Vector_Iterator	iter = m_ConnectionPool.begin();iter != m_ConnectionPool.end();iter++)
	{
		CConnection*	pConnectionObj = NULL;
		pConnectionObj = (*iter);
		if (pConnectionObj->GetConnectID() == pConnection->GetConnectID())
		{
			pConnectionObj->SetConnectionUseFlag(CONNECTION_IDLESSE);
			break;
		}
	}
	return TRUE;

}

// /**************************************************************************************************
// Function: BOOLAllConnectionUsed    
// DateTime: 2010/5/25 19:35	
// Description:    	判断所有的连接是否在使用,在使用
// Input:          	NULL
// Output:         	NULL
// Return:         	返回TRUE,否则返回FALSE
// Note:				// 备注
// **************************************************************************************************/
// BOOL            CDBConnectionPool::BOOLAllConnectionUsed()
// {		
// 	BOOL	bUseFlag  = TRUE;
// 	for (Connection_Vector_Iterator	iter = m_ConnectionPool.begin();iter != m_ConnectionPool.end();iter++)
// 	{
// 		CConnection*	pConnectionObj = NULL;
// 		pConnectionObj = (*iter);
// 		if (pConnectionObj != NULL)
// 		{
// 			if (pConnectionObj->GetConnectionUseFlag() == CONNECTION_IDLESSE)
// 			{
// 				bUseFlag = FALSE;
// 				break;
// 			}
// 		}
// 	}
// 	return bUseFlag ;
// 
// }

