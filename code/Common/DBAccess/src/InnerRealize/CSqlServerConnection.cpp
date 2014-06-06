#include "CSqlServerConnection.h"
#include "CDBConnectionPool.h"
#include "CSqlServerRecordSet.h"

using	namespace DBAccessModule;

CSqlServerConnection::CSqlServerConnection(void)
{

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

CSqlServerConnection::~CSqlServerConnection(void)
{
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
成功：TURE
失败：FALSE
Note:		   
**************************************************************************************************/
BOOL	CSqlServerConnection::Open()	
{
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

	// SQL SERVER 连接字符串
	// Provider=SQLOLEDB.1;Password=1234;Persist Security Info=True;User ID=sa;Initial Catalog=C3M_VIDEO_TEST;Data Source=192.168.5.101\SQLEXPRESS
	sprintf_s(szConnectStr,"Provider=SQLOLEDB.1;Password=%s;Persist Security Info=True;User ID=%s;Initial Catalog=%s;Data Source=%s",m_szPass,m_szUser,m_szDatabase,m_szServer);


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
		//	HRESULT hr=m_pConnection->Open(_bstr_t("Provider=SQLOLEDB.1;Password=1234;Persist Security Info=True;User ID=sa;Initial Catalog=C3M_VIDEO_TEST;Data Source=192.168.5.101\SQLEXPRESS"),"","",adModeUnknown);
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

				// 判断是连接成功
				if (!(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO))
				{					
					std::cout<<"---> DBAccess Error：CSqlServerConnection::Open() DB SQLConnect failed !"<<endl;
					SQLFreeHandle(SQL_HANDLE_DBC, m_hDbc); 
					SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
					return FALSE;
				}
				else
				{
					printf( "INFO SqlServer Connect %s@%s success!\n",
						m_szDatabase, m_szUser );
				}
			} 
			else
			{
				std::cout<<"---> DBAccess Error：CSqlServerConnection::Open() DB SQLAllocHandle failed !"<<endl;
				// 申请Sql连接句柄失败
				SQLFreeHandle(SQL_HANDLE_DBC, m_hDbc); 
				SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
				return FALSE;
			}
		}
		else
		{
			std::cout<<"---> DBAccess Error：CSqlServerConnection::Open() DB SQLSetEnvAttr failed !"<<endl;
			// 设置Sql 环境句柄失败
			SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
			return FALSE;
		}
	}
	else
	{
		std::cout<<"---> DBAccess Error：CSqlServerConnection::Open() DB SQLAllocHandle failed !"<<endl;
		// 申请Sql环境句柄失败
		SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
		return FALSE;
	}

	m_bConnectFlag = TRUE;
