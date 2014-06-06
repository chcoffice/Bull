#ifndef CSSPROTOCOLDEF_DEF_H
#define CSSPROTOCOLDEF_DEF_H
/**************************************************************************************************
  Copyright (C), 2010-2011, GOSUN 
  File name 	: CSSPROTOCOLDEF.H      
  Author 		: Liujs      
  Version 		: Vx.xx        
  DateTime 		: 2010/11/15 9:22
  Description 	: CSS 内部使用的通信协议，也就是：Master和Slaver通信用的命令协议
				  里面的命名可能会和CMD_PROTOCOL_DEF_DEF_H的相同，使用的时候，
				  要求带上命名空间CmdCssProtocolDef
**************************************************************************************************/

// 字节对齐大小
#define		PRAGMA_PACK_SIZE		1
#ifdef _WIN32
#define		ATTRIBUTE_PACKED 
#pragma		pack(push,PRAGMA_PACK_SIZE )
#else
#define ATTRIBUTE_PACKED    __attribute__ ((packed)) 
#endif

// 头文件，使用里面的长度的定义的宏，目的就是怕拷贝长度不一样的时候，造成长度不一致溢出
// 别的部分都不用
#include "CMD_PROTOCOL_DEF.H"

#define		DELETE_FILE_NUM					256										// 删除文件个数

// 定义自己的命名空间
namespace	CmdCssProtocolDef
{
	// 录像任务文件类型
	typedef		enum		EnumRecordTaskFileType
	{
		T_F_T_VIDEO	=	1,		//	视频录像，
		T_F_T_PICTURE ,			//	图片，
		T_F_T_AUDIO				//	音频录像
	}EnumRecordTaskFileType;

	// 任务状态信息
	typedef		enum		EnumRecordTaskStatus
	{
		INIT=0,			// 0(INIT): 初始化， Master启动时设置， 等待Slave确认
		WAIT,			// 1(WAIT): Master 发送任务，等待Slave完成
		DOING,			// 2(DOING): 正常工作
		ASSERT,			// 3(ASSERT): 断流
		SLAVEROFFLINE,	// 4（SLAVEOFFLINE）:Slave 下线
		DISKERR,		// 5（DISKERR）: 磁盘失效
		COMPLETE,		// 6 (COMPLETE) : 正常完成
		EXCEPTION       // 7 (EXCEPTION) : 异常
	}EnumRecordTaskStatus;

	/**************************************************************************************************
		CMD_ID		: EnumClientType
		CMD_NAME	: 服务，设备单元类型定义
		DateTime	: 2010/11/15 9:33	
		Description	: 服务，设备单元类型定义
		Author		: Liujs
		Note		: CLIENT_TYPE_MASTER，CLIENT_TYPE_SLAVER为CSS内部使用，其他与CMD_PROTOCOL_DEF.H中定义相同
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
		CLIENT_TYPE_USER,								// USER
		CLIENT_TYPE_MASTER,								// MASTER
		CLIENT_TYPE_SLAVER								// SLAVER
	}EnumClientType;

	/**************************************************************************************************
		CMD_ID		: EnumErrorCode
		CMD_NAME	: 错误返回代码
		DateTime	: 2010/11/15 9:45	
		Description	: 错误返回代码
		Author		: Liujs
		Note		: 在Master和Slaver中自己定义错误代码
	**************************************************************************************************/
	typedef		enum	EnumErrorCode
	{
		//--------------------------------------------------------------------
		// 通用操作成功，失败
		OPER_RESULT_SUCCESS				= 0,				// 操作成功
		OPER_RESULT_FAIL				= CmdProtocolDef::OPER_RESULT_FAIL,					// 操作失败
		OPER_UNKNOW_ERROR				= CmdProtocolDef::OPER_UNKNOW_ERROR,					// 未知错误

		//--------------------------------------------------------------------
		// 登陆返回结果
		LOG_RESULT_USER_NAME_ERROR		= CmdProtocolDef::LOG_RESULT_USER_NAME_ERROR,					// 用户名不存在	
		LOG_RESULT_PWD_ERROR			= CmdProtocolDef::LOG_RESULT_PWD_ERROR,					// 密码错	
		LOG_RESULT_HAS_EXIST			= CmdProtocolDef::LOG_RESULT_HAS_EXIST,					// 登录ID已存在	
		LOG_RESULT_SERVICE_LOG_FULL		= CmdProtocolDef::LOG_RESULT_SERVICE_LOG_FULL,					// 服务器容量达到极限

		//--------------------------------------------------------------------
		// 点流返回结果
		GET_STREAM_NO_STREAM			= CmdProtocolDef::GET_STREAM_NO_STREAM,					// 流不存在
		GET_STREAM_NO_DEVICE			= CmdProtocolDef::GET_STREAM_NO_DEVICE,					// 设备不存在，或者不可以使用
		GET_STREAM_NO_CHANNEL			= CmdProtocolDef::GET_STREAM_NO_CHANNEL,					// 设备对应通道不存在
		GET_STREAM_TIME_OUT				= CmdProtocolDef::GET_STREAM_TIME_OUT,					// 点流超时
		
		//--------------------------------------------------------------------
		// 开始录像
		START_RECORD_ERROR				= -11,					// 开始录像失败

		//--------------------------------------------------------------------
		// 停止录像
		STOP_RECORD_ERROR				= -20,					// 停止录像失败
		
		//--------------------------------------------------------------------
		// 改变存储位置
		CHANGE_STORAGE_ERROR			= -30,					// 改变存储录像位置失败

		//--------------------------------------------------------------------
		// 删除文件
		DELETE_FILE_ERROR				= -40,					// 删除文件失败
		DELETE_FILE_NO_FILE				= -41,					// 找不到要删除的文件

		//--------------------------------------------------------------------
		// 查询磁盘信息
		QUERY_DISK_INFO_ERROR			= -50,					// 查询磁盘信息失败

	

		//--------------------------------------------------------------------
		// 获取数据版本结果

		//--------------------------------------------------------------------
		// 告警相关代码

	}EnumErrorReturnCode;
	
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
		//--------------------------------------------------------------------
		// 登陆返回结果
		{	LOG_RESULT_USER_NAME_ERROR,								"用户名不存在"},
		{	LOG_RESULT_PWD_ERROR,									"密码错误"},
		{	LOG_RESULT_HAS_EXIST,									"登录ID已存在"},
		{	LOG_RESULT_SERVICE_LOG_FULL,							"服务器容量达到极限"},
		//--------------------------------------------------------------------
		// 点流返回结果
		{	GET_STREAM_NO_STREAM,									"流不存在"},
		{	GET_STREAM_NO_DEVICE,									"设备不存在，或者不可以使用"},
		{	GET_STREAM_NO_CHANNEL,									"设备对应通道不存在"},
		{	GET_STREAM_TIME_OUT,									"点流超时"},
		//--------------------------------------------------------------------
		// 开始录像
		{	START_RECORD_ERROR,										"开始录像失败"},
		//--------------------------------------------------------------------
		// 停止录像
		{	STOP_RECORD_ERROR,										"停止录像失败"},
		//--------------------------------------------------------------------
		// 改变存储位置
		{	CHANGE_STORAGE_ERROR,									"改变存储录像位置失败"},
		//--------------------------------------------------------------------
		// 删除文件
		{	DELETE_FILE_ERROR,										"删除文件失败"},
		{	DELETE_FILE_NO_FILE,									"找不到要删除的文件"},
		//--------------------------------------------------------------------
		// 查询磁盘信息
		{	QUERY_DISK_INFO_ERROR,									"查询磁盘信息失败"}

