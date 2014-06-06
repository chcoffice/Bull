#include "CSqlServerRecordSet.h"

using	namespace DBAccessModule;

// 游标索引
INT		CSqlServerRecordSet::m_iCursorIndex = 0;

CSqlServerRecordSet::CSqlServerRecordSet(void)
{
#if 0
	// 创建数据集实例
	m_pRecordset = NULL;
	m_pRecordset.CreateInstance("ADODB.Recordset");
#else
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
#endif
	m_pConnection = NULL;

}

CSqlServerRecordSet::~CSqlServerRecordSet(void)
{
	if (IsOpen())
	{
		Close();
	}
#if 0
	if (m_pRecordset!=NULL)
	{
		m_pRecordset.Release();
		m_pRecordset = NULL;
	}
#else
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

#endif

}



// ---------------------------------------------------------------------------------
// OuterInterface部分 
// ---------------------------------------------------------------------------------
/**************************************************************************************************
Function			:  Eof   
DateTime			: 2010/5/25 11:15	
Description			:    	查询游标是否在在记录集尾
Input				:          	NULL
Output				:           NULL
Return:         	
TRUE				: 记录集尾部
FALSE:不是记录集尾部
Note:				
**************************************************************************************************/
BOOL	CSqlServerRecordSet::Eof()
{
#if 0
	//	ASSERT(m_pRecordset != NULL);

	// 查询游标是否在在记录集尾
	try
	{
		if (m_pRecordset->RecordCount > 0 )
		{
			return m_pRecordset->adoEOF;
		}
		else
		{
			return TRUE;
		}
		
	}
	catch (_com_error e)
	{
		//		TRACE(_T("Warning: IsEOF 方法发生异常. 错误信息: %s; 文件: %s; 行: %d\n"), e.ErrorMessage(), __FILE__, __LINE__);
		return TRUE;
	}
#else
	// linux 

	m_iEditMode = EM_UnKnown;
	return m_bEof;
#endif
	return	TRUE;

}

/**************************************************************************************************
Function: MoveLast    
DateTime: 2010/5/25 11:22	
Description:    	移向最后一条记录
Input:          	NULL
Output:         	NULL
Return:         	
FALSE: 记录集移到最后一条记录，失败
TRUE: 记录集移到最后一条记录，成功
Note:				
**************************************************************************************************/
BOOL	CSqlServerRecordSet::MoveLast()
{
#if 0
	//	ASSERT(m_pRecordset != NULL);

	// 移动游标的位置到数据集的最后一条记录
	try
	{
		if (m_pRecordset != NULL) 
		{
			return SUCCEEDED(m_pRecordset->MoveLast());
		}
	}
	catch (_com_error e)
	{
		//		TRACE(_T("Warning: MoveLast 方法发生异常. 错误信息: %s; 文件: %s; 行: %d\n"), e.ErrorMessage(), __FILE__, __LINE__);
		return FALSE;
	} 
#else
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
		}
	}
	else
	{
		return	FALSE;
	}

#endif

	// 异常返回
	return	FALSE;

}


/**************************************************************************************************
Function:  MoveNext   
DateTime: 2010/5/25 11:30	
Description:      数据集移动向下一条记录
Input:          	NULL
Output:         	NULL
Return:         	
TRUE  : 移动成功
FALSE : 移动失败
Note:				
**************************************************************************************************/
BOOL	CSqlServerRecordSet::MoveNext()
{
#if 0
	// 数据集对象的有效性检查
	//	ASSERT(m_pRecordset != NULL);

	// 将当前记录向前移动一个记录
	try
	{
		if (m_pRecordset != NULL) 
		{
			return SUCCEEDED(m_pRecordset->MoveNext());
		}
	}
	catch (_com_error e)
	{
		//		TRACE(_T("Warning: MoveNext 方法发生异常. 错误信息: %s; 文件: %s; 行: %d\n"), e.ErrorMessage(), __FILE__, __LINE__);
		return FALSE;
	}
#else
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

#endif
	// 异常返回
	return FALSE;

}


/**************************************************************************************************
Function: AddNew() 
DateTime: 2010/5/25 11:07	
Description:    	添加一条空记录
Input:          	NULL
Output:         	NULL
Return:         	
TRUE:成功
FALSE:失败
Note:							
调用AddNew()
调用PutCollect()
调用Update()
**************************************************************************************************/
BOOL    CSqlServerRecordSet::AddNew()
{
#if 0
	//	ASSERT(m_pRecordset != NULL);

	// 添加一条空的记录
	try
	{
		if (m_pRecordset != NULL) 
		{
			if (m_pRecordset->AddNew() == S_OK)
			{
				return TRUE;
			}
		}
	}
	catch (_com_error e)
	{
		// 		strError.Format(_T("Warning: AddNew 方法发生异常. 错误信息: %s; 文件: %s; 行: %d\n"), e.ErrorMessage(), __FILE__, __LINE__);
		return FALSE;
	} 

#else
	// linux

	m_iEditMode = EM_AddNew;

	// 清空自增长列号码标志
	InitCollumnIndexID();
	ClearFieldInfoList();

	return	TRUE;

#endif
	return FALSE;

}





