#ifndef IDBACCESSMODULE_DEF_H
#define IDBACCESSMODULE_DEF_H

/**************************************************************************************************
Copyright (C), 2010-2011, GOSUN 
File name : IDBACCESSMODULE.H      
Author :  liujs      
Version : Vx.xx        
DateTime : 2010/5/22 9:56
Description :	数据库访问模块接口
· 链接对象类	IConnection
· 连接池类		IConnectionPool
· 数据集类		IRecordSet
*************************************************************************************************/

// 系统底层的封装头文件
#include "GSType.h"
#include "GSDefine.h" 
#include "GSCommon.h"


using namespace std;
namespace	DBAccessModule
{
	//----------------------------------------------------------------------------------
	#define ERROR_DBACCESS_START							5000							// 数据库操作访问开始位置

	#define ERROR_DB_CONNECT_ERROR							(ERROR_DBACCESS_START + 1)		// 连接数据库发生异常
	#define	ERROR_DB_CREATE_CONNECTION_ERROR				(ERROR_DBACCESS_START + 2)		// 创建ADO连接出错
	#define ERROR_DB_WRONG_CONNECTION_STR					(ERROR_DBACCESS_START + 3)		// 数据库连接字符串错误

	#define	ERROR_DB_TRANS_RET_ERROR						(ERROR_DBACCESS_START + 4)		// 事务处理异常返回
	#define	ERROR_DB_TRANS_RET_SUCCESS						(ERROR_DBACCESS_START + 5)		// 事务处理正常

	//----------------------------------------------------------------------------------
	// 数据库类型
	enum EnumDatabaseType
	{
		ORACLE = 0,		// oracle 
		SQLSERVER,		// SqlServer
		MYSQL,			// MySql 
		ACCESS,			// Access
		EXCEL,			// Excel数据库
		OCI				//oci
	};

	// 连接对象是否在使用
	enum EnumConnectionUseFlag
	{
		CONNECTION_IDLESSE	= 0,	// 链接空闲
		CONNECTION_OCCUPY	= 1		// 链接占用
	};

	/**************************************************************************************************
	功能说明：数据集对象接口，通过该对象，进行数据集的操作

	功能列表：
	<1>. Eof()
	<2>. MoveLast()
	<3>. MoveNext()
	<4>. GetCollect(const char* szFieldName,string&	szValue)
	<5>. PutCollect(const char* szFieldName,const string&	szValue)
	<6>. GetCollect(const char* szFieldName,INT&	iValue)
	<7>. PutCollect(const char* szFieldName,const INT&	iValue)
	<8>. GetCollect(const char* szFieldName,void*	   pValue,const INT iBuffSize,INT&	iDataLen)
	<9>. PutCollect(const char* szFieldName,const  void*	   pValue,const INT	iDataLen)
	<10>. PutDtCollect(const char* szFieldName,const INT iYear,const INT iMonth,const INT iDay,const INT iHour,const INT iMinute,const INT iSecond)
	<11>. AddNew()
	<12>. AddNew(const	char*	szTableName) 
	<13>. Update()
	<14>. ReleaseRecordSet()
	备	  注： 
	通过该数据集，可以进行数据库的访问操作[添加，删除，修改]等
	*************************************************************************************************/
	class IConnection;
	class IRecordSet
	{
	public:
		IRecordSet(void){};

	public:
		virtual ~IRecordSet(void){};
	public:
		/**************************************************************************************************
		Function		: Eof()    
		DateTime		: 2010/6/8 19:09	
		Description	: 记录集尾部
		Input			: NULL
		Output		: NULL
		Return		: TRUE:记录集尾部 ,FALSE:非尾部
		Note			: NULL
		**************************************************************************************************/
		virtual	BOOL	Eof() = 0;

		/**************************************************************************************************
		Function		: MoveLast()      
		DateTime		: 2010/6/8 19:10	
		Description	: 移向最后一条记录
		Input			: NULL
		Output		: NULL
		Return		: TRUE:操作成功，FALSE:操作失败
		Note			: NULL
		**************************************************************************************************/
		virtual	BOOL	MoveLast() = 0;

		/**************************************************************************************************
		Function		: MoveNext()    
		DateTime		: 2010/6/8 19:11	
		Description	: 移向下一条记录
		Input			: NULL
		Output		: NULL
		Return		: TRUE:操作成功，FALSE:操作失败
		Note			: NULL
		**************************************************************************************************/
		virtual	BOOL	MoveNext() = 0;

