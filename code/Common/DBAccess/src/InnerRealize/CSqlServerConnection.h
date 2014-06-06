#ifndef CSQLSERVERCONNECTION_DEF_H
#define CSQLSERVERCONNECTION_DEF_H

// 头文件
#include "CConnection.h"

/**************************************************************************************************
Copyright (C), 2010-2011, GOSUN 
File name 	: CSQLSERVERCONNECTION.H      
Author		: liujs     
Version 		: Vx.xx        
DateTime 		: 2010/6/7 14:49
Description 	:  Sql Server 2000 / 2005 数据库连接对象类
**************************************************************************************************/
namespace	DBAccessModule
{
	class CSqlServerConnection :public CConnection
	{
	public:
		CSqlServerConnection(void);
		virtual ~CSqlServerConnection(void);

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
		string			ToTime(const char*	szDateTime)
		{	
			char	szDt[64] = {0};
			sprintf(szDt, "'%s'", szDateTime);
			return	string(szDt);
		}
		string			ToDate(const char*	szDateTime)
		{	
			char	szDt[64] = {0};
			sprintf(szDt, "'%s'", szDateTime);
			return	string(szDt);
		}	
		string			ToDateTime(const char*	szDateTime)
		{
			char	szDt[64] = {0};
			sprintf(szDt, "'%s'", szDateTime);
			return	string(szDt);
		}

	public:
		// 内部接口
		BOOL	Open();
		// 关闭
		BOOL	Close();
		// 判断是否打开
		BOOL	IsOpen();

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

	protected:
		// 获取完整的sql语句
		BOOL	GetFullPageQuerySql(const char* szSql,const INT32	iStartRow,const INT32 iRowNum,string& strFullSql);
		// 查找关键字，成功：TRUE，失败：FALSE 并返回关键字前后的字符串
		BOOL	Find_keyWord_GetString(const char* szSql,const char* szKeyWord,string& strBefore,string& strAfter);

	public:
	#if 0
		// 连接对象 ----------------------------------
		inline	_ConnectionPtr& GetConnection() {return m_pConnection;};
	#else
		// Linux
		// 返回ODBC数据库环境句柄
		inline	SQLHENV&		GetOdbcSqlHEven()
		{
			return	m_hEnv;
		}
		// 返回ODBC数据库连接对象句柄
		inline	SQLHANDLE&		GetOdbcSqlHDbc()
		{
			return	m_hDbc;
		}
	#endif

	protected:
	#if 0    
		// 数据库连接对象成员变量
		_ConnectionPtr	m_pConnection;
	#else
		// Linux	
		SQLHENV			m_hEnv;			// ODBC 环境句柄
		SQLHDBC			m_hDbc;			// ODBC 连接对象句柄
		BOOL			m_bConnectFlag;	// 判断数据库是否连接，TRUE：连接  FALSE:不连接
	#endif   

	};


}

#endif // CSQLSERVERCONNECTION_DEF_H