/**************************************************************************************************
Function		: AddNew(IConnection*	pConnection,const	char*	szTableName)
DateTime		: 2010/11/27 7:47	
Description		: 在一个连接上，指定的表上面插入数据
Input			: pConnection：连接对象
szTableName		：表名称
Output			: NULL
Return			: TRUE:成功
Note			:
**************************************************************************************************/
BOOL	CSqlServerRecordSet::AddNew(const	char*	szTableName)
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



/**************************************************************************************************
Function: Update    
DateTime: 2010/5/25 11:05	
Description:    	更新数据集
Input:          	NULL
Output:         	NULL
Return:         	TRUE : 成功
FALSE: 失败
Note:				
调用AddNew()
调用PutCollect()
调用Update()
**************************************************************************************************/
BOOL    CSqlServerRecordSet::Update()
{
#if 0
	//	ASSERT(m_pRecordset != NULL);

	// 执行数据集更新操作
	try
	{
		if (m_pRecordset != NULL) 
		{
			if (m_pRecordset->Update() == S_OK)
			{
				return TRUE;
			}
		}
	}
	catch (_com_error e)
	{	
		// 如果更新失败,则取消更新
		CancelUpdate();
		//TRACE(_T("Warning: Update 方法发生异常. 错误信息: %s; 文件: %s; 行: %d\n"), e.ErrorMessage(), __FILE__, __LINE__);
		return	FALSE;
	}

	// 如果更新失败,则取消更新
	CancelUpdate();
#else
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
#endif
	return	FALSE;

}

/************************************************************************/
/* OuterInterface部分                                                         */
/************************************************************************/

/**************************************************************************************************
Function: GetCollect(const char* szFieldName,string&	szValue)    
DateTime: 2010/5/26 14:53	
Description:    	根据输入字段名称获取字符串值
Input:          	strFiledName : 字段名称
Output:         	szValue:获取的值
Return:         	
TRUE:成功
FALSE:失败
Note:				
根据列名称获取对应字段的值，
返回string 类型字符串(INT64,long long，float,double等都采用string处理)
**************************************************************************************************/
BOOL	CSqlServerRecordSet::GetCollect(const char* szFieldName,string&	szValue)
{
#if 0
	//	ASSERT(m_pRecordset != NULL);
	// 获取指定字段的值
	try
	{
		// 判断数据集是否打开
		if (!IsOpen())
		{
			return FALSE;
		} 
		if (m_pRecordset->adoEOF)
		{
			return FALSE;
		}
		// 获取值信息
		_variant_t value = m_pRecordset->GetCollect(_variant_t(szFieldName));
		// 转换成标准的字符串
		szValue = string((char*)(_bstr_t)value);

		return TRUE;	
	}
	catch (_com_error e)
	{
		//	TRACE(_T("Warning: 字段访问失败. 错误信息: %s; 文件: %s; 行: %d\n"), e.ErrorMessage(), __FILE__, __LINE__);
		return FALSE;
	}
#else
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

#endif
	return FALSE;

}


/**************************************************************************************************
Function: PutCollect(const char* szFieldName,const char*     szValue)    
DateTime: 2010/5/26 15:11	
Description:    	根据列名设置对应列的值
Input:          	
szFieldName:字段名称
szValue:字段值
Output:         	NULL
Return:         	
操作成功：TRUE
操作失败：FALSE
Note:				
类型字符串(INT64,long long，float,double等都采用string处理)
**************************************************************************************************/
BOOL	CSqlServerRecordSet::PutCollect(const char* szFieldName,const char*     szValue)
{
#if 0
	//	ASSERT(m_pRecordset != NULL);
	try
	{
		if (m_pRecordset != NULL) 
		{
			_variant_t vt;
			vt.SetString(szValue);

			// 设置值
			m_pRecordset->put_Collect(_variant_t(szFieldName), vt);
			return TRUE;
		}
	}
	catch (_com_error e)
	{
		return FALSE;
		//TRACE(_T("Warning: PutCollect 方法发生异常. 错误信息: %s; 文件: %s; 行: %d\n"), e.ErrorMessage(), __FILE__, __LINE__);
	} 
#else
	// linux

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

#endif
	return	FALSE;

}



