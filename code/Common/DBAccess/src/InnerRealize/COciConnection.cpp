/**************************************************************************************************
* Copyrights 2013  高新兴
*                  基础应用组
* All rights reserved.
*
* Filename：
*       COciConnection.cpp
* Indentifier：
*
* Description：
*       数据访问层外部接口
* Author:
*       LiuHongPing
* Finished：
*
* History:
*       2013年05月，创建
*
**************************************************************************************************/

#include "COciConnection.h"
#include "COciRecordSet.h"
#include "CDBConnectionPool.h"

using namespace DBAccessModule;



/**************************************************************************************************
*@input             Nothing
*@output            Nothing
*@return
*@description       构造函数
*@frequency of call 创建OCI连接类时
**************************************************************************************************/
COciConnection::COciConnection(void)
: m_pCon(NULL)
, m_bclose(TRUE)
{
}

/**************************************************************************************************
*@input             Nothing
*@output            Nothing
*@return
*@description       析构函数
*@frequency of call Nothing
**************************************************************************************************/
COciConnection::~COciConnection (void)
{
	this->fini();


}


/**************************************************************************************************
  Function:    COciConnection    
  DateTime:    2013/5	
  Description: 初始化环境
  Input:       NULL
  Output:      NULL
  Return:      
			   成功：TRUE
			   失败：FALSE
  Note:		   
**************************************************************************************************/
BOOL COciConnection::init(void)
{
	/*
	if (!DBAccessModule::m_pOciDeal->pOCI_Initialize(NULL, NULL, OCI_ENV_DEFAULT)) 
	{
		this->dump_error(m_pOciDeal->pOCI_GetLastError());
		return FALSE;
	}
	*/
	return TRUE;
}


/**************************************************************************************************
  Function:    COciConnection    
  DateTime:    2013/5	
  Description: 释放资源
  Input:       NULL
  Output:      NULL
  Return:      
			   成功：TRUE
			   失败：FALSE
  Note:		   
**************************************************************************************************/
BOOL COciConnection::fini(void)
{
	if (m_bclose)
	{
		m_bclose = FALSE;
		return m_pOciDeal->pOCI_Cleanup();

		
	}
	return TRUE;
}

/**************************************************************************************************
  Function:    dump_error    
  DateTime:    2013/5	
  Description: 出错处理
  Input:       NULL
  Output:      NULL
  Return:      
			   成功：TRUE
			   失败：FALSE
  Note:		   
**************************************************************************************************/
void COciConnection::dump_error(OCI_Error* err)
{
	memset (this->m_error, 0, sizeof (m_error));

	sprintf (m_error, "code: ORA-%05i\n"
					   "type: %i\n"
					   "text: %s",
					   m_pOciDeal->pOCI_ErrorGetOCICode (err), 
					   m_pOciDeal->pOCI_ErrorGetType (err), 
					   m_pOciDeal->pOCI_ErrorGetString (err));
}

/**************************************************************************************************
  Function:    Open    
  DateTime:    2013/5	
  Description: 打开数据库连接
  Input:       NULL
  Output:      NULL
  Return:      
			   成功：TRUE
			   失败：FALSE
  Note:		   
**************************************************************************************************/
BOOL	COciConnection::Open()
{
    char	m_db[coniDataLen];

	memset(m_db,0x00,coniDataLen);
	sprintf(m_db,"%s/%s",m_szServer,m_szDatabase);


	if (!init())
		return FALSE;

	this->m_pCon = m_pOciDeal->pOCI_ConnectionCreate(m_db, m_szUser, m_szPass, OCI_SESSION_DEFAULT);

	if (!this->m_pCon) 
	{
		this->dump_error(m_pOciDeal->pOCI_GetLastError());
		return FALSE;
	}
	else
	{
		printf( "INFO Oci Connect %s@%s success!\n",
			m_szDatabase, m_szUser );
	}
	return TRUE;

}

/**************************************************************************************************
  Function:     Close    
  DateTime:     2013/5
  Description:  关闭连接
  Input:        NULL
  Output:       NULL
  Return:      
			   成功：TRUE
			   失败：FALSE
  Note:		　　NULL
**************************************************************************************************/
BOOL	COciConnection::Close()
{
	if (m_bclose)
	{
		m_bclose = FALSE;

		if (m_pCon)
			m_pOciDeal->pOCI_ConnectionFree(m_pCon);

	}
	return TRUE;

}

/**************************************************************************************************
  Function:         IsOpen    
  DateTime:         2013/5
  Description:    	判断数据库是否打开
  Input:            NULL
  Output:         	NULL
  Return:      
			        成功：TRUE
			        失败：FALSE
  Note:				NULL
**************************************************************************************************/
BOOL	COciConnection::IsOpen()
{

	return	m_bclose;


}

