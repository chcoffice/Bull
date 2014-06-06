/**************************************************************************************************
* Copyrights 2013  高新兴
*                  基础应用组
* All rights reserved.
*
* Filename：
*       COciRecordSet.h
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

#ifndef COCIRECORDSET_DEF_H
#define COCIRECORDSET_DEF_H

#include "CRecordSet.h"
#include "COciConnection.h"
#include "DbAccessDataDef.h"
#include "CDBFieldInfo.h"

//#include "ocilib.h"

namespace	DBAccessModule
{

	class COciRecordSet :public CRecordSet
	{
	public:

		COciRecordSet(void);

		virtual ~COciRecordSet(void);


	public:

		// 移动索引
		// 记录集尾部
		BOOL	Eof();
		// 移向最后一条记录
		BOOL	MoveLast();
		// 下一条记录
		BOOL	MoveNext();
		// 添加记录
		BOOL    AddNew();
		// 更新记录
		BOOL    Update();
		// 添加记录
		BOOL	AddNew(const	char*	szTableName);


	public:

		// 值的获取，设置
		// 根据列名称获取对应字段的值，返回string 类型字符串(INT64,long long，float,double等都采用string处理)
		BOOL	GetCollect(const char* szFieldName,string&	strValue);

		BOOL GetBlobCollect(const char* szFieldName,string& strValue);
		// 根据列名设置对应列的值
		BOOL	PutCollect(const char* szFieldName,const char*     szValue);

		// 根据列名称获取对应字段的值，返回INT 类型字符串
		BOOL	GetCollect(const char* szFieldName,INT&	iValue);
		// 根据列名设置对应列的值
		BOOL	PutCollect(const char* szFieldName,const INT	iValue);

		// 根据列名称获取对应字段的值，返回viod* 类型字符串和数据
		BOOL	GetCollect(const char* szFieldName,void*	   pValue,const INT iBuffSize,INT&	iDataLen);
		// 根据列名设置对应列的值
		BOOL	PutCollect(const char* szFieldName,const  void*	   pValue,const INT	iDataLen);
		// 根据列名称设置对应时间字段的值
		BOOL	PutDtCollect(const char* szFieldName,const INT iYear,const INT iMonth,const INT iDay,const INT iHour,const INT iMinute,const INT iSecond);
		// 释放自身对象
		BOOL	ReleaseRecordSet();


		INT GetColumnNumber(void);


		BOOL GetCloumnName(INT iColIndex, std::string &oStrName );


	public:    

		// 查询接口
		IRecordSet*		QuerySql(const char*  szSql);

	public:

		// 设置连接对象
		BOOL			SetConnection(COciConnection* pConnection);


	protected:

		void harder_error(OCI_Error* err);

		// 获取数据集判断是否可以进行编辑
		BOOL	GetEditMode();
		// 取消在调用 Update 方法前对当前记录或新记录所作的任何更改.
		BOOL	CancelUpdate();
		// 判断是否打开
		BOOL	IsOpen();
		// 关闭数据集
		BOOL	Close();
		// 打开数据集
		BOOL	Open(const char*  szSql);

		// 初始化STMT句柄相关信息
		BOOL		Init();

		// 获取值
		// 通过字段名称获取对应的列的索引
		INT			GetColumnIndexByName(const char* szFieldName);
		// 获取字段名称列表
		BOOL		GetColumnList();
		// 清空字段名称列表
		BOOL		ClearColumnList();

		// 设置值
		// 根据查询数据的sql语句来获取其中的表名称
		BOOL		GetTableNameFromSqlText(const char* szSql,string& strTableName);
		// 清空数据字段列表（m_FieldInfoList ： AddNew 和 Update的时候用的）
		BOOL		ClearFieldInfoList();
		// 向字段列表中添加字段信息
		BOOL		AddFieldItem2FieldInfoList(CDBFieldInfo*	pcsDbFieldInfoItem);

	public:
		// 创建执行的Sql语句（INSERT 和 UPDATE 的语句）
		BOOL		BuildInsertUpdateSqlTxt();
		// 向Sql语句里面添加对应的值
		BOOL		AppendValueToSqlTxt();
		// 执行SQL 插入或者更新操作(INSERT 和 UPDATE)
		BOOL		ExecuteInsertUpdate();


	protected:	

		//声明
		OCI_Statement*  m_pStmtset;

		//结果集
		OCI_Resultset*  m_pResultset;

		//连接类指针
		COciConnection*	    m_pConnection;

		//错误处理数组
		char m_error[1024];

		// 数据集是否打开
		BOOL					m_bIsOpen;

		// 判断是否是记录集尾部
		BOOL					m_bEof;

		// 列数对应自增长ID,在Insert 和 Update的是时候用到
		INT						m_iCollumIndexID;


	public:
		// 清空自增长列数
		inline	void	InitCollumnIndexID()
		{
			m_iCollumIndexID = coniZero;
		}
		// 列数索引的产生
		inline		INT	GenerateCollumnIndexID()
		{
			return	++m_iCollumIndexID;
		}

	protected:
		// 字段名称列表(获取值的时候用到的)
		//ColumnItemVector		m_ColumnItemList;

	protected:
		// 插入和更新数据的时候用到

		// 插入和更新的时候用的
		//SQLHSTMT				m_hUpdateSqlStmt;
		// 编辑模式				
		INT						m_iEditMode;
		// 操作的表名称（插入和更新的时候用到，查询的时候从SQL 语句中将其自动的解析出来）
		string					m_strTableName;
		// 操作的SQL字符串
		string					m_strSqlText;
		// 最近查询出去的一条数据的ID
		INT						m_iCurrenID;
		// 操作的字段信息结合（AddNew 和 Update的时候用的）
		DBFieldInfoVector		m_FieldInfoList;



	};

}
#endif // COCIRECORDSET_DEF_H

