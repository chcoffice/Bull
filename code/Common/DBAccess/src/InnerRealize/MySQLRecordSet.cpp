#include "MySQLRecordSet.h"
#include "MySQLConnection.h"

using namespace DBAccessModule;

// 游标索引
INT		CMySQLRecordSet::m_iCursorIndex = 0;

CMySQLRecordSet::CMySQLRecordSet(void)
{
	// linux
	m_hSqlStmt = SQL_NULL_HSTMT;
	m_bIsOpen = FALSE;
	// 编辑模式
	m_iEditMode = EM_UnKnown;
	// 表名称
	m_strTableName = constrDefaultString ;

	// 更新用的
	m_hUpdateSqlStmt = SQL_NULL_HSTMT;
	// 是否是记录集尾部
	m_bEof = TRUE;

	m_pConnection = NULL;
}

CMySQLRecordSet::~CMySQLRecordSet(void)
{
	if (IsOpen())
	{
		Close();
	}
	// linux
	if (m_bIsOpen)
	{
		m_bIsOpen = FALSE;
		SQLFreeHandle(SQL_HANDLE_STMT,m_hSqlStmt);
		m_hSqlStmt = SQL_NULL_HSTMT;
	}
	// 插入用的
	if (m_hUpdateSqlStmt != SQL_NULL_HSTMT)
	{
		SQLFreeHandle(SQL_HANDLE_STMT,m_hUpdateSqlStmt);
		m_hUpdateSqlStmt = SQL_NULL_HSTMT;
	}
	m_iEditMode = EM_UnKnown;
	m_bEof = TRUE;
}

// 查询游标是否在在记录集尾
BOOL	CMySQLRecordSet::Eof()
{
	// linux 
	m_iEditMode = EM_UnKnown;
	return m_bEof;

}

// 移向最后一条记录
BOOL	CMySQLRecordSet::MoveLast()
{
	// linux
	// 记录游标移动到最后一个位置
	SQLRETURN	retCode = 0;
	if(m_hSqlStmt != SQL_NULL_HSTMT)
	{
		// 移到最后一条记录
		retCode = SQLFetchScroll(m_hSqlStmt,SQL_FETCH_LAST,0);
		if (retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			// 清空自增长列号码标志
			InitCollumnIndexID();

			m_bEof = FALSE;
			return	TRUE;
		}
		else
		{
			m_bEof = TRUE;
			return FALSE;
		}
	}
	else
	{
		return	FALSE;
	}
}

// 数据集移动向下一条记录
BOOL	CMySQLRecordSet::MoveNext()
{
	// linux
	SQLRETURN	retCode = 0;
	if (m_hSqlStmt != SQL_NULL_HSTMT)
	{
		// 移动向下一条记录
		retCode = SQLFetchScroll(m_hSqlStmt, SQL_FETCH_NEXT, 0);
		if (retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			// 清空自增长列号码标志
			InitCollumnIndexID();

			m_bEof = FALSE;
			return	TRUE;
		}
		else
		{
			m_bEof = TRUE;
			return	FALSE;
		}
	}
	else
	{
		// 已经是最后一条记录了，不能再向后面移动索引
		return	FALSE;
	}

}

// 添加一条空记录
BOOL    CMySQLRecordSet::AddNew()
{
	// linux
	m_iEditMode = EM_AddNew;

	// 清空自增长列号码标志
	InitCollumnIndexID();
	ClearFieldInfoList();

	return	TRUE;

}

// 在一个连接上，指定的表上面插入数据
BOOL	CMySQLRecordSet::AddNew(const	char*	szTableName)
{
	// 初始化参数
	Init();

	// 设置插入标志
	m_iEditMode = EM_AddNew;

	// 清空自增长列号码标志
	InitCollumnIndexID();
	ClearFieldInfoList();

	// 设置表名
	m_strTableName	= string(szTableName);

	return	TRUE;
}

// 更新数据集
BOOL    CMySQLRecordSet::Update()
{
	// linux
	// 创建字符串
	if (BuildInsertUpdateSqlTxt())
	{
		// 添加值
		if (AppendValueToSqlTxt())
		{
			// 执行操作
			if (ExecuteInsertUpdate())
			{
				// 清空自增长列号码标志
				InitCollumnIndexID();
				// 清空列表
				ClearFieldInfoList();

				return TRUE;
			}
		}
	}
	// 清空列表
	ClearFieldInfoList();
	// 清空自增长列号码标志
	InitCollumnIndexID();
	// 如果更新失败,则取消更新
	CancelUpdate();
	return	FALSE;

}

// 根据输入字段名称获取字符串值
BOOL	CMySQLRecordSet::GetCollect(const char* szFieldName,string&	szValue)
{
	// linux
	try
	{	
		// 当前字段的字节长度
		SQLLEN		iActualDataSize = 0;
		SQLCHAR			szData[FIELD_DATA_LEN] = {0};
		memset(szData,0x0,FIELD_DATA_LEN);
		SQLRETURN		iRetFlag = 0;

		// 获取列索引
		SQLSMALLINT		iColumnIndex = GetColumnIndexByName(szFieldName);
		if(iColumnIndex == ERROR_SQL_COLUMN_INDEX)
		{
			return FALSE;
		}
		// 获取数据
		iRetFlag = SQLGetData(m_hSqlStmt,iColumnIndex,SQL_C_CHAR,szData,FIELD_DATA_LEN,&iActualDataSize);
		// 判断获取数据是否成功
		if (SQL_SUCCEEDED(iRetFlag)) 
		{
			// 如果长度大小不对，返回错误
			if (iActualDataSize > FIELD_DATA_LEN)
			{
				return	FALSE;
			}
			else
			{
				// 读取的数据长度小于大于实际的数据长度
				// 获取数据成功
				szValue	=	string((char*)szData);
			}
			return	TRUE;

		}
		else
		{
			// 获取数据失败
			return	FALSE;
		}

	}
	catch(...)
	{
		return	FALSE;
	}
	return FALSE;
}

