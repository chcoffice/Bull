#if !defined (DataType_DEF_H)
#define DataType_DEF_H
/********************************************************************
	Copyright (C), 1997-2010, gosun Tech. Co., Ltd.
	FileName: 		DataType.h
	Author:        	陈聪
	Version :       v1.0   
	Date:			2010/04/28
	Description:    此文件定义提供给外部接口使用的数据类型或头文件

*********************************************************************/
//公共库头
#include "GSType.h"
#include "GSCommon.h"
#include "GSDefine.h"



namespace NetServiceLib
{

class ISocketChannel;

typedef enum 
{
	CLIENT,				//客户端
	SERVER,				//服务器端
}enumServerType;
//通道类型结构
typedef enum 
{
	LISTEN_CHANNEL,		//监听通道
	COMM_CHANNEL,		//通讯通道
}enumChannelType;
typedef enum
{
	NET_PROTOCOL_TCP,	//TCP协议
	NET_PROTOCOL_UDP,	//UDP协议
}enumNetProtocolType;
typedef enum
{
	NET_ACCEPT,			//接受连接
	NET_READ,			//读取，即收到数据
	NET_TIMEOUT,		//通道活动检测超时，即某通道在设定的时间内没有任何通讯
	NET_WRITE_FAIL,		//发送失败
	NET_REMOTE_DISCNN,	//对端主动断开连接，主要针对TCP方式
	NET_RECONNECT_FAIL,	//重连失败，针对TCP客户端方式
	NET_RECONNECT_SUCCESS,	//重连成功，针对TCP客户端方式
	NET_DISCNN,			//中断
	NET_ERR,			//发生错误
	NET_CLOSE,			//关闭
	NET_NORMAL,			//正常状态，表示该通道可以收发数据
	
}enumNetEventType;//供外部调用

//回调函数定义，回调函数必须是线程安全的
typedef INT (*pOnEventCallBack)(ISocketChannel* pSocketChnl, void* pUserData, enumNetEventType enumType, void* pData, UINT32 uiDataSize);





#define		ERROR_NET_UNKNOWN						(ERROR_NET_START + 1)	//未知错误
#define		ERROR_NET_PARAM_WRONG					(ERROR_NET_START + 2)	//传入的参数有误
#define		ERROR_NET_OVERFLOW_MAX_THREADS			(ERROR_NET_START + 3)	//超出最大线程数
#define		ERROR_NET_REPEAT_INIT					(ERROR_NET_START + 4)	//重复初始化
#define		ERROR_NET_CREATE_SOCKET_FAIL			(ERROR_NET_START + 5)	//创建socket失败
#define		ERROR_NET_BIND_SOCKET_FAIL				(ERROR_NET_START + 6)	//绑定socket失败
#define		ERROR_NET_MAX_CHANNEL_COUNT				(ERROR_NET_START + 7)	//已达到最大连接数
#define		ERROR_NET_WSASTARTUP_FAIL				(ERROR_NET_START + 8)	//启动网络失败
#define		ERROR_NET_MAX_CNETSERVICE_COUNT			(ERROR_NET_START + 9)	//达到CNetService最大计数
#define		ERROR_NET_CHANNEL_NOT_EXIST				(ERROR_NET_START + 10)	//通道不存在
#define		ERROR_NET_THREAD_NOT_EXIST				(ERROR_NET_START + 11)	//线程不存在
#define		ERROR_NET_SOCKET_NOT_EXIST				(ERROR_NET_START + 12)	//SOKCET不存在
#define		ERROR_NET_SOCKET_CONNECT_FAIL			(ERROR_NET_START + 13)	//连接时失败
#define		ERROR_NET_CREATE_CHANNEL_FAIL			(ERROR_NET_START + 14)	//创建通道失败
#define		ERROR_NET_CALLBACK_NOT_INIT				(ERROR_NET_START + 15)	//回调函数未赋初值
#define		ERROR_NET_INVALID_CHANNEL_POINT			(ERROR_NET_START + 16)	//无效的通道指针，可能通道已经被删除
#define		ERROR_NET_CHANNEL_STUTAS_CLOSE			(ERROR_NET_START + 17)	//通道属于无效状态
#define		ERROR_NET_CREATE_EPOLL_FAIL				(ERROR_NET_START + 18)	//创建EPOLL失败

}

using namespace NetServiceLib;

#endif //end #if !defined (DataType_DEF_H)