// 释放自身对象
/**************************************************************************************************
Function		: ReleaseRecordSet    
DateTime		: 2010/6/1 18:50	
Description	: 释放数据集对象
Input			: NULL
Output		: NULL
Return		: TRUE:成功，FALSE:失败
Note			: 调用该函数后，自身对象就不存在了，不能在进行调用对象的相关方法了
**************************************************************************************************/
BOOL	CSqlServerRecordSet::ReleaseRecordSet()
{
	delete	this;
	return  TRUE;
}

INT CSqlServerRecordSet::GetColumnNumber(void)
{
	if(m_hSqlStmt == SQL_NULL_HSTMT)
	{
		return	-1;
	}
	return m_ColumnItemList.size();	

}


BOOL CSqlServerRecordSet::GetCloumnName(INT iColIndex, std::string &oStrName )
{
	oStrName.clear();
	if(m_hSqlStmt == SQL_NULL_HSTMT || iColIndex<0 || iColIndex>=(int) m_ColumnItemList.size() )
	{
		return	FALSE;
	}
	oStrName = m_ColumnItemList[iColIndex].strColumn;
	return TRUE;
}

/**************************************************************************************************
Function: GetCollect(const char* szFieldName,INT&	iValue)    
DateTime: 2010/5/26 14:58	
Description:    	根据列名称获取对应字段的值，返回INT 类型字符串
Input:          	szFieldName:字段名称
Output:         	iValue:获取得到的值
Return:         	
TRUE:炒作成功
FALSE:操作失败
Note:				
根据列名称获取对应字段的值，返回INT 类型字符串
**************************************************************************************************/
BOOL	CSqlServerRecordSet::GetCollect(const char* szFieldName,INT&	iValue)
{
#if 0
	//	ASSERT(m_pRecordset != NULL);
	try
	{
		// 获取值
		_variant_t value = m_pRecordset->GetCollect(_variant_t(szFieldName));
		// 将值进行转换
		iValue = atol((char*)(_bstr_t)value);

		return TRUE;
	}
	catch (_com_error e)
	{
		iValue = -1;
		return FALSE;
	} 	
#else
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
		if (iRetFlag  != SQL_SUCCESS_WITH_INFO && iRetFlag != SQL_SUCCESS)
		{
			int i = 0;
			SQLCHAR SqlState[SQL_MAX_MESSAGE_LENGTH];
			SQLCHAR Msg[SQL_MAX_MESSAGE_LENGTH];
			memset(SqlState,0x0,SQL_MAX_MESSAGE_LENGTH);
			memset(Msg,0x0,SQL_MAX_MESSAGE_LENGTH);
			SQLINTEGER NativeError = 0;
			SQLSMALLINT MsgLen;
			SQLRETURN retcode = 0;
			while ((retcode = SQLGetDiagRec(SQL_HANDLE_STMT, m_hSqlStmt, i, SqlState, &NativeError, Msg, sizeof(Msg), &MsgLen) != SQL_NO_DATA_FOUND))
			{
				SqlState;
				NativeError;
				Msg;
				++i;
				memset(SqlState,0x0,SQL_MAX_MESSAGE_LENGTH);
				memset(Msg,0x0,SQL_MAX_MESSAGE_LENGTH);
			}
		}

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

#endif
	return TRUE;

}

/**************************************************************************************************
Function: PutCollect(const char* szFieldName,const INT	iValue)    
DateTime: 2010/5/26 15:08	
Description:    	根据列名设置对应列的值
Input:          	szFieldName:字段名称
iValue:设置的整形值
Output:			NULL
Return:           TRUE:成功
FALSE:失败
Note:				
要求使用者调用的时候，控制，要设置的字段必须是int类型的才可以
**************************************************************************************************/
BOOL	CSqlServerRecordSet::PutCollect(const char* szFieldName,const INT	iValue)
{
#if 0
	//	ASSERT(m_pRecordset != NULL);
	try
	{
		if (m_pRecordset != NULL) 
		{
			// 将整形转换成字符串处理
			_variant_t vt;
			char	szTemp[20] = {0};
			sprintf(szTemp,"%d",iValue);
			vt.SetString(szTemp);
			// 设置值
			m_pRecordset->put_Collect(_variant_t(szFieldName), vt);
			memset(szTemp,0x0,20);
			return TRUE;
		}
	}
	catch (_com_error e)
	{
		return FALSE;
		//TRACE(_T("Warning: PutCollect 方法发生异常. 错误信息: %s; 文件: %s; 行: %d\n"), e.ErrorMessage(), __FILE__, __LINE__);
	} 

#else
	// linux
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
#endif
	return	FALSE;

}

