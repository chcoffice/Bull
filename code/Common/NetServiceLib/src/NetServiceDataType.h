#if !defined (NetServiceDataType_DEF_H)
#define NetServiceDataType_DEF_H

/***************************************************************************************************
	Copyright (C), 1997-2010, gosun Tech. Co., Ltd.
	FileName: 		NetServiceDataType.h
	Author:        	陈聪
	Version :       v1.0   
	Date:			2010/06/03
	Description:    网络库公共的头文件和数据类型，指网络库自己内部使用的。 

****************************************************************************************************/

#ifdef WINCE
#define FD_SETSIZE      1024
#define	OPERATING_SYSTEM	1  
#endif

#include "DataType.h"
#include <assert.h>
#include <deque>			//C++STL的容器
#include <map>
#include <vector>
#include <algorithm>
using namespace std;

namespace NetServiceLib
{

#define ERROR_CTRLPRINTF	//输出调试信息

	#ifdef _WIN32
		#ifdef ERROR_CTRLPRINTF	
			#define CTRLPRINTF( xLog,xFormat, ... ) \
				do { \
					if( NULL != xLog) \
						(xLog)->Log(LNOTSET, NULL, " (%s %d) " xFormat,__FILE__, __LINE__, __VA_ARGS__); \
				}while(0)
				

		#else
			#define CTRLPRINTF( xLog, xFormat, ... )  do { } while( 0 )
		#endif
	#else
		#ifdef ERROR_CTRLPRINTF	
			#define CTRLPRINTF( xLog, xFormat, ... ) \
			do { \
				if( NULL != xLog) \
					(xLog)->Log(LNOTSET,NULL," (%s %d) " xFormat,__FILE__, __LINE__, ##__VA_ARGS__); \				
			}while(0)

		#else
			#define CTRLPRINTF( xLog, xFormat, ...)  do { } while( 0 )
		#endif
#endif

#ifdef _DEBUG
#define INFO_CTRLPRINTF	//DEBUG才输出的信息
#endif
#ifdef _WIN32
	#ifdef INFO_CTRLPRINTF	
		#define CTRLPRINTF_D( xLog, xFormat, ... ) \
			do { \
				if( NULL != xLog) \
					(xLog)->Log(LNOTSET,NULL," (%s %d) " xFormat,__FILE__, __LINE__, __VA_ARGS__); \
			}while(0)


	#else
		#define CTRLPRINTF_D( xLog, xFormat, ... )  do { } while( 0 )
	#endif
#else
	#ifdef INFO_CTRLPRINTF	
		#define CTRLPRINTF_D( xLog, xFormat, ... ) \
			do { \
				if( NULL != xLog) \
				(xLog)->Log(LNOTSET,NULL," (%s %d) " xFormat,__FILE__, __LINE__, ##__VA_ARGS__); \				
			}while(0)

	#else
		#define CTRLPRINTF_D( xLog, xFormat, ...)  do { } while( 0 )
	#endif
#endif

#ifdef _WIN32

#include <Windows.h>
#else	//linux
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <sys/types.h> 
#include <netdb.h>
#include <sys/epoll.h>

//#define HAVE_GETHOSTBYNAME2

#endif


#ifdef _WIN32
typedef 		SOCKET		SOCKETHANDLE;
#else
typedef			int			SOCKETHANDLE;
#endif



typedef enum
{
	LISTEN,			//监听
	ACCEPTDATA,		//接收数据
	ACTIVETEST,		//活动检测
	EPOLLEVENTWAIT,	//epoll事件
	CHANNELSTATUS,	//通道状态
	ACCEPTNOTICE,	// 新连接通知，指服务端

}enumThreadEventType;

//通道状态
typedef enum
{
	CHANNEL_NORMAL,		//正常状态
	CHANNEL_DISCNN,		//中断状态			//只有TCP才有这个状态
	CHANNEL_CLOSE,		//关闭
	CHANNEL_TIMEOUT,	//超时
	CHANNEL_ADD_IOCPOREPOLL, //通道需要加入IOCP或epoll
}enumChannelStatus;


#define	 DATA_BUFSIZE	8192	// socket接收缓存大小
// 操作类型
#define   RECV_POSTED			1001  
#define   SEND_POSTED			1002
#define   ACCESS_POSTED			1003

// 日志指针
extern ILogLibrary*			g_clsLogPtr;
/**************************************************************************
Function    : Utility::GetApplicationPath    
DateTime    : 2010/8/26 11:13	
Description : 获取应用程序目录
Input       : 无
Output      : 无
Return      : 返回应用程序目录
Note        :	
**************************************************************************/
string GetApplicationPath(void);
}

#endif


