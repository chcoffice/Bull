#ifndef NET_COMMON_STACK_DEF_H
#define NET_COMMON_STACK_DEF_H

#include "GSCommon.h"

#ifdef _WIN32

#else
#define CALLBACK 
#endif

//ID定义
typedef struct StruID
{
	UINT uiPlatformID;
	UINT uiSysID;
}StruID;

class INetMessage
{
public:
	INetMessage(){};
	virtual ~INetMessage(){};
public:
	/***************************************************************************
	  Function:        	GetMSGLen
	  DATE:				2010-6-9   9:06
	  Description: 		获取消息长度   
	  Input:  			           
	  Output:     		    
	  Return:   		      
	  Note:						
	****************************************************************************/
	virtual INT32 GetMSGLen() = 0;
	/***************************************************************************
	  Function:        	GetMSG
	  DATE:				2010-6-9   9:06
	  Description: 		获取消息   
	  Input:  			           
	  Output:     		    
	  Return:   		      
	  Note:						
	****************************************************************************/
	virtual void* GetMSG() = 0;
	/***************************************************************************
	  Function:        	GetSessionID
	  DATE:				2010-6-9   9:07
	  Description: 		获取会话ID   
	  Input:  			           
	  Output:     		    
	  Return:   		      
	  Note:						
	****************************************************************************/
	virtual UINT16 GetSessionID() = 0;
	/***************************************************************************
	  Function:        	GetCommandID
	  DATE:				2010-6-9   9:07
	  Description: 		获取命令ID   
	  Input:  			           
	  Output:     		    
	  Return:   		      
	  Note:						
	****************************************************************************/
	virtual UINT32 GetCommandID() = 0;
	virtual StruID* GetSrcModule() = 0;
	virtual StruID* GetDstModule() = 0;
};
class INetChannel;

#define NET_EVENT_CONNECT	0
#define NET_EVENT_READ		1
#define NET_EVENT_CLOSE		2
#define NET_EVENT_CONNECT_SERVER 3


typedef INT32 (CALLBACK* fnNetEvent)(void* pCaller, INetChannel* pChn, INT32 iEvent, INetMessage* pNetMessage);




//网络协议 SOCK_STREAM
#define PROTOCOL_TYPE_TCP				1			
//网络协议 SOCK_DGRAM
#define PROTOCOL_TYPE_UDP				2	

//默认支持客户端主机连接
#define DEFAULT_NET_CHANNEL_NUM					64
//最大支持客户端主机连接
#define MAX_NET_CHANNEL_NUM						(1024*1024)	

#define SUPPORT_MESSAGE_BUF				1
#define SUPPORT_NO_MESSAGE_BUF			0

#define DEFAULT_MSG_BUF_SIZE			(1024*2)
#define MAX_MSG_BUF_SIZE				(1024*1024)

#define DEFAULT_MSG_SIZE				(1024*8)
#define MAX_MSG_SIZE					(1024*1024*2)

#define DEFAULT_NET_PULSE_TIME			5000

#define SUPPORT_RECONNECT				1
#define SUPPORT_NO_RECONNECT			0
#define DEFAULT_RECONNECT_TIME			2000

//30秒没收到数据，则认为链路中断
#define NET_DATA_TIMEOUT				30

//重连时客户端是否用同一个端口进行重连 1表示默认重用.为了兼容之前的写法 所以默认重用
#define REUSE_NETPORT_FLAG					1

class INetConfig
{
public:
	INetConfig(){};
	virtual ~INetConfig(){};

public:
	virtual void SetProtocolType(INT32 iProtocolType) = 0;
	virtual INT32 GetProtocolType() = 0;

	virtual void SetMaxChnNum(INT32 iMaxChnNum) = 0;
	virtual INT32 GetMaxChnNum() = 0;

	virtual void SetSupportMsgBuf(INT32 iSupport) = 0;
	virtual INT32 GetSupportMsgBuf() = 0;
	virtual void SetMaxMsgBufSize(INT32 iBufSize) = 0;
	virtual INT32 GetMaxMsgBufSize() = 0;

	virtual void SetMaxMsgSize(INT32 iMsgSize) = 0;
	virtual INT32 GetMaxMsgSize() = 0;

	virtual void SetNetPulseTime(INT32 iPulseTime) = 0;
	virtual INT32 GetNetPulseTime() = 0;

	virtual void SetSupportReConnect(INT32 iSupport) = 0;
	virtual INT32 GetSupportReConnect() = 0;
	virtual void SetReConnectTime(INT32 iReConnTime) = 0;
	virtual INT32 GetReConnectTime() = 0;

