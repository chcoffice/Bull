
#ifndef DBACCESSDATADEF_DEF_H
#define DBACCESSDATADEF_DEF_H

// 系统库头文件
#include <algorithm>
#include <iostream>
#include "GSType.h"

using	namespace std;
/**************************************************************************************************
Copyright (C), 2010-2011, GOSUN 
File name 	: DBACCESSDATADEF.H      
Author		: liujs     
Version 		: Vx.xx        
DateTime 		: 2010/5/28 9:16
Description 	: 数据库模块的相关宏，变量定义模块，公共模块
**************************************************************************************************/
#if 0		//#ifdef _WIN32

// 导入 ado 库 -----------------------------------------------------------
#import "c:\program files\common files\system\ado\msado15.dll" no_namespace rename("EOF","adoEOF")
#include <icrsint.h>

// 消除sprintf 等函数在VS 2008 平台下的告警
#pragma warning(disable:4996)

// ODBC　API 接口，头文件
/*
#include <sql.h> 
#include <sqlext.h>
#include <sqltypes.h>	
#include <odbcinst.h>
#pragma comment(lib,"odbc32.lib")
#pragma comment(lib,"OdbcCP32.Lib")
*/

#else
// linux    数据库相关文件
#include <sql.h>					// ODBC 操作数据库方式
#include <sqlext.h>					// ODBC 方式
#include <sqltypes.h>				// ODBC 数据库类型

// 下面两句测试，linux要删除，在windows时候用
#ifdef _WIN32
#pragma comment(lib,"odbc32.lib")
#pragma comment(lib,"OdbcCP32.Lib")
#else
// linux
#endif

#endif

namespace	DBAccessModule
{
	// 连接池连接数最大(小)值大小
	const	INT					coniConnectionPoolMaxMum    = 20;
	const	INT					coniConnectionBatchNum		= 2;

	// 连接ID最小初始化值
	const	INT					coniDefaultConnectionAutoId = 1; 

	// 定义数据长度，存储数据库服务器地址，数据库名称，用户名，密码的数组的长度
	const	UINT				coniDataLen					= 256;
	// 数值0
	const	UINT				coniZero					= 0;
	// sql 语句长度，
	const	UINT				coniSqlStrLen				= 1024;

	// 插入值主键字段名称
	#define						INSERT_ID_FIELED_NAME		"ID"


	// 缺省的sting类型字符串
	const	string				constrDefaultString			= "";


#if 0	// #ifdef _WIN32
		
#else

	// 类型定义
	#define						SQLRETURN					SQLLEN						// 返回值类型定义 INT

	// SQL 行数定义
	#define						SQL_ROW_NUMBER_ZORE			0							// 没有记录
	// 列的索引错误码
	#define						ERROR_SQL_COLUMN_INDEX		-1							// 错误的字段索引

	// 一般数据的长度(从数据库中获取的时候用的)
	#define 					FIELD_DATA_LEN				8192

	// 名称长度
	#define						MAX_FNAME_LEN				256  

	// 编辑模式
	enum EnumEditMode
	{
		EM_UnKnown,				// 0 未知
		EM_Edit,				// 1 编辑
		EM_AddNew,				// 2 新加
		EM_NoUpdate				// 3 不能更新，例如从多张表查询数据的时候，就会用到
	};

	// 数据库表的字段的名称和索引结构
	typedef	struct StruColumnItem
	{
		// 列名称
		string		strColumn;
		// 列索引
		INT			iColumnIndex;
	}StruColumnItem,*StruColumnItemPtr;

	// 存储列名称和索引的结构列表
	typedef		vector<StruColumnItem>						ColumnItemVector;
	typedef		ColumnItemVector::iterator					ColumnItemVectorIterator;

	// 查找表名称的关键字  COracleRecordSet 和 CSqlServrRecordSet数据集中用到
	const		string			constrFromKeyWord			= "FROM";
	const		string			constrWhereKeyWord			= " WHERE ";
	const		string			constrSelectKeyword			= "SELECT";
	const		string			constrOrderKeyword			= "ORDER";
	const		string			constrGroupKeyWord			= "GROUP";
	const		string			constrBYKeyWord				= "BY";
	const		string			constrDISTINCTKeyWord		= "DISTINCT";


	// 数据集打开超时时间
	const		INT				coniOpenTimeOut				= 3;

	// 游标名称,用于更新的时候用，或者删除的时候
	#define		CURRENT_CURSOR_NAME							"CURRENT_CURSOR_NAME"

	// 游标名称长度%
	#define		MAX_CURSOR_NAME_LEN							64

	// 游标索引取值范围
	const		INT32			coniMaxCursorIndex			= 0x0FFFFFFF;			// 游标索引最大值
	const		INT32			coniMinCursorIndex			= 0x00000001;			// 游标索引最小值


	/**************************************************************************************************
	Function		: ErrorLogInfo(SQLHENV		hEnv,SQLHDBC	hDbc,SQLHSTMT	hStmt)
	DateTime		: 2010/11/30 13:49	
	Description		: 输出错误信息的函数
	Input			: hEnv:数据库环境句柄，hDbc：数据库链接句柄，hStmt：数据库查询语句句柄
	Output			: NULL
	Return			: TRUE
	Note			:
	**************************************************************************************************/
	BOOL	ErrorLogInfo(SQLHENV		hEnv,SQLHDBC	hDbc,SQLHSTMT	hStmt, INT *piErrno = NULL);

#endif



}

#endif // DBACCESSDATADEF_DEF_H