		/**************************************************************************************************
		  Function		: AddNew(IConnection*	pConnection,const	char*	szTableName)
		  DateTime		: 2010/11/27 7:47	
		  Description	: 在一个连接上，指定的表上面插入数据
		  Input			: szTableName：表名称
		  Output		: NULL
		  Return		: TRUE:成功
		  Note			:
		  <1> AddNew(const	char*	szTableName)
		  <2> PutCollect()
		  <3> Update()
		**************************************************************************************************/
		virtual	BOOL	AddNew(const	char*	szTableName) = 0;

		/**************************************************************************************************
		Function		: AddNew()    
		DateTime		: 2010/6/8 19:14	
		Description	: 添加一条新的记录
		Input			: NULL
		Output		: NULL
		Return		: TRUE:操作成功，FALSE:操作失败
		Note			:	添加记录：
		<1> 先查询出来，才能操作
		<2> AddNew()
		<3> PutCollect()
		<4> Update()
		**************************************************************************************************/
		virtual BOOL    AddNew() = 0;

		/**************************************************************************************************
		Function		: Update()    
		DateTime		: 2010/6/8 19:14	
		Description	: 更新记录
		Input			: NULL
		Output		: NULL
		Return		: TRUE:操作成功，FALSE:操作失败
		Note			: 更新记录：或参考AddNew 方法
		<1> PutCollect()
		<2> Update()
		**************************************************************************************************/
		virtual BOOL    Update() = 0;

	public:	
		/**************************************************************************************************
		Function		: GetCollect(const char* szFieldName,string&	strValue)     
		DateTime		: 2010/6/8 19:13	
		Description	: 根据列名称获取对应字段的字符串值
		Input			: szFieldName ：   列名称
		Output		: strValue ：       返回的字符串值
		Return		: TRUE:操作成功，FALSE:操作失败
		Note			: 对应的数据类型为：string,float,long,unsigned long ,double
		**************************************************************************************************/
		virtual	BOOL	GetCollect(const char* szFieldName,string&	strValue) = 0;

		/**************************************************************************************************
		Function	: GetBlobCollect(const char* szFieldName,string&	strValue)     
		DateTime	: 2013/8/7 9:13	
		Description	: 根据列名称获取对应二进制字段的数据
		Input		: szFieldName:列名称
		Output		: strValue :返回的字符串值,strValue.size(),做为实际数据大小
		Return		: TRUE:操作成功，FALSE:操作失败
		Note		: 对应的数据类型为：clob,blob
		by			:cuiyt
		**************************************************************************************************/
		virtual	BOOL	GetBlobCollect(const char* szFieldName,string& strValue) = 0;

		/**************************************************************************************************
		Function		: PutCollect(const char* szFieldName,const char*     szValue)    
		DateTime		: 2010/6/8 19:13	
		Description	: 根据列名称设置对应字段的字符串值
		Input			: szFieldName ：   列名称    ,szValue ：       要设置的值
		Output		: NULL
		Return		: TRUE:操作成功，FALSE:操作失败
		Note			: 对应的数据类型为：string,float,long,unsigned long ,double
		**************************************************************************************************/
		virtual	BOOL	PutCollect(const char* szFieldName,const char*     szValue) = 0;

		/**************************************************************************************************
		Function		: GetCollect(const char* szFieldName,INT&	iValue)     
		DateTime		: 2010/6/8 19:13	
		Description	: 根据列名称获取对应字段的INT值
		Input			: szFieldName ：   列名称
		Output		: iValue ：         返回的数值
		Return		: TRUE:操作成功，FALSE:操作失败
		Note			: NULL
		**************************************************************************************************/
		virtual	BOOL	GetCollect(const char* szFieldName,INT&	iValue) = 0;

		/**************************************************************************************************
		Function		: PutCollect(const char* szFieldName,const INT	iValue)     
		DateTime		: 2010/6/8 19:07	
		Description	: 根据列名称设置对应字段的INT值
		Input			: iValue ：         设置的数值
		szFieldName ：   列名称
		Output		: NULL
		Return		: TRUE:操作成功，FALSE:操作失败
		Note			: NULL
		**************************************************************************************************/
		virtual	BOOL	PutCollect(const char* szFieldName,const INT	iValue) = 0;

