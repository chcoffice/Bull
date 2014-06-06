#ifndef ICONNECTIONFACTORY_DEF_H
#define ICONNECTIONFACTORY_DEF_H

// 头文件
#include "DbAccessDataDef.h"
#include "COracleConnection.h"
#include "CSqlServerConnection.h"
#include "MySQLConnection.h"
#include "COciConnection.h"

/**************************************************************************************************
Copyright (C), 2010-2011, GOSUN 
File name : ICONNECTIONFACTORY.H      
Author : liujs     
Version : Vx.xx        
DateTime : 2010/5/23 16:27
Description :  连接工厂接口
**************************************************************************************************/
namespace	DBAccessModule
{

	class IConnectionFactory
	{
	public:
		IConnectionFactory(void);
		virtual ~IConnectionFactory(void);

	protected:
		// 连接对象自增长ID
		static          INT         m_iAutoConnectID;  	

	public:
		// 返回自增长连接对象ID
		inline  INT             GenerateAutoConnectID() const 
		{
			return  m_iAutoConnectID++;
		}

	public:
		// Oracle 数据库连接
		virtual	CConnection*		CreateOracleConnection(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass) = 0;


		// Sql 数据库连接
		virtual	CConnection*		CreateSqlConnection(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass) = 0;

		// Access 数据库连接
		virtual	CConnection*		CreateAccessConnection(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass) = 0;

		// MySQL 数据库连接
		virtual CConnection*		CreateMySQLConnection(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass) = 0;
		// Oci 数据库连接
		virtual	CConnection*		CreateOciConnection(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass) = 0;

	};

}
#endif // ICONNECTIONFACTORY_DEF_H