// 根据列名称设置对应时间字段的值
BOOL	CSqlServerRecordSet::PutDtCollect(const char* szFieldName,const INT iYear,const INT iMonth,const INT iDay,const INT iHour,const INT iMinute,const INT iSecond)
{
#if 0

#else

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
		// 设置值,sql server 应该可以直接用字符串设置时间，如果不行就参考oracle的方法
		char	szDt[64] = {0};
		sprintf(szDt,"%d-%02d-%02d %02d:%02d:%02d",iYear,iMonth,iDay,iHour,iMinute,iSecond);
		pcsDbFieldInfoItem->SetStringValue(szDt);

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

#endif
	return	FALSE;
}



// 根据列名称获取对应字段的值，返回viod** 类型字符串
/**************************************************************************************************
Function: GetCollect(const char* szFieldName,void**	   pValue,INT&	iDataLen)   
DateTime: 2010/5/26 14:28	
Description:    	根据列名称获取对应字段的值，返回viod** 类型数据
Input:			strFileName : 字段名称	
iBuffSize:	缓存区的大小
pValue:	外面调用分配好的内存地址
Output:         
pValue:	返回void*类型的数据
iDalaLen:数据长度
Return:         	
TRUE:操作成功
FALSE:操作失败
Note:				用于操作二进制大对象
**************************************************************************************************/
BOOL	CSqlServerRecordSet::GetCollect(const char* szFieldName,void*	   pValue,const INT iBuffSize,INT&	iDataLen)
{
#if 0

	// 获取字段集指针
	FieldPtr		pField = GetField(szFieldName);
	if (pField == NULL)
	{
		return	FALSE;
	}

	// 数据长度大小
	long lDataSize = pField->ActualSize;

	// 定义数据类型
	//	UCHAR chData;
	long index = 0;

	// 有数据存在，并且外面分配的内存空间大于数据内存空间
	if (index < lDataSize && iBuffSize > iDataLen)
	{ 
		try
		{
			// 一次全部取出来
			_variant_t varChunk = pField->GetChunk(lDataSize);

			// 判断数据类型 :判断类型是否为BYTE类型的数组 
			if (varChunk.vt != (VT_ARRAY | VT_UI1))
			{
				return FALSE;
			}

			char *pBuf = NULL;
			// 获取数据
			HRESULT	hr2 = SafeArrayAccessData(varChunk.parray,(void **)&pBuf);
			if (SUCCEEDED(hr2))
			{
				// 拷贝数据
				memcpy(pValue,pBuf,lDataSize);				
				// 是否内存
				SafeArrayUnaccessData (varChunk.parray);
			}
			else
			{
				FALSE;
			}

			// 返回数据长度
			iDataLen = lDataSize;

			// 			for (long i = 0; i < lDataSize; i++)
			// 			{
			// 				HRESULT hr = SafeArrayGetElement(varChunk.parray, &i, &chData);
			// 				if (SUCCEEDED(hr))
			// 				{
			// 					((UCHAR*)pValue)[index] = chData;
			// 					index++;
			// 				}
			// 				else
			// 				{
			// 					break;
			// 				}
			// 			}
		}
		catch (_com_error e)
		{			
			return FALSE;
		}
	}
	else
	{
		// 返回长度
		iDataLen = lDataSize;
		// 长度不够了
		return FALSE;
	}
#else
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
#endif
	return TRUE;

}

/**************************************************************************************************
Function: PutCollect(const char* szFieldName,const  void*	   pValue,const INT	iDataLen)    
DateTime: 2010/5/26 14:14	
Description:    	根据字段名称设置相应的值
Input:          	
strFileName:
pValue:
iDataLen:
Output:         	NULL
Return:         
TRUE: 成功
FALSE:失败
Note:				用于操作二进制大对象
**************************************************************************************************/
BOOL	CSqlServerRecordSet::PutCollect(const char* szFieldName,const  void*	   pValue,const INT	iDataLen)
{
#if 0
	// 获取字段集指针
	FieldPtr		pField = GetField(szFieldName);
	if (pField == NULL)
	{
		return	FALSE;
	}

	// 调用 SAFEARRAY 和  SAFEARRAYBOUND 来进行操作
	SAFEARRAY FAR *pSafeArray = NULL;
	SAFEARRAYBOUND rgsabound[1];

	try
	{
		rgsabound[0].lLbound = 0;
		rgsabound[0].cElements = iDataLen;
		pSafeArray = SafeArrayCreate(VT_UI1, 1, rgsabound);

		// 数据转换
		char*		pBuff	=	(char*)pValue;

		for (long i = 0; i < (long)iDataLen; i++)
		{
			// 写入数据
			HRESULT hr = SafeArrayPutElement(pSafeArray, &i, pBuff++);
			if (FAILED(hr))	
			{
				return FALSE;
			}
		}

		_variant_t varChunk;
		///将varChunk的类型设置为BYTE类型的数组 
		varChunk.vt = VT_ARRAY | VT_UI1;
		varChunk.parray = pSafeArray;

		return (pField->AppendChunk(varChunk) == S_OK);
	}
	catch (_com_error e)
	{
		//TRACE(_T("Warning: AppendChunk 方法发生异常. 错误信息: %s; 文件: %s; 行: %d\n"), e.ErrorMessage(), __FILE__, __LINE__);
		return FALSE;
	}
#else
	// linux

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

#endif
	return TRUE;

}


