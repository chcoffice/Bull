/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : SIPSTACK.H
Author :  邹阳星
Version : 1.0.0.1
Date: 2012/9/7 8:44
Description: SIP 协议栈接口定义
********************************************
*/

#ifndef _GS_H_SIPSTACK_H_
#define _GS_H_SIPSTACK_H_

#define SIP_DLL_API 


//SIP 会话 控制句柄定义
typedef void * SipSessionHandle;


typedef void * SipServiceHandle;


typedef void * SipDialogHandle;

#define SIP_INVALID_HANDLE NULL


//协议栈返回值错误码
typedef enum
{	
	eSIP_RET_SUCCESS = 0,
	eSIP_RET_FAILURE = -1,
	eSIP_RET_INIT_FAILURE					=	-11,  //初始化失败
	eSIP_RET_LISTEN_ADDR_FAILURE			=	-12,  //增加监听者失败
	eSIP_RET_BUILD_REGISTER_FAILURE			=	-13,  //生成注册数据失败
	eSIP_RET_SEND_REGISTER_FAILURE			=	-14,  //发送注册失败
	eSIP_RET_REGISTER_ACK_FAILURE			=	-15,  //注册对端返回失败
	eSIP_RET_REGISTER_ACK_TIMEOUT			=	-16,  //注册等待返回超时
	eSIP_RET_BUILD_INVITE_FAILURE			=	-17,  //生成 INVITE 域数据失败
	eSIP_RET_SEND_INVITE_FAILURE			=	-18,  //发送 INVITE 失败
	eSIP_RET_NOT_FIND_INVITESESSION			=	-19,  //
	eSIP_RET_BUILD_INFO_FAILURE				=	-20,
	eSIP_RET_SEND_INFO_FAILURE				=	-21,
	eSIP_RET_BUILD_NOTIFY_FAILURE			=	-22,
	eSIP_RET_SEND_NOTIFY_FAILURE			=	-23,
	eSIP_RET_BUILD_SUBSCRIBE_FAILURE		=	-24,
	eSIP_RET_SEND_SUBSCRIBE_FAILURE			=	-25,
	eSIP_RET_BUILD_DEFAULT_MSG_FAILURE		=	-26,
	eSIP_RET_SEND_DEFAULT_MSG_FAILURE		=	-27,
	eSIP_RET_ADD_AUTHENTICATION_FAILURE		=	-28,
	eSIP_RET_BUILD_ACK_FAILURE = -29,

	eSIP_RET_E_STATUS = -100, //错误状态
	eSIP_RET_E_NMEN = -101, //没有内存
	eSIP_RET_E_SNNEXIST = -102, //session 不存在
	eSIP_RET_E_TIMEOUT = -103, //超时
	eSIP_RET_E_INVALID = -104, //无效参数
	eSIP_RET_E_NFUNC = -105, //该功能没实现

	eSIP_RET_OSIP_E_NINIT = -1000, //OSIP 没有初始化
	eSIP_RET_OSIP_E_OPER = -10000, //OSIP 函数返回失败
	

} EnumSipErrorCode;


//SIP 协议的方式
typedef enum
{
	eSIP_METHOD_INVALID = 0,

	eSIP_METHOD_REGISTER,

	eSIP_METHOD_INVITE,
	eSIP_METHOD_INFO,
	eSIP_METHOD_NOTIFY,
	eSIP_METHOD_SUBSCRIBE,
	eSIP_METHOD_BYE,
	eSIP_METHOD_CANCEL,

	eSIP_METHOD_INFOEX,
	eSIP_METHOD_NOTIFYEX,

	eSIP_METHOD_MESSAGE,

	eSIP_METHOD_ACK,

	eSIP_METHOD_RESPONSE,

}EnumSipMethod;


//SIP Content 类型
typedef enum
{	
	eSIP_CONTENT_NONE = 0, //没有内容
	eSIP_CONTENT_SDP = 1,           //sdp 点流命令
	eSIP_CONTENT_MANSCDP_XML = 2, // manscdp_xml 控制命令
	eSIP_CONTENT_INNER = 3, //内部使用	
	eSIP_CONTENT_UNKNOWN = 4,
	eSIP_CONTENT_MANSRTSP = 5, // mansrtsp
	eSIP_CONTENT_DATE = 6, //日期时间   yyyy-mm-dd hh:mm:ss
	eSIP_CONTENT_RTSP = 7, //RTSP
}EnumSipContentType;


