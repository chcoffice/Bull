#include "COracleConnection.h"
#include "COracleRecordSet.h"
#include "CDBConnectionPool.h"

using namespace DBAccessModule;


#define MY_ERR_THROW( ) int iTimes = 20;\
	do { \
	if( iErrno==ERRNO_NETWORK_DISCNN || iErrno==ERRNO_NETWORK_LOSTCNN) \
	{    \
       Close(); \
	} \
	else break;\
    /* GetConnectionErrorAndDeal();*/ \
	} while((!Open())&&(--iTimes))

COracleConnection::COracleConnection(void)
{
//#ifdef _WIN32
#if 0
	// 创建 Connection 对象
	m_pConnection.CreateInstance("ADODB.Connection");

#else
	// linux
	// 连接句柄
	m_hDbc	=	SQL_NULL_HANDLE;
	// 环境句柄
	m_hEnv	=	SQL_NULL_HANDLE;	
	// 连接状态标志
	m_bConnectFlag = FALSE;
#endif

}

// 析构函数
COracleConnection::~COracleConnection(void)
{
//#ifdef _WIN32
#if 0
	if (m_pConnection != NULL)
	{
		if (IsOpen())
		{
			Close();
		}

		m_pConnection.Release();
		m_pConnection = NULL;
	}
#else
	//linux
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
#endif

}


//---------------------------------------------------------------------------------
// 内部接口:打开，关闭，判断打开
//---------------------------------------------------------------------------------
/**************************************************************************************************
  Function: Open    
  DateTime: 2010/5/24 21:01	
  Description: 打开数据库连接
  Input:       NULL
  Output:      NULL
  Return:      
			   成功：TRUE
			   失败：FALSE
  Note:		   
**************************************************************************************************/
BOOL	COracleConnection::Open()	
{
//#ifdef _WIN32
#if 0
	// 连接字符串长度
	const	INT		coniConStrLen = 1024;
	
	// 初始化连接串
	char	szConnectStr[coniConStrLen] = {0};
	memset(szConnectStr,0x0,coniConStrLen);

	if (m_pConnection == NULL)
	{
		m_pConnection.CreateInstance("ADODB.Connection");
	}

	// 获取连接字符串 Microsoft OLE DB Provider for Oracle 驱动的，不支持ORACLE 的CLOB等大字段
	// Provider=MSDAORA.1;Password=gxx01;User ID=gxx01;Data Source=192.168.26.127:1521/C3MLSC;Persist Security Info=True
	// sprintf_s(szConnectStr,"Provider=MSDAORA.1;Password=%s;User ID=%s;Data Source=%s:1521/%s;Persist Security Info=True",m_szUser,m_szPass,m_szServer,m_szDatabase);
	
	// 获取连接字符串  Oracle Provider for OLE DB 驱动的，支持ORACLE 的CLOB等大字段
	// Provider=OraOLEDB.Oracle.1;Password=video01;Persist Security Info=True;User ID=video01;Data Source=192.168.5.101:1521/C3M_VIDEO
	sprintf_s(szConnectStr,"Provider=OraOLEDB.Oracle.1;Password=%s;Persist Security Info=True;User ID=%s;Data Source=%s",m_szPass,m_szUser,m_szDatabase);


	// 对连接对象的有效性进行检查
	if (m_pConnection == NULL)
	{
		return	ERROR_DB_CREATE_CONNECTION_ERROR;
	}

	// 检查连接是否已经打开 
	if (IsOpen())
	{
		Close();
	}

	// 打开数据库连接
	try
	{
		//	连接数据库
		//	[adConnectUnspecified]:同步方式
		//	[adAsyncConnect]:异步方式
		HRESULT hr=m_pConnection->Open(_bstr_t(szConnectStr),"","",adModeUnknown);
		if(!SUCCEEDED(hr))   
		{
			return ERROR_DB_CONNECT_ERROR;
		}

	}
	catch (_com_error e)
	{
		char		szErrMsg[2013]={0};
		sprintf_s(szErrMsg,"Warning: 关闭数据库发生异常. 错误信息: %s; 文件: %s; 行: %d",e.ErrorMessage(), __FILE__, __LINE__);
		return ERROR_DB_CONNECT_ERROR;
	}
#else
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

				/*int i = 0;
				SQLCHAR SqlState[SQL_MAX_MESSAGE_LENGTH];
				SQLCHAR Msg[SQL_MAX_MESSAGE_LENGTH];
				memset(SqlState,0x0,SQL_MAX_MESSAGE_LENGTH);
				memset(Msg,0x0,SQL_MAX_MESSAGE_LENGTH);
				SQLINTEGER NativeError = 0;
				SQLSMALLINT MsgLen;
				while ((retcode = SQLGetDiagRec(SQL_HANDLE_STMT, m_hDbc, i, SqlState, &NativeError, Msg, sizeof(Msg), &MsgLen) != SQL_NO_DATA_FOUND))
				{
					std::cout<<"错误状态:"<<SqlState<<endl;
					std::cout<<"错误代码:"<<NativeError<<endl;
					std::cout<<"错误描述:"<<Msg<<endl;
					++i;
					memset(SqlState,0x0,SQL_MAX_MESSAGE_LENGTH);
					memset(Msg,0x0,SQL_MAX_MESSAGE_LENGTH);
				}*/
				
				// 判断是连接成功
				if (!(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO))
				{					
					std::cout<<"---> DBAccess Error：COracleConnection::Open() DB SQLConnect failed !"<<endl;
					SQLFreeHandle(SQL_HANDLE_DBC, m_hDbc); 
					SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
					return FALSE;
				}
				else
				{
					printf( "INFO Oracle Connect %s@%s success!\n",
						m_szDatabase, m_szUser );
				}
			} 
			else
			{
				std::cout<<"---> DBAccess Error：COracleConnection::Open() DB SQLAllocHandle failed !"<<endl;
				// 申请Sql连接句柄失败
				SQLFreeHandle(SQL_HANDLE_DBC, m_hDbc); 
				SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
				return FALSE;
			}
		}
		else
		{
			std::cout<<"---> DBAccess Error：COracleConnection::Open() DB SQLSetEnvAttr failed !"<<endl;
			// 设置Sql 环境句柄失败
			SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
			return FALSE;
		}
	}
	else
	{
		std::cout<<"---> DBAccess Error：COracleConnection::Open()  DB SQLAllocHandle failed !"<<endl;
		// 申请Sql环境句柄失败
		SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
		return FALSE;
	}

	m_bConnectFlag = TRUE;

