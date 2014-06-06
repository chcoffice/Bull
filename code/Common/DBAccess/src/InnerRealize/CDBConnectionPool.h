
#ifndef CDBCONNECTIONPOOL_DEF_H
#define CDBCONNECTIONPOOL_DEF_H

// 头文件
#include "../OuterInterface/IDBAccessModule.h"			//  外部接口头文件类
#include "CConnection.h"								//	连接类对象
#include "CConcreteConnectionFactory.h"					//  具体工厂类对象
#include "DbAccessDataDef.h"							//  公共变量定义
#include "OciDllDeal.h"


/**************************************************************************************************
Copyright (C), 2010-2011, GOSUN 
File name : CDBCONNECTIONPOOL.H      
Author : liujs     
Version : Vx.xx        
DateTime : 2010/5/22 10:21
Description : 连接池对象类
**************************************************************************************************/
namespace	DBAccessModule
{

	class CDBConnectionPool : public IConnectionPool
	{
	protected:
		// 不提供外部创建对象的方法，用单例提供的方法
		CDBConnectionPool(void);
		virtual ~CDBConnectionPool(void);

	private:
		// 单例模型
		static	CDBConnectionPool*		m_pInstance;
		// 单例锁
		static	CGSMutex				m_CsSingleton;


	public:
		// 创建单例
		static	CDBConnectionPool*		Instance();

		static  CDBConnectionPool* Create(void)
		{
			return new CDBConnectionPool();
		}
		// 释放资源
		BOOL	Release();

	public:
		// 获取连接对象接口 [5/22/2010 liujs] 
		virtual IConnection*	GetConnection(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass,const INT iDbType);

		// 释放连接，将连接对象放回连接池
		BOOL			ReleaseConnection(CConnection*	pConnection);
	private:
		// 判断用户输入条件是否准确
		BOOL			InputDataVerify(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass,const INT iDbType);

//	public:
		// 创建当个连接对象
		CConnection*    CreateConnectionHandle(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass,const INT iDbType);
		// 批量创建对象,添加到连接池中
		BOOL            BatchCreateConnHandle(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass,const INT iDbType,const INT iConnNum);
// 		// 判断所有的连接是否在使用,在使用，返回TRUE,否则返回FALSE
// 		BOOL            BOOLAllConnectionUsed();

//	public:
		// 添加到队列中
		BOOL            AddConnecttion(CConnection*   pConnection);
		// 删除队列中的元素
		BOOL            DeleteConnection(CConnection* pConnection);
		// 删除所有的元素
		BOOL			DeleteAllConnection();
		// 获取一个有效的连接
		CConnection*	GetOneConnectionFromPool();
		// 获取连接池元素个数
		INT				GetConnectionPoolNum();
		



	protected:
		// 连接池,暂时存入vector中，采用异构表的方式，支持多类型的处理
		// 唯一的连接ID,根据连接ID从连接池中找到唯一的连接
		Connection_Vector			m_ConnectionPool;  
		// 队列锁变量区
		CGSMutex					m_CsConnectionPool;
		// 连接抽象工厂对象
		IConnectionFactory*			m_pConnectionFactory;

	};
	// DLL处理类
	extern OciDllDeal*          m_pOciDeal;


}
#endif // CDBCONNECTIONPOOL_DEF_H