//SIP 连接方式
typedef enum
{
	eSIP_CONNECT_UDP = 0,
	eSIP_CONNECT_TCP = 1
}EnumSipConnectType;


#define SIP_MAX_IP_LEN 32
//连接信息
typedef struct _StruSipConnnectInfo
{
	EnumSipConnectType eConnectType;
	int bOnline;
	int iLocalPort;
	int iRemotePort;
	char szLocalIp[SIP_MAX_IP_LEN+1];
	char szRemoteIp[SIP_MAX_IP_LEN+1];
}StruSipConnnectInfo;

//SIP 协议回复码定义
#define SIP_RESPONSE_CODE_SUCCESS  200  //成功

#define SIP_RESPONSE_CODE_FAILURE		400  // 
 // 错误请求， 包含了错误语法
#define SIP_RESPONSE_CODE_BAD_REQUEST   400 
// 未鉴权
#define SIP_RESPONSE_CODE_UNAUTHORIZED	401 
// 禁止操作，可以识别语法， 拒绝完成
#define SIP_RESPONSE_CODE_FORBIDDEN	   403 
// 没有找到, 用户没有找到
#define SIP_RESPONSE_CODE_NOTFOUND	   404
// 方法不允许
#define SIP_RESPONSE_CODE_NOTALLOWED	   405
// 请求超时
#define SIP_RESPONSE_CODE_REQTIMEOUT	   408
// 不支持的媒体类型, content 类型
#define SIP_RESPONSE_CODE_BADMEDIA		415
// 系统忙
#define SIP_RESPONSE_CODE_BUSY		486
// 服务器错误
#define SIP_RESPONSE_CODE_ESRV		500
// 未实行接口
#define SIP_RESPONSE_CODE_UNIMP		501

//SIP 回复的基本结果
#define SIP_MAX_RESPONSE_ERROR_LEN 128
typedef struct _StruSipResponse
{
	bool bOk;
	int  iSipErrno;
	char szMessage[SIP_MAX_RESPONSE_ERROR_LEN];
}StruSipResponse;



#define SIP_MAX_CONTENT_LEN  (32*1024)



//SIP 数据包的类型
typedef enum
{
	eSIP_DATA_REQUEST = 0,
	eSIP_DATA_RESPONSE = 1
}EnumSipDataType;

#define SIP_MAX_SUBJECT_STRING 127

#define SIP_MAX_DIGID_STRING 128
#define SIP_MAX_TAG_STRING   128
#define SIP_MAX_VIAHOST_STIRNG 128

typedef struct _StruSipDialogKey
{
	long iCnnId; //连接ID
	int iCallId;   //通信的CALL ID
	int iDialogId; //对话ID
	int iTransId; // id for transactions (to be used for answers)	
	int iCSeq;	
	char szContactScheme[8];
	char szContactHost[128]; 
	char szContackUsername[128];
	int  iContactPort;  // host:port	
	int iExpirse;
	long iRcvTime; //接受到命令的时间
	char szSubject[SIP_MAX_SUBJECT_STRING+1];
	char szFrom[256];
	char szTo[256];

	char czDialogKey[SIP_MAX_DIGID_STRING+1];
	char czFromTag[SIP_MAX_TAG_STRING+1];
	char czToTag[SIP_MAX_TAG_STRING+1];
	char czViaHost[SIP_MAX_VIAHOST_STIRNG+1];
}StruSipDialogKey;



typedef struct _StruSipData
{
	StruSipDialogKey stDialog;
	EnumSipDataType eDataType;
	EnumSipMethod eMethod;	
	EnumSipContentType eContentType;
	int iContentLength;
	char vContent[SIP_MAX_CONTENT_LEN+1];
	StruSipResponse stResponseResult;
}StruSipData;

//关键字定义

#define SIP_STRKEY_USERNAME   "UserName"   //用户名
#define SIP_STRKEY_RESULT     "Result"     //操作结果