#endif
	LoadSeqMap();
	return  TRUE;

}

/**************************************************************************************************
  Function: Close    
  DateTime: 2010/5/24 22:36	
  Description:  关闭连接
  Input:        NULL
  Output:       NULL
  Return:       TRUE : 成功   FALSE :失败
  Note:		　　NULL
**************************************************************************************************/
BOOL	COracleConnection::Close()
{
//#ifdef _WIN32
#if 0
	// 数据库连接对象的有效性检查
	if (m_pConnection == NULL)
	{
		return FALSE;
	}

	// 检查数据库连接是否已经打开
	if (!IsOpen())
	{
		return TRUE;
	}

	// 关闭数据库连接
	try
	{
		if (m_pConnection != NULL && IsOpen()) 
		{
			m_pConnection->Close();
		}
		return TRUE;
	}
	catch (_com_error e)
	{
	//	m_strError.Format(_T("Warning: 关闭数据库发生异常. 错误信息: %s; 文件: %s; 行: %d\n"), e.ErrorMessage(), __FILE__, __LINE__);
		return FALSE;
	}
#else
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
#endif
	return	TRUE;

}

/**************************************************************************************************
  Function: IsOpen    
  DateTime: 2010/5/24 22:40	
  Description:    	判断数据库是否打开
  Input:            NULL
  Output:         	NULL
  Return:         	TRUE : 打开   FALSE:关闭
  Note:				NULL
**************************************************************************************************/
BOOL	COracleConnection::IsOpen()
{
//#ifdef _WIN32
#if 0
	// 获取数据库连接的状态,并取的该状态值中的打开参量adStateOpen
	try
	{
		return (m_pConnection != NULL && (m_pConnection->State & adStateOpen));
	}
	catch (_com_error e)
	{
		// TRACE(_T("Warning: IsOpen 方法发生异常. 错误信息: %s; 文件: %s; 行: %d\n"), e.ErrorMessage(), __FILE__, __LINE__);
		return FALSE;
	} 
#else
	// linux
	return	m_bConnectFlag;
#endif

	return	TRUE;

}