		/**************************************************************************************************
		Function		: GetCollect(const char* szFieldName,void*	   pValue,const INT iBuffSize,INT&	iDataLen)    
		DateTime		: 2010/6/8 19:05	
		Description	: 根据列名称设置对应字段的void*值
		Input			: szFieldName ：   列名称
		iBuffSize:	   传入获取值的缓存大小
		pValue:	外面调用分配好的内存地址
		Output		: pValue ：         获取的数据
		iDataLen:			获取的数据长度，字节长度
		Return		: TRUE:操作成功，FALSE:操作失败
		Note			: 用于操作二进制大对象的时候
		**************************************************************************************************/
		virtual	BOOL	GetCollect(const char* szFieldName,void*	   pValue,const INT iBuffSize,INT&	iDataLen) = 0;

		/**************************************************************************************************
		Function		: PutCollect(const char* szFieldName,const  void*	   pValue,const INT	iDataLen)   
		DateTime		: 2010/6/8 19:00	
		Description	: 根据列名称设置对应字段的void*值
		Input			: szFieldName ：   列名称
		iDataLen:			设置的数据长度	
		pValue:数据
		Output		: NULL
		Return		: TRUE:操作成功，FALSE:操作失败
		Note			: 用于操作二进制大对象的时候
		**************************************************************************************************/
		virtual	BOOL	PutCollect(const char* szFieldName,const  void*	   pValue,const INT	iDataLen) = 0;

		/**************************************************************************************************
		  Function		: PutDtCollect(const char* szFieldName,const char* szValue)
		  DateTime		: 2010/11/27 7:43	
		  Description	: 根据列名称设置对应时间字段的值
		  Input			: szFieldName：时间列名称，szValue：时间值，要求格式如下[YYYY-MM-DD HH:mm:SS]，例如：2010-11-27 12:12:12
		  Output		: NULL
		  Return		: NULL
		  Note			:
		**************************************************************************************************/
		virtual	BOOL	PutDtCollect(const char* szFieldName,const INT iYear,const INT iMonth,const INT iDay,const INT iHour,const INT iMinute,const INT iSecond) = 0;

		/**************************************************************************************************
		Function		: ReleaseRecordSet    
		DateTime		: 2010/6/1 18:47	
		Description	: 内部释放数据集对象
		Input			: NULL
		Output		: NULL
		Return		: TRUE:成功，FALSE:失败
		Note			: 使用完成IRecordSet对象后，调用
		调用该函数后，自身对象就不存在了，不能在进行调用对象的相关方法了
		只是接口定义才这么使用，否则不推荐使用delete this
		调用该函数后，将指针赋值为 ： NULL
		**************************************************************************************************/
		virtual	BOOL	ReleaseRecordSet() = 0;

		/*
		 *********************************************
		 Function : GetColumnNumber
		 DateTime : 2011/9/20 14:22
		 Description :  返回记录集的列数
		 Input :  
		 Output : 
		 Return : 出错返回-1
		 Note :   
		 *********************************************
		 */
		virtual INT GetColumnNumber(void) = 0;

		/*
		 *********************************************
		 Function : GetCloumnName
		 DateTime : 2011/9/20 14:24
		 Description :  获取列名
		 Input :  iColIndex 列索引 有 0 开始
		 Output : oStrName 返回的列名
		 Return : TRUE/FALSE
		 Note :   
		 *********************************************
		 */
		virtual BOOL GetCloumnName(INT iColIndex, std::string &oStrName ) = 0;

	};


	/**************************************************************************************************
	功能说明：提供获取数据库连接对象的接口
	功能列表：
	<1>. ExecuteSql( const char*  szSql)
	<2>. ExecuteSql(const char*  szSql,INT64& lRowId)
	<3>. ExecuteQuery(const char*  szSql)
	<4>. ReleaseConnection()
	<5>. BeginTrans()
	<6>. RollbackTrans()
	<7>. CommitTrans()
	<8>. ExecutePageQuery(const char*  szSql,const INT32	iStartRow,const INT32 iRowNum)
	<9>. GetEmptyRecordSet()
	<10>.ToTime(const char*	szDateTime)
	<11>.ToDate(const char*	szDateTime)
	<12>.ToDateTime(const char*	szDateTime)
	备	  注： 值提供一个OuterInterface，提供执行相关操作的方法

	<注意>. 由于各种数据库接口的不同，所有输入sql语句必须遵循如下原则：
	<1>. 输入标准SQL 语句的时候，要求全部大写，而且注意不能使用 * ,例如 SELECT * FROM TABLE
	<2>. SQL 语句中不能存在关键字字段，例如 SELECT USER_ID AS ID FROM TABLE等
	<3>. 数据库设计的是，必须避免使用关键字，尤其是ORACLE数据库。
	<4>. 数据库设计所有表名称，字段名称，必须大写

	*************************************************************************************************/
	class IConnection
	{
	protected:
		IConnection(void){};
		virtual ~IConnection(void){};