#define SIP_STRKEY_CMDTYPE                         "CmdType"        //命令类型
#define SIP_STRKEY_QUERY                           "Query"          //查询
#define SIP_STRKEY_SN                              "SN"             //命令序号
#define SIP_STRKEY_DEVICEID                        "DeviceID"       //设备ID
#define SIP_STRKEY_RESPONSE                        "Response"       //回复
#define SIP_STRKEY_SUMNUM                          "SumNum"         //总数
#define SIP_STRKEY_DEVICELIST                      "DeviceList"     //设备列表
#define SIP_STRKEY_NUM                             "Num"            //数量
#define SIP_STRKEY_ITEM                            "Item"           //项目
#define SIP_STRKEY_NAME                            "Name"           //名称
#define SIP_STRKEY_MANUFACTURER                    "Manufacturer"   //厂商
#define SIP_STRKEY_MODEL                           "Model"          //设备型号
#define SIP_STRKEY_OWNER                           "Owner"          //设备归属
#define SIP_STRKEY_CIVILCODE                       "CivilCode"      //行政区域
#define SIP_STRKEY_BLOCK                           "Block"          //警区
#define SIP_STRKEY_ADDRESS                         "Address"        //设备安装地址
#define SIP_STRKEY_PARENTAL                        "Parental"       //子节点存在标识
#define SIP_STRKEY_PARENTID                        "ParentID"       //父节点ID
#define SIP_STRKEY_SAFEWAY                         "SafeWay"        //信令安全模式
#define SIP_STRKEY_REGISTERWAY                     "RegisterWay"    //注册方式
#define SIP_STRKEY_CERTNUM                         "CertNum"        //证书序列号
#define SIP_STRKEY_CERTIFIABLE                     "Certifiable"    //证书有效标识
#define SIP_STRKEY_ERRCODE                         "ErrCode"        //无效原因
#define SIP_STRKEY_STARTTIME                       "StartTime"      //开始时间
#define SIP_STRKEY_ENDTIME                         "EndTime"        //结束时间
#define SIP_STRKEY_SECRECY                         "Secrecy"        //保密属性
#define SIP_STRKEY_IPADDRESS                       "IPAddress"      //IP地址
#define SIP_STRKEY_PORT                            "Port"           //端口
#define SIP_STRKEY_PASSWORD                        "Password"       //口令
#define SIP_STRKEY_STATUS                          "Status"         //状态
#define SIP_STRKEY_LONGITUDE                       "Longitude"      //经度
#define SIP_STRKEY_LATITUDE                        "Latitude"       //纬度
#define SIP_STRKEY_DEVICETYPE                      "DeviceType"     //设备类型(DVR等)
#define SIP_STRKEY_FIRMWARE                        "Firmware"       //设备固件版本
#define SIP_STRKEY_MAXCAMERA                       "MaxCamera"      //最大视频通道数
#define SIP_STRKEY_MAXALARM                        "MaxAlarm"       //最大告警通道数
#define SIP_STRKEY_ONLINE                          "Online"         //是否在线
#define SIP_STRKEY_STATUS                          "Status"         //是否正常工作
#define SIP_STRKEY_ENCODE                          "Encode"         //是否编码
#define SIP_STRKEY_RECORD                          "Record"         //是否录像
#define SIP_STRKEY_DEVICETIME                      "DeviceTime"     //设备时间和日期
#define SIP_STRKEY_ALARMSTATUS                     "Alarmstatus"    //报警设备状态
#define SIP_STRKEY_DUTYSTATUS                      "DutyStatus"     //任务状态
#define SIP_STRKEY_NOTIFY                          "Notify"         //通报
#define SIP_STRKEY_CONTROL                         "Control"        //控制
#define SIP_STRKEY_PTZCMD                          "PTZCmd"         //PTZ命令
#define SIP_STRKEY_INFO                            "Info"           //信息
#define SIP_STRKEY_CONTROLPRIORITY                 "ControlPriority" //控制优先级
#define SIP_STRKEY_RECORDCMD                       "RecordCmd"      //录像命令
#define SIP_STRKEY_FILEPATH                        "FilePath"       //文件路径名
#define SIP_STRKEY_TYPE                            "Type"           //录像产生类型
#define SIP_STRKEY_RECORDERID                      "RecorderID"     //录像触发者ID
#define SIP_STRKEY_RECORDLIST                      "RecordList"     //文件目录项列表
#define SIP_STRKEY_NOTIFYTYPE                      "NotifyType"     //通报类型
#define SIP_STRKEY_GUARDCMD                        "GuardCmd"       //布防/撤防命令
#define SIP_STRKEY_ALARMCMD                        "AlarmCmd"       //报警复位命令
#define SIP_STRKEY_TELEBOOT                        "TeleBoot"       //远程启动控制命令
#define SIP_STRKEY_ALARMPRIORITY                   "AlarmPriority"  //报警级别
#define SIP_STRKEY_ALARMMETHOD                     "AlarmMethod"    //报警方式
#define SIP_STRKEY_ALARMTIME                       "AlarmTime"      //报警时间
#define SIP_STRKEY_ALARMDESCRIPTION                "AlarmDescription" //报警内容描述
#define SIP_STRKEY_CHANNEL                         "Channel"        //视频输入通道数



