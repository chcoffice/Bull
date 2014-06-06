#ifndef CCONNECTION_DEF_H
#define CCONNECTION_DEF_H

// OuterInterface头文件
#include "../OuterInterface/IDBAccessModule.h"
#include "DbAccessDataDef.h"


/**************************************************************************************************
Copyright (C), 2010-2011, GOSUN 
File name : CCONNECTION.H      
Author : liujs     
Version : Vx.xx        
DateTime : 2010/5/22 11:26
Description : 
连接对象类
**************************************************************************************************/
namespace	DBAccessModule
{

	class CConnection : public IConnection
	{
	public:
		void *m_pCnnPool; //连接池
		CConnection(void);
		virtual	~CConnection(void);

	protected:
		// 服务器地址
		char	m_szServer[coniDataLen];
		// 数据库名称,ORACLE数据库，填写和m_strServer一样，采用ODBC部署的时候，数据源就存储到该变量
		char	m_szDatabase[coniDataLen];	
		// 用户名
		char	m_szUser[coniDataLen];	
		// 密码
		char	m_szPass[coniDataLen];	

	protected:
		// 使用标识
		EnumConnectionUseFlag	m_eConnectionUseFlag;    
		// 唯一的连接ID,根据连接ID从连接池中找到唯一的连接
		INT     m_iConnectID;


	public:

		// 读取是否被占用标识
		inline	EnumConnectionUseFlag	GetConnectionUseFlag() const 
		{
			return	m_eConnectionUseFlag;
		}
		// 设置是否被占用标识
		inline	void		SetConnectionUseFlag(EnumConnectionUseFlag eConnectionUseFlag)
		{
			m_eConnectionUseFlag = eConnectionUseFlag;
		}	
		// 获取唯一连接ID
		inline  INT     GetConnectID() const 
		{
			return  m_iConnectID;
		}
		// 设置唯一连接ID
		inline  void    SetConnectID(const INT iConnnectID)
		{
			m_iConnectID = iConnnectID;
		}
		// 初始化数据库信息
		BOOL	Initial(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass);

	public:
		//------------------------------------------------------------------------------------------------
		// 内部接口的定义
		// 打开数据库
		virtual	BOOL	Open()	=	0;
		// 关闭数据库连接
		virtual	BOOL	Close()	=	0;
		// 判断是否打开
		virtual	BOOL	IsOpen() = 0;

	public:
		// ------------------------------------------------------------------------------------------------
		// 事务处理 ，外部接口，内部无效定义接口
		// -----------------------------------------------------------------------------------------------

	};
	// 定义存储的队列
	typedef	vector<CConnection*>				Connection_Vector;
	// 队列的迭代器
	typedef vector<CConnection*>::iterator		Connection_Vector_Iterator;



}

#endif // CCONNECTION_DEF_H


