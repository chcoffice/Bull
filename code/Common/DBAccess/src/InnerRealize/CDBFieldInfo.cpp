#include "CDBFieldInfo.h"

using	namespace	DBAccessModule;

CDBFieldInfo::CDBFieldInfo(void)
{
	m_eFieldType = FT_UnKnown;
	m_iValue = coniZero;
	m_strFieldName = constrDefaultString;
	m_iBinaryValueLen = coniZero;
	m_pValue	=	NULL;
	m_iColumnIndex = coniZero;
	memset(m_szRefValue,0x0,FIELD_DATA_LEN);
	m_iRefValue = coniZero;
	m_icpValue = coniZero;
}

CDBFieldInfo::~CDBFieldInfo(void)
{
	m_eFieldType = FT_UnKnown;
	m_iValue = coniZero;
	m_strFieldName = constrDefaultString;
	m_iBinaryValueLen = coniZero;
	m_iColumnIndex = coniZero;

	// 释放内存
	if (m_pValue != NULL)
	{
		delete[] m_pValue;
		m_pValue = NULL;
	}
	// 对应 m_strValue的值的获取
	memset(m_szRefValue,0x0,FIELD_DATA_LEN);

	// 对应的m_iValue的值的获取
	m_iRefValue = coniZero;
	m_icpValue = coniZero;
}


/**************************************************************************************************
  Function		: Clear()    
  DateTime		: 2010/6/20 16:41	
  Description	: 清空字段对象
  Input			: NULL
  Output		: NULL
  Return		: NULL
  Note			: 调用时候，建议显示调用清除对象
**************************************************************************************************/
void	CDBFieldInfo::Clear()
{
	m_eFieldType = FT_UnKnown;
	m_iValue = coniZero;
	m_strFieldName = constrDefaultString;
	m_iBinaryValueLen = coniZero;
	m_iColumnIndex = coniZero;

	// 释放内存
	if (m_pValue != NULL)
	{
		delete[] m_pValue;
		m_pValue = NULL;
	}

	// 对应 m_strValue的值的获取
	memset(m_szRefValue,0x0,FIELD_DATA_LEN);

	// 对应的m_iValue的值的获取
	m_iRefValue = coniZero;
	m_icpValue = coniZero;
}


/**************************************************************************************************
  Function		: SetBinaryValue(const void* pSourceValue,const INT	iSourceLen)     
  DateTime		: 2010/6/20 15:43	
  Description	: 设置二进制对象大小
  Input			: pSourceValue：二进制数据    iSourceLen：长度(不包括‘\0’字符长度)
  Output		: NULL
  Return		: TRUE:成功   FALSE：失败
  Note			: 最大为10M的数据
**************************************************************************************************/
BOOL	CDBFieldInfo::SetBinaryValue(const void* pSourceValue,const INT	iSourceLen)
{
	if (iSourceLen > coniMaxBinaryValueSize)
	{
		return FALSE;
	}

	try
	{
		m_pValue = new char[iSourceLen+1];
		memset(m_pValue,0x0,iSourceLen+1);
		// 拷贝数据
		memcpy((void*)m_pValue,pSourceValue,iSourceLen);

		return	TRUE;
	}
	catch(...)
	{
		// 防止内存分配错误
		return	FALSE;
	}

	return	TRUE;

}

/**************************************************************************************************
  Function		: GetBinaryValue(void* pSourceValue,INT iGetLen)    
  DateTime		: 2010/6/20 15:56	
  Description	: 获取二进制数据
  Input			: pSourceValue: 要获取的数据， iGetLen ：要获取的数据长度：-1:表示全部获取
  Output		: pSourceValue: 要获取的数据， iGetLen ：已经获取的长度，不包括‘\0’
  Return		: TRUE:成功， FALSE:失败
  Note			: 获取二进制数据处理,pSourceValue：外外面分配的内存，要是错了，不负责
**************************************************************************************************/
BOOL	CDBFieldInfo::GetBinaryValue(void* pSourceValue,INT& iGetLen)
{
	try
	{
		// 全部拷贝
		if (iGetLen == -1)
		{
			// 拷贝
			memcpy(pSourceValue,m_pValue,m_iBinaryValueLen);

			iGetLen = m_iBinaryValueLen;
		}
		else if(iGetLen >= m_iBinaryValueLen)
		{
			// 要求的比里面大，取出全部
			// 拷贝
			memcpy(pSourceValue,m_pValue,m_iBinaryValueLen);

			iGetLen = m_iBinaryValueLen;
		}
		else
		{
			// 只拷贝部分
			// 拷贝
			memcpy(pSourceValue,m_pValue,iGetLen);
		}

	}
	catch(...)
	{
		return	FALSE;
	}

	return	TRUE;

}