#define SIP_STRKEY_SESSION_NAME                    "s"              //请求媒体流的操作类型
#define SIP_STRKEY_MEDIA_ATTRIBUTE                 "a"              //媒体编码属性
#define SIP_STRKEY_URI_DESCRIPTION                 "u"              //媒体源标识
#define SIP_STRKEY_TIME_DESCRIPTION                "t"              //时间描述


// 命令定义
#define SIP_STR_CATALOG                         "Catalog"        //目录
#define SIP_STR_DEVICEINFO                      "DeviceInfo"     //设备信息
#define SIP_STR_DEVICESTATUS                    "DeviceStatus"   //设备状态
#define SIP_STR_KEEPALIVE                       "Keepalive"      //存活
#define SIP_STR_RECORDINFO                      "RecordInfo"     //录像信息
#define SIP_STR_DEVICECONTROL                   "DeviceControl"  //设备控制
#define SIP_STR_RECORD                          "Record"         //录像
#define SIP_STR_STOPRECORD                      "StopRecord"     //录像
#define SIP_STR_MEDIASTATUS                     "MediaStatus"    //媒体状态
#define SIP_STR_SETGUARD                        "SetGuard"       //布防
#define SIP_STR_RESETGUARD                      "ResetGuard"     //撤防
#define SIP_STR_RESETALARM                      "ResetAlarm"     //报警复位
#define SIP_STR_ALARM                           "Alarm"          //报警通知
#define SIP_STR_RESETALARM                      "ResetAlarm"     //报警复位
#define SIP_STR_BOOT                            "Boot"           //设备启动

#define SIP_STR_PLAY                             "Play"          //实时流点播
#define SIP_STR_PLAYBACK                         "Playback"      //历史回放
#define SIP_STR_DOWNLOAD                         "Download"      //文件下载


#define SIP_STR_RTSP_PLAY                        "PLAY"          //播放
#define SIP_STR_RTSP_PAUSE                       "PAUSE"         //暂停
#define SIP_STR_RTSP_TEARDOWN                    "TEARDOWN"      //停止
#define SIP_STR_MANSRTSP_VERSION                 "MANSRTSP/1.0"  //版本
#define SIP_STR_RTSP_CSEQ                        "CSeq"          //序号
#define SIP_STR_RTSP_SCALE                       "Scale"         //速度
#define SIP_STR_RTSP_RANGE                       "Range"         //范围
#define SIP_STR_RTSP_NPT                         "npt"           //时间
#define SIP_STR_RTSP_NOW                         "now"           //当前



/*
 *********************************************
 Function : StruSipDialogKey_Cmp
 Version : 1.0.0.1
 Author : 邹阳星
 DateTime : 2012/12/7 9:33
 Description :  StruSipDialogKey 的比较函数
 Input :  pSrc 比较的数据
 Input :  pDest 比较的数据
 Output : 
 Return : 相等返回 0， 如果 *pSrc > *pDest 返回 1， 其他返回 -1
 Note :   
 *********************************************
 */
SIP_DLL_API int SipDialogKey_Cmp(const StruSipDialogKey *pSrc,const StruSipDialogKey *pDest );


/*
*********************************************
Function : SipSession_SetUserData
Version : 1.0.0.1
Author : 邹阳星
DateTime : 2012/8/17 9:45
Description :  存储用户到会话数据
Input : hSipSession  会话的句柄
Input :  pData 用户数据
Output : 
Return : 
Note :   初始默认为 NULL
*********************************************
*/
SIP_DLL_API void SipSession_SetUserData(SipSessionHandle hSipSession, void *pUserData );


/*
*********************************************
Function : SipSession_GetUserData
Version : 1.0.0.1
Author : 邹阳星
DateTime : 2012/8/17 9:46
Description :  返回 会话存储的用户数据
Input : hSipSession  会话的句柄
Input :  
Output : 
Return : 返回SetUserData存储的数
Note :   
*********************************************
*/
SIP_DLL_API void *SipSession_GetUserData(SipSessionHandle hSipSession);