#endif
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
BOOL	CSqlServerConnection::Close()
{
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
BOOL	CSqlServerConnection::IsOpen()
{
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
UINT	CSqlServerConnection::BeginTrans()
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
BOOL	CSqlServerConnection::RollbackTrans()
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
	// 	catch (_com_error e)
	catch(...)
	{
		//	TRACE(_T("Warning: RollbackTrans 方法发生异常. 错误信息: %s; 文件: %s; 行: %d\n"), e.ErrorMessage(), __FILE__, __LINE__);
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
BOOL	CSqlServerConnection::CommitTrans()
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
	// 	catch (_com_error e)
	catch(...)
	{
		//	TRACE(_T("Warning: RollbackTrans 方法发生异常. 错误信息: %s; 文件: %s; 行: %d\n"), e.ErrorMessage(), __FILE__, __LINE__);
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
BOOL			CSqlServerConnection::ExecuteSql(const char*  szSql)
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
					return	TRUE;
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
		}
		catch(...)
		{
			// 判断是否是连接出错
			GetConnectionErrorAndDeal();
		}
	}
#else
	//	linux 
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
			// 设置为静态游标
			//retcode = SQLSetStmtOption(hstmt, SQL_CURSOR_TYPE, SQL_CURSOR_STATIC);
			// 执行成功	// 执行Sql语句
			retcode = SQLExecDirect(hstmt, (SQLCHAR*)szSql, SQL_NTS) ;
			
			// SQL SERVER在删除数据库的时候虽然执行成功，但如果数据库是没有数据的会返回SQL_NO_DATA
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO || retcode == SQL_NO_DATA) 
			{
				SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
				return	TRUE;
			}
			else
			{				
				//----------------------------------------------------------------------------------
				// 输出错误信息 [11/30/2010 13:57 Modify by Liujs]
				ErrorLogInfo(m_hEnv,m_hDbc,hstmt);
				//----------------------------------------------------------------------------------

				SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
				return	FALSE;
			}
		}
		else
		{	

			//----------------------------------------------------------------------------------
			// 输出错误信息 [11/30/2010 13:57 Modify by Liujs]
			ErrorLogInfo(m_hEnv,m_hDbc,hstmt);
			//----------------------------------------------------------------------------------

			// 申请Sql 语句执行句柄错误，释放Sql查询语句句柄
			SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
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
Function		: ExecutePageQuery(const char *szSql, const INT32 iStartRow, const INT32 iRowNum)
DateTime		: 2010/12/2 10:26	
Description		: 分页查询
Input			: NULL
Output			: NULL
Return			: NULL
Note			:
**************************************************************************************************/
IRecordSet*		CSqlServerConnection::ExecutePageQuery(const char *szSql, const INT32 iStartRow, const INT32 iRowNum)
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
	// select top N * from tablename where id not in (select top M * from tablename) 
	// 从子集里面查询数据就可以了
	string	strFullSql = "";
	if (GetFullPageQuerySql(szSql,iStartRow,iRowNum,strFullSql))
	{
		// 创建一个对象，返回尾部使用
		CSqlServerRecordSet*		pcsCSqlServerRecordSet = NULL;
		pcsCSqlServerRecordSet = new CSqlServerRecordSet();
		// 设置连接对象指针
		pcsCSqlServerRecordSet->SetConnection(this);

		// 查询数据集
		if (pcsCSqlServerRecordSet->QuerySql(szSql)!=NULL)
		{
			return	pcsCSqlServerRecordSet;
		}
		else
		{
			delete	pcsCSqlServerRecordSet;
			pcsCSqlServerRecordSet = NULL;
			return NULL;
		}
	}
	return	NULL;
}