// ---------------------------------------------------------------------------------
// 内部接口部分  
// ---------------------------------------------------------------------------------
/**************************************************************************************************
Function: QuerySql    
DateTime: 2010/5/25 19:11	
Description:    	执行Sql语句返回数据集
Input:          	标准Sql语句
Output:         	NULL
Return:         	
IRecordSet* 返回数据集对象
Note:				执行SELECT查询操作
**************************************************************************************************/
IRecordSet*		CSqlServerRecordSet::QuerySql(const char*	szSql)
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

// ---------------------------------------------------------------------------------
// 内部方法
// ---------------------------------------------------------------------------------
/**************************************************************************************************
Function:     
DateTime: 2010/5/25 10:58	
Description:    	获取数据集判断是否可以进行编辑
Input:          	NULL
Output:         	NULL
Return:         
TRUE:	可以编辑
FALSE:  不可以编辑

Note:				NULL
**************************************************************************************************/
BOOL		CSqlServerRecordSet::GetEditMode()
{
#if 0
	//	ASSERT(m_pRecordset != NULL);

	// 获取记录集的编辑模式
	try
	{
		if (m_pRecordset != NULL) 
		{
			return m_pRecordset->GetEditMode() == adEditNone ? TRUE : FALSE;
		}
	}
	catch (_com_error e)
	{
		//	TRACE(_T("Warning: UpdateBatch 方法发生异常. 错误信息: %s; 文件: %s; 行: %d\n"), e.ErrorMessage(), __FILE__, __LINE__);
		return FALSE;
	} 
#else
	//linux
#endif
	return	FALSE; 

}


/**************************************************************************************************
Function: SetConnection    
DateTime: 2010/5/25 10:21	
Description:    	设置连接对象
Input:          	pConnection : 连接对象
Output:         	NULL
Return:         	// 函数返回值的说明
Note:				// 备注
**************************************************************************************************/
BOOL			CSqlServerRecordSet::SetConnection(CSqlServerConnection* pConnection)
{
	if (pConnection != NULL)
	{
		m_pConnection	=	pConnection;
		return	TRUE;
	}
	return FALSE;
}

/**************************************************************************************************
Function: CancelUpdate    
DateTime: 2010/5/25 10:57	
Description:		取消在调用 Update 方法前对当前记录或新记录所作的任何更改.
Input:          	NULL
Output:         	NULL
Return:         	成功：TRUE 失败：FALSE
Note:				
**************************************************************************************************/
BOOL	CSqlServerRecordSet::CancelUpdate()
{
#if 0
	//	ASSERT(m_pRecordset != NULL);

	// 取消存盘
	try
	{
		if (m_pRecordset != NULL) 
		{
			if (GetEditMode() || m_pRecordset->CancelUpdate() == S_OK)
			{
				return TRUE;
			}
		}
	}
	catch (_com_error e)
	{
		//	TRACE(_T("Warning: CancelUpdate 方法发生异常. 错误信息: %s; 文件: %s; 行: %d\n"), e.ErrorMessage(), __FILE__, __LINE__);
		return FALSE;
	} 
#else
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
#endif
	// 异常返回
	return FALSE;

}


/**************************************************************************************************
Function: IsOpen    
DateTime: 2010/5/25 17:23	
Description:    	判断数据集是否打开
Input:          	NULL
Output:         	NULL
Return:         	
TRUE:打开
FALSE:关闭
Note:				// 备注
**************************************************************************************************/
BOOL	CSqlServerRecordSet::IsOpen()
{
#if 0
	try
	{
		if (m_pRecordset != NULL)
		{
			// 打开的情况
			if ( m_pRecordset->GetState() & adStateOpen)
			{
				return	TRUE;
			}			
		}
	}
	catch (_com_error e)
	{
		//	TRACE(_T("Warning: IsOpen方法发生异常. 错误信息: %s; 文件: %s; 行: %d\n"), e.ErrorMessage(), __FILE__, __LINE__);
		return FALSE;
	} 
#else
	// linux
	return	m_bIsOpen;
#endif
	return FALSE;

}

