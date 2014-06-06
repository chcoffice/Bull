#include "MySQLConnection.h"
#include "MySQLRecordSet.h"
#include "CDBConnectionPool.h"

using namespace DBAccessModule;


CMySQLConnection::CMySQLConnection(void)
{
	// 连接句柄
	m_hDbc	=	SQL_NULL_HANDLE;
	// 环境句柄
	m_hEnv	=	SQL_NULL_HANDLE;	
	// 连接状态标志
	m_bConnectFlag = FALSE;
}

CMySQLConnection::~CMySQLConnection(void)
{
	if (IsOpen())
	{	
		// 断开操作
		SQLDisconnect(m_hDbc);
		SQLFreeHandle(SQL_HANDLE_DBC, m_hDbc);
		SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);

		// 设置资源
		m_hEnv = SQL_NULL_HANDLE;
		m_hDbc = SQL_NULL_HANDLE;

		// 连接状态标志
		m_bConnectFlag = FALSE; 
	}
}


// 打开数据库连接
BOOL	CMySQLConnection::Open()	
{
	//#ifdef _WIN32
	// linux
	// 返回值定义
	SQLRETURN	retcode;

	// 申请Sql环境句柄
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_hEnv) ;
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) 
	{
		// 设置SQL 环境句柄属性
		retcode = SQLSetEnvAttr(m_hEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0); 
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) 
		{
			// 申请sql连接句柄
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, m_hEnv, &m_hDbc);
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) 
			{
				// 连接数据库
				retcode = SQLConnect(m_hDbc, (SQLCHAR*)m_szDatabase, SQL_NTS, (SQLCHAR*)m_szUser, SQL_NTS, 
					(SQLCHAR*)m_szPass, SQL_NTS);

				// 判断是连接成功
				if (!(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO))
				{					
					std::cout<<"---> DBAccess Error：CMySQLConnection::Open() DB SQLConnect failed !"<<endl;
					SQLFreeHandle(SQL_HANDLE_DBC, m_hDbc); 
					SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
					return FALSE;
				}
				else
				{
					printf( "INFO MySql Connect %s@%s success!\n",
						m_szDatabase, m_szUser );
				}
			} 
			else
			{
				std::cout<<"---> DBAccess Error：CMySQLConnection::Open() DB SQLAllocHandle failed !"<<endl;
				// 申请Sql连接句柄失败
				SQLFreeHandle(SQL_HANDLE_DBC, m_hDbc); 
				SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
				return FALSE;
			}
		}
		else
		{
			std::cout<<"---> DBAccess Error：CMySQLConnection::Open() DB SQLSetEnvAttr failed !"<<endl;
			// 设置Sql 环境句柄失败
			SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
			return FALSE;
		}
	}
	else
	{
		std::cout<<"---> DBAccess Error：CMySQLConnection::Open() DB SQLAllocHandle failed !"<<endl;
		// 申请Sql环境句柄失败
		SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
		return FALSE;
	}
	m_bConnectFlag = TRUE;

	return  TRUE;
}

BOOL	CMySQLConnection::Close()
{
	//#ifdef _WIN32
	// linux
	// 如果是打开的
	if(m_bConnectFlag)
	{
		// 断开数据库连接
		SQLDisconnect(m_hDbc);
		// 释放数据库连接句柄
		SQLFreeHandle(SQL_HANDLE_DBC,m_hDbc); 
		// 释放数据库环境句柄
		SQLFreeHandle(SQL_HANDLE_ENV,m_hEnv);

		// 连接句柄
		m_hDbc	=	SQL_NULL_HANDLE;
		// 环境句柄
		m_hEnv	=	SQL_NULL_HANDLE;	

		// 连接标志
		m_bConnectFlag = FALSE;
	}
	return	TRUE;

}

BOOL	CMySQLConnection::IsOpen()
{
	//#ifdef _WIN32
	// linux
	return	m_bConnectFlag;

}

// 开始执行事物
UINT	CMySQLConnection::BeginTrans()
{
	// linux
	// 开始事务
	SQLRETURN	retcode;
	// 开始事务处理

	// 设置为手动提交模式, SQL_AUTOCOMMIT_OFF
	retcode	=	SQLSetConnectAttr (m_hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) SQL_AUTOCOMMIT_OFF, SQL_IS_POINTER);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		return	ERROR_DB_TRANS_RET_SUCCESS;
	}
	return	ERROR_DB_TRANS_RET_ERROR;
}