/**************************************************************************************************
  Function:         BeginTrans  
  DateTime:         2013/5	
  Description:    	开始执行事物
  Input:          	NULL
  Output:         	NULL
  Return:         	开始执行的事务的记录
  Note:				
**************************************************************************************************/
UINT	COciConnection::BeginTrans()
{
	if (!m_pOciDeal->pOCI_Commit(this->m_pCon)) 
	{
		this->dump_error(m_pOciDeal->pOCI_GetLastError());
		return 0;
	}
	return 1;

}

/**************************************************************************************************
  Function:         RollbackTrans   
  DateTime:         2013/5	
  Description:    	事务的回滚处理
  Input:            NULL
  Output:         	NULL
  Return:         	成功：TRUE  失败：FALSE
  Note:				
**************************************************************************************************/
BOOL	COciConnection::RollbackTrans()
{
	if (!m_pOciDeal->pOCI_Rollback(this->m_pCon)) 
	{
		this->dump_error(m_pOciDeal->pOCI_GetLastError());
		return FALSE;
	}
	return TRUE;

}


/**************************************************************************************************
Function:			CommitTrans   
DateTime:			2013/5	
Description:    	事务的提交
Input:				NULL
Output:         	NULL
Return:         	成功：TRUE  失败：FALSE
Note:				
**************************************************************************************************/
BOOL	COciConnection::CommitTrans()
{
	if (!m_pOciDeal->pOCI_Commit(this->m_pCon)) 
	{
		this->dump_error(m_pOciDeal->pOCI_GetLastError());
		return FALSE;
	}
	return TRUE;

}

/**************************************************************************************************
  Function:		  ExecuteSql(const char*  szSql)    
  DateTime:		  2013/5
  Description:    执行Sql语句
  Input:          strSql：标准Sql字符串
  Output:         NULL
  Return:         TRUE:成功			FALSE:失败
  Note:				
**************************************************************************************************/
BOOL	COciConnection::ExecuteSql(const char*  szSql)
{
	try
	{
		//statement
		OCI_Statement*  hStmt;

		//result
		OCI_Resultset*  hResult;

		if (!IsOpen())
		{
			if( !Open() )
			{
				return FALSE;
			}
		}

		hStmt = m_pOciDeal->pOCI_StatementCreate(m_pCon);

		if (!hStmt) 
		{
			this->dump_error(m_pOciDeal->pOCI_GetLastError());
			//MY_ERR_THROW();

			return FALSE;
		}

		m_pOciDeal->pOCI_ExecuteStmt(hStmt, szSql);
		if (!m_pOciDeal->pOCI_Commit(m_pCon))
		{
			this->dump_error(m_pOciDeal->pOCI_GetLastError());

			if (hStmt)
				m_pOciDeal->pOCI_StatementFree(hStmt);

			//MY_ERR_THROW();

			return FALSE;
		}

		if (strstr (szSql, "select") || strstr (szSql, "SELECT"))
		{
			hResult = m_pOciDeal->pOCI_GetResultset(hStmt);
			if(hResult != NULL)
			{
				if (hResult)
					m_pOciDeal->pOCI_ReleaseResultsets(hStmt);

				if (hStmt)
					m_pOciDeal->pOCI_StatementFree(hStmt);

				return TRUE;
			}
		}
		if (hStmt)
				m_pOciDeal->pOCI_StatementFree(hStmt);
		return TRUE;
	}
	catch(...)
	{
		// 故障处理
		GetConnectionErrorAndDeal();
	}
	return FALSE;
}