// 根据列名设置对应列的值
BOOL	CMySQLRecordSet::PutCollect(const char* szFieldName,const char*     szValue)
{
	try
	{
		// 获取对应的列索引
		INT		iColumnIndex = GenerateCollumnIndexID();/* GetColumnIndexByName(szFieldName);*/
		if (iColumnIndex == ERROR_SQL_COLUMN_INDEX)
		{
			return	FALSE;
		}

		// 构造字段信息
		CDBFieldInfo*	pcsDbFieldInfoItem = NULL;
		pcsDbFieldInfoItem = new CDBFieldInfo();

		// 设置列索引
		pcsDbFieldInfoItem->SetColumnIndex(iColumnIndex);
		// 设置列名称
		pcsDbFieldInfoItem->SetFieldName(szFieldName);
		// 设置字段类型
		pcsDbFieldInfoItem->SetFieldType(FT_String);
		// 设置值
		pcsDbFieldInfoItem->SetStringValue(szValue);

		// 添加到列表中
		if(AddFieldItem2FieldInfoList(pcsDbFieldInfoItem))
		{
			// 设置编辑模式
			if (m_iEditMode == EM_UnKnown )
			{
				m_iEditMode = EM_Edit;
			}
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	catch(...)
	{
		return	FALSE;
	}
	return	FALSE;
}

// 根据列名称设置对应时间字段的值
BOOL	CMySQLRecordSet::PutDtCollect(const char* szFieldName,const INT iYear,const INT iMonth,const INT iDay,const INT iHour,const INT iMinute,const INT iSecond)
{
	try
	{
		// 获取对应的列索引
		INT		iColumnIndex = GenerateCollumnIndexID();
		if (iColumnIndex == ERROR_SQL_COLUMN_INDEX)
		{
			return	FALSE;
		}

		// 构造字段信息
		CDBFieldInfo*	pcsDbFieldInfoItem = NULL;
		pcsDbFieldInfoItem = new CDBFieldInfo();
		// 设置列索引
		pcsDbFieldInfoItem->SetColumnIndex(iColumnIndex);
		// 设置列名称
		pcsDbFieldInfoItem->SetFieldName(szFieldName);
		// 设置字段类型
		pcsDbFieldInfoItem->SetFieldType(FT_DateTime);
		// 设置值
		pcsDbFieldInfoItem->SetDateTimeValue(iYear,iMonth,iDay,iHour,iMinute,iSecond);

		// 添加到列表中
		if(AddFieldItem2FieldInfoList(pcsDbFieldInfoItem))
		{
			// 设置编辑模式
			if (m_iEditMode == EM_UnKnown )
			{
				m_iEditMode = EM_Edit;
			}
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	catch(...)
	{
		return	FALSE;
	}
	return	FALSE;
}

// 释放数据集对象
BOOL	CMySQLRecordSet::ReleaseRecordSet()
{
	delete	this;
	return  TRUE;
}

INT CMySQLRecordSet::GetColumnNumber(void)
{
	if(m_hSqlStmt == SQL_NULL_HSTMT)
	{
		return	-1;
	}
	return m_ColumnItemList.size();	

}


BOOL CMySQLRecordSet::GetCloumnName(INT iColIndex, std::string &oStrName )
{
	oStrName.clear();
	if(m_hSqlStmt == SQL_NULL_HSTMT || iColIndex<0 || iColIndex>=(int) m_ColumnItemList.size() )
	{
		return	FALSE;
	}
	oStrName = m_ColumnItemList[iColIndex].strColumn;
	return TRUE;
}

// 根据列名称获取对应字段的值，返回INT 类型字符串
BOOL	CMySQLRecordSet::GetCollect(const char* szFieldName,INT&	iValue)
{
	// linux
	try
	{	
		// 当前字段的字节长度
		SQLLEN		iActualDataSize = 0;
		SQLCHAR			szData[FIELD_DATA_LEN] = {0};
		memset(szData,0x0,FIELD_DATA_LEN);
		SQLRETURN		iRetFlag = 0;

		// 获取列索引
		SQLSMALLINT		iColumnIndex = GetColumnIndexByName(szFieldName);
		if(iColumnIndex == ERROR_SQL_COLUMN_INDEX)
		{
			return FALSE;
		}
		// 获取数据
		iRetFlag = SQLGetData(m_hSqlStmt,iColumnIndex,SQL_C_CHAR,szData,FIELD_DATA_LEN,&iActualDataSize);
		// 判断获取数据是否成功
		if (SQL_SUCCEEDED(iRetFlag)) 
		{
			// 如果长度大小不对，返回错误
			if (iActualDataSize > FIELD_DATA_LEN)
			{
				return	FALSE;
			}
			else
			{
				// 读取的数据长度小于大于实际的数据长度
				// 获取数据成功
				iValue = atol((char*)szData);
			}

			return	TRUE;

		}
		else
		{
			// 获取数据失败
			return	FALSE;
		}

	}
	catch(...)
	{
		return	FALSE;
	}
	return TRUE;
}

// 根据列名称获取对应字段的值，返回INT 类型字符串
BOOL	CMySQLRecordSet::PutCollect(const char* szFieldName,const INT	iValue)
{
	try
	{
		// 获取对应的列索引
		INT		iColumnIndex = GenerateCollumnIndexID();/*GetColumnIndexByName(szFieldName);*/
		if (iColumnIndex == ERROR_SQL_COLUMN_INDEX)
		{
			return	FALSE;
		}

		// 构造字段信息
		CDBFieldInfo*	pcsDbFieldInfoItem = NULL;
		pcsDbFieldInfoItem = new CDBFieldInfo();
		// 设置列索引
		pcsDbFieldInfoItem->SetColumnIndex(iColumnIndex);
		// 设置列名称
		pcsDbFieldInfoItem->SetFieldName(szFieldName);
		// 设置字段类型
		pcsDbFieldInfoItem->SetFieldType(FT_Integer);
		// 设置值
		pcsDbFieldInfoItem->SetIntegerValue(iValue);

		// 添加到列表中
		if(AddFieldItem2FieldInfoList(pcsDbFieldInfoItem))
		{
			// 设置编辑模式
			if (m_iEditMode == EM_UnKnown )
			{
				m_iEditMode = EM_Edit;
			}
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	catch(...)
	{
		return	FALSE;
	}
	return	FALSE;
}


// 根据字段名称设置相应的值
BOOL	CMySQLRecordSet::GetCollect(const char* szFieldName,void*	   pValue,const INT iBuffSize,INT&	iDataLen)
{
	// linux

	// 定义数据存储区
	SQLCHAR*		pszData = NULL;
	try
	{	
		// 当前字段的字节长度
		SQLLEN		iActualDataSize = 0;

		// 分配内存
		pszData = new SQLCHAR[iBuffSize + 1];
		memset(pszData,0x0,iBuffSize+1);
		SQLRETURN		iRetFlag = 0;

		// 获取列索引
		SQLSMALLINT		iColumnIndex = GetColumnIndexByName(szFieldName);
		if(iColumnIndex == ERROR_SQL_COLUMN_INDEX)
		{
			return FALSE;
		}
		// 获取数据
		iRetFlag = SQLGetData(m_hSqlStmt,iColumnIndex,SQL_C_BINARY,pszData,iBuffSize,&iActualDataSize);
		// 判断获取数据是否成功
		if (SQL_SUCCEEDED(iRetFlag)) 
		{
			if (iBuffSize < iActualDataSize)
			{
				// 缓冲区小于数据数据长度，只返回大小
				iDataLen = iActualDataSize;
			}
			else if (iActualDataSize <=0)
			{
				iDataLen = coniZero;
			}
			else
			{
				// 返回数据，并且返回大小
				memcpy(pValue,pszData,iActualDataSize);
				iDataLen = iActualDataSize;
			}

		}
		else
		{
			if (pszData != NULL)
			{
				delete[] pszData;
				pszData = NULL;
			}
			// 获取数据失败
			return	FALSE;
		}

		if (pszData != NULL)
		{
			delete[] pszData;
			pszData = NULL;
		}

	}
	catch(...)
	{
		if (pszData != NULL)
		{
			delete[] pszData;
			pszData = NULL;
		}
		return	FALSE;
	}
	return TRUE;
}

// 根据字段名称设置相应的值
BOOL	CMySQLRecordSet::PutCollect(const char* szFieldName,const  void*	   pValue,const INT	iDataLen)
{
	try
	{
		// 获取对应的列索引
		INT		iColumnIndex = GenerateCollumnIndexID();/*GetColumnIndexByName(szFieldName);*/
		if (iColumnIndex == ERROR_SQL_COLUMN_INDEX)
		{
			return	FALSE;
		}

		// 构造字段信息
		CDBFieldInfo*	pcsDbFieldInfoItem = NULL;
		pcsDbFieldInfoItem = new CDBFieldInfo();
		// 设置列索引
		pcsDbFieldInfoItem->SetColumnIndex(iColumnIndex);
		// 设置列名称
		pcsDbFieldInfoItem->SetFieldName(szFieldName);
		// 设置字段类型
		pcsDbFieldInfoItem->SetFieldType(FT_Binary);
		// 设置二进制数据的长度
		pcsDbFieldInfoItem->SetBinaryValueLen(iDataLen);
		// 设置二进制数据值(数据的值和大小)
		pcsDbFieldInfoItem->SetBinaryValue(pValue,iDataLen);

		// 添加到列表中
		if(AddFieldItem2FieldInfoList(pcsDbFieldInfoItem))
		{
			// 设置编辑模式
			if (m_iEditMode == EM_UnKnown )
			{
				m_iEditMode = EM_Edit;
			}
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	catch(...)
	{
		return	FALSE;
	}
	return TRUE;

}

// 执行Sql语句返回数据集
IRecordSet*		CMySQLRecordSet::QuerySql(const char*	szSql)
{	
	//	ASSERT(m_pRecordset != NULL)
	// 执行操作返回数据集对象
	if (Open(szSql))
	{
		return (IRecordSet*)this;
	}
	else
	{
		return	NULL;
	}

}

// 获取数据集判断是否可以进行编辑
BOOL		CMySQLRecordSet::GetEditMode()
{
	return	FALSE; 
}


// 设置连接对象
BOOL			CMySQLRecordSet::SetConnection(CMySQLConnection* pConnection)
{
	if (pConnection != NULL)
	{
		m_pConnection	=	pConnection;
		return	TRUE;
	}
	return FALSE;

}

// 取消在调用 Update 方法前对当前记录或新记录所作的任何更改
BOOL	CMySQLRecordSet::CancelUpdate()
{
	//linux
	ClearFieldInfoList();	

	// 设置编辑模式为UnKnown
	m_iEditMode = EM_UnKnown;

	// 释放更新句柄
	if (m_hUpdateSqlStmt!=SQL_NULL_HSTMT)
	{
		SQLFreeHandle(SQL_HANDLE_STMT,m_hUpdateSqlStmt);
		m_hUpdateSqlStmt = SQL_NULL_HSTMT;
	}
	// 异常返回
	return FALSE;
}


// 
BOOL	CMySQLRecordSet::IsOpen()
{
	// linux
	return	m_bIsOpen;

}


// 关闭数据集
BOOL	CMySQLRecordSet::Close()
{
	//linux
	if (IsOpen())
	{
		m_bIsOpen = FALSE;
		SQLFreeHandle(SQL_HANDLE_STMT,m_hSqlStmt);
		m_hSqlStmt = SQL_NULL_HSTMT;
	}
	return	TRUE;

}


// 打开数据集
BOOL	CMySQLRecordSet::Open(const char*  szSql)
{
	//linux
	// 打开数据集对象
	try
	{
		// 关闭数据集对象
		if (IsOpen()) 
		{
			Close();
		}

		// 判断准备连接是否成功
		if (Init()) 
		{	
			// 执行sql语句
			SQLINTEGER	iRetCode = -1;
			iRetCode	=	SQLExecDirect(m_hSqlStmt, (SQLCHAR*)szSql, SQL_NTS);
			if ( iRetCode == SQL_ERROR)
			{
				Close();
				return FALSE;
			}
			else
			{
				m_strTableName = constrDefaultString ;
				// 已经执行成功，打开数据集了
				m_bIsOpen = TRUE;
				// 获取列名相关信息存入列表
				GetColumnList();
				// 光标移动到第一条记录
				iRetCode = SQLFetchScroll(m_hSqlStmt, SQL_FETCH_FIRST, 0);
				// 判断是否有记录存在
				if((iRetCode != SQL_SUCCESS) && (iRetCode != SQL_SUCCESS_WITH_INFO))
				{
					m_bEof = TRUE;
				}
				else
				{
					m_bEof = FALSE;
				}
				// 从Sql字符串中提取出表名称
				if (!GetTableNameFromSqlText(szSql,m_strTableName))
				{
					// 如果找不到表名称，就不能够更新，添加等操作
					m_iEditMode = EM_NoUpdate;
				}

				// 清空自增长列号码标志
				InitCollumnIndexID();

				return  TRUE;		
			}
		}
		else
		{	
			return FALSE;	
		}

	}
	catch (...)
	{
		// 如果断开连接了，要进行重新连接
		m_pConnection->GetConnectionErrorAndDeal();

		m_bIsOpen = FALSE;
		return FALSE;
	}
	return TRUE;
}

// 初始化STMT句柄相关信息
BOOL		CMySQLRecordSet::Init()
{
	try
	{ 
		// 先判断句柄是否可用
		if ( (m_pConnection->GetOdbcSqlHDbc() == SQL_NULL_HANDLE ) || (!m_pConnection->IsOpen()))
		{
			return FALSE;
		}

		SQLRETURN	retCode = 0;
		//分配SQL语句句柄
		retCode = SQLAllocHandle( SQL_HANDLE_STMT,m_pConnection->GetOdbcSqlHDbc(),&m_hSqlStmt);

		// 判断是否申请成功
		if (retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO) 
		{	
			//指定要使用的游标并发级别
			retCode = SQLSetStmtAttr(m_hSqlStmt, SQL_ATTR_CONCURRENCY,(SQLPOINTER) SQL_CONCUR_ROWVER, 0);
			if ( (retCode != SQL_SUCCESS) && (retCode != SQL_SUCCESS_WITH_INFO)) 
			{
				//----------------------------------------------------------------------------------
				// 输出错误信息 [11/30/2010 13:57 Modify by Liujs]
				ErrorLogInfo(m_pConnection->GetOdbcSqlHEven(),m_pConnection->GetOdbcSqlHDbc(),m_hSqlStmt);
				//----------------------------------------------------------------------------------
				Close(); 
				return FALSE;
			}


			//设置光标类型为键集光标,
			//键集光标能够检测到行的删除和修改，但是无法检测到检测到行的添加和结果集顺序变化。
			//因为在光标创建时就创建了整个结果集，结果集合中记录和顺序已经被固定，
			//这一点和静态光标一样。所以键集光标可以说是一种介于静态光标和动态光标之间的光标类型。
			retCode = SQLSetStmtAttr(m_hSqlStmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_DYNAMIC /* SQL_CURSOR_KEYSET_DRIVEN*/, 0);
			if ( (retCode != SQL_SUCCESS) && (retCode != SQL_SUCCESS_WITH_INFO)) 
			{
				//----------------------------------------------------------------------------------
				// 输出错误信息 [11/30/2010 13:57 Modify by Liujs]
				ErrorLogInfo(m_pConnection->GetOdbcSqlHEven(),m_pConnection->GetOdbcSqlHDbc(),m_hSqlStmt);
				//----------------------------------------------------------------------------------
				Close(); 
				return FALSE;
			}

			//-----------------------------MySQL不支持游标设置-------------------------
			//// 设置游标名称
			//char		szCursorName[MAX_CURSOR_NAME_LEN] = {0x0};
			//sprintf(szCursorName,"%s_%d",CURRENT_CURSOR_NAME,GetCursorIndex());
			//retCode = SQLSetCursorName(m_hSqlStmt,reinterpret_cast<SQLCHAR*>(szCursorName), SQL_NTS);
			//if ( (retCode != SQL_SUCCESS) && (retCode != SQL_SUCCESS_WITH_INFO)) 
			//{
			//	//----------------------------------------------------------------------------------
			//	// 输出错误信息 [11/30/2010 13:57 Modify by Liujs]
			//	
			//	int ierror;
			//	ierror= GetLastError();
			//	ErrorLogInfo(m_pConnection->GetOdbcSqlHEven(),m_pConnection->GetOdbcSqlHDbc(),m_hSqlStmt);
			//	//----------------------------------------------------------------------------------
			//	Close(); 
			//	return FALSE;
			//}

		}
		else
		{
			Close();
			return FALSE;
		}
	}
	catch (...)
	{
		Close();
		return FALSE;
	}

	return TRUE;
}

// 获取字段名称列表
BOOL		CMySQLRecordSet::GetColumnList()
{
	if (m_hSqlStmt == SQL_NULL_HSTMT)
	{
		return FALSE;
	}
	m_ColumnItemList.clear();

	// 列数
	SQLSMALLINT		iColumnNum = 0;
	// 获取列名称
	SQLNumResultCols(m_hSqlStmt, &iColumnNum);
	SQLCHAR			szColumnName[MAX_FNAME_LEN] = {0};


	int nType = SQL_C_DEFAULT;
	// 这些数据，在这个函数中，我们不用关心
	SQLSMALLINT		nSwCol=0, nSwType=0, nSwScale=0, nSwNull=0; 
	SQLULEN		pcbColDef=0;
	SQLRETURN		iRet=0;

	// 获取列名
	for(INT	i = 1;i<= iColumnNum ;i++)
	{
		// 列名
		memset(szColumnName,0x0,MAX_FNAME_LEN);
		// 获取列的相关信息
		iRet = SQLDescribeCol(m_hSqlStmt, i,(SQLTCHAR*)szColumnName,MAX_FNAME_LEN,&nSwCol, &nSwType, &pcbColDef,&nSwScale, &nSwNull); 
		if (iRet == SQL_SUCCESS || iRet == SQL_SUCCESS_WITH_INFO)
		{
			// 获取列名等信息
			StruColumnItem		stColumnItem;
			stColumnItem.strColumn = string((char*)szColumnName);
			stColumnItem.iColumnIndex = i;

			// 加入队列
			m_ColumnItemList.push_back(stColumnItem);
		}
		else
		{
			// 关闭
			Close();
			return FALSE;
		}
	}

	return	TRUE;
}

// 清空字段列表
BOOL		CMySQLRecordSet::ClearColumnList()
{
	m_ColumnItemList.clear();
	return	TRUE;

}


// 根据字段名称，获取列的索引
INT		  CMySQLRecordSet::GetColumnIndexByName(const char* szFieldName)
{
	if(m_hSqlStmt == SQL_NULL_HSTMT)
	{
		return	ERROR_SQL_COLUMN_INDEX;
	}

	INT		iColumnIndex = ERROR_SQL_COLUMN_INDEX;
	// 列名字符串
	string	strColumnName = string(szFieldName);
	// 获取列名称对应的索引号码
	for (ColumnItemVector::size_type i = 0;i< m_ColumnItemList.size();i++)
	{
		if(strColumnName == m_ColumnItemList[i].strColumn)
		{
			iColumnIndex = m_ColumnItemList[i].iColumnIndex;
			break;
		}
	}
	// 返回列的索引
	return	iColumnIndex;

}

// 清空数据字段列表（m_FieldInfoList ： AddNew 和 Update的时候用的）
BOOL		CMySQLRecordSet::ClearFieldInfoList()
{
	// 清空字段列表，
	for (DBFieldInfoVector::size_type i = 0;i<m_FieldInfoList.size();i++)
	{
		// 清空内存
		CDBFieldInfo*	pDbFieldInfo = NULL;
		pDbFieldInfo = m_FieldInfoList[i];
		if (pDbFieldInfo!=NULL)
		{
			pDbFieldInfo->Clear();
			delete	pDbFieldInfo;
			pDbFieldInfo = NULL;
		}
	}
	m_FieldInfoList.clear();

	return TRUE;

}

// 向字段列表中添加字段信息
BOOL		CMySQLRecordSet::AddFieldItem2FieldInfoList(CDBFieldInfo*	pcsDbFieldInfoItem)
{	
	// 先判断列名是否存在
	for (DBFieldInfoVector::size_type i = 0 ;i <m_FieldInfoList.size();i++)
	{
		if (m_FieldInfoList[i]->GetFieldName() == pcsDbFieldInfoItem->GetFieldName())
		{
			// 已经存在就返回
			return	FALSE;
		}
	}

	// 添加到列表中
	m_FieldInfoList.push_back(pcsDbFieldInfoItem);
	return	TRUE;

}

// 根据查询数据的sql语句来获取其中的表名称(主要是对表查询后，进行插入数据和更新数据的时候用)
BOOL		CMySQLRecordSet::GetTableNameFromSqlText(const char* szSql,string& strTableName)
{
	try
	{
		string	strSql  = string(szSql);
		// 查找里面第一个from后面的字符串


		// 转化为大写
		// transform (strSql.begin(),strSql.end(), strSql.begin(),toupper); 
		transform(strSql.begin(),strSql.end(),strSql.begin(),ToUpper());


		// FROM 字符出现的位置
		string::size_type	nFromPos = strSql.find(constrFromKeyWord.c_str());

		// 没有找到
		if (nFromPos == string::npos)
		{
			return	FALSE;
		}

		// 去除FROM以前的字符串
		strSql	=	strSql.substr(nFromPos+4,strSql.size() - nFromPos -4);

		// 删除表名前面的空格字符串
		strSql.erase(0,strSql.find_first_not_of(" "));
		// 去除右面的空格
		strSql.erase(strSql.find_last_not_of(" ") + 1,strSql.size()-1);

		// 开始查找表名称 "   TABLE "或者" TABLE"等
		// 表名称结束位置
		string::size_type	nTableTailPos  = strSql.find_first_of(" ");
		if (nTableTailPos == string::npos)
		{
			// 表名后面没有空格
			strTableName = strSql;
		}
		else
		{
			// 截取表名
			strTableName	=	strSql.substr(0,nTableTailPos);
		}

	}
	catch(...)
	{
		return	FALSE;
	}

	return	TRUE;
}


// 构造Sql语句进行操作
BOOL		CMySQLRecordSet::BuildInsertUpdateSqlTxt()
{
	// 确保有元素存在
	if (m_FieldInfoList.empty())
	{
		return FALSE;
	}

	if (m_hUpdateSqlStmt != SQL_NULL_HSTMT)
	{
		SQLFreeHandle(SQL_HANDLE_STMT,m_hUpdateSqlStmt);
		m_hUpdateSqlStmt = SQL_NULL_HSTMT;
	}

	// 返回句柄
	SQLRETURN retcode;
	// 申请Sql语句句柄
	retcode = SQLAllocHandle(SQL_HANDLE_STMT,m_pConnection->GetOdbcSqlHDbc(),&m_hUpdateSqlStmt);	
	// 判断是否申请成功
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) 
	{	
		// 清空字符串
		m_strSqlText = constrDefaultString;

		// 判断类型
		if (m_iEditMode == EM_AddNew)
		{
			// INSERT 操作
			m_strSqlText = "INSERT INTO ";
			m_strSqlText += m_strTableName;
			m_strSqlText += " ( ";
			// 逐个名称加入
			for (DBFieldInfoVector::size_type i=0;i<m_FieldInfoList.size();i++)
			{
				m_strSqlText += m_FieldInfoList[i]->GetFieldName();
				if (i!=m_FieldInfoList.size() - 1)
				{
					m_strSqlText += ",";
				}
				else
				{
					m_strSqlText += " ";
				}
			}

			m_strSqlText += " ) VALUES (";

			// 加入值
			for (DBFieldInfoVector::size_type j=0;j<m_FieldInfoList.size();j++)
			{
				m_strSqlText += "?";
				if (j!=m_FieldInfoList.size() - 1)
				{
					m_strSqlText += ",";
				}
				else
				{
					m_strSqlText += ")";
				}
			}

			// 例如： INSERT INTO TB_TEST(ID,NAME,KK) VALUES(?,?,?)

			// 准备Sql 语句
			SQLRETURN	retCode = 0;
			retcode = SQLPrepare(m_hUpdateSqlStmt,(SQLCHAR*)m_strSqlText.c_str(),SQL_NTS);
			if (retcode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{
				return TRUE;
			}
			//----------------------------------------------------------------------------------
			// 输出错误信息 [11/30/2010 13:57 Modify by Liujs]
			ErrorLogInfo(m_pConnection->GetOdbcSqlHEven(),m_pConnection->GetOdbcSqlHDbc(),m_hUpdateSqlStmt);
			//----------------------------------------------------------------------------------

		}
		else if (m_iEditMode == EM_Edit)
		{
			// UPDATE 操作
			m_strSqlText = "UPDATE ";
			m_strSqlText += m_strTableName;
			m_strSqlText += " Set ";

			// 设置列名
			// 逐个名称加入
			for (DBFieldInfoVector::size_type i=0;i<m_FieldInfoList.size();i++)
			{
				m_strSqlText += m_FieldInfoList[i]->GetFieldName();
				m_strSqlText += " = ? ";
				if (i!=m_FieldInfoList.size() - 1)
				{
					m_strSqlText += ",";
				}
				else
				{
					m_strSqlText += " ";
				}
			}

			// 加上最后的条件,通过游标来更新
			// m_strSqlText += " WHERE CURRENT OF CURRENT_CURSOR_NAME";

			m_strSqlText += " WHERE CURRENT OF ";
			SQLRETURN	nRetCode = 0;
			unsigned char	szCursorName[MAX_CURSOR_NAME_LEN+1] = {0};
			memset(szCursorName,0x0,MAX_CURSOR_NAME_LEN+1);
			SQLSMALLINT			iLenght = 0;
			nRetCode	=	SQLGetCursorName(m_hSqlStmt,szCursorName,MAX_CURSOR_NAME_LEN,&iLenght);
			if (nRetCode == SQL_SUCCESS || nRetCode == SQL_SUCCESS_WITH_INFO)
			{
				string	strTemp = (char*)szCursorName;
				m_strSqlText	+= strTemp;
			}
			else
			{
				//----------------------------------------------------------------------------------
				// 输出错误信息 [11/30/2010 13:57 Modify by Liujs]
				ErrorLogInfo(m_pConnection->GetOdbcSqlHEven(),m_pConnection->GetOdbcSqlHDbc(),m_hUpdateSqlStmt);
				//----------------------------------------------------------------------------------
				return FALSE;
			}

			// 例如： UPDATE TB_TEST SET NAME = ? ,PASS= ? WHERE KK = 'E'

			// 准备Sql 语句
			SQLRETURN	retCode = 0;
			retcode = SQLPrepare(m_hUpdateSqlStmt,(SQLCHAR*)m_strSqlText.c_str(),SQL_NTS);
			if (retcode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{
				return TRUE;
			}
			//----------------------------------------------------------------------------------
			// 输出错误信息 [11/30/2010 13:57 Modify by Liujs]
			ErrorLogInfo(m_pConnection->GetOdbcSqlHEven(),m_pConnection->GetOdbcSqlHDbc(),m_hUpdateSqlStmt);
			//----------------------------------------------------------------------------------
		}
		else
		{	
			// 只处理，更新和插入的，别的状态视为错误
			SQLFreeHandle(SQL_HANDLE_STMT,m_hUpdateSqlStmt);
			m_hUpdateSqlStmt = SQL_NULL_HSTMT;

			return FALSE;
		}

	}
	else
	{	
		// 声请Sql语句句柄失败
		SQLFreeHandle(SQL_HANDLE_STMT, m_hSqlStmt);
		return FALSE;	
	}

	return	FALSE;

}


// 向Sql语句里面添加对应的值
BOOL		CMySQLRecordSet::AppendValueToSqlTxt()
{
	// 确保有元素存在
	if (m_FieldInfoList.empty() || m_strSqlText.empty())
	{
		return FALSE;
	}

	// SQL 返回值
	SQLRETURN	retCode;

	// 判断对象是否存在
	if (m_hUpdateSqlStmt != SQL_NULL_HSTMT) 
	{	
		// 判断类型
		if (m_iEditMode == EM_AddNew)
		{
			// 例如： INSERT INTO TB_TEST(ID,NAME,KK) VALUES(?,?,?)

			// 逐个值的绑定添加
			for (DBFieldInfoVector::size_type i=0;i<m_FieldInfoList.size();i++)
			{
				switch (m_FieldInfoList[i]->GetFieldType())
				{
				case	FT_Integer:
					{
						// 绑定数据
						retCode = SQLBindParameter(m_hUpdateSqlStmt, m_FieldInfoList[i]->GetColumnIndex(),
							SQL_PARAM_INPUT,SQL_C_LONG,SQL_INTEGER, 0, 
							0,(SQLINTEGER*)&(m_FieldInfoList[i]->m_iRefValue),0,
							(SQLLEN*)&(m_FieldInfoList[i]->m_icpValue));

						// 拷贝数据
						m_FieldInfoList[i]->m_iRefValue = m_FieldInfoList[i]->GetIntegerValue();
						m_FieldInfoList[i]->m_icpValue = SQL_NTS;

						// 判断绑定是否成功
						if ((retCode != SQL_SUCCESS) && (retCode != SQL_SUCCESS_WITH_INFO))
						{
							//----------------------------------------------------------------------------------
							// 输出错误信息 [11/30/2010 13:57 Modify by Liujs]
							ErrorLogInfo(m_pConnection->GetOdbcSqlHEven(),m_pConnection->GetOdbcSqlHDbc(),m_hUpdateSqlStmt);
							//----------------------------------------------------------------------------------
							return FALSE;
						}
					} 
					break;
				case	FT_String:
					{
						// 绑定数据
						retCode = SQLBindParameter(m_hUpdateSqlStmt, m_FieldInfoList[i]->GetColumnIndex(), 
							SQL_PARAM_INPUT,SQL_C_CHAR,SQL_VARCHAR,	FIELD_DATA_LEN,0,
							(SQLCHAR*)(m_FieldInfoList[i]->m_szRefValue),0,
							(SQLLEN*)&(m_FieldInfoList[i]->m_icpValue));

						// 拷贝数据
						strcpy((m_FieldInfoList[i]->m_szRefValue),m_FieldInfoList[i]->GetStringValue().c_str());
						// 拷贝数据
						m_FieldInfoList[i]->m_icpValue = SQL_NTS;

						// 判断绑定是否成功
						if ((retCode != SQL_SUCCESS) && (retCode != SQL_SUCCESS_WITH_INFO))
						{
							//----------------------------------------------------------------------------------
							// 输出错误信息 [11/30/2010 13:57 Modify by Liujs]
							ErrorLogInfo(m_pConnection->GetOdbcSqlHEven(),m_pConnection->GetOdbcSqlHDbc(),m_hUpdateSqlStmt);
							//----------------------------------------------------------------------------------
							return FALSE;
						}

					} 
					break;
				case	FT_DateTime:
					{
						// 绑定数据
						retCode = SQLBindParameter(m_hUpdateSqlStmt,m_FieldInfoList[i]->GetColumnIndex(),
							SQL_PARAM_INPUT,SQL_C_TIMESTAMP,SQL_DATETIME,
							sizeof(TIMESTAMP_STRUCT),0,&m_FieldInfoList[i]->m_stDateTime,
							sizeof(TIMESTAMP_STRUCT),(SQLLEN*)&(m_FieldInfoList[i]->m_icpValue)); 

						// 拷贝数据
						m_FieldInfoList[i]->m_icpValue = 3;

						// 判断绑定是否成功
						if ((retCode != SQL_SUCCESS) && (retCode != SQL_SUCCESS_WITH_INFO))
						{
							//----------------------------------------------------------------------------------
							// 输出错误信息 [11/30/2010 13:57 Modify by Liujs]
							ErrorLogInfo(m_pConnection->GetOdbcSqlHEven(),m_pConnection->GetOdbcSqlHDbc(),m_hUpdateSqlStmt);
							//----------------------------------------------------------------------------------
							return FALSE;
						}

					}
					break;
				case FT_Binary:
					{
						// 二进制大对象 FT_Binary

						// 特别说明：ORACLE 和 SQL SERVER 的二进制操作方法不一样，别的方法都一样		
						// 刘建顺
						// 绑定数据
						retCode = SQLBindParameter(m_hUpdateSqlStmt, m_FieldInfoList[i]->GetColumnIndex(), 
							SQL_PARAM_INPUT,SQL_C_BINARY,SQL_LONGVARBINARY,
							0,0,
							(SQLPOINTER)m_FieldInfoList[i]->m_pValue,
							m_FieldInfoList[i]->GetBinaryValueLen(),(SQLLEN*)&(m_FieldInfoList[i]->m_icpValue));

						// 拷贝数据					
						/*
						The length of the parameter value stored in *ParameterValuePtr. This is ignored except for character or binary C data.
						SQL_NTS. The parameter value is a null-terminated string.
						*/
						m_FieldInfoList[i]->m_icpValue = m_FieldInfoList[i]->GetBinaryValueLen();

						// 判断绑定是否成功
						if ((retCode != SQL_SUCCESS) && (retCode != SQL_SUCCESS_WITH_INFO))
						{
							//----------------------------------------------------------------------------------
							// 输出错误信息 [11/30/2010 13:57 Modify by Liujs]
							ErrorLogInfo(m_pConnection->GetOdbcSqlHEven(),m_pConnection->GetOdbcSqlHDbc(),m_hUpdateSqlStmt);
							//----------------------------------------------------------------------------------
							return FALSE;
						}

					} 
					break;
				default:
					break;
				}

			} // end for (DBFieldInfoVector::size_type i=0;i<m_FieldInfoList.size();i++)

		} // end  if (m_iEditMode == EM_AddNew)
		else if (m_iEditMode == EM_Edit)
		{
			// UPDATE 操作
			// 例如： UPDATE TB_TEST SET NAME = ? ,PASS= ? WHERE KK = 'E'

			// 逐个值的绑定添加
			for (DBFieldInfoVector::size_type i=0;i<m_FieldInfoList.size();i++)
			{
				switch (m_FieldInfoList[i]->GetFieldType())
				{
				case FT_Integer:
					{
						// 绑定数据
						retCode = SQLBindParameter(m_hUpdateSqlStmt, m_FieldInfoList[i]->GetColumnIndex(),
							SQL_PARAM_INPUT,SQL_C_LONG,SQL_INTEGER, 0, 
							0,(SQLINTEGER*)&(m_FieldInfoList[i]->m_iRefValue),0,
							(SQLLEN*)&(m_FieldInfoList[i]->m_icpValue));

						// 拷贝数据
						m_FieldInfoList[i]->m_iRefValue = m_FieldInfoList[i]->GetIntegerValue();
						m_FieldInfoList[i]->m_icpValue = SQL_NTS;

						// 判断绑定是否成功
						if ((retCode != SQL_SUCCESS) && (retCode != SQL_SUCCESS_WITH_INFO))
						{
							//----------------------------------------------------------------------------------
							// 输出错误信息 [11/30/2010 13:57 Modify by Liujs]
							ErrorLogInfo(m_pConnection->GetOdbcSqlHEven(),m_pConnection->GetOdbcSqlHDbc(),m_hUpdateSqlStmt);
							//----------------------------------------------------------------------------------
							return FALSE;
						}

					} 
					break;
				case FT_String:
					{
						// 绑定数据
						retCode = SQLBindParameter(m_hUpdateSqlStmt, m_FieldInfoList[i]->GetColumnIndex(), 
							SQL_PARAM_INPUT,SQL_C_CHAR,SQL_VARCHAR,	FIELD_DATA_LEN,0,
							(SQLCHAR*)(m_FieldInfoList[i]->m_szRefValue),0,
							(SQLLEN*)&(m_FieldInfoList[i]->m_icpValue));

						// 拷贝数据
						strcpy((m_FieldInfoList[i]->m_szRefValue),m_FieldInfoList[i]->GetStringValue().c_str());
						// 拷贝数据
						m_FieldInfoList[i]->m_icpValue = (INT)SQL_NTS;

						// 判断绑定是否成功
						if ((retCode != SQL_SUCCESS) && (retCode != SQL_SUCCESS_WITH_INFO))
						{
							//----------------------------------------------------------------------------------
							// 输出错误信息 [11/30/2010 13:57 Modify by Liujs]
							ErrorLogInfo(m_pConnection->GetOdbcSqlHEven(),m_pConnection->GetOdbcSqlHDbc(),m_hUpdateSqlStmt);
							//----------------------------------------------------------------------------------
							return FALSE;
						}

					} 
					break;
				case FT_DateTime:
					{
						// 绑定数据
						retCode = SQLBindParameter(m_hUpdateSqlStmt,m_FieldInfoList[i]->GetColumnIndex(),
							SQL_PARAM_INPUT,SQL_C_TIMESTAMP,SQL_DATETIME,
							sizeof(TIMESTAMP_STRUCT), 0,&m_FieldInfoList[i]->m_stDateTime, 
							sizeof(TIMESTAMP_STRUCT),(SQLLEN*)&(m_FieldInfoList[i]->m_icpValue)); 

						// 拷贝数据
						m_FieldInfoList[i]->m_icpValue = 3;

						// 判断绑定是否成功
						if ((retCode != SQL_SUCCESS) && (retCode != SQL_SUCCESS_WITH_INFO))
						{
							//----------------------------------------------------------------------------------
							// 输出错误信息 [11/30/2010 13:57 Modify by Liujs]
							ErrorLogInfo(m_pConnection->GetOdbcSqlHEven(),m_pConnection->GetOdbcSqlHDbc(),m_hUpdateSqlStmt);
							//----------------------------------------------------------------------------------
							return FALSE;
						}

					} 
					break;
				case FT_Binary:
					{
						// 二进制大对象 FT_Binary

						// 特别说明：ORACLE 和 SQL SERVER 的二进制操作方法不一样，别的方法都一样		
						// 刘建顺
						// 绑定数据
						retCode = SQLBindParameter(m_hUpdateSqlStmt, m_FieldInfoList[i]->GetColumnIndex(), 
							SQL_PARAM_INPUT,SQL_C_BINARY,SQL_LONGVARBINARY,
							0,0,
							(SQLPOINTER)m_FieldInfoList[i]->m_pValue,
							m_FieldInfoList[i]->GetBinaryValueLen(),(SQLLEN*)&(m_FieldInfoList[i]->m_icpValue));

						// 拷贝数据					
						/*
						The length of the parameter value stored in *ParameterValuePtr. This is ignored except for character or binary C data.
						SQL_NTS. The parameter value is a null-terminated string.
						*/
						m_FieldInfoList[i]->m_icpValue = m_FieldInfoList[i]->GetBinaryValueLen();

						// 判断绑定是否成功
						if ((retCode != SQL_SUCCESS) && (retCode != SQL_SUCCESS_WITH_INFO))
						{
							//----------------------------------------------------------------------------------
							// 输出错误信息 [11/30/2010 13:57 Modify by Liujs]
							ErrorLogInfo(m_pConnection->GetOdbcSqlHEven(),m_pConnection->GetOdbcSqlHDbc(),m_hUpdateSqlStmt);
							//----------------------------------------------------------------------------------
							return FALSE;
						}

					} // end  FT_Binary
					break;

				default:
					break;
				}

			} // end for (DBFieldInfoVector::size_type i=0;i<m_FieldInfoList.size();i++)

		} //  end else if (m_iEditMode == EM_Edit)
		else
		{	
			// 只处理，更新和插入的，别的状态视为错误
			SQLFreeHandle(SQL_HANDLE_STMT,m_hUpdateSqlStmt);
			m_hUpdateSqlStmt = SQL_NULL_HSTMT;

			return FALSE;
		}

	}	//  end if (m_hUpdateSqlStmt != SQL_NULL_HSTMT) 
	else
	{	
		return FALSE;	
	}

	return TRUE;

}

// 执行SQL 插入或者更新操作(INSERT 和 UPDATE)
BOOL		CMySQLRecordSet::ExecuteInsertUpdate()
{
	// 执行操作
	if (m_hUpdateSqlStmt != SQL_NULL_HSTMT)
	{
		// SQL 返回值
		SQLRETURN	retCode;

		// 执行Sql操作
		retCode = SQLExecute(m_hUpdateSqlStmt);
		if((retCode != SQL_SUCCESS) && (retCode != SQL_SUCCESS_WITH_INFO))
		{
			//----------------------------------------------------------------------------------
			// 输出错误信息 [11/30/2010 13:57 Modify by Liujs]
			ErrorLogInfo(m_pConnection->GetOdbcSqlHEven(),m_pConnection->GetOdbcSqlHDbc(),m_hUpdateSqlStmt);
			//----------------------------------------------------------------------------------
			return	FALSE;
		}
	}
	else
	{
		return	FALSE;
	}

	return TRUE;

}