// 查找关键字，成功：TRUE，失败：FALSE 并返回关键字前后的字符串
BOOL	CSqlServerConnection::Find_keyWord_GetString(const char* szSql,const char* szKeyWord,string& strBefore,string& strAfter)
{
	strBefore	=	"";
	strAfter	=	"";

	// char*转换为string
	string	strSql		= string(szSql);
	string	strKeyWord	= string(szKeyWord);

	// 转换为大写
	transform(strSql.begin(),strSql.end(),strSql.begin(),ToUpper());
	transform(strKeyWord.begin(),strKeyWord.end(),strKeyWord.begin(),ToUpper());

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


// 完整字符串处理
// 输入：SELECT [id],[PerSonNo],[EmployerNo] FROM TB_DEVINFO EMP 
// 输出：
/* select top 20  [id] ,[PerSonNo],[EmployerNo] from [pub_tbPerSon] where id not in (select top 10 [id] from [pub_tbPerSon] ) */
BOOL		CSqlServerConnection::GetFullPageQuerySql(const char* szSql,const INT32	iStartRow,const INT32 iRowNum,string& strFullSql)
{
	// char*转化为string
	string	strSql = string(szSql);

	// strSql中字符全部转化为大写
	transform (strSql.begin(),strSql.end(), strSql.begin(),ToUpper()); 

	// 用于中间调度的string对象
	string	strTemp = "";

	// 插入行信息的字符串
	stringstream	strInfo;
	strInfo.str("");

	// 合成新的字符串
	strInfo << "SELECT ";
	strInfo << "TOP ";
	strInfo << iRowNum;
	strInfo << " ";

	// SELECT字符第一次出现的位置
	string::size_type	n_SELECT_Pos = strSql.find(constrSelectKeyword.c_str(),0);

	// 获取SELECT后面的字符串
	strSql	=	strSql.substr(n_SELECT_Pos + 6,strSql.size() - n_SELECT_Pos - 6);

	// FROM出现在szSql中的位置
	string::size_type n_FROM_Pos = strSql.find(constrFromKeyWord.c_str(),0);

	// 用于存储WHERE\ORDER\GROUP在szSql中最早出现的位置
	string::size_type n_KEYWORD_pos = string::npos;

	// WHERE出现在szSql中的位置
	string::size_type n_WHERE_Pos = strSql.find(constrWhereKeyWord.c_str(),0);

	// GROUP出现在szSql中的位置
	string::size_type n_GROUP_Pos = strSql.find(constrGroupKeyWord.c_str(),0);

	// ORDER出现在szSql中的位置
	string::size_type n_ORDER_Pos = strSql.find(constrOrderKeyword.c_str(),0);

	// 判断WHERE\ORDER\GROUP在szSql中最早出现的位置
	if (string::npos != n_WHERE_Pos)
	{
		n_KEYWORD_pos = n_WHERE_Pos;
	}
	else
	{
		if (string::npos != n_GROUP_Pos)
		{
			n_KEYWORD_pos = n_GROUP_Pos;
		}
		else
		{
			if (string::npos != n_ORDER_Pos)
			{
				n_KEYWORD_pos = n_ORDER_Pos;
			}
		}
	}

	// 如果strSql中存在WHERE\ORDER\GROUP中任意关键字
	if (string::npos != n_KEYWORD_pos)
	{
		// 截取WHERE\ORDER\GROUP在szSql中最早出现的位置之前的字符串
		strTemp = strSql.substr(0,n_KEYWORD_pos);

		// 添加strTemp中的内容
		strInfo << strTemp;

		// 如果存在WHERE关键字
		if (string::npos != n_WHERE_Pos)
		{
			if(string::npos != n_GROUP_Pos)
			{
				// 如果存在GROUP,截取WHERE关键字后面的判断语句
				strTemp = strSql.substr(n_WHERE_Pos,n_GROUP_Pos - n_WHERE_Pos);
				strInfo << strTemp;
				strInfo << " AND ID";
			}
			else
			{
				// 如果不存在GROUP关键字
				if (string::npos != n_ORDER_Pos)
				{
					// 如果存在ORDER,截取WHERE关键字后面的判断语句
					strTemp = strSql.substr(n_WHERE_Pos,n_ORDER_Pos - n_WHERE_Pos);
					strInfo << strTemp;
					strInfo << " AND ID";
				}
				else
				{
					// 如果不存在ORDER,截取WHERE关键字后面的判断语句
					strTemp = strSql.substr(n_WHERE_Pos,n_ORDER_Pos - n_WHERE_Pos);
					strInfo << strTemp;
					strInfo << " AND ID";
				}
			}
		}
		else
		{
			strInfo << " WHERE ID";
		}

		strInfo << " NOT IN (SELECT TOP ";
		strInfo << iStartRow;
		strInfo << " ID ";

		// szSql中FROM关键字出现的位置
		string::size_type	n_FROM_Pos =strSql.find(constrFromKeyWord.c_str());

		// 如果strSql中存在GROUP关键字
		if (string::npos != n_GROUP_Pos)
		{
			// 截取szSql中从FROM到GROUP之间的字符串
			strTemp = strSql.substr(n_FROM_Pos,strSql.length() - n_FROM_Pos);
			strInfo << strTemp;
			strInfo << ")";

			// 截取szSql中从GROUP到结尾的字符串
			strTemp = strSql.substr(n_GROUP_Pos,strSql.length() - n_GROUP_Pos);
			strInfo << strTemp;
		}
		// 如果strSql中不存在GROUP但存在GROUP关键字
		else if(string::npos != n_ORDER_Pos)
		{
			// 截取szSql中从FROM到GROUP之间的字符串
			strTemp = strSql.substr(n_FROM_Pos,strSql.length() - n_FROM_Pos);
			strInfo << strTemp;
			strInfo << ") ";

			// 截取szSql中从ORDER到结尾的字符串
			strInfo << " ";
			strTemp = strSql.substr(n_ORDER_Pos,strSql.length() - n_ORDER_Pos);
			strInfo << strTemp;
		}
		// strSql中不存在GROUP和ORDER字符串，即只存在WHERE关键字
		else
		{
			// 截取szSql中FROM到结尾的字符串
			strTemp = strSql.substr(n_FROM_Pos,strSql.length() - n_FROM_Pos);
			strInfo << strTemp;
			strInfo << ")";
		}
	}
	// strSql中不存在WHERE\ORDER\GROUP关键字
	else
	{
		strTemp = strSql;

		// 添加strTemp中的内容
		strInfo << strTemp;

		// 在szSql中截取FROM后面的字符串
		string::size_type	n_FROM_Pos =strSql.find(constrFromKeyWord.c_str());
		strTemp = strSql.substr(n_FROM_Pos,strSql.length()-n_FROM_Pos);

		strInfo << " WHERE ID NOT IN (SELECT TOP ";
		strInfo << iStartRow;
		strInfo << " ID ";
		strInfo << strTemp;
		strInfo << ")";
	}

	strFullSql	=	strInfo.str();
	return	TRUE;
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
BOOL			CSqlServerConnection::ExecuteSql(const char*  szSql,const char* szTable,INT64& lRowId)
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
					// Sql 查询语句
					// SELECT  ISNULL(MAX(ID),1) AS ID  FROM dbo.VID_TB_TEST_2
					sprintf(szInsertIdSql," SELECT  ISNULL(MAX(ID),1) AS %s  FROM %s",INSERT_ID_FIELED_NAME,szTable);

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

					// 提交事务
					if (bSucFlag)
					{
						return	TRUE;
					}				
				}
				else
				{
					// 回滚事务
					return	FALSE;
				}
			}// if (IsOpen())	
			else
			{
				// 打开数据库连接
				Open();
			}
		}
		catch(...)
		{
			// 判断是否是连接出错
			GetConnectionErrorAndDeal();
		}
	}