// 事务的回滚处理
BOOL	CMySQLConnection::RollbackTrans()
{
	SQLRETURN	retcode;
	// 开始回滚事务
	retcode	=	SQLTransact(m_hEnv,m_hDbc,SQL_ROLLBACK);
	if(retcode == SQL_SUCCESS_WITH_INFO || retcode == SQL_SUCCESS)
	{
		// 设置为自动提交模式
		retcode	=	SQLSetConnectAttr (m_hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) SQL_AUTOCOMMIT_ON, SQL_IS_POINTER);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			return	ERROR_DB_TRANS_RET_SUCCESS;
		}
	}
	return	ERROR_DB_TRANS_RET_ERROR;
}

// 事务的提交
BOOL	CMySQLConnection::CommitTrans()
{
	SQLRETURN	retcode;
	// 开始事务处理
	retcode	=	SQLTransact(m_hEnv,m_hDbc,SQL_COMMIT);
	if(retcode == SQL_SUCCESS_WITH_INFO || retcode ==  SQL_SUCCESS)
	{
		// 设置为自动提交模式
		retcode	=	SQLSetConnectAttr (m_hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) SQL_AUTOCOMMIT_ON, SQL_IS_POINTER);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			return	ERROR_DB_TRANS_RET_SUCCESS;
		}
	}
	return	ERROR_DB_TRANS_RET_ERROR;
}

// 执行Sql语句
BOOL			CMySQLConnection::ExecuteSql(const char*  szSql)
{
	// linux
	try
	{
		if (!IsOpen())
		{
			Open();
			// 返回错误
			return FALSE;
		}
		// Sql 查询语句句柄
		SQLHSTMT	hstmt;
		// 返回值代码
		SQLRETURN	retcode;

		// 申请SQL语句句柄，每次执行SQL语句都申请语句句柄，并且在执行完成后释放
		retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_hDbc, &hstmt);	
		// 判断声请Sql语句句柄是否成功，成功：执行Sql语句
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) 
		{
			// 执行成功	// 执行Sql语句
			retcode = SQLExecDirect(hstmt, (SQLCHAR*)szSql, SQL_NTS) ;
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO || retcode == SQL_NO_DATA) 
			{
				SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
				return	TRUE;
			}
			else
			{				
				//----------------------------------------------------------------------------------
				// 输出错误信息 [11/30/2010 13:57 Modify by Liujs]
				INT iErrno = 0;
				ErrorLogInfo(m_hEnv,m_hDbc,hstmt, &iErrno);
				//----------------------------------------------------------------------------------
				SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
				//MY_ERR_THROW();
				return	FALSE;
			}
		}
		else
		{	
			//----------------------------------------------------------------------------------
			// 输出错误信息 [11/30/2010 13:57 Modify by Liujs]
			INT iErrno = 0;
			ErrorLogInfo(m_hEnv,m_hDbc,hstmt, &iErrno);
			//----------------------------------------------------------------------------------

			// 申请Sql 语句执行句柄错误，释放Sql查询语句句柄
			SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			//MY_ERR_THROW();
			return FALSE;	
		}	
	}
	catch(...)
	{
		// 故障处理
		GetConnectionErrorAndDeal();
	}

	return FALSE;

}

