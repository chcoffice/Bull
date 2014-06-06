/**************************************************************************************************
* Copyrights 2013  高新兴
*                  基础应用组
* All rights reserved.
*
* Filename：
*       COciRecordSet.cpp
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


#include "COciRecordSet.h"
#include "COciConnection.h"
#include "CDBConnectionPool.h"

using	namespace	DBAccessModule;

/**************************************************************************************************
*@input             Nothing
*@output            Nothing
*@return
*@description       构造函数
*@frequency of call 创建OCI数据集类
**************************************************************************************************/
COciRecordSet::COciRecordSet(void)
{

	m_bIsOpen = FALSE;

	// 编辑模式
	m_iEditMode = EM_UnKnown;

	// 表名称
	m_strTableName = constrDefaultString ;


	// 是否是记录集尾部
	m_bEof = TRUE;

	m_pConnection = NULL;
}

/**************************************************************************************************
*@input             Nothing
*@output            Nothing
*@return
*@description       析构函数
*@frequency of call Nothing
**************************************************************************************************/
COciRecordSet::~COciRecordSet(void)
{
	if (IsOpen())
	{
		Close();
	}

	if (m_bIsOpen)
	{
		m_bIsOpen = FALSE;

		if (this->m_pResultset)
			m_pOciDeal->pOCI_ReleaseResultsets(this->m_pStmtset);

		if (this->m_pStmtset)
			m_pOciDeal->pOCI_StatementFree(this->m_pStmtset);

	}
	m_iEditMode = EM_UnKnown;
	m_bEof = TRUE;
}

/**************************************************************************************************
  Function:    harder_error    
  DateTime:    2013/5	
  Description: 出错处理
  Input:       NULL
  Output:      NULL
  Return:      
			   成功：TRUE
			   失败：FALSE
  Note:		   
**************************************************************************************************/
void COciRecordSet::harder_error(OCI_Error* err)
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
  Function:			IsOpen    
  DateTime:			2013/05
  Description:    	判断数据集是否打开
  Input:          	NULL
  Output:         	NULL
  Return:         	
					TRUE:打开
					FALSE:关闭
  Note:				// 备注
**************************************************************************************************/
BOOL	COciRecordSet::IsOpen()
{
	return	m_bIsOpen;
}

/**************************************************************************************************
  Function:		 Close    
  DateTime:		 2013/05	
  Description:   关闭数据集
  Input:         NULL
  Output:        NULL
  Return:        
				 TRUE: 关闭成功
				 FALSE: 关闭失败
  Note:		
**************************************************************************************************/
BOOL	COciRecordSet::Close()
{

	if (IsOpen())
	{
		m_bIsOpen = FALSE;
		if (this->m_pResultset)
			m_pOciDeal->pOCI_ReleaseResultsets(this->m_pStmtset);

		if (this->m_pStmtset)
			m_pOciDeal->pOCI_StatementFree(this->m_pStmtset);

	}
	return	TRUE;

}