/**************************************************************************************************
Function: Close    
DateTime: 2010/5/25 17:33	
Description:   关闭数据集
Input:         NULL
Output:        NULL
Return:        
TRUE: 关闭成功
FALSE: 关闭失败
Note:		
**************************************************************************************************/
BOOL	CSqlServerRecordSet::Close()
{
#if 0
	// 关闭处于打开状态的数据集对象
	try
	{
		if (m_pRecordset != NULL && m_pRecordset->State != adStateClosed)
		{
			if (GetEditMode() == adEditNone)
			{
				CancelUpdate();
			}

			m_pRecordset->Close();
		}
	}
	catch (_com_error e)
	{
		// TRACE(_T("Warning: Close 方法发生异常. 错误信息: %s; 文件: %s; 行: %d\n"), e.ErrorMessage(), __FILE__, __LINE__);
		return	FALSE;
	}
#else 
	//linux
	if (IsOpen())
	{
		m_bIsOpen = FALSE;
		SQLFreeHandle(SQL_HANDLE_STMT,m_hSqlStmt);
		m_hSqlStmt = SQL_NULL_HSTMT;
	}
#endif
	return	TRUE;

}

/**************************************************************************************************
Function: Open    
DateTime: 2010/5/25 17:34	
Description:   打开数据集
Input:         标准Sql语句
Output:        NULL
Return:        
TRUE: 打开成功
FALSE:打开失败
Note:			
**************************************************************************************************/
BOOL	CSqlServerRecordSet::Open(const char*  szSql)
{
#if 0
	// 连接对象和数据集对象的有效性检查
	if (m_pConnection == NULL || m_pRecordset == NULL)
	{
		return FALSE;
	}

	// 打开数据集对象
	try
	{
		// 关闭数据集对象
		if (IsOpen()) 
		{
			Close();
		}

		// 清空过滤器
		m_pRecordset->PutFilter("");

		// 重新打开数据集对象 , 参数说明，参考ADO访问数据库
		m_pRecordset->CursorLocation	=	adUseClient;
		HRESULT hr = m_pRecordset->Open(_variant_t(szSql),_variant_t((IDispatch*)m_pConnection->GetConnection(), true),adOpenStatic, adLockOptimistic, adCmdText);

		// 打开数据集失败
		if(!SUCCEEDED(hr))   
		{
			return FALSE;
		}

		// 移动向第一条
		if (m_pRecordset->RecordCount > 0)
		{
			m_pRecordset->MoveFirst();
		}
	}
	catch (_com_error e)
	{
		/*
		TRACE(_T("Warning: 打开记录集发生异常. 错误信息: %s; 文件: %s; 行: %d\n"), e.ErrorMessage(), __FILE__, __LINE__);
		TRACE(_T("%s\r\n"), GetLastError());
		*/
		// 如果断开连接了，要进行重新连接
		m_pConnection->GetConnectionErrorAndDeal();
		return FALSE;
	}
#else
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
			// 设置为静态游标
			//iRetCode = SQLSetStmtOption(m_hSqlStmt, SQL_CURSOR_TYPE, SQL_CURSOR_STATIC);
			iRetCode	=	SQLExecDirect(m_hSqlStmt, (SQLCHAR*)szSql, SQL_NTS);
			if ( iRetCode == SQL_ERROR)
			{
				//----------------------------------------------------------------------------------
				// 输出错误信息 [11/30/2010 13:57 Modify by Liujs]
				ErrorLogInfo(m_pConnection->GetOdbcSqlHEven(),m_pConnection->GetOdbcSqlHDbc(),m_hUpdateSqlStmt);
				//----------------------------------------------------------------------------------

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
#endif
	return TRUE;

}



//------------------------------------------------------------------------------------------------------
// 字段集的获取
#if 0

/**************************************************************************************************
Function: GetFields   
DateTime: 2010/5/26 13:57	
Description:    	字段集的获取
Input:          	NULL
Output:         	NULL
Return:         	FieldsPtr : 类型的字段集
Note:				
**************************************************************************************************/
FieldsPtr CSqlServerRecordSet::GetFields()
{
	//	ASSERT(m_pRecordset != NULL);
	try
	{
		return m_pRecordset->GetFields();
	}
	catch (_com_error e)
	{
		//TRACE(_T("Warning: GetFields 方法发生异常. 错误信息: %s; 文件: %s; 行: %d\n"), e.ErrorMessage(), __FILE__, __LINE__);
		return NULL;
	} 
	return NULL;

}


/**************************************************************************************************
Function:GetField     
DateTime: 2010/5/26 13:57	
Description:  字段集的获取
Input:        字段名称
Output:       NULL
Return:       FieldsPtr : 类型的字段集
Note:		
**************************************************************************************************/
FieldPtr  CSqlServerRecordSet::GetField(const char* szFieldName)
{
	try
	{
		return GetFields()->GetItem(_variant_t(szFieldName));
	}
	catch (_com_error e)
	{
		//	TRACE(_T("Warning: GetField发生异常. 错误信息: %s; 文件: %s; 行: %d\n"), e.ErrorMessage(), __FILE__, __LINE__);
		return NULL;
	}

}
#else
// linux

// --------------------------------------------------------------------------------------------
// 初始化相关信息
/**************************************************************************************************
Function		: Init()    
DateTime		: 2010/6/21 20:14	
Description		: 初始化STMT句柄相关信息
Input			: NULL
Output			: NULL
Return			: TRUE :成功  FALSE : 失败
Note			：OPEN函数中调用
**************************************************************************************************/
BOOL		CSqlServerRecordSet::Init()
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
				ErrorLogInfo(m_pConnection->GetOdbcSqlHEven(),m_pConnection->GetOdbcSqlHDbc(),m_hUpdateSqlStmt);
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
				ErrorLogInfo(m_pConnection->GetOdbcSqlHEven(),m_pConnection->GetOdbcSqlHDbc(),m_hUpdateSqlStmt);
				//----------------------------------------------------------------------------------

				Close(); 
				return FALSE;
			}

			// 设置属性

			/*			// 调用返回-1不知道为啥
			retCode = SQLSetStmtAttr(m_hSqlStmt, SQL_ATTR_QUERY_TIMEOUT, (SQLPOINTER)coniOpenTimeOut, SQL_IS_UINTEGER);
			if ( (retCode != SQL_SUCCESS) && (retCode != SQL_SUCCESS_WITH_INFO)) 
			{
			Close(); 
			return FALSE;
			}
			*/
			// 设置游标名称
			char		szCursorName[MAX_CURSOR_NAME_LEN] = {0x0};
			sprintf(szCursorName,"%s_%d",CURRENT_CURSOR_NAME,GetCursorIndex());
			retCode = SQLSetCursorName(m_hSqlStmt,reinterpret_cast<SQLCHAR*>(szCursorName), SQL_NTS);
			if ( (retCode != SQL_SUCCESS) && (retCode != SQL_SUCCESS_WITH_INFO)) 
			{
				//----------------------------------------------------------------------------------
				// 输出错误信息 [11/30/2010 13:57 Modify by Liujs]
				ErrorLogInfo(m_pConnection->GetOdbcSqlHEven(),m_pConnection->GetOdbcSqlHDbc(),m_hUpdateSqlStmt);
				//----------------------------------------------------------------------------------

				Close(); 
				return FALSE;
			}
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