//---------------------------------------------------------------------------------
// 内部接口:事务处理
//---------------------------------------------------------------------------------
/**************************************************************************************************
  Function:   BeginTrans  
  DateTime: 2010/5/24 22:41	
  Description:    	开始执行事物
  Input:          	NULL
  Output:         	NULL
  Return:         	开始执行的事务的记录
  Note:				一旦调用了 BeginTrans 方法, 在调用 CommitTrans 或 RollbackTrans
					结束事务之前, 数据库将不再立即提交所作的任何更改
**************************************************************************************************/
UINT	COracleConnection::BeginTrans()
{
#if 0
	// 提交事务
	try
	{
		return m_pConnection->BeginTrans();
	}
	catch (_com_error e)
	{
	//	TRACE(_T("Warning: BeginTrans 方法发生异常. 错误信息: %s; 文件: %s; 行: %d\n"), e.ErrorMessage(), __FILE__, __LINE__);
		return ERROR_DB_TRANS_RET_ERROR;
	}
#else
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

#endif
	return	ERROR_DB_TRANS_RET_ERROR;

}

/**************************************************************************************************
  Function: RollbackTrans   
  DateTime: 2010/5/24 22:41	
  Description:    	事务的回滚处理
  Input:            NULL
  Output:         	NULL
  Return:         	成功：TRUE  失败：FALSE
  Note:				
**************************************************************************************************/
BOOL	COracleConnection::RollbackTrans()
{
#if 0
	// 回滚事务
	try
	{
		if (SUCCEEDED(m_pConnection->RollbackTrans()))
		{
			return	TRUE;
		}		
	}
	catch(...)
	{
		return FALSE;
	} 
#else
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
#endif
	// 异常返回
	return FALSE;

}


/**************************************************************************************************
Function: CommitTrans   
DateTime: 2010/5/24 22:41	
Description:    	事务的提交
Input:				NULL
Output:         	NULL
Return:         	成功：TRUE  失败：FALSE
Note:				
**************************************************************************************************/
BOOL	COracleConnection::CommitTrans()
{
#if 0
	// 提交事务
	try
	{
		if (SUCCEEDED(m_pConnection->CommitTrans()))
		{
			return	TRUE;
		}		
	}
	catch(...)
	{
		return FALSE;
	} 
#else
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
#endif
	// 异常返回
	return FALSE;

}
//------------------------------------------------------------------------------------------------
// OuterInterface的实现
//------------------------------------------------------------------------------------------------
/**************************************************************************************************
  Function: ExecuteSql(const char*  szSql)    
  DateTime: 2010/5/26 17:14	
  Description:    执行Sql语句
  Input:          strSql：标准Sql字符串
  Output:         NULL
  Return:         TRUE:成功			FALSE:失败
  Note:				
**************************************************************************************************/
BOOL			COracleConnection::ExecuteSql(const char*  szSql)
{
#if 0
	if (m_pConnection != NULL)
	{
		try
		{
			// 连接打开了
			if (IsOpen())
			{
				// 执行sql语句,执行后数据集不为空
				if (m_pConnection->Execute(_bstr_t(szSql),NULL,adCmdText) != NULL)
				{
					// 提交事务
					return	TRUE;
				}
				else
				{
					// 回滚事务
					return	FALSE;
				}
			}
			else
			{
				// 打开数据库连接
				Open();
			}
		}// end try
		catch(...)
		{
			// 如果异常，判断是否是数据库连接出错
			GetConnectionErrorAndDeal();
		}
	}
#else
	// linux
	try
	{
		if (!IsOpen())
		{
			if( !Open() )
			{

				// 返回错误
				return FALSE;
			}
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
                MY_ERR_THROW();
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
            MY_ERR_THROW();
			return FALSE;	
		}	
	}
	catch(...)
	{
		// 故障处理
		GetConnectionErrorAndDeal();
	}
#endif
	return FALSE;

}