/**************************************************************************************************
  Function:		 Open    
  DateTime:		 2013/05	
  Description:   打开数据集
  Input:         标准Sql语句
  Output:        NULL
  Return:        
				 TRUE: 打开成功
				 FALSE:打开失败
  Note:			
**************************************************************************************************/
BOOL	COciRecordSet::Open(const char*  szSql)
{
	try
	{
		// 关闭数据集对象
		if (IsOpen()) 
		{
			Close();
		}
		//打开数据集
		if (Init())
		{
			if(!m_pOciDeal->pOCI_ExecuteStmt(m_pStmtset, szSql))
			{
				harder_error(m_pOciDeal->pOCI_GetLastError());
				return FALSE;
		
			}
			m_pResultset = m_pOciDeal->pOCI_GetResultset(m_pStmtset);
			if(m_pResultset == NULL)
			{
				
				harder_error(m_pOciDeal->pOCI_GetLastError());
				return FALSE;
			}

			m_strTableName = constrDefaultString ;
			// 已经执行成功，打开数据集了
			m_bIsOpen = TRUE;
			// 获取列名相关信息存入列表
			//GetColumnList();
			
		//	if(!OCI_FetchFirst(m_pResultset)) 
			if(!m_pOciDeal->pOCI_FetchNext(m_pResultset))
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

/**************************************************************************************************
  Function		: Init()    
  DateTime		: 2013/05	
  Description	: 初始化STMT句柄相关信息
  Input			: NULL
  Output		: NULL
  Return		: TRUE :成功  FALSE : 失败
  Note			：OPEN函数中调用
**************************************************************************************************/
BOOL		COciRecordSet::Init()
{

	if ( !m_pConnection->IsOpen() ) 
		if ( !m_pConnection->Open() )
			return FALSE;
/*
	if (this->m_pStmtset)
		m_pOciDeal->pOCI_StatementFree(this->m_pStmtset);
*/
	this->m_pStmtset = m_pOciDeal->pOCI_StatementCreate(m_pConnection->GetOciCon());

	if (!this->m_pStmtset) 
	{
		harder_error(m_pOciDeal->pOCI_GetLastError());
		return FALSE;
	}

	return TRUE;
}

/**************************************************************************************************
  Function:			QuerySql    
  DateTime:			2013/05	
  Description:    	执行Sql语句返回数据集
  Input:          	标准Sql语句
  Output:         	NULL
  Return:         	
					IRecordSet* 返回数据集对象
  Note:				执行SELECT查询操作
**************************************************************************************************/
IRecordSet*		COciRecordSet::QuerySql(const char*	szSql)
{	

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

/**************************************************************************************************
Function:			SetConnection    
DateTime:			2013/05	
Description:    	设置连接对象
Input:          	pConnection : 连接对象
Output:         	NULL
Return:         	// 函数返回值的说明
Note:				// 备注
**************************************************************************************************/
BOOL  COciRecordSet::SetConnection(COciConnection* pConnection)
{
	if (pConnection != NULL)
	{
		m_pConnection = pConnection;
		return	TRUE;
	}
	return FALSE;

}
/**************************************************************************************************
  Function		: ReleaseRecordSet    
  DateTime		: 2013/5	
  Description	: 释放数据集对象
  Input			: NULL
  Output		: NULL
  Return		: TRUE:成功，FALSE:失败
  Note			: 调用该函数后，自身对象就不存在了，不能在进行调用对象的相关方法了
**************************************************************************************************/
BOOL	COciRecordSet::ReleaseRecordSet()
{
	delete	this;
	return  TRUE;
}

/**************************************************************************************************
  Function:			Eof   
  DateTime:			2013/05	
  Description:    	查询游标是否在在记录集尾
  Input:          	NULL
  Output:           NULL
  Return:         	
					TRUE: 记录集尾部
					FALSE:不是记录集尾部
  Note:				
**************************************************************************************************/
BOOL	COciRecordSet::Eof()
{
	m_iEditMode = EM_UnKnown;
	return m_bEof;

}

/**************************************************************************************************
  Function:			MoveLast    
  DateTime:			2013/05
  Description:    	移向最后一条记录
  Input:          	NULL
  Output:         	NULL
  Return:         	
					FALSE: 记录集移到最后一条记录，失败
					TRUE: 记录集移到最后一条记录，成功
  Note:				
**************************************************************************************************/
BOOL	COciRecordSet::MoveLast()
{
	if (!m_pOciDeal->pOCI_FetchLast (this->m_pResultset)) 
	{
		this->harder_error(m_pOciDeal->pOCI_GetLastError());
		
		m_bEof = TRUE;

		return FALSE;
	}
	else
	{
		InitCollumnIndexID();
		m_bEof = FALSE;
	}
	return TRUE;

}


/**************************************************************************************************
  Function:			MoveNext   
  DateTime:			2013/05
  Description:      数据集移动向下一条记录
  Input:          	NULL
  Output:         	NULL
  Return:         	
					TRUE  : 移动成功
					FALSE : 移动失败
  Note:				
**************************************************************************************************/
BOOL	COciRecordSet::MoveNext()
{
	if (!m_pOciDeal->pOCI_FetchNext(this->m_pResultset)) 
	{
		this->harder_error(m_pOciDeal->pOCI_GetLastError());
		m_bEof = TRUE;
		return FALSE;
	}
	else
	{
		// 清空自增长列号码标志
		InitCollumnIndexID();
		m_bEof = FALSE;
		return	TRUE;
	}
	return TRUE;
}


/**************************************************************************************************
  Function:			AddNew() 
  DateTime:			2013/05
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
BOOL    COciRecordSet::AddNew()
{
	m_iEditMode = EM_AddNew;
	// 清空自增长列号码标志
	InitCollumnIndexID();
	ClearFieldInfoList();
	return	TRUE;


}


/**************************************************************************************************
Function		: AddNew(IConnection*	pConnection,const	char*	szTableName)
DateTime		: 2013/5
Description		: 在一个连接上，指定的表上面插入数据
Input			: pConnection：连接对象
szTableName		：表名称
Output			: NULL
Return			: TRUE:成功
Note			:
**************************************************************************************************/
BOOL	COciRecordSet::AddNew(const	char*	szTableName)
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
  Function:			Update    
  DateTime:			2013/05	
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
BOOL    COciRecordSet::Update()
{

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
	return TRUE;

}


INT COciRecordSet::GetColumnNumber(void)
{

	return m_pOciDeal->pOCI_GetColumnCount(m_pResultset); 	

}


BOOL COciRecordSet::GetCloumnName(INT iColIndex, std::string &oStrName )
{

	int n;
	n  = m_pOciDeal->pOCI_GetColumnCount(m_pResultset);  
	if(iColIndex > n)
		return FALSE;
  
    OCI_Column *col = m_pOciDeal->pOCI_GetColumn(m_pResultset, iColIndex);  
	oStrName = m_pOciDeal->pOCI_ColumnGetName(col);

	return TRUE;
}
/**************************************************************************************************
  Function:			GetCollect(const char* szFieldName,string&	szValue)    
  DateTime:			2013/5
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
BOOL	COciRecordSet::GetCollect(const char* szFieldName,string&	szValue)
{

	if (m_pOciDeal->pOCI_IsNull2(this->m_pResultset, szFieldName))
		return FALSE;

	szValue = m_pOciDeal->pOCI_GetString2(this->m_pResultset, szFieldName);

	return TRUE;

}

/**************************************************************************************************
  Function:     
  DateTime:			2013/05	
  Description:    	获取数据集判断是否可以进行编辑
  Input:          	NULL
  Output:         	NULL
  Return:         
					TRUE:	可以编辑
					FALSE:  不可以编辑

  Note:				NULL
**************************************************************************************************/
BOOL		COciRecordSet::GetEditMode()
{
	return	FALSE; 
}

/**************************************************************************************************
  Function:			PutCollect(const char* szFieldName,const char*     szValue)    
  DateTime:			2013/5
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
BOOL	COciRecordSet::PutCollect(const char* szFieldName,const char*     szValue)
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
		return FALSE;

}


/**************************************************************************************************
  Function		: PutDtCollect(const char* szFieldName,const char* szValue)
  DateTime		: 2013/5	
  Description	: 根据列名称设置对应时间字段的值
  Input			: szFieldName：字段名称，szValue：时间值，字符串
  Output		: NULL
  Return		: NULL
  Note			:
**************************************************************************************************/
BOOL	COciRecordSet::PutDtCollect(const char* szFieldName,const INT iYear,const INT iMonth,const INT iDay,const INT iHour,const INT iMinute,const INT iSecond)
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
		return FALSE;
}



/**************************************************************************************************
  Function:			GetCollect(const char* szFieldName,INT&	iValue)    
  DateTime:			2013/5
  Description:    	根据列名称获取对应字段的值，返回INT 类型字符串
  Input:          	szFieldName:字段名称
  Output:         	iValue:获取得到的值
  Return:         	
					TRUE:炒作成功
					FALSE:操作失败
  Note:				
					根据列名称获取对应字段的值，返回INT 类型字符串
**************************************************************************************************/
BOOL	COciRecordSet::GetCollect(const char* szFieldName,INT&	iValue)
{

	if (m_pOciDeal->pOCI_IsNull2(this->m_pResultset, szFieldName))
		return FALSE;

	iValue = m_pOciDeal->pOCI_GetUnsignedInt2(this->m_pResultset, szFieldName);

	return TRUE;

}

/**************************************************************************************************
  Function:			PutCollect(const char* szFieldName,const INT	iValue)    
  DateTime:			2013/5
  Description:    	根据列名设置对应列的值
  Input:          	szFieldName:字段名称
		         	iValue:设置的整形值
  Output:			NULL
  Return:           TRUE:成功
					FALSE:失败
  Note:				
					要求使用者调用的时候，控制，要设置的字段必须是int类型的才可以
**************************************************************************************************/
BOOL	COciRecordSet::PutCollect(const char* szFieldName,const INT	iValue)
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
		return	FALSE;

}

// 根据列名称获取对应字段的值，返回viod** 类型字符串
/**************************************************************************************************
  Function:			GetCollect(const char* szFieldName,void**	   pValue,INT&	iDataLen)   
  DateTime:			2013/5
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
BOOL	COciRecordSet::GetCollect(const char* szFieldName,void*	   pValue,const INT iBuffSize,INT&	iDataLen)
{
	SQLCHAR*		pszData = NULL;
	try
	{


		// 当前字段的字节长度
		UINT		iActualDataSize = 0;
/*
		// 获取列索引
		SQLSMALLINT		iColumnIndex = GetColumnIndexByName(szFieldName);
		if(iColumnIndex == ERROR_SQL_COLUMN_INDEX)
		{
			return FALSE;
		}
*/
		// 分配内存
		pszData = new SQLCHAR[iBuffSize + 1];
		memset(pszData,0x0,iBuffSize+1);

		if (m_pOciDeal->pOCI_IsNull2(this->m_pResultset, szFieldName))
		{
			if (pszData != NULL)
			{
					delete[] pszData;
					pszData = NULL;
			}

			return FALSE;
		}

		OCI_Lob* lob = m_pOciDeal->pOCI_GetLob2(this->m_pResultset, szFieldName);
		iActualDataSize = m_pOciDeal->pOCI_LobGetLength(lob);

		if (iActualDataSize <= 0)
		{
			if (pszData != NULL)
			{
					delete[] pszData;
					pszData = NULL;
			}

			return FALSE;
		}
		m_pOciDeal->pOCI_LobRead(lob, pszData, iActualDataSize);
		// 返回数据，并且返回大小
		memcpy(pValue,pszData,iActualDataSize);
		iDataLen = iActualDataSize;
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
BOOL COciRecordSet::GetBlobCollect(const char* szFieldName,string& strValue)
{
	SQLCHAR*		pszData = NULL;
	try
	{
		// 当前字段的字节长度
		UINT		iActualDataSize = 0;

		// 分配内存
		if (m_pOciDeal->pOCI_IsNull2(this->m_pResultset, szFieldName))
		{		
			return FALSE;
		}

		OCI_Lob* lob = m_pOciDeal->pOCI_GetLob2(this->m_pResultset, szFieldName);
		iActualDataSize = m_pOciDeal->pOCI_LobGetLength(lob);

		if (iActualDataSize <= 0)
		{			
			return FALSE;
		}
		strValue.resize(iActualDataSize);
		m_pOciDeal->pOCI_LobRead(lob, (void*)strValue.c_str(), iActualDataSize);
		
	}
	catch(...)
	{
		strValue.clear();
		return	FALSE;
	}

	return TRUE;
}
/**************************************************************************************************
  Function:			PutCollect(const char* szFieldName,const  void*	   pValue,const INT	iDataLen)    
  DateTime:			2013/5	
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
BOOL	COciRecordSet::PutCollect(const char* szFieldName,const  void*	   pValue,const INT	iDataLen)
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

		return TRUE;

}

/**************************************************************************************************
Function		: ClearFieldInfoList()    
DateTime		: 2013/5	
Description		: 清空数据字段列表（m_FieldInfoList ： AddNew 和 Update的时候用的）
Input			: NULL
Output			: NULL
Return			: TRUE
Note			:（m_FieldInfoList ： AddNew 和 Update的时候用的）	
**************************************************************************************************/
BOOL		COciRecordSet::ClearFieldInfoList()
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

/**************************************************************************************************
  Function:			CancelUpdate    
  DateTime:			2013/05	
  Description:		取消在调用 Update 方法前对当前记录或新记录所作的任何更改.
  Input:          	NULL
  Output:         	NULL
  Return:         	成功：TRUE 失败：FALSE
  Note:				
**************************************************************************************************/
BOOL	COciRecordSet::CancelUpdate()
{
	ClearFieldInfoList();	
	
	// 设置编辑模式为UnKnown
	m_iEditMode = EM_UnKnown;


	return FALSE;

}


/**************************************************************************************************
  Function:			AddFieldItem2FieldInfoList    
  DateTime:			2013/05	
  Description:		向字段列表中添加字段信息
  Input:          	NULL
  Output:         	NULL
  Return:         	成功：TRUE 失败：FALSE
  Note:				
**************************************************************************************************/
BOOL		COciRecordSet::AddFieldItem2FieldInfoList(CDBFieldInfo*	pcsDbFieldInfoItem)
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
  DateTime		: 2013/5	
  Description	: 根据查询数据的sql语句来获取其中的表名称(主要是对表查询后，进行插入数据和更新数据的时候用)
  Input			: Sql 查询语句，例如：SELECT * from TB_TEST;
  Output		: strTableName:操作的表名称
  Return		: TRUE:成功，FALSE:失败
  Note			: 打开数据集的地方要使用，将数据集的对应的表名称存入起来，用于更新和插入时候用
**************************************************************************************************/
BOOL		COciRecordSet::GetTableNameFromSqlText(const char* szSql,string& strTableName)
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


/**************************************************************************************************
Function		: BuildInsertUpdateSqlTxt()    
DateTime		: 2013/5	
Description		: 构造Sql语句进行操作
Input			: NULL
Output			: NULL
Return			: TRUE: 成功 ，FALSE :失败
Note			: 
**************************************************************************************************/
BOOL		COciRecordSet::BuildInsertUpdateSqlTxt()
{
	// 确保有元素存在
	if (m_FieldInfoList.empty())
	{
		return FALSE;
	}

	// 关闭数据集对象
	if (IsOpen()) 
	{
		Close();
	}
	//打开数据集
	if (Init())
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
				m_strSqlText += ":";
				m_strSqlText += m_FieldInfoList[j]->GetFieldName();
				if (j!=m_FieldInfoList.size() - 1)
				{
					m_strSqlText += ",";
				}
				else
				{
					m_strSqlText += ")";
				}
			}
			

			m_pOciDeal->pOCI_Prepare(m_pStmtset,(char*)m_strSqlText.c_str());


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
				m_strSqlText += " = ";
				m_strSqlText += ":";
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
			m_pOciDeal->pOCI_Prepare(m_pStmtset,(char*)m_strSqlText.c_str());

		}
		else
		{	
			// 只处理，更新和插入的，别的状态视为错误
			if (m_pStmtset)
				m_pOciDeal->pOCI_StatementFree(m_pStmtset);
			return FALSE;
		}

	}
	else
	{	
		return FALSE;	
	}

	return	TRUE;

}


