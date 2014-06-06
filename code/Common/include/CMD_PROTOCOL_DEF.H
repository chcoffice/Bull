#ifndef CMD_PROTOCOL_DEF_DEF_H
#define CMD_PROTOCOL_DEF_DEF_H

/**************************************************************************************************
  Copyright (C), 2010-2011, GOSUN 
  File name 	: CMD_PROTOCOL_DEF.CPP      
  Author 		: Liujs      
  Version 		: Vx.xx        
  DateTime 		: 2010/7/29 10:36
  Description 	: 传输协议定义格式
**************************************************************************************************/
#include "TableDataType.h"		// 基础数据类型

// 字节对齐大小
#define		PRAGMA_PACK_SIZE		1
#ifdef _WIN32
#define		ATTRIBUTE_PACKED 
#pragma		pack(push,PRAGMA_PACK_SIZE )
#else
#define ATTRIBUTE_PACKED    __attribute__ ((packed)) 
#endif

#define			MAX_NAME_LEN				128			// 名称最大长度
#define			MAX_NAME_LEN_256			256			// 名称最大长度
#define			MAX_PWD_LEN					128			// 登陆密码最大长度
#define			MAX_CONN_LEN				128			// 登陆通道最大长度
#define			MAX_VERSION_LEN				32			// 版本号最大长度
#define			MAX_IP_ADDR_LEN				16			// IP地址最大长度
#define			MAX_REMARKS_LEN				256			// 备注信息最大长度
#define			MAX_CHANNEL_NUM				128			// 最大通道个数
#define			MAX_TYPE_NUM				16			// 每一次最大类型个数[告警类型，设备类型，厂商类型等]
#define			MAX_URI_LEN					512			// 最大点流结果：URI长度	
#define			MAX_DEV_STATUS_NUM			256			// 设备状态的设备个数最大值
#define			MAX_DEV_NUM					1			// 设备最大数据,动态处理
#define			MAX_ALARM_ITEM_NUM			1			// 最大告警节点个数，一个包的个数
#define			MAX_TIME_SPAN_NUM			5			// 时间段个数
#define			MAX_WEEK_NUM				7			// 星期个数
#define			MAX_NAME_DAY_NUM			20			// 指定日期录像天数
#define			MAX_LINKAGE_EVENT_NUM		10			// 最大联动动作个数
#define			MAX_NODE_LEN				64			// 设备数节点长度
#define			MAX_PRIV_LEN				16			// 最大权限长度
#define         MAX_ID_STRING_LEN			64          // 最大ID字符串长度
#define			MAX_DISK_PARTITION_LEN		32			// 盘符的最大长度
#define			MAX_NUMBER_STRING_LEN		32			// 最大编号长度
#define			MAX_DATA_VERSION_NUM		22			// 最大数据版本类型个数
#define         MAX_DISK_LABEL_LEN          256          //盘符字符串最大长度
#define         MAX_SPLIT_LEN               1024         //电视墙分屏屏幕编号字符串长度



// 协议命名空间
namespace	CmdProtocolDef
{
	class Packet;//前向声明
	// 通信方式结构定义
	typedef	enum		EnumCommuType
	{
		COMMU_TYPE_TCP	= 1,								// TCP 通信方式
		COMMU_TYPE_UDP,										// UDP 通信方式
		COMMU_TYPE_COM										// 串口通信方式							
	}EnumCommuType;

	//什么操作.
	typedef		enum	EnumUpdateType
	{
		ENUM_ADD_OPT = 0,
		ENUM_DEL_OPT,
		ENUM_EDIT_OPT,
	}EnumUpdateType;

	/**************************************************************************************************
		CMD_ID		: EnumClientType
		CMD_NAME	: 所有服务类型定义
		DateTime	: 2010/11/3 13:46	
		Description	: 登陆客户端类型定义
		Author		: Liujs
		Note		: NULL
	**************************************************************************************************/
	typedef		enum	EnumClientType
	{
		CLIENT_TYPE_PMS = 101,							// PMS
		CLIENT_TYPE_DAS	,								// DAS
		CLIENT_TYPE_STS	,								// STS
		CLIENT_TYPE_LMS ,								// LMS
		CLIENT_TYPE_AMS ,								// AMS
		CLIENT_TYPE_CSS ,								// CSS
		CLIENT_TYPE_CLI ,								// CLI
		CLIENT_TYPE_CMU ,								// CMU
		CLIENT_TYPE_PU	,								// PU
		CLIENT_TYPE_VMS ,								// VMS
		CLIENT_TYPE_USER,								// USER
		CLIENT_TYPE_CMS = 120,								// CMS
		CLIENT_TYPE_BMS = 130,								// BMS
		CLIENT_TYPE_IVS =  140,								// IVS，智能分析服务
		CLIENT_TYPE_MCLI = 150 ,		                    //手机客户端
	}EnumClientType;

	/**************************************************************************************************
	CMD_ID		: EnumClientType
	CMD_NAME	: 通道类型定义
	DateTime	: 2010/11/3 13:46	
	Description	: 通道类型定义
	Author		: Liujs
	Note		: NULL
	**************************************************************************************************/
	typedef		enum	EnumChannelType
	{
		CHANNEL_TYPE_VIDEO = 101,							// 视频通道类型
		CHANNEL_TYPE_INPUT	,								// 告警输入类型
		CHANNEL_TYPE_OUTPUT									// 告警输出类型
	}EnumChannelType;


	/**************************************************************************************************
		CMD_ID		: EnumErrorCode
		CMD_NAME	: 所有的错误代码的描述
		DateTime	: 2010/11/3 11:28	
		Description	: 所有的错误代码的描述,命令操作结果类型，错误代码
		Author		: Liujs
		Note		: NULL
	**************************************************************************************************/
	typedef	enum		EnumErrorCode
	{
		//--------------------------------------------------------------------
		// 通用操作成功，失败
		OPER_RESULT_SUCCESS				= 0,				// 操作成功
		OPER_RESULT_FAIL				,					// 操作失败
		OPER_UNKNOW_ERROR				,					// 未知错误
		

		//--------------------------------------------------------------------
		// 登陆返回结果
		LOG_RESULT_USER_NAME_ERROR		,					// 用户名不存在	
		LOG_RESULT_USER_NOTENABLE       ,                   //用户被禁用  
		LOG_RESULT_PWD_ERROR			,					// 密码错	
		LOG_RESULT_HAS_EXIST			,					// 登录ID已存在	
		LOG_RESULT_SERVICE_LOG_FULL		,					// 服务器容量达到极限
		LOG_RESULT_ROLE_NOTEXIST		,					// 角色不存在
		LOG_RESULT_ROLE_INVALID			,					// 角色时间段无效
		LOG_RESULT_FORBID_REPEATLOGIN	,					// 不允许该用户重复登录
		LOG_RESULT_IPADDRESS_ERROR		,					// IP地址不在有效范围内
		LOG_RESULT_SINGLE_SIGN_ON_FAIL	,					// 免登录失败


		//--------------------------------------------------------------------
		// 点流返回结果
		GET_STREAM_NO_STREAM			,					// 流不存在，PMS点STS的时候，如果不存在就返回该结果
		GET_STREAM_NO_DEVICE			,					// 设备不存在，或者不可以使用
		GET_STREAM_NO_CHANNEL			,					// 设备对应通道不存在
		GET_STREAM_TIME_OUT				,					// 点流超时
		GET_STREAM_NO_DEV_DAS			,					// 设备对应的DAS服务没有登陆
		GET_STREAM_WAIT_ACK             ,                   // 点流等待处理保持连接

		//--------------------------------------------------------------------
		// 获取数据版本结果
		GET_DATA_VERSION_FAIL			,					// 获取数据版本失败

		//--------------------------------------------------------------------
		// 平台录像回复结果
		GET_SER_RECORD_FAIL				,					// 平台录像规则获取失败

		//--------------------------------------------------------------------
		// 获取预置点信息
		GET_PTZ_PRESET_FAIL				,					// 获取预置点失败

		//--------------------------------------------------------------------
		// 告警相关代码
		GET_ALARM_RULE_FAIL				,					// 告警规则获取失败
		ALARM_LINKAGE_EVENT_OK			,					// 告警联动事件通知OK
		ALARM_LINKAGE_EVENT_ERROR		,					// 告警联动事件通知错误

		//--------------------------------------------------------------------
		// 用户权限
		GET_USER_RIGHT_FAIL				,					//获取用户权限失败
		USER_NO_OPERATOR_RIGHT			,					//用户无操作权限

		//--------------------------------------------------------------------
		// 录像文件操作
		FIND_TRANSFER_CHN_ERROR			,					// 查找传输通道错误
		PACKET_LEN_CHECK_ERROR			,					// 包长度校验错误
		QUERY_PARAM_ERROR	,								// 查询参数错非法
        QUERY_DBASE_ERROR,                                  //数据库操作失败
        QUERY_STROAGE_ERROR,								//不支持的存储类型查找参数
        PLAYBACK_VERSION_ERR,								//不支持的版本类型
        PLAYBACK_FILE_NEXIST,                               //文件不存在
        PLAYBACK_FILE_NOPER,                                //文件不可以操作
        

		//--------------------------------------------------------------------
		//布防撤防相关代码
		GET_DEPLOYMENT_DENFENCE_RULE_FAIL,					//获取布防撤防规则失败
		DEPLOYMENT_DEFENCE_COMMAND_NOTICE_OK,				//布防撤防命令通知成功
		DEPLOYMENT_DEFENCE_COMMAND_NOTICE_FAIL,				//布防撤防命令通知失败

		DATA_PACKET_ILLEGAL,								// 数据包错误
		INVALID_PARAM,										// 参数非法
		FOUND_NO_TRANSMIT_CHANNEL,							// 无法找到传输通道
		FOUND_DATA_FAIL,									// 无法找到相应数据

		OPERATE_LOCK_MONOLIZE,								//操作被独占控制锁定
		
		TRANSMIT_PACKET_SEND_FAIL,							//转发数据包发送失败
		PARAM_NOT_CONFIG,									//参数未配置

		DATABASE_OPER_FAIL,									//数据库操作失败

		//电视墙
		MATRIX_UNABLE_DECODE,				

		PMS_NO_INTERCASCADE,									//平台不允许相互级联

		
		//---------------  录像返回
		RET_RECORD_DEVICE_NO_FUNCTION,						//设备不支持该功能
		RET_RECORD_DATABASE_ERROR,							//数据库操作失败
		RET_RECORD_NO_DISK,					//"没有存储器"
		RECORD_REVIEW_DOWNLOAD_ACCESS_MAX,                  //录像回放/下载接入数已达到最大值

		DEVICE_NOT_SUPPORT,						// 设备不支持该功能

		PLATFORMID_ERROR,						// 平台ID错误		
		USER_NO_PRIORITY,                       //用户优先级不够
		CMS_OFFLINE,						// CMS不在线
		DOWN_BMS_OFFLINE,					// 下级BMS不在线

		DEVICE_OFFLINE,                     // 设备不在线
		OVER_MAX_NUM,                   // 超过最大值

		USER_RIGHTGRADE_LOW,		//权限等级过低.
	}EnumErrorCode;

	// 获取码流成功
	#define		GET_STREAM_SUCCESS		OPER_RESULT_SUCCESS
	// 登陆成功
	#define		LOG_RESULT_SUCCESS		OPER_RESULT_SUCCESS


	// 返回值信息结构
	typedef		struct	StruRetInfo 
	{
		INT32			iRetCode;
		std::string		strRetInfo;
	}StruRetCodeInfo;

	// 错误代码的返回值
	const		StruRetCodeInfo		conStRetInfo[]={
		{	OPER_RESULT_SUCCESS,									"操作成功"},
		{	OPER_RESULT_FAIL,										"操作失败"},
		{	OPER_UNKNOW_ERROR,										"未知错误"},
		{   DEVICE_NOT_SUPPORT,								 "设备不支持该功能"},

		//--------------------------------------------------------------------
		// 登陆返回结果
		{	LOG_RESULT_USER_NAME_ERROR,								"用户名不存在"},
		{	LOG_RESULT_USER_NOTENABLE,								"用户被禁用"},
		{	LOG_RESULT_PWD_ERROR,									"密码错误"},
		{	LOG_RESULT_HAS_EXIST,									"登录ID已存在"},
		{	LOG_RESULT_SERVICE_LOG_FULL,							"服务器容量达到极限"},
		{	LOG_RESULT_ROLE_NOTEXIST,								"角色不存在"},
		{	LOG_RESULT_ROLE_INVALID,								"角色时间段无效"},
		{	LOG_RESULT_FORBID_REPEATLOGIN,							"不允许该用户重复登录"},
		{	LOG_RESULT_IPADDRESS_ERROR,								"IP地址不在有效范围内"},
		{	LOG_RESULT_SINGLE_SIGN_ON_FAIL,							"免登录失败"},
		{	PLATFORMID_ERROR,										"平台ID错误"},

		//--------------------------------------------------------------------
		// 点流返回结果
		{	GET_STREAM_NO_STREAM,									"流不存在"},
		{	GET_STREAM_NO_DEVICE,									"设备不存在，或者不可以使用"},
		{	GET_STREAM_NO_CHANNEL,									"设备对应通道不存在"},
		{	GET_STREAM_TIME_OUT,									"点流超时"},
		{	GET_STREAM_NO_DEV_DAS,									"设备对应的DAS服务没有登陆"},
		{   GET_STREAM_WAIT_ACK,									"点流等待处理保持连接"},

		//--------------------------------------------------------------------
		// 获取数据版本结果
		{	GET_DATA_VERSION_FAIL,									"获取数据版本失败"},

		//--------------------------------------------------------------------
		// 平台录像回复结果
		{	GET_SER_RECORD_FAIL,									"平台录像规则获取失败"},

		//--------------------------------------------------------------------
		// 获取预置点信息
		{	GET_PTZ_PRESET_FAIL,									"获取预置点失败"},
		//--------------------------------------------------------------------
		// 告警相关代码
		{	GET_ALARM_RULE_FAIL,									"告警规则获取失败"},
		{	ALARM_LINKAGE_EVENT_OK,									"告警联动事件通知OK"},
		{	ALARM_LINKAGE_EVENT_ERROR,								"告警联动事件通知错误"},

		//--------------------------------------------------------------------
		//用户权限
		{	GET_USER_RIGHT_FAIL,									"获取用户权限失败"},
		{	USER_NO_OPERATOR_RIGHT,									"用户无操作权限"},
		
		//--------------------------------------------------------------------
		// 录像文件操作
		{	FIND_TRANSFER_CHN_ERROR,								"查找传输通道错误！"},
		{	PACKET_LEN_CHECK_ERROR,									"包长度校验错误！"},
		{	QUERY_PARAM_ERROR,										"查询参数错非法"},
        {	QUERY_DBASE_ERROR,										"数据库操作失败"},
        {	QUERY_STROAGE_ERROR,									"不支持的存储类型查找"},
        {	PLAYBACK_VERSION_ERR,									"不支持的版本号" },
        {   PLAYBACK_FILE_NEXIST,									"文件不存在"},
        {   PLAYBACK_FILE_NOPER,									"文件不可以操作"},
		{	RECORD_REVIEW_DOWNLOAD_ACCESS_MAX,                      "录像回放/下载接入数已达到最大值"},

		//--------------------------------------------------------------------
		//布防撤防相关代码
		{	GET_DEPLOYMENT_DENFENCE_RULE_FAIL,						"获取布防撤防规则失败"},
		{	DEPLOYMENT_DEFENCE_COMMAND_NOTICE_OK,					"布防撤防命令通知成功"},
		{	DEPLOYMENT_DEFENCE_COMMAND_NOTICE_FAIL,					"布防撤防命令通知失败"},

		{	DATA_PACKET_ILLEGAL,									"数据包错误"},
		{	INVALID_PARAM,											"参数非法"},
		{	FOUND_NO_TRANSMIT_CHANNEL,								"无法找到传输通道"},
		{	FOUND_DATA_FAIL,										"无法找到相应数据"},

		{	OPERATE_LOCK_MONOLIZE,									"操作被独占控制锁定"},

		{	TRANSMIT_PACKET_SEND_FAIL,								"转发数据包发送失败"},
		{	PARAM_NOT_CONFIG,										"参数未配置"},
		{	DATABASE_OPER_FAIL,										"数据库操作失败"},

		{	MATRIX_UNABLE_DECODE,									"未能解码"},



		//录像操作
		{	RET_RECORD_DEVICE_NO_FUNCTION,				"不支持录像功能"},
		{	RET_RECORD_DATABASE_ERROR,					"数据库操作失败"},
		{	RET_RECORD_NO_DISK,					"没有存储器"},
		{   USER_NO_PRIORITY,                   "用户优先级不够"},
		{   CMS_OFFLINE,                   "CMS不在线"},
		{   DOWN_BMS_OFFLINE,                   "下级BMS不在线"},
		{   DEVICE_OFFLINE,                     "设备不在线"},
		{   OVER_MAX_NUM,                     "超过最大值"},

	};

	// 动作运行标志
	typedef	enum	EnumRunFlag
	{
		L_R_F_STOP = 0,				// 停止
		L_R_F_START					// 开始
	}EnumRunFlag;

	/**************************************************************************************************
	CMD_ID		: 基础类型数据  
	CMD_NAME		: 基础类型初级
	DateTime		: 2010/7/30 16:05	
	Author 		: Liujs      
	Description	: 命令描述
	Note			: 备注
	**************************************************************************************************/
	// 类型信息，存储的是由编号和名称构成的信息
	// 例如：[设备类型ID,设备类型名称]，[厂家ID,厂家名称]，[告警级别ID,告警级别名称]，
	// [通道类型ID,通道类型名称]，[告警类型ID,告警类型名称]
	typedef		struct	StruTypeInfo	StruCmdTypeInfo,*StruCmdTypeInfoPtr;

	// 设备型号信息
	typedef		StruCmdTypeInfo					StruCmdDevModelItem;
	typedef		StruCmdDevModelItem*			StruCmdDevModelItemPtr;

	// 设备类型信息
	typedef		StruCmdTypeInfo					StruCmdDevTypeItem;
	typedef		StruCmdDevTypeItem*				StruCmdDevTypeItemPtr;

	// 设备厂家信息
	typedef		StruCmdTypeInfo					StruCmdDevFactoryItem;
	typedef		StruCmdDevFactoryItem*			StruCmdDevFactoryItemPtr;

	// 通道类型信息
	typedef		StruCmdTypeInfo					StruCmdChannelTypeItem;
	typedef		StruCmdChannelTypeItem*			StruCmdChannelTypeItemPtr;

	// 告警级别信息
	typedef		StruCmdTypeInfo					StruCmdAlarmLevelTypeItem;
	typedef		StruCmdAlarmLevelTypeItem*		StruCmdAlarmLevelTypeItemPtr;

	// 告警类型信息
	typedef		StruCmdTypeInfo					StruCmdAlarmTypeItem;
	typedef		StruCmdAlarmTypeItem*			StruCmdAlarmTypeItemPtr;
	
	//--------------------------------------------------------------------------------------------------------------------
	// 服务类型定义数组
	const		INT32				coniServerTypeNum		=		10;

	// 服务类型信息
	typedef		StruCmdTypeInfo		StruServerTypeItem;

	// 服务类型获取
	const		StruServerTypeItem				conStServerType[] =
	{
		{	CLIENT_TYPE_PMS ,			"平台管理服务"},
		{	CLIENT_TYPE_DAS	,			"设备接入服务"},	
		{	CLIENT_TYPE_STS	,			"流媒体存储服务"},		
		{	CLIENT_TYPE_LMS ,			"日志管理服务"},		
		{	CLIENT_TYPE_AMS ,			"告警关联服务"},	
		{	CLIENT_TYPE_CSS ,			"中心存储服务"},		
		{	CLIENT_TYPE_CLI ,			"中心客户端"},
		{	CLIENT_TYPE_CMU ,			"配置管理中心"},		
		{	CLIENT_TYPE_PU	,			"前端设备"},	
		{	CLIENT_TYPE_VMS ,			"电视墙服务"},
	};

	/**************************************************************************************************
		CMD_ID		: EnumNodeType
		CMD_NAME	: EnumNodeType
		DateTime	: 2010/12/2 9:21	
		Description	: 节点类型信息
		Author		: Liujs
		Note		: NULL
	**************************************************************************************************/
	typedef		enum	EnumNodeType
	{
		NODE_DOMAIN		=	99,								// 域类型1
		NODE_DEVICE			  ,								// 设备
		NODE_CHANNEL	=	CHANNEL_TYPE_VIDEO,				// 视频通道
		NODE_INPUT      =	CHANNEL_TYPE_INPUT,				// 告警输入
		NODE_OUTPUT		=   CHANNEL_TYPE_OUTPUT             // 告警输出
	}EnumNodeType;




	/**************************************************************************************************
	  CMD_ID		: CMD_LOGIN    
	  CMD_NAME		: 登陆PMS    
	  DateTime		: 2010/7/29 10:39	
	  Author 		: Liujs      
	  Description	: 登陆命令
	  Note			: 客户端登陆PMS命令
	**************************************************************************************************/
	#define			CMD_LOGIN_REQUEST			1			// 平台内客户端（登陆PMS）登陆命令

	// 空的登陆ID,填写为0
	#define			EMPTY_LOGIN_ID				0

	// WEB的登陆ID,填写为1,判断是否为web客户端应结合iLoginType类型
	#define			WEB_CLI_LOGIN_ID			1
	// 单点登录标志，iLoginID等于该值时，表示此用户已经从其它系统登录，允许其免登录。
	#define			SINGLE_SIGN_ON_ID			2
	//OCX控件调用Videosdk登录bms，主要为了在bms端判断在线用户时，区分不同的登录
	#define			OCX_CALL_VIDEOSDK_ID		3
	//gis系统登录时的ID，允许其免登录。
	#define			GIS_ID						4

	// 登陆命令数据结构体
	typedef		struct		StruCmdLoginInfo 
	{
		INT32		iLoginID;								// 登陆ID，如果没有就填写 EMPTY_LOGIN_ID
		INT32		iLoginType;								// 登陆类型	参考EnumClientType中定义
		char		szUserName[MAX_NAME_LEN];				// 登陆名称
		char		szPassword[MAX_NAME_LEN];				// 登陆密码
		char		szVersion[MAX_VERSION_LEN];				// 版本号，CLI登录填0，其他服务登录填服务本身的版本号
		char		szLoginIP[MAX_IP_ADDR_LEN];				// 登陆IP
	}StruCmdLoginInfo,*StruCmdLoginInfoPtr;	



	/**************************************************************************************************
	  CMD_ID		: CMD_LOGIN_RESPONSE 
	  CMD_NAME		: 登陆应答命令       
	  DateTime		: 2010/7/29 11:03	
	  Author 		: Liujs      
	  Description	: 登陆命令回复
	  Note			: 
	**************************************************************************************************/
	#define		CMD_LOGIN_RESPONSE				2			// 登陆命令回复

	// 登陆命令回复结构体
	typedef		struct		StruCmdLoginResponse 
	{
		INT32		iLogResult;								// 登陆结果，参考EnumErrorCode中定义
		INT32		iLoginID;								// 登陆返回ID
		INT32		iPlatformID;							// 平台ID
	}StruCmdLoginResponse,*StruCmdLoginResponsePtr;


	/**************************************************************************************************
	  CMD_ID		: CMD_GET_STREAM_REQUEST   
	  CMD_NAME		: 发起点流
	  DateTime		: 2010/7/29 11:50	
	  Author 		: Liujs      
	  Description	: 命令描述
	  Note			: 备注
	**************************************************************************************************/
	#define		CMD_GET_STREAM_REQUEST			101			// 点流发起命令

	// 码流类型枚举
	typedef enum EnumStreamType
	{
		STREAM_TYPE_FIRST = 1,          // 第一码流
		STREAM_TYPE_SECOND,             // 第二码流
		STREAM_TYPE_THIRD               // 第三码流
	}EnumStreamType;

	// 发起点流的命令数据
	typedef		struct		StruCmdGetStream
	{
		INT32		iClientType;							// 点流发起的客户端类型,参考结构：EnumClientType
		INT32		iChannelNum;							// 通道号
		INT32       iStreamType;                            // 码流类型(默认第一码流),参考枚举类型： EnumStreamType
		char        szIPAddr[MAX_IP_ADDR_LEN];				// 客户端IP地址
	}StruCmdGetStream,*StruCmdGetStreamPtr;
	
	// 用户信息					
	typedef struct StruUserInfo
	{
		char szUserName[MAX_NAME_LEN];						// 用户名称
		char szUserPwd[MAX_PWD_LEN];						// 用户密码
	}StruUserInfo,*StruUserInfoPtr;

	// 级联点流请求
	typedef struct StruCmdCascadeGetStream
	{
		StruCmdGetStream	stCmdContent;					// 具体含义参考其定义
		INT32				iClientNum;						// 用户信息数目
		StruUserInfo		stUserInfo[1];					// 用户信息，变长结构
	}StruCmdCascadeGetStream,*StruCmdCascadeGetStreamPtr;

	/**************************************************************************************************
	  CMD_ID		: CMD_GET_STREAM_RESPONSE  
	  CMD_NAME		: 点流结果返回命令
	  DateTime		: 2010/7/29 14:14	
	  Author 		: Liujs      
	  Description	: 点流命令返回结果信息
	  Note			: 
	**************************************************************************************************/
	#define		CMD_GET_STREAM_RESPONSE			102			// 点流返回结果命令

	// 点流返回结果命令数据
	typedef		struct		StruCmdGetStreamResponse 
	{
		INT32		iGetStreamResult;						// 点流返回结果，参考EnumErrorCode	
		INT32		iClientType;							// 点流发起的客户端类型,参考结构：EnumClientType
		INT32		iChannelNum;							// 点流通道信息
		char		szStreamURI[MAX_URI_LEN];				// 点流返回URI资源信息，最大长度为512字符,256中文字符
	}StruCmdGetStreamResponse,*StruCmdGetStreamResponsePtr;

	/**************************************************************************************************
	  CMD_ID		: CMD_GET_DEV_STATUS  
	  CMD_NAME		: 取设备状态
	  DateTime		: 2012/4/16 14:17	
	  Author 		: CHC  
	  Description	: 取设备状态
	  Note			: 目前主要用于BMS向CMS取设备状态,其应答和主动上报状态的命令CMD_DEV_STATUS_NOTICE相同
	**************************************************************************************************/
	#define		CMD_GET_DEV_STATUS				200				// 取设备状态
	//无参数

	/**************************************************************************************************
	  CMD_ID		: CMD_DEV_ONLINE_NOTICE  
	  CMD_NAME		: 设备和服务状态通知
	  DateTime		: 2010/7/29 14:17	
	  Author 		: Liujs      
	  Description	: 设备，服务状态通知
	  Note			: 服务上线的时候
	**************************************************************************************************/
	#define		CMD_DEV_STATUS_NOTICE			201				// 设备状态通知命令

	// 设备状态类型
	typedef	enum		EnumDevStatusType
	{
		UNIT_ONLINE		=	1,									// 上线
		UNIT_OFFLINE	=	0									// 下线
	}EnumDevStatusType;

	// 通道状态类型
	//typedef	enum		EnumChnStatusType
	//{
	//	CHN_VIDEO_FAULT		=	0,								// 视频故障
	//	CHN_VIDEO_OK		=	1								// 视频正常
	//}EnumChnStatusType;

	// 通道状态类型定义
	typedef enum
	{
		CHANNEL_NORMAL = 0,             // 正常
		CHANNEL_SIGNAL_LOST = 1,        // 信号丢失
		CHANNEL_CONNECT_FAIL = 2,       // 设备连接不上

	} EnumChannelStatus;


	// 设备状态信息
	typedef		struct		StruUnitStatus
	{
		INT32			iUnitID;								// 设备ID
		INT32			iStatusType;							// 设备状态，设备状态类型，参考结构EnumDevStatusType
	}StruUnitStatus,*StruUnitStatusPtr;

	// 设备状态通知命令数据
	typedef		struct		StruCmdDevStatusNotice
	{
		INT32			iPlatformID;							// 平台ID
		INT32			iUnitType;								// 设备类型,参考结构EnumClientType
		INT32			iUnitNum;								// 设备ID个数
		StruUnitStatus	stUnitStatusArray[MAX_DEV_STATUS_NUM];	// 设备状态数组
	}StruCmdDevStatusNotice,*StruCmdDevStatusNoticePtr;


	// 设备状态通知确认命令   modified by jiangsx 20130814 
    #define CMD_DEV_STATUS_NOTICE_AFFIRM 202
	// 不需要消息体


	// 设备状态信息心跳       modified by jiangsx 20130814 
    #define CMD_DEV_STATUS_KEEPALIVE  203

	// 设备状态信息心跳结构定义
	typedef struct StruCmdDevStatusKeepalive
	{
		INT32 iPlatformID;                  // 平台ID
		INT16 iStatusType;                  // 设备状态，设备状态类型，参考结构EnumDevStatusType
		INT16 iDevNum;                      // 设备个数
		INT32 iDevID[1];                    // 设备ID列表
	}StruCmdDevStatusKeepalive, *StruCmdDevStatusKeepalivePtr;


	/**************************************************************************************************
	  CMD_ID		: CMD_BASS_DATA_REQUEST  
	  CMD_NAME		: 基础数据请求命令数据
	  DateTime		: 2010/7/30 15:17	
	  Author 		: Liujs      
	  Description	: 基础数据请求命令数据
	  Note			: 
	**************************************************************************************************/
	#define		CMD_BASS_DATA_REQUEST			301				// 基础数据请求命令

	// 基础数据请求类型
	typedef		enum		EnumBassDataRequestType
	{
		REQUEST_DEV_MODEL	=	1,								// 请求设备型号信息
		REQUEST_DEV_TYPE	=	2,								// 设备类型信息
		REQUEST_DEV_FACTORY =	3,								// 请求设备厂家信息
		REQUEST_CHANNEL_TYPE=	4,								// 请求通道类型信息
		REQUEST_ALARM_TYPE	=	5,								// 告警类型信息
		REQUEST_ALARM_LEVEL	=	6,								// 请求告警级别信息

		REQUEST_ALL_BASE_INFO = 100								// 请求所有类型
	};

	// 请求基础数据发送命令结构体
	typedef		struct		StruCmdBaseDataRequest
	{	
		INT32					iClientType;					// 发起请求的客户端类型,参考结构：EnumClientType
		INT32					iRequestType;					// 设备数据请求类型 参考EnumBassDataRequestType结构
	}StruCmdBaseDataRequest,*StruCmdBaseDataRequestPtr;

	/**************************************************************************************************
	  CMD_ID		: CMD_BASS_DATA_RESPONSE  
	  CMD_NAME		: 基础数据回复命令
	  DateTime		: 2010/7/30 16:20	
	  Author 		: Liujs      
	  Description	: 基础数据回复命令
	  Note			: 基础数据回复命令
	**************************************************************************************************/
	#define		CMD_BASS_DATA_RESPONSE			302				// 基础数据回复命令

	//------------------------------------------------------------------------------------------------
	// 请求类型：REQUEST_DEV_TYPE
	// 设备型号数据信息结构
	typedef		struct	StruBaseDevModel	struCmdDevModelInfo,*struCmdDevModelInfoPtr;

	// 设备型号数据结构
	// 当请求类型为：REQUEST_DEV_MODEL的时候返回数据的结构体
	typedef		struct		struCmdDevModelResponse 
	{
		INT32					iNum;							// 设备型号个数
		struCmdDevModelInfo		stCmdDevModelInfo[MAX_TYPE_NUM];// 设备型号信息
	}struCmdDevModelResponse,*struCmdDevModelResponsePtr;
	//------------------------------------------------------------------------------------------------


	//------------------------------------------------------------------------------------------------
	// 请求类型：REQUEST_DEV_TYPE
	// 设备类型数据结构
	// 当请求类型为：REQUEST_DEV_TYPE的时候返回的数据结构体
	typedef		struct		struCmdDevTypeResponse 
	{
		INT32					iNum;							// 设备类型个数
		StruCmdDevTypeItem		stCmdDevtypeItem[MAX_TYPE_NUM];	// 设备类型信息
	}struCmdDevTypeResponse,*struCmdDevTypeResponsePtr;
	//------------------------------------------------------------------------------------------------


	//------------------------------------------------------------------------------------------------
	// 请求类型：REQUEST_DEV_FACTORY
	// 厂家信息数据结构
	// 当请求类型为：REQUEST_DEV_FACTORY的时候返回的数据结构体
	typedef		struct		struCmdDevFactoryResponse 
	{
		INT32					iNum;								// 厂家信息个数
		StruCmdDevFactoryItem	stCmdDevFactoryItem[MAX_TYPE_NUM];	// 厂家信息
	}struCmdDevFactoryResponse,*struCmdDevFactoryResponsePtr;
	//------------------------------------------------------------------------------------------------


	//------------------------------------------------------------------------------------------------
	// 请求类型：REQUEST_CHANNEL_TYPE
	// 通道类型数据结构
	// 当请求类型为：REQUEST_CHANNEL_TYPE的时候返回的数据结构体
	typedef		struct		struCmdChannelTypeResponse 
	{
		INT32					iNum;								// 通道类型个数
		StruCmdChannelTypeItem	stCmdChannelTypeItem[MAX_TYPE_NUM];	// 通道类型信息
	}struCmdChannelTypeResponse,*struCmdChannelTypeResponsePtr;
	//------------------------------------------------------------------------------------------------


	//------------------------------------------------------------------------------------------------
	// 请求类型：REQUEST_ALARM_TYPE
	// 告警类型数据结构
	// 当请求类型为：REQUEST_ALARM_TYPE的时候返回的数据结构体
	typedef		struct		struCmdAlarmTypeResponse 
	{
		INT32					iNum;								// 告警类型个数
		StruCmdAlarmTypeItem	stCmdAlarmTypeItem[MAX_TYPE_NUM];	// 告警类型信息
	}struCmdAlarmTypeResponse,*struCmdAlarmTypeResponsePtr;
	//------------------------------------------------------------------------------------------------


	//------------------------------------------------------------------------------------------------
	// 请求类型：REQUEST_ALARM_LEVEL
	// 告警级别数据结构
	// 当请求类型为：REQUEST_ALARM_LEVEL的时候返回的数据结构体
	typedef		struct		struCmdAlarmLevelTypeResponse 
	{
		INT32						iNum;								// 告警级别个数
		StruCmdAlarmLevelTypeItem	stCmdDevFactoryItem[MAX_TYPE_NUM];	// 告警级别信息
	}struCmdAlarmLevelTypeResponse,*struCmdAlarmLevelTypeResponsePtr;
	//------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	// 请求类型：REQUEST_ALL_BASE_INFO
	// 数据结构，请求类型为：REQUEST_ALL_BASE_INFO 的时候回复的信息
	typedef		struct		struCmdAllBassDataResponse 
	{
		struCmdDevModelResponse			stDevModel;						// 设备型号信息，参考 struCmdDevModelResponse
		struCmdDevTypeResponse			stDevType;						// 设备类型，参考 struCmdDevTypeResponse
		struCmdDevFactoryResponse		stDevFactory;					// 设备厂商信息，参考 struCmdDevFactoryResponse
		struCmdChannelTypeResponse		stChannelType;					// 设备通道类型，参考 struCmdChannelTypeResponse
		struCmdAlarmTypeResponse		stAlarmType;					// 告警类型，参考 struCmdAlarmTypeResponse
		struCmdAlarmLevelTypeResponse	stAlarmLevelType;				// 告警级别，参考 struCmdAlarmLevelTypeResponse
	}struCmdAllBassDataResponse,*struCmdAllBassDataResponsePtr;


	// 返回给请求方的发送数据结构体
	typedef		struct		struCmdBassDataResponse 
	{		
		INT32			iClientType;									// 发起请求的客户端类型,参考结构：EnumClientType
		INT32			iRequestType;									// 请求数据类型，参考结构EnumBassDataRequestType
		INT32			iDataLen;										// 数据长度	
		/* 
		返回数据，根据EnumBassDataRequestType类型来决定，
		请求类型						数据结构体
		REQUEST_DEV_MODEL				struCmdDevModelResponse
		REQUEST_DEV_TYPE				struCmdDevTypeResponse
		REQUEST_DEV_FACTORY				struCmdDevFactoryResponse
		REQUEST_CHANNEL_TYPE			struCmdChannelTypeResponse
		REQUEST_ALARM_TYPE				struCmdAlarmTypeResponse
		REQUEST_ALARM_LEVEL				struCmdAlarmLevelTypeResponse
		REQUEST_ALL_BASE_INFO			struCmdAllBassDataResponse
		*/
		void*			pData;											// 返回数据，根据EnumDataRequestType类型来决定，											
	}struCmdBassDataResponse,*struCmdBassDataResponsePtr;



	/**************************************************************************************************
	  CMD_ID		: CMD_DEV_LIST_REQUEST   
	  CMD_NAME		: 设备数据请求
	  DateTime		: 2010/7/29 15:09	
	  Author 		: Liujs      
	  Description	: 设备数据请求
	  Note			: NULL
	**************************************************************************************************/
	#define		CMD_DEV_DATA_REQUEST			401				// 设备列表请求命令

	// 设备数据请求类型
	typedef		enum		EnumDataRequestType
	{
		REQUEST_DOMAIN_LIST_INFO		= 1,				// 域列表数据的请求[域列表信息]
		REQUEST_BASS_INFO				,					// 设备基础信息[单元ID，版本号]
		REQUEST_CHANNEL_INFO			,					// 设备通道信息获取[设备通道信息]
		REQUEST_DOMAIN_DESC_INFO		,					// 获取单元描述信息[单元域、描述信息]
		REQUEST_EXTERN_INFO				,					// 获取扩展参数信息[设备扩展参数信息]
		REQUEST_LOGIN_INFO				,					// 获取设备登陆信息[通信方式，登陆信息]
		REQUEST_SIMPLE_DEV_CHN_INFO		,					// 简单设备通道信息[简单的设备，域名称，通道列表]
		REQUEST_GB28181CODE_INFO        ,                   // 获取国标28181编码信息[节点国标编码关联信息]
		REQUEST_ALL_INFO									// 获取所有信息[所有设备的相关信息]
	};
	// 请求类型为：REQUEST_DOMAIN_LIST_INFO的时候，域ID为：0
	#define		EMPTY_DOMAIN_ID					0
	// 请求类型为：REQUEST_ALL_INFO的时候，域ID为：-1
	#define		REQUEST_ALL_DOMAIN_ID			-1
	// 如果请求数据的客户端不是用户终端客户端（例如：DAS，STS等，不关注域），那么就填写 NOT_USER_CLI_REQUEST
	#define		NOT_USER_CLI_REQUEST			-2

	// 设备列表请求命令数据
	typedef		struct		StruCmdDevDataRequest 
	{
		INT32					iClientType;					// 发起请求的客户端类型,参考结构：EnumClientType
		INT32					iRequestType;					// 设备数据请求类型 参考EnumDataRequestType结构
		INT32					iDomainID;						// 域ID,请求数据类型为REQUEST_DOMAIN_LIST_INFO时候，填写:EMPTY_DOMAIN_ID
	}StruCmdDevDataRequest,*StruCmdDevDataRequestPtr;


	/**************************************************************************************************
	  CMD_ID		: CMD_DEV_DATA_RESPONSE
	  CMD_NAME		: 设备列表应答命令数据
	  DateTime		: 2010/7/29 15:13	
	  Author 		: Liujs      
	  Description	: 设备列表应答数据
	  Note			: NULL
	**************************************************************************************************/
	#define		CMD_DEV_DATA_RESPONSE			402				// 设备数据请求数据应答

	//------------------------------------------------------------------------------------------------
	// 请求类型：REQUEST_DOMAIN_LIST_INFO

	// 域信息基本结构
	typedef		struct		StruDomainInfoItem StruCmdDomainInfoItem,*StruCmdDomainInfoItemPtr;

	// 域结构信息,请求类型为:REQUEST_DOMAIN_LIST_INFO,返回的数据
	typedef		struct		StruCmdDomainInfo 
	{
		INT32					iNum;								// 域的个数
		StruCmdDomainInfoItem	stCmdDomainInfoItem[MAX_DEV_NUM];	// 域的数组
	}StruCmdDomainInfo,*StruCmdDomainInfoPtr;

	//------------------------------------------------------------------------------------------------


	//------------------------------------------------------------------------------------------------
	// 请求类型：REQUEST_BASS_INFO

	// 单元（设备，客户端，以及服务）基本信息
	typedef		struct		StruCmdDevBaseInfoItem
	{
		INT32					iPlatFormID;						// 平台ID
		INT32					iServID;							// 设备所属DAS的ID
		StruBaseInfo			stDevBaseInfo;						// 设备基础信息
		INT32					iStatusType;						// 设备状态，设备状态类型，参考结构EnumDevStatusType
	}StruCmdDevBaseInfoItem,*StruCmdDevBaseInfoItemPtr;

	// 请求数据类型为：REQUEST_BASS_INFO的命令数据
	typedef		struct		StruCmdDevBase
	{
		INT32					iDevNum;							// 设备个数
		StruCmdDevBaseInfoItem	stCmdDevBassInfoItem[MAX_DEV_NUM];	// 单元基本信息
	}StruCmdDevBase,*StruCmdDevBasePtr;
	//------------------------------------------------------------------------------------------------



	//------------------------------------------------------------------------------------------------
	// 请求类型：REQUEST_LOGIN_INFO
	// 设备登陆信息基础信息
	typedef		struct		StruCmdDevLoginInfoItem 
	{	
		INT32					iUnitID;							// 设备单元ID
		INT32					iPlatFormID;						// 平台ID
		StruDevLoginInfo		stDevLoginInfo;						// 设备登陆信息
	}StruCmdDevLoginInfoItem,*StruCmdDevLoginInfoItemPtr;


	// 请求数据类型为：REQUEST_LOGIN_INFO的命令数据
	typedef		struct		StruCmdDevLogin
	{
		INT32					iDevNum;							// 设备个数
		StruCmdDevLoginInfoItem	stCmdDevLoginInfoItem[MAX_DEV_NUM];	// 单元登陆信息
	}StruCmdDevLogin,*StruCmdDevLoginPtr;
	//------------------------------------------------------------------------------------------------


	//------------------------------------------------------------------------------------------------
	// 请求类型：REQUEST_DOMAIN_DESC_INFO

	// 请求域信息命令数据
	// 请求数据类型为：REQUEST_DOMAIN_DESC_INFO 的命令数据
	typedef		struct		 StruCmdDomainDescInfoItem
	{
		INT32						iUnitID;								// 设备单元ID
		INT32						iPlatFormID;							// 平台ID
		StruDomain					stDomain;								// 所属域
		StruDescri					stDescri;								// 设备描述信息
	}StruCmdDomainDescInfoItem,*StruCmdDomainDescInfoItemPtr;

	// 请求数据类型为：REQUEST_DOMAIN_DESC_INFO 的命令数据
	typedef		struct		 StruCmdDomainDesc
	{
		INT32						iDevNum;								// 设备个数
		StruCmdDomainDescInfoItem	stCmdDomainDescInfoItem[MAX_DEV_NUM];	// 单元域，描述信息
	}StruCmdDomainDesc,*StruCmdDomainDescPtr;
	//------------------------------------------------------------------------------------------------



	//------------------------------------------------------------------------------------------------
	// 请求类型：REQUEST_CHANNEL_INFO

	// 通道基础信息数据结构
	typedef		struct		StruCmdChannelItemInfo
	{
		INT32					iChannelNo;								// 通道号
		char					szChannelName[MAX_NAME_LEN];			// 通道名称
		INT32					iChannelTypeID;							// 通道类型ID
		INT32					iStatusType;							// 通道状态，参考结构EnumChannelStatus
	}StruCmdChannelItemInfo,*StruCmdChannelItemInfoPtr;

	// 通道类型信息
	typedef		struct		StruCmdChannelInfo 
	{
		INT32					iUnitID;								// 设备单元ID
		INT32					iPlatFormID;							// 平台ID
		INT32					byChannelNum;							// 通道个数，（0，MAX_CHANNEL_NUM）
		StruCmdChannelItemInfo	stCmdChannelItemInfo[MAX_CHANNEL_NUM];	// 通道列表信息
	}StruCmdChannelInfo,*StruCmdChannelInfoPtr;

	// 请求数据类型为：REQUEST_CHANNEL_INFO的时候用到
	typedef		struct		 StruCmdChannel
	{
		INT32					iDevNum;								// 设备个数
		StruCmdChannelInfo		stCmdChannelInfoItem[MAX_DEV_NUM];		// 单元通道信息
	}StruCmdChannel,*StruCmdChannelPtr;
	//------------------------------------------------------------------------------------------------



	//------------------------------------------------------------------------------------------------
	// 请求类型：REQUEST_SIMPLE_DEV_CHN_INFO

	// 简单设备通道信息基础结构
	typedef		struct		StruCmdSimpleDevChnItem
	{
		INT32					iUnitID;								// 设备单元ID
		char					szUnitName[MAX_NAME_LEN];				// 单元名称
		INT32					iStatusType;							// 设备状态，设备状态类型，参考结构EnumDevStatusType
		INT32					iPlatFormID;							// 平台ID
		INT32					iDomainID;								// 域ID
		char					szDomainName[MAX_NAME_LEN];				// 域名称
		INT32					byChannelNum;							// 通道个数，（0，MAX_CHANNEL_NUM）
		StruCmdChannelItemInfo	stCmdChannelItemInfo[MAX_CHANNEL_NUM];	// 通道列表信息
	}StruCmdSimpleDevChnItem,*StruCmdSimpleDevChnItemPtr;

	// 请求数据类型为：REQUEST_SIMPLE_DEV_CHN_INFO的时候用到
	typedef		struct		 StruCmdSimpleDevChnInfo
	{
		INT32						iDevNum;								// 设备个数
		StruCmdSimpleDevChnItem		stCmdSimpleDevChnItem[MAX_DEV_NUM];		// 简单设备通道信息基础结构
	}StruCmdSimpleDevChnInfo,*StruCmdSimpleDevChnInfoPtr;
	//------------------------------------------------------------------------------------------------


	//-------------------------------------------------------------------------------------------------
	// 请求类型: REQUEST_GB28181CODE_INFO

	// 国标编码节点结构定义
	typedef struct StruCmdGB28181Node
	{
		INT32 iPlatformID;                            // 平台ID
		INT32 iDomainID;                              // 域ID
		INT32 iDevID;                                 // 设备ID
		INT32 iChanID;                                // 通道号
		INT32 iNodeType;                              // 节点类型，参考EnumNodeType
		char  szNodeName[MAX_NAME_LEN];               // 节点名称（如：域名、设备名、通道名等）
		char  szGBCodeID[MAX_ID_STRING_LEN];          // 国标编码
	} StruCmdGB28181Node, *StruCmdGB28181NodePtr;

	// 国标28181编码信息结构定义
	typedef struct StruCmdGB28181CodeInfo
	{
		INT32 iNum;
		StruCmdGB28181Node stCmdGBNode[1];
	} StruCmdGB28181CodeInfo, *StruCmdGB28181CodeInfoPtr;

	//-------------------------------------------------------------------------------------------------



	//------------------------------------------------------------------------------------------------
	// 请求类型：REQUEST_EXTERN_INFO

	// 扩展参数信息基础结构
	typedef		struct	StruMemBlock	StruCmdDevExternItem,*StruCmdDevExternItemPtr;

	// 设备扩展信息
	typedef		struct		StruCmdDevExternInfo 
	{	
		INT32					iUnitID;								// 设备单元ID
		INT32					iPlatFormID;							// 平台ID
		StruDevExInfo			stDevExInfo;							// 设备扩展参数信息
	}StruCmdDevExternInfo,*StruCmdDevExternInfoPtr;

	// 请求数据类型为：REQUEST_EXTERN_INFO的时候用到
	typedef		struct		 StruCmdDevExtern
	{
		INT32					iDevNum;								// 设备个数
		StruCmdDevExternInfo	stCmdDevExternInfoItem[MAX_DEV_NUM];	// 单元通道信息
	}StruCmdDevExtern,*StruCmdDevExternPtr;
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	// 请求类型：REQUEST_ALL_INFO
	// 设备所有信息
	// 请求数据类型为：REQUEST_ALL_INFO的时候用到的
	typedef		struct		StruCmdAllDevInfo 
	{
		// 域列表信息	
		StruCmdDomainInfo		stDomainInfo;							// 域列表信息结构，参考StruCmdDomainInfo
		// 设备基础信息
		StruCmdDevBase			stDevBassInfo;							// 设备基础信息结构，参考StruCmdDevBase
		// 设备登陆信息
		StruCmdDevLogin			stDevLoginInfo;							// 设备登陆信息结构，参考StruCmdDevLogin
		// 设备域、描述信息
		StruCmdDomainDesc		stDomainDescInfo;						// 设备域描述信息，参考StruCmdDomainDesc
		// 设备通道信息
		StruCmdChannel			stChannelInfo;							// 设备通道信息，参考StruCmdChannel
		// 国标28181编码信息
		StruCmdGB28181CodeInfo  stGB28181CodeInfo;                      // 国标28181编码信息，参考StruCmdGB28181CodeInfo
		// 设备扩展信息
		StruCmdDevExtern		stDevExternInfo;						// 设备扩展信息，参考StruCmdDevExtern
	}StruCmdAllDevInfo,*StruCmdAllDevInfoPtr;
	//------------------------------------------------------------------------------------------------


	// 返回给请求方的发送数据结构体
	typedef		struct		struCmdDevDataResponse 
	{		
		INT32					iClientType;							// 发起请求的客户端类型,参考结构：EnumClientType
		INT32					iRequestType;							// 请求数据类型，参考结构EnumDataRequestType
		INT32					iDataLen;								// 数据长度	
		/* 
		返回数据，根据EnumDataRequestType类型来决定，
		请求类型						数据结构体
		REQUEST_DOMAIN_LIST_INFO		StruCmdDomainInfo
		REQUEST_BASS_INFO				StruCmdDevBaseInfo
		REQUEST_UNIT_DOMAIN_DESC_INFO   StruCmdDomainDescInfo
		REQUEST_CHANNEL_INFO			StruCmdChannelInfo
		REQUEST_EXTERN_INFO				StruCmdDevExternInfo
		REQUEST_LOGIN_INFO				StruCmdDevLoginInfo
		REQUEST_ALL_INFO				StruCmdAllDevInfo
		*/
		void*					pData;									// 返回数据，根据EnumDataRequestType类型来决定，											
	}struCmdDevDataResponse,*struCmdDevDataResponsePtr;


	/**************************************************************************************************
	  Function		: CMD_USER_ONLINE_LIST_REQUEST    
	  DateTime		: 2011/6/28 14:08	
	  Description	:获取在线用户列表请求
	  Note			:// 备注
	**************************************************************************************************/

	#define		CMD_USER_ONLINE_LIST_REQUEST			403
	
	/**************************************************************************************************
	Function		: CMD_USER_ONLINE_LIST_RESPONSE    
	DateTime		: 2011/6/28 14:08	
	Description		:获取在线用户列表回复
	Note			:// 备注
	**************************************************************************************************/

	#define		CMD_USER_ONLINE_LIST_RESPONSE			404

	typedef		struct		StruUserOnlineInfo 
	{
		INT32				iClientType;				//客户端类型，分为本平台CLI和级联平台，参考EnumClientType
		INT32				iPort;						//端口
		char				szIPAddr[MAX_IP_ADDR_LEN];	//IP地址
		char				szUserName[MAX_NAME_LEN];	//用户名称
	}StruUserOnlineInfo,*StruUserOnlineInfoPtr;

	typedef		struct		StruCmdUserOnlineListResponse
	{
		INT32				iResult;				//请求结果		
		INT32				iNum;					//个数
		StruUserOnlineInfo	stUserOnlineInfo[1];	//用户信息
	}StruCmdUserOnlineListResponse,*StruCmdUserOnlineListResponsePtr;

	//-----------------------------------------------------------------------------------------------
	// 级联平台节点信息请求(一般用于 BMS 向 SGS 请求)
	#define    CMD_CASCADE_NODE_INFO_REQUEST    405

	// 级联平台节点信息请求结构
	typedef StruCmdBaseDataRequest StruCmdCascadeNodeInfoRequest;
	typedef StruCmdBaseDataRequestPtr StruCmdCascadeNodeInfoRequestPtr;


	#define    CMD_CASCADE_NODE_INFO_RESPONSE   406

	// 级联平台节点信息回复结构
	typedef struct StruCmdCascadeNodeInfoResponse
	{
		INT32 iNum;
		StruCmdGB28181Node stCascadeNode[1];
	} StruCmdCascadeNodeInfoResponse, *StruCmdCascadeNodeInfoResponsePtr;

	//-----------------------------------------------------------------------------------------------


	/**************************************************************************************************
	  CMD_ID		: CMD_AUDIO_COMM_REQUEST   
	  CMD_NAME		: 语音对讲请求
	  DateTime		: 2011/12/27 16:30	
	  Author 		: Jiangsx      
	  Description	: 
	  Note			: 
	**************************************************************************************************/
	#define		CMD_AUDIO_COMM_REQUEST			      750	// 语音对讲请求

	// 语音对讲请求
	typedef		struct		StruCmdAudioComm
	{
		INT32       iOptType;                               // 操作类型，参考EnumGSOperationType
		INT16		iObjType;								// 对讲对象类型,参考结构：EnumClientType
		INT16		iChannelNum;							// 对讲通道号
		char        szIPAddr[MAX_IP_ADDR_LEN];				// 客户端IP地址
	} StruCmdAudioComm,*StruCmdAudioCommPtr;
	
	// 级联语音对讲请求  (供鉴权用)
	typedef struct StruCmdCascadeAudioComm
	{
		StruCmdAudioComm	stCmdContent;				    // 具体含义参考其定义
		INT32				iClientNum;						// 用户信息数目
		StruUserInfo		stUserInfo[1];					// 用户信息，变长结构
	} StruCmdCascadeAudioComm,*StruCmdCascadeAudioCommPtr;

	/**************************************************************************************************
	  CMD_ID		: CMD_AUDIO_COMM_RESPONSE  
	  CMD_NAME		: 语音对讲回复
	  DateTime		: 2011/12/27 16:30
	  Author 		: Jiangsx      
	  Description	: 
	  Note			: 
	**************************************************************************************************/
	#define		CMD_AUDIO_COMM_RESPONSE			751			// 语音对讲回复

	// 音频编码类型
	typedef enum EnumAudioEncodeType
	{
		ENCODE_TYPE_PCM = 0,
		ENCODE_TYPE_G711a,
		ENCODE_TYPE_G711u
		// to add:
	} EnumAudioEncodeType;

	// 音频信息(开启语音对讲回复时使用)
	typedef struct StruCmdAudioInfo
	{
		INT8         iEncodeType;        // 编码类型，参考EnumAudioEncodeType
		INT8		 iChns;		         // 声道数
		INT16		 iBitsPerSample;	 // 采样深度
		INT32		 iSamplesPerSec;	 // 采样率
	} StruCmdAudioInfo, *StruCmdAudioInfoPtr;

	// 语音对讲回复
	typedef		struct		StruCmdAudioCommResponse 
	{
		INT16            iOptType;                       // 操作类型，参考EnumGSOperationType
		INT32		     iOptResult;					 // 返回结果，参考EnumErrorCode
		INT16		     iChannelNum;				     // 对讲通道号
		StruCmdAudioInfo stAudioInfo;                    // 具体含义参考其定义
	}StruCmdAudioCommResponse,*StruCmdAudioCommResponsePtr;

	/**************************************************************************************************
	  CMD_ID		: CMD_AUDIO_COMM_ACK
	  CMD_NAME		: 语音对讲心跳命令
	  DateTime		: 2011/12/27 16:30
	  Author 		: Jiangsx      
	  Description	: 
	  Note			: 
	**************************************************************************************************/
	#define  CMD_AUDIO_COMM_ACK    752             // 语音对讲心跳命令(5秒一个心跳)

	// 语音对讲心跳结构
	typedef struct  StruCmdAudioCommAck
	{
		INT16		iObjType;								// 对讲对象类型,参考结构：EnumClientType
		INT16		iChannelNum;							// 对讲通道号 
	} StruCmdAudioCommAck, *StruCmdAudioCommAckPtr;


	/**************************************************************************************************
	  CMD_ID		: CMD_SEND_AUDIO_DATA
	  CMD_NAME		: 发送音频数据
	  DateTime		: 2011/12/27 16:30
	  Author 		: Jiangsx      
	  Description	: 
	  Note			: 
	**************************************************************************************************/
	#define  CMD_SEND_AUDIO_DATA   753                 // 发送音频数据
	
	// 音频数据结构
	typedef struct  StruCmdAudioData
	{
		INT16	iObjType;					   // 对讲对象类型,参考结构：EnumClientType
		INT16   iChannelNum;				   // 对讲通道号 
		INT32   iLen;                          // 音频数据长度
		char    szData[1];                     // 音频数据
	} StruCmdAudioData, *StruCmdAudioDataPtr;

	// 语音广播...   


	/**************************************************************************************************
	CMD_ID		: CMD_PTZ_CONTROL
	CMD_NAME	: 云台控制
	DateTime	: 2010/11/3 11:13	
	Description	: 获取所有数据版本好信息回复命令
	Author		: shidj
	Note		: NULL
	**************************************************************************************************/
	#define			CMD_PTZ_CONTROL 					500				//云台控制命令ID

	//云台启动停止
	typedef enum EnumPTZCtrlType
	{
		PTZ_STOP = 0,
		PTZ_START
	}ENUM_PTZ_CTRL_TYPE;


	//云台控制操作类型
	typedef	enum	EnumPTZControl
	{
		PTZ_NULL = 0,					// 云台停止,没有运动		
		PTZ_LIGHT_PWRON	= 1,			// 接通灯光电源
		PTZ_WIPER_PWRON,				// 接通雨刷开关
		PTZ_ZOOM_IN,					// 焦距变大(倍率变大)
		PTZ_ZOOM_OUT,					// 焦距变小(倍率变小)
		PTZ_FOCUS_NEAR,					// 焦点前调
		PTZ_FOCUS_FAR,					// 焦点后调
		PTZ_IRIS_OPEN,					// 光圈扩大
		PTZ_IRIS_CLOSE,					// 光圈缩小
		PTZ_TILT_UP,					// 云台上仰
		PTZ_TILT_DOWN,					// 云台下俯
		PTZ_PAN_LEFT,					// 云台左转
		PTZ_PAN_RIGHT,					// 云台右转
		PTZ_UP_LEFT,					// 云台上仰和左转
		PTZ_UP_RIGHT,					// 云台上仰和右转
		PTZ_DOWN_LEFT,					// 云台下俯和左转
		PTZ_DOWN_RIGHT,					// 云台下俯和右转
		PTZ_PAN_AUTO,					// 云台左右自动扫描
		PTZ_GOTO_PRESET					// 转到预置点
	}EnumPTZControl;

	// 云台控制命令，发送对应数据结构
	typedef struct StruCmdPTZControl
	{
		INT32	iChannel;								// 通道号
		INT32	iOperType;								// 操作类型,参考枚举EnumPTZControl
		INT32	iOperContent;							// 操作内容,1表示动作开始,0表示动作结束
		INT32	iExtParam;								// 如该动作带有相关参数(比如速度,预置点的索引等)，则表示该参数值；否则，可以置0
		char szName[MAX_NAME_LEN];						// 预置点控制操作时，代表预置点名称，其他操作目前未用
	}StruCmdPTZControl,*StruCmdPTZControlPtr;

	// 级联云台控制请求
	typedef struct StruCmdCascadePTZControl
	{
		StruCmdPTZControl			stCmdContent;			// 具体含义参考其定义
		INT32						iClientNum;				// 用户信息数目
		StruUserInfo				stUserInfo[1];			// 用户信息，变长结构
	}StruCmdCascadePTZControl,*StruCmdCascadePTZControlPtr;
	
	/**************************************************************************************************
	CMD_ID		: CMD_OPER_RESULT_NOTICE
	CMD_NAME	: 操作结果通知
	DateTime	: 2011/07/28 11:13	
	Description	: 操作结果通知命令
	Author		: hf
	Note		: NULL
	**************************************************************************************************/
	#define			CMD_OPER_RESULT_NOTICE 					501				//操作结果通知命令ID
	
	//操作类型
	typedef enum EnumControlType
	{
		CONTROL_TYPE_PTZ = 1			//云台控制
	}EnumControlType;

	typedef	struct StruCmdOperResultNotice 
	{
		INT32		iOperResult;				//操作结果，参见EnumErrorCode
		INT32		iOperType;					//操作类型，参见EnumControlType
		INT32		iPlatformID;				//要操作的设备平台ID
		INT32		iDevID;						//要操作的设备ID
		INT32		iChnID;						//要操作的通道ID,未涉及到通道的操作时填-1
		INT32		iChnType;					//要操作的通道类型ID，未涉及到通道的操作时填-1
	}StruCmdOperResultNotice,*StruCmdOperResultNoticePtr;

	/**************************************************************************************************
		CMD_ID		: CMD_DATA_VERSION_REQUEST
		CMD_NAME	: 获取数据版本号
		DateTime	: 2010/11/3 10:41	
		Description	: 获取所有数据版本号信息
		Author		: Liujs
		Note		: NULL
	**************************************************************************************************/
	#define			CMD_DATA_VERSION_REQUEST			550				// 获取指定数据的版本请求命令

	// 数据版本类型
	typedef		enum		EnumDataTypeVersion
	{
		D_T_V_ALL_DEVICE_LIST	= 0,				// 所有数据类型
		D_T_V_ALARM_RULE=1,							// 告警规则数据版本类型（表TB_LINKAGERULE,TB_DEVALARM,TB_LINKAGETARGET,TB_LINKAGEACTION）
		D_T_V_RECORD_RULE=2,							// 录像规则数据版本类型（表TB_TASKTIME,TB_TASKINFO,TB_TASKTIMESECT,TB_TIMESECT,TB_DEPLOYMENTRULE）
		D_T_V_DEPLOYMENT_DEFENCE=3,					// 布防撤防数据版本类型(表TB_RECORDRULE,TB_TASKINFO,TB_TASKTIMESECT,TB_TIMESECT,TB_TASKTIME)
		D_T_V_ALARM_RULE_TASK=4,						// 告警联动规则任务关联信息数据版本类型（表TB_TASKTIME,TB_TASKINFO,TB_TASKTIMESECT,TB_TIMESECT,TB_TASKLINKRULE）
		D_T_V_SYNTIME_RULE=5,							// 定时校时数据版本类型(表TB_TASKINFO,TB_TIMESYNCHRO)
		D_T_V_DEVICE_INFO=6,							// 设备信息数据版本类型（表TB_DEVINFO）
		D_T_V_CHN_INFO=7,								// 通道信息数据版本类型（表TB_DEVCHN）
		D_T_V_DOMAIN_INFO=8,							// 域信息数据版本类型（表TB_DOMAINMAP）
		D_T_V_SERVICE_INFO=9,							// 服务基本信息数据版本类型（表TB_SERVICEINFO）
		D_T_V_SERVICE_DEV_INFO=10,						// 服务设备关联信息数据版本类型（TB_SERMNG）
		//D_T_V_USER_INFO=11,							// 用户信息数据版本类型（表TB_USERINFO,TB_USERGROUPMAP）
		//D_T_V_RIGHT_INFO=12,							// 权限信息数据版本类型（表TB_RIGHTSROLLMAP,TB_RIGHTSGROUP,TB_RIGHTSDEVMAP,TB_REFERRIGHTSGROUP,TB_CASCADEUSER,TB_DEVINFO,TB_DEVORG,TB_DEVCHNN,TB_DOMAINMAP,TB_ROLERIGHTS）
		D_T_V_USER_INFO=11,							// 用户信息数据版本类型（表TB_V2_USER,TB_V2_ORGANIZE,TB_V2_ORGANIZEDEVICEDOMAIN）
		D_T_V_RIGHT_INFO=12,							// 权限信息数据版本类型（表TB_V2_DEVAUTH_USER,TB_V2_DEVAUTHORIZATION,TB_V2_DEVAUTH_CHN,TB_V2_ROLEUSER,TB_V2_ROLEINFO,TB_V2_ROLERIGHT,TB_V2_RIGHTINFO,TB_V2_RIGHTMAP,TB_V2_TIMETEMPLATE,TB_V2_TIMETEMPLATEPARAM,TB_V2_TIMESPANPARAM）
		D_T_V_PLATFORM_INFO=13,						// 平台信息数据版本类型（表TB_PLATFORMINFO）
		D_T_V_CSS_CONFIG=14,							// CSS配置信息数据版本类型（表TB_CSSCONFIG）
		D_T_V_VMS_CONFIG=15,							// VMS配置信息数据版本类型（表TB_VMSSPLITSCREEN）
		D_T_V_RECORD_BACKUPRULE=16,                    //录像备份规则(TB_RECORDBACKUPRULE)
        D_T_V_VMS_SLAVEINFO=17,                        //VMS的SLave配置
		D_T_V_STORAGE_POLICY = 18,					    //存储策略(TB_DEV_STOPOLICY_REG)
		D_T_V_REPLACE_POLICY=19,					   //替换策略(TB_DEVREPLACEPOL_REG)
		D_T_V_SLAVE_REGITER_INFO=20,				   //获取SLAVE 服务配置信息
		D_T_V_DISK_REGISTER_INFO=21,				   //获取磁盘配置配置信息
		//国标编码版本类型
		D_T_V_GB28181CODE_INFO=22
	}EnumDataTypeVersion;

	// 获取请求的命令数据
	typedef		struct		StruCmdDataVersionRequest
	{
		INT32		iClientType;										// 获取数据版本的客户端类型,参考结构：EnumClientType
		INT32		iDataTypeVersion;									// 获取的数据版本的类型，参考结构，EnumDataTypeVersion
	}StruCmdDataVersionRequest,*StruCmdDataVersionRequestPtr;


	/**************************************************************************************************
		CMD_ID		: CMD_DATA_VERSION_RESPONSE
		CMD_NAME	: 数据版本号回复
		DateTime	: 2010/11/3 11:13	
		Description	: 获取所有数据版本好信息回复命令
		Author		: Liujs
		Note		: NULL
	**************************************************************************************************/
	#define			CMD_DATA_VERSION_RESPONSE			551				// 获取指定数据的版本回复命令

	#define			MAX_DATA_VERSION					0xF0000000		// 数据版本号最大值
	#define			MIN_DATA_VERSION					0x00000001		// 数据版本号最小值
	// 版本号回复
	typedef		struct		StruCmdDataVersionResponse
	{
		INT32		iOperResult;										// 操作结果,参考结构EnumErrorCode	
		INT32		iDataTypeVersion;									// 获取的数据版本的类型，参考结构，EnumDataTypeVersion
		INT32		iDataVersion;										// 内部版本号增加，循环利用，取值范围[MIN_DATA_VERSION,MAX_DATA_VERSION]
	}StruCmdDataVersionResponse,*StruCmdDataVersionResponsePtr;

	/**************************************************************************************************
		CMD_ID		: CMD_PTZ_PRESET_CONTROL
		CMD_NAME	: 云台预置点控制
		DateTime	: 2010/11/3 11:13	
		Description	: 云台预置点操作
		Author		: shidj
		Note		: NULL
	**************************************************************************************************/
	#define			CMD_PTZ_PRESET_CONTROL 						510		   //云台预置点操作命令ID

	// 云台预置点操作类型
	typedef	enum	EnumPTZPreset
	{
		PTZ_SET_PRESET,					// 设置预置点
		PTZ_CLE_PRESET,					// 清除预置点
		PTZ_CLE_ALL_PRESET				// 清除所有预置点
	}EnumPTZPreset;

	// 云台预置点控制
	typedef struct StruCmdPTZPreset
	{	
		INT32	iChannel;									// 通道号
		INT32	iOperType;									// 操作类型,参考枚举EnumPTZPreset
		INT32	iIndex;										// 索引号，当操作类型为PTZ_CLE_ALL_PRESET时，置0
		char    szName[MAX_NAME_LEN];						// 预置点名称，当操作类型为PTZ_CLE_ALL_PRESET或PTZ_CLE_PRESET时，置空串
	}StruCmdPTZPreset,*StruCmdPTZPresetPtr;

	/**************************************************************************************************
		CMD_ID		: CMD_PTZ_PRESET_CONTROL_RESPONSE
		CMD_NAME	: 云台预置点操作回复命令
		DateTime	: 2010/11/3 11:13	
		Description	: 云台预置点操作结果
		Author		: shidj
		Note		: NULL
	**************************************************************************************************/
	#define			CMD_PTZ_PRESET_CONTROL_RESPONSE				511        	//云台预置点操作回复命令ID

	// 云台预置点操作回复命令对应数据结构
	typedef struct StruCmdPTZPresetResponse 
	{
		INT32	iOperResult;										// 操作结果,参考结构EnumErrorCode
		INT32 	iChannel;											// 通道号
		INT32 	iOperType;											// 操作类型
		INT32	iIndex;												// 索引号，当操作类型为PTZ_SET_PRESET时，返回DAS调用SDK所得到的预置点索引号，其他按原值返回
		char    szName[MAX_NAME_LEN];								// 预置点名称，DAS按原值返回给PMS
	}StruCmdPTZPresetResponse,*StruCmdPTZPresetResponsePtr;

	/**************************************************************************************************
		CMD_ID		: CMD_PTZ_PRESET_GET
		CMD_NAME	: 获取预置点
		DateTime	: 2010/11/3 11:13	
		Description	: 云台预置点操作结果
		Author		: shidj
		Note		: NULL
	**************************************************************************************************/
	#define CMD_PTZ_PRESET_GET	512

	typedef struct StruCmdPTZPresetGet
	{
		INT32	iChnID;									// 通道号
	}StruCmdPTZPresetGet,*StruCmdPTZPresetGetPtr;

	/**************************************************************************************************
		CMD_ID		: CMD_PTZ_PRESET_GET_RESPONSE
		CMD_NAME	: 获取预置点命令回复
		DateTime	: 2010/11/3 11:13	
		Description	: 云台预置点操作结果
		Author		: shidj
		Note		: NULL
	**************************************************************************************************/
	#define CMD_PTZ_PRESET_GET_RESPONSE							513

	// 默认状态
	typedef enum EnumDefaultStatus
	{
		DEFAULT_NO	= 0,	// 非默认
		DEFAULT_YES = 1		// 默认
	}EnumDefaultStatus;

	typedef struct StruPreset 
	{
		INT32 iIndex;				//预置点索引号
		INT32 iDefaultPreset;		//是否默认预置点，参见枚举EnumDefaultStatus
		char  szName[MAX_NAME_LEN];	//预置点名称
	}StruPreset,*StruPresetPtr;

	typedef	struct StruCmdPTZPresetGetResponse
	{
		INT32	iResult;							// 回复结果,枚举EnumErrorCode定义
		INT32	iChnID;								// 通道ID
		INT32	iNum;								// 预置点索引号个数
		StruPreset	stPreset[1];					// 预置点信息
	}StruCmdPTZPresetGetResponse,*StruCmdPTZPresetGetResponsePtr;


	/**************************************************************************************************
		CMD_ID		: CMD_PTZ_PRESET_DEFAULT_REQUEST
		CMD_NAME	: 设置默认预置点
		DateTime	: 2011/6/7 11:13	
		Description	: 设置默认预置点
		Author		: shidj
		Note		: NULL
	**************************************************************************************************/
	#define CMD_PTZ_PRESET_DEFAULT_REQUEST						514

	typedef struct StruCmdPTZPreseDefualtRequest
	{
		INT32   iPlatformID;							// 平台ID
		INT32	iDevID;									// 设备ID
		INT32	iChnID;									// 通道ID
		INT32   iIndexID;								// 预置点索引ID
	}StruCmdPTZPreseDefualtRequest,*StruCmdPTZPreseDefualtRequestPtr;

	/**************************************************************************************************
		CMD_ID		: CMD_PTZ_PRESET_GET_RESPONSE
		CMD_NAME	: 设置默认预置点回复
		DateTime	: 2011/6/7 11:13	
		Description	: 设置默认预置点回复
		Author		: shidj
		Note		: NULL
	**************************************************************************************************/
	#define CMD_PTZ_PRESET_DEFAULT_RESPONSE						515

	typedef struct StruCmdPTZPreseDefualtReponse 
	{
		INT32	iResult;							// 回复结果,枚举EnumErrorCode定义
	}StruCmdPTZPreseDefualtReponse,*StruCmdPTZPreseDefualtReponsePtr;

	/**************************************************************************************************
	CMD_ID		: CMD_GET_CRUISE_TRACK_REQUEST
	CMD_NAME	: CMD_GET_CRUISE_TRACK_REQUEST
	DateTime	: 2011/09/05 11:13	
	Description	: 获取巡航轨迹请求
	Author		: hf
	Note		: NULL
	**************************************************************************************************/
	#define	CMD_GET_CRUISE_TRACK_REQUEST						503
	//设备通道信息
	typedef    struct  StruCruiseDevChn
	{
		INT32	iPlatformID;				//平台ID
		INT32	iDevID;						//设备ID
		INT32	iChnID;						//通道ID
	}StruCruiseDevChn,StruCruiseDevChnPtr;
	//
	typedef		struct	StruGetCruiseTrackRequest 
	{
		StruCruiseDevChn   stDevChn;
	}StruGetCruiseTrackRequest,*StruGetCruiseTrackRequestPtr;

	/**************************************************************************************************
	CMD_ID		: CMD_GET_CRUISE_TRACK_RESPONSE
	CMD_NAME	: CMD_GET_CRUISE_TRACK_RESPONSE
	DateTime	: 2011/09/05 11:13	
	Description	: 获取巡航轨迹请求
	Author		: hf
	Note		: NULL
	**************************************************************************************************/
	#define	CMD_GET_CRUISE_TRACK_RESPONSE						504

	typedef		struct  StruCruiseInfo 
	{
		INT32	 iCruiseID;								//巡航轨迹ID
		char	 szCruiseName[MAX_NAME_LEN];			//巡航轨迹名称
		INT32	 iCruiseInterval;						//巡航时间间隔
		INT32	 iCruiseSpeed;							//巡航速度
		INT32	 iCruiseOnce;							//是否只巡航一次
		INT32	 iEnable;								//是否正在执行的巡航轨迹
		INT32	 iCruiseDefault;						//是否默认巡航轨迹
		INT32	 iTrackNum;								//轨迹的预置点个数
		INT32	 iCruiseTrack[1];						//预置点索引；变长
		INT32	 iOldTrackNum;							//旧轨迹的预置点个数
		INT32	 iOldCruiseTrack[1];					//旧预置点索引；变长
	}StruCruiseInfo,*StruCruiseInfoPtr;

	typedef		struct  StruGetCruiseTrackResponse
	{
		INT32			iOperResult;			//操作结果
		INT32			iNum;					//巡航轨迹个数
		StruCruiseInfo	stCruiseInfo[1];		//轨迹信息
	}StruGetCruiseTrackResponse,*StruGetCruiseTrackResponsePtr;

	/**************************************************************************************************
	CMD_ID		: CMD_CRUISE_OPER_REQUEST
	CMD_NAME	: CMD_CRUISE_OPER_REQUEST
	DateTime	: 2011/09/05 11:13	
	Description	: 巡航操作请求
	Author		: hf
	Note		: NULL
	**************************************************************************************************/
	#define	CMD_CRUISE_OPER_REQUEST							505
	// 云台巡航操作类型
	typedef	enum	EnumPTZCruiseTrack
	{
		PTZ_SET_CRUISETRACK,					// 设置巡航数据
		PTZ_CLE_CRUISETRACK,					// 清除巡航数据
		PTZ_START_CRUISETRACK,				    //开始巡航
        PTZ_STOP_CRUISETRACK,                   //停止巡航
		PTZ_GET_STATUS,							//获取云台状态
		PTZ_EDIT_CRUISETRACK                    //编辑巡航路线
	}EnumPTZCruiseTrack;

	// 云台状态
	typedef enum EnumPTZStatus
	{
		PTZ_UNKNOW_STATUS = 0,					// 未知状态
		PTZ_MOTION_STATUS,						// 运动状态
		PTZ_IDLE_STATUS							// 空闲状态
	}EnumPTZStatus;

	// 云台动作
	typedef enum EnumPTZAction
	{
		PTZ_ACTION_PRESET = 0,					// 预置点
		PTZ_ACTION_LINESCAN, 					// 线扫
		PTZ_ACTION_CRUISE,						// 巡航
		PTZ_ACTION_NAVIGATION,					// 巡迹
		PTZ_ACTION_PANCRUISE					// 水平旋转
	};

	typedef		struct  StruCruiseOperRequest 
	{
		INT32						 iOperType;			//操作类型，参考EnumPTZCruiseTrack
		StruCruiseDevChn	          stDevInfo;			//设备信息
		StruCruiseInfo				 stCruiseInfo;		//巡航信息
	}StruCruiseOperRequest,*StruCruiseOperRequestPtr;
  
	
	#define	 CMD_CRUISE_OPER_RESPONSE						 506
     typedef struct StruCruiseOperResponse
	 {
		 INT32          iResult;                //操作结果，参考结构EnumErrorCode
		 INT32          iOperType;              //操作类型，参考结构EnumPTZCruiseTrack
		 INT32			iPTZStatus;				//云台状态, 参考结构EnumPTZStatus
		 INT32			iPTZAction;				//云台动作，参考结构EnumPTZAction
		 INT32			iPtzActionID;			//云台巡航路径ID
		 StruCruiseDevChn	    stDevInfo;	    //设备信息
		 StruCruiseInfo	stCruiseInfo;		    //轨迹信息		 
	 }StruCruiseOperResponse,*StruCruiseOperResponsePtr;

	/**************************************************************************************************
		CMD_ID		: EnumAlarmType
		CMD_NAME	: 告警类型定义
		DateTime	: 2010/11/4 10:24	
		Description	: 告警类型，设备告警类型，服务告警类型
		Author		: Liujs
		Note		: NULL
	**************************************************************************************************/
	typedef		enum	EnumAlarmType
	{
		//--------------------------------------------------------------------------------------------------------------------\\
		// 没有告警填写-1
		NO_ALARM	=	-1,							// 没有告警

		//--------------------------------------------------------------------------------------------------------------------
		// 设备 告警
		A_T_ALARM_IO = 0,							// IO告警

		A_T_ALARM_MD,								// 移动侦测告警

		A_T_ALARM_VIDEO_COVER,						// 视频遮挡告警

		A_T_ALARM_VIDEO_MISSING,					// 视频丢失告警

		A_T_ALARM_VIDEO_SIGNAL_ABNORMAL,			// 视频信号异常告警

		A_T_ALARM_DISK_DAMAGE,						// 设备磁盘损坏告警
		A_T_ALARM_FLASH_FAULT,						// 设备FLASH故障告警
		A_T_ALARM_DISK_FULL,						// 设备磁盘满告警

		//--------------------------------------------------------------------------------------------------------------------
		// CSS 告警
		A_T_ALARM_STORAGE_FULL,						// CSS存储已满告警
		A_T_ALARM_RW_DISK_ERROR,					// CSS读写磁盘出错告警

		A_T_ALARM_CLI,								//客户端告警
		A_T_ALARM_PTN_TRANSMIT_RENEW,				//PTN传输恢复


		//下面是智能分析服务产生的告警
		A_T_ALARM_OVER_BOUND_DETECTOR=100,				   //越界检测
		A_T_ALARM_INTRUSION_RESTRICTEDAREA_DETECTOR,   //进出禁区检测
		A_T_ALARM_VIDEO_SHIFT_DETECTOR,						   //视频移动检测
		A_T_ALARM_HINDER_WANDER_DETECTOR,				       //滞留徘徊检测
		A_T_ALARM_ABANDON_OBJECT_DETECTOR,			      //遗弃物检测
		A_T_ALARM_LOSE_OBJECT_DETECTOR,					      //目标丢失检测
		A_T_ALARM_CROWD_ILLEGALGATHER_DETECTOR,		  //人群聚集检测
		A_T_ALARM_TRAFFIC_ACCIDENT_DETECTOR,                //交通事故检测
		A_T_ALARM_FACE_DETECTOR,                                   //人脸检测
		A_T_ALARM_TPEDESTRIAN_COUNT_DETECTOR,	          //人数统计（倾斜角）
		A_T_ALARM_VPEDESTRIAN_COUNT_DETECTION,          //人数统计（垂直角）
		A_T_ALARM_CAR_COUNT_DETECTOR,                        //车流量统计
		A_T_ALARM_IILEGAL_PARKING_DETECTOR,                 //非法停车检测
		A_T_ALARM_VIOLENT_MOTION_DETECTOR,                //激烈运动检测
		A_T_ALARM_ABNORMAL_RUNNING_DETECTOR,            //异常奔跑检测
		A_T_ALARM_ALGORITHMTYPE_ERROR = 199,              //错误检测 


		A_T_ALARM_RAIL_OUT = 200,				                    //围栏越界报警
		A_T_ALARM_RAIL_OUT_CANCEL = 201                     //取消围栏越界报警
	}EnumAlarmType;



	/**************************************************************************************************
	CMD_ID		: EnumAlarmLevelType
	CMD_NAME	: 告警等级定义
	DateTime	: 2010/11/4 10:30	
	Description	: 告警等级定义
	Author		: Liujs
	Note		: NULL
	**************************************************************************************************/
	typedef		enum	EnumAlarmLevelType
	{
		A_L_UNKNOWN = 0,					// 未知告警
		A_L_NORMAL,							// 一般告警
		A_L_IMPORTMENT,						// 重要告警
		A_L_URGENCY,						// 紧急告警
		A_L_SERIOUS							// 严重告警
	}EnumAlarmLevelType;

	/**************************************************************************************************
	CMD_ID		: EnumLinkageEvent
	CMD_NAME	: 联动动作类型
	DateTime	: 2010/11/4 16:19	
	Description	: 联动动作类型
	Author		: Liujs
	Note		: NULL
	**************************************************************************************************/
	typedef		enum	 EnumLinkageType
	{
		L_E_NOTICE_CLIENT = 0,						//	告警上送客户端
		L_E_GET_STREAM,								//	码流上送
		L_E_PLATFORM_RECORD,						//	平台录像
		L_E_GOTO_PRESET,							//	转到预置点
		L_E_SEND_SMS,								//	发短信通知
		L_E_SEND_EMAIL,								//	发E-Mail通知
		L_E_CANCLE_GUARD,							//	联动撤防动作
		L_E_STREAM_MATRIX,							//	码流上墙
		L_E_CAPTURE_PIC,							//	图像抓拍动作
		L_E_PU_CLUE,								//  前端设备提示动作
		L_E_RECORD_DOWNLOAD,						//	PTN传输恢复录像下载
		L_E_FIXED_MAP,								//	定位到地图
		L_E_VIDEO_ROBIN								//  视频轮询

	}EnumLinkageType;



	/**************************************************************************************************
	CMD_ID		: CMD_ALARM
	CMD_NAME	: 告警命令信息
	DateTime	: 2010/11/4 10:38	
	Description	: 告警命令信息
	Author		: Liujs
	Note		: NULL
	**************************************************************************************************/
	#define			CMD_ALARM						600		//	告警命令

	// 日期时间结构
	typedef	struct StruDateTime 
	{
		INT16		iYear;					// 年份
		INT16		iMonth;					// 月份
		INT16		iDay;					// 天
		INT16		iHour;					// 小时
		INT16		iMinute;				// 分钟
		INT16		iSecond;				// 秒钟
		INT32		iMilliSecond;			// 毫秒
	}StruDateTime,*StruDateTimePtr;

	//告警开始结束标志
	typedef	enum	EnumAlarmSourceTypeFlag
	{
		L_R_F_ALARM_END = 0,				// 告警结束
		L_R_F_ALARM_START					// 告警开始
	}EnumAlarmSourceTypeFlag;

	// 告警源节点项
	typedef	struct StruAlarmSourceItem 
	{
		INT32			iDeviceType;							// 告警源设备类型，参考EnumClientType结构
		INT32			iPlatformID;							// 平台ID
		INT32			iDeviceID;								// 设备ID，服务ID
		INT32			iChannelID;								// 通道ID,如果是服务就：SERVER_ALARM_CHANNEL_ID.
		INT32			iChannelTypeID;							// 通道类型ID，如果是服务就：SERVER_ALARM_CHANNEL_ID.
		INT32			iAlarmType;								// 告警类型，参考EnumAlarmType结构
		INT32			iAlarmLevelType;						// 告警级别，参考EnumAlarmLevelType结构
		INT32			iReserve;								// 告警开始结束标识，1为告警开始，0为告警结束
	}StruAlarmSourceItem,*StruAlarmSourceItemPtr;

	// 告警节点项
	typedef	struct	StruAlarmItem
	{
		StruAlarmSourceItem		stAlarmSourceItem;				// 告警源节点项
		StruDateTime			stDateTime;						// 告警时间
		char					szAlarmDecri[MAX_REMARKS_LEN];	// 告警描述
	}StruAlarmItem,*StruAlarmItemPtr;

	// 告警命令对应结构
	typedef	struct	StruCmdAlarm
	{
		INT32			iClientType;						// 告警传输源客户端类型
		INT32			iAlarmNum;							// 告警个数
		StruAlarmItem	stAlarmItem[MAX_ALARM_ITEM_NUM];	// 告警信息，动态添加
	}StruCmdAlarm,*StruCmdAlarmPtr;

	// 服务告警时候，通道ID：SERVER_ALARM_CHANNEL_ID
	#define			SERVER_ALARM_CHANNEL_ID					-1


	/**************************************************************************************************
	CMD_ID		: CMD_ALARM_RULE_REQUEST
	CMD_NAME	: 告警联动规则请求
	DateTime	: 2010/11/4 15:13	
	Description	: 告警规则请求
	Author		: Liujs
	Note		: NULL
	**************************************************************************************************/
	#define			CMD_ALARM_RULE_REQUEST			601		//	告警联动规则请求		

	// 告警规则请求，数据为空，不需要


	/**************************************************************************************************
	CMD_ID		: CMD_ALARM_RULE_RESPONSE
	CMD_NAME	: 告警规则回复
	DateTime	: 2010/11/4 15:22	
	Description	: 告警规则回复
	Author		: Liujs
	Note		: NULL
	**************************************************************************************************/
	#define			CMD_ALARM_RULE_RESPONSE			602		// 告警规则回复

	// 联动接收目标
	typedef	struct  StruLinkageDest
	{
		INT32		iLinkageDestType;			//	联动接收目标类型,参考结构EnumClientType
		INT32		iPlatformID;				//	联动接收目标平台ID
		INT32		iLinkageDestID;				//  联动接收目标ID(可以是服务ID，也可以是角色ID)
	}StruLinkageDest,*StruLinkageDestPtr;


	// 联动动作
	typedef	struct	StruLinkageEvent 
	{
		INT32		iLinkageFlag;				//	执行标志，参考EnumRunFlag
		INT32		iLinkageEventType;			//	联动动作类型,参考EnumLinkageType
		INT32		iReserve;					//	备注（预留）（4 字节）
		INT32		iLinkageEventLength;		//	联动动作长度（4 字节）
		char*		szLinkageParam;				//  联动动作参数，转换结构见各个告警动作规则结构，存入数据库时结构的各个参数之间用,分隔。
		//----------------------------------------------------------------------------
		//> iLinkageEventType：客户端呈现规则：保留
		//> iLinkageEventType：平台录像动作规则：见结构StruAlarmPlatformRecordAction
		//> iLinkageEventType：码流上送动作规则：见结构StruStreamSendAction
		//> iLinkageEventType：云台控制动作规则：指向云台控制结构
		//> iLinkageEventType：发短信通知动作规则：保留
		//> iLinkageEventType：发E-Mail通知动作规则：保留
		//> iLinkageEventType：联动撤防动作规则：见结构StruCancelGuardAction
	}StruLinkageEvent,*StruLinkageEventPtr;
	

	//告警上送客户端联动规则结构体
	typedef struct StruNoticeClient
	{
		INT32   iSoundId;						//报警声音ID
	}StruNoticeClient,*StruNoticeClientPtr;

	//定位到地图联动规则
	typedef struct StruFixedMapAction
	{
		INT32    iPlatformId;					//联动设备平台ID
		INT32    iDevId;						//联动设备ID  
		INT32    iChannelId;					//联动设备通道ID
		INT32    iChannelTypeId;				//联动设备通道类型ID
	}StruFixedMapAction,*StruFixedMapActionPtr;


	//平台录像动作规则
	typedef		struct		StruAlarmPlatformRecordAction 
	{
		INT32		iPlatformID;				// 接收目标平台ID
		INT32		iDeviceID;					// 接收目标设备ID
		INT32		iChannelID;					// 联动接收目标通道.
		INT32		iChannelTypeID;				// 通道类型ID，如果是服务就：SERVER_ALARM_CHANNEL_ID.
		INT16		iMinRecordTime;				// 最短录像时间
		INT16		iMaxRecordTime;				// 最长录像时间
	}StruAlarmPlatformRecordAction,*StruAlarmPlatformRecordActionPtr;

	//码流上送动作规则
	typedef		struct		StruStreamSendAction 
	{
		INT32		iPlatformID;				// 联动目标平台ID
		INT32		iDeviceID;					// 联动目标设备ID
		INT32		iChannelID;					// 联动目标通道.
		INT32		iChannelTypeID;				// 通道类型ID，如果是服务就：SERVER_ALARM_CHANNEL_ID.
		INT16		iUsedLocalRecord;			// 表示客户端本地录像是否启用，0：禁止，1
		INT16		iDisplayWindowID;			// 指定视频显示窗格，-1为不指定窗格
	}StruStreamSendAction,*StruStreamSendActionPtr;

	//码流上墙动作规则
	typedef		struct		StruStreamMatrixAction 
	{
		INT32		iPlatformID;				// 联动目标平台ID
		INT32		iDeviceID;					// 联动目标设备ID
		INT32		iChannelID;					// 联动目标通道.
		INT32		iChannelTypeID;				// 通道类型ID.
		INT32		iVMSID;						// 电视墙ID
		INT32		iVMSChnID;					// 电视墙逻辑通道ID
	}StruStreamMatrixAction,*StruStreamMatrixActionPtr;

	//动作参数类型标志
	typedef	enum	EnumPTZParamTypeFlag
	{
		L_R_F_PTZ_SPEED = 1,					// 速度
		L_R_F_PTZ_PRESET_INDEX					// 预置点索引
	}EnumPTZParamTypeFlag;
	// 转到预置点命令，发送对应数据结构
	typedef struct StruAlarmCmdGotoPreset
	{
		INT32	iPlatformID;				// 接收目标平台ID
		INT32	iDeviceID;					// 接收目标设备ID
		INT32	iChannelID;					// 联动接收目标通道.
		INT32	iChannelTypeID;				// 通道类型ID，如果是服务就：SERVER_ALARM_CHANNEL_ID.
		INT32	iPresetIndex;				// 预置点索引
	}StruAlarmCmdGotoPreset,*StruAlarmCmdGotoPresetPtr;

	//撤防动作规则
	typedef		struct		StruCancelGuardAction 
	{
		INT32		iPlatformID;				//接收目标平台ID
		INT32		iDeviceID;					//接收目标设备ID
		INT16		iCancelGuardTime;			//撤防时间
	}StruCancelGuardAction,*StruCancelGuardActionPtr;
	
	//图像抓拍动作规则
	typedef		struct		StruAlarmCapturePicAction 
	{
		INT32		iPlatformID;				// 接收目标平台ID
		INT32		iDeviceID;					// 接收目标设备ID
		INT32		iChannelID;					// 联动接收目标通道.
		INT32		iChannelTypeID;				// 通道类型ID，如果是服务就：SERVER_ALARM_CHANNEL_ID.
		INT16		iPicNum;					// 抓拍张数
		INT16		iCaptureInterval;			// 抓拍间隔（以秒为单位）
		INT32		iReserve;					// 图像抓拍保留字段
	}StruAlarmCapturePicAction,*StruAlarmCapturePicActionPtr;

	//前端设备提示动作规则，输出告警，如灯光，警笛等
	typedef		struct		StruAlarmPUClueAction 
	{
		INT32		iPlatformID;				// 接收目标平台ID
		INT32		iDeviceID;					// 接收目标设备ID
		INT32		iChannelID;					// 联动接收目标通道.
		INT32		iChannelTypeID;				// 通道类型ID，如果是服务就：SERVER_ALARM_CHANNEL_ID.
		INT16		iMinClueTime;				// 最短提示时间
		INT16		iMaxClueTime;				// 最长提示时间
	}StruAlarmPUClueAction,*StruAlarmPUClueActionPtr;

	//PTN传输恢复联动录像下载动作规则
	typedef		struct		StruAlarmRecordDownAction
	{
		INT32			iPlatformID;			// 接收目标平台ID
		INT32			iDeviceID;				// 接收目标设备ID
		INT32			iChannelID;				// 联动接收目标通道.
		StruDateTime	stBeginTime;			// 设备下线时间
		StruDateTime	stEndTime;				// 设备上线时间

	}StruAlarmRecordDownAction,*StruAlarmRecordDownActionPtr;
	
	//在告警规则回复中增加告警规则ID和任务ID的对应结构
	typedef	struct  StruLinkageEventEx
	{
		INT64					iLinkRuleID;		//联动规则ID
		StruLinkageEvent		stLinkageEvent;		//联动动作
	}StruLinkageEventEx,*StruLinkageEventExPtr;

	// 告警规则，对应数据结构
	typedef	struct	StruAlarmRuleItem 
	{
		StruAlarmSourceItem		stAlarmSourceItem;							// 告警源信息
		StruLinkageDest			stLinkageDest;								// 告警联动接收目标,《里面需要添加客户端对应的用户名，否则无法联动客户端》
		INT32					iLinkageEventNum;							// 联动动作个数
		StruLinkageEventEx		stLinkageEvent[MAX_LINKAGE_EVENT_NUM];		// 告警联动动作集合
	}StruAlarmRuleItem,*StruAlarmRuleItemPtr;

	// 告警规则回复，对应数据结构
	typedef	struct StruCmdAlarmRuleResponse 
	{
		INT32					iOperResult;								// 操作结果,参考结构EnumErrorCode
		INT32					iAlarmRuleNum;								// 告警规则个数
		StruAlarmRuleItem		stAlarmRuleItem[1];							// 告警规则，动态的添加
	}StruCmdAlarmRuleResponse,*StruCmdAlarmRuleResponsePtr;

	/**************************************************************************************************
	CMD_ID		: CMD_ALARM_LINKAGE_EVENT_NOTICE
	CMD_NAME	: 告警联动事件通知
	DateTime	: 2010/11/5 16:02	
	Description	: 告警联动事件通知
	Author		: Liujs
	Note		: NULL
	**************************************************************************************************/
	#define			CMD_ALARM_LINKAGE_EVENT_NOTICE				603

	// 告警联动事件，对应数据结构
	typedef	struct	StruCmdLinkageEventNotice 
	{
		INT32					iOperResult;								// 操作结果,参考结构EnumErrorCode
		StruLinkageDest			stLinkageDest;								// 告警联动接收目标
		INT64					iAlarmID;									// 告警ID，告警ID为0，表示为告警结束，无告警ID
		StruAlarmSourceItem		stAlarmSourceItem;							// 告警源信息
		StruDateTime			stDateTime;									// 告警时间
		StruLinkageEvent		stLinkageEvent;								// 告警联动动作
	}StruCmdLinkageEventNotice,*StruCmdLinkageEventNoticePtr;


	/**************************************************************************************************
	CMD_ID		: CMD_ALARM_QUERY_REQUEST
	CMD_NAME	: 告警查询请求
	DateTime	: 2010/11/5 16:02	
	Description	: 告警查询请求
	Author		: Liujs
	Note		: NULL
	**************************************************************************************************/
	#define			CMD_ALARM_QUERY_REQUEST				604

	//无效ID
	#define			ALARM_QUERY_INVALID_ID				-1

	//告警查询，查询类型
	typedef enum EnumAlarmQueryType
	{
		QUERY_BY_ID = 0,		//按树节点查询
		QUERY_BY_KEYWORD,		//按关键字（全部（域名称，设备名称，通道名称））查询
		QUERY_BY_KEYWORD_DOMAIN, //按关键字（域名称）查询
		QUERY_BY_KEYWORD_DEVICE,	//按关键字（设备名称）查询
		QUERY_BY_KEYWORD_CHANNEL	//按关键字（通道名称）查询
	}EnumAlarmQueryType;


	// 告警查询请求，对应数据结构，
	typedef	struct	StruCmdAlarmQueryRequest 
	{
		char					szUserName[MAX_NAME_LEN];					// 告警查询的用户名名称
		INT32					iPageIndex;									// 要查询的数据页数，第一次查询填ALARM_QUERY_INVALID_ID
		INT32					iPageSize;									// 查询每一页显示的最大容量
		INT32					iQueryType;									// 查询类型，参考EnumAlarmQueryType
	
		INT32					iAlarmStatus;								// 告警状态,参考EnumAlarmStatusType结构，查询时未赋值填ALARM_QUERY_INVALID_ID
		INT32					iAlarmType;									// 告警类型，参考EnumAlarmType结构，查询时未赋值填ALARM_QUERY_INVALID_ID
		INT32					iAlarmSourceType;							// 告警源类型，查询时未赋值填ALARM_QUERY_INVALID_ID
		INT32					iAlarmLevel;								// 告警级别，查询时未赋值填ALARM_QUERY_INVALID_ID
		StruDateTime			TimeStart;									// 查询开始时间，查询时未赋值填0
		StruDateTime			TimeEnd;									// 查询结束时间，查询时未赋值填0

		//指定树节点查询，未指定时节点ID填ALARM_QUERY_INVALID_ID
		INT32					iPlatformID;								// 平台ID
		INT32					iDomainID;									// 域ID
		INT32					iDeviceID;									// 设备ID
		INT32					iChannelID;									// 通道ID
		INT32					iChannelTypeID;								// 通道类型ID

		//按关键字查询
		char					szKeyName[MAX_REMARKS_LEN];					// 名称关键字（包括设备名称，通道名称，域名称）
		char					szDescriKey[MAX_REMARKS_LEN];				// 告警描述关键字
		
	}StruCmdAlarmQueryRequest,*StruCmdAlarmQueryRequestPtr;

	/**************************************************************************************************
	CMD_ID		: CMD_ALARM_QUERY_RSPONSE
	CMD_NAME	: 告警查询回复
	DateTime	: 2010/11/5 16:02	
	Description	: 告警查询回复
	Author		: Liujs
	Note		: NULL
	**************************************************************************************************/
	#define			CMD_ALARM_QUERY_RSPONSE				605
	
	// 告警查询每一条告警的内容，对应数据结构
	typedef	struct	StruCmdAlarmQueryContent 
	{
		INT64					iAlarmID;								// 告警ID
		INT32					iAlarmStatus;							// 告警状态，参考EnumAlarmStatusType结构
		INT32					iDeviceType;							// 设备类型，参考结构EnumClientType
		INT32					iAlarmType;								// 告警类型，参考EnumAlarmType结构	
		INT32					iAlarmLevel;							// 告警级别
		StruDateTime			stAlarmTime;							// 告警时间
		INT32					iPlatformID;							// 平台ID
		INT32					iDomainID;								// 上一级父域ID
		char					szDomainName[MAX_NAME_LEN];				// 上一级父域名称
		INT32					iDeviceID;								// 设备ID
		char					szDeviceName[MAX_NAME_LEN];				// 设备名称
		INT32					iChannelID;								// 通道ID,如果是服务就：SERVER_ALARM_CHANNEL_ID
		INT32					iChannelTypeID;							// 通道类型ID，如果是服务就：SERVER_ALARM_CHANNEL_ID.
		char					szChannemName[MAX_NAME_LEN];			// 通道名称
		char					szDescriKey[MAX_REMARKS_LEN];			// 告警描述	
		INT32					iAlarmRepeatNum;						// 重复告警次数
		INT32					iAlarmShieldTime;						// 告警过滤时间（单位：秒）
	}StruCmdAlarmQueryContent,*StruCmdAlarmQueryContentPtr;

	// 告警查询回复，对应数据结构
	typedef	struct	StruCmdAlarmQueryResponse 
	{
		INT32						iOperResult;							// 操作结果
		INT32						iTotalAlarmNum;							// 符合此类查询的总条数
		INT32						iPageIndex;								// 此次返回查询数据的页码
		INT32						iAlarmNum;								// 此次发送的条数
		StruCmdAlarmQueryContent	stCmdAlarmQueryContent[1];				// 告警查询到的内容 
	}StruCmdAlarmQueryResponse,*StruCmdAlarmQueryResponsePtr;

	/**************************************************************************************************
	CMD_ID		: CMD_ALARM_CONFIRM
	CMD_NAME	: 告警确认
	DateTime	: 2011/06/08 16:02	
	Description	: 告警确认
	Author		: hf
	Note		: NULL
	**************************************************************************************************/
	#define			CMD_ALARM_CONFIRM				606
	
	//告警状态类型
	typedef enum EnumAlarmStatusType
	{
		ALARM_STATUS_NOT_CONFIRM = 0,		//未确认
		ALARM_STATUS_CONFIRMED,				//已确认
		ALARM_STATUS_CLOSED,				//已关闭
	}EnumAlarmStatusType;
	
	typedef struct StruCmdAlarmConfirmItem
	{
		INT64	iAlarmID;					//告警ID
		INT32	iAlarmStatus;				//告警状态，参考EnumAlarmStatusType结构
	}StruCmdAlarmConfirmItem,*StruCmdAlarmConfirmItemPtr;

	typedef struct StruCmdAlarmConfirm
	{
		INT32			iNum;							// 告警个数
		StruCmdAlarmConfirmItem	stAlarmConfirmItem[MAX_ALARM_ITEM_NUM];	// 告警信息，动态添加
	}StruCmdAlarmConfirm,*StruCmdAlarmConfirmPtr;

	
	
	/**************************************************************************************************
	CMD_ID		: CMD_SER_RECORD_RULE_REQUEST
	CMD_NAME	: 平台录像规则请求
	DateTime	: 2010/11/4 15:31	
	Description	: 平台录像规则请求
	Author		: Liujs
	Note		: NULL
	**************************************************************************************************/
	#define			CMD_SER_RECORD_RULE_REQUEST		650		// 平台录像规则请求

	// 平台录像规则请求，数据为空，不需要

	/**************************************************************************************************
	CMD_ID		: CMD_SER_RECORD_RULE_RESPONSE
	CMD_NAME	: 平台录像规则回复
	DateTime	: 2010/11/5 10:37	
	Description	: 平台录像规则回复
	Author		: Liujs
	Note		: NULL
	**************************************************************************************************/
	#define			CMD_SER_RECORD_RULE_RESPONSE	651		// 平台录像规则回复

	// 时间段结构
	typedef		struct		StruTimeSpanItem 
	{
		INT8		iBeginHour;				// 开始时间，小时,[0-23]
		INT8		iBeginMinute;			// 开始时间，分钟,[0-59]
		INT8		iBeginSecond;			// 开始时间，秒钟,[0-59]
		INT8		iEndHour;				// 结束时间，小时,[0-23]，任务为设备定时时间同步时，该字段填为-1
		INT8		iEndMinute;				// 结束时间，分钟,[0-59]，任务为设备定时时间同步时，该字段填为-1
		INT8		iEndSecond;				// 结束时间，秒钟,[0-59]，任务为设备定时时间同步时，该字段填为-1
	}StruTimeSpanItem,*StruTimeSpanItemPtr;

	// 每天录像规则，最大MAX_TIME_SPAN_NUM
	typedef		struct		StruDayTimeSpanItem 
	{
		INT16				iTimeSpanNum;						// 时间段个数，最大值为：MAX_TIME_SPAN_NUM
		StruTimeSpanItem	stTimeSpanItem[MAX_TIME_SPAN_NUM];	// 时间段个数
	}StruDayTimeSpanItem,*StruDayTimeSpanItemPtr;

	// 星期标志枚举
	typedef		enum		EnumWeekDay
	{
		W_D_SUNDAY,						// 周日
		W_D_MONDAY,						// 周一
		W_D_TUESDAY,					// 周二
		W_D_WEDNESDAY,					// 周三
		W_D_THURSDAY,					// 周四
		W_D_FRIDAY,						// 周五
		W_D_SATURDAY					// 周六
	}EnumWeekDay;

	// 星期每天时段结构
	typedef		struct		StruWeekDayTimeSpanItem 
	{
		INT16				iWeekIndex;							// 星期标志，EnumWeekDay
		StruDayTimeSpanItem	stDayTimeSpanItem;					// 每天录像规则
	}StruWeekDayTimeSpanItem,*StruWeekDayTimeSpanItemPtr;

	// 星期录像规则
	typedef		struct		StruCmdWeekDaySpanItem 
	{
		INT16						iWeekDayNum;							// 每周的天数
		StruWeekDayTimeSpanItem		stWeekDayTimeSpanItem[MAX_WEEK_NUM];	// 具体的每天的时间结构
	}StruCmdWeekDaySpanItem,*StruCmdWeekDaySpanItemPtr;

	//指定日期录像规则
	typedef     struct      StruCmdDateDaySpanItem
	{
		StruDateTime                 stDateBegin;                  //日期开始时间
		StruDateTime                 stDateEnd;                    //日期结束时间
	}StruCmdDateDaySpanItem,*StruCmdDateDaySpanItemPtr;

	//指定月录像规则(暂时不处理)
	typedef     struct       StruMonthDayTimeSpanItem
	{
		INT16						 iDayIndex;                    //月份中的天数
		StruDayTimeSpanItem			 stDayTimeSpanItem;						// 每天录像规则
	}StruMonthDayTimeSpanItem,*StruMonthDayTimeSpanItemPtr;

	typedef     struct      StruCmdMonthDaySpanItem
	{
		INT16						 iDayNum;							// 月指定天数，[1,MAX_NAME_DAY_NUM]
		StruMonthDayTimeSpanItem     stMonthDayTime[MAX_NAME_DAY_NUM];
	}StruCmdMonthDaySpanItem,*StruCmdMonthDaySpanItemPtr;

	// 指定天录像规则(暂时保留)
	typedef		struct		StruNameDayTimeSpanItem 
	{
		INT8						iMonthIndex;							// 月份,[1-12]
		INT8						iDayIndex;								// 月份对应的天数,[1-31]:根据月份确定
		StruDayTimeSpanItem			stDayTimeSpanItem;						// 每天录像规则
	}StruNameDayTimeSpanItem,*StruNameDayTimeSpanItemPtr;

	// 指定天录像规则
	typedef		struct		StruCmdNameDaySpanItem 
	{
		INT16						iNameDayNum;							// 日期指定天数，[1,MAX_NAME_DAY_NUM]
		StruNameDayTimeSpanItem		stNameDayTimeSpanItem[MAX_NAME_DAY_NUM];// 是定天数录像信息结构
	}StruCmdNameDaySpanItem,*StruCmdNameDaySpanItemPtr;


	// 录像规则,时间结构
	typedef		union		UnionCmdSerRcordTimeSpan
	{
		StruCmdWeekDaySpanItem	stCmdWeekRecordTimeSpan;					//每周录像时间段 
		StruCmdDateDaySpanItem  stCmdDateRecordTimeSpan;                    //日期录像时间段
		StruCmdMonthDaySpanItem stCmdMonthRecordTimeSpan;                   //月录像时间段
		StruDayTimeSpanItem     stCmdDayRecordTimeSpan;                     //天录像时间段
	}UnionCmdSerRcordTimeSpan;

	// 设备通道索引结构
	typedef		struct		StruDevChnIndex 
	{
		INT32				iPlatformID;						// 平台ID
		INT32				iDevID;								// 设备ID
		INT16				iChannelNum;						// 通道个数,最大值是MAX_CHANNEL_NUM
		INT8				iChannels[MAX_CHANNEL_NUM];			// 通道ID
	}StruDevChnIndex,*StruDevChnIndexPtr;

	// 每天，每周启用标志
	typedef		enum		EnumSerRecordTimeFlag
	{
		S_R_T_WEEKLY=1,			// 每周
		S_R_T_NAMEED,			// 日期
		S_R_T_MONTH,            //月
		S_R_T_DAY               //天
	}EnumSerRecordTimeFlag;

	// 录像规则记录，对应结构体
	typedef		struct		StruSerRecordItem 
	{	
		INT16						iRecordTimeFlag;				// 录像时间段标志，参考结构EnumSerRecordTimeFlag
		UnionCmdSerRcordTimeSpan	unSerRecordTimeSpan;			// 如果录像标志：S_R_T_DAY--->StruDayTimeSpanItem;S_R_T_WEEKLY--->StruCmdWeekDaySpanItem;
																	// S_R_T_MONTH-->StruCmdMonthDaySpanItem;S_R_T_NAMEED->StruCmdDateDaySpanItem
		INT32						iDevNum;						// 设备个数
		StruDevChnIndex				stDevChanIndex[MAX_DEV_NUM];	// 设备通道映射信息
	}StruSerRecordItem,*StruSerRecordItemPtr;

	// 录像规则请求回复命令，对应结构体
	typedef		struct		StruCmdSerRecordResponse 
	{
		INT32						iOperResult;					// 回复结果,枚举EnumErrorCode定义
		INT32						iRecordRuleNum;					// 规则记录个数
		StruSerRecordItem			stSerRecordItem[1];				// 录像规则记录集，支持多个，动态添加
	}StruCmdSerRecordResponse,*StruCmdSerRecordResponsePtr;

	/**************************************************************************************************
	DateTime		: 2011/07/20 9:04	
	Author 			: hf      
	Description		: CSS参数配置获取
	Note			: 
	**************************************************************************************************/
	#define		CMD_CSS_CONFIG_PARAM_REQUEST				652

	/**************************************************************************************************
	DateTime		: 2011/07/20 9:04	
	Author 			: hf      
	Description		: CSS参数配置回复
	Note			: 
	**************************************************************************************************/
	#define		CMD_CSS_CONFIG_PARAM_RESPONSE				653

	//CSS存储策略枚举
	typedef	enum	EnumStoragePolicyType
	{
		POLICY_TYPE_CIRCLE_COVER = 1,	//循环覆盖
		POLICY_TYPE_OVERRUN_ALARM	//超限告警
	}EnumStoragePolicyType;

	//CSS配置参数类型
	typedef	enum	EnumCSSConfigParamType
	{
		CSS_ALARM_RECORD_SHIELD_TIME = 1,	//告警录像过滤时间，单位:秒
		CSS_SERIES_RECORD_FILE_SIZE,		//连续录像文件大小，即分割时间，单位：分钟
		CSS_FILE_MIN_RECORD_TIME,			//文件最短录像时间，单位：分钟
		CSS_GET_DISKINFO_INTERVAL,			//CSS的Master获取磁盘信息的时间间隔，单位：秒
		CSS_STORAGE_POLICY,					//CSS磁盘满的存储策略，见枚举EnumStoragePolicyType
		CSS_FILE_LOCK_TIME,					//文件锁定时间，单位：小时
		CSS_DISK_USE_PERCENT				//磁盘使用最大百分比
	}EnumCSSConfigParamType;

	// CSS配置参数的返回名称（PMS与CSS内部使用）
	const		StruRetCodeInfo		conStCSSConfigInfo[]={
		{	CSS_ALARM_RECORD_SHIELD_TIME,		"CSS_ALARM_RECORD_SHIELD_TIME"},
		{	CSS_SERIES_RECORD_FILE_SIZE,		"CSS_SERIES_RECORD_FILE_SIZE"},
		{	CSS_FILE_MIN_RECORD_TIME,			"CSS_FILE_MIN_RECORD_TIME"},
		{	CSS_GET_DISKINFO_INTERVAL,			"CSS_GET_DISKINFO_INTERVAL"},
		{	CSS_STORAGE_POLICY,					"CSS_STORAGE_POLICY"},
		{	CSS_FILE_LOCK_TIME,					"CSS_FILE_LOCK_TIME"},
		{	CSS_DISK_USE_PERCENT,				"CSS_DISK_USE_PERCENT"},

	};

	//CSS参数配置回复
	typedef	struct StruCmdCSSConfigResponse 
	{
		INT32		iOperResult;		//回复结果
		INT32		iLength;			//参数长度
		char		szParam[1];			//参数字符串
	}StruCmdCSSConfigResponse,*StruCmdCSSConfigResponsePtr;



	/**************************************************************************************************
	DateTime		: 2011/07/20 14:40	
	Author 			: hf      
	Description		: 获取当前录像任务状态
	Note			: 
	**************************************************************************************************/
	#define		CMD_RECORD_TASK_STATUS_REQUEST				654
	
	/**************************************************************************************************
	DateTime		: 2011/07/20 14:40	
	Author 			: hf      
	Description		: 当前录像任务状态回复
	Note			: 
	**************************************************************************************************/
	#define		CMD_RECORD_TASK_STATUS_RESPONSE				655

	typedef	enum	EnumRecordTaskType
	{
		TASK_TYPE_PLAN = 1,	//计划录像
		TASK_TYPE_ALARM,	//告警录像
		TASK_TYPE_MANUAL	//手动录像
	}EnumRecordTaskType;

	//录像任务项
	typedef	struct	StruRecordStatusTaskItem
	{
		char			szIPAddr[MAX_IP_ADDR_LEN];		//录像的slave的IP
		INT32			iPort;							//录像的slave的端口
		INT32			iPlatformID;					//执行录像任务的平台ID
		INT32			iDevID;							//执行录像任务的设备ID，如果是-1，代表无效
		INT32			iChannelID;						//执行录像任务的通道ID，如果是-1，代表无效
		INT32			iRecordType;					//正在录像的任务类型，参见枚举EnumRecordTaskType
		StruDateTime	stRecordStartTime;				//录像开始时间
		StruDateTime	stRecordEndTime;				//录像结束时间
		char			szDiskInfo[MAX_NAME_LEN];		//录像所在磁盘的名称
		INT64 iTaskID;									//任务ID
		char szUserName[64];							//手动录像时的用户， "" 表示无效

	}StruRecordStatusTaskItem,*StruRecordStatusTaskItemPtr;

	//请求通道项信息
	typedef struct StruRecordStatusRequestItem
	{
		INT32 iDevID;						//设备id，如果是-1，代表无效
		INT32 iChanID;						//通道id，如果是-1，代表无效
	}StruRecordStatusRequestItem, *StruRecordStatusRequestItemPtr;

	//当前录像任务状态请求
	typedef struct StruCmdRecordStatusTaskRequest 
	{
		int iTaskType;								  //任务类型  -1 表示所有， 枚举EnumRecordTaskType 计划录像， 手动录像， 告警录像
		char szUserName[MAX_NAME_LEN];                //手动录像时的用户， "" 表示无效
		INT32 iPlatformID;							  // 设备所属平台ID
		int iChanArrayNum;                            //请求通道个数，-1 表示查询所有
		StruRecordStatusRequestItem iChanArray[1];    //通道数组，变长
	}StruCmdRecordStatusTaskRequest,*StruCmdRecordStatusTaskRequestPtr;

	//当前录像任务状态回复
	typedef	struct StruCmdRecordStatusTaskResponse 
	{
		INT32						iOperResult;						//回复结果
		INT32						iNum;								//任务条数
		StruRecordStatusTaskItem	stRecordTaskItem[1];				//录像任务信息
	}StruCmdRecordStatusTaskResponse,*StruCmdRecordStatusTaskResponsePtr;

	/**************************************************************************************************
	DateTime		: 2010/8/4 9:04	
	Author 			: shidj      
	Description		: 撤防布防规则获取
	Note			: 
	**************************************************************************************************/
	#define		CMD_DEPLOYMENT_DEFENCE_REQUEST				530

	typedef		struct		StruCmdDeploymentDefenceGet 
	{
		INT32 iClientType;		//	客户端类型
		INT32 iClientID;		//	客户端ID
	}StruCmdDeploymentDefenceGet,*StruCmdDeploymentDefenceGetPtr;

	/**************************************************************************************************
	DateTime		: 2010/8/4 9:04	
	Author 			: shidj      
	Description		: 撤防布防规则获取回复
	Note			: 
	**************************************************************************************************/
	#define		CMD_DEPLOYMENT_DEFENCE_RESPONSE				531

	typedef	enum EnumDeploymentTimeFlag						//	布防撤防类型
	{
		S_R_T_WEEKLY_OLD,
		S_R_T_NAMEED_OLD
	}EnumDeploymentTimeFlag;

	typedef	UnionCmdSerRcordTimeSpan	UnionCmdDeploymentTimeSpan;					//	布防撤防规则
	
	//布防撤防设备信息
	typedef	struct StruDeploymentDevInfo
	{
		INT32				iPlatformID;						// 平台ID,本平台默认为0
		INT32				iDevID;								// 设备ID
	}StruDeploymentDevInfo,*StruDeploymentDevInfoPtr;

	typedef	UnionCmdSerRcordTimeSpan	UnionCmdTaskInfoTimeSpan;					//	任务时间段

	typedef		struct StruTaskTimeInfo
	{
		INT32						iTimeID;			//任务时间段ID
		INT32						iTimeFlag;			//任务时间段类型,参考结构EnumDeploymentTimeFlag
		UnionCmdTaskInfoTimeSpan	unTaskInfoTime;		//任务时间段
	}StruTaskTimeInfo,*StruTaskTimeInfoPtr;	
	
	//布防撤防规则项
	typedef struct StruDeploymentDefenceItem	
	{
		INT32							iTaskID;		//	任务ID
		INT32							iNum;			//	设备个数
		StruDeploymentDevInfo			DevInfoList[1];	//  设备信息列表
		INT32							iTimeNum;		//  任务时间段个数
		StruTaskTimeInfo				stTimeInfo[1];	//	任务时间段		
	}StruDeploymentDefenceItem,*StruDeploymentDefenceItemPtr;

	//	撤防布防规则获取回复结构
	typedef		struct  StruCmdDeploymentDefenceResponse
	{
		INT32							iOperResult;					//	回复结果,枚举EnumErrorCode定义
		INT32							iClientType;					//	客户端类型
		INT32							iClientID;						//	客户端ID
		INT32							iDepoymentNum;					//  规则数目
		StruDeploymentDefenceItem		stDeploymentDefenceItem[1];		//  布防撤防规则
	}StruCmdDeploymentDefenceResponse,*StruCmdDeploymentDefenceResponsePtr;

	/**************************************************************************************************
	DateTime		: 2011/02/20 9:04	
	Author 			: hf      
	Description		: 布防撤防命令
	Note			: 
	**************************************************************************************************/
	#define		CMD_DEPLOYMENT_DEFENCE					532
	// 布防撤防标识
	typedef		enum		EnumCmdDefenceFlag
	{
		CMD_DEFENCE_CANCEL,			// 撤防
		CMD_DEFENCE_START			// 布防
	}EnumCmdDefenceFlag;

	typedef		struct	StruCmdDeploymentDefence 
	{
		INT32							iCmdType;			//命令类型，详细见结构EnumCmdDefenceFlag
		INT32							iNum;				//设备个数
		StruDeploymentDevInfo			DevInfoList[1];		//设备信息列表
	}StruCmdDeploymentDefence,*StruCmdDeploymentDefencePtr;

	/**************************************************************************************************
	DateTime		: 2011/02/20 9:04	
	Author 			: hf      
	Description		: 布防撤防命令回复
	Note			: 
	**************************************************************************************************/
	#define		CMD_DEFENCE_COMMAND_RESPONSE			533

	typedef		struct StruCmdDefenceCommandResponse
	{
		INT32							iOperResult;		//回复结果,枚举EnumErrorCode定义
		INT32							iCmdType;			//操作类型，详细见结构EnumCmdDefenceFlag
		StruDeploymentDevInfo			stDevInfo;		//设备信息
	}StruCmdDefenceCommandResponse,*StruCmdDefenceCommandResponsePtr;
	
	/**************************************************************************************************
	CMD_ID		: CMD_GET_LINKAGE_TASK_REQUEST
	CMD_NAME	: 获取联动任务信息请求
	DateTime	: 2011/09/09 11:02	
	Description	: 获取联动任务信息请求
	Author		: hf
	Note		: NULL
	**************************************************************************************************/
	#define		CMD_GET_LINKAGE_TASK_REQUEST			607


	/**************************************************************************************************
	CMD_ID		: CMD_GET_LINKAGE_TASK_RESPONSE
	CMD_NAME	: 获取任务信息回复
	DateTime	: 2011/09/09 11:02	
	Description	: 获取任务信息回复
	Author		: hf
	Note		: NULL
	**************************************************************************************************/
	#define		CMD_GET_LINKAGE_TASK_RESPONSE			608

	//任务信息
	typedef		struct	StruTaskInfoItem 
	{
		INT64						iTaskID;			//任务ID
		INT32						iTaskType;			//任务类型ID
		INT32						iTaskEnale;			//任务使能标识
		INT32						TaskTimeNum;		//任务时间个数
		StruTaskTimeInfo			stTaskInfo[1];		//任务信息
	}StruTaskInfoItem,*StruTaskInfoItemPtr;

	//任务联动规则信息结构,由于任务ID和联动规则是多对多的关系
	//为了简化存取的复杂度，由任务ID去对应到规则ID
	typedef		struct	StruTaskLinkRuleInfo 
	{
		INT32				iLinkRuleNum;	//联动规则个数
		INT64				iLinkRuleID[1];	//联动规则ID
		StruTaskInfoItem	stTaskInfo;		//任务信息
	}StruTaskLinkRuleInfo,*StruTaskLinkRuleInfoPtr;

	//获取任务信息回复
	typedef		struct	StruLinkageTaskInfoResponse 
	{
		INT32					iOperResult;			//操作结果
		INT32					iNum;					//联动任务信息个数
		StruTaskLinkRuleInfo	stTaskLinkRuleInfo[1];	//联动任务信息列表
	}StruLinkageTaskInfoResponse,*StruLinkageTaskInfoResponsePtr;
	
	/**************************************************************************************************
		CMD_ID		: CMD_LEVEL_NODE_DATA_REQUEST
		CMD_NAME	: CMD_LEVEL_NODE_DATA_REQUEST
		DateTime	: 2010/12/6 9:11	
		Description	: 分层节点数据（设备树）获取
		Author		: Liujs
		Note		: 客户端SDK获取新的设备树的数据
	**************************************************************************************************/
	#define		CMD_LEVEL_NODE_DATA_REQUEST						700

	// 发送分层请求数据格式命令信息
	typedef		struct		StruCmdNodeDataRequest 
	{
//		INT32					iClientType;					// 发起请求的客户端类型,参考结构：EnumClientType
		char					szNodeId[MAX_NODE_LEN];			// 平台ID_域ID_设备ID_通道ID,不足的部分用#替代，例如：0_#_#_#,第一次取，由PMS命令解析模块处理成内部结构
		INT32					iStartIndex;					// 其实索引，从0开始，-1：表示全部获取
		INT32					iNodeNum;						// 节点数目，如果索引为-1，条数无效
	}StruCmdNodeDataRequest,*StruCmdNodeDataRequestPtr;

	
	/**************************************************************************************************
		CMD_ID		: CMD_LEVEL_NODE_DATA_RESPONSE
		CMD_NAME	: CMD_LEVEL_NODE_DATA_RESPONSE
		DateTime	: 2010/12/6 9:24	
		Description	: 分层获取设备树回复命令
		Author		: Liujs
		Note		: 客户端SDK获取新的设备树的数据
	**************************************************************************************************/
	#define		CMD_LEVEL_NODE_DATA_RESPONSE				701

	// 分层取数据时该节点是否有子节点标识
	typedef enum EnumSubNodeFlag
	{
		SUBNODE_NO = 0,						// 没有子节点
		SUBNODE_YES = 1						// 有子节点
	}EnumSubNodeFlag;


	// 分层数据节点信息结构
	typedef		struct		StruNodeItemInfo
	{
		INT32			iNodeType;						// 节点类型，参考EnumNodeType结构
		char			szNodeId[MAX_NODE_LEN];			// 平台ID_域ID_设备ID_通道ID,不足的部分用#替代，例如：0_#_#_#,第一次取
		char			szNodeName[MAX_NAME_LEN];		// 节点名称
		INT32			iNodeStatus;					// 当节点类型是设备时，参考EnumDevStatusType结构；当节点类型是通道时，EnumChnStatusType结构
		INT32			iSubNodeFlag;					// 表示该节点是否存在子节点，参考EnumSubNodeFlag结构
		// 用户权限,位标识,每个字符标识一项权限，目前暂时分为0-9个等级，
		// 权限均为：0表示无权限，1表示有权限。 字符顺序标识意义如下：
		// 1：实时视频浏览
		// 2：语音对讲
		// 3：平台回放
		// 4：前端回放
		// 5：业务报警
		// 6：PTZ控制,   0:表示无权限，1-9表示云台控制的优先级；
		// 7：前端抓拍
		char			szPrivilege[MAX_PRIV_LEN];		
	}StruNodeItemInfo,*StruNodeItemInfoPtr;

	// 分层获取设备树节点信息结构
	typedef		struct		StruCmdNodeInfo 
	{
		INT32					iOperResult;					// 回复结果,枚举EnumErrorCode定义
		char					szUpNodeId[MAX_NODE_LEN];		// 上层节点ID:平台ID_域ID_设备ID_通道ID,不足的部分用#替代，例如：0_#_#_#,第一次取
		INT32					iNodeSum;						// 节点总数，所有节点的总数目
		INT32					iNodeNum;						// 节点个数，数据包实际个数发送
		StruNodeItemInfo 		stCmdNodeItemInfo[MAX_DEV_NUM];	// 节点列表信息，参考StruNodeItemInfo结构
	}StruCmdNodeInfo,*StruCmdNodeInfoPtr;

	/**************************************************************************************************
		CMD_ID		: CMD_ALL_NODE_DATA_REQUEST
		CMD_NAME	: CMD_ALL_NODE_DATA_REQUEST
		DateTime	: 2013/2/6 10:25
		Description	: 获取指定节点下面的所有数据（设备树）获取
		Author		: hongsg
		Note		: 添加这个指令主要是为了一次获取所有组织结构树以加强通信效率，防止多用户频繁请求bms效率慢
	**************************************************************************************************/
	#define		CMD_ALL_NODE_DATA_REQUEST						705

	// 发送请求数据格式命令信息
	typedef		struct		StruCmdAllNodeDataRequest 
	{
		char					szNodeId[MAX_NODE_LEN];			// 平台ID_域ID_设备ID_通道ID,不足的部分用#替代，例如：0_#_#_#,第一次取，由PMS命令解析模块处理成内部结构
		INT32					iStartIndex;					// 其实索引，从0开始，-1：表示全部获取
		INT32					iNodeNum;						// 节点数目，如果索引为-1，条数无效
	}StruCmdAllNodeDataRequest,*StruCmdAllNodeDataRequestPtr;



		/**************************************************************************************************
		CMD_ID		: CMD_ALL_NODE_DATA_RESPONSE
		CMD_NAME	: CMD_ALL_NODE_DATA_RESPONSE
		DateTime	: 2013/2/6 10:25	
		Description	: 获取指定节点下面的所有数据(设备树)回复命令
		Author		: hongsg
		Note		: 添加这个指令主要是为了一次获取所有组织结构树以加强通信效率，防止多用户频繁请求bms效率慢
		              此处采用json的数据通信交互方式，主要是出于与不同语言（如java）通信解析的方便性及扩平台考虑，
					  可调试性都会增强；缺点是通信效率有下降，特别是大数据量的时候。
	**************************************************************************************************/
	#define		CMD_ALL_NODE_DATA_RESPONSE			     	706


	/**************************************************
	结构树JSon结构,整个数据用root表示
	组织结构树的每个节点都用一个node表示，node字段至少包括：
	Node["ID"]     节点id
	Node["Name"]   节点名称
	Node["Type"]   节点类型 取值范围：DOMAIN、VIDEO_DEV、VIDEO_CHN、INPUT_CHN、OUTPUT_CHN ，分别代表域，前端设备，视频通道，输入通道，输出通道

	当Node["Type"]为VIDEO_DEV，也就是前端设备类型时，node还包含下面字段:
	  Node["Status"]  节点状态，取值范围：ONLINE, OFFLILNE ，代表设备是否在线
	  Node["AllTimeTemp"]：授权设备有效的时间模板ID，此为数组
			jTimeTempID["TimeTemplate"]:有效的时间模板ID

	当Node["Type"]为VIDEO_CHN、INPUT_CHN、OUTPUT_CHN，也就是相关通道类型时，node还包含下面字段:
	  Node["Status"]  节点状态，取值范围：OK, FAIL , 代表通道是否处于正常工作状态
	  Node["Privilege"] 节点相关权限，相关请查看上面StruNodeItemInfo结构的szPrivilege定义注释
	  Node["Facade"] 外观（0枪机、1球机）


    ************************************************/

	/**************************************************************************************************
	CMD_ID		: CMD_GET_DOMAIN_MAP_REQUEST
	CMD_NAME	: CMD_GET_DOMAIN_MAP_REQUEST
	DateTime	: 2011/2/21 9:04	
	Description	: 获取域映射表请求
	Author		: hf
	Note		: 获取域映射表请求
	**************************************************************************************************/
	#define		CMD_GET_DOMAIN_MAP_REQUEST			710


	/**************************************************************************************************
	CMD_ID		: CMD_GET_DOMAIN_MAP_RESPONSE
	CMD_NAME	: CMD_GET_DOMAIN_MAP_RESPONSE
	DateTime	: 2011/2/21 9:04	
	Description	: 获取域映射表回复
	Author		: hf
	Note		: 获取域映射表回复
	**************************************************************************************************/
	#define		CMD_GET_DOMAIN_MAP_RESPONSE			711

	typedef	struct StruDomainMapItem 
	{
		INT32		iDevTypeID;				//设备类型ID
		INT32		iDevID;					//设备ID
		char		szAllDomainID[MAX_NAME_LEN_256]; //所有域名称
	}StruDomainMapItem,*StruDomainMapItemPtr;

	typedef	struct StruDomainMapResponse 
	{
		INT32	iOperResult;					//回复结果
		INT32	iNum;							//映射表的条数
		StruDomainMapItem stDomainMapItem[1];	//域映射数据
	}StruDomainMapResponse,*StruDomainMapResponsePtr;

	/**************************************************************************************************
		CMD_ID		: CMD_RECORD_FILE_RETRIEVAL_REQUEST
		CMD_NAME	: CMD_RECORD_FILE_RETRIEVAL_REQUEST
		DateTime	: 2011/2/21 9:04	
		Description	: 录像文件检索请求
		Author		: sdj
		Note		: Client发起录像文件检索请求
	**************************************************************************************************/
	#define		CMD_RECORD_FILE_RETRIEVAL_REQUEST			721

	// 存储类型
	typedef enum EnumStorageType
	{

		STORAGE_TYPE_CSS_SER = 0,		// css服务器
		STORAGE_TYPE_DEVICE			// 前端设备
	}EnumStroageType;

	// 媒体类型
	typedef enum EnumMediaType
	{
		MEDIA_TYPE_ALL = -1,	// 所有类型
		MEDIA_TYPE_VIDEO,		// 视频
		MEDIA_TYPE_GRAPHIC,		// 图像
		MEDIA_TYPE_AV			// 音视频
	}EnumMediaType;

	// 录像原因
	typedef enum EnumRecordReason
	{

		RECORD_REASON_ALL = -1,		// 所有录像
		RECORD_REASON_ALARM,		// 告警录像
		RECORD_REASON_PLAN,			// 计划录像
		RECORD_REASON_MANUAL,		// 手动录像
		RECORD_REASON_DOWNLOAD,		// 下载录像
		RECORD_REASON_BACKUP        //录像文件备份
	}EnumRecordReason;

	// 是否锁定时间
	typedef enum EnumTimeLock
	{

		TIME_LOCK_INVALID = 0,			// 该锁无效,即不支持锁定
		TIME_LOCK_OFF,				// 没有被锁定
		TIME_LOCK_ON,				// 被锁定

	}EnumTimeLock;

	// 排序方式
	typedef enum EnumSortWay
	{
		SORT_WAY_ASC = 0,				// 升序
		SORT_WAY_DEC				// 降序

	}EnumSortWay;


	//录像文件检索请求命令结构
	typedef struct StruRecordFileRetrievalRequest
	{
		INT32 iPlatformID;						// 设备所属平台ID
		INT32 iNodeID;							// 节点ID
		INT32 iChnID;							// 通道号 ID，如果查询节点类型为通道，则表示通道号，其他，填-1
		INT16 iNodeType;						// 节点类型ID，参考枚举类型EnumNodeType
		INT16 iStorageType;						// 存储类型，参考枚举类型EnumStorageType
		INT16 iMediaType;						// 媒体类型，参考枚举类型EnumMediaType
		INT16 iRelevantReason;					// 录像原因，参考枚举类型EnumRecordReason
		StruDateTime stRcdStartTime;			// 录像开始时间
		StruDateTime stRcdEndTime;			    // 录像结束时间
		INT16 iLocked;							// 是否锁定，参考枚举类型EnumTimeLock
		INT16 iSortWay;							// 排序方式，参考枚举类型EnumSortWay
		INT64 iStartRowIndex;					// 表示从哪一条记录开始查询，如果是-1，则表示从第一条记录开始查询
		INT32 iWantedRcdNum;					// 表示本次查询所要返回的记录的条数
	}StruRecordFileRetrievalRequest,*StruRecordFileRetrievalRequestPtr;

/**************************************************************************************************
		CMD_ID		: CMD_RECORD_FILE_RETRIEVAL_V2_REQUEST
		CMD_NAME	: CMD_RECORD_FILE_RETRIEVAL_V2_REQUEST
		DateTime	: 2013/7/16 10:04	
		Description	: 录像文件检索请求
		Author		: cuiyt
		Note		: Client发起录像文件检索请求
	**************************************************************************************************/
	#define		CMD_RECORD_FILE_RETRIEVAL_V2_REQUEST			1024
	struct   StruRecord
	{
		INT32 iPlatformID;						// 设备所属平台ID
		INT32 iNodeID;							// 节点ID
		INT32 iChnID;							// 通道号 ID
	};

	//录像文件检索请求命令结构V2
	typedef struct StruRecordFileRetrievalRequest_V2
	{		
		INT16 iStorageType;						// 存储类型，参考枚举类型EnumStorageType
		INT16 iMediaType;						// 媒体类型，参考枚举类型EnumMediaType
		INT16 iRelevantReason;					// 录像原因，参考枚举类型EnumRecordReason
		StruDateTime stRcdStartTime;			// 录像开始时间
		StruDateTime stRcdEndTime;			    // 录像结束时间
		INT16 iLocked;							// 是否锁定，参考枚举类型EnumTimeLock
		INT16 iSortWay;							// 排序方式，参考枚举类型EnumSortWay
		INT64 iStartRowIndex;					// 表示从哪一条记录开始查询，如果是-1，则表示从第一条记录开始查询
		INT32 iWantedRcdNum;					// 表示本次查询所要返回的记录的条数
		INT32 iCount;
		StruRecord Record[1];
	}StruRecordFileRetrievalRequest_V2,*StruRecordFileRetrievalRequestPtr_V2;

	/**************************************************************************************************
		CMD_ID		: CMD_RECORD_FILE_RETRIEVAL_RESPONSE
		CMD_NAME	: CMD_RECORD_FILE_RETRIEVAL_RESPONSE
		DateTime	: 2011/2/21 9:04	
		Description	: CMD_RECORD_FILE_RETRIEVAL_RESPONSE
		Author		: sdj
		Note		: Client发起录像文件检索请求回复
	**************************************************************************************************/
	#define		CMD_RECORD_FILE_RETRIEVAL_RESPONSE			722

	// 录像文件信息
	typedef struct StruRecordFileInfo
	{
		INT64 iRowIndex;						// 录像文件所在行索引
		INT32 iPlatformID;						// 设备所属平台ID
		INT32 iDevID;							// 设备ID
		INT32 iChnID;							// 通道ID
		INT16 iStorageType;						// 存储类型，参考枚举类型EnumStorageType
		INT16 iMediaType;						// 媒体类型，参考枚举类型EnumMediaType
		INT16 iRelevantReason;					// 录像原因，参考枚举类型EnumRecordReason
		INT16 iLocked;							// 是否锁定,-1	表示该功能无效;0	表示当前没有被锁定;1	表示当前被锁定
		UINT32 uiRcdFileSize;					// 录像文件文件大小，字节计算
		StruDateTime stRcdStartTime;			// 录像开始时间
		StruDateTime stRcdEndTime;			    // 录像结束时间
		StruDateTime stLockStartTime;			// 录像锁定开始时间
		StruDateTime stLockEndTime;			    // 录像锁定结束时间
		char strRcdFileID[MAX_ID_STRING_LEN];	// 录像文件ID	
	}StruRecordFileInfo;


	//录像文件检索回复命令结构
	typedef struct StruRecordFileRetrievalResponse
	{
		INT32					iOperResult;					// 回复结果,枚举EnumErrorCode定义
		INT32					iVersion;						// 主要是用于兼容不同存储介质录像文件ID不同表示方法，暂时填0
		UINT32                  iTotals;                        //总体条数
        INT32                   iRcdNum;						// 实际记录条数
		StruRecordFileInfo      stRcdContent[1];				// 记录内容,变长，大小由条数决定
	}StruRecordFileRetrievalResponse,*StruRecordFileRetrievalResponsePtr;
	/**************************************************************************************************
		CMD_ID		: CMD_RECORD_FILE_OPERRATE_REQUEST
		CMD_NAME	: CMD_RECORD_FILE_OPERRATE_REQUEST
		DateTime	: 2011/2/21 9:04	
		Description	: 录像文件操作请求
		Author		: sdj
		Note		: Client发起录像文件操作请求
	**************************************************************************************************/
	#define		CMD_RECORD_FILE_OPERATE_REQUEST			723
	#define		CMD_RECORD_FILE_OPERATE_REQUEST_EX		725
	
	//录像文件操作类型枚举结构
	typedef		enum	EnumRecordOperType
	{
		RECORD_FILE_REPLAY = 0,		//文件回放
		RECORD_FILE_DOWNLOAD,	//文件下载
		RECORD_TIME_REPLAY,		// 录像时间回放
		RECORD_TIME_DOWNLOAD,	// 录像时间下载

	};

	//录像文件操作请求命令结构
	typedef struct StruRecordFileOperRequest
	{
		INT32 iOperType;						// 操作类型：见枚举EnumRecordOperType，表示对录像文件进行的操作类型，0	表示文件回放；1	表示文件下载；2 表示时间回放；3 表示时间下载
		INT32 iVersion;							// 主要是用于兼容不同存储介质录像文件ID不同表示方法，暂时填0
		char  strRcdFileID[MAX_ID_STRING_LEN];	// 文件：录像文件ID  时间： 开始时间和结束时间（如 2011-01-01-00-00-00,2011-01-01-01-00-00）
		INT32 iPlatformID;						// 平台ID
		INT32 iDevID;							// 设备ID
		INT32 iChnID;							// 通道ID
		INT16 iStorageType;						// 存储类型，参考枚举类型EnumStorageType
	
		//INT16 iReserve;				// 保留，（若为锁定命令，则为锁定时长,单位为(小时)）
		INT16 iRelevantReason;				// 录像原因 ,按时间回放时使用，参考枚举类型EnumRecordReason
		
		char  szIPAddr[MAX_IP_ADDR_LEN];		// 客户端IP地址

		//按时时间回放时 指定查询条件
		
	

	}StruRecordFileOperRequest,*StruRecordFileOperRequestPtr;
	// 级联录像回放下载
	typedef struct StruCmdCascadeRecordFileOperRequest
	{
		StruRecordFileOperRequest	stCmdContent;			// 具体含义参考其定义
		INT32						iClientNum;				// 用户信息数目
		StruUserInfo				stUserInfo[1];			// 用户信息，变长结构
	}StruCmdCascadeRecordFileOperRequest,*StruCmdCascadeRecordFileOperRequestPtr;
    
	
	/**************************************************************************************************
		CMD_ID		: CMD_RECORD_FILE_OPERRATE_RESPONSE
		CMD_NAME	: CMD_RECORD_FILE_OPERRATE_RESPONSE
		DateTime	: 2011/2/21 9:04	
		Description	: 录像文件操作请求回复
		Author		: sdj
		Note		: Client发起录像文件操作请求回复
	**************************************************************************************************/
	#define		CMD_RECORD_FILE_OPERATE_RESPONSE			724
	#define		CMD_RECORD_FILE_OPERATE_RESPONSE_EX			726
	
	//录像文件操作请求回复命令结构
	typedef struct StruRecordFileOperResponse
	{
		INT32 iOperResult;					    // 回复结果,枚举EnumErrorCode定义
		INT32 iOperType;						// 操作类型：表示对录像文件进行的操作类型，0	表示回放；1	表示下载
		INT32 iVersion;							// 主要是用于兼容不同存储介质录像文件ID不同表示方法，暂时填0
		char  strRcdFileID[MAX_ID_STRING_LEN];	// 录像文件ID	
		char  strRecFileURI[MAX_URI_LEN];		// 录像文件资源地址
		INT16 iStorageType;						// 存储类型，参考枚举类型EnumStorageType
		INT16 iReserve;							// 保留
	}StruRecordFileOperResponse,*StruRecordFileOperResponsePtr;
	
	/**************************************************************************************************
	CMD_ID		: CMD_RECORD_FILE_CONTROL_REQUEST
	CMD_NAME	: CMD_RECORD_FILE_CONTROL_REQUEST
	DateTime	: 2012/5/3 10:48	
	Description	: 
	Author		: zf
	Note		: NULL
	**************************************************************************************************/ 
	//录像文件操作类型枚举结构
	typedef		enum	EnumRecordFileControlType
	{	
		//RECORD_FILE_INVALID = 0, //录像文件无效
		RECORD_FILE_LOCK,         //录像文件锁定
		RECORD_FILE_UNLOCK       //录像文件解锁
	};
	//录像文件控制请求命令(录像文件加锁解锁命令)
	#define  CMD_RECORD_FILE_CONTROL_REQUEST   909 
	#define  CMD_RECORD_FILE_CONTROL_RESPONSE  910
    //录像文件控制请求结构
	typedef StruRecordFileOperRequest  StruRecordFileControlInfo;  // 操作类型：见枚举EnumRecordFileControlType
	typedef struct StruRecordFileControlRequest
	{
		StruRecordFileControlInfo    stRcecordFileLockInfo;            //要锁定的录像文件,CMD_RECORD_FILE_CONTROL_REQUEST,// 操作类型：见枚举EnumRecordFileControlType
		INT32				        iLockTimeLen;                     //锁定时长，以分为单位 (当操作类型为解锁时，参数无意义)                  
	}StruRecordFileControlRequest,*StruRecordFileControlRequestPtr;
    //录像文件控制回复结构
	typedef StruRecordFileOperResponse  StruRecordFileControlResponse;// 操作类型：见枚举EnumRecordFileControlType

	

	/**************************************************************************************************
	DateTime	: 2012/3/6 10:30	
	Description	: 录像文件备份
	Author		: zf
	Note		: 
	**************************************************************************************************/
	//录像文件自动备份规则
	typedef 	StruSerRecordItem  StruRecordBackupItem;

	typedef struct StruRecordFileAutoBackUpRuleItem
	{	
		INT16 iMediaType;						     //媒体类型，EnumMediaType
		INT16 eRecordReason;                        // 录像原因, 参考EnumRecordReason
		StruDateTime  stBackUpTime;                 //执行录像备份操作的时间
		StruRecordBackupItem  stRecordBackupItem;   //参考StruSerRecordItem
	}StruRecordFileAutoBackUpRuleItem,*StruRecordFileAutoBackUpRuleItemPtr;
	//规则请求命令
	#define CMD_RECORD_FILE_AUTOBACKUP_RULE_REQUEST  727

	//规则回复命令
	#define CMD_RECORD_FILE_AUTOBACKUP_RULE_RESPONSE 728
	//回复命令结构
	typedef struct StruRecordAutoBackupRuleResponse
	{
		INT32 iResult;                                                   //回复操作结果
		INT32 iRuleNum;                                                  //规则个数
		StruRecordFileAutoBackUpRuleItem stRecordAutoBackupRuleItem[1];  //规则项

	}StruRecordAutoBackupRuleResponse,*StruRecordAutoBackupRuleResponsePtr;

	//手动备份命令
	#define CMD_RECORD_FILE_MANUALBACKUP_OPER_REQUEST       729
	//手动备份回复命令
	#define CMD_RECORD_FILE_MANUALBACKUP_OPER_RESPONSE      736

	typedef struct StruBackupRecordFileID
	{	
		char strRcdFileID[MAX_ID_STRING_LEN];	// 录像文件ID	
	}StruBackupRecordFileID;

	//手动备份命令请求结构
	typedef struct StruRecordFileManualBackupOperRequest
	{   
		INT32 iPlatformID;						             // 设备所属平台ID
		INT32 iRecordFileNum;                                 //录像文件数
		StruBackupRecordFileID stRecordFileID[1];             //录像文件ID

	}StruRecordFileManualBackupOperRequest,*StruRecordFileManualBackupOperRequestPtr;
	//手动备份命令回复结构
	typedef struct StruRecordFileManualBackupOperResponse
	{

		INT32 iOperResult;                             //操作结果

	}StruRecordFileManualBackupOperResponse,*StruRecordFileManualBackupOperResponseptr;

#define CMD_RECORD_FILE_BACKUP_RETRIEVAL_REQUEST       737

#define CMD_RECORD_FILE_BACKUP_RETRIEVAL_RESPONSE      738


	//录像备份文件检索请求
	typedef struct StruRecordFileRetrievalRequest StruRecordFileBackupRetrievalRequest,*StruRecordFileBackupRetrievalRequestPtr;
	//录像备份文件检索回复
	typedef   StruRecordFileRetrievalResponse StruRecordFileBackupRetrievalResponse,*StruRecordFileRetrievalResponsePtr;



	/**************************************************************************************************
		CMD_ID		: CMD_DEV_LOGIN_INFO_REQUEST
		CMD_NAME	: CMD_DEV_LOGIN_INFO_REQUEST
		DateTime	: 2011/5/9 14:45	
		Description	: 获取设备连接信息请求
		Author		: sdj
		Note		: Client发起获取设备连接信息请求（用于语音对讲）
	**************************************************************************************************/
	#define		CMD_DEV_LOGIN_INFO_REQUEST			740  // 获取设备连接信息请求
	typedef struct StruDevLoginInfoRequest
	{
		INT32 iPlatformID;						// 设备所在平台ID
		INT32 iDevID;							// 设备ID
	}StruDevLoginInfoRequest,*StruDevLoginInfoRequestPtr;

	/**************************************************************************************************
		CMD_ID		: CMD_DEV_LOGIN_INFO_REQUEST
		CMD_NAME	: CMD_DEV_LOGIN_INFO_REQUEST
		DateTime	: 2011/5/9 14:45	
		Description	: 获取设备连接信息回复
		Author		: sdj
		Note		: 
	**************************************************************************************************/
	#define		CMD_DEV_LOGIN_INFO_RESPONSE			741  // 获取设备连接信息回复
	typedef struct StruDevLoginInfoResponse
	{
		INT32 iOperResult;					// 回复结果,枚举EnumErrorCode定义
		INT32 iPlatformID;					// 设备所在平台ID
		INT32 iDevID;						// 设备ID
		char szDevConn[TB_MAX_CONN_LEN];	// 设备连接方式
		char szLoginName[TB_MAX_NAME_LEN];	// 登录名
		char szLoginPWD[TB_MAX_PWD_LEN];	// 登陆密码
		INT32 iModelID;						// 设备型号
	}StruDevLoginInfoResponse,*StruDevLoginInfoResponsePtr;
	
	typedef enum EnumGSOperationType
	{
		OPERATE_TYPE_STOP = 0,			// 关闭
		OPERATE_TYPE_START = 1			// 打开
	}EnumGSOperationType;

	/**************************************************************************************************
		CMD_ID		: CMD_VOICE_LISTENING_REQUEST
		CMD_NAME	: CMD_VOICE_LISTENING_REQUEST
		DateTime	: 2011/5/12 9:00	
		Description	: 语音监听请求命令
		Author		: sdj
		Note		: 包括打开语音监听请求和关闭语音监听请求
	**************************************************************************************************/
	#define CMD_VOICE_LISTENING_REQUEST			745
	typedef struct StruVioceListeningRequest
	{
		INT32 iPlatformID;				// 平台ID
		INT32 iDevID;					// 设备ID
		INT32 iChnID;					// 通道ID
		INT32 iOperateType;				// 操作类型，参考枚举类型EnumOperType
	}StruVioceListeningRequest,*StruVioceListeningRequestPtr;

	// 级联语音监听请求
	typedef struct StruCascadeVioceListeningRequest
	{
		StruVioceListeningRequest	stCmdContent;				// 具体含义参考其定义
		INT32				iClientNum;						// 用户信息数目
		StruUserInfo		stUserInfo[1];					// 用户信息，变长结构// 设备ID
	}StruCascadeVioceListeningRequest,*StruCascadeVioceListeningRequestPtr;

	/**************************************************************************************************
		CMD_ID		: CMD_VOICE_LISTENING_RESPONSE
		CMD_NAME	: CMD_VOICE_LISTENING_RESPONSE
		DateTime	: 2011/5/12 9:00	
		Description	: 语音监听回复命令
		Author		: sdj
		Note		: 
	**************************************************************************************************/
	#define CMD_VOICE_LISTENING_RESPONSE		746
	typedef struct StruVioceListeningResponse
	{
		INT32 iOperResult;				// 回复结果,枚举EnumErrorCode定义
		INT32 iOperateType;				// 操作类型，参考枚举类型EnumOperType
	}StruVioceListeningResponse,*StruVioceListeningResponsePtr;

	/**************************************************************************************************
	CMD_ID		: CMD_STREAM_MATRIX_REQUEST
	CMD_NAME	: CMD_STREAM_MATRIX_REQUEST
	DateTime	: 2011/5/12 9:00	
	Description	: 码流上墙请求命令
	Author		: chc
	Note		: 客户端指定某一路码流上墙时的指令
	**************************************************************************************************/
	#define CMD_STREAM_MATRIX_REQUEST			780
	typedef enum ENUM_REQ_TYPE
	{
		REQ_MANUAL = 0,		//人工
		REQ_ALARM,		// 告警
		REQ_AUTO,		// 自动
		REQ_CIRCLE,		// 轮询
		REQ_DEVICEOFF,  // 设备不在线   
		REQ_UNKNOWN		// 未知
	}EnumReqType;
	typedef struct StruDeviceInfoRequest
	{
		INT32 iOperType;				// 操作类型 详见EnumRunFlag
		INT32 iPlatformID;				// 平台ID
		INT32 iDevID;					// 设备ID
		INT32 iChnID;					// 通道ID
		INT32 iVMSID;					// 电视墙ID
		INT32 iLogicalOutChnlID;		// 逻辑输出通道ID
		INT32 iReqType;					// 请求类型 详见ENUM_REQ_TYPE
		INT32 iStreamType;				// 0：实时流 1：录像流
		CHAR  szURI[MAX_URI_LEN];		// 如果是录像上墙 则必须填URI
	}StruDeviceInfoRequest,*StruDeviceInfoRequestPtr;

	/**************************************************************************************************
	CMD_ID		: CMD_STREAM_MATRIX_RESPONSE
	CMD_NAME	: CMD_STREAM_MATRIX_RESPONSE
	DateTime	: 2011/5/12 9:00	
	Description	: 码流上墙回复命令
	Author		: chc
	Note		: 
	**************************************************************************************************/
	#define CMD_STREAM_MATRIX_RESPONSE			781
	typedef struct StruDeviceInfoResponse
	{
		INT32 iOperResult;				// 回复结果,枚举EnumErrorCode定义
	}StruDeviceInfoResponse,*StruDeviceInfoResponsePtr;

	/**************************************************************************************************
	CMD_ID		: CMD_OPERATE_LOCK_REQUEST
	CMD_NAME	: CMD_OPERATE_LOCK_REQUEST
	DateTime	: 2011/5/25 17:00	
	Description	: 操作锁定请求命令
	Author		: hf
	Note		: 
	**************************************************************************************************/
	#define CMD_OPERATE_LOCK_REQUEST			790
	
	// 操作类型
	typedef enum EnumOperateType
	{
		OPER_TYPE_STREAM_MATRIX = 0,	// 码流上墙
		OPER_TYPE_PTZ_CONTROL,		// 云台控制
		OPER_TYPE_VOICE_CALL,		// 语音对讲
		OPER_TYPE_LOCK_MATRIX	// 控制电视墙

	}EnumOperateType;

	typedef enum EnumLockType
	{
		LOCK_TYPE_LOCK = 0,				// 锁定
		LOCK_TYPE_UNLOCK,			// 未锁定
		LOCK_TYPE_MONOLIZE_LOCK		// 抢占锁定
	}EnumLockType;

	typedef	struct StruOperUserInfo 
	{
		char	szUserName[MAX_NAME_LEN];	//用户名称
		INT32	iUserLevel;					//用户等级
	}StruOperUserInfo,*StruOperUserInfoPtr;

	typedef struct StruOperLockDeviceInfo
	{
		INT32	iPlatformID;						// 平台ID
		INT32	iDevID;								// 设备ID或者电视墙ID	
		INT32	iChnID;								// 通道ID(设备所对应的通道或者电视墙所对应逻辑通道)，如果锁定操作无需通道(比如语音对讲)参数，填-1
		INT32	iOperType;							// 操作类型，参考枚举EnumOperateType定义
	}StruOperLockDeviceInfo,*StruOperLockDeviceInfoPtr;
    
	
	typedef struct StruOperLockRequest
	{
		StruOperLockDeviceInfo	stOperLockDeviceInfo;					// 操作的设备信息
		INT32					iLock;									// 锁定状态，参考枚举EnumLockType定义
		char					szLockReason[MAX_REMARKS_LEN];			// 锁定原因（如果是锁定操作，需要填原因，如果是解锁操作，不需要填原因）
	}StruOperLockRequest,*StruOperLockRequestPtr;

	/**************************************************************************************************
	CMD_ID		: CMD_OPERATE_LOCK_RESPONSE
	CMD_NAME	: CMD_OPERATE_LOCK_RESPONSE
	DateTime	: 2011/5/25 17:00	
	Description	: 操作锁定回复命令
	Author		: sdj
	Note		: 
	**************************************************************************************************/
	#define CMD_OPERATE_LOCK_RESPONSE			791
	typedef struct StruOperLockResponse
	{
		INT32 iOperResult;		// 回复结果,枚举EnumErrorCode定义

	}StruOperLockResponse,*StruOperLockResponsePtr;
	
	
	/**************************************************************************************************
	CMD_ID		: CMD_OPERATE_STATUS_QUERY_REQUEST
	CMD_NAME	: CMD_OPERATE_STATUS_QUERY_REQUEST
	DateTime	: 2011/5/25 17:00	
	Description	: 通道操作状态查询请求命令
	Author		: hf
	Note		: 
	**************************************************************************************************/
	#define CMD_OPERATE_STATUS_QUERY_REQUEST			792
	typedef struct StruOperStatusQueryRequest
	{
		StruOperLockDeviceInfo	stOperLockDeviceInfo;	//操作的设备信息

	}StruOperStatusQueryRequest,*StruOperStatusQueryRequestPtr;
	/**************************************************************************************************
	CMD_ID		: CMD_OPERATE_STATUS_QUERY_RESPONSE
	CMD_NAME	: CMD_OPERATE_STATUS_QUERY_RESPONSE
	DateTime	: 2011/5/25 17:00	
	Description	: 通道操作状态查询回复命令
	Author		: hf
	Note		: 
	**************************************************************************************************/
	#define CMD_OPERATE_STATUS_QUERY_RESPONSE			793
	
	typedef enum EnumUserMonolizeRightType
	{
		USER_NO_MONOLIZE_RIGHT = 0,				// 不可抢占
		USER_MONOLIZE_RIGHT,				// 可抢占	
	}EnumUserMonolizeRightType;

	typedef struct StruOperStatusQueryResponse
	{
		INT32						iOperResult;						// 回复结果,枚举EnumErrorCode定义
		StruOperLockDeviceInfo		stOperLockDeviceInfo;				// 操作的设备信息
		INT32						iLock;								// 锁定状态，参考枚举EnumLockType定义
		INT32						iUserMonolizeRight;					// 当前用户抢占权限，参考枚举EnumUserMonolizeRightType
		StruOperUserInfo			stLockUserInfo;						// 独占用户信息，如果未锁定，填0
		char						szLockReason[MAX_REMARKS_LEN];		// 锁定原因（如果锁定，填写原因，未锁定，填0）
	}StruOperStatusQueryResponse,*StruOperStatusQueryResponsePtr;

	/**************************************************************************************************
	CMD_ID		: CMD_OPERATE_MONOLIZE_STATUS_NOTICE
	CMD_NAME	: CMD_OPERATE_MONOLIZE_STATUS_NOTICE
	DateTime	: 2011/5/25 17:00	
	Description	: 设备独占状态通知命令
	Author		: hf
	Note		: 设备锁定被抢占时，发送，
				  设备锁定超时解锁时，发送，
				  设备锁定被高级用户解锁时，发送
	**************************************************************************************************/
	#define CMD_OPERATE_MONOLIZE_STATUS_NOTICE			794

	typedef enum EnumMonolizeStatusNoticeType
	{
		MONOLIZE_TYPE_LOCK = 0,					// 独占控制
		MONOLIZE_TYPE_UNLOCK,				// 解除独占控制	
		MONOLIZE_TYPE_TIMEOUT_UNLOCK		// 超时解除独占控制
	}EnumMonolizeStatusNoticeType;

	typedef struct StruOperStatusNotice
	{
		INT32					iMonolizeType;						// 通知类型，参考枚举EnumMonolizeStatusNoticeType
		StruOperLockDeviceInfo	stOperLockDeviceInfo;				// 操作的设备信息
		char					szReason[MAX_REMARKS_LEN];			// 原因
		StruOperUserInfo		stLockUserInfo;						// 独占用户信息或者解锁用户信息（若为超时解除独占通知，填0）
		
	}StruOperStatusNotice,*StruOperStatusNoticePtr;
	/**************************************************************************************************
	CMD_ID		: CMD_SAVE_BROWSE_CHN_REQUEST
	CMD_NAME	: CMD_SAVE_BROWSE_CHN_REQUEST
	DateTime	: 2011/5/30 08:00	
	Description	: 保存浏览通道请求命令
	Author		: sdj
	Note		: 
	**************************************************************************************************/
	#define CMD_SAVE_BROWSE_CHN_REQUEST			800

	// 通道信息
	typedef struct StruChannelNode
	{
		INT32 iPlatformID;  // 平台ID
		INT32 iDevID;		// 设备ID
		INT32 iChnID;		// 通道ID
		INT32 iWindowID;	// 浏览窗口ID
	}StruChannelNode,*StruChannelNodePtr;

	typedef struct StruSaveBrowseChnRequest
	{
		INT32 iNum;							// 通道个数
		StruChannelNode stChannelNode[1];	// 通道节点,变长
	}StruSaveBrowseChnRequest,*StruSaveBrowseChnRequestPtr;

	/**************************************************************************************************
	CMD_ID		: CMD_SAVE_BROWSE_CHN_RESPONSE
	CMD_NAME	: CMD_SAVE_BROWSE_CHN_RESPONSE
	DateTime	: 2011/5/30 08:00	
	Description	:  保存浏览通道回复命令
	Author		: sdj
	Note		: 
	**************************************************************************************************/
	#define CMD_SAVE_BROWSE_CHN_RESPONSE		801

	typedef struct StruSaveBrowseChnRequestResponse
	{
		INT32 iOperResult;		// 回复结果,枚举EnumErrorCode定义
	}StruSaveBrowseChnResponse,*StruSaveBrowseChnResponsePtr;

	/**************************************************************************************************
	CMD_ID		: CMD_GET_BROWSE_CHN_REQUEST
	CMD_NAME	: CMD_GET_BROWSE_CHN_REQUEST
	DateTime	: 2011/5/30 08:00	
	Description	:  获取最后浏览通道请求命令
	Author		: sdj
	Note		: 
	**************************************************************************************************/
	#define CMD_GET_BROWSE_CHN_REQUEST			802


	/**************************************************************************************************
	CMD_ID		: CMD_GET_BROWSE_CHN_RESPONSE
	CMD_NAME	: CMD_GET_BROWSE_CHN_RESPONSE
	DateTime	: 2011/5/30 08:00	
	Description	: 获取最后浏览通道回复命令
	Author		: sdj
	Note		: 
	**************************************************************************************************/
	#define CMD_GET_BROWSE_CHN_RESPONSE			803

	typedef struct StruGetBrowseChnResponse 
	{
		INT32 iOperResult;		// 回复结果,枚举EnumErrorCode定义
		INT32 iNum;							// 通道个数
		StruChannelNode stChannelNode[1];	// 通道节点,变长
	}StruGetBrowseChnResponse,*StruGetBrowseChnResponsePtr;

	/**************************************************************************************************
	CMD_ID		: CMD_GET_POLLING_RECORD_REQUEST
	CMD_NAME	: CMD_GET_POLLING_RECORD_REQUEST
	DateTime	: 2011/6/27 17:00	
	Description	:  获取轮询记录请求命令
	Author		: sdj
	Note		: 
	**************************************************************************************************/
	#define CMD_GET_POLLING_RECORD_REQUEST			804
   
	
	/**************************************************************************************************
	CMD_ID		: CMD_GET_POLLING_RECORD_RESPONSE
	CMD_NAME	: CMD_GET_POLLING_RECORD_RESPONSE
	DateTime	: 2011/6/27 17:00		
	Description	: 获取轮询记录回复命令
	Author		: sdj
	Note		: 
	**************************************************************************************************/
	#define CMD_GET_POLLING_RECORD_RESPONSE			805

	typedef struct StruPollingRecord
	{
		INT32 iLen;				// 长度
		char  *pContent;		// 内容
	}StruPollingRecord;

	typedef struct StruGetPollingRecordResponse
	{
		INT32				iResult;						// 结果
		StruPollingRecord	stPollingRecord[2];				// 轮询记录
	}StruGetPollingRecordResponse,*StruGetPollingRecordResponsePtr;

	/**************************************************************************************************
	CMD_ID		: CMD_SAVE_POLLING_RECORD_REQUEST
	CMD_NAME	: CMD_SAVE_POLLING_RECORD_REQUEST
	DateTime	: 2011/6/27 17:00		
	Description	: 保存轮询记录请求命令
	Author		: sdj
	Note		: 
	**************************************************************************************************/
	#define CMD_SAVE_POLLING_RECORD_REQUEST			806
	
	typedef struct StruSavePollingRecordRequest
	{
		StruPollingRecord	stPollingRecord[2];				// 轮询记录
	}StruSavePollingRecordRequest,*StruSavePollingRecordRequestPtr;

	/**************************************************************************************************
	CMD_ID		: CMD_SAVE_POLLING_RECORD_RESPONSE
	CMD_NAME	: CMD_SAVE_POLLING_RECORD_RESPONSE
	DateTime	: 2011/6/27 17:00	
	Description	: 保存轮询记录回复命令
	Author		: sdj
	Note		: 
	**************************************************************************************************/
	#define CMD_SAVE_POLLING_RECORD_RESPONSE		807
	
	typedef struct StruSavePollingRecordResponse
	{
		INT32				iResult;						// 结果
	}StruSavePollingRecordResponse,*StruSavePollingRecordResponsePtr;



	/**************************************************************************************************
		CMD_ID		: CMD_TIME_SYNCHRONIZATION
		CMD_NAME	: CMD_TIME_SYNCHRONIZATION
		DateTime	: 2010/12/3 10:48	
		Description	: 服务时间同步命令
		Author		: Liujs
		Note		: NULL
	**************************************************************************************************/
	#define	CMD_SYS_TIME_SYNCHRONIZATION			808
	
	
	// 时间同步数据
	typedef	struct	StruCmdSynchronizationTime 
	{
		StruDateTime		stDateTime;							// 时间信息
	}StruCmdSynchronizationTime,*StruCmdSynchronizationTimePtr;

	/**************************************************************************************************
	CMD_ID		: CMD_DEV_TIME_SYNCHRO_RULE_REQUEST
	CMD_NAME	: CMD_DEV_TIME_SYNCHRO_RULE_REQUEST
	DateTime	: 2011/07/11 10:48	
	Description	: 设备时间同步规则请求
	Author		: hf
	Note		: NULL
	**************************************************************************************************/
	#define	CMD_DEV_TIME_SYNCHRO_RULE_REQUEST		809

	/**************************************************************************************************
	CMD_ID		: CMD_DEV_TIME_SYNCHRO_RULE_RESPONSE
	CMD_NAME	: CMD_DEV_TIME_SYNCHRO_RULE_RESPONSE
	DateTime	: 2011/07/11 10:48	
	Description	: 设备时间同步规则回复
	Author		: hf
	Note		: NULL
	**************************************************************************************************/
	#define	CMD_DEV_TIME_SYNCHRO_RULE_RESPONSE		810
	
	//	设备时间同步规则回复
	typedef		struct  StruCmdSynchroDevRuleResponse
	{
		INT32							iOperResult;										//	回复结果,枚举EnumErrorCode定义
		INT32							iTimeInterval;										//  校时间隔，单位：秒
	}StruCmdSynchroDevRuleResponse,*StruCmdSynchroDevRuleResponsePtr;
	
	/**************************************************************************************************
	CMD_ID		: CMD_DEV_REMOTE_OPERATION_REQUEST
	CMD_NAME	: CMD_DEV_REMOTE_OPERATION_REQUEST
	DateTime	: 2011/07/11 10:48	
	Description	: 设备远程控制请求
	Author		: hf
	Note		: NULL
	**************************************************************************************************/
	#define		CMD_DEV_REMOTE_OPERATION_REQUEST		811
	
	typedef enum EnumDevRemoteOperationType
	{
		DEV_OPERATION_RESTART = 1,		//设备重启
		DEV_OPERATION_TIME_SYNCHRO,		//设备校时
		DEV_OPERATION_IO_OUTPUT,		//IO输出，输出灯光，警笛等
		DEV_OPERATION_ALARM_RESET       //告警复位
	}EnumDevRemoteOperationType;
	

	//设备远程控制请求
	typedef		struct  StruCmdDevRemoteOperRequest
	{
		INT32			iOperType;					// 操作类型，参见枚举EnumDevRemoteOperationType定义
		INT32			iPlatformID;				// 平台ID
		INT32			iDevID;						// 设备ID
		INT32			iEnable;					//使能标志，参见枚举EnumRunFlag，操作类型为设备重启和校时时，此参数无效，填-1
		INT32			iChannelType;				// 通道类型，操作类型为设备重启和校时时，此参数无效，填-1
		INT32			iChannelID;					//	通道ID,操作类型为设备重启和校时时，此参数无效，填-1
	}StruCmdDevRemoteOperRequest,*StruCmdDevRemoteOperRequestPtr;

	/**************************************************************************************************
	CMD_ID		: CMD_DEV_REMOTE_OPERATION_RESPONSE
	CMD_NAME	: CMD_DEV_REMOTE_OPERATION_RESPONSE
	DateTime	: 2011/07/11 10:48	
	Description	: 设备远程控制回复
	Author		: hf
	Note		: NULL
	**************************************************************************************************/
	#define		CMD_DEV_REMOTE_OPERATION_RESPONSE		812

	//设备远程控制回复
	typedef		struct  StruCmdDevRemoteOperResponse
	{
		INT32			iOperResult;				//	回复结果,枚举EnumErrorCode定义
		
	}StruCmdDevRemoteOperResponse,*StruCmdDevRemoteOperResponsePtr;
	
	/**************************************************************************************************
	CMD_ID		: CMD_SAVE_PLAY_LIST_REQUEST
	CMD_NAME	: CMD_SAVE_PLAY_LIST_REQUEST
	DateTime	: 2011/09/05 10:48	
	Description	: 保存播放列表请求
	Author		: hf
	Note		: NULL
	**************************************************************************************************/
	#define		CMD_SAVE_PLAY_LIST_REQUEST				813

	typedef		struct StruPlayListInfo 
	{
		INT64				iPlayListID;					//列表编号,保存列表请求时，此编号可填为0
		char				szPlayListName[MAX_NAME_LEN];	//列表名称
		INT32				iChnNum;						//通道个数
		StruChannelNode		stChnNode[1];					//通道信息
	}StruPlayListInfo,*StruPlayListInfoPtr;

	typedef		struct StruSavePlayList 
	{
		INT32					iOperType;			//值详细见EnumUpdateType
		INT32					iNum;				//列表个数
		StruPlayListInfo		stPlaylistInfo[1];	//播放列表信息
	}StruSavePlayList,*StruSavePlayListPtr;


	/**************************************************************************************************
	CMD_ID		: CMD_OPER_RESPONSE
	CMD_NAME	: CMD_OPER_RESPONSE
	DateTime	: 2011/09/05 10:48	
	Description	: 操作回复
	Author		: hf
	Note		: NULL
	**************************************************************************************************/
	#define		CMD_OPER_RESPONSE				814

	typedef		struct  StruCmdOperResponse
	{
		INT32			iOperResult;				//	回复结果,枚举EnumErrorCode定义

	}StruCmdOperResponse,*StruCmdOperResponsePtr;

	/**************************************************************************************************
	CMD_ID		: CMD_GET_PLAY_LIST_REQUEST
	CMD_NAME	: CMD_GET_PLAY_LIST_REQUEST
	DateTime	: 2011/09/05 10:48	
	Description	: 获取播放列表请求
	Author		: hf
	Note		: NULL
	**************************************************************************************************/
	#define		CMD_GET_PLAY_LIST_REQUEST		815

	/**************************************************************************************************
	CMD_ID		: CMD_GET_PLAY_LIST_RESPONSE
	CMD_NAME	: CMD_GET_PLAY_LIST_RESPONSE
	DateTime	: 2011/09/05 10:48	
	Description	: 获取播放列表请求
	Author		: hf
	Note		: NULL
	**************************************************************************************************/
	#define		CMD_GET_PLAY_LIST_RESPONSE		816

	typedef		struct	StruCmdGetPlayListResponse 
	{
		INT32				iOperResult;			//回复结果
		StruSavePlayList	stSavePlayList;		//播放列表
	}StruCmdGetPlayListResponse,*StruCmdGetPlayListResponsePtr;
	

	/**************************************************************************************************
	CMD_ID		: CMD_POLLING_OPER_REQUEST
	CMD_NAME	: CMD_POLLING_OPER_REQUEST
	DateTime	: 2011/8/27 17:00	
	Description	:  轮询操作请求命令
	Author		: CHC
	Note		: 
	**************************************************************************************************/
	#define CMD_POLLING_OPER_REQUEST			817	

	//启动或停止轮询
	typedef enum EnumPollType
	{
		POLL_STOP = 0,
		POLL_START
	}ENUM_POLL_TYPE;
	typedef struct StruPollingOperRequest
	{
		INT32		iPollType;				// 轮询操作类型 ENUM_POLL_TYPE
		INT32		iDefautSchema;			// 0：采用默认方案 1：采用新的方案，即协议中带的方案StruPollingRecord
		StruPollingRecord	stPollingRecord[2];				// 轮询记录
	}StruExecutePollingRequest,*StruExecutePollingRequestPtr;

	/**************************************************************************************************
	CMD_ID		: CMD_POLLING_OPER_RESPONSE
	CMD_NAME	: CMD_POLLING_OPER_RESPONSE
	DateTime	: 2011/8/27 17:00		
	Description	: 执行轮询回复命令
	Author		: CHC
	Note		: 
	**************************************************************************************************/
	#define CMD_POLLING_OPER_RESPONSE			818
	typedef struct StruPollingOperResponse
	{
		INT32				iResult;						// 结果
	}StruPollingOperResponse,*StruPollingOperResponsePtr;
	

	/**************************************************************************************************
	CMD_ID		: CMD_GET_MATRIX_LIST_REQUEST
	CMD_NAME	: CMD_GET_MATRIX_LIST_REQUEST
	DateTime	: 2011/8/27 17:00		
	Description	: 获取电视墙列表命令
	Author		: CHC
	Note		: 
	**************************************************************************************************/
	#define CMD_GET_MATRIX_LIST_REQUEST			819
	typedef struct StruGetMatrixListRequest
	{
		INT32		iMatrixID;				// 电视墙ID， -1;表示取所有电视墙列表，否则根据参数的电视墙ID取其信息
	}StruGetMatrixListRequest,*StruGetMatrixListRequestPtr;

	/**************************************************************************************************
	CMD_ID		: CMD_GET_MATRIX_LIST_RESPONSE
	CMD_NAME	: CMD_GET_MATRIX_LIST_RESPONSE
	DateTime	: 2011/8/27 17:00		
	Description	: 获取电视墙列表回复命令
	Author		: CHC
	Note		: 
	**************************************************************************************************/
	#define CMD_GET_MATRIX_LIST_RESPONSE			820
	// 分屏的信息
	typedef struct StruScreenInfo
	{
		char	szName[MAX_NAME_LEN];	//分屏名称
		INT32	iLen;		// 长度
		char	pContent[1];	// 内容

	}StruScreenInfo, *StruScreenInfoPtr;

	// 通道配置信息
	typedef struct StruMatrixChnlConfig
	{
		INT32	iLen;		// 长度
		char	pContent[1];	// 内容

	}StruMatrixChnlConfig, *StruMatrixChnlConfigPtr;

	// 电视墙信息
	typedef struct StruMatrixInfo
	{
		INT32	iMatrixID;				// 电视墙ID
		char	szName[MAX_NAME_LEN];	// 电视墙名称
		INT32	iNum;					// 分屏个数,也就是通道配置结构的个数
		StruScreenInfo	stScreenInfo[1];	//分屏
		StruMatrixChnlConfig	stMatrixChnlConfig[1];			// 通道配置结构

	}StruMatrixInfo, *StruMatrixInfoPtr;
	typedef struct StruGetMatrixListResponse
	{
		INT32				iResult;						// 结果
		INT32				iNum;							// 电视墙个数
		StruMatrixInfo		stMatrixInfo[1];				// 电视墙信息
	}StruGetMatrixListResponse,*StruGetMatrixListResponsePtr;
    

	/**************************************************************************************************
	DateTime	: 2012/5/28 17:00		
	Description	:  Savler和 磁盘的相关信息
	Author		: zf
	Note		: 
	**************************************************************************************************/
	typedef enum EnumSlaverServerRole
	{
		SLAVER_ROLE_COMMON = 0,											//普通
		SLAVER_ROLE_DISAS_TOLERANT = 1,									//容灾
		SLAVER_ROLE_BACKUP = 2,											//备份

	}EnumSlaverServerRole;
	typedef enum EnumDiskRole
	{
		DISK_ROLE_MAIN = 0,												//主盘
		DISK_ROLE_DISAS_TOLERANT = 1,									//容灾盘
		DISK_ROLE_BACKUP = 2											//备份盘

	}EnumDiskRole;

//	#define CMD_GET_SLAVE_DISK_MAP_REQUEST		831
//	#define CMD_GET_SLAVE_DISK_MAP_RESPONSE		832
	//Slaver登记表

	//slaver 服务id 分配方式
	#define SLAVER_SRVID_USER_MIN 1L			//SLAVER 服务ID 用户分配范围,最小
	#define SLAVER_SRVID_USER_MAX   0x2fffffffL		///SLAVER 服务ID 用户分配范围，最大

	#define SLAVER_SRVID_SYSAUTO_MIN  0x3fffffffL	//SLAVER 服务ID 系统自动分配范围,最小
	#define SLAVER_SRVID_SYSAUTO_MAX  0x5fffffffL	///SLAVER 服务ID 系统自动分配范围，最大

	

	/**************************************************************************************************
	CMD_ID		: CMD_GET_SLAVE_REGISTER_INFO_REQUEST
	CMD_NAME	: CMD_GET_SLAVE_REGISTER_INFO_REQUEST
	DateTime	: 2012/5/28 17:00		
	Description	: 获取Savle配置信息
	Author		: zf
	Note		: 
	**************************************************************************************************/
	//获取Savle配置信息
	#define CMD_GET_SLAVE_REGISTER_INFO_REQUEST   920  //请求命令，没有命令体
	#define CMD_GET_SLAVE_REGISTER_INFO_RESPONSE   921 //回复命令， 命令体为 StruSlaverResiterInfoRes

	typedef  struct StruSlaverRegisterInfo
	{
		INT32				 iServID;		//服务ID，范围参考 服务id 分配方式
		INT32				 iRole;										//角色ID  EnumSlaverServerRole	
		BYTE				 szPermitLoginIP[MAX_IP_ADDR_LEN] ;			//允许登录IP
		INT32				 iPermitLoginPort;							//允许登录端口
	}StruSlaverRegisterInfo,*StruSlaverRegisterInfoPtr;

	
	typedef struct _StruSlaverResiterInfoRes
	{
		INT32 iResult; //操作结果
		INT32 iItems;
		StruSlaverRegisterInfo vectorItems[1]; //个数由iItems 决定
	}StruSlaverResiterInfoRes;




	/**************************************************************************************************
	CMD_ID		: CMD_GET_DISK_REGISTER_INFO_REQUEST
	CMD_NAME	: CMD_GET_DISK_REGISTER_INFO_REQUEST
	DateTime	: 2012/5/28 17:00		
	Description	: 获取磁盘配置信息
	Author		: zf
	Note		: 
	**************************************************************************************************/
	//获取DISK配置信息

	#define CMD_GET_DISK_REGISTER_INFO_REQUEST   922  //请求命令，没有命令体
	#define CMD_GET_DISK_REGISTER_INFO_RESPONSE   923 //回复命令， 命令体为 StruDiskResiterInfoRes

	//磁盘登记表
	typedef  struct  StruDiskRegisterInfo
	{
		BYTE				UUID[MAX_ID_STRING_LEN];					//磁盘UUID	
		INT32				iEnable;									//是否使用
		INT32				iRole;										//磁盘角色  EnumDiskRole
	}StruDiskRegisterInfo,*StruDiskRegisterInfoPtr;


	typedef struct _StruDiskResiterInfoRes
	{
		INT32 iResult; //操作结果
		INT32 iItems;
		StruDiskRegisterInfo vectorItems[1]; //个数由iItems 决定
	}StruDiskResiterInfoRes;



	/**************************************************************************************************
	CMD_ID		: CMD_GET_SLAVE_DISK_RUNING_MAP_REQUEST
	CMD_NAME	: CMD_GET_SLAVE_DISK_RUNING_MAP_RESPONSE
	DateTime	: 2012/5/28 17:00		
	Description	: 获取运行时SLAVE 与 磁盘的关系
	Author		: zf
	Note		: 
	**************************************************************************************************/
	//获取运行时SLAVE 与 磁盘的关系

	#define CMD_GET_SLAVE_DISK_RUNING_MAP_REQUEST		924  //没有命令内容
	#define CMD_GET_SLAVE_DISK_RUNING_MAP_RESPONSE		925  //命令内容为 StruSlaveDiskRuningMap


	// 运行时Slave 信息
	typedef struct _StruSlaverRunningInfo
	{
		INT32 iSrvID;			//服务ID ，范围参考 服务id 分配方式
		INT32 iRole;			//角色ID  EnumSlaverServerRole
		INT16 bOnline;			//是否在线
		UINT16 iLoginPort;		//当前登录端口  
		BYTE  szServiceIP[MAX_IP_ADDR_LEN];	//当前登录IP，当在线是有效
		BYTE  szLoginTime[32];		//登录时间, yyyy-mm-dd hh:mm:ss
	}StruSlaverRunningInfo;


	//运行时磁盘信息
	typedef  struct  _StruDiskRunningInfo
	{
		BYTE				UUID[MAX_ID_STRING_LEN];					//磁盘UUID
		BYTE				szDiskLabel[MAX_DISK_LABEL_LEN];			//盘符
		INT64				iTotalSize;									//总空间, 单位 GBytes
		INT64				iFreeSize;									//空闲空间, 单位 GBytes
		INT64				iLockSize;									//磁盘锁定空间, 单位 Bytes
		INT32				iEnable;									//是否使用
		INT32				iRole;										//磁盘角色  EnumDiskRole
		INT32				bActive;									//是否活动
	}StruDiskRunningInfo;


	//运行时 Slave 与 Disk 的关系
	typedef struct _StruSlaveDiskRuningMapItem
	{
		StruSlaverRunningInfo stSlave;
		INT32 iDiskNums;
		StruDiskRunningInfo vectorDisks[1];  //个数由 iDiskNums 指定
	}StruSlaveDiskRuningMapItem;


	typedef struct _StruSlaveDiskRuningMap
	{
		INT32 iResult; //操作结果
		INT32 iItemNums;
		StruSlaveDiskRuningMapItem vectorItemp[1]; //个数由iItemNums 指定
	}StruSlaveDiskRuningMap;
    

	/**************************************************************************************************
	CMD_ID		: CMD_GET_DEV_STORAGEPOLICY_REQUEST
	CMD_NAME	: CMD_GET_DEV_STORAGEPOLICY_REQUEST
	DateTime	: 2012/5/28 17:00		
	Description	: 获取设备存储策略登记表命令
	Author		: zf
	Note		: 
	**************************************************************************************************/
   //获取设备存储策略登记表
	#define CMD_GET_DEV_STORAGEPOLICY_REQUEST		926
	#define CMD_GET_DEV_STORAGEPOLICY_RESPONSE		927
    
    typedef struct StruDevStoragePolicyItem
	{
		INT32				iGroupID;							//策略组ID
		BYTE				szPolicyName[MAX_NAME_LEN];			//策略名
		INT32				iPlatformID;						//平台ID
		INT32				iDevID;								//设备ID
		INT32				iChnID;								//通道ID
		BYTE				UUID[MAX_ID_STRING_LEN];			//磁盘UUID
		INT32				iSlaverServID;						//Slaver ID
	}StruDevStoragePolicyItem,*StruDevStoragePolicyItemPtr;
	
	typedef struct StruDevStoragePolicyResponse
	{   
		INT32				iResult;                          //操作结果
		INT32				iNum;                             //
		StruDevStoragePolicyItem  stDevStoragePolicyItem[1];  //
		
	}StruDevStoragePolicyResponse,*StruDevStoragePolicyResponsePtr;
    

	/**************************************************************************************************
	CMD_ID		: CMD_GET_DEV_REPLACEPOLICY_REQUEST
	CMD_NAME	: CMD_GET_DEV_REPLACEPOLICY_REQUEST
	DateTime	: 2012/5/28 17:00		
	Description	: 获取设备替换策略登记表命令
	Author		: zf
	Note		: 
	**************************************************************************************************/
	//获取设备替换策略登记表命令
	typedef enum EnumDevReplacePolicy
	{  
		TIME_LIMITED = 1,							//规定时间,参数类型为  时间长度, 单位 天
		SPACE_LIMITED = 2,							//规定空间,参数类型为  空间大小, 单位为 GBytes
		FREE_COMPETITION = 3,						//自由竞争, 参数无用
	}EnumDevReplacePolicy;
	#define CMD_GET_DEV_REPLACEPOLICY_REQUEST		928
	#define CMD_GET_DEV_REPLACEPOLICY_RESPONSE		929
    
	//设备替换策略
	typedef struct  StruDevReplacePolicyItem
	{
		INT32				iGroupID;							//策略组ID
		INT32				iPlatformID;						//平台ID
		INT32				iDevID;								//设备ID
		INT32				iChnID;								//通道ID
		BYTE				szPolicyName[MAX_NAME_LEN];			//策略名
		INT32				iPolicyType;						//策略类型  EnumDevReplacePolicy
		BYTE				szPolicyArgs[MAX_REMARKS_LEN];		//策略参数	  
	}StruDevReplacePolicyItem, *StruDevReplacePolicyItemPtr;

	//设备替换策略登记表回复
   typedef struct  StruDevReplacePolicyResponse
   {
	   INT32				     iResult;                      //操作结果
	   INT32				     iNum;                          //
	   StruDevReplacePolicyItem  stDevReplacePolicyItem[1];     //	  
   }StruDevReplacePolicyResponse,*StruDevReplacePolicyResponsePtr;


	
	/**************************************************************************************************
	CMD_ID		: CMD_JOURNAL_QUERY_REQUEST
	CMD_NAME	: CMD_JOURNAL_QUERY_REQUEST
	DateTime	: 2011/09/20 14:48	
	Description	: 操作日志查询请求命令
	Author		: hf
	Note		: NULL
	**************************************************************************************************/
	#define CMD_JOURNAL_QUERY_REQUEST			821
	
	typedef	struct  StruCmdJournalQueryRequest
	{
		UINT32 iRowStart; //指定返回结果的开始行号， 由1 开始
		UINT32 iPageRows; //每页结果条数, 0 不指定

		char czFuncName[MAX_NAME_LEN]; //查询功能， 统一用小写，使用字符
		char czArgs[MAX_NAME_LEN]; //查询参数， 参数使用  ';' 分割
	}StruCmdJournalQueryRequest,*StruCmdJournalQueryRequestPtr;

	/**************************************************************************************************
	CMD_ID		: CMD_JOURNAL_QUERY_RESPONSE
	CMD_NAME	: CMD_JOURNAL_QUERY_RESPONSE
	DateTime	: 2011/09/20 14:48	
	Description	: 操作日志查询回复命令
	Author		: hf
	Note		: NULL
	**************************************************************************************************/
	#define CMD_JOURNAL_QUERY_RESPONSE			822
	
	typedef	struct StruCmdJournalQueryResponse 
	{
		INT32  iOperResult;   //结果
		//如果eResult失败，以下值无用
		UINT32 iTotals;		//总的记录数
		UINT32 iRowStart;  //当前结果集开始的行号， 由 1 开始
		UINT32 iRows;      //当前结果的行数
		INT32  iResultSize;  // 结果数据的pResultData 大小，
		char   szResultData[1]; // 结果数据, 内容格式为 XML
	}StruCmdJournalQueryResponse,*StruCmdJournalQueryResponsePtr;

	/**************************************************************************************************
	CMD_ID		: CMD_GET_MATRIX_DECODECARD_REQUEST
	CMD_NAME	: CMD_GET_MATRIX_DECODECARD_REQUEST
	DateTime	: 2011/12/02 17:00		
	Description	: 获取电视墙解码卡信息命令
	Author		: CHC
	Note		: 
	**************************************************************************************************/
	#define CMD_GET_MATRIX_DECODECARD_REQUEST			823
	typedef StruGetMatrixListRequest StruGetMatrixDecodecardRequest;//取电视墙解码卡信息

	/**************************************************************************************************
	CMD_ID		: CMD_GET_MATRIX_DECODECARD_RESPONSE
	CMD_NAME	: CMD_GET_MATRIX_DECODECARD_RESPONSE
	DateTime	: 2011/12/02 17:00		
	Description	: 获取电视墙解码卡信息回复命令
	Author		: CHC
	Note		: 
	**************************************************************************************************/
	#define CMD_GET_MATRIX_DECODECARD_RESPONSE			824
	typedef struct StruDecodecardInfo
	{
		char	szSlaveName[MAX_NAME_LEN];
		char	szCardName[MAX_NAME_LEN];
		INT32	iCardType;
		INT32	iLen;
		char	szPChnlSplitMode[1];	//物理通道分割模式,变长

	}StruDecodecardInfo, *StruDecodecardInfoPtr;
	typedef struct StruDecodecardResponse
	{
		INT32				iOperResult;					//	回复结果,枚举EnumErrorCode定义
		INT32				iNum;							// 解码卡数目
		StruDecodecardInfo	stDecodecardInfo[1];//变长
	}StruDecodecardResponse, *StruDecodecardResponsePtr;

	/**************************************************************************************************
	CMD_ID		: CMD_OPRATE_MATRIX_REQUEST
	CMD_NAME	: CMD_OPRATE_MATRIX_REQUEST
	DateTime	: 2012/01/02 17:00		
	Description	: 控制电视墙请求
	Author		: CHC
	Note		: 
	**************************************************************************************************/
	#define CMD_OPRATE_MATRIX_REQUEST			825
	typedef struct StruOprateMatrixRequest
	{
		INT32				iMatrixID;								// 电视墙ID
		INT32				iLock;									// 锁定状态，参考枚举EnumLockType定义
		StruOperUserInfo	stUserInfo;								// 暂时保留
	}StruOprateMatrixRequest,*StruOprateMatrixRequestPtr;

	/**************************************************************************************************
	CMD_ID		: CMD_OPRATE_MATRIX_RESPONSE
	CMD_NAME	: CMD_OPRATE_MATRIX_RESPONSE
	DateTime	: 2012/01/02 17:00		
	Description	: 控制电视墙请求回复
	Author		: CHC
	Note		: 
	**************************************************************************************************/
	#define CMD_OPRATE_MATRIX_RESPONSE			826
	typedef struct StruOprateMatrixResponse
	{
		INT32				iResult;						// 结果
		StruOperUserInfo	stUserInfo;						// 当iResult失败时，用户和IP填写当前控制电视墙的用户信息.否则不填写。
		char				szIP[MAX_IP_ADDR_LEN];
		UINT16				uiPort;
	}StruOprateMatrixResponse,*StruOprateMatrixResponsePtr;
	/**************************************************************************************************
	CMD_ID		: CMD_SYNNOTICE_MATRIX_REQUEST
	CMD_NAME	: CMD_SYNNOTICE_MATRIX_REQUEST
	DateTime	: 2012/01/02 17:00		
	Description	: 通知电视墙同步请求
	Author		: CHC
	Note		: 
	**************************************************************************************************/
	#define CMD_SYNNOTICE_MATRIX_REQUEST			827
	typedef struct StruSynNoticeRequest
	{
		INT32				iStartFlag;						// 参考EnumRunFlag
	}StruSynNoticeRequest,*StruSynNoticeRequestPtr;
	
	/**************************************************************************************************
	CMD_ID		: CMD_SYNNOTICE_MATRIX_RESPONSE
	CMD_NAME	: CMD_SYNNOTICE_MATRIX_RESPONSE
	DateTime	: 2012/01/02 17:00		
	Description	: 通知电视墙同步请求
	Author		: CHC
	Note		: 
	**************************************************************************************************/
	#define CMD_SYNNOTICE_MATRIX_RESPONSE			828
	typedef struct StruSynNoticeResponse
	{
		INT32				iResult;						// 结果
	}StruSynNoticeResponse,*StruSynNoticeResponsePtr;
	/**************************************************************************************************
	CMD_ID		: CMD_DATASYN_MATRIX_NOTICE
	CMD_NAME	: CMD_DATASYN_MATRIX_NOTICE
	DateTime	: 2012/01/02 17:00		
	Description	: 电视墙数据同步请求。master发给pms再转给cli
	Author		: CHC
	Note		: 
	**************************************************************************************************/
	#define CMD_DATASYN_MATRIX_NOTICE			829
	typedef StruDeviceInfoRequest StruDataSynRequest;
	/**************************************************************************************************
	CMD_ID		: CMD_DATASYN_MATRIX_RESPONSE
	CMD_NAME	: CMD_DATASYN_MATRIX_RESPONSE
	DateTime	: 2012/01/02 17:00		
	Description	: 电视墙同步请求
	Author		: CHC
	Note		: 此命令不用，保留命令号
	**************************************************************************************************/
	#define CMD_DATASYN_MATRIX_RESPONSE			830
	// 设计为不需要应答

	/**************************************************************************************************
	CMD_ID		: CMD_STATUSSYN_MATRIX_NOTICE
	CMD_NAME	: CMD_STATUSSYN_MATRIX_NOTICE
	DateTime	: 2012/07/14 17:00		
	Description	: 电视墙状态同步通知。master发给pms再转给cli
	Author		: CHC
	Note		: 此命令不需要应答
	**************************************************************************************************/
	#define CMD_STATUSSYN_MATRIX_NOTICE			831
	typedef	enum EnumStatusMatrixType
	{
		enum_MasterOnLineType,			//服务
		enum_SlaveOnLineType,
		enum_DecoderOnLineType,			//解码器
		enum_SchemaType,			//轮询方案
		enum_SplitUpdate,			//分屏更新
	}EnumStatusMatrixType;

	//enum_MasterOnLineType类型的数据结构
	typedef		struct		StruMasterOnLineResponse
	{
		INT32	iMasterID;	// masterID
		INT32	iOnLine;	//详见EnumDevStatusType
	}StruMasterOnLineResponse,*StruMasterOnLineResponsePtr;
	//enum_SlaveOnLineType类型的数据结构
	typedef		struct		StruSlaveOnLineResponse
	{
		CHAR	szName[MAX_NAME_LEN];	//slave
		INT32	iMasterID;	// 所属masterID
		INT32	iOnLine;	//详见EnumDevStatusType
	}StruSlaveOnLineResponse,*StruSlaveOnLineResponsePtr;

	//enum_DecoderOnLineType类型的数据结构
	typedef		struct		StruDecoderOnLineResponse
	{
		CHAR	szName[MAX_NAME_LEN];	//解码器名称
		CHAR	szDecoderIP[MAX_IP_ADDR_LEN];	//解码器IP
		CHAR	szSalveName[MAX_NAME_LEN];	//所属slave
		INT32	iOnLine;	//详见EnumDevStatusType
	}StruDecoderOnLineResponse,*StruDecoderOnLineResponsePtr;
	
	//enum_SchemaType类型的数据结构
	typedef		struct		StruSchemaResponse
	{
		CHAR	szName[MAX_NAME_LEN];	//方案名称
		INT32	iSchemaID;	//方案ID
	}StruSchemaResponse,*StruSchemaResponsePtr;
	
	//enum_SplitUpdate类型不需要传递数据

	//主数据结构
	typedef		struct		StruStatusSynMatrixNotice 
	{		
		INT32			iRequestType;									// 请求类型，参考结构EnumStatusMatrixType
		INT32			iDataLen;										// 数据长度	
		/* 
		返回数据，根据EnumStatusMatrixType类型来决定，
		请求类型						数据结构体
		enum_OnLineType				StruOnLineResponse
		*/
		void*			pData;											// 返回数据，根据EnumStatusMatrixType类型来决定，											
	}StruStatusSynMatrixNotice,*StruStatusSynMatrixNoticePtr;

	/**************************************************************************************************
	CMD_ID		: CMD_MATRIX_PLAN_CONTROL_REQUEST
	CMD_NAME	: CMD_MATRIX_PLAN_CONTROL_REQUEST
	DateTime	: 2013/01/22 10:55	
	Description	: 电视墙预案控制请求
	Author		: clj
	Note		: CU->BMS->VMS
	**************************************************************************************************/
	#define	CMD_MATRIX_PLAN_CONTROL_REQUEST						832
	typedef struct StruMatrixPlanControlRequest
	{
		INT32	iVMSID;					// 电视墙ID
		INT32	iType;					// 类型，1:开始 2:结束
		INT32	iPlanId;				// 预案ID
	}StruMatrixPlanControlRequest, *StruMatrixPlanControlRequestPtr;

	/**************************************************************************************************
	CMD_ID		: CMD_MATRIX_PLAN_CONTROL_RESPONSE
	CMD_NAME	: CMD_MATRIX_PLAN_CONTROL_RESPONSE
	DateTime	: 2013/01/22 10:55	
	Description	: 电视墙预案控制回复
	Author		: clj
	Note		: VMS->BMS->CU
	**************************************************************************************************/
	#define	CMD_MATRIX_PLAN_CONTROL_RESPONSE					833
	typedef struct StruMatrixPlanControlResponse
	{
		INT32	iOperResult;			// 返回值
	}StruMatrixPlanControlResponse, *StruMatrixPlanControlResponsePtr;

	/**************************************************************************************************
	CMD_ID		: CMD_MATRIX_PLAN_ADD_REQUEST
	CMD_NAME	: CMD_MATRIX_PLAN_ADD_REQUEST
	DateTime	: 2013/01/22 10:55	
	Description	: 电视墙预案增加
	Author		: weihaoz
	Note		: VMS->BMS->CU
	**************************************************************************************************/
	#define	CMD_MATRIX_PLAN_ADD_REQUEST					834
	//保存预案基本信息，缺省摄像机配置
	typedef struct StruScenarioInfo  
	{
		INT32 nID;
		INT32 nVMSID;
		INT32 nRunTime;
		INT32 nTimeInterval;
		INT32 nISTempltate;//1-为模版；0-不是模版
		CHAR  szUsrID[MAX_NAME_LEN];
		CHAR  szName[MAX_NAME_LEN];	//预案名称
		CHAR  szStartTime[MAX_NAME_LEN];
		CHAR  szSplitNum[MAX_SPLIT_LEN];

	}StruScenarioInfoRequest, *StruScenarioInfoRequestPtr;
	
	//
	typedef struct StruScenarioInfoExt 
	{
		INT32 nID;
		INT32 nVMSID;
		INT32 nRunTime;
		INT32 nTimeInterval;
		INT32 nISTempltate;//1-为模版；0-不是模版
		CHAR  szUsrID[MAX_NAME_LEN];
		CHAR  szName[MAX_NAME_LEN];	//预案名称
		CHAR  szStartTime[MAX_NAME_LEN];
		CHAR  szSplitNum[MAX_SPLIT_LEN];
		INT32	nCamerLen;
		CHAR	szCamer[1];								//变长数组。
	}StruScenarioInfoRequestExt, *StruScenarioInfoRequestExtPtr;
	
	typedef struct	StruScenarioGetResp
	{
		INT32			iOperResult;			//操作结果
		INT32			iNum;					//记录个数
		StruScenarioInfoRequestExt	ScenarioInfoRequestExt[1];
	}StruScenarioGetResp, *StruScenarioGetRespPtr;
	/**************************************************************************************************
	CMD_ID		: CMD_MATRIX_PLAN_ADD_RESPONSE
	CMD_NAME	: CMD_MATRIX_PLAN_ADD_RESPONSE
	DateTime	: 2013/01/22 10:55	
	Description	: 电视墙预案控制回复
	Author		: clj
	Note		: VMS->BMS->CU
	**************************************************************************************************/
	#define	CMD_MATRIX_PLAN_ADD_RESPONSE					835
	typedef struct StruMatrixPlanAddResponse
	{
		INT32	iOperResult;			// 返回值
	}StruMatrixPlanAddResponse, *StruMatrixPlanAddResponsePtr;
	
	#define CMD_MATRIX_PLAN_GET_REQUEST					836
	typedef	 struct StruMatrixScenarioReq
	{
		int	iReqType;			//0,请求全部，1根据id获取
		int	iID;					//预案ID,iReqType为0时不用填。
		int	iClientType;		//客户端类型。
	}StruMatrixScenarioReq, *StruMatrixScenarioReqPtr;

	#define CMD_MATRIX_PLAN_GET_RESPONSE				837

	/**************************************************************************************************
	CMD_ID		: CMD_CMD_MARTIX_PLAN_NOTICE 
	CMD_NAME	: CMD_CMD_MARTIX_PLAN_NOTICE 
	DateTime	: 2013/01/25 10:55	
	Description	: 电视墙预案处理消息回复
	Author		: weihaoz
	Note		: VMS->BMS->CU
	**************************************************************************************************/
	#define CMD_MATRIX_PLAN_NOTICE                      838
	typedef enum EnumMatrixPlanNotice
	{
		enum_PlanAutioStart = 1,        //预案自动启动
		enum_PlanAutioStop,             //预案自动停止
		enum_PlanStart,                 //预案手动启动
		enum_PlanStop,                  //预案手动停止
		enum_PlanAdd,                   //预案增加
		enum_PlanModify,                //预案修改
		enum_PlanDelete,                //预案删除
		enum_PlanGet,                   //预案获取
		enum_PlanGetState,              //预案状态获取
	};

	//预案返回消息结构体
	typedef struct StruMatrixPlanNotice
	{
		EnumMatrixPlanNotice  nType;    //当前操作类型
		int nScenarioID;                //预案ID
		int nISTemplate;                //是否为模版         
	}StruMatrixPlanNoticeResponse, *StruMatrixPlanNoticeResponsePtr;

	/**************************************************************************************************
	CMD_ID		: CMD_CMD_MARTIX_PLAN_NOTICE 
	CMD_NAME	: CMD_CMD_MARTIX_PLAN_NOTICE 
	DateTime	: 2013/01/25 10:55	
	Description	: 电视墙预案状态回复
	Author		: weihaoz
	Note		: VMS->BMS->CU
	**************************************************************************************************/
	#define CMD_MATRIX_PLAN_STATE_RESPONSE                839

	/**************************************************************************************************
	CMD_ID		: CMD_DEV_ALARMSTATUS_STATISTICS
	CMD_NAME	: CMD_DEV_ALARMSTATUS_STATISTICS
	DateTime	: 2013/02/14 10:48	
	Description	: 摄像头告警状态统计
	Author		: chc
	Note		: NULL
	**************************************************************************************************/

	#define	CMD_DEV_ALARMSTATUS_STATISTICS							896
	typedef struct StruDevAlarmStatusStatistics 
	{
		char	szUserName[MAX_NAME_LEN];
	}StruDevAlarmStatusStatistics,*StruDevAlarmStatusStatisticsPtr;
	/**************************************************************************************************
	CMD_ID		: CMD_DEV_ALARMSTATUS_STATISTICS_RESPONSE
	CMD_NAME	: CMD_DEV_ALARMSTATUS_STATISTICS_RESPONSE
	DateTime	: 2013/02/14 10:48	
	Description	: 摄像头告警状态统计结果
	Author		: chc
	Note		: NULL
	**************************************************************************************************/

	#define	CMD_DEV_ALARMSTATUS_STATISTICS_RESPONSE					897
	
	// 摄像头状态结果命令数据
	typedef		struct		StruCmdDevStatusStatisticsResult
	{		
		INT32						iOperResult;							// 操作结果
		INT32						iAlarmNum;								// 告警总数
		StruCmdAlarmQueryContent	stAlarmContent[1];						// 告警内容
	}StruCmdDevStatusStatisticsResult,*StruCmdDevStatusStatisticsResultPtr;


	/**************************************************************************************************
	CMD_ID		: CMD_DEV_ONLINE_QUERY
	CMD_NAME	: CMD_DEV_ONLINE_QUERY
	DateTime	: 2013/02/14 10:48	
	Description	: 摄像头上下线状态统计
	Author		: chc
	Note		: NULL
	**************************************************************************************************/

	#define	CMD_DEV_ONLINE_QUERY							898
	typedef struct StruDevOnlineQuery 
	{
		char	szUserName[MAX_NAME_LEN];
	}StruDevOnlineQuery,*StruDevOnlineQueryPtr;
	/**************************************************************************************************
	CMD_ID		: CMD_DEV_ONLINE_QUERY_RESPONSE
	CMD_NAME	: CMD_DEV_ONLINE_QUERY_RESPONSE
	DateTime	: 2013/02/14 10:48	
	Description	: 摄像头上下线状态统计结果
	Author		: chc
	Note		: NULL
	**************************************************************************************************/

	#define	CMD_DEV_ONLINE_QUERY_RESPONSE					899
	
	// 摄像头状态结果命令数据
	typedef		struct		StruCmdDevStatusResult
	{		
		INT32			iTotalNum;								// 摄像头总数
		INT32			iOnLineNum;								// 在线摄像头数		
	}StruCmdDevStatusResult,*StruCmdDevStatusResultPtr;

	/**************************************************************************************************
	CMD_ID		: CMD_SERVICE_RUN_STATUS_REQUEST
	CMD_NAME	: CMD_SERVICE_RUN_STATUS_REQUEST
	DateTime	: 2011/07/14 10:48	
	Description	: 服务运行状态请求
	Author		: hf
	Note		: NULL
	**************************************************************************************************/

	#define	CMD_SERVICE_RUN_STATUS_REQUEST				900

	/**************************************************************************************************
	CMD_ID		: CMD_SERVICE_RUN_STATUS_RESPONSE
	CMD_NAME	: CMD_SERVICE_RUN_STATUS_RESPONSE
	DateTime	: 2011/07/14 10:48	
	Description	: 服务运行状态回复
	Author		: hf
	Note		: NULL
	**************************************************************************************************/

	#define	CMD_SERVICE_RUN_STATUS_RESPONSE				901

	// 服务运行状态
	typedef enum EnumSerRunStatus
	{
		STATUS_NOT_ONLINE	= 0,	// 不在线
		STATUS_ONLINE_NORMAL,		// 在线，正常
		STATUS_ONLINE_ABNORMAL,		// 在线，异常

	}EnumSerRunStatus;

	typedef struct StruSerRunStatus 
	{
		INT32		iSerID;								//服务ID；
		INT32		iSerType;							//服务类型，类型定义见EnumClientType；
		char		szSerName[MAX_NAME_LEN];			//服务名称；
		INT32		iRunStatus;							//运行状态,类型参见EnumSerRunStatus；		
		char		szIPAddr[MAX_IP_ADDR_LEN];			//IP地址,服务不在线填0
		INT32		iPort;								//端口，服务不在线填0
		char		szSerVersion[MAX_VERSION_LEN];		//服务版本号，服务不在线填0;
		char		szRemark[MAX_REMARKS_LEN];			//服务备注信息
	}StruSerRunStatus,*StruSerRunStatusPtr;

	//回复服务运行状态的结构
	typedef struct StruCmdSerRunStatusResponse 
	{
		INT32						iOperResult;						//回复结果,枚举EnumErrorCode定义
		char						szSysVersion[MAX_VERSION_LEN];		//系统版本号;
		INT32						iNum;								//个数
		StruSerRunStatus			stSerRunStatus[1];					//服务运行状态
	}StruCmdSerRunStatusResponse,*StruCmdSerRunStatusResponsePtr;

	/**************************************************************************************************
	CMD_ID		: CMD_SYS_INFO_REQUEST
	CMD_NAME	: CMD_SYS_INFO_REQUEST
	DateTime	: 2011/07/14 10:48	
	Description	: 请求服务系统信息
	Author		: hf
	Note		: NULL
	**************************************************************************************************/

	#define	CMD_SYS_INFO_REQUEST				902
	
	typedef enum EnumSysInfoType
	{
		SYS_INFO_TYPE_MEM = 1,		//内存
		SYS_INFO_TYPE_VIRTUAL_MEM,	//虚拟内存
		SYS_INFO_TYPE_CPU,			//CPU		
		SYS_INFO_TYPE_DISK,			//磁盘
		SYS_INFO_TYPE_NET			//网络
	}EnumSysInfoType;

	//请求系统信息的结构
	typedef struct StruCmdSysInfoRequest 
	{
		INT32	iSysInfoType;					//要获取的系统信息类型，类型定义见EnumSysInfoType;
		char	szIPAddr[MAX_IP_ADDR_LEN];		//要获取的系统信息的服务器IP,为0时取所有在线服务器的系统信息；
	}StruCmdSysInfoRequest,*StruCmdSysInfoRequestPtr;


	/**************************************************************************************************
	CMD_ID		: CMD_SYS_INFO_RESPONSE
	CMD_NAME	: CMD_SYS_INFO_RESPONSE
	DateTime	: 2011/07/14 10:48	
	Description	: 请求系统信息的回复
	Author		: hf
	Note		: NULL
	**************************************************************************************************/
	#define	CMD_SYS_INFO_RESPONSE				903

	//CPU使用率结构
	typedef struct StruCPUInfoParamItem
	{
		INT32	iCPUUtiRatio;			//CPU利用率
	}StruCPUInfoParamItem,*StruCPUInfoParamItemPtr;

	typedef	struct	StruCPUInfoResponse
	{
		INT32					iNum;				//CPU个数
		StruCPUInfoParamItem	stCPUInfoParam[1];	//CPU参数
	}StruCPUInfoResponse,*StruCPUInfoResponsePtr;

	//存储信息结构
	typedef	struct	StruMemInfoParamItem
	{
		INT64	iMemUseSize;			//使用大小
		INT64	iMemTotalSize;			//总大小
	}StruMemInfoParamItem,*StruMemInfoParamItemPtr;

	//内存和虚拟内存信息结构
	typedef	struct	StruMemInfoResponse 
	{
		StruMemInfoParamItem stMemInfoParam;	//存储信息
	}StruMemInfoResponse,*StruMemInfoResponsePtr;

	//磁盘信息结构
	typedef	struct StruDiskInfoParamItem
	{
		char					DiskPartition[MAX_DISK_PARTITION_LEN];		//盘符
		StruMemInfoParamItem	stDiskMemInfo;								//磁盘的存储信息
	}StruDiskInfoParamItem,*StruDiskInfoParamItemPtr;

	typedef	struct StruDiskInfoResponse
	{
		INT32					iNum;				//磁盘个数
		StruDiskInfoParamItem	stDiskInfoParam[1];	//磁盘信息
	}StruDiskInfoResponse,*StruDiskInfoResponsePtr;


	//网络流量结构
	typedef	struct	StruNetInfoParamItem
	{
		char	szName[MAX_NAME_LEN_256];	//网卡描述名称
		INT64	iRecv;						//总共接收字节数
		INT64	iTrans;						//总共传送字节数
		INT32	iNetUtiRatio;				//网络利用率
	}StruNetInfoParam,*StruNetInfoParamPtr;

	typedef	struct StruNetInfoResponse
	{
		INT32					iNum;			   //网卡个数
		StruNetInfoParamItem	stNetInfoParam[1]; //网卡信息
	}StruNetInfoResponse,*StruNetInfoResponsePtr;

	// 系统信息结构联合体
	typedef		union		UnionCmdSysInfoResponse
	{
		StruMemInfoResponse	stMemInfoResponse;		//  内存和虚拟内存信息回复结构，请求类型为SYS_INFO_TYPE_MEM或SYS_INFO_TYPE_VIRTUAL_MEM；
		StruCPUInfoResponse	stCPUInfoResponse;		//	CPU信息回复结构，请求类型为SYS_INFO_TYPE_CPU； 
		StruDiskInfoResponse stDiskInfoResponse;	//	磁盘信息回复结构，请求类型为SYS_INFO_TYPE_DISK；
		StruNetInfoResponse	 stNetInfoResponse;		//	网络信息回复结构，请求类型为SYS_INFO_TYPE_NET。
	}UnionCmdSysInfoResponse;

	//系统信息结构
	typedef	struct	StruSysInfoResponse
	{	
		char						szIPAddr[MAX_IP_ADDR_LEN];			//对应系统信息的服务器IP；
		UnionCmdSysInfoResponse		UnionSysInfo;						//系统信息
	}StruSysInfoResponse,*StruSysInfoResponsePtr;

	//请求系统信息的回复结构
	typedef	struct	StruCmdSysInfoResponse
	{	
		INT32						iOperResult;			//回复结果,枚举EnumErrorCode定义
		INT32						iSysInfoType;			//请求类型，类型定义见EnumSysInfoType；
		INT32						iNum;					//个数					
		StruSysInfoResponse			stSysInfo[1];			//系统信息
	}StruCmdSysInfoResponse,*StruCmdSysInfoResponsePtr;

	/*************************************************
	设备远程参数设置
	**************************************************/
		// 前端设备参数设置请求
	#define CMD_SET_DEVICE_PARA_REQUEST  730

		// 前端设备参数设置回复
	#define CMD_SET_DEVICE_PARA_RESPONSE  731

		// 前端设备参数获取请求
	#define CMD_GET_DEVICE_PARA_REQUEST  732

		// 前端设备参数获取回复
	#define CMD_GET_DEVICE_PARA_RESPONSE  733

		// 批量前端设备参数设置请求
	#define CMD_SET_MANY_DEVICE_PARA_REQUEST  734

		// 批量前端设备参数设置回复
	#define CMD_SET_MANY_DEVICE_PARA_RESPONSE  735

	#define CMD_GET_DECODE_DEVICE_PARA_REQUEST 788
	#define CMD_GET_DECODE_DEVICE_PARA_RESPONSE 789

	// 设备参数配置参数信息
	typedef struct StruDevParaCfg 
	{
		INT32 iLen;                   // XML数据长度
		char pXMLData[1];   // XML数据 (具体内容参考XML相关参数定义)
	} StruDevParaCfg, *StruDevParaCfgPtr;

	// 设备参数配置结果回复信息
	typedef struct StruDevParaCfgResult
	{
		INT32 iResult;                   // 操作结果
		StruDevParaCfg stDevParaCfg;     // 设备参数配置参数信息 (参数获取等用到)
	} StruDevParaCfgResult, *StruDevParaCfgResultPtr;

	/**************************************************************************************************
	CMD_ID		: CMD_DATA_UPDATE_NOTICE
	CMD_NAME	: CMD_DATA_UPDATE_NOTICE
	DateTime	: 2011/10/12 10:48	
	Description	: PMS与配置中心交互的数据更新通知
	Author		: hf
	Note		: NULL
	**************************************************************************************************/
	#define		CMD_DATA_UPDATE_NOTICE			1001

	//说明：配置中心发数据更新通知，发命令号到BMS即可.
	/*
		root["PlatformID"]:父平台ID#下级平台ID#下下级平台ID#_父平台ID#下级平台ID#下下级平台ID#...
		如果只有一个平台发生了应用更新，则格式为平台ID#_
	*/
	
	/**************************************************************************************************
	CMD_ID		: CMD_DATA_UPDATE_NOTICE_EX
	CMD_NAME	: CMD_DATA_UPDATE_NOTICE_EX
	DateTime	: 2011/10/12 10:48	
	Description	: PMS平台内与各服务之间的数据更新通知
	Author		: hf
	Note		: NULL
	**************************************************************************************************/
	#define		CMD_DATA_UPDATE_NOTICE_EX		904
	
	typedef	struct  StruDataVersion
	{
		INT32		iDataTypeVersion;	// 获取的数据版本的类型，参考结构，EnumDataTypeVersion
		INT32		iDataVersion;		// 内部版本号增加，循环利用，取值范围[MIN_DATA_VERSION,MAX_DATA_VERSION]
	}StruDataVersion,*StruDataVersionPtr;

	typedef	struct	StruCmdDataUpdateNoticeEx 
	{
		StruDataVersion		stDataVersion[MAX_DATA_VERSION_NUM];						
	}StruCmdDataUpdateNoticeEx,*StruCmdDataUpdateNoticeExPtr;
	
	/**************************************************************************************************
	CMD_ID		: CMD_DATA_UPDATE_NOTICE_EX_RESPONSE
	CMD_NAME	: CMD_DATA_UPDATE_NOTICE_EX_RESPONSE
	DateTime	: 2011/10/12 10:48	
	Description	: PMS平台内与各服务之间的数据更新通知回复
	Author		: hf
	Note		: NULL
	**************************************************************************************************/
	#define		CMD_DATA_UPDATE_NOTICE_EX_RESPONSE			905


	#define			CMD_DATA_VERSION_REQUEST_EX			552			// 获取所有数据的版本请求命令，发命令即可，不用发数据包

	#define			CMD_DATA_VERSION_RESPONSE_EX		553			// 获取所有数据的版本回复命令

	typedef	 StruCmdDataUpdateNoticeEx StruCmdDataVersionResponseEX;	//	所有数据版本回复



	/**************************************************************************************************
	CMD_ID		: CMD_PMS_CLI_NOTICE
	CMD_NAME	: CMD_PMS_CLI_NOTICE
	DateTime	: 2011/10/12 10:48	
	Description	: PMS发给CLI的通知
	Author		: hf
	Note		: NULL
	**************************************************************************************************/
	#define		CMD_PMS_CLI_NOTICE			906
	
	typedef	enum	EnumPmsCliNoticeType
	{
		USER_IS_DELETED = 0,				//用户被删除
		PRESET_IS_UPDATED =1,				//预置点已更新
		POLLING_RECORD_IS_UPDATED =2,		//轮巡分组方案更新
		PLAY_LIST_IS_UPDATED = 3,			//逻辑分组更新
		CRUISETRACK_UPDATE=4,                //巡航路线更新
		USER_IS_NOTENABLE=5,                  //用户被禁用
		RECORD_FILE_CONSTATUS_UPDATE=6,        //录像文件控制状态更新（锁定或锁定变解锁）
		MATRIX_SCENARIO_UPDATED = 7,
		FORCE_USER_OFFLINE = 8					//强制用户下线
	};

	typedef	struct StruCmdPmsCliNotice 
	{
		INT32	iNoticeType;			//通知类型,见枚举EnumPmsCliNoticeType
		INT32	iDataLen;				// 数据长度
		char	szData[1];				// 数据内容，变长，由通知类型决定数据的结构。
	}StruCmdPmsCliNotice,*StruCmdPmsCliNoticePtr;

	typedef		struct  StruDevChnInfoItem 
	{
		INT32	iPlatformID;		//平台ID
		INT32	iDevID;				//设备ID
		INT32	iChnID;				//通道号
		INT32	iChnType;			//通道类型
	}StruDevChnInfoItem,*StruDevChnInfoItemPtr;

	//PRESET_IS_UPDATED通知类型的数据结构
	typedef	StruDevChnInfoItem StruPresetUpdatedNotice;

	typedef StruDevChnInfoItem StruCruiseTrackUpdateNotice; 



	//手动录像命令定义
	 #define  CMD_MANUAL_RECORD_REQUEST   907  //手动录像请求 命令体 StruManualRecordRequest
	 #define  CMD_MANUAL_RECORD_RESPONSE  908  //手动录像请求回复 命令体 StruManualRecordResponse


	#define MAX_MANUAL_RECOURD_TIME	(60)	//1 小时, 单位 分钟 ， 最大手动录像时间
    #define DEFAULT_MANUAL_RECOURD_TIME (5)    // 5 分钟， 单位 分钟， 默认的录像时间

	#define DEFAULT_MANUAL_RECOURD_TIMES  DEFAULT_MANUAL_RECOURD_TIME
	#define MAX_MANUAL_RECOURD_TIMES  MAX_MANUAL_RECOURD_TIME

	typedef enum 
	{
		MANUAL_RECORD_START = 0, //开始手动录像
		MANUAL_RECORD_STOP = 1,  //停止手动录像
	}EnumManualRecordOperation;
	
	//手动录像请求
	typedef struct _StruManualRecordRequest
	{ 
		INT16 eOperation;  //操作类型 参考 EnumManualRecordOperation
		INT16 iStorageType;	// 存储类型，参考枚举类型EnumStorageType
		UINT16 iChannelCounts;  //控制的通道数 aChannels/aTask 的个数，一定大于 0
		INT32 iMaxRecordTime;	// 录像的时间控制，当操作类型为 MANUAL_RECORD_START 有效
		// * 单位 分钟
		// 大于 0， <=  MAX_MANUAL_RECOURD_TIMES
		// 如果 小于 1 将使用默认时间(DEFAULT_MANUAL_RECOURD_TIMES)
		// 如果 大于 最大手动录像时间（MAX_MANUAL_RECOURD_TIMES）
		//	将使用MAX_MANUAL_RECOURD_TIMES
		union {
			StruDevChnInfoItem aChannels[1];   //录像的通道 开始录录像 时使用
			INT64 aTask[1]; //停止录像时使用 任务ID, 有请求返回 的返回
		}vArgs;
	}StruManualRecordRequest;


	
	//操作结果
	typedef struct _StruManualRecordResult
	{
		INT32 iOperResult;		// 回复结果,枚举EnumErrorCode定义
		char strRcdFileID[MAX_ID_STRING_LEN];	// 录像文件ID
		StruDevChnInfoItem stDeviceChannel; //控制的通道
		INT64 iTaskID; //任务ID
	}StruManualRecordResult;

	//手动录像请求 回复
	typedef struct _StruManualRecordResponse
	{
		INT16 eOperation;  //操作类型 参考 EnumManualRecordOperation
		UINT16 iResultCounts;  //返回的结果 aResults 的个数，一定大于 0	
		StruManualRecordResult aResults[1];
	}StruManualRecordResponse;
	
	
	/**************************************************************************

								外部系统接入协议
	****************************************************************************/
	
	/**************************************************************************************************
	CMD_ID		: CMD_ON_OFF_DOOR_SIGNAL
	CMD_NAME	: CMD_ON_OFF_DOOR_SIGNAL
	DateTime	: 2011/09/06 10:48	
	Description	: 2100开关门信号
	Author		: hf
	Note		: NULL
	**************************************************************************************************/
	#define		CMD_ON_OFF_DOOR_SIGNAL		2001

	typedef		struct	StruCmdOnOffDoor 
	{
		INT64			iLocalID;								//位置ID（门所对应的位置ID）
		INT32			iOnOffType;								//开关类型,枚举参考EnumRunFlag
		StruDateTime	stDateTime;								//动作时间
		char			szCardNumber[MAX_NUMBER_STRING_LEN];	//卡号
	}StruCmdOnOffDoor,*StruCmdOnOffDoorPtr;

	
	//平台内部通信协议



	typedef		struct	StruInnerCmdOnOffDoor 
	{
		StruCmdOnOffDoor	stOnOffDoorInfo;		//开关门信号信息
		INT32				iDevNum;				//设备个数
		StruDevChnInfoItem	stDevInfo[1];		//设备信息
	}StruInnerCmdOnOffDoor,*StruInnerCmdOnOffDoorPtr;

	/**************************************************************************************************
	CMD_ID		: CMD_GET_USERINFO
	CMD_NAME	: CMD_GET_USERINFO
	DateTime	: 2013/05/07 9:35	
	Description	: 获取用户信息请求
	Author		: wjb
	Note		: 此为用户权限管理中,获取用户信息请求
	**************************************************************************************************/
	#define		CMD_GET_USERINFO             1002     
	
	/**************************************************************************************************
	用户信息请求JSon结构,用jUserInfo表示
	jUserInfo["UserName"]:用户名称
	**************************************************************************************************/
	
	/**************************************************************************************************
	CMD_ID		: CMD_GET_USERINFO_RESPONSE
	CMD_NAME	: CMD_GET_USERINFO_RESPONSE
	DateTime	: 2013/05/07 9:37	
	Description	: 获取用户信息请求应答
	Author		: wjb
	Note		: 此为用户权限管理中,获取用户信息请求应答
	**************************************************************************************************/
	#define		CMD_GET_USERINFO_RESPONSE	 1003
	
	/**************************************************
	用户信息请求应答JSon结构,用root表示:
	root["AccountID"]:账号ID 
	root["Gender"] ：性别
	root["Digitallogin"] ：数字证书登陆
	root["Priority"] ：用户优先级
	root["Enable"] ：是否使用
	root["Isdelete"] ：是否已经被删除
	root["Accountname"] ：账号名称
	root["Password"] ：账号密码
	root["Username"] ：用户姓名
	root["Usercode"] ：用户编码
	root["Idcard"] ：身份证号码
	root["Policeid"] ：警号id
	root["Contact"] ：联系方式
	root["Remark"] ：备注
	root["AllOrganize"]：所有组织结构ID（包括父组织结构）,此为数组
	    jOrganize["OrganizeID"] :组织机构ID
		jOrganize["ParentID"] :父组织机构ID
		jOrganize["OrderNo"] :在同级机构中排序顺序
		jOrganize["OrgnaizeName"] : 组织机构名称
		jOrganize["Path"] : 从根到本级的ID集合,格式为ID#ID#ID, ID之间用#分割
	root["AllRole"]:所有角色ID，此为数组
		jRole["RoleID"]:角色ID
		jRole["RoleType"] ：角色类型
		jRole["RoleName"] ：角色名称
		jRole["Timetemplateid"] :时间模板ID
	**************************************************/
	  
	/**************************************************************************************************
	CMD_ID		: CMD_GET_RIGHTSINFO
	CMD_NAME	: CMD_GET_RIGHTSINFO
	DateTime	: 2013/05/07 9:54	
	Description	: 获取权限信息请求
	Author		: wjb
	Note		: 此为用户权限管理中,获取权限信息请求
	**************************************************************************************************/
	#define		CMD_GET_RIGHTSINFO	        1004
	
	/**************************************************************************************************
	用户权限信息请求JSon结构,用jUserInfo表示
	jUserInfo["UserName"]:用户名称
	**************************************************************************************************/
	
	/**************************************************************************************************
	CMD_ID		: CMD_GET_RIGHTSINFO
	CMD_NAME	: CMD_GET_RIGHTSINFO
	DateTime	: 2013/05/07 9:54	
	Description	: 获取权限信息请求应答
	Author		: wjb
	Note		: 此为用户权限管理中,获取权限信息请求应答
	**************************************************************************************************/
	#define		CMD_GET_RIGHTSINFO_RESPONSE  1005
	
	/**************************************************************************************************
	用户权限信息应答JSon结构,用root表示
	root["UserID"]:账号ID
	root["AllRole"]：账号下的角色，此为数组
		jRole["RoleID"]：账号ID
		jRole["AllRights"]：角色权限，此为数组
			jRight["RightID"] ：权限ID			
			jRight["ParentID"]	：父权限ID		
			jRight["Enable"]	：是否启用			
			jRight["Selected"]	：是否可选择		
			jRight["RightType"]	：权限类型
			jRight["Operatelevel"]	：操作级别
			jRight["Code"]	：编码	
			jRight["RightName"]：权限名称
	**************************************************************************************************/
	
	/**************************************************************************************************
	CMD_ID		: CMD_JUDGE_RIGHTS
	CMD_NAME	: CMD_JUDGE_RIGHTS
	DateTime	: 2013/05/07 10:09	
	Description	: 判断是否有某些权限
	Author		: wjb
	Note		: 此为用户权限管理中,判断是否有某些权限
	**************************************************************************************************/
	#define		CMD_JUDGE_RIGHTS		1006
	
	/**************************************************************************************************
	CMD_ID		: CMD_JUDGE_RIGHTS_RESPONSE
	CMD_NAME	: CMD_JUDGE_RIGHTS_RESPONSE
	DateTime	: 2013/05/07 10:11	
	Description	: 判断是否有某些权限应答
	Author		: wjb
	Note		: 此为用户权限管理中,判断是否有某些权限应答
	**************************************************************************************************/
	#define		CMD_JUDGE_RIGHTS_RESPONSE		1007
	
	/**************************************************************************************************
	CMD_ID		: CMD_GET_ROLEINFO
	CMD_NAME	: CMD_GET_ROLEINFO
	DateTime	: 2013/05/07 10:13	
	Description	: 获取用户角色请求
	Author		: wjb
	Note		: 此为用户权限管理中,获取用户角色请求
	**************************************************************************************************/
	#define		CMD_GET_ROLEINFO			   1008
	
	/**************************************************************************************************
	CMD_ID		: CMD_GET_ROLEINFO_RESPONSE
	CMD_NAME	: CMD_GET_ROLEINFO_RESPONSE
	DateTime	: 2013/05/07 10:11	
	Description	: 获取用户角色请求应答
	Author		: wjb
	Note		: 此为用户权限管理中,获取用户角色请求应答
	**************************************************************************************************/
	#define		CMD_GET_ROLEINFO_RESPONSE		1009
	
	/**************************************************************************************************
	CMD_ID		: CMD_GET_TIMETEMPLATEINFO
	CMD_NAME	: CMD_GET_TIMETEMPLATEINFO
	DateTime	: 2013/05/10 10:13	
	Description	: 获取用户时间模板请求
	Author		: wjb
	Note		: 此为用户权限管理中,获取用户角色请求
	**************************************************************************************************/
	#define		CMD_GET_TIMETEMPLATEINFO			   1010
	
	/**************************************************************************************************
	CMD_ID		: CMD_GET_TIMETEMPLATEINFO_RESPONSE
	CMD_NAME	: CMD_GET_TIMETEMPLATEINFO_RESPONSE
	DateTime	: 2013/05/10 10:11	
	Description	: 获取用户时间模板应答
	Author		: wjb
	Note		: 此为用户权限管理中,获取用户角色请求应答
	**************************************************************************************************/
	#define		CMD_GET_TIMETEMPLATEINFO_RESPONSE		1011
	

	/**************************************************************************************************
	CMD_ID		: CMD_FORCE_USEROFFLINE_REQUEST
	CMD_NAME	: CMD_FORCE_USEROFFLINE_REQUEST
	DateTime	: 2013/05/13 10:34	
	Description	: 强制用户下线请求
	Author		: hm
	Note		: 此为用户权限管理中,强制用户下线请求
	**************************************************************************************************/
	#define		CMD_FORCE_USEROFFLINE_REQUEST         1012
	
	//强制用户下线请求
	typedef		struct  StruCmdForceUserOfflineRequest
	{
		char		szUserName[32];				//用户账号
		UINT16		iLoginPort;				    //登录的端口号  
		char        szUserIP[MAX_IP_ADDR_LEN];	//强制用户下线的IP地址
	}StruCmdForceUserOfflineRequest,*StruCmdForceUserOfflineRequestPtr;

	/**************************************************************************************************
	CMD_ID		: CMD_GET_ROLEINFO_RESPONSE
	CMD_NAME	: CMD_GET_ROLEINFO_RESPONSE
	DateTime	: 2013/05/13 10:34	
	Description	: 强制用户下线请求应答
	Author		: hm
	Note		: 此为用户权限管理中,强制用户下线请求应答
	**************************************************************************************************/
	#define		CMD_FORCE_USEROFFLINE_RESPONSE		1013

	//强制用户下线回复
	typedef		struct  StruCmdForceUserOfflineResponse
	{
		INT32			iOperResult;				//	回复结果参照EnumErrorCode

	}StruCmdForceUserOfflineResponse,*StruCmdForceUserOfflineResponsePtr;



	/**************************************************************************************************
	CMD_ID		: CMD_GET_USERONLINESTATUS_REQUEST
	CMD_NAME	: CMD_GET_USERONLINESTATUS_REQUEST
	DateTime	: 2013/05/13 10:34	
	Description	: 获取用户是否在线状态请求
	Author		: hm
	Note		: 此为用户权限管理中,获取用户是否在线状态请求
	**************************************************************************************************/

	#define   CMD_GET_USERONLINESTATUS_REQUEST  1014

	//获取用户是否在线状态请求
	typedef		struct  StruCmdGetUserOnlineStatusRequest
	{
		char		szUserName[32];				//用户账号名称,不填，表示获取所有用户在线状态信息
		
	}StruCmdGetUserOnlineStatusRequest,*StruCmdGetUserOnlineStatusRequestPtr;

	/**************************************************************************************************
	CMD_ID		: CMD_GET_USERONLINESTATUS_RESPONSE
	CMD_NAME	: CMD_GET_USERONLINESTATUS_RESPONSE
	DateTime	: 2013/05/13 10:34	
	Description	: 获取用户是否在线状态请求的回复
	Author		: hm
	Note		: 此为用户权限管理中,获取用户是否在线状态请求的回复
	**************************************************************************************************/

	#define   CMD_GET_USERONLINESTATUS_RESPONSE  1015

	// 用户是否在线状态类型
	typedef	enum		EnumUserStatusType
	{
		USER_ONLINE		=	1,									// 上线
		USER_OFFLINE	=	0									// 下线
	}EnumUserStatusType;
	
	//获取用户是否在线状态请求回复
	typedef   struct  StruGetUserOnlineStatusInfo
	{
		char		szUserName[32];				//用户账号名称					
		INT32       bOnline;					// 用户是否在线状态，参考结构EnumUserStatusType
		UINT16		iLoginPort;				 //登录的端口号  
		char        szUserIP[MAX_IP_ADDR_LEN];	//当前登录IP，当在线是有效
		char        szLoginTime[32];		//登录时间, yyyy-mm-dd hh:mm:ss
		char        szBMSCurrentTime[32];		//BMS当前时间, yyyy-mm-dd hh:mm:ss,用于计算在线时长
	}StruGetUserOnlineStatusInfo,StruGetUserOnlineStatusInfoPtr;

	typedef struct _StruGetUserOnlineStatusMapResponse
	{
		INT32		iOperResult;							//	回复结果参照EnumErrorCode
		INT32		iUserNums;					//获取到用户账号的数目
		StruGetUserOnlineStatusInfo vectorUsers[1];  //个数由 iUserNums 指定
	}StruGetUserOnlineStatusMapResponse;

	/**************************************************************************************************
	CMD_ID		: CMD_GET_USERONLINESTATUS_RESPONSE
	CMD_NAME	: CMD_GET_USERONLINESTATUS_RESPONSE
	DateTime	: 2013/05/13 10:34	
	Description	: 获取系统管理员信息请求
	Author		: lxp
	Note		: 
	**************************************************************************************************/
	#define		CMD_GET_SYSADMININFO_RREQUEST		1016
	
		/**************************************************************************************************
	CMD_ID		: CMD_GET_USERONLINESTATUS_RESPONSE
	CMD_NAME	: CMD_GET_USERONLINESTATUS_RESPONSE
	DateTime	: 2013/05/13 10:34	
	Description	: 获取系统管理员信息回复
	Author		: lxp
	Note		: 
	**************************************************************************************************/
	#define		CMD_GET_SYSADMININFO_RESPONSE		1017

	
	//系统管理员json结构.
	/*
	root["iAccountid"]:				账号id
	root["iRepeatLogin"]:				是否允许重复登录.
	root["strAccountName"]:		账号名称.
	root["strPassWord"]:				账号密码.
	root["strUserName"];				管理员姓名.
	root["strPoliceID"];				管理员警号.
	root["strOrganizeName"];		单位名称。
	root["strContact"];					联系方式.
	root["strRemark"];					备注
	root["strStartip"];					开始IP
	root["strEndip"];					结束IP
	*/

	/**************************************************************************************************
	用户模板应答JSon结构,用root表示
	root["AllTimeTemp"]:时间模板信息，此为数组
		jTemp["Timetemplateid"] :时间模板ID
		jTemp["Ttpid"] ：时间模板参数ID			
		jTemp["Type"] 	：时间类型			
		jTemp["Weekdate"] ：星期			
		jTemp["Name"] ：名称				
		jTemp["Startdate"] 	：开始日期		
		jTemp["Stopdate"] ：结束日期
		jTemp["AllSpan"]:时间模板参数下的时间段,此为数组
			jSpan["Timespanbegin"] ：时间段开始
			jSpan["Timespanend"] ：时间段结束
	/**************************************************************************************************

	
	/**************************************************************************************************
	CMD_ID		: CMD_GET_KEBOARDINFO_REQUST
	CMD_NAME	: CMD_GET_KEBOARDINFO_REQUST
	DateTime	: 2013/05/17 15:34	
	Description	: 获取系统管理员信息回复
	Author		: cyt
	Note		: 
	**************************************************************************************************/
	#define		CMD_GET_KEBOARDINFO_REQUST		1018
	class Packet
	{
	public:
		virtual int GetSize(){return 0;}
		virtual bool DeCode(char* buf,const int nSize){buf;nSize;return false;}
		virtual bool EnCode(char* buf,const int nSize){buf;nSize;return false;}

		template <typename T>
		void Write(T a,char** buf)
		{
			memcpy(*buf,&a,sizeof(T));
			*buf += sizeof(T);
		}
		void Write(const char* pData,int nSize,char** buf)
		{
			memcpy((void*)*buf,pData,nSize);
			*buf += nSize;
		}
		template <typename T>
		void Read(T &a,char** buf)
		{
			memcpy(&a,*buf,sizeof(T));
			
			*buf += sizeof(T);
		}
		void Read(const char* pData,int nSize,char** buf)
		{
			memcpy((void*)pData,*buf,nSize);
			*buf += nSize;
		}
	};


	typedef struct StruKeyboard 
	{
		std::string strKbName;//键盘名称
		INT32 iVMSID;//虚拟矩阵ID
		INT32 iKbType;//键盘类型
		INT32 iKbInputType;//键盘输入类型
		std::string strUserName;//用户名
		std::string strUserPswd;//密码
		INT32 iPort;//IP端口
		std::string strIP;//IP地址
		INT32 iSerialComPort;//串口端口号
		std::string strSerialParam;//串口参数
		INT32 iSerialComType;//串口使用类型
	}StruKeyboard, *StruKeyboardPtr;
	typedef std::vector<StruKeyboard> KeboardList;
	typedef struct StruGetKeyboardResponse:public Packet
	{
		std::vector<StruKeyboard> m_KeyboardList;
		virtual int GetSize()
		{
			INT32 nSize = 0;
			for (KeboardList::iterator it = m_KeyboardList.begin();
				it != m_KeyboardList.end();++it )
			{
				nSize += sizeof(int)//数组大小
					+ sizeof(int) + it->strKbName.size()
					+ sizeof(it->iVMSID)
					+ sizeof(it->iKbType)
					+ sizeof(it->iKbInputType)
					+ sizeof(int) + it->strUserName.size()
					+ sizeof(int) + it->strUserPswd.size()
					+ sizeof(it->iPort)
					+ sizeof(int) + it->strIP.size()
					+ sizeof(it->iSerialComPort)
					+ sizeof(int) + it->strSerialParam.size()
					+ sizeof(it->iSerialComType);

			}
			return nSize;
		}

		virtual bool DeCode(char* buf,const int nSize)
		{
			if( buf == NULL || nSize<=0 )return false;
			char * pCurBuf = buf;
			int nCount = 0;
			Read(nCount,&pCurBuf);
			if ( nCount > nSize )
			{
				return false;
			}
			m_KeyboardList.resize(nCount);
			for (KeboardList::iterator it = m_KeyboardList.begin();
				it != m_KeyboardList.end();++it )
			{				
				Read(nCount,&pCurBuf);
				it->strKbName.resize(nCount);
				Read(&it->strKbName[0],nCount,&pCurBuf);

				Read(it->iVMSID,&pCurBuf);
				Read(it->iKbType,&pCurBuf);
				Read(it->iKbInputType,&pCurBuf);

				Read(nCount,&pCurBuf);
				it->strUserName.resize(nCount);
				Read(&it->strUserName[0],nCount,&pCurBuf);

				Read(nCount,&pCurBuf);
				it->strUserPswd.resize(nCount);
				Read(&(it->strUserPswd[0]),nCount,&pCurBuf);

				Read(it->iPort,&pCurBuf);

				Read(nCount,&pCurBuf);
				it->strIP.resize(nCount);
				Read(&(it->strIP[0]),nCount,&pCurBuf);

				Read(it->iSerialComPort,&pCurBuf);
			
				Read(nCount,&pCurBuf);
				it->strSerialParam.resize(nCount);
				Read(&(it->strSerialParam[0]),nCount,&pCurBuf);

				Read(it->iSerialComType,&pCurBuf);
			}
			return true;
		}
		virtual bool EnCode(char* buf,const int nSize)
		{
			if (nSize < GetSize())
			{
				return false;
			}
			char * pCurBuf = buf;
			int nCount = 0;
			Write((int)m_KeyboardList.size(),&pCurBuf);
			for (KeboardList::iterator it = m_KeyboardList.begin();
				it != m_KeyboardList.end();++it )
			{
				nCount = it->strKbName.size();
				Write(nCount,&pCurBuf);				
				Write(&it->strKbName[0],nCount,&pCurBuf);

				Write(it->iVMSID,&pCurBuf);
				Write(it->iKbType,&pCurBuf);
				Write(it->iKbInputType,&pCurBuf);

				nCount = it->strUserName.size();
				Write(nCount,&pCurBuf);				
				Write(&it->strUserName[0],nCount,&pCurBuf);

				nCount = it->strUserPswd.size();
				Write(nCount,&pCurBuf);				
				Write(&(it->strUserPswd[0]),nCount,&pCurBuf);

				Write(it->iPort,&pCurBuf);

				nCount = it->strIP.size();
				Write(nCount,&pCurBuf);			
				Write(&(it->strIP[0]),nCount,&pCurBuf);

				Write(it->iSerialComPort,&pCurBuf);			
				
				nCount = it->strSerialParam.size();
				Write(nCount,&pCurBuf);			
				Write(&(it->strSerialParam[0]),nCount,&pCurBuf);

				Write(it->iSerialComType,&pCurBuf);
			}
			return true;
		}
	}StruGetKeyboardResponse, *StruGetKeyboardResponsePtr;
	/**************************************************************************************************
	CMD_ID		: CMD_GET_KEBOARDINFO_RESPONSE
	CMD_NAME	: CMD_GET_KEBOARDINFO_RESPONSE
	DateTime	: 2013/05/17 15:34	
	Description	: 获取系统管理员信息回复
	Author		: cyt
	Note		: 
	**************************************************************************************************/
	#define		CMD_GET_KEBOARDINFO_RESPONSE		1019

	/**************************************************************************************************
	CMD_ID		: CMD_GET_DEVCHNNINFO_REQUST
	CMD_NAME	: CMD_GET_DEVCHNNINFO_REQUST
	DateTime	: 2013/05/17 15:34	
	Description	: 获取系统管理员信息回复
	Author		: cyt
	Note		: 
	**************************************************************************************************/
	#define		CMD_GET_DEVCHNNINFO_REQUST		1020
			/**************************************************************************************************
	CMD_ID		: CMD_GET_DEVCHNNINFO_RESPONSE
	CMD_NAME	: CMD_GET_DEVCHNNINFO_RESPONSE
	DateTime	: 2013/05/17 15:34	
	Description	: 获取系统管理员信息回复
	Author		: cyt
	Note		: 
	**************************************************************************************************/
	#define		CMD_GET_DEVCHNNINFO_RESPONSE		1021

	
	typedef struct StruDevChnnel
	{	
		INT32 iPlatformID; //平台ID
		INT32 iDevID;//设备ID
		INT32 iDevChannelID;//设备通道ID
		INT32 iChannelType;//通道类型		
		std::string strChannelExtParam;//通道扩展参数	
	}StruDevChnnel, *StruDevChnnelPtr;
	typedef struct StruDevChnnelResponse :public Packet
	{		
		std::vector<StruDevChnnel> DevChnList;
		virtual int GetSize()
		{
			INT32 nSize = 0;
			for (std::vector<StruDevChnnel>::iterator it = DevChnList.begin();
				it != DevChnList.end();++it )
			{
				nSize += sizeof(int) //数组大小
					+ sizeof(it->iPlatformID)
					+ sizeof(it->iDevID)
					+ sizeof(it->iDevChannelID)
					+ sizeof(it->iChannelType)
					+ sizeof(int) + it->strChannelExtParam.size();

			}
			return nSize;
		}

		virtual bool DeCode(char* buf,const int nSize)
		{
			if( buf == NULL || nSize<=0 )return false;
			char * pCurBuf = buf;
			int nCount = 0;
			Read(nCount,&pCurBuf);
			if ( nCount > nSize )
			{
				return false;
			}
			DevChnList.resize(nCount);
			for (std::vector<StruDevChnnel>::iterator it = DevChnList.begin();
				it != DevChnList.end();++it )
			{
				nCount = 0;
				Read(it->iPlatformID,&pCurBuf);
				Read(it->iDevID,&pCurBuf);
				Read(it->iDevChannelID,&pCurBuf);
				Read(it->iChannelType,&pCurBuf);
				Read(nCount,&pCurBuf);
				it->strChannelExtParam.resize(nCount);
				Read(&(it->strChannelExtParam[0]),nCount,&pCurBuf);
			}
			return true;
		}
		virtual bool EnCode(char* buf,const int nSize)
		{
			if (nSize < GetSize())
			{
				return false;
			}
			char * pCurBuf = buf;
			Write((int)DevChnList.size(),&pCurBuf);

			for (std::vector<StruDevChnnel>::iterator it = DevChnList.begin();
				it != DevChnList.end();++it )
			{
				Write(it->iPlatformID,&pCurBuf);
				Write(it->iDevID,&pCurBuf);
				Write(it->iDevChannelID,&pCurBuf);
				Write(it->iChannelType,&pCurBuf);
				Write((int)it->strChannelExtParam.size(),&pCurBuf);
				Write(it->strChannelExtParam.c_str(),it->strChannelExtParam.size(),&pCurBuf);
			}
			return true;
		}
	}StruDevChnnelResponse, *StruDevChnnelResponsePtr;

	/**************************************************************************************************
	CMD_ID		: CMD_GET_VERSION_REQUEST
	CMD_NAME	: CMD_GET_VERSION_RESPONSE
	DateTime	: 2013/06/17 10:34	
	Description	: 获取后台的版本
	Author		: lxp
	Note		: 
	**************************************************************************************************/
	#define CMD_GET_VERSION_REQUEST   1022  
	#define CMD_GET_VERSION_RESPONSE   1023  
	typedef   struct  StruCmdGetVersionResponse
	{
		INT32	iOperResult;				//	回复结果参照EnumErrorCode
		INT32   iMajorVersion;	
		INT32	iMinorVersion;
		INT32   iInnerVersion;
		INT32   iRevVersion;
	}StruCmdGetVersionResponse,*StruCmdGetVersionResponsePtr;

	/**************************************************************************************************
	CMD_ID		: CMD_GET_HOT_SPOT_REQUEST
	CMD_NAME	: CMD_GET_HOT_SPOT_REQUEST
	DateTime	: 2013/09/03 15:33	
	Description	: 获取用户热点摄像头点击数据请求
	Author		: wjb
	Note		: 
	**************************************************************************************************/
	#define		CMD_GET_HOT_SPOT_REQUEST	 1025
	
	typedef	enum	EnumHotspotReqType
	{
		Enum_Hotspot_Total = 1,
		Enum_Hotspot_Range
	}EnumHotspotReqType;

	typedef	struct	StruCmdGetHotspotRequest
	{
		INT32	nHotspotType;			//参考EnumHotspotReqType
		char		szBeginTime[64];		//开始时间.
		char		szEndTime[64];			//结束时间.
		INT32	nHotspotChnNum;	//请求通道数量.
	}StruCmdGetHotspotRequest, *StruCmdGetHotspotRequestPtr;
	/**************************************************************************************************
	CMD_ID		: CMD_GET_HOT_SPOT_RESPONSE
	CMD_NAME	: CMD_GET_HOT_SPOT_RESPONSE
	DateTime	: 2013/09/03 15:36	
	Description	: 获取用户热点摄像头点击数据回复
	Author		: wjb
	Note		: 
	**************************************************************************************************/
	#define		CMD_GET_HOT_SPOT_RESPONSE	 1026
	
	/**************************************************
	用户热点摄像头点击数据回复JSon结构,用root表示:
	root["OperResult"]:返回结果,OPER_RESULT_SUCCESS成功
	root["AllChannelNum"]:所有的摄像头数目
	root["AllChannel"]:所有的摄像头信息,此为数组 
	    jChannel["ChannelID"] :摄像头ID
		jChannel["ChannelName"] :摄像头名称
		jChannel["TotalStats"] :摄像头累计点击指数
		jChannel["WeekStats"] : 摄像头周点击指数
		jChannel["DayStats"] : 摄像头日点击指数
	**************************************************/
	
		//级联的服务在线状态
	#define CMD_CASCADE_SERVICE_RUN_STATUS_REQUEST   1027  
	#define CMD_CASCADE_SERVICE_RUN_STATUS_RESPONSE   1028 
	//级联的服务在线状态json结构
	/*
	root["OperResult"]:返回结果,OPER_RESULT_SUCCESS成功
	root[PlatformNum]:平台个数，此为数组
		jInfo[PlatformID]:平台ID
		jInfo[ParentPlatformID]:父平台ID
		jInfo[Online]:是否在线 值见EnumDevStatusType
	*/	

	/**************************************************************************************************
	CMD_ID		: CMD_ALARM_SEND
	CMD_NAME	: CMD_ALARM_SEND
	DateTime	: 2013/09/03 15:36	
	Description	: 级联告警转发
	Author		: wjb
	Note		: 级联告警的内容结构体依赖于原E_CMD_ALARM
	**************************************************************************************************/
	#define		CMD_ALARM_SEND	 1029
	
	//请求连接下级平台的协议
	#define CMD_CASCADE_PLATFORM_CONNECT_REQUEST   1030
	typedef enum EnumConnectType
	{
		E_TYPE_CONNECT = 1,				//连接
		E_TYPE_RECONNECT,				//重连
		E_TYPE_DELCONNECT					//删除连接
	}EnumConnectType;
	/*
	root["Flag"]: 见枚举EnumConnectType
	root["ServID"]:服务ID。有就填真实值，没有就填0.
	root["PlatformID"]:平台ID
	root["IsDns"]:IP或者域名的标志 0:IP 1:域名
	root["Addr"]:IP或者域名
	root["Port"]:端口
	root["UserName"]:用户名
	root["Password"]:密码
	*/
	#define CMD_CASCADE_PLATFORM_CONNECT_RESPONSE   1031 
	/*
	root["OperResult"]:返回结果,OPER_RESULT_SUCCESS连接成功,其它值失败。
	root["PlatformID"]:平台ID
	*/

	//级联平台GIS信息同步请求
	#define CMD_CASCADE_GIS_INFO_REQUEST   1032  
	/*
	root["Flag"]:0表示取全部。1表示取增量。
	*/
	#define CMD_CASCADE_GIS_INFO_RESPONSE   1033 
	/*  这个应答只是表示BMS收到此请求，具体的应答结果由CMD_CASCADE_GIS_INFO_NOTICE返回。
	root["OperResult"]:结果,OPER_RESULT_SUCCESS成功,其它值失败。
	*/

	//BMS之间或者BMS->GIS的级联平台GIS信息同步请求
	#define CMD_BMS_CASCADE_GIS_INFO_REQUEST   1034  
	/*
	root["Flag"]:0表示取全部。1表示取增量。
	*/
	#define CMD_BMS_CASCADE_GIS_INFO_RESPONSE   1035 
	/*  
	root["OperResult"]:结果,OPER_RESULT_SUCCESS成功,其它值失败。
	GIS内容为json结构，由gis系统自拟。（当失败时，没有此json结构）
	*/

	//级联平台GIS信息主动同步通知
	#define CMD_CASCADE_GIS_INFO_NOTICE   1036  
	/*
	root["OperResult"]:结果,OPER_RESULT_SUCCESS成功,其它值失败。
	GIS内容为json结构，由gis系统自拟
	*/
	#define CMD_CASCADE_GIS_INFO_NOTICE_RESPONSE   1037 
	/*
	root["OperResult"]:返回结果,OPER_RESULT_SUCCESS成功,其它值失败。
	*/

	//级联的服务在线状态主动通知
	#define CMD_CASCADE_SERVICE_RUN_STATUS_NOTICE   1038  
	/*结构同CMD_CASCADE_SERVICE_RUN_STATUS_RESPONSE
	*/
	#define CMD_CASCADE_SERVICE_RUN_STATUS_NOTICE_RESPONSE   1039 
	/*
	root["OperResult"]:返回结果,OPER_RESULT_SUCCESS成功
	*/

	//级联平台回环检测
	#define CMD_CASCADE_LOOPBACK_CHECK_REQUEST   1040  
	/*
	root[PlatformID]:平台ID
	root[ParentPlatformID]:平台ID
	*/
	#define CMD_CASCADE_LOOPBACK_CHECK_RESPONSE   1041 
	/*
	root["OperResult"]:回环检测结果,OPER_RESULT_SUCCESS成功,其它值失败。
	root[PlatformID]:平台ID
	*/
	/*********************************************************************************************/
	
	/**************************************************************************************************
	CMD_ID		: CMD_GET_LAYOUT_REQUEST
	CMD_NAME	: CMD_GET_LAYOUT_REQUEST
	DateTime	: 2013/9/4 14:10:41 	
	Description	: 获取用户布局数据请求
	Author		: lxp
	Note		: 
	**************************************************************************************************/
	#define		CMD_GET_LAYOUT_REQUEST 1042
	
	/**************************************************************************************************
	CMD_ID		: CMD_GET_LAYOUT_RESPONSE
	CMD_NAME	: CMD_GET_LAYOUT_RESPONSE
	DateTime	: 2013/9/4 14:10:41 	
	Description	: 获取用户布局数据回复.
	Author		: lxp
	Note		: 
	**************************************************************************************************/
	#define		CMD_GET_LAYOUT_RESPONSE 1043
	
	/************************************************************************
	用户布局数据回复JSon结构,用root表示:
	root["operResult"]:返回结果,0成功,非0失败.
	root["layoutNum"]:布局数目.
	root["allLayout"]:布局数组.
		root["id"]:布局id.
		root["name"]: 布局名字.
		root["isDefault"]:是否是默认.
		root["layoutInfo"]:布局json串,这个由客户端自己生成服务端不操作这个字段.
	************************************************************************/
	
		/**************************************************************************************************
	CMD_ID		: CMD_OPER_LAYOUT_REQUEST
	CMD_NAME	: CMD_OPER_LAYOUT_REQUEST
	DateTime	: 2013/9/4 14:31:15 	
	Description	: 获取用户布局数据回复.
	Author		: lxp
	Note		: 
	**************************************************************************************************/
	#define		CMD_OPER_LAYOUT_REQUEST  1044
	
	typedef	enum EnumLayoutType
	{
		ENUM_ADD_LAYOUT,
		ENUM_DEL_LAYOUT,
		ENUM_EDIT_LAYOUT,
	};

	typedef	 struct	StruCmdOperLayoutRequest
	{
		INT32	iID;							//布局id.
		INT32	iOperType;				//参考.EnumLayoutType;
		INT32	iIsDefault;					//是否是默认.
		char		szLayoutName[128];	//布局名称.
		INT32	iLayoutInfoLen;			//数据字段定义成4000,一般用不了那么多,改成变长.
		char		szLayoutInfo[1];
	}StruCmdOperLayoutRequest, *StruCmdOperLayoutRequestPtr;


	/**************************************************************************************************
	CMD_ID		: CMD_OPER_LAYOUT_RESPONSE
	CMD_NAME	: CMD_OPER_LAYOUT_RESPONSE
	DateTime	: 2013/9/4 14:31:15 	
	Description	: 获取用户布局数据回复.
	Author		: lxp
	Note		: 
	**************************************************************************************************/
	#define		CMD_OPER_LAYOUT_RESPONSE	1045
	
	/************************************************************************
	用户布局数据回复JSon结构,用root表示:
	root["operResult"] 返回结果,0成功,非0失败.
	************************************************************************/
	
	/**************************************************************************************************
	CMD_ID		: CMD_INCREMENT_UPDATE_NOTIFY
	CMD_NAME	: CMD_INCREMENT_UPDATE_NOTIFY
	DateTime	: 2013/9/4 14:31:15 	
	Description	: 增量更新.
	Author		: lxp
	Note		: 
	**************************************************************************************************/
	#define		CMD_INCREMENT_UPDATE_NOTIFY	1046	
	
	//哪个表更新了.
	typedef		enum	EnumUpdateTable
	{	
		ENUM_DOMAIN_UPDATE = 0,
		ENUM_DEVICE_UPDATE,
		ENUM_DEVCHN_UPDATE,
	}EnumUpdateTable;
		
	/************************************************************************
	用户布局数据回复JSon结构,用root表示:
	root["MSGTYPE"]   参考EnumUpdateTable.
	root["UpdateType"] 参考EnumUpdateType.

	域json格式.
	root["MSGTYPE"]			= EnumUpdateTable;
	root["UpdateType"]		= EnumUpdateType;
	root["DomainID"]			= Department.nDepID;
	root["DomainName"]	= Department.szDepName;
	root["ParentID"]			= Department.nParentID;
	root["PlatformID"]			= Department.nPlatformID;

	设备json格式.
	root["MSGTYPE"]			= EnumUpdateTable;
	root["UpdateType"]		= EnumUpdateType;
	root["DevID"]				= DecInfo.nDASID;
	root["DomainID"]			= DecInfo.nDomainID;
	root["CategoryID"]		= DecInfo.nCategoryID;
	root["DevName"]			= DecInfo.szDevName;
	root["PlatformID"]			= DecInfo.nPlatformID;
	root["DASID"]				= DecInfo.nDASID;
	root["DevPassword"]		= DecInfo.szDevPassword;
	root["DevUserName"]	= DecInfo.szDevUserName;
	
	//通道
	root["MSGTYPE"]			= EnumUpdateTable;
	root["UpdateType"]		= EnumUpdateType;
	root["ID"]						= DecChnInfo.nID;
	root["DevID"]				= DecChnInfo.nDevID;
	root["DomainID"]			= DecChnInfo.nDomainID;
	root["ChnTypeID"]		= DecChnInfo.nChnTypeID;
	root["ChnNo"]				= DecChnInfo.nChnNo;
	root["PlatformID"]			= DecChnInfo.nPlatformID;
	root["ChnName"]			= DecChnInfo.szChnName;
	root["28181Code"]		= DecChnInfo.sz28181Code;
	root["nStreamType"]		= DecChnInfo.nStreamType;
	************************************************************************/
	
	/**************************************************************************************************
	CMD_ID		: CMD_INCREMENT_UPDATE_NOTIFY
	CMD_NAME	: CMD_INCREMENT_UPDATE_NOTIFY
	DateTime	: 2013/9/4 14:31:15 	
	Description	: 增量更新通知客户端.
	Author		: lxp
	Note		: 
	**************************************************************************************************/
	#define		CMD_INCREMENT_UPDATE_NOTIFY_CLI	1047
	
	/************************************************************************
	
	//通道.
	root["Type"]					VIDEO_CHN,INPUT_CHN,OUTPUT_CHN
	root["UpdateType"]		EnumUpdateType
	root["UpdateTable"]		EnumUpdateTable
	root["ID"]				xx_xx_xx_xx_xx.				//删除时用这个.
	root["OldID"]			xx_xx_xx_xx_xx.				//修改时才有意义。增加和删除时填空字符串。
	root["Name"]				
	root["DefaultStream"]	//默认码流.

	//设备.
	root["Type"]					VIDEO_CHN,INPUT_CHN,OUTPUT_CHN
	root["UpdateType"]		EnumUpdateType
	root["UpdateTable"]		EnumUpdateTable
	root["ID"]				xx_xx_xx_#_xx.
	root["OldID"]			xx_xx_xx_xx_xx.				//修改时才有意义。增加和删除时填空字符串。
	root["Name"]				
	root["Status"]				//状态。

	//域
	root["Type"]					VIDEO_CHN,INPUT_CHN,OUTPUT_CHN
	root["UpdateType"]		EnumUpdateType
	root["UpdateTable"]		EnumUpdateTable
	root["ID"]						xx_xx_#_#_xx.
	root["Name"]				
	**************************************************************************************************/
	
	/**************************************************************************************************
	CMD_ID		: CMD_CHN_STATUS_NOTICE  
	CMD_NAME		: 通道状态通知
	DateTime		: 2013/10/21 
	Author 		: chc      
	Description	: 
	Note			: 
	**************************************************************************************************/
	#define		CMD_CHN_STATUS_NOTICE			1048				// 通道状态通知命令
	/*
	root["ChnNum"]:通道数目,数组个数
	ChnInfo["ID"]: 格式 xx_xx_xx_xx_xx
	ChnInfo["Status"]: 值详见EnumDevStatusType
	*/

	#define		CMD_CHN_STATUS_NOTICE_RESPONSE	 1049				// 通道状态通知应答命令
	//应答不需要内容

	/**************************************************************************************************
	CMD_ID		: CMD_GET_USER_ALL_OPEN_VIDEO_INFO_REQUEST
	CMD_NAME	: CMD_GET_USER_ALL_OPEN_VIDEO_INFO_REQUEST
	DateTime	: 2013/10/23 17:29:15 	
	Description	: 管理员远程强制关闭用户视频,获取用户所有视频信息
	Author		: wjb
	Note		: 
	**************************************************************************************************/
	#define		CMD_GET_USER_ALL_OPEN_VIDEO_INFO_REQUEST	1050
	/*JSON格式:
	root["UserName"]:用户名,不能为空
	*/

	/**************************************************************************************************
	CMD_ID		: CMD_GET_USER_ALL_OPEN_VIDEO_INFO_RESPONSE
	CMD_NAME	: CMD_GET_USER_ALL_OPEN_VIDEO_INFO_RESPONSE
	DateTime	: 2013/10/23 17:31:15 	
	Description	: 管理员远程强制关闭用户视频,获取用户所有视频信息应答
	Author		: wjb
	Note		: 
	**************************************************************************************************/
	#define		CMD_GET_USER_ALL_OPEN_VIDEO_INFO_RESPONSE	1051

	/*JSON格式:
	root["OperResult"]:操作结果,EnumErrorCode错误码,OPER_RESULT_FAIL操作错误;LOG_RESULT_USER_NAME_ERROR,用户名错误
	root["UserName"]:用户名
	root["AllOpenVideo"]:此用户下所有打开的视频,此为数组
		jChannel["Node"]:视频节点名称,#_#_#_#_#字符串
		jChannel["VideoType"]:视频类型,EnumOpenVideoType
		jChannel["ChannelName"]:摄像机名称
	*/

	/**************************************************************************************************
	CMD_ID		: CMD_OPER_USER_OPEN_VIDEO
	CMD_NAME	: CMD_OPER_USER_OPEN_VIDEO
	DateTime	: 2013/10/23 17:34:15 	
	Description	: 管理员远程强制关闭用户视频,增加或减少用户打开的视频数
	Author		: wjb
	Note		: 
	**************************************************************************************************/
	#define		CMD_OPER_USER_OPEN_VIDEO	1052
	
	/*JSON格式:
	root["UserName"]:用户名
	root["ChannelName"]:通道名称,#_#_#_#_#字符串
	root["VideoType"]:视频类型,EnumOpenVideoType
	root["OperType"]:操作类型,USER_OPENED_VIDEO_SAVE:保存打开的视频,USER_OPENED_VIDEO_DEL:删除打开的视频
	*/
	
	typedef enum EnumOperUserOpenedVideoType
	{
		USER_OPENED_VIDEO_SAVE =0,//保存
		USER_OPENED_VIDEO_DEL     //关闭
	}EnumOperUserOpenedVideoType;

	typedef enum EnumOpenVideoType
	{
		VIDEO_REAL,//实时视频
		VIDEO_RECORD//录像视频
	}EnumOpenVideoType;

	/**************************************************************************************************
	CMD_ID		: CMD_FORCE_USER_CLOSE_VIDEO
	CMD_NAME	: CMD_FORCE_USER_CLOSE_VIDEO
	DateTime	: 2013/10/23 17:36:15 	
	Description	: 管理员远程强制关闭用户视频
	Author		: wjb
	Note		: 
	**************************************************************************************************/
	#define		CMD_FORCE_USER_CLOSE_VIDEO	1053

	/*JSON格式:
	root["UserName"]:用户名
	root["AllUserVideo"]:需要关闭的用户视频,此为数组
		jChannel["Node"]:视频节点名称,#_#_#_#_#字符串
		jChannel["VideoType"]:视频类型,EnumOpenVideoType
	*/
	
	/**************************************************************************************************
	CMD_ID		: CMD_CLOSE_VIDEO_WITH_ALL_USER
	CMD_NAME	: CMD_CLOSE_VIDEO_WITH_ALL_USER
	DateTime	: 2013/10/25 17:16:15 	
	Description	: 管理员远程强制关闭用户视频,关闭所有正在观看某视频的所有用户的该视频
	Author		: wjb
	Note		: 
	**************************************************************************************************/
	#define		CMD_CLOSE_VIDEO_WITH_ALL_USER	1054

	/*JSON格式:
	root["Node"]:视频节点名称,#_#_#_#_#字符串
	root["AllUserVideo"]:需要关闭的用户视频,此为数组
		jUser["UserName"]:用户名
		jUser["IP"]:用户IP地址
		jUser["Port"]:端口
		jUser["VideoType"]:视频类型,EnumOpenVideoType
	*/

	/**************************************************************************************************
	CMD_ID		: CMD_CLOSE_VIDEO_RESPONSE
	CMD_NAME	: CMD_CLOSE_VIDEO_RESPONSE
	DateTime	: 2013/10/25 17:19:15 	
	Description	: 管理员远程强制关闭用户视频应答结果
	Author		: wjb
	Note		: 
	**************************************************************************************************/
	#define		CMD_CLOSE_VIDEO_RESPONSE	1055

	/*JSON格式:
	root["OperResult"]:操作结果,参考EnumErrorCode枚举类型,OPER_RESULT_SUCCESS,操作成功;USER_NO_OPERATOR_RIGHT:用户无操作权限;USER_NO_PRIORITY,用户优先级不够;
	root["FailSize"]:操作失败的数目
	root["AllFailNode"]:操作失败返回的信息条目
		root["UserName"]:用户名称
		root["IP"]:用户IP地址
		root["Port"]:用户端口
		root["Node"]:节点名称
		root["VideoType"]:视频类型
	*/

	/**************************************************************************************************
	CMD_ID		: CMD_CLOSE_VIDEO_RESPONSE_CLIENT
	CMD_NAME	: CMD_CLOSE_VIDEO_RESPONSE_CLIENT
	DateTime	: 2013/11/4 15:45:15 	
	Description	: 管理员远程强制关闭用户视频,将要关闭的视频发送给客户端
	Author		: wjb
	Note		: 
	**************************************************************************************************/
	#define		CMD_CLOSE_VIDEO_RESPONSE_CLIENT	1056

	/*JSON格式:
	root["AllChannelName"]:所有要关闭视频的数组
		jChannel["Node"]:要关闭视频的节点名称
		jChannel["VideoType"]:要关闭的视频类型
	*/
	
	/**************************************************************************************************
	CMD_ID		: CMD_GET_USERINFO_WITH_CHANNEL_RESPONSE
	CMD_NAME	: CMD_GET_USERINFO_WITH_CHANNEL_RESPONSE
	DateTime	: 2013/11/4 15:45:15 	
	Description	: 获取某个摄像机正在观看的用户信息请求
	Author		: wjb
	Note		: 
	**************************************************************************************************/
	#define		CMD_GET_USERINFO_WITH_CHANNEL_REQUEST	1057

	/*JSON格式:
	root["ChannelName"]:摄像机节点,#_#_#_#_#字符串
	*/
	
	/**************************************************************************************************
	CMD_ID		: CMD_GET_USERINFO_WITH_CHANNEL_RESPONSE
	CMD_NAME	: CMD_GET_USERINFO_WITH_CHANNEL_RESPONSE
	DateTime	: 2013/11/4 15:45:15	
	Description	: 获取某个摄像机正在观看的用户信息回复
	Author		: wjb
	Note		: 
	**************************************************************************************************/
	#define		CMD_GET_USERINFO_WITH_CHANNEL_RESPONSE	1058

	/*JSON格式:
	root["ChannelName"]:摄像机节点,#_#_#_#_#字符串
	root["AllUser"]:所有的用户,数组
		jUser["AccountName"]:账号
		jUser["Name"]:姓名
		jUser["UserType"]:用户类型
		jUser["OrganizeID"]:所属组织
		jUser["IP"]:ip地址
		jUser["Port"]:端口
		jUser["AllVideoType"]:类型数组
			jType["VideoType"]:视频类型
	*/
	
	/**************************************************************************************************
	CMD_ID		: CMD_SEND_USER_ALL_OPEN_VIDEO_AFTER_LOGIN
	CMD_NAME	: CMD_SEND_USER_ALL_OPEN_VIDEO_AFTER_LOGIN
	DateTime	: 2013/11/4 15:45:15	
	Description	: 当用户浏览视频时,客户端与BMS断开,此时视频还在连接.当客户端与BMS重连登录成功时,发送打开的实时视频或录像视频信息
	Author		: wjb
	Note		: 
	**************************************************************************************************/
	#define		CMD_SEND_USER_ALL_OPEN_VIDEO_AFTER_LOGIN	1059
	
	/*JSON格式:
	root["UserName"]:用户名
	root["OperType"]:操作类型,USER_OPENED_VIDEO_SAVE:保存打开的视频,USER_OPENED_VIDEO_DEL:删除打开的视频
	root["AllNode"]:所有打开的视频节点信息
		root["Node"]:通道名称,#_#_#_#_#字符串
		root["VideoType"]:视频类型,EnumOpenVideoType
	*/
	/**************************************************************************************************
	CMD_ID		: CMD_GET_CHN_STATUS  
	CMD_NAME		: 取通道状态
	DateTime		: 2013/10/21 
	Author 		: chc      
	Description	: 
	Note			: 
	**************************************************************************************************/
	#define		CMD_GET_CHN_STATUS			1060			//取通道状态
	//不需要请求内容。应答结果为CMD_CHN_STATUS_NOTICE命令
	
	/**************************************************************************************************
	/**************************************************************************************************
	CMD_ID		: CMD_GET_ALL_DEV_CHN_STATUS_REQUEST  
	CMD_NAME		: 取所有通道和设备状态
	DateTime		: 2013/11/8
	Author 		:       
	Description	: 
	Note			: 
	**************************************************************************************************/
	#define		CMD_GET_ALL_DEV_CHN_STATUS_REQUEST		1061			//取所有通道和设备状态hmc服务器用到.
	//不需要请求内容。
	
	/**************************************************************************************************
	/**************************************************************************************************
	CMD_ID		: CMD_GET_CHN_STATUS  
	CMD_NAME		: 取所有通道和设备状态
	DateTime		: 2013/11/8
	Author 		:       
	Description	: 
	Note			: 
	**************************************************************************************************/
	#define		CMD_GET_ALL_DEV_CHN_STATUS_RESPONSE			1062		//取所有通道和设备状态hmc服务器用到.
	
	//回复json格式.
	/*
	root["DevStatusInfo"]
				["DevStatusArray"]				//数组.数组的个数可以调用json对象的size方法得到.
					 ["NodeID"]							
					 ["Status"]							//状态.
	root["ChnStatusInfo"]
				["ChnStatusArray"]				//数组.数组的个数可以调用json对象的size方法得到.
					["NodeID"]								
					["Status"]							//状态.
					["ID"]								//通道主键id.
	root["Result"]			0成功,非0失败.
	*/

/**************************************************************************************************
	CMD_ID		: CMD_CASCADE_PLATFORM_DATA_SYN_NOTICE  
	CMD_NAME		: 
	DateTime		: 2013/11/8
	Author 		:       
	Description	: 
	Note			: 
	**************************************************************************************************/
	#define		CMD_CASCADE_PLATFORM_DATA_SYN_NOTICE		1063	//级联平台数据同步通知消息		
	typedef enum EnumSynType
	{
		E_TYPE_LOAD = 1,				//挂载、重新挂载
		E_TYPE_UNLOAD,					//取消挂载
		E_TYPE_UPDATE,					//数据有更新
		E_TYPE_UPDATE_ALL,			//全部刷新.
	}EnumSynType;
	//json格式.
	/*
	root["Flag"] : 内容见EnumSynType	
	root["PlatformID"]:父平台ID#下级平台ID#下下级平台ID#_父平台ID#下级平台ID#下下级平台ID#...
	如果只有一个平台发生了应用更新，则格式为平台ID#_
	*/
	
	/**************************************************************************************************
	CMD_ID		: CMD_GET_ALL_DEV_CHN_STATUS_RESPONSE  
	CMD_NAME		: //级联平台数据同步通知消息
	DateTime		: 2013/11/8
	Author 		:       
	Description	: 
	Note			: 
	**************************************************************************************************/
	#define		CMD_CASCADE_PLATFORM_DATA_SYN_REPONSE			1064	//级联平台数据同步通知消息
	/*
	root["OperResult"]:结果,OPER_RESULT_SUCCESS成功,其它值失败。
	*/

	/**************************************************************************************************
	CMD_ID		: CMD_CASCADE_PLATFORM_RELATION_NOTICE  
	CMD_NAME		: 
	DateTime		: 2013/11/8
	Author 		:       
	Description	: 
	Note			: 
	**************************************************************************************************/
	#define		CMD_CASCADE_PLATFORM_RELATION_NOTICE		1065	//	级联平台关联关系通知	
	//json格式.
	/*
	root["Flag"] : 内容见EnumConnectType	
	root["PlatformNum"]:平台个数
	jInfo["PlatformID"]:平台ID
	*/
	
	/**************************************************************************************************
	CMD_ID		: CMD_CASCADE_PLATFORM_RELATION_REPONSE  
	CMD_NAME		: 级联平台关联关系通知	
	DateTime		: 2013/11/8
	Author 		:       
	Description	: 
	Note			: 
	**************************************************************************************************/
	#define		CMD_CASCADE_PLATFORM_RELATION_REPONSE			1066	//
	/*
	root["OperResult"]:结果,OPER_RESULT_SUCCESS成功,其它值失败。
	*/


	/**************************************************************************************************
	CMD_ID		: CMD_SAVE_CAPTURE_REQUEST  
	CMD_NAME		: 
	DateTime		: 2014/02/11
	Author 		:       
	Description	: 
	Note			: 
	**************************************************************************************************/
	#define		CMD_SAVE_CAPTURE_REQUEST							1067	//	保存抓图记录日志	
	typedef struct StruSaveCaptureRequest
	{
		INT32 iPlatformID;				// 平台ID
		INT32 iDevID;					// 设备ID
		INT32 iChnID;					// 通道ID
		INT32 iOperResult;				// 操作结果
	}StruSaveCaptureRequest,*StruSaveCaptureRequestPtr;
	
	
	/**************************************************************************************************
	CMD_ID		: CMD_SAVE_CAPTURE_RESPONSE  
	CMD_NAME		: 保存抓图记录日志应答		
	DateTime		: 2014/02/11
	Author 		:       
	Description	: 
	Note			: 
	**************************************************************************************************/
	#define		CMD_SAVE_CAPTURE_RESPONSE							1068	//

	typedef struct StruSaveCaptureResponse
	{
		INT32				iResult;
	}StruSaveCaptureResponse,*StruSaveCaptureResponsePtr;
	
	

	#define   CMD_CASCADE_GET_USERONLINESTATUS_REQUEST  1069					//获取级联平台在线用户信息

	/*JSON格式:
	root["PlatformID"]: 如果平台ID为0，表示取所有平台的在线用户
	*/

	#define   CMD_CASCADE_GET_USERONLINESTATUS_RESPONSE  1070
	
	//json格式.
	/*
	root["OperResult"] : 结果,OPER_RESULT_SUCCESS成功,其它值失败
	root["PlatformNum"]:平台个数
		Item["PlatformID"]:平台ID
		Item["Result"] : 结果,OPER_RESULT_SUCCESS成功,其它值失败，表示取该平台的在线用户失败。
		Item["Num"]:用户个数			
			jInfo["UserName"]:用户名
			jInfo["Online"]:  在线状态	参考结构EnumUserStatusType
			jInfo["LoginPort"]:  登录的端口号
			jInfo["UserIP"]:  当前登录IP
			jInfo["LoginTime"]:  登录时间
			jInfo["AccountName"]:  账户名称
			jInfo["OrgnaizeName"]:  所属组织
	*/

	#define   CMD_BMS_CASCADE_GET_USERONLINESTATUS_REQUEST  1071		//BMS之间专用的通讯协议。获取级联平台在线用户信息

	/*JSON格式:
	root["PlatformID"]: 如果平台ID为0，表示取所有平台的在线用户
	root["SrcPlatformID"]: 发起请求的源平台ID
	root["SrcClientID"]: 发起请求的源客户端ID
	*/

	#define   CMD_BMS_CASCADE_GET_USERONLINESTATUS_RESPONSE  1072
	
	//json格式.
	/*
	root["OperResult"] : 结果,OPER_RESULT_SUCCESS成功,其它值失败
	root["DstPlatformID"]: 目的平台ID，和请求协议中的SrcPlatformID相同。
	root["DstClientID"]: 目的客户端ID，和请求协议中的SrcClientID相同。
	root["PlatformNum"]:平台个数
		Item["PlatformID"]:平台ID
		Item["Result"] : 结果,OPER_RESULT_SUCCESS成功,其它值失败，表示取该平台的在线用户失败。
		Item["Num"]:用户个数
			jInfo["UserName"]:用户名
			jInfo["Online"]:  在线状态	参考结构EnumUserStatusType
			jInfo["LoginPort"]:  登录的端口号
			jInfo["UserIP"]:  当前登录IP
			jInfo["LoginTime"]:  登录时间
			jInfo["AccountName"]:  账户名称
			jInfo["OrgnaizeName"]:  所属组织
	*/

	/**************************************************************************************************
	CMD_ID		: CMD_CASCADE_GET_USER_ALL_OPEN_VIDEO_INFO_REQUEST
	CMD_NAME	: CMD_CASCADE_GET_USER_ALL_OPEN_VIDEO_INFO_REQUEST
	DateTime	: 2013/10/23 17:29:15 	
	Description	: 管理员远程强制关闭用户视频,获取用户所有视频信息
	Author		: CHC
	Note		: 
	**************************************************************************************************/
	#define		CMD_CASCADE_GET_USER_ALL_OPEN_VIDEO_INFO_REQUEST	1073
	/*JSON格式:
	root["UserName"]:用户名,不能为空
	root["PlatformID"]:用户所在的平台ID,不能为空
	*/

	/**************************************************************************************************
	CMD_ID		: CMD_CASCADE_GET_USER_ALL_OPEN_VIDEO_INFO_RESPONSE
	CMD_NAME	: CMD_CASCADE_GET_USER_ALL_OPEN_VIDEO_INFO_RESPONSE
	DateTime	: 2013/10/23 17:31:15 	
	Description	: 管理员远程强制关闭用户视频,获取用户所有视频信息应答
	Author		: CHC
	Note		: 
	**************************************************************************************************/
	#define		CMD_CASCADE_GET_USER_ALL_OPEN_VIDEO_INFO_RESPONSE	1074

	/*JSON格式:
	root["OperResult"]:操作结果,EnumErrorCode错误码,OPER_RESULT_FAIL操作错误;LOG_RESULT_USER_NAME_ERROR,用户名错误
	root["UserName"]:用户名
	root["PlatformID"]:用户所在的平台ID,不能为空
	root["AllOpenVideo"]:此用户下所有打开的视频,此为数组
		jChannel["Node"]:视频节点名称,#_#_#_#_#字符串
		jChannel["VideoType"]:视频类型,EnumOpenVideoType
		jChannel["ChannelName"]:摄像机名称
	*/

	/**************************************************************************************************
	CMD_ID		: CMD_CASCADE_GET_USERINFO_WITH_CHANNEL_REQUEST
	CMD_NAME	: CMD_CASCADE_GET_USERINFO_WITH_CHANNEL_REQUEST
	DateTime	: 2013/11/4 15:45:15 	
	Description	: 获取级联平台某个摄像机正在观看的用户信息请求
	Author		: CHC
	Note		: 
	**************************************************************************************************/
	#define		CMD_CASCADE_GET_USERINFO_WITH_CHANNEL_REQUEST	1075

	/*JSON格式:
	root["ChannelName"]:摄像机节点,#_#_#_#_#字符串
	*/
	
	/**************************************************************************************************
	CMD_ID		: CMD_CASCADE_GET_USERINFO_WITH_CHANNEL_RESPONSE
	CMD_NAME	: CMD_CASCADE_GET_USERINFO_WITH_CHANNEL_RESPONSE
	DateTime	: 2013/11/4 15:45:15	
	Description	: 获取级联平台某个摄像机正在观看的用户信息回复
	Author		: CHC
	Note		: 
	**************************************************************************************************/
	#define		CMD_CASCADE_GET_USERINFO_WITH_CHANNEL_RESPONSE	1076

	/*JSON格式:
	root["ChannelName"]:摄像机节点,#_#_#_#_#字符串
	root["AllUser"]:所有的用户,数组
		jUser["AccountName"]:账号
		jUser["Name"]:姓名
		jUser["UserType"]:用户类型
		jUser["OrganizeID"]:所属组织
		jUser["IP"]:ip地址
		jUser["Port"]:端口
		jUser["AllVideoType"]:类型数组
			jType["VideoType"]:视频类型
	*/

	/**************************************************************************************************
	CMD_ID		: CMD_DEV_REBOOT_RULE_REQUEST
	CMD_NAME	: CMD_DEV_REBOOT_RULE_REQUEST
	DateTime	: 20141/04/08 10:48	
	Description	: 设备定时重启规则请求
	Author		: sw
	Note		: NULL
	**************************************************************************************************/
	#define	CMD_DEV_REBOOT_RULE_REQUEST		1077

	/**************************************************************************************************
	CMD_ID		: CMD_DEV_REBOOT_RULE_RESPONSE
	CMD_NAME	: CMD_DEV_REBOOT_RULE_RESPONSE
	DateTime	: 20141/04/08 10:48		
	Description	: 设备定时重启规则回复
	Author		: sw
	Note		: NULL
	**************************************************************************************************/
	#define	CMD_DEV_REBOOT_RULE_RESPONSE	1078
	
	//	设备定时重启规则回复
	typedef		struct  StruCmdDevRebootRuleResponse
	{
		INT32							iOperResult;					//	回复结果,枚举EnumErrorCode定义
		INT32							iTimeInterval;					//  时间间隔，单位：秒
	}StruCmdDevRebootRuleResponse,*StruCmdDevRebootRuleResponsePtr;


	/**************************************************************************************************
	DateTime		: 2010/8/4 9:04	
	Author 			: Liujs      
	Description		: 所有的命令号的枚举结构
	Note			: 所有命令的枚举结构
	**************************************************************************************************/
	// 所有的命令号的枚举结构
	typedef	enum	EnumCmdIDInfo
	{
		// 登陆
		E_CMD_LOGIN_REQUEST					=	CMD_LOGIN_REQUEST,				// 登陆命令
		E_CMD_LOGIN_RESPONSE				=	CMD_LOGIN_RESPONSE,				// 登陆应答命令

		// 点流
		E_CMD_GET_STREAM_REQUEST			=	CMD_GET_STREAM_REQUEST,			// 点流命令
		E_CMD_GET_STREAM_RESPONSE			=	CMD_GET_STREAM_RESPONSE,		// 点流应答命令

		// 上下线通知
		E_CMD_GET_DEV_STATUS			    =   CMD_GET_DEV_STATUS,				// 取设备状态命令
		E_CMD_DEV_STATUS_NOTICE				=	CMD_DEV_STATUS_NOTICE,			// 设备状态通知命令
		E_CMD_DEV_STATUS_NOTICE_AFFIRM      =   CMD_DEV_STATUS_NOTICE_AFFIRM,   // 设备状态通知确认命令
		E_CMD_DEV_STATUS_KEEPALIVE          =   CMD_DEV_STATUS_KEEPALIVE,       // 设备状态信息心跳


		// 基础数据请求
		E_CMD_BASS_DATA_REQUEST				=	CMD_BASS_DATA_REQUEST,			// 基础数据请求命令
		E_CMD_BASS_DATA_RESPONSE			=	CMD_BASS_DATA_RESPONSE,			// 基础数据应答命令

		// 设备列表请求
		E_CMD_DEV_DATA_REQUEST				=	CMD_DEV_DATA_REQUEST,			// 设备数据请求命令
		E_CMD_DEV_DATA_REQPONSE				=	CMD_DEV_DATA_RESPONSE,			// 设备数据请求回复命令	
			
		// 数据版本请求
		E_CMD_DATA_VERSION_REQUEST			=	CMD_DATA_VERSION_REQUEST,		// 数据版本请求
		E_CMD_DATA_VERSION_RESPONSE			=	CMD_DATA_VERSION_RESPONSE,		// 数据版本请求回复

		// 所有数据版本请求
		E_CMD_DATA_VERSION_REQUEST_EX		=	CMD_DATA_VERSION_REQUEST_EX,	// 数据版本请求
		E_CMD_DATA_VERSION_RESPONSE_EX		=	CMD_DATA_VERSION_RESPONSE_EX,	// 数据版本请求回复

		// 云台控制
		E_CMD_PTZ_CONTROL					=	CMD_PTZ_CONTROL,				// 云台控制命令
		E_CMD_PTZ_PRESET_CONTROL			=	CMD_PTZ_PRESET_CONTROL,			// 云台预置点操作命令
		E_CMD_PTZ_PRESET_CONTROL_RESPONSE	=	CMD_PTZ_PRESET_CONTROL_RESPONSE,// 云台预置点操作回复命令
		E_CMD_PTZ_PRESET_GET				=	CMD_PTZ_PRESET_GET,				// 云台预置点获取命令
		E_CMD_PTZ_PRESET_GET_RESPONSE		=	CMD_PTZ_PRESET_GET_RESPONSE,	// 云台预置点获取返回结果
		E_CMD_PTZ_PRESET_DEFAULT_REQUEST	=	CMD_PTZ_PRESET_DEFAULT_REQUEST, // 设置为默认预置点
		E_CMD_PTZ_PRESET_DEFAULT_RESPONSE	=	CMD_PTZ_PRESET_DEFAULT_RESPONSE,// 设置为默认预置点回复
		E_CMD_GET_CRUISE_TRACK_REQUEST		=	CMD_GET_CRUISE_TRACK_REQUEST,	// 云台巡航信息请求
		E_CMD_GET_CRUISE_TRACK_RESPONSE		=	CMD_GET_CRUISE_TRACK_RESPONSE,	// 云台巡航信息请求回复
		E_CMD_CRUISE_OPER_REQUEST			=	CMD_CRUISE_OPER_REQUEST,		// 云台巡航操作请求
		E_CMD_CRUISE_OPER_RESPONSE			=	CMD_CRUISE_OPER_RESPONSE,		// 云台巡航操作结果回复

		E_CMD_OPER_RESULT_NOTICE			=	CMD_OPER_RESULT_NOTICE,			// 操作结果通知

		// 告警
		E_CMD_ALARM							=	CMD_ALARM,						// 告警命令
		E_CMD_ALARM_RULE_REQUEST			=	CMD_ALARM_RULE_REQUEST,			// 告警规则请求
		E_CMD_ALARM_RULE_RESPONSE			=	CMD_ALARM_RULE_RESPONSE,		// 告警规则回复
		E_CMD_ALARM_LINKAGE_EVENT_NOTICE	=	CMD_ALARM_LINKAGE_EVENT_NOTICE,	// 告警联动事件通知
		E_CMD_ALARM_QUERY_REQUEST			=	CMD_ALARM_QUERY_REQUEST,		// 告警查询命令
		E_CMD_ALARM_QUERY_RSPONSE			=	CMD_ALARM_QUERY_RSPONSE,		// 告警查询回复命令
		E_CMD_ALARM_CONFIRM					=	CMD_ALARM_CONFIRM,				// 告警状态确认

		//获取联动任务信息
		E_CMD_GET_LINKAGE_TASK_REQUEST		=	CMD_GET_LINKAGE_TASK_REQUEST,		// 获取任务信息请求
		E_CMD_GET_LINKAGE_TASK_RESPONSE		=	CMD_GET_LINKAGE_TASK_RESPONSE,		// 获取任务信息回复
		
		// 录像规则
		E_CMD_SER_RECORD_RULE_REQUEST		=	CMD_SER_RECORD_RULE_REQUEST,	// 录像规则请求命令
		E_CMD_SER_RECORD_RULE_RESPONSE		=	CMD_SER_RECORD_RULE_RESPONSE,	// 录像规则回复命令

		// 布防撤防
		E_CMD_DEPLOYMENT_DEFENCE_REQUEST	=	CMD_DEPLOYMENT_DEFENCE_REQUEST,	// 布防撤防规则请求
		E_CMD_DEPLOYMENT_DEFENCE_RESPONSE	=	CMD_DEPLOYMENT_DEFENCE_RESPONSE,// 布防撤防规则回复
		E_CMD_DEPLOYMENT_DEFENCE			=	CMD_DEPLOYMENT_DEFENCE,			//	布防撤防命令	
		E_CMD_DEFENCE_COMMAND_RESPONSE		=	CMD_DEFENCE_COMMAND_RESPONSE,//	布防撤防命令回复
		
		// 分层获取设备树
		E_CMD_LEVEL_NODE_DATA_REQUEST		=	CMD_LEVEL_NODE_DATA_REQUEST,	// 分层获取设备树
		E_CMD_LEVEL_NODE_DATA_RESPONSE		=	CMD_LEVEL_NODE_DATA_RESPONSE,	// 分层获取设备树回复


		// 一次获取所有设备树
		E_CMD_ALL_NODE_DATA_REQUEST		    =	CMD_ALL_NODE_DATA_REQUEST,	// 一次获取所有设备树
		E_CMD_ALL_NODE_DATA_RESPONSE		=	CMD_ALL_NODE_DATA_RESPONSE,	// 一次获取所有设备树回复

		// 录像文件操作
		E_CMD_RECORD_FILE_RETRIEVAL_REQUEST		=	CMD_RECORD_FILE_RETRIEVAL_REQUEST,	// 录像文件检索请求
		E_CMD_RECORD_FILE_RETRIEVAL_V2_REQUEST  =	CMD_RECORD_FILE_RETRIEVAL_V2_REQUEST,//录像文件检索请求版本2
		E_CMD_RECORD_FILE_RETRIEVAL_RESPONSE    =	CMD_RECORD_FILE_RETRIEVAL_RESPONSE, // 录像文件检索回复
		E_CMD_RECORD_FILE_OPERATE_REQUEST		=	CMD_RECORD_FILE_OPERATE_REQUEST,	// 录像回放下载请求
		E_CMD_RECORD_FILE_OPERATE_RESPONSE		=	CMD_RECORD_FILE_OPERATE_RESPONSE,	// 录像回放下载回复
		E_CMD_RECORD_FILE_OPERATE_REQUEST_EX	=	CMD_RECORD_FILE_OPERATE_REQUEST_EX,	// 录像回放下载请求
		E_CMD_RECORD_FILE_OPERATE_RESPONSE_EX	=	CMD_RECORD_FILE_OPERATE_RESPONSE_EX,	// 录像回放下载回复

		// 获取设备连接信息
		E_CMD_DEV_LOGIN_INFO_REQUEST			=	CMD_DEV_LOGIN_INFO_REQUEST,			// 获取设备连接信息请求
		E_CMD_DEV_LOGIN_INFO_RESPONSE			=	CMD_DEV_LOGIN_INFO_RESPONSE,		// 获取设备连接信息回复

		// 语音监听
		E_CMD_VOICE_LISTENING_REQUEST			=	CMD_VOICE_LISTENING_REQUEST,		// 语音监听请求
		E_CMD_VOICE_LISTENING_RESPONSE			=	CMD_VOICE_LISTENING_RESPONSE,		// 语音监听回复

		// 语音对讲
		E_CMD_AUDIO_COMM_REQUEST				=	CMD_AUDIO_COMM_REQUEST,            // 语音对讲请求
		E_CMD_AUDIO_COMM_RESPONSE				=   CMD_AUDIO_COMM_RESPONSE,           // 语音对讲回复
		E_CMD_AUDIO_COMM_ACK					=   CMD_AUDIO_COMM_ACK,				   // 语音对讲心跳命令
		E_CMD_SEND_AUDIO_DATA                   =   CMD_SEND_AUDIO_DATA,			   // 发送音频数据

		
		// 独占控制权
		E_CMD_OPERATE_LOCK_REQUEST				=	CMD_OPERATE_LOCK_REQUEST,			//独占控制权请求
		E_CMD_OPERATE_LOCK_RESPONSE				=	CMD_OPERATE_LOCK_RESPONSE,			//独占控制权请求回复
		E_CMD_OPERATE_STATUS_QUERY_REQUEST		=	CMD_OPERATE_STATUS_QUERY_REQUEST,	//查询设备的锁定状态
		E_CMD_OPERATE_STATUS_QUERY_RESPONSE		=	CMD_OPERATE_STATUS_QUERY_RESPONSE,	//设备锁定状态查询回复
		E_CMD_OPERATE_MONOLIZE_STATUS_NOTICE	=	CMD_OPERATE_MONOLIZE_STATUS_NOTICE,	//设备锁定状态通知

		// 浏览记录
		E_CMD_SAVE_BROWSE_CHN_REQUEST			=	CMD_SAVE_BROWSE_CHN_REQUEST,		// 保存浏览通道记录请求
		E_CMD_SAVE_BROWSE_CHN_RESPONSE			=	CMD_SAVE_BROWSE_CHN_RESPONSE,		// 保存浏览通道记录回复
		E_CMD_GET_BROWSE_CHN_REQUEST			=	CMD_GET_BROWSE_CHN_REQUEST,			// 获取浏览通道记录请求
		E_CMD_GET_BROWSE_CHN_RESPONSE			=	CMD_GET_BROWSE_CHN_RESPONSE,		// 获取浏览通道记录回复

		//  时间同步
		E_CMD_SYS_TIME_SYNCHRONIZATION			=	CMD_SYS_TIME_SYNCHRONIZATION,		// 服务时间同步命令
		E_CMD_DEV_TIME_SYNCHRO_RULE_REQUEST		=	CMD_DEV_TIME_SYNCHRO_RULE_REQUEST,	// 设备时间同步规则请求
		E_CMD_DEV_TIME_SYNCHRO_RULE_RESPONSE	=	CMD_DEV_TIME_SYNCHRO_RULE_RESPONSE,	// 设备时间同步规则回复
		
		//服务运行状态信息
		E_CMD_SERVICE_RUN_STATUS_REQUEST		=	CMD_SERVICE_RUN_STATUS_REQUEST,				//获取服务运行状态信息请求
		E_CMD_SERVICE_RUN_STATUS_RESPONSE		=	CMD_SERVICE_RUN_STATUS_RESPONSE,				//获取服务运行状态信息回复

		//服务系统信息
		E_CMD_SYS_INFO_REQUEST					=	CMD_SYS_INFO_REQUEST,				//获取服务系统信息请求
		E_CMD_SYS_INFO_RESPONSE					=	CMD_SYS_INFO_RESPONSE,				//获取服务系统信息回复

		//分组轮巡数据
		E_CMD_GET_POLLING_RECORD_REQUEST		= CMD_GET_POLLING_RECORD_REQUEST,		//获取轮巡数据请求
		E_CMD_GET_POLLING_RECORD_RESPONSE		= CMD_GET_POLLING_RECORD_RESPONSE,		//获取轮巡数据请求回复
		E_CMD_SAVE_POLLING_RECORD_REQUEST		= CMD_SAVE_POLLING_RECORD_REQUEST,		//保存轮巡数据请求
		E_CMD_SAVE_POLLING_RECORD_RESPONSE		= CMD_SAVE_POLLING_RECORD_RESPONSE,		//保存轮巡数据请求回复
		
		//设备远程控制
		E_CMD_DEV_REMOTE_OPERATION_REQUEST		= CMD_DEV_REMOTE_OPERATION_REQUEST,		//设备远程控制请求
		E_CMD_DEV_REMOTE_OPERATION_RESPONSE		= CMD_DEV_REMOTE_OPERATION_RESPONSE,	//设备远程控制回复

		//获取在线用户列表
		E_CMD_USER_ONLINE_LIST_REQUEST			= CMD_USER_ONLINE_LIST_REQUEST,			//获取在线用户列表请求
		E_CMD_USER_ONLINE_LIST_REPONSE			= CMD_USER_ONLINE_LIST_RESPONSE,		//获取在线用户列表回复
	
		//获取CSS配置信息
		E_CMD_CSS_CONFIG_PARAM_REQUEST			= CMD_CSS_CONFIG_PARAM_REQUEST,			//获取配置信息请求
		E_CMD_CSS_CONFIG_PARAM_RESPONSE			= CMD_CSS_CONFIG_PARAM_RESPONSE,		//获取配置信息回复

		//获取录像任务状态
		E_CMD_RECORD_TASK_STATUS_REQUEST		= CMD_RECORD_TASK_STATUS_REQUEST,		//获取录像任务状态请求
		E_CMD_RECORD_TASK_STATUS_RESPONSE		= CMD_RECORD_TASK_STATUS_RESPONSE,		//获取录像任务状态回复

		//获取域映射表
		E_CMD_GET_DOMAIN_MAP_REQUEST			= CMD_GET_DOMAIN_MAP_REQUEST,			//获取域关系映射表请求
		E_CMD_GET_DOMAIN_MAP_RESPONSE			= CMD_GET_DOMAIN_MAP_RESPONSE,			//获取域关系映射表回复

		//播放列表
		E_CMD_SAVE_PLAY_LIST_REQUEST			= CMD_SAVE_PLAY_LIST_REQUEST,			//保存播放列表请求
		E_CMD_SAVE_PLAY_LIST_RESPONSE			= CMD_OPER_RESPONSE,					//保存播放列表回复
		E_CMD_GET_PLAY_LIST_REQUEST				= CMD_GET_PLAY_LIST_REQUEST,			//获取播放列表请求
		E_CMD_GET_PLAY_LIST_RESPONSE			= CMD_GET_PLAY_LIST_RESPONSE,			//获取播放列表回复

		//电视墙
		E_CMD_POLLING_OPER_REQUEST				= CMD_POLLING_OPER_REQUEST,				// 执行轮询
		E_CMD_POLLING_OPER_RESPONSE				= CMD_POLLING_OPER_RESPONSE,			// 执行轮询回复
		E_CMD_GET_MATRIX_LIST_REQUEST			= CMD_GET_MATRIX_LIST_REQUEST,			// 获取电视墙列表
		E_CMD_GET_MATRIX_LIST_RESPONSE			= CMD_GET_MATRIX_LIST_RESPONSE,			// 获取电视墙列表回复
		E_CMD_GET_MATRIX_DECODECARD_REQUEST		= CMD_GET_MATRIX_DECODECARD_REQUEST,	// 获取电视墙解码卡
		E_CMD_GET_MATRIX_DECODECARD_RESPONSE	= CMD_GET_MATRIX_DECODECARD_RESPONSE,	// 获取电视墙解码卡回复
		E_CMD_STREAM_MATRIX_REQUEST				= CMD_STREAM_MATRIX_REQUEST,			// 码流上墙
		E_CMD_STREAM_MATRIX_RESPONSE			= CMD_STREAM_MATRIX_RESPONSE,			// 码流上墙回复
		E_CMD_OPRATE_MATRIX_REQUEST				= CMD_OPRATE_MATRIX_REQUEST,			// 控制电视墙请求
		E_CMD_OPRATE_MATRIX_RESPONSE			= CMD_OPRATE_MATRIX_RESPONSE,			// 控制电视墙请求回复
		E_CMD_SYNNOTICE_MATRIX_REQUEST			= CMD_SYNNOTICE_MATRIX_REQUEST,			// 通知电视墙同步请求
		E_CMD_SYNNOTICE_MATRIX_RESPONSE			= CMD_SYNNOTICE_MATRIX_RESPONSE,		// 通知电视墙同步请求回复
		E_CMD_DATASYN_MATRIX_NOTICE				= CMD_DATASYN_MATRIX_NOTICE,			// 电视墙数据同步请求
		E_CMD_DATASYN_MATRIX_RESPONSE			= CMD_DATASYN_MATRIX_RESPONSE,			// 电视墙数据同步请求回复
		E_CMD_STATUSSYN_MATRIX_NOTICE			= CMD_STATUSSYN_MATRIX_NOTICE,			// 电视墙状态同步命令
		
		//电视墙预案控制
		E_CMD_MATRIX_PLAN_CONTROL_REQUEST		= CMD_MATRIX_PLAN_CONTROL_REQUEST,		// 电视墙预案控制请求
		E_CMD_MATRIX_PLAN_CONTROL_RESPONSE		= CMD_MATRIX_PLAN_CONTROL_RESPONSE,		// 电视墙预案控制请求回复
		//预案信息保存
		E_CMD_MATRIX_PLAN_ADD_REQUEST			= CMD_MATRIX_PLAN_ADD_REQUEST,          // 电视墙预案增加请求
		E_CMD_MATRIX_PLAN_ADD_RESPONSE          = CMD_MATRIX_PLAN_ADD_RESPONSE,         // 电视墙预案增加请求回复
		E_CMD_MARTIX_PLAN_NOTICE				= CMD_MATRIX_PLAN_NOTICE,			    // 预案信息的通知
		E_CMD_MATRIX_PLAN_GET_REQUEST			= CMD_MATRIX_PLAN_GET_REQUEST,			//获取电视墙预案请求
		E_CMD_MATRIX_PLAN_GET_RESPONSE			= CMD_MATRIX_PLAN_GET_RESPONSE,		    //电视墙预案请求回复
		E_CMD_MATRIX_PLAN_STATE_RESPONSE        = CMD_MATRIX_PLAN_STATE_RESPONSE,       //电视墙预案状态请求
		
		E_CMD_JOURNAL_QUERY_REQUEST				= CMD_JOURNAL_QUERY_REQUEST,			// 操作日志查询请求
		E_CMD_JOURNAL_QUERY_RESPONSE			= CMD_JOURNAL_QUERY_RESPONSE,			// 操作日志查询回复

		//远程参数设置
		E_CMD_SET_DEVICE_PARA_REQUEST			= CMD_SET_DEVICE_PARA_REQUEST,			// 前端设备参数设置请求
		E_CMD_SET_DEVICE_PARA_RESPONSE			= CMD_SET_DEVICE_PARA_RESPONSE,			// 前端设备参数设置回复
		E_CMD_GET_DEVICE_PARA_REQUEST			= CMD_GET_DEVICE_PARA_REQUEST,			// 前端设备参数获取请求
		E_CMD_GET_DEVICE_PARA_RESPONSE			= CMD_GET_DEVICE_PARA_RESPONSE,			// 前端设备参数获取回复
		E_CMD_SET_MANY_DEVICE_PARA_REQUEST		= CMD_SET_MANY_DEVICE_PARA_REQUEST,		// 批量前端设备参数设置请求
		E_CMD_SET_MANY_DEVICE_PARA_RESPONSE		= CMD_SET_MANY_DEVICE_PARA_RESPONSE,	// 批量前端设备参数设置回复
		E_CMD_GET_DECODE_DEVICE_PARA_REQUEST    = CMD_GET_DECODE_DEVICE_PARA_REQUEST,   // 解码器设备参数获取请求
		E_CMD_GET_DECODE_DEVICE_PARA_RESPONSE   = CMD_GET_DECODE_DEVICE_PARA_RESPONSE,  // 解码器设备参数获取回复

		//数据同步命令
		E_CMD_DATA_UPDATE_NOTICE				= CMD_DATA_UPDATE_NOTICE,				// 与配置中心交互的数据更新通知
		E_CMD_DATA_UPDATE_NOTICE_EX				= CMD_DATA_UPDATE_NOTICE_EX,			// 平台内与各服务之间的数据更新通知
		E_CMD_DATA_UPDATE_NOTICE_EX_RESPONSE	= CMD_DATA_UPDATE_NOTICE_EX_RESPONSE,	// PMS平台内与各服务之间的数据更新通知回复
		E_CMD_PMS_CLI_NOTICE					= CMD_PMS_CLI_NOTICE,					// PMS发给CLI的通知

		//手动录像命令定义
		E_CMD_MANUAL_RECORD_REQUEST =  CMD_MANUAL_RECORD_REQUEST,   //手动录像请求 
																	//命令体 StruManualRecordRequest
		E_CMD_MANUAL_RECORD_RESPONSE = CMD_MANUAL_RECORD_RESPONSE,  //手动录像请求回复 
			 													//命令体 StruManualRecordResponse

		//2100发送开关门信号
		E_CMD_ON_OFF_DOOR_SIGNAL				 = CMD_ON_OFF_DOOR_SIGNAL,				//2100发送开关门信号

		E_CMD_RECORD_FILE_AUTOBACKUP_RULE_REQUEST   =CMD_RECORD_FILE_AUTOBACKUP_RULE_REQUEST,     //录像自动备份规则请求
		E_CMD_RECORD_FILE_AUTOBACKUP_RULE_RESPONSE  =CMD_RECORD_FILE_AUTOBACKUP_RULE_RESPONSE,    //录像自动备份规则回复
		E_CMD_RECORD_FILE_MANUALBACKUP_OPER_REQUEST =CMD_RECORD_FILE_MANUALBACKUP_OPER_REQUEST,    //录像手动备份操作请求
		E_CMD_RECORD_FILE_MANUALBACKUP_OPER_RESPONSE=CMD_RECORD_FILE_MANUALBACKUP_OPER_RESPONSE,   //录像手动备份操作回复
		E_CMD_RECORD_FILE_BACKUP_RETRIEVAL_REQUEST  =CMD_RECORD_FILE_BACKUP_RETRIEVAL_REQUEST,     //录像备份检索请求
		E_CMD_RECORD_FILE_BACKUP_RETRIEVAL_RESPONSE =CMD_RECORD_FILE_BACKUP_RETRIEVAL_RESPONSE,    //录像备份检索回复

		E_CMD_RECORD_FILE_CONTROL_REQUEST			 =CMD_RECORD_FILE_CONTROL_REQUEST,                   //录像文件控制请求命令(录像文件加锁解锁命令)
		E_CMD_RECORD_FILE_CONTROL_RESPONSE			 =CMD_RECORD_FILE_CONTROL_RESPONSE,                  //录像文件控制回复命令

		E_CMD_GET_DEV_STORAGEPOLICY_REQUEST			 =CMD_GET_DEV_STORAGEPOLICY_REQUEST,				//获取设备存储策略登记表请求命令
		E_CMD_GET_DEV_STORAGEPOLICY_RESPONSE		 =CMD_GET_DEV_STORAGEPOLICY_RESPONSE,				//获取设备存储策略登记表回复命令

		E_CMD_GET_DEV_REPLACEPOLICY_REQUEST			 = CMD_GET_DEV_REPLACEPOLICY_REQUEST,			//获取设备替换策略登记表请求命令
		E_CMD_GET_DEV_REPLACEPOLICY_RESPONSE		 = CMD_GET_DEV_REPLACEPOLICY_RESPONSE,		//获取设备替换策略登记表回复命令

		//获取SLAVE 配置信息表
		E_CMD_GET_SLAVE_REGISTER_INFO_REQUEST = CMD_GET_SLAVE_REGISTER_INFO_REQUEST,  //方向  CSS -> PMS
		E_CMD_GET_SLAVE_REGISTER_INFO_RESPONSE = CMD_GET_SLAVE_REGISTER_INFO_RESPONSE, // 方向  PMS->CSS

		//获取磁盘 配置信息表
		E_CMD_GET_DISK_REGISTER_INFO_REQUEST = CMD_GET_DISK_REGISTER_INFO_REQUEST, //方向  CSS -> PMS
		E_CMD_GET_DISK_REGISTER_INFO_RESPONSE = CMD_GET_DISK_REGISTER_INFO_RESPONSE, // 方向  PMS->CSS
		
		//获取运行时SLAVE 与 磁盘的关系
		E_CMD_GET_SLAVE_DISK_RUNING_MAP_REQUEST = CMD_GET_SLAVE_DISK_RUNING_MAP_REQUEST, // 方向   HMC->PMS->CSS
		E_CMD_GET_SLAVE_DISK_RUNING_MAP_RESPONSE = CMD_GET_SLAVE_DISK_RUNING_MAP_RESPONSE, // 方向 CSS->PMS->HMC

		//摄像头状态统计
		E_CMD_DEV_ONLINE_QUERY					= CMD_DEV_ONLINE_QUERY,			// 摄像头状态统计请求
		E_CMD_DEV_ONLINE_QUERY_RESPONSE			= CMD_DEV_ONLINE_QUERY_RESPONSE, // 摄像头状态统计结果

		//摄像头告警状态统计结果请求
		E_CMD_DEV_ALARMSTATUS_STATISTICS		= CMD_DEV_ALARMSTATUS_STATISTICS,			// 摄像头告警状态统计结果请求
		E_CMD_DEV_ALARMSTATUS_STATISTICS_RESPONSE	 = CMD_DEV_ALARMSTATUS_STATISTICS_RESPONSE, // 摄像头告警状态统计结果
		//获取用户信息请求
		E_CMD_GET_USERINFO                     = CMD_GET_USERINFO,
		E_CMD_GET_USERINFO_RESPONSE            = CMD_GET_USERINFO_RESPONSE,
		
		//获取权限信息请求
		E_CMD_GET_RIGHTSINFO                   = CMD_GET_RIGHTSINFO,
		E_CMD_GET_RIGHTSINFO_RESPONSE          = CMD_GET_RIGHTSINFO_RESPONSE,
		
		//判断是否有某些权限
		E_CMD_JUDGE_RIGHTS                     = CMD_JUDGE_RIGHTS,
		E_CMD_JUDGE_RIGHTS_RESPONSE            = CMD_JUDGE_RIGHTS_RESPONSE,
		
		//获取用户角色请求
		E_CMD_GET_ROLEINFO                     = CMD_GET_ROLEINFO,
		E_CMD_GET_ROLEINFO_RESPONSE            = CMD_GET_ROLEINFO_RESPONSE,
		E_CMD_GET_TIMETEMPLATEINFO             = CMD_GET_TIMETEMPLATEINFO,
		E_CMD_GET_TIMETEMPLATEINFO_RESPONSE    = CMD_GET_TIMETEMPLATEINFO_RESPONSE,

		//级联数据
        E_CMD_CASCADE_NODE_INFO_REQUEST          = CMD_CASCADE_NODE_INFO_REQUEST,//级联数据获取请求
		E_CMD_CASCADE_NODE_INFO_RESPONSE          = CMD_CASCADE_NODE_INFO_RESPONSE,//级联数据获取回复

		//获取用户在线状态
		E_CMD_GET_USERONLINESTATUS_REQUEST     =  CMD_GET_USERONLINESTATUS_REQUEST,		
		E_CMD_GET_USERONLINESTATUS_RESPONSE     =  CMD_GET_USERONLINESTATUS_RESPONSE,		
		//强制用户下线请求
		E_CMD_FORCE_USEROFFLINE_REQUEST        = CMD_FORCE_USEROFFLINE_REQUEST,
		E_CMD_FORCE_USEROFFLINE_RESPONSE       = CMD_FORCE_USEROFFLINE_RESPONSE,
		
		//后去系统管理员信息.
		E_CMD_GET_SYSADMININFO_RREQUEST	= CMD_GET_SYSADMININFO_RREQUEST,
		E_CMD_GET_SYSADMININFO_RESPONSE		= CMD_GET_SYSADMININFO_RESPONSE,

		//获取键盘信息
		E_CMD_GET_KEBOARDINFO_REQUST = CMD_GET_KEBOARDINFO_REQUST,
		E_CMD_GET_KEBOARDINFO_RESPONSE = CMD_GET_KEBOARDINFO_RESPONSE,

		//获取设备通道信息
		E_CMD_GET_DEVCHNNINFO_REQUST = CMD_GET_DEVCHNNINFO_REQUST,
		E_CMD_GET_DEVCHNNINFO_RESPONSE = CMD_GET_DEVCHNNINFO_RESPONSE,
		//获取系统的版本号
		E_CMD_GET_VERSION_REQUEST	= CMD_GET_VERSION_REQUEST,
		E_CMD_GET_VERSION_RESPONSE = CMD_GET_VERSION_RESPONSE,
		
		//热点功能
		E_CMD_GET_HOT_SPOT_REQUEST = CMD_GET_HOT_SPOT_REQUEST,
		E_CMD_GET_HOT_SPOT_RESPONSE = CMD_GET_HOT_SPOT_RESPONSE,

		//级联的服务运行状态
		E_CMD_CASCADE_SERVICE_RUN_STATUS_REQUEST = CMD_CASCADE_SERVICE_RUN_STATUS_REQUEST,
		E_CMD_CASCADE_SERVICE_RUN_STATUS_RESPONSE = CMD_CASCADE_SERVICE_RUN_STATUS_RESPONSE,
		//级联的服务在线状态主动通知
		E_CMD_CASCADE_SERVICE_RUN_STATUS_NOTICE = CMD_CASCADE_SERVICE_RUN_STATUS_NOTICE,
		E_CMD_CASCADE_SERVICE_RUN_STATUS_NOTICE_RESPONSE = CMD_CASCADE_SERVICE_RUN_STATUS_NOTICE_RESPONSE,

		//请求连接下级平台的协议
		E_CMD_CASCADE_PLATFORM_CONNECT_REQUEST = CMD_CASCADE_PLATFORM_CONNECT_REQUEST,
		E_CMD_CASCADE_PLATFORM_CONNECT_RESPONSE = CMD_CASCADE_PLATFORM_CONNECT_RESPONSE,		
		
		//级联告警转发
		E_CMD_ALARM_SEND = CMD_ALARM_SEND,

		//级联平台GIS信息同步请求
		E_CMD_CASCADE_GIS_INFO_REQUEST = CMD_CASCADE_GIS_INFO_REQUEST,
		E_CMD_CASCADE_GIS_INFO_RESPONSE = CMD_CASCADE_GIS_INFO_RESPONSE,

		//BMS之间或者BMS->GIS的级联平台GIS信息同步请求
		E_CMD_BMS_CASCADE_GIS_INFO_REQUEST = CMD_BMS_CASCADE_GIS_INFO_REQUEST,
		E_CMD_BMS_CASCADE_GIS_INFO_RESPONSE = CMD_BMS_CASCADE_GIS_INFO_RESPONSE,

		//级联平台GIS信息主动同步通知
		E_CMD_CASCADE_GIS_INFO_NOTICE = CMD_CASCADE_GIS_INFO_NOTICE,
		E_CMD_CASCADE_GIS_INFO_NOTICE_RESPONSE = CMD_CASCADE_GIS_INFO_NOTICE_RESPONSE,		
		
		//级联平台回环检测
		E_CMD_CASCADE_LOOPBACK_CHECK_REQUEST = CMD_CASCADE_LOOPBACK_CHECK_REQUEST,
		E_CMD_CASCADE_LOOPBACK_CHECK_RESPONSE = CMD_CASCADE_LOOPBACK_CHECK_RESPONSE,

		//获取布局.
		E_CMD_GET_LAYOUT_REQUEST = CMD_GET_LAYOUT_REQUEST,
		E_CMD_GET_LAYOUT_RESPONSE = CMD_GET_LAYOUT_RESPONSE,

		//操作布局.
		E_CMD_OPER_LAYOUT_REQUEST = CMD_OPER_LAYOUT_REQUEST,
		E_CMD_OPER_LAYOUT_RESPONSE =  CMD_OPER_LAYOUT_RESPONSE,
		
		//增量更新.
		E_CMD_INCREMENT_UPDATE_NOTIFY = CMD_INCREMENT_UPDATE_NOTIFY,
		E_CMD_INCREMENT_UPDATE_NOTIFY_CLI = CMD_INCREMENT_UPDATE_NOTIFY_CLI,

		//管理员远程关闭视频
		E_CMD_GET_USER_ALL_OPEN_VIDEO_INFO_REQUEST=CMD_GET_USER_ALL_OPEN_VIDEO_INFO_REQUEST,
		E_CMD_GET_USER_ALL_OPEN_VIDEO_INFO_RESPONSE =CMD_GET_USER_ALL_OPEN_VIDEO_INFO_RESPONSE,
		E_CMD_OPER_USER_OPEN_VIDEO = CMD_OPER_USER_OPEN_VIDEO,
		E_CMD_FORCE_USER_CLOSE_VIDEO = CMD_FORCE_USER_CLOSE_VIDEO,
		E_CMD_CLOSE_VIDEO_WITH_ALL_USER = CMD_CLOSE_VIDEO_WITH_ALL_USER,
		E_CMD_CLOSE_VIDEO_RESPONSE = CMD_CLOSE_VIDEO_RESPONSE,
		E_CMD_CLOSE_VIDEO_RESPONSE_CLIENT=CMD_CLOSE_VIDEO_RESPONSE_CLIENT,
		E_CMD_GET_USERINFO_WITH_CHANNEL_REQUEST=CMD_GET_USERINFO_WITH_CHANNEL_REQUEST,
		E_CMD_GET_USERINFO_WITH_CHANNEL_RESPONSE=CMD_GET_USERINFO_WITH_CHANNEL_RESPONSE,
		E_CMD_SEND_USER_ALL_OPEN_VIDEO_AFTER_LOGIN=CMD_SEND_USER_ALL_OPEN_VIDEO_AFTER_LOGIN,

		//通道状态命令 
		E_CMD_CHN_STATUS_NOTICE = CMD_CHN_STATUS_NOTICE,
		E_CMD_CHN_STATUS_NOTICE_RESPONSE = CMD_CHN_STATUS_NOTICE_RESPONSE,
		E_CMD_GET_CHN_STATUS = CMD_GET_CHN_STATUS,
		
		//获取所有设备和通道状态.
		E_CMD_GET_ALL_DEV_CHN_STATUS_REQUEST	= CMD_GET_ALL_DEV_CHN_STATUS_REQUEST,
		E_CMD_GET_ALL_DEV_CHN_STATUS_RESPONSE = CMD_GET_ALL_DEV_CHN_STATUS_RESPONSE,

		//级联平台数据同步通知消息
		E_CMD_CASCADE_PLATFORM_DATA_SYN_NOTICE = CMD_CASCADE_PLATFORM_DATA_SYN_NOTICE,
		E_CMD_CASCADE_PLATFORM_DATA_SYN_REPONSE = CMD_CASCADE_PLATFORM_DATA_SYN_REPONSE,

		//级联平台关联关系通知
		E_CMD_CASCADE_PLATFORM_RELATION_NOTICE = CMD_CASCADE_PLATFORM_RELATION_NOTICE,
		E_CMD_CASCADE_PLATFORM_RELATION_REPONSE = CMD_CASCADE_PLATFORM_RELATION_REPONSE,

		//保存抓图记录日志
		E_CMD_SAVE_CAPTURE_REQUEST = CMD_SAVE_CAPTURE_REQUEST,
		E_CMD_SAVE_CAPTURE_RESPONSE = CMD_SAVE_CAPTURE_RESPONSE,

		//获取级联平台的在线用户
		E_CMD_CASCADE_GET_USERONLINESTATUS_REQUEST     =  CMD_CASCADE_GET_USERONLINESTATUS_REQUEST,		
		E_CMD_CASCADE_GET_USERONLINESTATUS_RESPONSE     =  CMD_CASCADE_GET_USERONLINESTATUS_RESPONSE,		
		//BMS之间的通讯协议。获取级联平台的在线用户
		E_CMD_BMS_CASCADE_GET_USERONLINESTATUS_REQUEST     =  CMD_BMS_CASCADE_GET_USERONLINESTATUS_REQUEST,		
		E_CMD_BMS_CASCADE_GET_USERONLINESTATUS_RESPONSE     =  CMD_BMS_CASCADE_GET_USERONLINESTATUS_RESPONSE,
		//管理员远程强制关闭用户视频,获取用户所有视频信息
		E_CMD_CASCADE_GET_USER_ALL_OPEN_VIDEO_INFO_REQUEST     =  CMD_CASCADE_GET_USER_ALL_OPEN_VIDEO_INFO_REQUEST,		
		E_CMD_CASCADE_GET_USER_ALL_OPEN_VIDEO_INFO_RESPONSE     =  CMD_CASCADE_GET_USER_ALL_OPEN_VIDEO_INFO_RESPONSE,

		//设备定时重启
		E_CMD_DEV_REBOOT_RULE_REQUEST	=	CMD_DEV_REBOOT_RULE_REQUEST,	// 设备定时重启规则请求
		E_CMD_DEV_REBOOT_RULE_RESPONSE	=	CMD_DEV_REBOOT_RULE_RESPONSE	// 设备定时重启规则回复

}EnumCmdIDInfo;


}

#ifdef _WIN32
#pragma pack(pop)
#endif

#endif // CMD_PROTOCOL_DEF_DEF_H