/**************************************************************************************************
  Function: ExecuteSql(const char*  szSql,INT64& lRowId)    
  DateTime: 2010/5/26 17:16	
  Description:    	执行Sql操作
  Input:          	标准Sql查询语句,szTable 表名称
  Output:         	lRowID:影响的行的ID
  Return:         	成功TRUE,失败FALSE
  Note:				// 备注
**************************************************************************************************/
BOOL			COracleConnection::ExecuteSql(const char*  szSql,const char* szTable,INT64& lRowId)
{
#if 0
	if (m_pConnection != NULL)
	{
		try
		{
			// 连接打开了
			if (IsOpen())
			{
				// 操作标识
				BOOL		bSucFlag = FALSE;

				// 执行sql语句,执行后数据集不为空
				if (m_pConnection->Execute(_bstr_t(szSql),NULL,adCmdText) != NULL)
				{

					// 获取最大值ID
					char	szInsertIdSql[coniSqlStrLen] = {0};
					memset(szInsertIdSql,0x0,coniSqlStrLen);
					// Sql 语句的用法
					// SELECT  NVL(MAX(ID),1) AS ID FROM VID_TB_TEST_2
					sprintf(szInsertIdSql,"SELECT  NVL(MAX(ID),1) AS  %s  FROM %s",INSERT_ID_FIELED_NAME,szTable);

					// 执行sql查询
					_RecordsetPtr	_ResordSet;
					_ResordSet = m_pConnection->Execute(_bstr_t(szInsertIdSql),NULL,adCmdText);
					while (!_ResordSet->adoEOF)
					{
						// 获取值信息
						_variant_t value = _ResordSet->GetCollect(_variant_t(INSERT_ID_FIELED_NAME));
						// 将值进行转换
						lRowId = atol((char*)(_bstr_t)value);
						bSucFlag = TRUE;
						break;
					}				

					memset(szInsertIdSql,0x0,coniSqlStrLen);
					// 
					if (bSucFlag)
					{
						return	TRUE;
					}	
				}
				else
				{
					return	FALSE;
				}
			}// if (IsOpen())
			else
			{
				// 打开数据库连接
				Open();
			}
		}// end try
		catch(...)
		{
			// 如果异常，判断是否是数据库连接出错
			GetConnectionErrorAndDeal();
		}
	}
	
#else
	// linux
	try
	{
		if (!IsOpen())
		{
			if( !Open() )
			{

				// 返回错误
				return FALSE;
			}
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
				// 指定要使用的游标并发级别
				SQLSetStmtAttr(hstmtQuery, SQL_ATTR_CONCURRENCY,(SQLPOINTER) SQL_CONCUR_ROWVER, 0);
				// 设置光标类型为键集光标,
				SQLSetStmtAttr(hstmtQuery, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_DYNAMIC /* SQL_CURSOR_KEYSET_DRIVEN*/, 0);

				// 获取受影响的行的对应ID
				char	szInsertIdSql[coniSqlStrLen] = {0};
				memset(szInsertIdSql,0x0,coniSqlStrLen);
				// Sql 语句的用法

				const std::string &strSeqName = GetIDSequenceNameOfTableName(szTable);
				if( strSeqName.empty() )
				{

					sprintf(szInsertIdSql,"select S_%s.CURRVAL AS  %s  from dual",
						szTable,INSERT_ID_FIELED_NAME);
				}
				else
				{
					sprintf(szInsertIdSql,"select %s.CURRVAL AS  %s  from dual",
						strSeqName.c_str() ,INSERT_ID_FIELED_NAME);
				}
				retcode = SQLExecDirect(hstmtQuery, (SQLCHAR*)szInsertIdSql, SQL_NTS);

				if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO)) 
				{
					assert(0);
					//没有序列
					sprintf(szInsertIdSql,"SELECT  NVL(MAX(ID),1) AS  %s  FROM %s",INSERT_ID_FIELED_NAME,szTable);



					// 执行sql语句
					retcode = SQLExecDirect(hstmtQuery, (SQLCHAR*)szInsertIdSql, SQL_NTS);
					if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO)) 
					{
						
						// 输出错误信息 [11/30/2010 13:57 Modify by Liujs]
						INT iErrno = 0;
						ErrorLogInfo(m_hEnv,m_hDbc,hstmt, &iErrno);						
						SQLFreeHandle(SQL_HANDLE_STMT, hstmtQuery);	
						MY_ERR_THROW();
						return FALSE;
					}

				}

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
                         MY_ERR_THROW();
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
                 MY_ERR_THROW();
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
             MY_ERR_THROW();
			return FALSE;	
		}	
	}
	catch(...)
	{
		// 故障处理
		GetConnectionErrorAndDeal();
	}