//---------------------------------------------------------------------------------------------
// 获取值相关辅助函数

/**************************************************************************************************
Function		: GetColumnList()    
DateTime		: 2010/6/19 14:52	
Description		: 获取字段名称列表
Input			: NULL
Output			: NULL
Return			: 成功返回TRUE,失败返回FALSE
Note			: NULL
**************************************************************************************************/
BOOL		CSqlServerRecordSet::GetColumnList()
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

/**************************************************************************************************
Function		: ClearColumnList()    
DateTime		: 2010/6/19 14:51	
Description		: 清空字段列表
Input			: NULL
Output			: NULL
Return			: TRUE
Note			: NULL
**************************************************************************************************/
BOOL		CSqlServerRecordSet::ClearColumnList()
{
	m_ColumnItemList.clear();
	return	TRUE;

}


/**************************************************************************************************
Function		: GetColumnIndexByName(const char* szFieldName)   
DateTime		: 2010/6/19 11:03	
Description		: 根据字段名称，获取列的索引
Input			: 字段名称
Output			: NULL
Return			: 列的索引
Note			: 获取失败返回 ERROR_SQL_COLUMN_INDEX
**************************************************************************************************/
INT		  CSqlServerRecordSet::GetColumnIndexByName(const char* szFieldName)
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

//---------------------------------------------------------------------------------------------
// 设置值相关辅助函数

/**************************************************************************************************
Function		: ClearFieldInfoList()    
DateTime		: 2010/6/20 18:20	
Description		: 清空数据字段列表（m_FieldInfoList ： AddNew 和 Update的时候用的）
Input			: NULL
Output			: NULL
Return			: TRUE
Note			:（m_FieldInfoList ： AddNew 和 Update的时候用的）	
**************************************************************************************************/
BOOL		CSqlServerRecordSet::ClearFieldInfoList()
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
BOOL		CSqlServerRecordSet::AddFieldItem2FieldInfoList(CDBFieldInfo*	pcsDbFieldInfoItem)
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