/**************************************************************************************************
  Function:			ExecuteSql(const char*  szSql,INT64& lRowId)    
  DateTime:			2013/5
  Description:    	执行Sql操作
  Input:          	标准Sql查询语句,szTable 表名称
  Output:         	lRowID:影响的行的ID
  Return:         	成功TRUE,失败FALSE
  Note:				// 备注
**************************************************************************************************/
BOOL	COciConnection::ExecuteSql(const char*  szSql,const char* szTable,INT64& lRowId)
{

	try
	{
		//statement
		OCI_Statement*  hStmt;

		//result
		OCI_Resultset*  hResult;

		if (!IsOpen())
		{
			if( !Open() )
			{
				return FALSE;
			}
		}

		hStmt = m_pOciDeal->pOCI_StatementCreate(m_pCon);

		if (!hStmt) 
		{
			this->dump_error(m_pOciDeal->pOCI_GetLastError());
			//MY_ERR_THROW();

			return FALSE;
		}

		m_pOciDeal->pOCI_ExecuteStmt(hStmt, szSql);
		if (!m_pOciDeal->pOCI_Commit(m_pCon))
		{
			this->dump_error(m_pOciDeal->pOCI_GetLastError());

			if (hStmt)
				m_pOciDeal->pOCI_StatementFree(hStmt);
			

			//MY_ERR_THROW();

			return FALSE;
		}

		// 获取受影响的行的对应ID
		char	szInsertIdSql[coniSqlStrLen] = {0};

		memset(szInsertIdSql,0x0,coniSqlStrLen);

		sprintf(szInsertIdSql,"select S_%s.CURRVAL AS  %s  from dual",
							szTable,INSERT_ID_FIELED_NAME);

		m_pOciDeal->pOCI_ExecuteStmt(hStmt, szInsertIdSql);
		if (!m_pOciDeal->pOCI_Commit(m_pCon))
		{
			this->dump_error(m_pOciDeal->pOCI_GetLastError());

			if (hStmt)
				m_pOciDeal->pOCI_StatementFree(hStmt);

			//MY_ERR_THROW();

			return FALSE;
		}
		sprintf(szInsertIdSql,"SELECT  NVL(MAX(ID),1) AS  %s  FROM %s",INSERT_ID_FIELED_NAME,szTable);

		m_pOciDeal->pOCI_ExecuteStmt(hStmt, szInsertIdSql);
		if (!m_pOciDeal->pOCI_Commit(m_pCon))
		{
			this->dump_error(m_pOciDeal->pOCI_GetLastError());

			if (hStmt)
				m_pOciDeal->pOCI_StatementFree(hStmt);

			//MY_ERR_THROW();

			return FALSE;
		}
		else
		{

			hResult = m_pOciDeal->pOCI_GetResultset(hStmt);

			if (!m_pOciDeal->pOCI_FetchLast(hResult)) 
			{
				this->dump_error(m_pOciDeal->pOCI_GetLastError());

				if (hStmt)
					m_pOciDeal->pOCI_StatementFree(hStmt);

				//MY_ERR_THROW();

				return FALSE;
			}
			lRowId = m_pOciDeal->pOCI_GetInt(hResult, 1);

		}

/*
		if (strstr (szSql, "select") || strstr (szSql, "SELECT"))
		{
			hResult = OCI_GetResultset(hStmt);
			if(hResult != NULL)
			{
				if (hResult)
					OCI_ReleaseResultsets(hStmt);

				if (hStmt)
					OCI_StatementFree(hStmt);

				return TRUE;
			}
		}
*/
		if (hStmt)
				m_pOciDeal->pOCI_StatementFree(hStmt);

		if (hResult)
				m_pOciDeal->pOCI_ReleaseResultsets(hStmt);
		


		return TRUE;
	}
	catch(...)
	{
		// 故障处理
		GetConnectionErrorAndDeal();
	}
	return FALSE;

}

/**************************************************************************************************
  Function		: ExecutePageQuery(const char *szSql, const INT32 iStartRow, const INT32 iRowNum)
  DateTime		: 2013/5	
  Description	: 分页查询
  Input			: NULL
  Output		: NULL
  Return		: NULL
  Note			:
**************************************************************************************************/
IRecordSet*		COciConnection::ExecutePageQuery(const char *szSql, const INT32 iStartRow, const INT32 iRowNum)
{
	if (iStartRow < coniZero)
	{
		return NULL;
	}
	if (iRowNum <= coniZero)
	{
		return	NULL;
	}

	// 将输入的字符串，合成下面的字符串格式
	string	strFullSql = "";
	if (GetFullPageQuerySql(szSql,iStartRow,iRowNum,strFullSql))
	{
		// 创建一个对象，返回尾部使用
		COciRecordSet*		pcsCOciRecordSet = NULL;
		pcsCOciRecordSet = new COciRecordSet();
		// 设置连接对象指针
		pcsCOciRecordSet->SetConnection(this);

		// 查询数据集
		if (pcsCOciRecordSet->QuerySql(strFullSql.c_str())!=NULL)
		{
			return	pcsCOciRecordSet;
		}
		else
		{
			delete	pcsCOciRecordSet;
			pcsCOciRecordSet = NULL;
			return NULL;
		}
	}
	return	NULL;
}

/**************************************************************************************************
  Function		: GetFullPageQuerySql
  DateTime		: 2013/5	
  Description	: 完整字符串处理
  Input			: 
  Output		: 
  Return		: 
  Note			:
**************************************************************************************************/
BOOL		COciConnection::GetFullPageQuerySql(const char* szSql,const INT32	iStartRow,const INT32 iRowNum,string& strFullSql)
{
	//修改实现方式，直接将接口传过来的SQL进行封装即可,hf修改于2011.03.23
	string strSql(szSql);
	stringstream	strFullSqlInfo;

	//添加SQL前面语句
	strFullSqlInfo << "SELECT  * FROM (SELECT ROWNUM R,GS_SELECT.* FROM ( ";

	strFullSqlInfo << strSql;

	//添加SQL后面语句
	strFullSqlInfo << " ) GS_SELECT WHERE ROWNUM < ";
	strFullSqlInfo <<  iStartRow + iRowNum; 
	strFullSqlInfo << " ) WHERE R >= ";
	strFullSqlInfo << iStartRow;

	strFullSql	=	strFullSqlInfo.str();

	return	TRUE;
}

