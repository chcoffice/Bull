/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : GSPCONFIG.H
Author :  zouyx
Version : 0.0.0.0
Date: 2012/3/13 15:11
Description: GSP 相关的定义
********************************************
*/

#ifndef _GS_H_GSPCONFIG_H_
#define _GS_H_GSPCONFIG_H_




// 从 Windows 头中排除极少使用的资料
#define WIN32_LEAN_AND_MEAN             



#if defined(WIN32) || defined(_WIN32)
#include <windows.h>
#endif

#if !defined(_WIN32) && !defined(_LINUX)

#error  Only support WIN32 or LINUX

#else

#if defined(_WIN32) && defined(_LINUX)
#error  Only support WIN32 or LINUX in one times
#endif

#endif

#if defined(_DEBUG) || defined(DEBUG)

#define GSP_DEBUG_MSG

#else
#ifndef NDEBUG
#define NDEBUG
#endif
#endif

#include <GSType.h>
#include <GSCommon.h> 

namespace GSP
{


	//网卡配置
	//IP 字符长度
#define GSP_STRING_IP_LEN 64  
	//路径字符长度
#define GSP_PATH_LEN 256  

#define CGSPString CGSString //转义




// URI 最大长度
#define GSP_MAX_URI_LEN 1024  

//最大支持的媒体通道数
#define GSP_MAX_MEDIA_CHANNELS 8  

//无效通道号  
#define GSP_INVALID_SUBCHANNEL  (-1) 


 //UDP 包的最大长度
#define MAX_UDP_PACKET 1500 

//GSP 包的最大长度
#define GSP_PACKET_V1_SIZE 1500 

#define GSP_PACKET_SIZE_OFFSET_HEADER 64

#define GSP_PACKET_V2_SIZE (2*KBYTES)

#define GSP_PACKET_SIZE GSP_PACKET_V2_SIZE

#define  FIX_BUFFER_SIZE GSP_PACKET_V2_SIZE

#define MAX_GSP_COMMAND_LEN (2*KBYTES)

	//URI Key 部分的最大长度
#define GSP_MAX_URI_KEY_LEN 128  

	//最大连接数
#define GSP_MAX_CONNECT  3000  




 //使能GSP 协议 编译GSP 模块
#define  ENABLE_GSP_MODULE 

//使能RTSP 协议 编译RTSP 模块
//#define  ENABLE_RTSP_MODULE

	//使能SIP28181 协议 编译SIP28181 模块
#define  ENABLE_SIP_MODULE

 //使用嵌入式
	//#define ON_EMBEDDED

//使用第三方 流分析库
#define ENABLE_MANUFACTURERS_STREAM_ANALYZER


#ifndef ON_EMBEDDED

#ifdef _WINCE
#define ON_EMBEDDED
#endif

#endif


	//数据结构定义
	typedef struct _StruIOV
	{		
		unsigned long iSize;
		void *pBuffer;
	}StruIOV;

	/*
	 *********************************************
	 Function : FuncPtrFree
	 DateTime : 2012/4/24 10:30
	 Description :  释放函数定义
	 Input :  pData 被释放的对象
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
	typedef void (*FuncPtrFree)(void *pData);

	/*
	 *********************************************
	 Function : FuncPtrCompares
	 DateTime : 2012/4/24 10:31
	 Description :  比较函数定义
	 Input :  pData 比较值
	 Input : pKey 比较值
	 Output : 
	 Return :  如果 *pData == *pKey  返回 0, *pData > *pKey 返回 1， 其他返回-1
	 Note :   
	 *********************************************
	 */
	typedef int (*FuncPtrCompares)(const void *pData,const void *pKey);


	typedef enum 
	{
		ePRO_GSP = 0, // gsp 协议
		ePRO_RTSP = 1, // rtsp 协议
		ePRO_SIP = 2, // sip28181 协议
		ePRO_UNK = 100, //未知
	}EnumProtocol;

 //server gsp tcp 监听端口
	#define GSP_SERVER_DEFAULT_TCP_LISTEN_PORT    8443

	//server gsp udp 监听端口
	#define GSP_SERVER_DEFAULT_UDP_LISTEN_PORT	  8444	 

// rtsp tcp 监听端口
    #define RTSP_SERVER_DEFAULT_TCP_LISTEN_PORT    554 

	// rtp udp 端口范围最小值
	#define RTP_DEFAULT_UDP_PORT_RANGE_BEGIN    0  

	// rtp udp 端口范围最大值
	#define RTP_DEFAULT_UDP_PORT_RANGE_END    0 


	// SIP UDP 监听端口
	 #define SIP_SERVER_DEFAULT_UDP_LISTEN_PORT    5060



 //兼容用
#define GSP_API   

//兼容用
#define GSP_ASSERT_RET GS_ASSERT_RET 

	//兼容用  
#define GSP_ASSERT    GS_ASSERT   

	//兼容用
#define GSP_ASSERT_RET_VALUE    GS_ASSERT_RET_VAL   

	//兼容用
#define GSP_ASSERT_EXIT    GS_ASSERT_EXIT 

#define GSP_STACK_VERSION2  2
}  //end namespace GSP


#include "GSPStru.h"
#include "GSMediaDefs.h"

 //兼容使用
#define GSP_PRINTF(...) do{}while(0) 

#endif //end _GS_H_GSPCONFIG_H_