/**************************************************************************************************
Function		: AppendValueToSqlTxt()    
DateTime		: 2013/5
Description		: 向Sql语句里面添加对应的值
Input			: NULL
Output			: NULL
Return			: TRUE:成功，FALSE:失败
Note			: 前件条件：BuildInsertUpdateSqlTxt()执行成功
				后件条件：ExecuteInsertUpdate()的执行
**************************************************************************************************/
BOOL		COciRecordSet::AppendValueToSqlTxt()
{
	// 确保有元素存在
	if (m_FieldInfoList.empty() || m_strSqlText.empty())
	{
		return FALSE;
	}

	// 判断对象是否存在
	if (m_pStmtset) 
	{	
		// 判断类型
		if (m_iEditMode == EM_AddNew)
		{
			// 逐个值的绑定添加
			for (DBFieldInfoVector::size_type i=0;i<m_FieldInfoList.size();i++)
			{
				switch (m_FieldInfoList[i]->GetFieldType())
				{
				case	FT_Integer:
					{
						// 绑定数据
						if (!m_pOciDeal->pOCI_BindInt(m_pStmtset, m_FieldInfoList[i]->GetFieldName().c_str(), &m_FieldInfoList[i]->m_iValue)) 
						{
							harder_error(m_pOciDeal->pOCI_GetLastError());
							return FALSE;
						}
					} 
					break;
				case	FT_String:
					{
						
						if (!m_pOciDeal->pOCI_BindString(m_pStmtset, m_FieldInfoList[i]->GetFieldName().c_str(), (char*)m_FieldInfoList[i]->m_strValue.c_str(), m_FieldInfoList[i]->m_strValue.size()+1))
						{
							harder_error(m_pOciDeal->pOCI_GetLastError());
							return FALSE;
						}
						
					} 
					break;
				case	FT_DateTime:
					{
						
						OCI_Date* date = m_pOciDeal->pOCI_DateCreate(NULL);
						if (!date) 
						{
							harder_error(m_pOciDeal->pOCI_GetLastError());
							return FALSE;
						}

						m_pOciDeal->pOCI_DateSetDateTime(date,
							m_FieldInfoList[i]->m_stDateTime.year, m_FieldInfoList[i]->m_stDateTime.month,m_FieldInfoList[i]->m_stDateTime.day,
							m_FieldInfoList[i]->m_stDateTime.hour,m_FieldInfoList[i]->m_stDateTime.minute,m_FieldInfoList[i]->m_stDateTime.second);

						if(!m_pOciDeal->pOCI_BindDate(m_pStmtset, m_FieldInfoList[i]->GetFieldName().c_str(), date))
						{
							harder_error(m_pOciDeal->pOCI_GetLastError());
							return FALSE;
						}
						

					}
					break;
				case FT_Binary:
					{
						if (!m_pOciDeal->pOCI_BindString(m_pStmtset, m_FieldInfoList[i]->GetFieldName().c_str(), (char*)m_FieldInfoList[i]->m_pValue, m_FieldInfoList[i]->GetBinaryValueLen()+1))
						{
							harder_error(m_pOciDeal->pOCI_GetLastError());
							return FALSE;
						}
						
					} 
					break;
				default:
					break;
				}

			} 

		}
		else if (m_iEditMode == EM_Edit)
		{
			for (DBFieldInfoVector::size_type i=0;i<m_FieldInfoList.size();i++)
			{
				switch (m_FieldInfoList[i]->GetFieldType())
				{
				case	FT_Integer:
					{ 
						// 绑定数据
						if (!m_pOciDeal->pOCI_BindInt(m_pStmtset, m_FieldInfoList[i]->GetFieldName().c_str(), &m_FieldInfoList[i]->m_iValue)) 
						{
							harder_error(m_pOciDeal->pOCI_GetLastError());
							return FALSE;
						}
					} 
					break;
				case	FT_String:
					{
						if (!m_pOciDeal->pOCI_BindString(m_pStmtset, m_FieldInfoList[i]->GetFieldName().c_str(), (char*)m_FieldInfoList[i]->m_strValue.c_str(), m_FieldInfoList[i]->m_strValue.size()+1))
						{
							harder_error(m_pOciDeal->pOCI_GetLastError());
							return FALSE;
						}
					} 
					break;
				case	FT_DateTime:
					{
						OCI_Date* date = m_pOciDeal->pOCI_DateCreate(NULL);
						if (!date) 
						{
							harder_error(m_pOciDeal->pOCI_GetLastError());
							return FALSE;
						}

						m_pOciDeal->pOCI_DateSetDateTime(date,
							m_FieldInfoList[i]->m_stDateTime.year, m_FieldInfoList[i]->m_stDateTime.month,m_FieldInfoList[i]->m_stDateTime.day,
							m_FieldInfoList[i]->m_stDateTime.hour,m_FieldInfoList[i]->m_stDateTime.minute,m_FieldInfoList[i]->m_stDateTime.second);

						if(!m_pOciDeal->pOCI_BindDate(m_pStmtset, m_FieldInfoList[i]->GetFieldName().c_str(), date))
						{
							harder_error(m_pOciDeal->pOCI_GetLastError());
							return FALSE;
						}
						

					}
					break;
				case FT_Binary:
					{
						if (!m_pOciDeal->pOCI_BindString(m_pStmtset, m_FieldInfoList[i]->GetFieldName().c_str(), (char*)m_FieldInfoList[i]->m_pValue, m_FieldInfoList[i]->GetBinaryValueLen()+1))
						{
							harder_error(m_pOciDeal->pOCI_GetLastError());
							return FALSE;
						}

					} 
					break;
				default:
					break;
				}

			}

		}
		else
		{	
			// 只处理，更新和插入的，别的状态视为错误
			if (m_pStmtset)
				m_pOciDeal->pOCI_StatementFree(m_pStmtset);
			return FALSE;
		}

	}
	else
	{	
		return FALSE;	
	}

	return	TRUE;
}