#else
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
			// 设置为静态游标
			//retcode = SQLSetStmtOption(hstmt, SQL_CURSOR_TYPE, SQL_CURSOR_STATIC);
			// 执行Sql语句
			// 执行成功	// 执行Sql语句
			retcode = SQLExecDirect(hstmt, (SQLCHAR*)szSql, SQL_NTS) ;
			
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) 
			{			
				// 是否对象
				SQLFreeHandle(SQL_HANDLE_STMT, hstmt);	

				// 申请SQL语句句柄，每次执行SQL语句都申请语句句柄，并且在执行完成后释放
				retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_hDbc, &hstmtQuery);	
				if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) 
				{
					//----------------------------------------------------------------------------------
					// 输出错误信息 [11/30/2010 13:57 Modify by Liujs]
					ErrorLogInfo(m_hEnv,m_hDbc,hstmtQuery);
					//----------------------------------------------------------------------------------
					SQLFreeHandle(SQL_HANDLE_STMT,	hstmt);	
					SQLFreeHandle(SQL_HANDLE_STMT,	hstmtQuery);
					return FALSE;
				}
				// 获取受影响的行的对应ID
				char	szInsertIdSql[coniSqlStrLen] = {0};
				memset(szInsertIdSql,0x0,coniSqlStrLen);
				// Sql 语句的用法
				// SELECT  NVL(MAX(ID),1) AS ID FROM VID_TB_TEST_2
				sprintf(szInsertIdSql," SELECT  ISNULL(MAX(ID),1) AS %s  FROM %s",INSERT_ID_FIELED_NAME,szTable);

				// 指定要使用的游标并发级别
				SQLSetStmtAttr(hstmtQuery, SQL_ATTR_CONCURRENCY,(SQLPOINTER) SQL_CONCUR_ROWVER, 0);
				// 设置光标类型为键集光标,
				SQLSetStmtAttr(hstmtQuery, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_DYNAMIC /* SQL_CURSOR_KEYSET_DRIVEN*/, 0);

				// 设置为静态游标
				//retcode = SQLSetStmtOption(hstmt, SQL_CURSOR_TYPE, SQL_CURSOR_STATIC);
				// 执行sql语句
				retcode = SQLExecDirect(hstmtQuery, (SQLCHAR*)szInsertIdSql, SQL_NTS);

				if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO)) 
				{
					//----------------------------------------------------------------------------------
					// 输出错误信息 [11/30/2010 13:57 Modify by Liujs]
					ErrorLogInfo(m_hEnv,m_hDbc,hstmtQuery);
					//----------------------------------------------------------------------------------
					SQLFreeHandle(SQL_HANDLE_STMT, hstmtQuery);	
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
						ErrorLogInfo(m_hEnv,m_hDbc,hstmtQuery);
						//----------------------------------------------------------------------------------
						// 获取数据失败
						SQLFreeHandle(SQL_HANDLE_STMT, hstmtQuery);	
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
				ErrorLogInfo(m_hEnv,m_hDbc,hstmt);
				//----------------------------------------------------------------------------------
				// 执行失败
				SQLFreeHandle(SQL_HANDLE_STMT, hstmt);	
				return FALSE;
			}			
		}
		else
		{	
			// 申请Sql 语句执行句柄错误，释放Sql查询语句句柄
			SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
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
Function: ExecuteQuery(const char*  szSql)    
DateTime: 2010/5/26 17:18	
Description:    	// 函数功能、性能等的描述
Input:          	执行操作的Sql语句
Output:         	NULL
Return:         	IRecordset对象接口对象，可以根据IRecordset接口执行相应的接口提供方法
Note:				
IRecordset对象接口对象，可以根据IRecordset接口执行相应的接口提供方法
**************************************************************************************************/
IRecordSet*		CSqlServerConnection::ExecuteQuery(const char*  szSql)
{
	// 创建一个对象，返回尾部使用
	CSqlServerRecordSet*		pcsCSqlServerRecordSet = NULL;
	pcsCSqlServerRecordSet = new CSqlServerRecordSet();
	// 设置连接对象指针
	pcsCSqlServerRecordSet->SetConnection(this);

	// 查询数据集
	if (pcsCSqlServerRecordSet->QuerySql(szSql)!=NULL)
	{
		return	pcsCSqlServerRecordSet;
	}
	else
	{
		delete	pcsCSqlServerRecordSet;
		pcsCSqlServerRecordSet = NULL;
		return NULL;
	}
}	

/**************************************************************************************************
  Function		: GetEmptyRecordSet()
  DateTime		: 2010/11/29 17:53	
  Description	: 返回一个空的数据集，Addnew数据的时候用到的
  Input			: NULL
  Output		: NULL
  Return		: NULL
  Note			:
**************************************************************************************************/
IRecordSet*		CSqlServerConnection::GetEmptyRecordSet()
{
	CSqlServerRecordSet*		pcsCSqlServerRecordSet = NULL;
	pcsCSqlServerRecordSet = new CSqlServerRecordSet();
	// 设置连接对象指针
	pcsCSqlServerRecordSet->SetConnection(this);

	// 在外面调用 ReleaseRecordSet函数释放
	return	pcsCSqlServerRecordSet;
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
BOOL			CSqlServerConnection::ReleaseConnection() 
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
  DateTime		: 2010/6/11 14:48	
  Description	: 获取错误链接并进行相应操作：主要针对网络断开，数据库断开等操作控制
  Input			: NULL
  Output		: NULL
  Return		: TRUE:成功  FALSE:失败
  Note			:
**************************************************************************************************/
BOOL	CSqlServerConnection::GetConnectionErrorAndDeal()
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
		for (long n = 0; n < pErrors->Count; n++)
		{
			ErrorPtr pError = pErrors->GetItem(n);

			// 状态
			string	strSQLState = string((char*)pError->SQLState);
			// 错误
			long NativeError=pError->NativeError;

			long nNumber = pError->GetNumber();
			//pError->Description;
			//pError->Source;

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