	virtual void SetNetEventCB(void* pCaller, fnNetEvent EventEntry) = 0;
	virtual void* GetNetEventCaller() = 0;
	virtual fnNetEvent GetNetEventEntry() = 0;

	virtual void SetNetDataTimeout(INT32 iTimeOut) = 0;
	virtual INT32 GetNetDataTimeout() = 0;

	virtual void SetReuseNetPortFlag(INT32 iReuse) = 0;
	virtual INT32 GetReuseNetPortFlag() = 0;
};



#define NET_CHANNEL_STATUS_INVALID		0
#define NET_CHANNEL_STATUS_ACTIVE		1
#define NET_CHANNEL_STATUS_SHUTDOWN		2
#define NET_CHANNEL_STATUS_SOCKET_ERROR	3
#define NET_CHANNEL_STATUS_NO_CONNECT	4


class INetChannel
{
public:
	INetChannel(){};
	virtual ~INetChannel(){};

	virtual char* GetRemoteIPAddrString() = 0;
	virtual UINT32 GetRemoteIPAddr() = 0;
	virtual char* GetLocalIPAddrString() = 0;
	virtual UINT32 GetLocalIPAddr() = 0;
	virtual UINT16 GetRemotePort() = 0;
	virtual UINT16 GetLocalPort() = 0;
	/***************************************************************************
	  Function:        	GetLogicRemoteIPAddrString
	  DATE:				2013-6-5   9:01
	  Description: 		获取逻辑的远端ip地址，不一定是真正建立socket连接的地址，由用户自由选择设置该地址。
						比如，用于BS登陆服务器时，代表BS客户端的地址，而不是tomcat的地址。   
	  Return:   		逻辑的远端IP地址 
	****************************************************************************/
	virtual void SetLogicRemoteIPAddrString(const char* pszIPAddr) = 0;
	/***************************************************************************
	  Function:        	GetLogicRemoteIPAddrString
	  DATE:				2013-6-5   9:01
	  Description: 		获取逻辑的远端ip地址，不一定是真正建立socket连接的地址，由用户自由选择设置该地址。
						比如，用于BS登陆服务器时，代表BS客户端的地址，而不是tomcat的地址。   
	  Return:   		逻辑的远端IP地址 
	****************************************************************************/	
	virtual char* GetLogicRemoteIPAddrString() = 0;

	virtual INT32 ConnectServer(char* pszRemoteIPAddr, UINT16 uiRemotePort,
		char* pszBindingIPAddr, UINT16 uiBindingPort) = 0;
	virtual INT32 CloseChannel() = 0;
	virtual INT32 CloseChannelEx() = 0;
	virtual INT32 GetChnStatus() = 0;
	/***************************************************************************
	  Function:        	SendRemoteMsg
	  DATE:				2010-5-26   9:01
	  Description: 		通过通道向远端发送消息   
	  Input:  			uiCMDID:命令号
						pMSG:发送的消息内容
						iMSGLen:发送的长度
						uiSessionID:会话ID，由用户填写
						pSrcID:发送原单元
						pDstID:发送目的单元
	  Output:     		    
	  Return:   		发送的字节数，负数表示失败      
	  Note:						
	****************************************************************************/
	virtual INT32 SendRemoteMsg(UINT32 uiCMDID,
								void* pMSG, 
								INT32 iMSGLen, 
								StruID* pSrcID,
								StruID* pDstID,
								UINT16 uiSessionID = 0) = 0;
};

class INetServer
{
public:
	INetServer(){};
	virtual ~INetServer(){};

	virtual void SetNetConfig(INetConfig* pCfg) = 0;
	virtual INetConfig* GetNetConfig() = 0;

	virtual INT32 StartListen(char* pszBindingIPAddr, UINT16 uiBindingPort, char* pszLogPath = NULL) = 0;
	virtual INT32 StopListen() = 0;
};


class INetClient
{
public:
	INetClient(){};
	virtual ~INetClient(){};

	virtual void SetNetConfig(INetConfig* pCfg) = 0;
	virtual INetConfig* GetNetConfig() = 0;

	virtual INT32 StartNetService(char* pszLogPath = NULL) = 0;
	virtual INT32 StopNetService() = 0;

	virtual INetChannel* AddNetChannel(char* pszRemoteIPAddr, UINT16 uiRemotePort, 
						char* pszBindingIPAddr, UINT16 uiBindingPort) = 0;
};

#ifdef _WIN32
EXPORT_API INT32 InitNetCommuStack();
EXPORT_API INetServer* CreateNetServer();
EXPORT_API INetClient* CreateNetClient();
#else
INT32 InitNetCommuStack();
INetServer* CreateNetServer();
INetClient* CreateNetClient();
#endif

#endif