/**************************************************************************************************
  Function		: ExecuteInsertUpdate()    
  DateTime		: 2013/5	
  Description	: 执行SQL 插入或者更新操作(INSERT 和 UPDATE)
  Input			: NULL
  Output		: NULL
  Return		: TRUE:成功，FALSE:失败
  Note			: 前件条件：BuildInsertUpdateSqlTxt()执行成功
							AppendValueToSqlTxt()执行成功	
**************************************************************************************************/
BOOL		COciRecordSet::ExecuteInsertUpdate()
{
	// 执行操作
	if (m_pStmtset)
	{
		m_pOciDeal->pOCI_Execute(m_pStmtset);
		if(!m_pOciDeal->pOCI_Commit(m_pConnection->GetOciCon()))
			return	FALSE;
	}
	else
	{
		return	FALSE;
	}

	return TRUE;
}

/**************************************************************************************************
  Function		: GetColumnList()    
  DateTime		: 2013/5	
  Description	: 获取字段名称列表
  Input			: NULL
  Output		: NULL
  Return		: 成功返回TRUE,失败返回FALSE
  Note			: NULL
**************************************************************************************************/
BOOL		COciRecordSet::GetColumnList()
{
	return	FALSE;
}

/**************************************************************************************************
  Function		: ClearColumnList()    
  DateTime		: 2013/5	
  Description	: 清空字段列表
  Input			: NULL
  Output		: NULL
  Return		: TRUE
  Note			: NULL
**************************************************************************************************/
BOOL		COciRecordSet::ClearColumnList()
{
	//m_ColumnItemList.clear();
	return	FALSE;

}


/**************************************************************************************************
  Function		: GetColumnIndexByName(const char* szFieldName)   
  DateTime		: 2013/5	
  Description	: 根据字段名称，获取列的索引
  Input			: 字段名称
  Output		: NULL
  Return		: 列的索引
  Note			: 获取失败返回 ERROR_SQL_COLUMN_INDEX
**************************************************************************************************/
INT		  COciRecordSet::GetColumnIndexByName(const char* szFieldName)
{
	/*
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
	*/
	return 0;

}