#endif
	// 无法实现
	lRowId = -1;
	return	FALSE;

}

/**************************************************************************************************
  Function		: ExecutePageQuery(const char *szSql, const INT32 iStartRow, const INT32 iRowNum)
  DateTime		: 2010/12/2 10:26	
  Description	: 分页查询
  Input			: NULL
  Output		: NULL
  Return		: NULL
  Note			:
**************************************************************************************************/
IRecordSet*		COracleConnection::ExecutePageQuery(const char *szSql, const INT32 iStartRow, const INT32 iRowNum)
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
		COracleRecordSet*		pcsCOracleRecordSet = NULL;
		pcsCOracleRecordSet = new COracleRecordSet();
		// 设置连接对象指针
		pcsCOracleRecordSet->SetConnection(this);

		// 查询数据集
		if (pcsCOracleRecordSet->QuerySql(strFullSql.c_str())!=NULL)
		{
			return	pcsCOracleRecordSet;
		}
		else
		{
			delete	pcsCOracleRecordSet;
			pcsCOracleRecordSet = NULL;
			return NULL;
		}
	}
	return	NULL;
}


// 时间字符串转换，时间基类接口
string	COracleConnection::ToTime(const char*	szDateTime)
{
	char	szDt[64] = {0};
	sprintf(szDt, "TO_DATE('%s', 'HH24:Mi:SS')", szDateTime);
	return	string(szDt);
}
string	COracleConnection::ToDate(const char*	szDateTime)
{
	char	szDt[64] = {0};
	sprintf(szDt, "TO_DATE('%s', 'YYYY-MM-DD')", szDateTime);
	return	string(szDt);
}
string	COracleConnection::ToDateTime(const char*	szDateTime)
{
	char	szDt[64] = {0};
	sprintf(szDt, "TO_DATE('%s', 'YYYY-MM-DD HH24:Mi:SS')", szDateTime);
	return	string(szDt);
}


