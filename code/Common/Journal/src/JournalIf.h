/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : JOURNALIF.H
Author :  zouyx
Version : 0.0.0.0
Date: 2011/9/6 13:56
Description: 定义日志插件的接口
********************************************
*/

#ifndef _GS_H_JOURNALIF_H_
#define _GS_H_JOURNALIF_H_

#include "CMD_PROTOCOL_DEF.H"


#ifdef _WIN32

#ifdef JOU_API_EXPORT

#define JOU_API EXPORT_API

#else 

#define JOU_API IMPORT_API

#endif

#else

#define JOU_API

#endif


#define JOU_OPERATOR_ID_LEN  MAX_NAME_LEN   //操作员ID 长度
#define JOU_DESCRI_LEN 128		 //描述字符长度
#define JOU_HOSTNAME_LEN 128	 //计算机IP/名 长度
#define  JOU_OPERATOR_CONTENT_LEN 256

/*
*********************************************************************
*
*@brief : 日志类型
*
*********************************************************************
*/
typedef enum
{
	eJOU_TYPE_OPERATOR = 0, //操作日志
	eJOU_TYPE_RUNSTATUS, //运行日志
	eJOU_TYPE_LOGIN,	//登陆日志
}EnumJournalType;

/*
*********************************************************************
*
*@brief : 操作日志
*
*********************************************************************
*/
typedef struct _StruJouOperation
{
	INT32 iCliPmsID;						//客户端PMS ID
	INT32 iCliID;							//客户端ID
	INT32 iCmdSectionID;					//命令SectionID


	char OperatorID[JOU_OPERATOR_ID_LEN];  //操作人员工号
	char czHostName[JOU_HOSTNAME_LEN];     //客户端IP
	CmdProtocolDef::EnumClientType eClientType;			   //被操作设备类型
	INT32 iPmsID;                          //被操作设备平台
	INT32 iDevID;				           //被操作设备ID， 如果是服务为服务ID
	INT32 iChn;							   //被操作设备通道好， 无效为-1
	CmdProtocolDef::EnumChannelType	    eChnType;		   //设备类型, 如果iChn无效， 该值也无效
	char czContent[JOU_OPERATOR_CONTENT_LEN];  //操作内容
	INT32 eResult; //结果
	char  czFailure[JOU_DESCRI_LEN]; //失败原因

	
}StruJouOperation;


/*
*********************************************************************
*
*@brief : 运行状态日志
*
*********************************************************************
*/
typedef struct _StruJouRunStatus
{
	CmdProtocolDef::EnumClientType eClientType;			//设备类型
	INT32 iPmsID;						//设备所属平台
	INT32 iDevID;						//设备ID， 如果是服务为服务ID	
	INT32 iChnID;						// 通道ID
	INT32 iChnType;						// 通道类型
	INT32 iOnline;						// 在线标志
	char czContent[128];  //操作内容
	char  czDescri[JOU_DESCRI_LEN];		//描述
}StruJouRunStatus;


/*
*********************************************************************
*
*@brief : 登陆日志
*
*********************************************************************
*/
typedef struct _StruJouLogin
{

	char OperatorID[JOU_OPERATOR_ID_LEN];  //操作人员工号	
	char czHostName[JOU_HOSTNAME_LEN];	   //登陆的IP
	char czContent[128];					//操作内容
	INT32 eResult;						   //结果
	char czDescri[JOU_DESCRI_LEN];		   //操作描述
	char  czFailure[JOU_DESCRI_LEN];	   //失败原因
}StruJouLogin;


/*
*********************************************************************
*
*@brief : 日志信息
*
*********************************************************************
*/
typedef struct _StruJournalInfo
{
	EnumJournalType eType; //日志类型
	CmdProtocolDef::StruDateTime stTime;   //发生时间
	INT64 iSeqID; // 内部使用
	union
	{
		StruJouLogin stLogin;  //登陆日志
		StruJouRunStatus stRunStatus; //运行日志
		StruJouOperation stOperation; //操作日志		
	}unLog;	 //日志的具体内容， 由eType 确定
}StruJournalInfo;


/*
*********************************************************************
*
*@brief :  查询参数
*
*********************************************************************
*/
#define JOU_FUNC_NAME_LEN 16  //功能函数名
#define JOU_ARG_LEN  1024     //参数长度



typedef struct _StruQueryArgs
{
	UINT32 iCliPmsID;  //发起查询的客户端所属的PMS
	UINT32 iCliID;	  //发起查询的客户端ID
	UINT32 iCmdTag;	  //本次查询的命令号
	void *pChn;

	UINT32 iRowStart; //指定返回结果的开始行号， 由1 开始
	UINT32 iPageRows; //每页结果条数, 0 不指定由程序指定默认 为 1000, 最大不能超过 10000

	char czFuncName[JOU_FUNC_NAME_LEN]; //查询功能， 统一用小写，使用字符
	char czArgs[JOU_ARG_LEN]; //查询参数， 参数使用  ';' 分割
}StruQueryArgs;