// 功能参考接口类
BOOL			CMySQLConnection::ExecuteSql(const char*  szSql,const char* szTable,INT64& lRowId)
{
	// linux
	try
	{
		if (!IsOpen())
		{
			Open();
			// 返回错误
			return FALSE;
		}

		// 操作标识
		BOOL		bSucFlag = FALSE;

		// Sql 查询语句句柄
		SQLHSTMT	hstmt;
		SQLHSTMT	hstmtQuery;
		// 返回值代码
		SQLRETURN	retcode;

		// 申请SQL语句句柄，每次执行SQL语句都申请语句句柄，并且在执行完成后释放
		retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_hDbc, &hstmt);	
		// 判断声请Sql语句句柄是否成功，成功：执行Sql语句
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) 
		{	
			// 执行Sql语句
			retcode = SQLExecDirect(hstmt, (SQLCHAR*)szSql, SQL_NTS) ;
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) 
			{			
				// 是否对象
				SQLFreeHandle(SQL_HANDLE_STMT, hstmt);	

				// 申请SQL语句句柄，每次执行SQL语句都申请语句句柄，并且在执行完成后释放
				retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_hDbc, &hstmtQuery);	
				if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) 
				{
					SQLFreeHandle(SQL_HANDLE_STMT,	hstmt);	
					SQLFreeHandle(SQL_HANDLE_STMT,	hstmtQuery);                    
					return FALSE;
				}
				// 获取受影响的行的对应ID
				char	szInsertIdSql[coniSqlStrLen] = {0};
				memset(szInsertIdSql,0x0,coniSqlStrLen);
				// Sql 语句的用法
				// SELECT  NVL(MAX(ID),1) AS ID FROM VID_TB_TEST_2
				sprintf(szInsertIdSql,"SELECT  IFNULL(MAX(ID),1) AS  %s  FROM %s",INSERT_ID_FIELED_NAME,szTable);

				// 指定要使用的游标并发级别
				retcode = SQLSetStmtAttr(hstmtQuery, SQL_ATTR_CONCURRENCY,(SQLPOINTER) SQL_CONCUR_ROWVER, 0);
				// 设置光标类型为键集光标,
				retcode = SQLSetStmtAttr(hstmtQuery, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_DYNAMIC /* SQL_CURSOR_KEYSET_DRIVEN*/, 0);

				// 执行sql语句
				retcode = SQLExecDirect(hstmtQuery, (SQLCHAR*)szInsertIdSql, SQL_NTS);
				if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO)) 
				{
					//----------------------------------------------------------------------------------
					// 输出错误信息 [11/30/2010 13:57 Modify by Liujs]
					INT iErrno = 0;
					ErrorLogInfo(m_hEnv,m_hDbc,hstmt, &iErrno);
					//----------------------------------------------------------------------------------
					SQLFreeHandle(SQL_HANDLE_STMT, hstmtQuery);	
					//MY_ERR_THROW();
					return FALSE;
				}
				else
				{				
					// 获取数据
					SQLRETURN	iRetFlag = 0;
					SQLCHAR			szData[FIELD_DATA_LEN] = {0};
					memset(szData,0x0,FIELD_DATA_LEN);
					SQLLEN		iActualDataSize = 0;
					// 第一列,索引从1开始
					SQLSMALLINT	iIdCollumn = 1;					
					// 光标移动到第一条记录
					SQLFetchScroll(hstmtQuery, SQL_FETCH_FIRST, 0);

					// 获取值出来
					iRetFlag = SQLGetData(hstmtQuery,iIdCollumn,SQL_C_CHAR,szData,FIELD_DATA_LEN,&iActualDataSize);
					// 判断获取数据是否成功
					if (SQL_SUCCEEDED(iRetFlag)) 
					{
						// 获取最大的ID
						lRowId = atol((char*)szData);
					}
					else
					{
						//----------------------------------------------------------------------------------
						// 输出错误信息 [11/30/2010 13:57 Modify by Liujs]
						INT iErrno = 0;
						ErrorLogInfo(m_hEnv,m_hDbc,hstmt, &iErrno);
						//----------------------------------------------------------------------------------
						// 获取数据失败
						SQLFreeHandle(SQL_HANDLE_STMT, hstmtQuery);	
						//MY_ERR_THROW();
						return	FALSE;
					}
					SQLFreeHandle(SQL_HANDLE_STMT, hstmtQuery);	
					return  TRUE;		
				}

			}
			else
			{
				//----------------------------------------------------------------------------------
				// 输出错误信息 [11/30/2010 13:57 Modify by Liujs]
				INT iErrno = 0;
				ErrorLogInfo(m_hEnv,m_hDbc,hstmt, &iErrno);
				//----------------------------------------------------------------------------------

				// 执行失败
				SQLFreeHandle(SQL_HANDLE_STMT, hstmt);	
				//MY_ERR_THROW();
				return FALSE;
			}			
		}
		else
		{	
			//----------------------------------------------------------------------------------
			// 输出错误信息 [11/30/2010 13:57 Modify by Liujs]
			INT iErrno = 0;
			ErrorLogInfo(m_hEnv,m_hDbc,hstmt, &iErrno);
			//----------------------------------------------------------------------------------

			// 申请Sql 语句执行句柄错误，释放Sql查询语句句柄
			SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			//MY_ERR_THROW();
			return FALSE;	
		}	
	}
	catch(...)
	{
		// 故障处理
		GetConnectionErrorAndDeal();
	}
	// 无法实现
	lRowId = -1;
	return	FALSE;

}