// 查找关键字，成功：TRUE，失败：FALSE 并返回关键字前后的字符串
BOOL	COracleConnection::Find_keyWord_GetString(const char* szSql,const char* szKeyWord,string& strBefore,string& strAfter)
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
BOOL		COracleConnection::GetBeforeFromSqlText(const char* szSql,string& strBeforeFromSql)
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
BOOL		COracleConnection::InsertRowinfo2SqlText(const char* szSql,string& strSql)
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
BOOL		COracleConnection::GetFullPageQuerySql(const char* szSql,const INT32	iStartRow,const INT32 iRowNum,string& strFullSql)
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

		//分割线
	// 完整字符串处理
	// 输入：SELECT ID,DEV_ID,DEV_NAME FROM TB_DEVINFO EMP 
	// 输出：SELECT ID,DEV_ID,DEV_NAME  FROM (SELECT ROWNUM R,ID,DEV_ID,DEV_NAME FROM TB_DEVINFO EMP WHERE ROWNUM < 200 )WHERE R>=100
	/*BOOL		COracleConnection::GetFullPageQuerySql(const char* szSql,const INT32	iStartRow,const INT32 iRowNum,string& strFullSql)
	{*/

	//___________________________________________________________________________________
	//string	strSql(szSql);
	//// 转化为大写,输入用户名时，因为有大小写之分，不能直接转化为大写，之后再修改，hf修改于2010.01.26
	////transform(strSql.begin(),strSql.end(),strSql.begin(),ToUpper()); 

	//stringstream	strFullSqlInfo;

	//// 临时字符串
	//string		strTemp;
	//strTemp = "";

	//// 获取from前面的字符串
	//if (!GetBeforeFromSqlText(strSql.c_str(),strTemp))
	//{
	//	return	FALSE;
	//}

	////若查询条件中添加了表名加列名，则会出错，暂时全部取出，hf修改于2010.01.26
	////strFullSqlInfo << strTemp;
	//strFullSqlInfo << "SELECT * FROM ( ";

	//// 获取条件查询字符串
	//string		strConditionSql = "" ;

	//// 插入关键字信息 [ROWNUM R,] 例如
	//// 输入：SELECT ID,DEV_ID,DEV_NAME FROM TB_DEVINFO EMP 
	//// 输出：SELECT ROWNUM R,ID,DEV_ID,DEV_NAME FROM TB_DEVINFO EMP 
	//if (!InsertRowinfo2SqlText(strSql.c_str(),strConditionSql))
	//{
	//	return	FALSE;
	//}

	//// 开始查找关键字信息 [12/9/2010 10:08 Modify by Liujs]
	//// 查找顺序：
	//// 1. WHERE
	//// 2. GROUP
	//// 3. ORDER
	//string		strBefore;
	//string		strAfter;
	//// 查找where
	//if (Find_keyWord_GetString(strConditionSql.c_str(),constrWhereKeyWord.c_str(),strBefore,strAfter))
	//{
	//	// 找到WHERE，查找GROUP
	//	if (Find_keyWord_GetString(strConditionSql.c_str(),constrGroupKeyWord.c_str(),strBefore,strAfter))
	//	{
	//		// 1.查找GROUP,添加信息
	//		strFullSqlInfo << strBefore;
	//		strFullSqlInfo << " AND ";
	//		strFullSqlInfo << " ROWNUM ";
	//		strFullSqlInfo << " < ";
	//		strFullSqlInfo << iStartRow + iRowNum;
	//		strFullSqlInfo << " ";
	//		strFullSqlInfo << constrGroupKeyWord;
	//		strFullSqlInfo << " ";

	//		string		strTemp = strAfter;

	//		// 查找BY,添加信息
	//		if (Find_keyWord_GetString(strTemp.c_str(),constrBYKeyWord.c_str(),strBefore,strAfter))
	//		{
	//			strFullSqlInfo << constrBYKeyWord;
	//			strFullSqlInfo << " ROWNUM, ";
	//			strFullSqlInfo << strAfter;
	//		}
	//	}
	//	else
	//	{
	//		// 2.没有找到GROUP，就查找ORDER
	//		if (Find_keyWord_GetString(strConditionSql.c_str(),constrOrderKeyword.c_str(),strBefore,strAfter))
	//		{
	//			// 找到ORDER
	//			strFullSqlInfo << strBefore;
	//			strFullSqlInfo << " AND ";
	//			strFullSqlInfo << " ROWNUM ";
	//			strFullSqlInfo << " < ";
	//			strFullSqlInfo << iStartRow + iRowNum;
	//			strFullSqlInfo << " ";
	//			strFullSqlInfo << constrOrderKeyword;
	//			strFullSqlInfo << " ";
	//			strFullSqlInfo << strAfter;				
	//		}
	//		else
	//		{
	//			// 没有找到ORDER
	//			strFullSqlInfo << strConditionSql;
	//			strFullSqlInfo << " AND ";
	//			strFullSqlInfo << " ROWNUM ";
	//			strFullSqlInfo << " < ";
	//			strFullSqlInfo << iStartRow + iRowNum;
	//		}	
	//	}
	//}
	//else
	//{
	//	// 没有找到 WHERE，查找GROUP
	//	if (Find_keyWord_GetString(strConditionSql.c_str(),constrGroupKeyWord.c_str(),strBefore,strAfter))
	//	{
	//		// 1.查找GROUP,添加信息
	//		strFullSqlInfo << strBefore;
	//		strFullSqlInfo << " WHERE ";
	//		strFullSqlInfo << " ROWNUM ";
	//		strFullSqlInfo << " < ";
	//		strFullSqlInfo << iStartRow + iRowNum;
	//		strFullSqlInfo << " ";
	//		strFullSqlInfo << constrGroupKeyWord;
	//		strFullSqlInfo << " ";

	//		string		strTemp = strAfter;
	//		// 查找BY,添加信息
	//		if (Find_keyWord_GetString(strTemp.c_str(),constrBYKeyWord.c_str(),strBefore,strAfter))
	//		{
	//			strFullSqlInfo << constrBYKeyWord;
	//			strFullSqlInfo << " ROWNUM, ";
	//			strFullSqlInfo << strAfter;
	//		}
	//	}
	//	else
	//	{
	//		// 2.没有找到GROUP，就查找ORDER
	//		if (Find_keyWord_GetString(strConditionSql.c_str(),constrOrderKeyword.c_str(),strBefore,strAfter))
	//		{
	//			// 找到ORDER
	//			strFullSqlInfo << strBefore;
	//			strFullSqlInfo << " WHERE ";
	//			strFullSqlInfo << " ROWNUM ";
	//			strFullSqlInfo << " < ";
	//			strFullSqlInfo << iStartRow + iRowNum;
	//			strFullSqlInfo << " ";
	//			strFullSqlInfo << constrOrderKeyword;
	//			strFullSqlInfo << " ";
	//			strFullSqlInfo << strAfter;				
	//		}
	//		else
	//		{
	//			// 没有找到ORDER
	//			strFullSqlInfo << strConditionSql;
	//			strFullSqlInfo << " WHERE ";
	//			strFullSqlInfo << " ROWNUM ";
	//			strFullSqlInfo << " < ";
	//			strFullSqlInfo << iStartRow + iRowNum;
	//		}	
	//	}
	//}

	//strFullSqlInfo << " ) ";

	//strFullSqlInfo << " WHERE R >= ";
	//strFullSqlInfo << iStartRow;

	/*strFullSql	=	strFullSqlInfo.str();

	return	TRUE;*/