	public:
		/**************************************************************************************************
		  Function		: GetEmptyRecordSet()
		  DateTime		: 2010/11/29 17:50	
		  Description	: 从练接对象获取一个空的数据集
		  Input			: NULL
		  Output		: NULL
		  Return		: NULL
		  Note			: 插入数据的时候用
		**************************************************************************************************/
		virtual	IRecordSet*		GetEmptyRecordSet()	= 0;

		/**************************************************************************************************
		Function		: ReleaseConnection()    
		DateTime		: 2010/6/8 18:10	
		Description	: 释放连接对象接口，将使用完毕后的连接对象，放回到连接池中
		Input			: NULL
		Output		: NULL
		Return		: TRUE:操作成功，FALSE:操作失败
		Note			: NULL
		**************************************************************************************************/
		virtual	BOOL			ReleaseConnection() = 0 ;

		/**************************************************************************************************
		Function		: ExecuteSql(const char*  szSql)     
		DateTime		: 2010/6/8 18:01	
		Description	: 执行Sql语句
		Input			: strSql：            标准的sql语句
		Output		: TRUE
		Return		: TRUE:操作成功，FALSE:操作失败
		Note			: 通过标准sql语句，执行sql操作[insert,update,delete]
		**************************************************************************************************/
		virtual	BOOL			ExecuteSql(const char*  szSql) = 0;

		/**************************************************************************************************
		Function		: ExecuteSql(const char*  szSql,const char* szTable,INT64& lRowId)  
		DateTime		: 2010/6/8 17:58	
		Description	: 执行Sql语句
		Input			: strSql：            标准的sql语句
		: szTable:			  操作的表名
		Output		: lRowId:             插入操作，对应的影响行数的ID
		Return		: TRUE:操作成功，FALSE:操作失败
		Note			: 通过标准sql语句，执行sql操作[insert]
		**************************************************************************************************/
		virtual	BOOL			ExecuteSql(const char*  szSql,const char* szTable,INT64& lRowId) = 0;

		/**************************************************************************************************
		Function		: ExecuteQuery(const char*  szSql)    
		DateTime		: 2010/6/8 17:56	
		Description	: 通过连接对象和sql查询语句，返回数据集对象
		Input			: strSql：            标准的sql语句
		Output		: NULL
		Return		: IRecordSet* 对象指针，进行相应的操作，
		Note			: 通过标准sql语句，执行sql操作[select]
		**************************************************************************************************/
		virtual	IRecordSet*		ExecuteQuery(const char*  szSql) = 0;


		/**************************************************************************************************
		Function		: ExecutePageQuery(const char*  szSql,const INT32	iStartRow,const INT32 iRowNum)
		DateTime		: 2011/01/2 10:56	
		Description	: 通过连接对象和sql查询语句，返回数据集对象
		Input			: strSql：            标准的sql语句,SQL SERVER 只支持单表或者单个视图，如果ORACLE支持多表或者视图联合
						  需要分页查询，最好都事先建立好视图，在统一查询
						  iStartRow:		  开始行
						  iRowNum：			  行数目
		Output			: NULL
		Return			: IRecordSet* 对象指针，进行相应的操作，
		Note			: 通过标准sql语句，执行sql操作[select]
		**************************************************************************************************/
		virtual	IRecordSet*		ExecutePageQuery(const char*  szSql,const INT32	iStartRow,const INT32 iRowNum) = 0;	

		/**************************************************************************************************
		Function		: BeginTrans()    
		DateTime		: 2010/6/8 17:49	
		Description	: 开始执行事物
		Input			: NULL
		Output		: NULL
		Return		: TRUE:成功，FALSE:失败
		Note			:			
		**************************************************************************************************/
		virtual	UINT			BeginTrans() = 0;