/*
*********************************************************************
* 日期格式 YYYY-MM-DD HH:MM:SS  如 2011-09-17 23:23:06
*@brief : 默认查询功能函数
*
****操作日志
返回值 : ID,操作时间,操作员,设备,通道,操作内容 

* 操作日志按操作时间段查询:
查询功能名字：  op_tm    参数： 开始时间;结束时间; asc(按操作时间升序)|[desc  (按操作时间降序)]

* 操作日志按操作员:
查询功能名字：  op_user    参数：操作员;开始时间;结束时间; asc(按操作时间升序)|[desc  (按操作时间降序)] 

* 操作日志按设备名:
查询功能名字：  op_devname    参数：设备名;开始时间;结束时间; asc(按操作时间升序)|[desc  (按操作时间降序)]

* 按设备ID
查询功能名字：  op_devid    参数：设备ID;通道ID;开始时间;结束时间; asc(按操作时间升序)|[desc  (按操作时间降序)]

* 按设备通道
查询功能名字：  op_devchn    参数：设备ID;通道ID;通道类型;开始时间;结束时间; asc(按操作时间升序)|[desc  (按操作时间降序)]

* 按域名
查询功能名字：  op_domname    参数：开始时间;结束时间;域名; asc(按操作时间升序)|[desc  (按操作时间降序)]

* 按域ID
查询功能名字：  op_domid    参数：开始时间;结束时间;域ID; asc(按操作时间升序)|[desc  (按操作时间降序)]


3. 操作日志多条件:
查询功能名字：  op_multi1    参数：操作员;设备名;开始时间;结束时间; asc(按操作时间升序)|[desc  (按操作时间降序)]
查询功能名字：  op_multi2    参数：操作员;设备ID;通道ID;通道类型;开始时间;结束时间; asc(按操作时间升序)|[desc  (按操作时间降序)]
查询功能名字：  op_multi3    参数：操作员;开始时间;结束时间;域名; asc(按操作时间升序)|[desc  (按操作时间降序)]
查询功能名字：  op_multi4    参数：操作员;设备ID;开始时间;结束时间; asc(按操作时间升序)|[desc  (按操作时间降序)]
查询功能名字：  op_multi5    参数：操作员;开始时间;结束时间;域ID; asc(按操作时间升序)|[desc  (按操作时间降序)]


****登录日志
返回值 : ID,时间,操作员,登录IP,操作内容,结果,错误原因

1. 登录日志按发生时间段查询:
查询功能名字：  login_tm    参数： 开始时间;结束时间; asc(按发生时间升序)|[desc  (按发生时间升序)]

2. 登录日志操作员:
查询功能名字：  login_user    参数：操作员;开始时间;结束时间; asc(按发生时间升序)|[desc  (按发生时间升序)] 

****运行日志
返回值 : ID,时间,设备类型,设备ID,内容

1. 运行日志按发生时间段查询:
查询功能名字：  run_tm    参数： 开始时间;结束时间; asc(按发生时间升序)|[desc  (按发生时间升序)]



*********************************************************************
*/

/*
*********************************************************************
*
*@brief :  日志模块返回值
*
*********************************************************************
*/
typedef enum
{
	eJOU_R_SUCCESS = 0,
	eJOU_E_UNKNOWN, 
	eJOU_E_NINIT,   //没有初始化
	eJOU_E_NJOUTYPE, //不存在的日志类型
	eJOU_E_INVALID,  //非法参数
	eJOU_E_NFUNCTION, //不支持该功能
	eJOU_E_BUSY,	//系统繁忙
	eJOU_E_NMEM, //系统资源匮乏

	eJOU_E_DB_MODULE, //初始化数据库模块失败
	eJOU_E_DB_CONNPOOL, //初始化数据库连接池失败
	eJOU_E_DB_GETCONN, //获取数据库连接失败
	eJOU_E_DB_EXESQL,		//数据库执行SQL错误
	eJOU_E_DB_ASSERT, //数据库执行发送异常

}EnumJouErrno;


/*
*********************************************************************
*
*@brief :  查询结果返回值
*
*********************************************************************
*/
typedef struct _StruQueryResult
{
	UINT32 iCliPmsID;  //发起查询的客户端所属的PMS
	UINT32 iCliID;	  //发起查询的客户端ID
	UINT32 iCmdTag;	  //本次查询的命令号
	void *pChn;

	EnumJouErrno    eResult;   //结果
	//如果eResult失败，以下值无用
	UINT32 iTotals;		//总的记录数
	UINT32 iRowStart;  //当前结果集开始的行号， 由 1 开始
	UINT32 iRows;      //当前结果的行数
	INT  iResultSize;  // 结果数据的pResultData 大小，
	void *pResultData; // 结果数据, 内容格式为 XML
}StruQueryResult;


/*
 *********************************************
 Function : JouGetError
 DateTime : 2011/9/6 15:11
 Description :  返回错误描述
 Input :  
 Output : 
 Return : 返回错误描述字符串
 Note :   
 *********************************************
 */
