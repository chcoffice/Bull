#ifndef GSP_ISERVER_DEF_H
#define GSP_ISERVER_DEF_H

#include "GSPConfig.h"
#include "IUri.h"
#include "ISource.h"



namespace GSP
{

//为兼容使用



	class  CGSPServerOptions 
	{
	public:
		BOOL m_bSecurity; //使能安全检查
		BOOL m_bTCP; //使能 TCP 协议
		INT  m_iTCPPort;  //TCP 监听端口
		UINT32 m_iMaxKeepalive; //秒
		BOOL m_bUDP; //使能 UDP 协议
		INT m_iUDPPort;    //UDP 监听端口
		UINT32 m_iMaxClients;            
		BOOL m_bFlowCtrl; //使能流量控制
		BOOL m_bCoderConver; //使能转码功能	
		std::string m_strDynamicModulePath; //动态模块库存放目录
		UINT16 m_iRetryPktsCache; //重传包的CACHE个数大小  默认 GSP_RETRYS_PACKETS_CACHE
		BOOL m_bEnableSecurity; //是否启用安全印证 ,默认 TRUE
		UINT m_iLogLevelConsole;
		UINT m_iLogLevelFile;

		CGSPServerOptions(void);

	};
 


/*
******************************************
Copyright (C), 2010-2011, GOSUN
Filename : IGSPSERVER.H
Author :  zouyx
Version : 0.1.0.0
Date: 2010/6/11 12:10
Description:  GSP 服务器接口
********************************************
*/
class CIServer;


/*
*********************************************
Function :   GSPServerEventFunctionPtr
DateTime : 2010/6/11 16:46
Description : 服务器事件回调函数
Input :  pServer 对应的服务器
Input :  pSource  发生事件的数据源, 如果是服务器事件该值为NULL， 具体参考 EnumGSPServerEvent
Input :  eEvent 事件类型， 参考  EnumGSPServerEvent定义
Input :  pEventParams 事件参数， 具体值由 eEvent决定， 参考  EnumGSPServerEvent定义
Input :  iLen pEventParams 的长度
Input :  pParamData 回调用户参数

Output :
Return : 返回URI描述的数据源, 失败返回NULL
Note :
*********************************************
*/
typedef enum
{
   GSP_SRV_EVT_CTRL,  //控制数据源事件，pSource 有效，  事件参数pEventPararms 为 StruGSPCmdCtrl *, 返回TRUE/FALSE
   GSP_SRV_EVT_REF,   //数据源引用增加事件，pSource 有效，  事件参数pEventPararms 为 NULL, 返回值无用
   GSP_SRV_EVT_UNREF, //数据源引用减少事件，pSource 有效，  事件参数pEventPararms 为 NULL, 返回值无用
   GSP_SRV_EVT_RQUEST_SOURCE, //请求数据源回调函数, pSource 无效， 事件参数pEventPararms 为 CIUri *, 返回 CISource *
}EnumGSPServerEvent;     

typedef void* (*GSPServerEventFunctionPtr)(CIServer *pServer, CISource *pSource,
										  EnumGSPServerEvent eEvent, 
                                               void *pEventPararms, 
											   INT iLen,  void *pParamData );

//状态查询
typedef enum
{
	GSP_SRV_STATUS_SUPPORT_PROTOCOL = 0, //支持的协议	, 数据结果为 StruProtocolService*
	GSP_SRV_STATUS_SOURCE_LIST, //存储的数据源. 数据结果为 StruSourceVector *
	GSP_SRV_STATUS_GET_CLIENTS_INFO, //返回客户端描述,  StruClientVector *
}EnumGSPServerStatus;


typedef struct _StruProtocolService
{
	INT iNums; //指定 vService 的个数
	struct _StruServiceInfo
	{
		const  char *czProtocol;
		const char *czSrvBindIP;
		INT iSrvBindPort;
		INT iStatus;   // 0 正常， 其他异常
	}vService[1];
}StruProtocolService;

typedef struct _StruSourceVector
{
	INT iNums; //指定 vSourceIndex 的个数
	UINT16 vSourceIndex[1]; //数据源的索引
}StruSourceVector;


typedef struct _StruClientInfo
{
	EnumProtocol eProtocol; //协议
	char szRemoteIP[GSP_STRING_IP_LEN]; //对端IP
	INT iReomtePort; //对端端口
	char szSourceKey[GSP_MAX_URI_KEY_LEN]; //点流数据源的KEY
	UINT64 iRecv; //接收的字节数
	UINT64 iSend; //发送的字节数
	UINT32 iLostFrames; //丢帧数
	UINT32 iResendPackets; //重传包数
} StruClientInfo;

typedef struct _StruClientVector
{
	INT iNums; //指定 vSourceIndex 的个数
	StruClientInfo vClient[1];
}StruClientVector;



class  CIServer 
{
public:
	
    virtual ~CIServer(void)
    {

    }

    /*
    *********************************************
    Function :  InitLog 
    DateTime : 2010/6/11 16:19
    Description : 设置日志数据接口
    Input :   czPathName 设置日志接口的路径
    Output :  
    Return :  
    Note :
    *********************************************
    */
    virtual void InitLog( const char *czPathName ) = 0;


    