		/**************************************************************************************************
		Function		: RollbackTrans()    
		DateTime		: 2010/6/8 17:49	
		Description	: 事务回滚处理
		Input			: NULL
		Output		: NULL
		Return		: TRUE:成功 ， FALSE:失败
		Note			:
		**************************************************************************************************/
		virtual	BOOL			RollbackTrans() = 0;

		/**************************************************************************************************
		Function		: CommitTrans()    
		DateTime		: 2010/6/8 17:49	
		Description	: 提交事务
		Input			: NULL
		Output		: NULL
		Return		: TRUE:成功，FALSE:失败
		Note			: 
		**************************************************************************************************/
		virtual	BOOL			CommitTrans() = 0;

		/**************************************************************************************************
		  Function		: ToTime,ToDate,ToDateTime
		  DateTime		: 2010/12/9 17:25	
		  Description	: 将字符串转换成标准的数据库的时间字符串
		  Input			: NULL
							ToTime:HH:MI:SS
						    ToDate:YYYY-MM-DD
							ToDateTime:YYYY-MM-DD HH:MI:SS
		  Output		: 
							ToTime:HH:MI:SS
							ToDate:YYYY-MM-DD
							ToDateTime:YYYY-MM-DD HH:MI:SS
		  Return		: NULL
		  Note			: 不同的数据库，不一样的用法，例如：sql server 不用转换，ORACLE需要转换处理
						  注意：外部使用，无需再添加[']和[']
		**************************************************************************************************/
		virtual	string			ToTime(const char*	szDateTime) = 0;
		virtual	string			ToDate(const char*	szDateTime) = 0;
		virtual	string			ToDateTime(const char*	szDateTime) = 0;
	};


	/**************************************************************************************************
	功能说明：数据库连接对象池
	功能列表：
	<1>. GetConnectionHandle(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass,const INT iDbType)

	备	  注： 
	数据库连接对象池
	*************************************************************************************************/
	class IConnectionPool 
	{
	protected:
		IConnectionPool(void){};
	public:
		virtual ~IConnectionPool(void){};

	public:
		/**************************************************************************************************
		Function		: GetConnection(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass,const INT iDbType)    
		DateTime		: 2010/6/8 17:52	
		Description	: 返回连接对象接口,从连接池中取出一个已存在连接，如果都在使用返回NULL，
		Input			:     
		szServer ：   数据库服务器地址
		szDatabase：  数据库名称
		szUser  ：    数据库用户名
		szPass  ：    数据库密码
		iDbType ：     数据库类型[sql ,oracle 等]
		Output		: NULL
		Return		: IConnection* 连接对象接口指针
		Note			: NULL
		**************************************************************************************************/
		virtual	IConnection*	GetConnection(const	char*   szServer,const	char*   szDatabase,const	char*   szUser,const	char*   szPass,const INT iDbType) = 0 ;


		

	};


	/**************************************************************************************************
	Function			: StartDBAccessModule    
	DateTime			: 2010/5/23 10:51	
	Description		: 启动数据库模块
	Input				: NULL
	Output			: NULL
	Return			: TRUE:成功  FALSE:失败
	Note				: 提供启动数据库模块服务，该函数不提供外部调用者使用
	　		  只提供底层服务启动的启动接口
			  *************************************************************************************************/
	extern	BOOL	StartDBAccessModule();

	/**************************************************************************************************
	Function			: GetConnectionPoolInstance
	DateTime			: 2010/5/23 10:15	
	Description		: 获取连接池对象
	Input				: NULL
	Output			: NULL
	Return			: IConnectionPool*指针对象
	Note				: 返回的连接池对象满足单例实例
	前提条件：StartDBAccessModule成功，底层是单例，和StartDBAccessModule 的功能一样的
	*************************************************************************************************/
	extern 	IConnectionPool*	GetConnectionPoolInstance();


	extern IConnectionPool* CreateConnectionPool();


	/**************************************************************************************************
	Function			: StopDBAccessModule    
	DateTime			: 2010/5/23 10:51	
	Description		: 停止数据库模块服务，并内部释放所有的资源
	Input				: NULL
	Output			: NULL
	Return			: TRUE : 成功，FALSE:失败
	Note				: 该函数不提供外部调用者使用，只提供底层服务退出的退出接口
	*************************************************************************************************/
	extern	BOOL	StopDBAccessModule();

}


#endif // IDBACCESSMODULE_DEF_H

