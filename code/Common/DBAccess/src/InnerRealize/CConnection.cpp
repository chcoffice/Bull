#include "CConnection.h"

using	namespace	DBAccessModule;



CConnection::CConnection(void)
{
	// 属性
	memset(m_szServer,0x0,coniDataLen);
	memset(m_szDatabase,0x0,coniDataLen);
	memset(m_szUser,0x0,coniDataLen);
	memset(m_szPass,0x0,coniDataLen);
	m_eConnectionUseFlag = CONNECTION_IDLESSE ;    
	// 唯一的连接ID,根据连接ID从连接池中找到唯一的连接
	m_iConnectID = 0;
	m_pCnnPool = NULL;

}

CConnection::~CConnection(void)
{
	// 属性
	memset(m_szServer,0x0,coniDataLen);
	memset(m_szDatabase,0x0,coniDataLen);
	memset(m_szUser,0x0,coniDataLen);
	memset(m_szPass,0x0,coniDataLen);
	m_eConnectionUseFlag = CONNECTION_IDLESSE ;    
	// 唯一的连接ID,根据连接ID从连接池中找到唯一的连接
	m_iConnectID = 0;


}


/**************************************************************************************************
  Function:  Initial  
  DateTime: 2010/5/23 16:15	
  Description: 初始化数据库服务器，数据库名称，用户名，密码等信息
  Input:          	 // 输入参数说明，包括每个参数的作
  Output:         	// 对输出参数的说明。
  Return:         	// 函数返回值的说明
  Note:				// 备注
**************************************************************************************************/
BOOL	CConnection::Initial(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass)
{
	memset(m_szServer,0x0,coniDataLen);
	memcpy(m_szServer,szServer,strlen(szServer));
	
	memset(m_szDatabase,0x0,coniDataLen);
	memcpy(m_szDatabase,szDatabase,strlen(szDatabase));

	memset(m_szUser,0x0,coniDataLen);
	memcpy(m_szUser,szUser,strlen(szUser));


	memset(m_szPass,0x0,coniDataLen);
	memcpy(m_szPass,szPass,strlen(szPass));
	return  TRUE;

}