    /*
    *********************************************
    Function : Init
    DateTime : 2010/6/11 16:22
    Description :  启动服务
    Input :
    Output :
    Return : TRUE/FALSE
    Note :
    *********************************************
    */
    virtual BOOL Init(const char *csConfigFilename,const char *csLogPathName) = 0;

    /*
    *********************************************
    Function : Stop
    DateTime : 2010/6/11 16:22
    Description :  停止服务
    Input :
    Output :
    Return : TRUE/FALSE
    Note :
    *********************************************
    */
    virtual BOOL Stop(void) = 0;


    /*
    *********************************************
    Function :   FindSource
    DateTime : 2010/6/11 16:27
    Description :  用键值来查找数据源
    Input : czKey 要查找的数据源键值
    Output :
    Return :  不存在返回NULL
    Note :
    *********************************************
    */
    virtual CISource *FindSource(const char *czKey) = 0;

    virtual CISource *FindSource(UINT16 iSourceIndex ) = 0;


    /*
    *********************************************
    Function :   AddPushSource
    DateTime : 2010/6/11 16:27
    Description :  添加推模式数据源
    Input : czKey 数据源键值
    Output :
    Return : 返回建立的推模式数据源， 失败返回NULL
    Note :
    *********************************************
    */
    virtual CISource *AddSource( const char *czKey ) = 0;


	/*
	*********************************************
	Function :   AddPushSource
	DateTime : 2010/6/11 16:27
	Description :  添加拉模式数据源
	Input : czKey 数据源键值
	Output :
	Return : 返回建立的推模式数据源， 失败返回NULL
	Note :
	*********************************************
	*/
	virtual CISource *AddPullSource( const char *czKey ) = 0;


 

    virtual CISource::EnumRetNO WriteData( UINT16 iSourceIndex,  const void *pData, INT iLen, UINT iChn, BOOL bKey) = 0; 


    virtual CISource::EnumRetNO WriteSysHeader( UINT16 iSourceIndex, const void *pData, INT iLen, UINT iChn) = 0;

    virtual CISource::EnumRetNO WriteDataV( UINT16 iSourceIndex,  const StruIOV *pIOV, INT iVNums, UINT iChn, BOOL bKey) = 0; 


    virtual CISource::EnumRetNO WriteSysHeaderV( UINT16 iSourceIndex, const StruIOV *pIOV, INT iVNums, UINT iChn) = 0;


    /*
    *********************************************
    Function : SetEventCallback
    DateTime : 2010/6/11 16:32
    Description :  设置服务器器事件回调
    Input :   fnOnEvent 回调函数
    Input : pParam  回调的用户参数
    Output :
    Return : 
    Note :
    *********************************************
    */
    virtual void SetEventCallback( GSPServerEventFunctionPtr fnOnEvent, void *pParam) = 0;

	
	virtual CIUri *CreateEmptyUriObject(void) = 0;

    virtual void InitURI( CIUri &csRet, const char *czKey, const char *czPro = "gsp", const char *szRemoteIP = NULL) = 0;

	/*
	 *********************************************
	 Function : QueryStatus
	 DateTime : 2012/3/13 15:38
	 Description :  状态查询
	 Input :  eQueryStatus 指定查询的选项
	 Output : ppResult 返回的数据结果
	 Output : pResultSize 结果的大小
	 Return : 
	 Note :   结果需要调用 FreeQueryStatusResult 释放
	 *********************************************
	 */
	virtual BOOL QueryStatus(const EnumGSPServerStatus eQueryStatus, char **ppResult,INT *pResultSize  ) = 0;

	/*
	 *********************************************
	 Function : FreeQueryStatusResult
	 DateTime : 2012/3/13 15:44
	 Description :  释放查询的结果集
	 Input :  
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
	virtual void FreeQueryStatusResult( char *pResult) = 0;



	/*
	*********************************************
	Function : GetOptions
	DateTime : 2010/6/11 16:19
	Description : 返回服务器的配置项
	Input :
	Output :  返回配置项
	Return :
	Note : 改函数已经废弃
	*********************************************
	*/
	virtual BOOL GetOptions(CGSPServerOptions &Opts) const  = 0;

	/*
	*********************************************
	Function : SetOptions
	DateTime : 2010/6/11 16:19
	Description : 设置服务器的配置项
	Input :   csOptions 参数， 参考  CGSPServerOptions 定义
	Output :  
	Return :  TRUE/FALSE
	Note :  改函数已经废弃
	*********************************************
	*/
	virtual BOOL SetOptions(const CGSPServerOptions &csOptions) = 0;
 
};




/*
*********************************************
Function :   CreateGSPServerInterface
DateTime : 2010/8/5 17:22
Description :  创建服务器接口对象
Input :
Output :  返回模块接口对象， 失败返回NULL
Return :
Note :
*********************************************
*/
GS_API GSP::CIServer *CreateGSPServerInterface(void);


/*
 *********************************************
 Function : GspSetMaxCacheMemorySize
 Version : 1.0.0.1
 Author : 邹阳星
 DateTime : 2013/5/14 11:20
 Description :  设置缓冲区大小
 Input :  iSizeMByte 设定值， 单位 兆 
 Output : 
 Return : 
 Note :   在 CreateGSPServerInterface 和 CreateGSPClientSectionInterface 调用前设定有效
 *********************************************
 */
GSP_API  void GspSetMaxCacheMemorySize( UINT iSizeMByte );

};

#endif
