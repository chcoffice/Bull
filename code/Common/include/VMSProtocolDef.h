#ifndef VMSPROTOCOLDEF_DEF_H
#define VMSPROTOCOLDEF_DEF_H
/**************************************************************************************************
  Copyright (C), 2010-2011, GOSUN 
  File name 	: VMSPROTOCOLDEF.H      
  Author 		: Liujs      
  Version 		: Vx.xx        
  DateTime 		: 2010/11/15 9:22
  Description 	: VMS 内部使用的通信协议，也就是：Master和Slaver通信用的命令协议
				  里面的命名可能会和CMD_PROTOCOL_DEF_DEF_H的相同，使用的时候，
				  要求带上命名空间CmdVMSProtocolDef
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
namespace	CmdVMSProtocolDef
{
	/**************************************************************************************************
		CMD_ID		: EnumClientType
		CMD_NAME	: 服务，设备单元类型定义
		DateTime	: 2010/11/15 9:33	
		Description	: 服务，设备单元类型定义
		Author		: Liujs
		Note		: CLIENT_TYPE_MASTER，CLIENT_TYPE_SLAVER为VMS内部使用，其他与CMD_PROTOCOL_DEF.H中定义相同
	**************************************************************************************************/
	typedef		enum	EnumClientType
	{
		CLIENT_TYPE_PMS = 101,							// PMS
		CLIENT_TYPE_DAS	,								// DAS
		CLIENT_TYPE_STS	,								// STS
		CLIENT_TYPE_LMS ,								// LMS
		CLIENT_TYPE_AMS ,								// AMS
		CLIENT_TYPE_VMS ,								// VMS
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
		OPER_RESULT_FAIL				,					// 操作失败
		OPER_UNKNOW_ERROR				,					// 未知错误

		//--------------------------------------------------------------------
		// 登陆返回结果
		LOG_RESULT_USER_NAME_ERROR		,					// 用户名不存在	
		LOG_RESULT_PWD_ERROR			,					// 密码错	
		LOG_RESULT_HAS_EXIST			,					// 登录ID已存在	
		LOG_RESULT_SERVICE_LOG_FULL		,					// 服务器容量达到极限

		//--------------------------------------------------------------------
		// 点流返回结果
		GET_STREAM_NO_STREAM			,					// 流不存在
		GET_STREAM_NO_DEVICE			,					// 设备不存在，或者不可以使用
		GET_STREAM_NO_CHANNEL			,					// 设备对应通道不存在
		GET_STREAM_TIME_OUT									// 点流超时
		
		

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
		char        szClientIP[MAX_IP_ADDR_LEN];            // 本地客户端IP
		INT32       iClientPort;                            // 本地客户端端口
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

	}EnumAlarmType;



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
	CMD_ID			: CMD_CHNLMATCH_NOTICE_RESPONSE  
	CMD_NAME		: 通道关联信息通知命令
	DateTime		: 2011/7/29 14:14	
	Author 			: chc      
	Description		: 通道关联信息通知命令
	Note			: 
	**************************************************************************************************/
	#define		SLV_CMD_CHNLMATCH_NOTICE_RESPONSE			111			// 通道关联信息通知命令
	typedef struct StruCmdChnl
	{
		INT32	iLogicalChnl;					// 逻辑通道
		INT32	iPhysicsChnl;					// 物理通道

	}StruCmdChnl,*StruCmdChnlPtr;

	typedef struct StruCmdChnlInfo
	{
		INT32	iNum;							// 通道数目
		StruCmdChnl	stCmdchnl[1];				// 通道结构,变长

	}StruCmdChnlInfo,*StruCmdChnlInfoPtr;

	/**************************************************************************************************
	CMD_ID		: 
	CMD_NAME	: 
	DateTime	: 2011/5/12 9:00	
	Description	: 码流上墙请求命令
	Author		: chc
	Note		: 客户端指定某一路码流上墙时的指令
	**************************************************************************************************/
	#define		SLV_CMD_STREAM_MATRIX_REQUEST			115
	typedef struct StruDeviceLoginInfo
	{
		CHAR  szIP[MAX_IP_ADDR_LEN];
		INT32 iPort;
		CHAR  szUserName[MAX_NAME_LEN];
		CHAR  szPwd[MAX_PWD_LEN];
		CHAR  szDevCodeID[MAX_NAME_LEN_256];
	}StruDeviceLoginInfo,*StruDeviceLoginInfoPtr;
	typedef struct StruDeviceInfoRequest
	{
		INT32 iPlatformID;				// 平台ID
		INT32 iDevID;					// 设备ID
		INT32 iChnID;					// 通道ID
		INT32 iLogicalOutChnlID;		// 逻辑输出通道ID
		INT32 iStreamType;				// 0：实时流 1：录像流
		CHAR  szURI[MAX_URI_LEN];		// 如果是录像上墙 则必须填URI
		StruDeviceLoginInfo stDevLoginInfo;	//设备登陆信息
	}StruDeviceInfoRequest,*StruDeviceInfoRequestPtr;

	/**************************************************************************************************
	CMD_ID		: 
	CMD_NAME	: 
	DateTime	: 2011/5/12 9:00	
	Description	: 码流上墙回复命令
	Author		: chc
	Note		: 
	**************************************************************************************************/
	#define		SLV_CMD_STREAM_MATRIX_RESPONSE			116
	typedef struct StruDeviceInfoResponse
	{
		INT32 iOperResult;				// 回复结果,枚举EnumErrorCode定义
		StruDeviceInfoRequest	stDeviceInfo;	// 设备信息 
	}StruDeviceInfoResponse,*StruDeviceInfoResponsePtr;

	/**************************************************************************************************
	CMD_ID		: 
	CMD_NAME	: 
	DateTime	: 2011/5/12 9:00	
	Description	: 停止码流上墙命令
	Author		: chc
	Note		: 客户端指定停止某一路码流上墙时的指令
	**************************************************************************************************/