// 分页查询
IRecordSet*		CMySQLConnection::ExecutePageQuery(const char *szSql, const INT32 iStartRow, const INT32 iRowNum)
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
	// SELECT ID FROM (SELECT ROWNUM R ,ID FROM TB_DEVINFO EMP WHERE ROWNUM < 200 ) WHERE R>100
	// 从子集里面查询数据就可以了
	string	strFullSql = "";
	if (GetFullPageQuerySql(szSql,iStartRow,iRowNum,strFullSql))
	{
		// 创建一个对象，返回尾部使用
		CMySQLRecordSet*		pcsMySQLRecordSet = NULL;
		pcsMySQLRecordSet = new CMySQLRecordSet();
		// 设置连接对象指针
		pcsMySQLRecordSet->SetConnection(this);

		// 查询数据集
		if (pcsMySQLRecordSet->QuerySql(strFullSql.c_str())!=NULL)
		{
			return	pcsMySQLRecordSet;
		}
		else
		{
			delete	pcsMySQLRecordSet;
			pcsMySQLRecordSet = NULL;
			return NULL;
		}
	}
	return	NULL;
}

// 时间字符串转换，时间基类接口
string	CMySQLConnection::ToTime(const char*	szDateTime)
{
	char	szDt[64] = {0};
	sprintf(szDt, "'%s'", szDateTime);
	return	string(szDt);
}
string	CMySQLConnection::ToDate(const char*	szDateTime)
{
	char	szDt[64] = {0};
	sprintf(szDt, "'%s'", szDateTime);
	return	string(szDt);
}
string	CMySQLConnection::ToDateTime(const char*	szDateTime)
{
	char	szDt[64] = {0};
	sprintf(szDt, "'%s'", szDateTime);
	return	string(szDt);
}

// 查找关键字，成功：TRUE，失败：FALSE 并返回关键字前后的字符串
BOOL	CMySQLConnection::Find_keyWord_GetString(const char* szSql,const char* szKeyWord,string& strBefore,string& strAfter)
{
	strBefore	=	"";
	strAfter	=	"";

	// char*转换为string
	string	strSql		= string(szSql);
	string	strKeyWord	= string(szKeyWord);

	// 转换为大写
	//transform(strSql.begin(),strSql.end(),strSql.begin(),ToUpper());
	//transform(strKeyWord.begin(),strKeyWord.end(),strKeyWord.begin(),ToUpper());

	// szKeyWord出现在szSql中的位置
	string::size_type	nFromPos = strSql.find(strKeyWord.c_str(),0);

	// 没有找到返回FALSE
	if (string::npos == nFromPos)
	{
		return FALSE;
	}

	// 如果找到继续截取操作
	try
	{
		// 获取strKeyWord的长度
		string::size_type	nLenKeyWord = strKeyWord.length();

		// 获取strSql的长度
		string::size_type	nLenSql		= strSql.length();

		// 截取strSql中出现strKeyWord之前的字符
		strBefore	= strSql.substr(0,nFromPos);

		// 截取strSql中出现strKeyWord之后的字符
		strAfter	= strSql.substr(nFromPos+nLenKeyWord,nLenSql-nFromPos-nLenKeyWord);

		return TRUE;
	}
	catch(...)
	{
		return FALSE;
	}

	return	TRUE;
}

// 获取 FROM前面的字符串
// SELECT ROWNUM R,ID,DEV_ID,DEV_NAME FROM TB_DEVINFO EMP 
// ---> SELECT ROWNUM R,ID,DEV_ID,DEV_NAME 
BOOL		CMySQLConnection::GetBeforeFromSqlText(const char* szSql,string& strBeforeFromSql)
{
	try
	{
		string	strSql  = string(szSql);
		// 查找里面第一个from后面的字符串

		// 转化为大写
		//transform(strSql.begin(),strSql.end(),strSql.begin(),ToUpper()); 

		// FROM 字符出现的位置
		string::size_type	nFromPos = strSql.find(constrFromKeyWord.c_str());

		// 没有找到
		if (nFromPos == string::npos)
		{
			return	FALSE;
		}

		// 获取 from 以前的字符串
		strBeforeFromSql  =	strSql.substr(0,nFromPos);

	}
	catch(...)
	{
		return	FALSE;
	}

	return	TRUE;
}


