/**************************************************************************************************
* Copyrights 2013  高新兴
*                  基础应用组
* All rights reserved.
*
* Filename：
*       COciConnection.h
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

#ifndef COCICONNECTION_DEF_H
#define COCICONNECTION_DEF_H


#include "CConnection.h"
#include "ocilib.h"

namespace	DBAccessModule
{

	class COciConnection :
		public CConnection
	{
	public:
		COciConnection(void);
		virtual ~COciConnection(void);

	public:
		// 执行Sql语句，输入参数
		// const char*  szSql:执行操作的Sql语句
		// 返回值：成功TRUE,失败FALSE
		BOOL			ExecuteSql(const char*  szSql);

		// 执行Sql语句，输入参数
		// const char*  szSql:执行操作的Sql语句
		// 返回值：成功TRUE,失败FALSE
		BOOL			ExecuteSql(const char*  szSql,const char* szTable,INT64& lRowId);

		// 查询sql语句,输入参数
		// const char*  szSql:执行操作的Sql语句
		// 返回值：IRecordset对象接口对象，可以根据IRecordset接口执行相应的接口提供方法
		IRecordSet*		ExecuteQuery(const char*  szSql);	

		// 返回一个空的数据集，Addnew数据的时候用到的
		IRecordSet*		GetEmptyRecordSet();

		// 释放连接对象接口，将使用完毕后的连接对象，返回到连接池中
		// 参数说明
		// 返回值: 成功TRUE，失败FALSE
		BOOL			ReleaseConnection();

		// 分页查询数据
		IRecordSet*		ExecutePageQuery(const char*  szSql,const INT32	iStartRow,const INT32 iRowNum);

		// 时间字符串转换，时间基类接口
		string			ToTime(const char*	szDateTime);
		string			ToDate(const char*	szDateTime);
		string			ToDateTime(const char*	szDateTime);

	public:
		// 内部接口
		BOOL	Open();
		// 关闭
		BOOL	Close();
		// 判断是否打开
		BOOL	IsOpen();
		// 释放资源
		BOOL fini();
		// 初始化环境
		BOOL init();
		// 出错处理
		void dump_error(OCI_Error* err);

	public:
		// 开始执行事物
		UINT	BeginTrans();
		// 回滚事物
		BOOL	RollbackTrans();
		// 提交事物
		BOOL	CommitTrans();

	public:
		// 获取错误链接并进行相应操作：主要针对网络断开，数据库断开等操作控制
		BOOL	GetConnectionErrorAndDeal();
	public:

		// 返回数据库连接对象句柄
		inline	OCI_Connection*	GetOciCon()
		{
			return	m_pCon;
		}


	protected:
		// 查找关键字，成功：TRUE，失败：FALSE 并返回关键字前后的字符串
		BOOL	Find_keyWord_GetString(const char* szSql,const char* szKeyWord,string& strBefore,string& strAfter);
		// 获取sql语句中“from” 前面的字符串
		BOOL	GetBeforeFromSqlText(const char* szSql,string& strBeforeFromSql);
		// 在Select 后面插入 ROWNUM R，关键字 
		BOOL	InsertRowinfo2SqlText(const char* szSql,string& strSql);
		// 将简单的sql语句，其实行号，行数，合成完整的字符串
		BOOL	GetFullPageQuerySql(const char* szSql,const INT32	iStartRow,const INT32 iRowNum,string& strFullSql);

	protected:

		//连接对象
		OCI_Connection* m_pCon;

		//错误处理数组
		char m_error[1024];

		//关闭标志
		boolean m_bclose;

		

	};

}
#endif // COCICONNECTION_DEF_H