#define		SLV_CMD_STOP_MATRIX_REQUEST			117
	// 注意：参数SLV_CMD_STREAM_MATRIX_REQUEST参数相同

	/**************************************************************************************************
	CMD_ID		: 
	CMD_NAME	: 
	DateTime	: 2011/5/12 9:00	
	Description	: 停止码流上墙回复命令
	Author		: chc
	Note		: 客户端指定停止某一路码流上墙时的回复指令
	**************************************************************************************************/
#define		SLV_CMD_STOP_MATRIX_RESPONSE			118
	// 注意：参数SLV_CMD_STREAM_MATRIX_RESPONSE命令相同

	/**************************************************************************************************
	CMD_ID		: 
	CMD_NAME	: 
	DateTime	: 2011/12/12 9:00	
	Description	: 请求解码卡信息命令
	Author		: chc
	Note		: 
	**************************************************************************************************/
#define		SLV_CMD_GET_DECODECARD_REQUEST			119
	// 注意:此命令无参数


	/**************************************************************************************************
	CMD_ID		: 
	CMD_NAME	: 
	DateTime	: 2011/12/12 9:00	
	Description	: 解码卡信息请求回复命令
	Author		: chc
	Note		: 
	**************************************************************************************************/
#define		SLV_CMD_GET_DECODECARD_RESPONSE			120
	typedef struct StruDecodecardInfo
	{
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
	CMD_ID		: 
	CMD_NAME	: 
	DateTime	: 2011/12/12 9:00	
	Description	: 请求分屏和通道映射命令
	Author		: chc
	Note		: 
	**************************************************************************************************/
#define		SLV_CMD_GET_SPLITCHNL_REQUEST			121
	// 注意:此命令无参数


	/**************************************************************************************************
	CMD_ID		: 
	CMD_NAME	: 
	DateTime	: 2011/12/12 9:00	
	Description	: 分屏和通道映射请求回复命令
	Author		: chc
	Note		: 
	**************************************************************************************************/
#define		SLV_CMD_GET_SPLITCHNL_RESPONSE			122
	typedef struct StruChnlConfigResponse
	{
		INT32				iResult;						// 结果
		INT32				iLen;							// 长度
		char				pContent[1];					// 内容
	}StruChnlConfigResponse,*StruChnlConfigResponsePtr;

	/**************************************************************************************************
	CMD_ID		: CMD_STATUSSYN_MATRIX_NOTICE_VMS
	CMD_NAME	: CMD_STATUSSYN_MATRIX_NOTICE_VMS
	DateTime	: 2012/07/26 11:00		
	Description	: 电视墙状态同步通知。slave发给master
	Author		: CHC
	Note		: 此命令不需要应答
	**************************************************************************************************/
	#define CMD_STATUSSYN_MATRIX_NOTICE_VMS			123
	typedef	enum EnumStatusMatrixType
	{
		enum_DecoderOnLineType,			//解码器
		enum_CurMatrixStreamType,		// 当前在墙上的码流信息。数据结构和SLV_CMD_STREAM_MATRIX_REQUEST完全相同
	}EnumStatusMatrixType;

	//enum_DecoderOnLineType类型的数据结构
	typedef		struct		StruDecoderOnLineResponse
	{
		CHAR	szName[MAX_NAME_LEN];	//解码器名称
		CHAR	szDecoderIP[MAX_IP_ADDR_LEN];	//解码器IP
		CHAR	szSalveName[MAX_NAME_LEN];	//所属slave
		INT32	iOnLine;	//详见EnumDevStatusType
		StruDecoderOnLineResponse()
		{
			memset(szName, 0x0, MAX_NAME_LEN);
			memset(szDecoderIP, 0x0, MAX_IP_ADDR_LEN);
			memset(szSalveName, 0x0, MAX_NAME_LEN);
		};
	}StruDecoderOnLineResponse,*StruDecoderOnLineResponsePtr;
	
	//enum_CurMatrixStreamType类型的数据结构和SLV_CMD_STREAM_MATRIX_REQUEST完全相同

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

#define SLV_CMD_GET_DECODE_DEVICE_PARA_REQUEST 301
#define SLV_CMD_GET_DECODE_DEVICE_PARA_RESPONSE 302

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
	CMD_ID		: CMD_DATA_UPDATE_NOTICE_EX_RESPONSE
	CMD_NAME	: CMD_DATA_UPDATE_NOTICE_EX_RESPONSE
	DateTime	: 2012/07/17 10:48	
	Description	: master与slave之间的数据更新通知
	Author		: chc
	Note		: 不需要回复
	**************************************************************************************************/
	#define		CMD_DATA_UPDATE_NOTICE_VMS			905

	typedef	enum
	{
		// 登陆 
		E_CMD_LOGIN_REQUEST					=	CMD_LOGIN_REQUEST,				// 登陆命令
		E_CMD_LOGIN_RESPONSE				=	CMD_LOGIN_RESPONSE,				// 登陆应答命令

		// 点流
		E_CMD_GET_STREAM_REQUEST			=	CMD_GET_STREAM_REQUEST,			// 点流命令
		E_CMD_GET_STREAM_RESPONSE			=	CMD_GET_STREAM_RESPONSE,		// 点流应答命令

		// 任务处理
		E_SLV_CMD_STREAM_MATRIX_REQUEST		=	SLV_CMD_STREAM_MATRIX_REQUEST,		// 码流上墙
		E_SLV_CMD_STREAM_MATRIX_RESPONSE	=	SLV_CMD_STREAM_MATRIX_RESPONSE,			// 码流上墙回复
		E_SLV_CMD_STOP_MATRIX_REQUEST		=	SLV_CMD_STOP_MATRIX_REQUEST,		// 停止码流上墙
		E_SLV_CMD_STOP_MATRIX_RESPONSE		=	SLV_CMD_STOP_MATRIX_RESPONSE,			// 停止码流上墙回复
		
		E_SLV_CMD_CHNLMATCH_NOTICE_RESPONSE   = SLV_CMD_CHNLMATCH_NOTICE_RESPONSE,		//通道映射通知.不再使用此命令chc2011.12.12
		E_SLV_CMD_GET_DECODECARD_REQUEST   = SLV_CMD_GET_DECODECARD_REQUEST,		//获取解码卡信息
		E_SLV_CMD_GET_DECODECARD_RESPONSE   = SLV_CMD_GET_DECODECARD_RESPONSE,		//获取解码卡信息回复
		E_SLV_CMD_GET_SPLITCHNL_REQUEST   = SLV_CMD_GET_SPLITCHNL_REQUEST,			//获取通道映射信息
		E_SLV_CMD_GET_SPLITCHNL_RESPONSE   = SLV_CMD_GET_SPLITCHNL_RESPONSE,		//获取通道映射信息回复

		E_SLV_CMD_GET_DECODE_DEVICE_PARA_REQUEST    = SLV_CMD_GET_DECODE_DEVICE_PARA_REQUEST,   // 解码器设备参数获取请求
		E_SLV_CMD_GET_DECODE_DEVICE_PARA_RESPONSE   = SLV_CMD_GET_DECODE_DEVICE_PARA_RESPONSE,  // 解码器设备参数获取回复

		E_CMD_STATUSSYN_MATRIX_NOTICE_VMS	= CMD_STATUSSYN_MATRIX_NOTICE_VMS,		//状态同步命令

		E_CMD_TIME_SYNCHRONIZATION			=	CMD_TIME_SYNCHRONIZATION,		// 时间同步命令

		E_CMD_DATA_UPDATE_NOTICE_VMS		= CMD_DATA_UPDATE_NOTICE_VMS,	// master与slave服务之间的数据更新通知回复

		E_CMD_ALARM_NOTICE					=	CMD_ALARM_NOTICE,				// 告警通知

	}EnumCmdType;

}


#ifdef _WIN32
#pragma		pack(pop)
#endif


#endif 
// VMSPROTOCOLDEF_DEF_H