/*
*********************************************
Function : SipSession_Resigter
Version : 1.0.0.1
Author : 邹阳星
DateTime : 2012/8/17 9:47
Description :  会话发送注册命令
Input : hSipSession  会话的句柄
Input :  czUsername 用户名
Input :  csPassword  密码
Input : iTimeout 超时时间,单位 毫秒
Output : pRes 返回的结果， 如果 为NULL 表示 不等待结果返回
Input : iTimeouts 等待返回的超时时间 ，单位 毫秒
Return : 
Note :   
*********************************************
*/
SIP_DLL_API EnumSipErrorCode SipSession_Resigter(SipSessionHandle hSipSession,
												 const char *czUserName, 
												 const char *czPassword,
												 StruSipData *pRes, int iTimeout );
SIP_DLL_API EnumSipErrorCode SipSession_Unresigter(SipSessionHandle hSipSession,
												 StruSipData *pRes, int iTimeout );

/*
*********************************************
Function : SipSession_Disconnect
Version : 1.0.0.1
Author : 邹阳星
DateTime : 2012/8/17 9:50
Description :  断开本次会话
Input : hSipSession  会话的句柄
Input :  
Output : 
Return : 
Note :   
*********************************************
*/
SIP_DLL_API EnumSipErrorCode SipSession_Disconnect(SipSessionHandle hSipSession);

SIP_DLL_API void SipSession_Release(SipSessionHandle hSipSession);
/*
*********************************************
Function : SipSession_SendMessage
Version : 1.0.0.1
Author : 邹阳星
DateTime : 2012/8/17 9:17
Description :  发送数据
Input : hSipSession  会话的句柄
Input :  eMethod 发送的方法
Input : eContentType 内容的类型描述
Input : czContent 文本方式Content
Input : pContent 二进制方式的Content
Input : iContentLength pContent 长度
Output : ppRes 返回的结果， 如果 为NULL 表示 不等待结果返回
Input : iTimeouts 等待返回的超时时间 ，单位 毫秒
Output : pOutDlgKey 如果不为NULL 返回 会话的的Key
Return : 
Note :   
*********************************************
*/
SIP_DLL_API EnumSipErrorCode SipSession_SendMessage(SipSessionHandle hSipSession,			   
								  const StruSipData *pSendData, StruSipData *pRes,
								  int iTimeouts,  StruSipDialogKey *pOutDlgKey);



//获取会话的连接信息
SIP_DLL_API EnumSipErrorCode SipSession_GetConnectInfo(SipSessionHandle hSipSession, StruSipConnnectInfo *pRes);

//获取会话的管理服务
SIP_DLL_API SipServiceHandle SipSession_GetService(SipSessionHandle hSipSession);


//重联, 重联后需要重新注册
SIP_DLL_API EnumSipErrorCode SipSession_Reconnect(SipSessionHandle hSipSession);


//返回注册登陆的用户名
SIP_DLL_API const char *SipSession_Authorization_Username(SipSessionHandle hSipSession);

/*
 *********************************************
 Function : SipSession_Authorize
 Version : 1.0.0.1
 Author : 邹阳星
 DateTime : 2012/9/26 8:51
 Description :  权限核准, 判断 给出的 czUsername，czPasswrd 是否符合注册收到的权限证书相等
 Input :  czUsername 给出的用户名，明文
 Input :  czPasswrd 给出的密码, 明文
 Output : 
 Return : 符合判断返回 SUCCESS, 其他表示错误
 Note :   
 *********************************************
 */
SIP_DLL_API EnumSipErrorCode SipSession_Authorize(SipSessionHandle hSipSession, 
												   const char *czUsername, const char *czPasswrd );

/*
 *********************************************
 Function : SipSession_SetOptions
 Version : 1.0.0.1
 Author : 邹阳星
 DateTime : 2012/12/26 14:20
 Description :  Session 的选项控制函数
 Input :  iOptionName 选项名称
 Input :  pValue 选项值
 Input :  iValueSize pValue 的长度
 Output : 
 Return : 
 Note :   
 *********************************************
 */
//控制 Session 断线后是否 调用 Unresigter , pValue 为 int *  1 表示 是， 0 表示 否
#define SIP_SESSION_O_UNREGISTER  1  
//设置日志等级， pValue 为 int *  ， 值 0~7
#define SIP_GLOBAL_DEBUG_LEVEL 2
SIP_DLL_API EnumSipErrorCode SipSession_SetOptions(SipSessionHandle hSipSession, 
												  int iOptionName, const void *pValue, int iValueSize );