/**************************************************************************************************
Function		: GetTableNameFromSqlText(const char* szSql)    
DateTime		: 2010/6/20 9:54	
Description		: 根据查询数据的sql语句来获取其中的表名称(主要是对表查询后，进行插入数据和更新数据的时候用)
Input			: Sql 查询语句，例如：SELECT * from TB_TEST;
Output			: strTableName:操作的表名称
Return			: TRUE:成功，FALSE:失败
Note			: 打开数据集的地方要使用，将数据集的对应的表名称存入起来，用于更新和插入时候用
**************************************************************************************************/
BOOL		CSqlServerRecordSet::GetTableNameFromSqlText(const char* szSql,string& strTableName)
{
	try
	{
		string	strSql  = string(szSql);
		// 查找里面第一个from后面的字符串

		// 转化为大写
		// transform (strSql.begin(),strSql.end(), strSql.begin(), toupper); 
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


/**************************************************************************************************
Function		: BuildInsertUpdateSqlTxt()    
DateTime		: 2010/6/20 21:04	
Description		: 构造Sql语句进行操作
Input			: NULL
Output			: NULL
Return			: TRUE: 成功 ，FALSE :失败
Note			: 
**************************************************************************************************/
BOOL		CSqlServerRecordSet::BuildInsertUpdateSqlTxt()
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
				//----------------------------------------------------------------------------------
				// 输出错误信息 [11/30/2010 13:57 Modify by Liujs]
				ErrorLogInfo(m_pConnection->GetOdbcSqlHEven(),m_pConnection->GetOdbcSqlHDbc(),m_hUpdateSqlStmt);
				//----------------------------------------------------------------------------------

				return TRUE;
			}
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
				return FALSE;
			}

			// 例如： UPDATE TB_TEST SET NAME = ? ,PASS= ? WHERE KK = 'E'

			// 准备Sql 语句
			SQLRETURN	retCode = 0;
			retcode = SQLPrepare(m_hUpdateSqlStmt,(SQLCHAR*)m_strSqlText.c_str(),SQL_NTS);
			if (retcode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{
				//----------------------------------------------------------------------------------
				// 输出错误信息 [11/30/2010 13:57 Modify by Liujs]
				ErrorLogInfo(m_pConnection->GetOdbcSqlHEven(),m_pConnection->GetOdbcSqlHDbc(),m_hUpdateSqlStmt);
				//----------------------------------------------------------------------------------

				return TRUE;
			}
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


/**************************************************************************************************
Function		: AppendValueToSqlTxt()    
DateTime		: 2010/6/20 23:09	
Description		: 向Sql语句里面添加对应的值
Input			: NULL
Output			: NULL
Return			: TRUE:成功，FALSE:失败
Note			: 前件条件：BuildInsertUpdateSqlTxt()执行成功
：后件条件：ExecuteInsertUpdate()的执行
**************************************************************************************************/
BOOL		CSqlServerRecordSet::AppendValueToSqlTxt()
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
				case FT_Binary:
					{
						// 二进制大对象 FT_Binary
						// 特别说明：ORACLE 和 SQL SERVER 的二进制操作方法不一样，别的方法都一样		
						// 刘建顺

						// 绑定数据
						retCode = SQLBindParameter(m_hUpdateSqlStmt,  m_FieldInfoList[i]->GetColumnIndex(),
							SQL_PARAM_INPUT, SQL_C_BINARY, SQL_LONGVARBINARY,
							m_FieldInfoList[i]->GetBinaryValueLen(), 0, 
							(SQLPOINTER) m_FieldInfoList[i]->m_pValue, 
							0, (SQLLEN*)&(m_FieldInfoList[i]->m_icpValue));

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
				case FT_DateTime:
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

					} // end if (m_FieldInfoList[i]->GetFieldType() == FT_Integer)
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

					} // else if (m_FieldInfoList[i]->GetFieldType() == FT_String)
					break;
				case FT_Binary:
					{
						// 二进制大对象 FT_Binary

						// 特别说明：ORACLE 和 SQL SERVER 的二进制操作方法不一样，别的方法都一样		
						// 刘建顺
						// 绑定数据
						retCode = SQLBindParameter(m_hUpdateSqlStmt,  m_FieldInfoList[i]->GetColumnIndex(),
							SQL_PARAM_INPUT, SQL_C_BINARY, SQL_LONGVARBINARY,
							m_FieldInfoList[i]->GetBinaryValueLen(), 0, 
							(SQLPOINTER) m_FieldInfoList[i]->m_pValue, 
							0, (SQLLEN*)&(m_FieldInfoList[i]->m_icpValue));

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
				case FT_DateTime:
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

/**************************************************************************************************
Function		: ExecuteInsertUpdate()    
DateTime		: 2010/6/21 0:19	
Description		: 执行SQL 插入或者更新操作(INSERT 和 UPDATE)
Input			: NULL
Output			: NULL
Return			: TRUE:成功，FALSE:失败
Note			: 前件条件：BuildInsertUpdateSqlTxt()执行成功
AppendValueToSqlTxt()执行成功	
**************************************************************************************************/
BOOL		CSqlServerRecordSet::ExecuteInsertUpdate()
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

#endif



