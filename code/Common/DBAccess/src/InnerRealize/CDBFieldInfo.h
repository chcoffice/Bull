#ifndef CDBFIELDINFO_DEF_H
#define CDBFIELDINFO_DEF_H

#include "DbAccessDataDef.h"

namespace	DBAccessModule
{

	// 字段类型，使用ODBC API进行数据库操作的时候（AddNew PutCollect Update时候用），存储字段的类型
	// 类型为：字符串，整数，二进制
	enum EnumFieldType
	{
		FT_UnKnown = -1,			// 未知类型
		FT_Integer = 0,			// 0：整形
		FT_String,				// 1：字符串(Float ,double,大数据等)
		FT_Binary,				// 2：二进制大对象类型
		FT_DateTime				// 3：时间类型
	};

	// 二进制大对象的最大值大小为 100M
	// char 表示一个字节，1024 * 1 = 1KB
	// 1024 * 1(KB) = 1 MB
	// 1024 * 1024 = 1MB   1024 * 1024* 10*10 = 100M
	const		INT			coniMaxBinaryValueSize = 10 * 10 * 1024 *1024 ;				


	/**************************************************************************************************
	Copyright (C), 2010-2011, GOSUN 
	File name 	: CDBFIELDINFO.H      
	Author 		: Liujs      
	Version 		: Vx.xx        
	DateTime 		: 2010/6/20 12:01
	Description 	: 用于存储ODBC API进行数据库字段操作的时候的，字段信息的存储
	LINUX时候用，WINDOWS用ADBC或者别的已经存在了				  
	**************************************************************************************************/
	class CDBFieldInfo
	{
	public:
		CDBFieldInfo(void);
		virtual ~CDBFieldInfo(void);
		// 显示的清空对象
		void	Clear();

	protected:
		// 字段名称(也就是列名称)
		string			m_strFieldName;
		// 字段类型
		EnumFieldType	m_eFieldType;
		// 列的索引
		INT				m_iColumnIndex;

	public:

		// 设置、获取字段名称
		inline		void	SetFieldName(const char* szFiledName)
		{
			m_strFieldName	=	string(szFiledName);
		}
		inline		string	GetFieldName()
		{
			return	m_strFieldName;
		}

		// 设置、获取字段类型
		inline		void	SetFieldType(const EnumFieldType eFtType)
		{
			m_eFieldType = eFtType;
		}
		inline		EnumFieldType	GetFieldType()
		{
			return	m_eFieldType;
		}

		// 设置，获取列的索引
		inline		void	SetColumnIndex(const INT iColumnIndex)
		{
			m_iColumnIndex = iColumnIndex;
		}
		inline		INT		GetColumnIndex()
		{
			return	m_iColumnIndex;
		}

	public:
		// 进行SQLBindParameter绑定时候的，操作函数,SQLBindParameter函数对应的最后一个参数
		INT				m_icpValue;

	public:
		// 传入值的时候用的
		// 对应 m_strValue的值的获取
		char			m_szRefValue[FIELD_DATA_LEN];
		// 对应的m_iValue的值的获取
		INT				m_iRefValue;

		//protected:
	public:
		// 字段对应的值(选其一类型)

		// 整形
		INT				m_iValue;

		// 字符型（string , float ,double ,日期等）
		string			m_strValue;

		// 二进制对象值
		INT				m_iBinaryValueLen;
		char*			m_pValue;

		// oracle 时间字段
		TIMESTAMP_STRUCT	m_stDateTime;

	public:
		// 读取和设置对应的整形的值
		inline		void	SetIntegerValue(const INT	iValue)
		{
			m_iValue = iValue;
		}
		inline		INT		GetIntegerValue()
		{
			return	m_iValue;
		}
		
		// 设置时间字段的值
		inline		void	SetDateTimeValue(const INT iYear,const INT iMonth,const INT iDay,const INT iHour,const INT iMinute,const INT iSecond)
		{
			m_stDateTime.year = iYear;
			m_stDateTime.month = iMonth;
			m_stDateTime.day = iDay;
			m_stDateTime.hour = iHour;
			m_stDateTime.minute = iMinute;
			m_stDateTime.second = iSecond;
		}

		// 字符型（string , float ,double ,日期等）
		// 读取和设置对应的字符串类型的值
		inline		void	SetStringValue(const char* szValue)
		{
			m_strValue = string(szValue);
		}
		inline		string	GetStringValue()
		{
			return	m_strValue;
		}

		// 二进制对象值
		// 读取和设置二进制对象大小的
		inline		void	SetBinaryValueLen(const		INT	iBinaryValueLen)
		{
			m_iBinaryValueLen = iBinaryValueLen;
		}
		inline		INT	GetBinaryValueLen()
		{
			return	m_iBinaryValueLen;
		}

		// 获取二进制对象数据和设置二进制对象数据
		BOOL	SetBinaryValue(const void* pSourceValue,const INT	iSourceLen);
		// 获取数据
		BOOL	GetBinaryValue(void* pSourceValue,INT& iGetLen);
	};

	// 存储字段的列表，用于ADDNEW 和 修改值的时候，用内存对象
	typedef	vector<CDBFieldInfo*>				DBFieldInfoVector;
	typedef	vector<CDBFieldInfo*>::iterator		DBFieldInfoVectorIterator;



	// 返回一个将字符串大写的类，配合transform使用，linux版本下用
	class ToUpper{
	public: 
		char operator()(char val){   
			return toupper( val ); 
		} 
	}; 

}

#endif // CDBFIELDINFO_DEF_H