//}



/**************************************************************************************************
  Function: ExecuteQuery(const char*  szSql)    
  DateTime: 2010/5/26 17:18	
  Description:    	// 函数功能、性能等的描述
  Input:          	执行操作的Sql语句
  Output:         	NULL
  Return:         	IRecordset对象接口对象，可以根据IRecordset接口执行相应的接口提供方法
  Note:				
					IRecordset对象接口对象，可以根据IRecordset接口执行相应的接口提供方法
**************************************************************************************************/
IRecordSet*		COracleConnection::ExecuteQuery(const char*  szSql)
{
	// 创建一个对象，返回尾部使用
	COracleRecordSet*		pcsCOracleRecordSet = NULL;
	pcsCOracleRecordSet = new COracleRecordSet();
	// 设置连接对象指针
	pcsCOracleRecordSet->SetConnection(this);

	// 查询数据集
	if (pcsCOracleRecordSet->QuerySql(szSql)!=NULL)
	{
		return	pcsCOracleRecordSet;
	}
	else
	{
		delete	pcsCOracleRecordSet;
		pcsCOracleRecordSet = NULL;
		return NULL;
	}

}	

/**************************************************************************************************
Function		: GetEmptyRecordSet()
DateTime		: 2010/11/29 17:53	
Description		: 返回一个空的数据集，Addnew数据的时候用到的
Input			: NULL
Output			: NULL
Return			: NULL
Note			:
**************************************************************************************************/
IRecordSet*		COracleConnection::GetEmptyRecordSet()
{
	// 创建一个对象，返回尾部使用
	COracleRecordSet*		pcsCOracleRecordSet = NULL;
	pcsCOracleRecordSet = new COracleRecordSet();
	// 设置连接对象指针
	pcsCOracleRecordSet->SetConnection(this);

	// 在外面调用 ReleaseRecordSet函数释放
	return	pcsCOracleRecordSet;
}

/**************************************************************************************************
  Function:ReleaseConnection()     
  DateTime: 2010/5/26 17:22	
  Description:    	释放连接对象接口，将使用完毕后的连接对象，返回到连接池中
  Input:          	NULL
  Output:         	NULL
  Return:         	成功：TRUE，失败：FALSE
  Note:				
					释放连接对象接口，将使用完毕后的连接对象，返回到连接池中
**************************************************************************************************/
BOOL			COracleConnection::ReleaseConnection() 
{
	// 获取单例
	if( m_pCnnPool) 
	{
		CDBConnectionPool *p = (CDBConnectionPool*) m_pCnnPool;
		p->ReleaseConnection((CConnection*)this);
	}
	else
	{


		if (CDBConnectionPool::Instance()!=NULL)
		{
			CDBConnectionPool::Instance()->ReleaseConnection((CConnection*)this);
		}
	}
	return TRUE;

}