/*
*********************************************************************
*
*@brief : 
*
*********************************************************************
*/

//事件监听者
typedef struct _StruSipListener
{
	/*
	*********************************************
	Function : OnClientConnectEvent
	Version : 1.0.0.1
	Author : 邹阳星
	DateTime : 2012/8/17 9:53
	Description :  接受到新连接
	Input :  hService 接受到连接的服务器
	Input : pNewSession 新的连接
	Output : 
	Return : 返回任意错误表示拒绝连接
	Note :   
	*********************************************
	*/
	EnumSipErrorCode (*OnClientConnectEvent)( SipServiceHandle hService,
										   SipSessionHandle hNewSession );

	//断开连接
	void (*OnClientDisconnectEvent)(SipServiceHandle hService, SipSessionHandle hNewSession);

	//给上层的回调皆用此接口 是网络请求的回调还是应答回调根据 pData类型决定
	void (*OnSIPPacketEvent)(SipServiceHandle hService, 
						SipSessionHandle hNewSession,
						StruSipData *pData);
}StruSipListener;



SIP_DLL_API SipServiceHandle SipService_Create(const StruSipListener *pListener);

SIP_DLL_API void SipService_Release(SipServiceHandle hSipService);

SIP_DLL_API void SipService_SetUserData(SipServiceHandle hSipService, void *pUserData );

SIP_DLL_API void *SipService_GetUserData(SipServiceHandle hSipService);

//设置服务器名称
SIP_DLL_API EnumSipErrorCode SipService_Set_ServerName(SipServiceHandle hSipService, const char *czServerName);

//设置网关
SIP_DLL_API EnumSipErrorCode SipService_Set_Gateway(SipServiceHandle hSipService, const char *czGatewayIPV4);


/*
*********************************************
Function : SipService_Start
Version : 1.0.0.1
Author : 邹阳星
DateTime : 2012/8/17 9:56
Description :  开始服务
Input : hSipService 控制的服务器
Input :  eCnnType 网络类型, 当前只支持 UDP
Input :  czLocalIp 本地绑定的IP， 如果为 NULL 表示全监听
Input :  iLocalPort 本地绑定的端口,如果 为 0， 系统指定选定可以
		有 SipService_Get_LocalPort 函数获取真实端口
Output : 
Return : 
Note :   本函数只可以调用一次
*********************************************
*/
SIP_DLL_API EnumSipErrorCode SipService_Start(SipServiceHandle hSipService,
										  EnumSipConnectType eCnnType,
										  const char *czLocalIp, int iLocalPort);

//返回本地端口， 当Start iLocalPort 为 < 1 时， 由系统指定时， 可以由此函数返回真实的监听端口
SIP_DLL_API int SipService_Get_LocalPort(SipServiceHandle hSipService);


//停止服务
SIP_DLL_API void SipService_Stop(SipServiceHandle hSipService);

/*
*********************************************
Function : SipService_Connect/SipService_ConnectEx
Version : 1.0.0.1
Author : 邹阳星
DateTime : 2012/8/17 9:58
Description :  建立连接
Input : hSipService 服务的句柄
Input :  eCnnType 网络类型
Input : czRemoteHost 对端地址
Input :  iRemotePort 对端IP
Output : 
Return : 失败返回 SIP_INVALID_HANDLE， 否则返回信息会话句柄
Note :   
*********************************************
*/
SIP_DLL_API SipSessionHandle    SipService_Connect( SipServiceHandle hSipService,
								EnumSipConnectType eCnnType, 
								const char *czRemoteHost,int iRemotePort
								,const char *czRemoteSipServerName  = 0);


/*
 *********************************************
 Function : SipService_GetListenInfo
 Version : 1.0.0.1
 Author : 邹阳星
 DateTime : 2012/9/7 9:54
 Description :  获取监听的网络信息
 Input : hSipService 服务的句柄
 Output : pRes 获取到得信息
 Return : 
 Note :   
 *********************************************
 */
SIP_DLL_API EnumSipErrorCode SipService_GetListenInfo(SipServiceHandle hSipService, StruSipConnnectInfo *pRes);

#endif //end _GS_H_SIPSTACK_H_