		//--------------------------------------------------------------------
		// 继续添加
	};

	
	/**************************************************************************************************
	CMD_ID			: CMD_LOGIN    
	CMD_NAME		: 登陆PMS    
	DateTime		: 2010/7/29 10:39	
	Author 			: Liujs      
	Description		: 登陆命令
	Note			: 客户端登陆PMS命令
	**************************************************************************************************/
	#define			CMD_LOGIN_REQUEST			1			// 平台内客户端（登陆PMS）登陆命令

		// 空的登陆ID,填写为0
	#define			EMPTY_LOGIN_ID				0

	// 登陆命令数据结构体
	typedef		struct		StruCmdLoginInfo 
	{
		INT32		iLoginID;								// 登陆ID，如果没有就填写 EMPTY_LOGIN_ID
		INT32		iLoginType;								// 登陆类型	参考EnumClientType中定义
		char		szUserName[MAX_NAME_LEN];				// 登陆名称
		char		szPassword[MAX_NAME_LEN];				// 登陆密码
		char		szVersion[MAX_VERSION_LEN];				// 版本号，CLI登录填0，其他服务登录填服务本身的版本号
	}StruCmdLoginInfo,*StruCmdLoginInfoPtr;	



	/**************************************************************************************************
	CMD_ID			: CMD_LOGIN_RESPONSE 
	CMD_NAME		: 登陆应答命令       
	DateTime		: 2010/7/29 11:03	
	Author 			: Liujs      
	Description		: 登陆命令回复
	Note			: 
	**************************************************************************************************/
	#define		CMD_LOGIN_RESPONSE				2			// 登陆命令回复

	// 登陆命令回复结构体
	typedef		struct		StruCmdLoginResponse 
	{
		INT32		iLogResult;								// 登陆结果，参考EnumErrorCode中定义
		INT32		iMasterID;								// MasterID,通信的时候用到
		INT32		iLoginID;								// 登陆返回ID
	}StruCmdLoginResponse,*StruCmdLoginResponsePtr;

	/**************************************************************************************************
	CMD_ID			: CMD_SLAVE_CAPABILITY_REQUEST 
	CMD_NAME		: CMD_SLAVE_CAPABILITY_REQUEST       
	DateTime		: 2012/9/5 15:26	
	Author 			: Jiangsx      
	Description		: 从服务能力获取请求
	Note			: 
	**************************************************************************************************/
	#define CMD_SLAVE_CAPABILITY_REQUEST  11

	/**************************************************************************************************
	CMD_ID			: CMD_SLAVE_CAPABILITY_RESPONSE 
	CMD_NAME		: CMD_SLAVE_CAPABILITY_RESPONSE       
	DateTime		: 2012/9/5 15:26	
	Author 			: Jiangsx      
	Description		: 从服务能力获取回复
	Note			: 
	**************************************************************************************************/
	#define CMD_SLAVE_CAPABILITY_RESPONSE  12
	
	// 从服务能力结构定义
	typedef struct StruSlaveCapability
	{
		INT32 iMaxRecordTasks;              // 最大录像任务数（包括计划录像、告警录像、手动录像等）
	}StruCapability, *StruCapabilityPtr;


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
		NO_ALARM							= CmdProtocolDef::NO_ALARM,									// 没有告警

		//--------------------------------------------------------------------------------------------------------------------
		// 设备 告警
	/*	ALARM_IO_START						= CmdProtocolDef::A_T_ALARM_IO_START,						// IO告警开始
		ALARM_IO_END						= CmdProtocolDef::A_T_ALARM_IO_END,							// IO告警结束

		ALARM_MD_START						= CmdProtocolDef::A_T_ALARM_MD_START,						// 移动侦测告警开始
		ALARM_MD_END						= CmdProtocolDef::A_T_ALARM_MD_END,							// 移动侦测告警结束

		ALARM_VIDEO_COVER_START				= CmdProtocolDef::A_T_ALARM_VIDEO_COVER_START,				// 视频遮挡告警开始
		ALARM_VIDEO_COVER_END				= CmdProtocolDef::A_T_ALARM_VIDEO_COVER_END,				// 视频遮挡告警结束

		ALARM_VIDEO_MISSING_START			= CmdProtocolDef::A_T_ALARM_VIDEO_MISSING_START,			// 视频丢失告警开始
		ALARM_VIDEO_MISSING_END				= CmdProtocolDef::A_T_ALARM_VIDEO_MISSING_END,				// 视频丢失告警结束

		ALARM_VIDEO_SIGNAL_ABNORMAL_START	= CmdProtocolDef::A_T_ALARM_VIDEO_SIGNAL_ABNORMAL_START,	// 视频信号异常告警开始
		ALARM_VIDEO_SIGNAL_ABNORMAL_END		= CmdProtocolDef::A_T_ALARM_VIDEO_SIGNAL_ABNORMAL_END,		// 视频信号异常告警结束

		ALARM_DISK_DAMAGE					= CmdProtocolDef::A_T_ALARM_DISK_DAMAGE,					// 设备磁盘损坏告警
		ALARM_FLASH_FAULT					= CmdProtocolDef::A_T_ALARM_FLASH_FAULT,					// 设备FLASH故障告警
		ALARM_DISK_FULL						= CmdProtocolDef::A_T_ALARM_DISK_FULL,						// 设备磁盘满告警

		//--------------------------------------------------------------------------------------------------------------------
		// CLI 告警
		ALARM_MANUAL_START					= CmdProtocolDef::A_T_ALARM_MANUAL_START,					// CLI手动告警开始
		ALARM_MANUAL_END					= CmdProtocolDef::A_T_ALARM_MANUAL_END,						// CLI手动告警结束

		//--------------------------------------------------------------------------------------------------------------------
		// CSS 告警
		ALARM_STORAGE_FULL					= CmdProtocolDef::A_T_ALARM_STORAGE_FULL,					// CSS存储已满告警
		ALARM_RW_DISK_ERROR					= CmdProtocolDef::A_T_ALARM_RW_DISK_ERROR					// CSS读写磁盘出错告警
*/
	}EnumAlarmType;

	/**************************************************************************************************
		CMD_ID		: CMD_START_RECORD_REQUEST
		CMD_NAME	: 开始录像命令
		DateTime	: 2010/11/15 16:45	
		Description	: 开始录像命令
		Author		: Liujs
		Note		: NULL
	**************************************************************************************************/
	#define		CMD_START_RECORD_REQUEST				50			

	// 文件名称类型
	typedef		enum	EnumFileNameType
	{
		F_N_T_BY_ID,						// 文件名称类型为：ID
		F_N_T_BY_NAME						// 文件名称类型为：名称
	}EnumFileNameType;

	// 录像任务类型
	typedef		enum	EnumRecordTastType
	{
		R_T_T_PLAN			=	0,			// 计划录像任务
		R_T_T_ALARM,						// 告警录像任务
		R_T_T_DOWNLOAD,                      // 下载录像任务
		R_T_T_MANUAL,						//手动录像
	}EnumRecordTastType;

	// 开始录像命令结构
	typedef		struct	StruCmdStartRecordRequest
	{
		UINT64		iCtrlIndex;							// 录像控制唯一索引
		INT32		iPlatformID;						// 录像平台ID
		INT32		iDeviceID;							// 录像设备ID
		INT32		iChannelID;							// 录像通道ID
		char		szDevName[MAX_NAME_LEN];			// 录像设备名称
		char		szChnName[MAX_NAME_LEN];			// 录像通道名称
		INT32		iRecordTaskType;					// 录像任务类型，参考EnumRecordTastType结构
		INT32       iStreamType;                        // 参考平台码流类型定义 CmdProtocolDef::EnumStreamType

		INT32		iAlarmType;							// 告警类型参考EnumAlarmType，如果是计划录像则为：NO_ALARM
		INT32		iAlarmPlatformID;					// 告警平台ID
		INT32		iAlarmDeviceID;						// 告警设备ID
		INT32		iAlarmChannelID;					// 告警通道ID
        char		szAlarmDevName[MAX_NAME_LEN];		// 告警设备名称
		char		szAlarmChnName[MAX_NAME_LEN];		// 告警通道名称
		CmdProtocolDef::StruDateTime	stAlarmStartDateTime;	// 告警时间开始时间
        UINT64      iAlarmIndex;                        // 告警索引

		INT32		iUsedName;							// 文件名使用名称信息，参考结构EnumFileNameType
		INT32		iMinLenTM;							// 最短录像时间，单位：分钟
		INT32		iMaxLenTM;							// 最长录像时间  -1 表示无限长，单位：分钟
		INT32		iFileLenTM;							// 每个文件的录像时间长度，单位：分钟
		char		szStorageUUID[MAX_NAME_LEN];		// 录像的储存器，磁盘ID 

		CmdProtocolDef::StruDateTime	stStartTime;	// 开始时间
		CmdProtocolDef::StruDateTime	stEndTime;	    // 结束时间

		INT32    bFileIndex; //文件索引是否使用
		UINT64   iFileIndex;  //文件索引
		INT32	 iStorageGroupID; //存储的组ID
       
	}StruCmdStartRecordRequest,*StruCmdStartRecordRequestPtr;


	/**************************************************************************************************
		CMD_ID		: CMD_START_RECORD_RESPONSE
		CMD_NAME	: 开始录像回复
		DateTime	: 2010/11/15 17:56	
		Description	: 开始录像回复，在SLAVER开始录像了以后，回复MASTER的命令
		Author		: Liujs
		Note		: NULL
	**************************************************************************************************/
	#define		CMD_START_RECORD_RESPONSE				51			

	// 开始录像回复命令结构体
	typedef		struct	StruCmdStartRecordResponse 
	{
		INT32		iOperResult;						// 操作结果，参考EnumErrorCode
		UINT64		iCtrlIndex;							// 录像控制唯一索引	
	}StruCmdStartRecordResponse,*StruCmdStartRecordResponsePtr;


	/**************************************************************************************************
		CMD_ID		: CMD_STOP_RECORD_REQUEST
		CMD_NAME	: 录像停止命令
		DateTime	: 2010/11/15 18:02	
		Description	: 录像停止命令
		Author		: Liujs
		Note		: NULL
	**************************************************************************************************/
	#define		CMD_STOP_RECORD_REQUEST					52

	// 停止录像请求
	typedef		struct StruCmdStopRecordRequest
	{
		UINT64		iCtrlIndex;							// 录像控制唯一索引
	}StruCmdStopRecordRequest,*StruCmdStopRecordRequestPtr;


	/**************************************************************************************************
		CMD_ID		: CMD_STOP_RECORD_RESPONSE
		CMD_NAME	: 停止录像回复
		DateTime	: 2010/11/15 18:44	
		Description	: 录像停止命令回复
		Author		: Liujs
		Note		: NULL
	**************************************************************************************************/
	#define		CMD_STOP_RECORD_RESPONSE					53

	// 停止录像回复命令
	typedef		struct	StruCmdStopRecordResponse 
	{
		INT32		iOperResult;						// 操作结果，参考EnumErrorCode
		UINT64		iCtrlIndex;							// 录像控制唯一索引
	}StruCmdStopRecordResponse,*StruCmdStopRecordResponsePtr;


	/**************************************************************************************************
		CMD_ID		: CMD_CHANGE_STORAGE_REQUEST
		CMD_NAME	: 改变存储位置请求
		DateTime	: 2010/11/15 18:02	
		Description	: 录像磁盘迁移命令，在新旧磁盘在同一SLAVE下进行
		Author		: Liujs
		Note		: NULL
	**************************************************************************************************/
	#define		CMD_CHANGE_STORAGE_REQUEST					54

	// 改变存储位置请求命令结构
	typedef		struct StruCmdChangeStorageRequest
	{
		UINT64		iCtrlIndex;							// 录像控制唯一索引
		char		szStorageUUID[MAX_NAME_LEN];		// 磁盘uuid标志
	}StruCmdChangeStorageRequest,*StruCmdChangeStorageRequestPtr;


	/**************************************************************************************************
		CMD_ID		: CMD_CHANGE_STORAGE_RESPONSE
		CMD_NAME	: 改变存储位置命令回复
		DateTime	: 2010/11/15 18:52	
		Description	: 改变存储位置命令回复
		Author		: Liujs
		Note		: NULL
	**************************************************************************************************/
	#define		CMD_CHANGE_STORAGE_RESPONSE					55

	// 改变存储命令请求回复命令，对应数据结构
	typedef		struct	StruCmdChangeStorageResponse 
	{
		INT32		iOperResult;						// 操作结果，参考EnumErrorCode
		UINT64		iCtrlIndex;							// 录像控制唯一索引
	}StruCmdChangeStorageResponse,*StruCmdChangeStorageResponsePtr;

	
	/**************************************************************************************************
		CMD_ID		: CMD_DELETE_FILE_REQUEST
		CMD_NAME	: 删除录像文件命令
		DateTime	: 2010/11/15 18:02	
		Description	: 删除录像文件命令请求
		Author		: Liujs
		Note		: NULL
	**************************************************************************************************/
	#define		CMD_DELETE_FILE_REQUEST						56

	//删除录像文件对应结构,数据为空
	
	/**************************************************************************************************
		CMD_ID		: CMD_DELETE_FILE_RESPONSE
		CMD_NAME	: 删除文件回复命令
		DateTime	: 2010/11/15 19:01	
		Description	: 删除文件回复命令
		Author		: Liujs
		Note		: NULL
	**************************************************************************************************/
	#define		CMD_DELETE_FILE_RESPONSE					57

	// 删除文件对应回复命令为空

	/**************************************************************************************************
	CMD_ID			: CMD_GET_STREAM_REQUEST   
	CMD_NAME		: 发起点流
	DateTime		: 2010/7/29 11:50	
	Author 			: Liujs      
	Description		: 命令描述
	Note			: 备注
	**************************************************************************************************/
	#define		CMD_GET_STREAM_REQUEST			101			// 点流发起命令

	// 发起点流的命令数据
	typedef		struct		StruCmdGetStream
	{
		INT32		iPlatformID;							// 平台ID
		INT32		iDeviceID;								// 设备ID
		INT32		iChannelNum;							// 通道号
		INT32		iStreamType;							// 码流类型，保留
	}StruCmdGetStream,*StruCmdGetStreamPtr;


	/**************************************************************************************************
	CMD_ID			: CMD_GET_STREAM_RESPONSE  
	CMD_NAME		: 点流结果返回命令
	DateTime		: 2010/7/29 14:14	
	Author 			: Liujs      
	Description		: 点流命令返回结果信息
	Note			: 
	**************************************************************************************************/
	#define		CMD_GET_STREAM_RESPONSE			102			// 点流返回结果命令

	// 点流返回结果命令数据
	typedef		struct		StruCmdGetStreamResponse 
	{
		INT32		iGetStreamResult;						// 点流返回结果，参考EnumErrorCode	
		INT32		iPlatformID;							// 平台ID
		INT32		iDeviceID;								// 设备ID
		INT32		iChannelNum;							// 点流通道信息
		char		szStreamURI[MAX_URI_LEN];				// 点流返回URI资源信息，最大长度为512字符,256中文字符
	}StruCmdGetStreamResponse,*StruCmdGetStreamResponsePtr;

	/**************************************************************************************************
		CMD_ID		: CMD_RESOURCES_INFO_QUERY_REQUEST
		CMD_NAME	: 资源信息查询请求命令
		DateTime	: 2010/11/15 18:03	
		Description	: 资源信息查询请求命令
		Author		: Liujs
		Note		: NULL
	**************************************************************************************************/
	#define		CMD_RESOURCES_INFO_QUERY_REQUEST					200

	// 资源信息查询基础命令结构,无需命令数据

	/**************************************************************************************************
		CMD_ID		: CMD_RESOURCES_INFO_QUERY_RESPONSE
		CMD_NAME	: 资源信息查询回复命令
		DateTime	: 2010/11/15 19:25	
		Description	: 资源信息查询回复命令
		Author		: Liujs
		Note		: NULL
	**************************************************************************************************/
	#define		CMD_RESOURCES_INFO_QUERY_RESPONSE				201

	// 磁盘可以使用标志
	typedef		enum	EnumDiskUseFlag
	{
		D_U_F_NO			= 0	,			// 不可以使用
		D_U_F_YES							// 可以使用
	}EnumDiskUseFlag;

	// 磁盘类型枚举
	typedef	enum	EnumDiskLocationType
	{
		DISK_LOCAL			=	1,					// 本地磁盘
		DISK_NAS,									// NAS
		DISK_BACKUP                                 // 备份盘
	}EnumDiskLocationType;

	// 磁盘信息查询命令结构信息回复命令
	typedef		struct StruDiskQueryResponseItem
	{
		INT32		iOperResult;							// 操作结果，参考EnumErrorCode
		char		szStorageUUID[MAX_NAME_LEN];			// 磁盘UUID唯一标志
		UINT64		iTotalSize;								// 磁盘空间总大小，单位：MB
		UINT64		iTotalFree;								// 磁盘剩余空间大小,单位：MB
		INT16		iUseRate;								// 可以使用百分比[0-100]	
		INT16		iCanUseFlag;							// 是否可以使用标志，参考	EnumDiskUseFlag
		INT16       iDiskType;                              // 磁盘类型，参考  EnumDiskLocationType
		char        szDisk[MAX_DISK_PARTITION_LEN+32];      // 磁盘标识(如 "c:/"或者"//hame//...")
	}StruDiskQueryResponseItem,*StruDiskQueryResponseItemPtr;

	// 资源信息查询回复结构
	typedef		struct StruCmdResourcesQueryResponse 
	{
		UINT32                          iWorkingSetSize;                           // 内存    (单位:MB)
		UINT32                          iPagefileUsage;                            // 虚拟内存(单位:MB)
		UINT32							iCpuUsage;								   // CPU使用率
		INT32						    iDiskNum;						           // 节点个数
		StruDiskQueryResponseItem	    stDiskQueryResponseItem[1];		           // 磁盘信息查询结构
	}StruCmdResourcesQueryResponse,*StruCmdResourcesQueryResponsePtr;


	/**************************************************************************************************
		CMD_ID		: CMD_TASK_SYNCHRONIZATION
		CMD_NAME	: CMD_TASK_SYNCHRONIZATION
		DateTime	: 2010/12/3 10:43	
		Description	: 录像任务信息同步
		Author		: Liujs
		Note		: NULL
	**************************************************************************************************/
	#define		CMD_TASK_SYNCHRONIZATION					250

	// 命令无需要数据
	
	/**************************************************************************************************
		CMD_ID		: CMD_TIME_SYNCHRONIZATION
		CMD_NAME	: CMD_TIME_SYNCHRONIZATION
		DateTime	: 2010/12/3 10:48	
		Description	: 时间同步命令
		Author		: Liujs
		Note		: NULL
	**************************************************************************************************/
	#define		CMD_TIME_SYNCHRONIZATION					280
	
	// 时间同步数据
	typedef	struct	StruCmdSynchronizationTime 
	{
		CmdProtocolDef::StruDateTime		stDateTime;							// 时间信息
	}StruCmdSynchronizationTime,*StruCmdSynchronizationTimePtr;


	/**************************************************************************************************
	CMD_ID		: CMD_ALARM_NOTICE
	CMD_NAME	: CMD_ALARM_NOTICE
	DateTime	: 2010/12/8 16:19	
	Description	: 磁盘告警通知
	Author		: Liujs
	Note		: NULL
	**************************************************************************************************/
	#define		CMD_ALARM_NOTICE							300

	// 告警上送命令结构
	typedef		struct	StruCmdAlarmNotice 
	{
		INT32				iAlarmType;							// 告警类型,参考EnumAlarmType
		char				szStorageUUID[MAX_NAME_LEN];		// 磁盘UUID唯一标志
	}StruCmdAlarmNotice,*StruCmdAlarmNoticePtr;

	/**************************************************************************************************
	CMD_ID		: CMD_RECORD_FILE_OPERATE_REQUEST
	CMD_NAME	: CMD_RECORD_FILE_OPERATE_REQUEST
	DateTime	: 2011/02/23 16:19	
	Description	: 录像回放下载请求
	Author		: CHC
	Note		: NULL
	**************************************************************************************************/
	#define		CMD_RECORD_FILE_DOWN_REPLAY_REQUEST				350
	#define     CMD_RECORD_FILE_DOWN_REPLAY_REQUEST_EX          352

	//录像文件操作类型枚举结构
	typedef		enum	EnumRecordOperType
	{
		RECORD_FILE_REPLAY,		//文件回放
		RECORD_FILE_DOWNLOAD,	//文件下载
		RECORD_TIME_REPLAY,		// 录像时间回放
		RECORD_TIME_DOWNLOAD	// 录像时间下载
	};

	// 录像文件信息结构
	typedef struct StruRecFileInfo
	{
		char  strFileTime[MAX_ID_STRING_LEN];		// 录像文件时间： 开始时间和结束时间（如 2011-01-01-00-00-00,2011-01-01-01-00-00）
		char  szRcdFileName[MAX_NAME_LEN_256];		// 录像文件名称
		char  szStorageUUID[64];					// 录像的储存器，磁盘ID 
	} StruRecFileInfo, *StruRecFileInfoPtr;

	//录像文件操作请求命令结构
	typedef struct StruRecordFileOperRequest
	{
		INT32 iOperType;							// 操作类型：见枚举EnumRecordOperType，表示对录像文件进行的操作类型，0	表示文件回放；1	表示文件下载；2 表示时间回放；3 表示时间下载
		INT32 iVersion;								// 主要是用于兼容不同存储介质录像文件ID不同表示方法，暂时填0
        
        INT32 iCliPlatformID;						// 发送请求命令的客户端的平台ID  **在回复中带回
        INT32 iCliUserID;							//  发送请求命令的设备ID    **在回复中带回
		char  szIPAddr[MAX_IP_ADDR_LEN];			// 客户端IP地址
        
        INT32 iPlatformID;							// 平台ID
		INT32 iDevID;								// 设备ID
		INT32 iChnID;								// 通道ID

		INT16 iStorageType;							// 存储类型，参考枚举类型EnumStorageType   **在回复中带回
		INT16 iReserve;								// 保留

		char  strRcdFileID[MAX_ID_STRING_LEN];		// 文件：录像文件ID  时间： 开始时间和结束时间（如 2011-01-01-00-00-00,2011-01-01-01-00-00） **在回复中带回

		INT16 iFileNum;                             // 文件个数
		StruRecFileInfo stFileInfo[1] ;             // 录像文件信息结构

	}StruRecordFileOperRequest,*StruRecordFileOperRequestPtr;

	/**************************************************************************************************
	CMD_ID		: CMD_RECORD_FILE_OPERATE_RESPONSE
	CMD_NAME	: CMD_RECORD_FILE_OPERATE_RESPONSE
	DateTime	: 2011/02/23 16:19	
	Description	: 录像回放下载回复
	Author		: CHC
	Note		: NULL
	**************************************************************************************************/
	#define		CMD_RECORD_FILE_DOWN_REPLAY_RESPONSE			351
	#define		CMD_RECORD_FILE_DOWN_REPLAY_RESPONSE_EX			353

	//录像文件操作请求回复命令结构
	typedef struct StruRecordFileOperResponse
	{
		INT32 iOperResult;					    // 回复结果,枚举EnumErrorCode定义
		INT32 iOperType;						// 操作类型：见枚举EnumRecordOperType，表示对录像文件进行的操作类型，0	表示文件回放；1	表示文件下载；2 表示时间回放；3 表示时间下载
		INT32 iVersion;							// 主要是用于兼容不同存储介质录像文件ID不同表示方法，暂时填0
        
        INT32 iCliPlatformID;						// 发送请求命令的客户端的平台ID
        INT32 iCliUserID;							//  发送请求命令的设备ID

        char  strRcdFileID[MAX_ID_STRING_LEN];	// 录像文件ID	
		char  szRecFileURI[MAX_URI_LEN];		// 录像文件资源地址
		INT16 iStorageType;						// 存储类型，参考枚举类型EnumStorageType
		INT16 iReserve;							// 保留
	}StruRecordFileOperResponse,*StruRecordFileOperResponsePtr;


	/**************************************************************************************************
	CMD_ID		: CMD_BACKUP_FILE_SEARCH_REQUEST
	CMD_NAME	: CMD_BACKUP_FILE_SEARCH_REQUEST
	DateTime	: 2012/03/01 14:36	
	Description	: 备份文件检索请求
	Author		: Jiangsx
	Note		: 
	**************************************************************************************************/
    #define CMD_BACKUP_FILE_SEARCH_REQUEST     381


	// 文件状态信息枚举
	typedef enum EnumFileStatusMark
	{
		INIT_STATE     = 0x00,          // 初始状态
		LOCKED         = 0x01,          // 已锁定
		NEED_BACKUP    = 0x02,          // 需要备份
		ALREADY_BACKUP = 0x04           // 已备份

		// to add: 若添加该状态，需要同步修改Master和Slave用到的SQL语句
	};

	// 备份文件检索请求结构
	typedef CmdProtocolDef::StruRecordFileRetrievalRequest StruBackupFileSearchReuqest;


	/**************************************************************************************************
	CMD_ID		: CMD_BACKUP_FILE_SEARCH_RESPONSE
	CMD_NAME	: CMD_BACKUP_FILE_SEARCH_RESPONSE
	DateTime	: 2012/03/01 14:36	
	Description	: 备份文件检索回复
	Author		: Jiangsx
	Note		: 
	**************************************************************************************************/
	 #define CMD_BACKUP_FILE_SEARCH_RESPONSE     382

	// 备份文件检索回复结构
	typedef CmdProtocolDef::StruRecordFileRetrievalResponse StruBackupFileSearchResponse;


	/**************************************************************************************************
	CMD_ID		: CMD_FILE_BACKUP_OPER_REQUEST
	CMD_NAME	: CMD_FILE_BACKUP_OPER_REQUEST
	DateTime	: 2012/03/01 14:36	
	Description	: 文件备份操作请求
	Author		: Jiangsx
	Note		: 
	**************************************************************************************************/
	 #define CMD_FILE_BACKUP_OPER_REQUEST     383

	// 文件备份操作类型枚举
	 typedef enum EnumFileBackupOperType
	 {
		BF_BACKUP = 0,       // 备份
		BF_RESUME,           // 恢复
		BF_DELETE            // 删除
	 };

	// 存储信息结构定义
	typedef struct StruStorageInfo 
	{
		INT8  bCover;                               // 是否覆盖
		char  szRcdFileID[MAX_ID_STRING_LEN];	    // 文件ID	
		char  szStorageUUID[64];					// 文件的储存器，磁盘ID 
	} StruStorageInfo, *StruStorageInfoPtr;
	
	// 文件备份操作请求结构
	typedef struct StruFileBackupOperRequest
	{
		INT32 iOperType;                     // 操作类型，参考枚举EnumFileBackupOperType
		INT32 iNum;                          // 存储信息个数
		StruStorageInfo stStorageInfo[1];    // 参考结构StruStorageInfo
	} StruFileBackupOperRequest, *StruFileBackupOperRequestPtr;


	/**************************************************************************************************
	CMD_ID		: CMD_FILE_BACKUP_OPER_RESPONSE
	CMD_NAME	: CMD_FILE_BACKUP_OPER_RESPONSE
	DateTime	: 2012/03/01 14:36	
	Description	: 文件备份操作回复
	Author		: Jiangsx
	Note		: 
	**************************************************************************************************/
	 #define CMD_FILE_BACKUP_OPER_RESPONSE     384
	
	typedef struct StruFileBackupOperResponse 
	{
		INT32 iOperType;    // 操作类型，参考枚举EnumFileBackupOperType
		INT32 iResult;      // 回复结果,枚举EnumErrorCode定义
	} StruFileBackupOperResponse, *StruFileBackupOperResponsePtr;




	/*
	录像任务命令， 包括 PTN， 手动录像， 告警录像
	*/
	


	//录像任务命令请求命令结构
	 #define CMD_RECORD_TASK_REQUEST     385
	typedef struct StruRecordTaskRequest
	{
		INT64 iTaskID;							    //任务ID
		INT32 iTaskType;							// 任务类型 参考 EnumRecordTastType
		INT32 iCliPlatformID;						// 发送请求命令的客户端的平台ID  **在回复中带回
		INT32 iCliUserID;							//  发送请求命令的设备ID    **在回复中带回
		INT32 iCmdSN;								//命令序号,**在回复中带回
		INT32 iSessionID;							//命令序号,**在回复中带回
		
		INT32 iPlatformID;							// 平台ID
		INT32 iDevID;								// 设备ID
		INT32 iChnID;								// 通道ID

		INT32 iRcdTimeLen;							//录像时间长度， 单位 分钟, 手动录像，和告警录像有效
		//参数长度
		INT32 iArgStrLen;							

		unsigned char szArg[1];  //参数
		/*
		 为PTN 下载时为：   开始时间;结束时间  yyyy-mm-ss hh24:mi:ss;yyyy-mm-ss hh24:mi:ss
		 为告警录像时 表示告警参数  : KeyName$Value&KeyName$Value 
		KeyName :
		AlType 告警类型; AlPmsId 告警平台ID; AlDevId 告警设备ID; AlChn 告警通道ID;
		AlIdx  告警索引; AlDevType 告警设备类型;AlChnType 告警通道类型;
		AlLevel 告警级别; AlStart 告警开始时间(yyyy-mm-ss hh24:mi:ss);
		AlDevName 告警设备名称; AlChnName 告警通道名称
		*/

	}StruRecordTaskRequest,*StruRecordTaskRequestPtr;


	//录像任务命令请求回复结构
	 #define CMD_RECORD_TASK_RESPONSE     386
	typedef struct StruRecordTaskResponse
	{
		INT32 iOperResult;					    // 回复结果,枚举EnumErrorCode定义
		INT32 iTaskType;						// 任务类型 参考 EnumRecordTastType
		INT64 iTaskID;							 //任务ID		

		//录像的平台通道
		INT32 iPlatformID;							// 平台ID
		INT32 iDevID;								// 设备ID
		INT32 iChnID;								// 通道ID


		INT32 iCmdSN;							//请求的命令序号
		INT32 iCliPlatformID;					// 发送请求命令的客户端的平台ID
		INT32 iCliUserID;						//  发送请求命令的设备ID
		INT32 iSessionID;						//请求的命令序号	
		char  strRcdFileID[MAX_ID_STRING_LEN];	//录像的文件ID， 手动录像需要返回
	}StruRecordTaskResponse,*StruRecordTaskResponsePtr;


	//停止录像任务请求命令结构
	#define CMD_STOP_RECORD_TASK_REQUEST    387
	typedef struct StruStopRecordTaskRequest
	{
		INT64 iTaskID;					    //录像任务的ID
		INT32 iCmdSN;							//请求的命令序号
		INT32 iCliPlatformID;					// 发送请求命令的客户端的平台ID
		INT32 iCliUserID;						//  发送请求命令的设备ID
		INT32 iSessionID;						//请求的命令序号
	}StruStopRecordTaskRequest,*StruStopRecordTaskRequestPtr;


	//停止录像任务请求回复命令结构
	#define CMD_STOP_RECORD_TASK_RESPONSE    388
	typedef struct StruStopRecordTaskResponse
	{
		INT64 iTaskID;					    //录像任务的ID
		INT32 iResult;							//结果
		INT32 iCmdSN;							//请求的命令序号
		INT32 iCliPlatformID;					// 发送请求命令的客户端的平台ID
		INT32 iCliUserID;						//  发送请求命令的设备ID
		INT32 iSessionID;						//请求的命令序号
	}StruStopRecordTaskResponse,*StruStopRecordTaskResponsePtr;


	//盘满告警
	#define CMD_DISK_FULL_ALARM    389
	typedef struct _StruDiskFullAlarm
	{		
		//存储策略ID
		INT64 iRcdStoragePly;
		//存储策略名
		unsigned char czPlyName[256];
	}StruDiskFullAlarm;



	typedef	enum
	{
		// 登陆 
		E_CMD_LOGIN_REQUEST					=	CMD_LOGIN_REQUEST,				// 登陆命令
		E_CMD_LOGIN_RESPONSE				=	CMD_LOGIN_RESPONSE,				// 登陆应答命令

		E_CMD_SLAVE_CAPABILITY_REQUEST      =   CMD_SLAVE_CAPABILITY_REQUEST,   // 从服务能力获取请求
		E_CMD_SLAVE_CAPABILITY_RESPONSE     =   CMD_SLAVE_CAPABILITY_RESPONSE,  // 从服务能力获取回复

		// 点流
		E_CMD_GET_STREAM_REQUEST			=	CMD_GET_STREAM_REQUEST,			// 点流命令
		E_CMD_GET_STREAM_RESPONSE			=	CMD_GET_STREAM_RESPONSE,		// 点流应答命令

		// 任务处理
		E_CMD_START_RECORD_REQUEST			=	CMD_START_RECORD_REQUEST,		// 录像请求命令
		E_CMD_START_RECORD_RESPONSE			=	CMD_START_RECORD_RESPONSE,		// 录像请求回复

		E_CMD_STOP_RECORD_REQUEST			=	CMD_STOP_RECORD_REQUEST,		// 停止录像
		E_CMD_STOP_RECORD_RESPONSE			=	CMD_STOP_RECORD_RESPONSE,		// 停止录像回复
		
		E_CMD_CHANGE_STORAGE_REQUEST		=	CMD_CHANGE_STORAGE_REQUEST,		// 改变存储位置
		E_CMD_CHANGE_STORAGE_RESPONSE		=	CMD_CHANGE_STORAGE_RESPONSE,	// 改变存储位置

		E_CMD_DELETE_FILE_REQUEST			=	CMD_DELETE_FILE_REQUEST,		// 删除文件请求
		E_CMD_DELETE_FILE_RESPONSE			=	CMD_DELETE_FILE_RESPONSE,		// 删除文件请求回复

		E_CMD_RESOURCES_INFO_QUERY_REQUEST		=	CMD_RESOURCES_INFO_QUERY_REQUEST,	// 资源信息查询请求
		E_CMD_RESOURCES_INFO_QUERY_RESPONSE		=	CMD_RESOURCES_INFO_QUERY_RESPONSE,	// 资源信息查询回复		

		E_CMD_TASK_SYNCHRONIZATION			=	CMD_TASK_SYNCHRONIZATION,		// 录像任务信息同步
	
		E_CMD_TIME_SYNCHRONIZATION			=	CMD_TIME_SYNCHRONIZATION,		// 时间同步命令

		E_CMD_ALARM_NOTICE					=	CMD_ALARM_NOTICE,				// 告警通知

		E_CMD_RECORD_FILE_DOWN_REPLAY_REQUEST	    =	CMD_RECORD_FILE_DOWN_REPLAY_REQUEST,	// 录像回放下载请求
		E_CMD_RECORD_FILE_DOWN_REPLAY_RESPONSE	    =	CMD_RECORD_FILE_DOWN_REPLAY_RESPONSE,	// 录像回放下载回复
		E_CMD_RECORD_FILE_DOWN_REPLAY_REQUEST_EX	=	CMD_RECORD_FILE_DOWN_REPLAY_REQUEST_EX,	// 录像回放下载请求
		E_CMD_RECORD_FILE_DOWN_REPLAY_RESPONSE_EX	=	CMD_RECORD_FILE_DOWN_REPLAY_RESPONSE_EX,// 录像回放下载回复

		E_CMD_BACKUP_FILE_SEARCH_REQUEST  = CMD_BACKUP_FILE_SEARCH_REQUEST,    // 备份文件检索
		E_CMD_BACKUP_FILE_SEARCH_RESPONSE = CMD_BACKUP_FILE_SEARCH_RESPONSE,   // 备份文件检索回复
		E_CMD_FILE_BACKUP_OPER_REQUEST    = CMD_FILE_BACKUP_OPER_REQUEST,      // 文件备份操作
		E_CMD_FILE_BACKUP_OPER_RESPONSE   = CMD_FILE_BACKUP_OPER_RESPONSE,     // 文件备份操作回复


		E_CMD_RECORD_TASK_REQUEST = CMD_RECORD_TASK_REQUEST, //录像任务， 除计划外的录像任务
		E_CMD_RECORD_TASK_RESPONSE = CMD_RECORD_TASK_RESPONSE, //录像任务， 除计划外的录像任务
		E_CMD_STOP_RECORD_TASK_REQUEST = CMD_STOP_RECORD_TASK_REQUEST, //停止录像任务
		E_CMD_STOP_RECORD_TASK_RESPONSE = CMD_STOP_RECORD_TASK_RESPONSE,
		E_CMD_DISK_FULL_ALARM = CMD_DISK_FULL_ALARM,

	}EnumCmdType;

}


#ifdef _WIN32
#pragma		pack(pop)
#endif

//数据版本
//设备信息
#define CSS_DATAVERKEY_DEVINFO   "aDevInfo"
//设备状态
#define CSS_DATAVERKEY_DEVSTATUS   "aDevStatu"
//计划录像计划
#define CSS_DATAVERKEY_PLANRCDPOLICY   "aPlanPly"
//存储规则
#define CSS_DATAVERKEY_STORAGEPOLICY   "aStgPly"

//磁盘配置信息
#define CSS_DATAVERKEY_DISKCONFIG "aDiskCfg"

//Slaver 配置信息
#define CSS_DATAVERKEY_SLAVERCONFIG "aSlaveCfg"
//强求同步数据
#define CSS_DATAVERKEY_FORCE_SYNCDATA "aFSync"

#endif 
// CSSPROTOCOLDEF_DEF_H