// 添加 ROWNUM R , 在SELECT 后面
// SELECT ID,DEV_ID,DEV_NAME 
// SELECT ROWNUM R,ID,DEV_ID,DEV_NAME 
BOOL		CMySQLConnection::InsertRowinfo2SqlText(const char* szSql,string& strSql)
{
	try
	{
		// 插入行信息的字符串
		stringstream	strInfo;
		strInfo.str("");

		strSql  = string(szSql);
		string	strTemp = "";
		// 查找里面第一个SELECT 后面的字符串

		// 转化为大写
		//transform(strSql.begin(),strSql.end(),strSql.begin(),ToUpper());

		// FROM 字符出现的位置
		string::size_type	nSelectPos = strSql.find(constrSelectKeyword.c_str());

		// 没有找到
		if (nSelectPos == string::npos)
		{
			return	FALSE;
		}

		// 获取SELECT 后面的字符串
		strSql	=	strSql.substr(nSelectPos + 6,strSql.size() - nSelectPos - 6);

		// DISTINCT出现在szSql中的位置
		string::size_type	nDISTINCTPos;
		nDISTINCTPos = strSql.find(constrDISTINCTKeyWord.c_str());

		// 没有找到DISTINCT
		if (nDISTINCTPos == string::npos)
		{
			// 合成新的字符串
			strInfo << "SELECT ";
			strInfo << " ROWNUM R";
			strInfo << " , ";
			strInfo	<< strSql;
		}
		else
		{
			// 获取DISTINCT后面的字符串
			strSql	=	strSql.substr(nDISTINCTPos + 8,strSql.size() - nDISTINCTPos - 8);

			// 合成新的字符串
			strInfo << "SELECT DISTINCT";
			strInfo << " ROWNUM R";
			strInfo << " , ";
			strInfo	<< strSql;
		}

		// 获取字符串
		strSql	=	strInfo.str();

		strInfo.str("");
	}
	catch(...)
	{
		return	FALSE;
	}

	return	TRUE;
}

// 完整字符串处理
// 输入：SELECT ID,DEV_ID,DEV_NAME FROM TB_DEVINFO EMP 
// 输出：SELECT  * FROM (SELECT ROWNUM R,TB_GS_SELECT.* FROM((SELECT ID,DEV_ID,DEV_NAME FROM TB_DEVINFO EMP  )) TB_GS_SELECT ) WHERE  ROWNUM  < 51 AND R >= 1
BOOL		CMySQLConnection::GetFullPageQuerySql(const char* szSql,const INT32	iStartRow,const INT32 iRowNum,string& strFullSql)
{
	//修改实现方式，直接将接口传过来的SQL进行封装即可,hf修改于2011.03.23
	string strSql(szSql);
	stringstream	strFullSqlInfo;

	strFullSqlInfo << strSql;
	strFullSqlInfo << " LIMIT    ";
	strFullSqlInfo << iStartRow -1;
	strFullSqlInfo <<",   ";
	strFullSqlInfo <<iRowNum;

	strFullSql	=	strFullSqlInfo.str();

	return	TRUE;
}

// 执行标准SQL语句
IRecordSet*		CMySQLConnection::ExecuteQuery(const char*  szSql)
{
	// 创建一个对象，返回尾部使用
	CMySQLRecordSet*		pcsCMySQLRecordSet = NULL;
	pcsCMySQLRecordSet = new CMySQLRecordSet();
	// 设置连接对象指针
	pcsCMySQLRecordSet->SetConnection(this);

	// 查询数据集
	if (pcsCMySQLRecordSet->QuerySql(szSql)!=NULL)
	{
		return	pcsCMySQLRecordSet;
	}
	else
	{
		delete	pcsCMySQLRecordSet;
		pcsCMySQLRecordSet = NULL;
		return NULL;
	}

}

// 返回一个空的数据集，Addnew数据的时候用到的
IRecordSet*		CMySQLConnection::GetEmptyRecordSet()
{
	// 创建一个对象，返回尾部使用
	CMySQLRecordSet*		pcsCMySQLRecordSet = NULL;
	pcsCMySQLRecordSet = new CMySQLRecordSet();
	// 设置连接对象指针
	pcsCMySQLRecordSet->SetConnection(this);

	// 在外面调用 ReleaseRecordSet函数释放
	return	pcsCMySQLRecordSet;
}

// 释放连接对象接口，将使用完毕后的连接对象，返回到连接池中
BOOL			CMySQLConnection::ReleaseConnection() 
{
	// 获取单例
	if (CDBConnectionPool::Instance()!=NULL)
	{
		CDBConnectionPool::Instance()->ReleaseConnection((CConnection*)this);
	}
	return TRUE;

}

//
BOOL	CMySQLConnection::GetConnectionErrorAndDeal()
{
	// linux
	Open();
	return	TRUE;
}