/**************************************************************************************************
Function		: GetConnectionErrorAndDeal()    
DateTime		: 2010/6/11 14:48	
Description		: 获取错误链接并进行相应操作：主要针对网络断开，数据库断开等操作控制
Input			: NULL
Output			: NULL
Return			: TRUE:成功  FALSE:失败
Note			:
**************************************************************************************************/
BOOL	COracleConnection::GetConnectionErrorAndDeal()
{
#if 0

	// 获取详细的错误代码,并进行相应处理
	{
		// 获取ADO连接对象
		_ConnectionPtr PtrCon = GetConnection();
		if (PtrCon == NULL)
		{
			return FALSE;
		}

		// 获取出错对象
		ErrorsPtr pErrors = PtrCon->Errors;
		int i = pErrors->Count;
		for (long n = 0; n < pErrors->Count; n++)
		{
			ErrorPtr pError = pErrors->GetItem(n);
			/*
			// 状态
			string	strSQLState = string((char*)pError->SQLState);
			// 错误
			long NativeError=pError->NativeError;

			long nNumber = pError->GetNumber();

			// 网络连接断开:重新建立连接
			if ((strSQLState == "08S0 ") && (NativeError==11))
			{
				Open();
			}
			// 网络连接断开:重新建立连接
			if ((strSQLState == "08001") && (NativeError==17))
			{
				Open();
			}
			// 网络连接断开:重新建立连接
			if ((strSQLState == "08S0 ") && (nNumber==0x80004005))
			{
				Open();
			}
			*/
			// 如果是ORACLE数据库就直接重新连接
			Open();
			break;
		}
	}

	return TRUE;
#else
	// linux
	Close();
	Open();
#endif

	return	TRUE;
}



BOOL COracleConnection::LoadSeqMap(void)
{
	m_setSeqName.clear();
	m_mapSeq.clear();
	m_strEmptySeqName.clear();

	IRecordSet *pSet = ExecuteQuery("select SEQUENCE_OWNER,SEQUENCE_NAME from ALL_SEQUENCES");
	if( !pSet )
	{
		GS_ASSERT(0);
		return FALSE;
	}
	std::string strVal;
	for( ; !pSet->Eof(); pSet->MoveNext() )
	{
		strVal.clear();
		pSet->GetCollect("SEQUENCE_NAME", strVal);
		if( !strVal.empty() )
		{
			m_setSeqName.insert(GSStrUtil::ToUpper(strVal));
		}
	}
	pSet->ReleaseRecordSet();
	return TRUE;
}

const std::string &COracleConnection::GetIDSequenceNameOfTableName( const std::string &strTableName )
{
	

	
	std::string strTemp = GSStrUtil::ToUpper(strTableName);

	std::map<std::string, std::string>::iterator csMapIt = m_mapSeq.find(strTemp);
	if( csMapIt != m_mapSeq.end() )
	{
		return csMapIt->second;
	}
	else
	{

		std::set<std::string >::iterator csIt;
		UINT iPos;
		UINT iPos2;
		for( csIt = m_setSeqName.begin(); csIt != m_setSeqName.end(); ++csIt )
		{
			iPos = (*csIt).find(strTemp);
			iPos2 = iPos+strTemp.length();
			if( iPos != std::string::npos &&
				(iPos==0 || (*csIt)[iPos-1] == '_' ) &&
				((*csIt)[iPos2] == '_' || (*csIt)[iPos2]=='\0') )
			{
				//找到
				m_mapSeq.insert(make_pair(strTemp, *csIt) ); //加入到map 加快下次的查找
				return *csIt;
			}
		}
		std::string strOther = strTemp.substr(strTemp.find("TB_")+3);
		for( csIt = m_setSeqName.begin(); csIt != m_setSeqName.end(); ++csIt )
		{
			iPos = (*csIt).find(strOther);
			iPos2 = iPos+strOther.length();
			if( iPos != std::string::npos &&
				(iPos==0 || (*csIt)[iPos-1] == '_' ) &&
				((*csIt)[iPos2] == '_' || (*csIt)[iPos2]=='\0') )
			{
				//找到
				m_mapSeq.insert(make_pair(strTemp, *csIt) ); //加入到map 加快下次的查找
				return *csIt;
			}
		}

	}
	return m_strEmptySeqName;
}