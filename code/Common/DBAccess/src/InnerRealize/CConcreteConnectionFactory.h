#ifndef CCONCRETECONNECTIONFACTORY_DEF_H
#define CCONCRETECONNECTIONFACTORY_DEF_H


#include "IConnectionFactory.h"
/**************************************************************************************************
Copyright (C), 2010-2011, GOSUN 
File name 	: CCONCRETECONNECTIONFACTORY.H      
Author		: liujs     
Version 		: Vx.xx        
DateTime 		: 2010/6/7 14:24
Description 	: 具体工厂类实现
**************************************************************************************************/
namespace	DBAccessModule
{

	class CConcreteConnectionFactory :public IConnectionFactory
	{
	public:
		CConcreteConnectionFactory(void);
		virtual ~CConcreteConnectionFactory(void);

	public:
		// 接口实现

		// Oracle 数据库连接
		CConnection*		CreateOracleConnection(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass);
		// Sql 数据库连接
		CConnection*		CreateSqlConnection(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass);	
		// Access 数据库连接
		CConnection*		CreateAccessConnection(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass);
		// MySQL 数据库连接
		CConnection*		CreateMySQLConnection(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass);
		// Oci 数据库连接
		CConnection*		CreateOciConnection(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass);
	};

}

#endif // CCONCRETECONNECTIONFACTORY_DEF_H