/**************************************************************************************************
  Function: ExecuteQuery(const char*  szSql)    
  DateTime: 2013/5	
  Description:    	// 函数功能、性能等的描述
  Input:          	执行操作的Sql语句
  Output:         	NULL
  Return:         	IRecordset对象接口对象，可以根据IRecordset接口执行相应的接口提供方法
  Note:				
					IRecordset对象接口对象，可以根据IRecordset接口执行相应的接口提供方法
**************************************************************************************************/
IRecordSet*		COciConnection::ExecuteQuery(const char*  szSql)
{
	// 创建一个对象，返回尾部使用
	COciRecordSet*		pcsCOciRecordSet = NULL;
	pcsCOciRecordSet = new COciRecordSet();
	// 设置连接对象指针
	pcsCOciRecordSet->SetConnection(this);

	// 查询数据集
	if (pcsCOciRecordSet->QuerySql(szSql)!=NULL)
	{
		return	pcsCOciRecordSet;
	}
	else
	{
		delete	pcsCOciRecordSet;
		pcsCOciRecordSet = NULL;
		return NULL;
	}

}	

/**************************************************************************************************
Function		: GetEmptyRecordSet()
DateTime		: 2013/5	
Description		: 返回一个空的数据集，Addnew数据的时候用到的
Input			: NULL
Output			: NULL
Return			: NULL
Note			:
**************************************************************************************************/
IRecordSet*		COciConnection::GetEmptyRecordSet()
{
	// 创建一个对象，返回尾部使用
	COciRecordSet*		pcsCOciRecordSet = NULL;
	pcsCOciRecordSet = new COciRecordSet();
	// 设置连接对象指针
	pcsCOciRecordSet->SetConnection(this);

	// 在外面调用 ReleaseRecordSet函数释放
	return	pcsCOciRecordSet;
}

/**************************************************************************************************
  Function:ReleaseConnection()     
  DateTime: 2013/5
  Description:    	释放连接对象接口，将使用完毕后的连接对象，返回到连接池中
  Input:          	NULL
  Output:         	NULL
  Return:         	成功：TRUE，失败：FALSE
  Note:				
					释放连接对象接口，将使用完毕后的连接对象，返回到连接池中
**************************************************************************************************/
BOOL			COciConnection::ReleaseConnection() 
{
	// 获取单例
	if (CDBConnectionPool::Instance()!=NULL)
	{
		CDBConnectionPool::Instance()->ReleaseConnection((CConnection*)this);
	}
	return TRUE;

}



/**************************************************************************************************
Function		: GetConnectionErrorAndDeal()    
DateTime		: 2013/5	
Description		: 获取错误链接并进行相应操作：主要针对网络断开，数据库断开等操作控制
Input			: NULL
Output			: NULL
Return			: TRUE:成功  FALSE:失败
Note			:
**************************************************************************************************/
BOOL	COciConnection::GetConnectionErrorAndDeal()
{
//	Close();
//	Open();
	return	FALSE;
}

// 时间字符串转换，时间基类接口
string	COciConnection::ToTime(const char*	szDateTime)
{
	char	szDt[64] = {0};
	sprintf(szDt, "TO_DATE('%s', 'HH24:Mi:SS')", szDateTime);
	return	string(szDt);
}
string	COciConnection::ToDate(const char*	szDateTime)
{
	char	szDt[64] = {0};
	sprintf(szDt, "TO_DATE('%s', 'YYYY-MM-DD')", szDateTime);
	return	string(szDt);
}
string	COciConnection::ToDateTime(const char*	szDateTime)
{
	char	szDt[64] = {0};
	sprintf(szDt, "TO_DATE('%s', 'YYYY-MM-DD HH24:Mi:SS')", szDateTime);
	return	string(szDt);
}


// 查找关键字，成功：TRUE，失败：FALSE 并返回关键字前后的字符串
BOOL	COciConnection::Find_keyWord_GetString(const char* szSql,const char* szKeyWord,string& strBefore,string& strAfter)
{

	return	TRUE;

}
// 获取 FROM前面的字符串
BOOL		COciConnection::GetBeforeFromSqlText(const char* szSql,string& strBeforeFromSql)
{
	return	FALSE;
}

// 添加 ROWNUM R , 在SELECT 后面
BOOL		COciConnection::InsertRowinfo2SqlText(const char* szSql,string& strSql)
{
	return	FALSE;
}