JOU_API const char * JouGetError( EnumJouErrno eErrno);


/*
*********************************************
Function : LogModuleInit
DateTime : 2011/9/6 9:19
Description :  初始化日志模块
Input :  czConfFilename 配置文件名称
Input :  pConnectionPoolArgs 连接池参数，对象为 StruConnectPoolArgs
Output : 
Return : 参考 EnumJouErrno 定义
Note :   要使用日志功能首先要调用该函数
*********************************************
*/
typedef struct _StruConnectPoolArgs
{
	void *pConnectPool; //连接池
	const char *szServer;  //数据库机器IP
	const char *szDatabase; //数据库名
	const char *szUser;  //用户名
	const char *szPWD;  //密码
	INT  eDbaseType; //EnumDatabaseType
}StruConnectPoolArgs;

JOU_API EnumJouErrno  JouModuleInit( const char *czConfFilename,void* pConnectionPoolArgs);

/*
*********************************************
Function : LogModuleUnint
DateTime : 2011/9/6 9:19
Description :  卸载载日志模块
Input :  
Output : 
Return : 无
Note :  在程序退出时一定要调用， 否则可能有缓冲的数据给丢失
*********************************************
*/
JOU_API void  JouModuleUnint(void);

/*
 *********************************************
 Function : JouAdd
 DateTime : 2011/9/6 15:24
 Description :  写日志信息
 Input :  pLog 要写的信息描述
 Output : 
 Return : 参考 EnumJouErrno 定义
 Note :   采用异步方式， 信息放到缓冲后将返回
 *********************************************
 */
JOU_API EnumJouErrno JouAdd( const StruJournalInfo *pLog);

/*
 *********************************************
 Function : JouDelete
 DateTime : 2011/9/6 15:25
 Description : 删除日志记录 
 Input :  eType 日志类型
 Input :  iKeyID 日志的自增长ID
 Input :  czFuncName 调用的功能函数, 统一用小写，使用字符
 Input :  czArgs 调用函数的参数，采用';' 分割
 Output : 
 Return : 参考 EnumJouErrno 定义
 Note :   
 *********************************************
 */
JOU_API EnumJouErrno  JouDelete(  EnumJournalType eType, INT64 iKeyID);
JOU_API EnumJouErrno  JouDeleteExt( const char czFuncName[JOU_FUNC_NAME_LEN],
								   const char czArgs[JOU_ARG_LEN] );


/*
 *********************************************
 Function : JouUpdate
 DateTime : 2011/9/6 15:25
 Description :  更新操作日志状态
 Input :  eType 被更新的日志类型
 Input :  iKeyID 被更新日志的自增长ID 
 Input :  pNewLog 新的日志信息
 Output : 
 Return :  参考 EnumJouErrno 定义
 Note :   日志可以被更新吗 ????
 *********************************************
 */
typedef struct _StruJouOperationUpdate
{
	INT32 iCliPmsID;						//客户端PMS ID
	INT32 iCliID;							//客户端ID
	INT32 iCmdSectionID;					//命令SectionID

	INT32 eResult; //结果
	char  czFailure[JOU_DESCRI_LEN]; //失败原因
}StruJouOperationUpdate;

JOU_API EnumJouErrno  JouUpdateOperation( const StruJouOperationUpdate *pData );



/*
 *********************************************
 Function : JouAsyncQuery
 DateTime : 2011/9/6 15:51
 Description :  查询接口
 Input :  pArgs 查询的参数项
 Output : 
 Return : 参考 EnumJouErrno 定义
 Note :   使用的异步查询方式, 结果将在 JouSetAsyncQueryCallback 设定回调返回
 *********************************************
 */
JOU_API EnumJouErrno  JouAsyncQuery( const StruQueryArgs *pArgs );


/*
 *********************************************
 Function : JouFuncPtrAsyncQueryCallback
 DateTime : 2011/9/6 15:51
 Description :  查询结果的回调函数定义
 Input :  
 Output : pResult 返回的结果，参考 StruQueryResult
 Output : pUserContent 用户的上下文
 Return : 无
 Note :   
 *********************************************
 */
typedef void   JouFuncPtrAsyncQueryCallback(  const StruQueryResult *pResult,  
											void *pUserContent );
/*
 *********************************************
 Function : LogSetAsyncQueryCallback
 DateTime : 2011/9/6 15:51
 Description :  设定异步查询的回调函数
 Input :  fnCallback 回调函数， 参考 JouFuncPtrAsyncQueryCallback定义
 Input :  pUserContent 用户上下文数据
 Output :  
 Return : 参考 EnumJouErrno 定义
 Note :   
 *********************************************
 */
JOU_API EnumJouErrno  JouSetAsyncQueryCallback( JouFuncPtrAsyncQueryCallback fnCallback, 
											   void *pUserContent );

#endif //end _GS_H_JOURNALIF_H_


