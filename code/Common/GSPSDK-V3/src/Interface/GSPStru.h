#ifndef GSP_GSPSTRU_DEF_H
#define GSP_GSPSTRU_DEF_H
#include <GSType.h>



/*
******************************************
Copyright (C), 2010-2011, GOSUN
Filename : GSPSTRU.H
Author :  zouyx
Version : 0.1.0.0
Date: 2010/5/14 9:47
Description: 定义GSP 协议用到的一些机构和合命令声明
********************************************
*/

/*
****************************************
brief :  一下定义协议用的的一些结果
****************************************
*/

    namespace GSP
    {
//结构为单字节对齐
#ifdef _WIN32
#define ATTRIBUTE_PACKED 
#pragma pack( push,1 )
#else
#define ATTRIBUTE_PACKED    __attribute__ ((packed)) 
#endif


#define GSP_PACKET_MAX_LEN (512*1024)


/*
****************************************
brief : 命令ID的定义
****************************************
*/
typedef enum
{
    GSP_CMD_ID_NONE  = 0,


//请求链接, 
    GSP_CMD_ID_REQUEST,             //参数 StruGSPCmdRequest
    GSP_CMD_ID_RET_REQUEST,         //参数 StruGSPCmdRetRequest

//控制命令 
    GSP_CMD_ID_CTRL,                //参数  StruGSPCmdCtrl
    GSP_CMD_ID_RET_CTRL,            //参数  StruGSPCmdReturn

//Keepalive 命令
     GSP_CMD_ID_KEEPAVLIE = 5,      //参数  StruGSPCmdKeepalive
    //----该命令没回复



//断开链接
     GSP_CMD_ID_CLOSE,              //参数 StruGSPCmdReturn
     GSP_CMD_ID_RET_CLOSE,          //参数 StruGSPCmdReturn 

//播放(下载)完成命令
     GSP_CMD_ID_COMPLETE,           //参数  空
     GSP_CMD_ID_RET_COMPLETE,       //参数  StruGSPCmdReturn

//重传命令 
     GSP_CMD_ID_RESEND = 10,        //参数  StruGSPCmdResend
    //--回复通 参考 GSP 重传类型定义


//媒体参数信息命令 
     GSP_CMD_ID_MEDIAATTRI,         // 参数  StruMediaInfoTable

//请求回复播放的状态
     GSP_CMD_ID_REQUEST_STATUS,            // 参数  StruRequestStatus;
     GSP_CMD_ID_RET_STATUS,            // 参数 StruPlayStatus;
//非法命令回复
     GSP_CMD_ID_RET_UNKNOWN_COMMAND,         //如果回复未知命令  参数  StruGSPCmdReturn, 其中iErrno 表示不认识得命令号

     //请求流异常
     GSP_CMD_ID_STREAM_ASSERT = 15,       //参数  StruGSPCmdReturn,


     //出现异常，连接将被关闭
     GSP_CMD_ID_ASSERT_AND_CLOSE,       //参数  StruGSPCmdReturn, 不需要回复

	//使用UDP 进行流传输设计
	 GSP_UDP_SET_SETUP,				//参数  StruUdpSetupRequest, 需要回复
	 GSP_UDP_SET_SETUP_RESPONSE,    //参数  StruUdpSetupRespone, 需要回复

	 GSP_UDP_SEND_TEST_ADDR,			//请求UDP发送TEST_ADDR 命令,  空
	 GSP_UDP_SEND_TEST_ADDR_RESPONSE = 20,	//GSP_UDP_SEND_TEST_ADDR 回复,  参数   StruGSPCmdReturn

	 GSP_RESET_TRANS_ON_TCP,		     //要求从新使用TCP 进行流传输, 参数  空
	 GSP_RESET_TRANS_ON_TCP_RESPONSE,		//参数  StruGSPCmdReturn
	 
}EnumGSPCommandID;


/*
****************************************
brief : 媒体属性名定义
****************************************
*/
typedef enum
{
    GSP_MEDIA_ATTRI_NAME_NONE = 0,
    GSP_MEDIA_ATTRI_NAME_SECTION_BEGIN,
    GSP_MEDIA_ATTRI_NAME_SECTION_END,
    GSP_MEDIA_ATTRI_NAME_MEDIATYPE,  //媒体类型, 参考媒体类型, EnumGSMediaType定义
    GSP_MEDIA_ATTRI_NAME_SECTION_ID,    //子通道号
    GSP_MEDIA_ATTRI_NAME_CODE_ID,    //编码类型编号

    GSP_MEDIA_ATTRI_NAME_VIDEBEGIN = 30,
    GSP_MEDIA_ATTRI_NAME_VIDEO_WIDTH,      //视频流的图像的宽
    GSP_MEDIA_ATTRI_NAME_VIDEO_HEIGHT,     //视频流的图像的高
    GSP_MEDIA_ATTRI_NAME_VIDEO_FRAMERATE,  //视频流的帧率 , 高8位表示 整数部分 ,低8位表示 小数部分                                            
                                           //如  0x0c05 表示 12.5 帧

    GSP_MEDIA_ATTRI_NAME_VIDEO_BITRATE,    //视频流码率   单位  Kbytes

  


    GSP_MEDIA_ATTRI_NAME_AUDIOBEGIN =  100, 
    GSP_MEDIA_ATTRI_NAME_AUDIO_SAMPLE,    //采样频率     单位 KHz
    GSP_MEDIA_ATTRI_NAME_AUDIO_CHANNELS,  //音频的通道数
    GSP_MEDIA_ATTRI_NAME_AUDIO_BITS,   //音频的采样位数


    GSP_MEDIA_ATTRI_NAME_OSDBEGIN = 200,
    GSP_MEDIA_ATTRI_NAME_OSD_X,           //OSD叠加的坐标 X
    GSP_MEDIA_ATTRI_NAME_OSD_Y,             //OSD叠加的坐标 Y
    GSP_MEDIA_ATTRI_NAME_OSD_DATA_TYPE,         // OSD叠加 数据类型
    GSP_MEDIA_ATTRI_NAME_OSD_TRANSPARENCY,   //OSD 叠加的透明度  0~100  100为不透明


    GSP_MEDIA_ATTRI_NAME_BINARYBEGIN = 300,  //二进制编码
    GSP_MEDIA_ATTRI_NAME_BINARY_LEN,        //长的 1~16 位
    GSP_MEDIA_ATTRI_NAME_BINARY_LEN1,        //长的16~32位
    GSP_MEDIA_ATTRI_NAME_BINARY_LEN2,        //长的32~48位
    GSP_MEDIA_ATTRI_NAME_BINARY_LEN3,        //长的48~64位

    GSP_MEDIA_ATTRI_NAME_SYSHEADER_BEGIN = 400,  //系统头
    GSP_MEDIA_ATTRI_NAME_SYSHEADER_LEN,   //长的 1~16 位
    GSP_MEDIA_ATTRI_NAME_SYSHEADER_LEN1,   //长的16~32位

}EnumGSPMediaAttrName;




/*
****************************************
brief : GSP 重传类型定义
****************************************
*/
const INT GSP_RESEND_TYPE_MEDIA_INFO   =   1;   //重传 媒体信息, 
//iSubChn, iArgs1,iArgs2, iArgs3 参数不使用
// 返回  GSP_CMD_ID_MEDIAATTRI
const INT GSP_RESEND_TYPE_STREAM_SYS_INFO  =  2;   //重传 流的系统头，有些流存在系统头
//iSubChn, iArgs1,iArgs2, iArgs3 参数不使用
// 返回 按正常的流媒体包发送

const INT  GSP_RESEND_TYPE_PACKET        =     3;   //重传 数据包, 
//iSubChn   对应包的子通道,参考包头定义
//iArgs1,   对应包的Seq 号,参考包头定义
//iArgs2    对应包的SSeq 开始号, 不包括自身,参考包头定义
//iArgs3    对应包的SSeq 结束号,, 不包括自身,参考包头定义      iArgs2<SSeq<iArgs3
// 返回 按正常的包发送



/*
****************************************
brief : 效验方式定义
****************************************
*/

#define CRC_TYPE_NONE 0       //无效验
#define CRC_TYPE_CRC16 0x01  //crc16
#define CRC_TYPE_CRC32 0x02   //crc32
#define CRC_TYPE_MD5   0x03   // md5

/*
****************************************
brief : 当前GSP版本号定义
****************************************
*/
const INT  GSP_VERSION =2;
const INT  GSP_VERSION_V1=0;

/*
****************************************
brief : GSP 包数据类型定义
****************************************
*/
const INT  GSP_PACKET_TYPE_CMD  = 0;   //负载为命令
const INT  GSP_PACKET_TYPE_STREAM = 1; //负载为流

/*
****************************************
brief : TSP Keepavlie 参数定义
****************************************
*/
const INT  KEEPALIVE_ARGS_NONE     = 0; //为没有意义， 
const INT  KEEPALIVE_ARGS_REQUEST  = 1; //表示 接受者尽快发送Keepavlie

/*
****************************************
brief : GSP传输类型定义
****************************************
*/
const INT  GSP_TRAN_RTPLAY       = 0; //实时预览
const INT  GSP_TRAN_REPLAY      =  1;  //文件回放
const INT  GSP_TRAN_DOWNLOAD    =  2;  //下载方式  

/*
****************************************
brief :  GSP 协议错误号定义
****************************************
*/

const INT  GSP_PRO_RET_SUCCES      =   0;  //成功
const INT  GSP_PRO_RET_EUNKNOWN    =   1;  //未知错误
const INT  GSP_PRO_RET_EPERMIT     =   2;  //没有权限
const INT  GSP_PRO_RET_ENCHN       =   3;  //请求通道不存在
const INT  GSP_PRO_RET_EVER        =   4;  //不支持请求的版本
const INT  GSP_PRO_RET_ECODE       =   5;  //不支持请求的编码格式
const INT  GSP_PRO_RET_ECLOSE      =   6;  //联接已经被关闭
const INT  GSP_PRO_RET_EINVALID    =   7;  //非法参数
const INT  GSP_PRO_RET_EEND       =    8;  //流已经结束
const INT  GSP_PRO_RET_ENEXIST      =  9;  //不存在的请求资源
const INT  GSP_PRO_RET_EPRO         =   10; //非法协议
const INT   GSP_PRO_RET_ECMD        =    11; //非法命令
const INT  GSP_PRO_RET_EILLEGAL     =   12;//逻辑错误
const INT  GSP_PRO_RET_EEXIST      =    13; //请求资源已经在打开
const INT  GSP_PRO_RET_EASSERT     =    14;  //操作异常
const INT  GSP_PRO_RET_EIO         =    15;  //输入输出错误
const INT  GSP_PRO_RET_EBUSY       =     16;  //服务器资源缺乏
const INT  GSP_PRO_RET_EPACKET     =     17;  //收到非法数据包
const INT  GSP_PRO_RET_EKP        =     18;      // Keepalive 超时
const INT  GSP_PRO_RET_ESTREAM_ASSERT = 19;    //流异常






/*
****************************************
brief :    GSP 控制能力定义
****************************************
*/

//iSubChn  0 表示控制所有通道 ,其他表示控制对应的子通道号
//iArgs1  表示是否由关键帧开始播放 TRUE/FALSE
//iArgs2  不使用
const INT  GSP_CTRL_PLAY   =   (1<<0);    //播放  


//iSubChn  0 表示控制所有通道,其他表示控制对应的子通道号
//iArgs1  不使用
//iArgs2 不使用
const INT  GSP_CTRL_STOP   =   (1<<1);    //停止 

//iSubChn  0 表示控制所有通道,其他表示控制对应的子通道号
//iArgs1  不使用
//iArgs2 不使用
const INT  GSP_CTRL_PAUSE   =  (1<<2);     //暂停

//iSubChn  0 表示控制所有通道,其他表示控制对应的子通道号
//iArgs1  快进的倍数 范围 1~32
//iArgs2  不使用
const INT  GSP_CTRL_FAST   =   (1<<3);     //快进

//iSubChn  0 表示控制所有通道,其他表示控制对应的子通道号
//iArgs1  快进的倍数 范围 1~32
//iArgs2  不使用
const INT  GSP_CTRL_BFAST   =  (1<<4);     //快退

//iSubChn  0 表示控制所有通道,其他表示控制对应的子通道号
//iArgs1  不使用
//iArgs2 不使用
const INT  GSP_CTRL_STEP   =   (1<<5) ;    //单帧前

//iSubChn  0 表示控制所有通道,其他表示控制对应的子通道号
//iArgs1  不使用
//iArgs2 不使用
const INT  GSP_CTRL_BSTEP   =  (1<<6) ;    //单帧后

//iSubChn  0 表示控制所有通道,其他表示控制对应的子通道号
//iArgs1  快进的倍数 范围 1~32
//iArgs2  不使用
const INT  GSP_CTRL_SLOW   =   (1<<7);     //慢进

//iSubChn  0 表示控制所有通道,其他表示控制对应的子通道号
//iArgs1  快进的倍数 范围 1~32
//iArgs2  不使用
const INT  GSP_CTRL_BSLOW   =  (1<<8) ;    //慢退

//iSubChn  0 表示控制所有通道,其他表示控制对应的子通道号
//iArgs1  表示偏移类型 (参考 GSP 偏移类型定义)
//iArgs2  偏移的值
const INT  GSP_CTRL_SETPOINT = (1<<9)  ;   //拖动

//iSubChn  0 表示控制所有通道,其他表示控制对应的子通道号
//iArgs1  图片数
//iArgs2  时间间隔（秒数）
const INT  GSP_CTRL_SECTION   = (1<<10) ;    //切片

//iSubChn  0 表示控制所有通道,其他表示控制对应的子通道号
//iArgs1   TRUE 表示开始， FALSE 表示结束
//iArgs2 不使用
const INT  GSP_CTRL_FLOWCTRL   = (1<<11) ;    //进行流控




/*
****************************************
brief : GSP 偏移类型定义
****************************************
*/
const INT  GSP_OFFSET_TYPE_BYTES  = 0 ; //偏移文件开头的字节数
const INT  GSP_OFFSET_TYPE_SECS  = 1 ; //偏移文件开头的秒数
const INT  GSP_OFFSET_TYPE_RATE  = 2 ; //偏移的万分比


/*
****************************************
brief :  GSP 协议 过滤类型定义
****************************************
*/
const INT  GSP_PRO_FILTER_TYPE_NKEY  = 1;  //过滤非关键帧



/*
*********************************************************************
*
*@brief : GSP 协议流传输方式
*
*********************************************************************
*/
const INT GSP_STREAM_TRANS_MODE_MULTI_TCP = 0x00;    //复合流， 在GSP信令上复合传输流
const INT GSP_STREAM_TRANS_MODE_RTP_UDP = 0x01;		//使用RTP/UDP 传输
const INT GSP_STREAM_TRANS_MODE_TCP = 0x02;			//使用独立单向TCP 传输

/*
****************************************
brief :  定义GSP通信的头格式
****************************************
*/
typedef struct _StruGSPPacketHeader
{

    UINT32 iLen : 24;   //整个包长度, 等于 头部+Playload+效验
    UINT32 iVersion : 3;   //GSP 版本号
    UINT32 iDataType : 1;  //数据类型 （参考 GSP 包数据类型定义)
    UINT32 iCRC : 3;       //效验方式 ( 参考效验方式定义 )
    UINT32 bEnd : 1;       //是否为结束包
    UINT16 iSeq;            //数据包的索引号
    UINT16 iSSeq;          //分包的索引要，由0开始
    UINT8  iSubChn;        // 会话的子通道号,有1开始，命令子通道号指定为1, 非法通道
    UINT8  iExtraVal;     //附加值，可以供具体的内容实用
    unsigned char cPlayload[2]; //负载占位,定义为2是为了字节对齐，
    //真正的头长度由 GSP_PACKET_HEADER_LEN获取
} ATTRIBUTE_PACKED StruGSPPacketHeader;

/*
****************************************
brief : 定义协议头的长度
****************************************
*/
#define GSP_PACKET_HEADER_LEN  (sizeof(StruGSPPacketHeader)-2)


/*
****************************************
brief :  媒体信息列表结构
****************************************
*/
typedef struct _StruMediaAttribute
{
    UINT16 iNname; //属性名 , 参考 enumGSPMediaAttrName
    UINT16 iValue; //属性值
} ATTRIBUTE_PACKED StruMediaAttribute;


typedef struct _struMediaInfoTable
{
    UINT32 iRows;  //指明 aRows的个数 
    StruMediaAttribute aRows[1];
} ATTRIBUTE_PACKED StruMediaInfoTable;

static INLINE UINT32 GetMediaInfoTableSize( const StruMediaInfoTable *struTablePtr )
{
UINT32 iRet;
    if( !struTablePtr )
    {
        return 0;
    }
    iRet = struTablePtr->iRows*sizeof(StruMediaAttribute);
    return iRet+4;
}

/*
****************************************
brief : 定义命令格式
****************************************
*/
typedef struct _StruGSPCommand
{      
    UINT32 iCmdID; //命令ID   （ 参考 命令ID的定义 )
    UINT32 iTag;   //识别号，发送命令者填充， 命令返回者原样返回
    unsigned char cPlayload[4];  //真正的命令内容
} ATTRIBUTE_PACKED StruGSPCommand;

#define GSP_PRO_COMMAND_HEADER_LENGTH (sizeof(StruGSPCommand)-4)

/*
****************************************
brief : 通用的回复命令定义
****************************************
*/
typedef struct _StruGSPCmdReturn
{    
    UINT32 iErrno;    //操作返回  (参考 GSP 协议错误号定义 )
} ATTRIBUTE_PACKED StruGSPCmdReturn;



/*
****************************************
brief : 点流命令定义
****************************************
*/
#define GSP_MAGIC  ('G'<<24|'s'<<16|'p'<<8|0xa3)
#define MAX_URI_SIZE  128




//请求命令
typedef struct _StruGSPCmdRequest
{
    UINT32 iMagic;    //魔术字，等于 GSP_MAGIC，如果不等将判断为非法链接
    UINT8 iTransMode; //传输模式 （GSP 协议流传输方式)
	UINT8 iTransType; //传输类型 （参考 GSP传输类型定义)   
	UINT16 iClientPort; // RTP_UDP/TCP 模式时的客户端的UDP端口/TCP 客户端端口
	unsigned char czClientIP[66];  // RTP_UDP/TCP 模式时的客户端的IP
	UINT16 iURIBufLen; 
    unsigned char szURI[4];  //资源定位  长度由 iURIBufLen 定义
    //StruMediaInfoTable *stStreamAttri; //请求流的属性
} ATTRIBUTE_PACKED StruGSPCmdRequest;

//设置 struGSPCmdRequest 动态成员 stStreamAttri 值的宏

#define GSPCMDREQUEST_STREAMATTRI(x)  ((StruMediaInfoTable*)((unsigned char *)(x)+8+68+(x)->iURIBufLen))

#define GSPCMDREQUEST_BASE_SIZE  (8+68)


//回复命令
typedef struct _StruGSPCmdRetRequest
{
    UINT32 iMagic;    //魔术字，等于 GSP_MAGIC，如果不等将判断为非法链接  
    UINT16 iErrno;      //操作返回  (参考 GSP 协议错误号定义 )
    UINT16 iKeepaliveTimeout; // keepalive 超时定义 ， 单位 秒
    UINT32 iAbilities; //能力标准， 有为表示， （ 参考 GSP 控制能力定义)
	UINT8 iTransType; //传输类型 （参考 GSP传输类型定义)  	
	UINT8 iTransMode;  //传输模式 （GSP 协议流传输方式)
    unsigned char czReserver[5];  //保留
	UINT8  iRtpPlayloadType;  //当使用 RTP_UDP 模式时的RTP 使用的PT
	UINT32 iRtpSSRC;  //当使用 RTP_UDP 模式时的RTP 使用的SSRC
	UINT16 iServerPort; // RTP_UDP/TCP 模式时的服务器的UDP端口/TCP 客户端端口
	unsigned char czServerIP[66];  // RTP_UDP/TCP 模式时的服务器的IP	
    StruMediaInfoTable stStreamAttri;  //流的属性 可以为NULL
} ATTRIBUTE_PACKED StruGSPCmdRetRequest;

/*
****************************************
brief : 断开命令定义
****************************************
*/
//发送命令， 命令体为空 

//回复命令 ,命令体为 StruGSPCmdReturn


/*
****************************************
brief : 控制命令定义
****************************************
*/

//发送
typedef struct _StruGSPCmdCtrl
{
    UINT32 iCtrlID; //控制命令 （ 参考 GSP 控制能力定义)
    UINT16 iSubChn; //指定控制的子通道， 0 表示所有通道
    INT16 iArgs1;   //控制的参数 1 ,和控制命令相关 （ 参考 GSP 控制能力定义)
    INT32 iArgs2;   //控制的参数 2 ，和控制命令相关  （ 参考 GSP 控制能力定义)


} ATTRIBUTE_PACKED StruGSPCmdCtrl;

//回复命令 ,命令体为 StruGSPCmdReturn


/*
****************************************
brief : Keepalive 命令定义
****************************************
*/

typedef struct _StruGSPCmdKeepalive
{
    UINT32 iMagic; //魔术字，等于 GSP_MAGIC，如果不等将判断为非法链接
    UINT32 iArgs;  //参数，（参考 GSP Keepavlie 参数定义) 
} ATTRIBUTE_PACKED StruGSPCmdKeepalive;

/*
****************************************
brief : 重传命令定义
****************************************
*/

//请求命令
typedef struct _StruGSPCmdResend
{
    UINT16 iType;   //请求的重传类型  ( 参考 GSP 重传类型定义)
    UINT16 iSubChn; //指定控制的子通道， 0 表示所有通道
    UINT16 iArgs1;  //参数，和类型相关， ( 参考 GSP 重传类型定义)
    UINT16 iArgs2;  //参数，和类型相关， ( 参考 GSP 重传类型定义)
    UINT16 iArgs3;   //参数，和类型相关， ( 参考 GSP 重传类型定义)
} ATTRIBUTE_PACKED StruGSPCmdResend;

//回复，按相关类型正常发送


/*
****************************************
brief : 媒体参数信息命令定义
****************************************
*/
//命令体为 StruMediaAttribute

/*
****************************************
brief :   GSP 状态定义
****************************************
*/
const INT   GSP_STATUS_READY =  0; //准备就绪
const INT   GSP_STATUS_NORMAL  = 1; //正常播放
const INT   GSP_STATUS_PAUSE   = 2; //暂停
const INT   GSP_STATUS_FAST   =  3;  //快进
const INT   GSP_STATUS_SLOW    = 4;  //慢放
const INT   GSP_STATUS_BFAST  =  5;   //后快进
const INT   GSP_STATUS_BSLOW  =  6 ;  //后慢放
const INT   GSP_STATUS_STEP    = 7;  //单帧前进
const INT   GSP_STATUS_BSTEP  =  8 ; //单帧后退


/*
****************************************
brief :   GSP 状态请求定义
****************************************
*/
const INT   GSP_REQUEST_ONE      =   1;  //只返回一次  , iArgs无意义
const INT   GSP_REQUEST_INTERVAL  =  2 ; //间隔自动返回 , iArgs 参数为间隔的秒数 , 0 表示自动
const INT   GSP_REQUEST_STOP      =  3 ; //停止自动返回状态, iArgs无意义

typedef struct _StruRequestStatus
{
   UINT16 iRequest;  //请求方式, 参考GSP 状态请求定义
   UINT16 iArg;      //参考GSP 状态请求定义
} ATTRIBUTE_PACKED StruRequestStatus;

typedef struct _StruPlayStatus
{
   UINT8 iStatus;  //状态, 参考 GSP 状态定义 
   UINT8 iSpeed;
   UINT16 iPosition; //当前位置， 万分比
} ATTRIBUTE_PACKED StruPlayStatus;



#define MAX_UDP_PACKET_CHANNEL 200
//重传请求 
#define UDP_CHANNEL_REQUEST_RESEND  (MAX_UDP_PACKET_CHANNEL+1)  
	//设置 UDP 地址 内容为0
#define UDP_CHANNEL_TEST_ADDR		(MAX_UDP_PACKET_CHANNEL+2)	
//UDP 端口洞穿 内容为0
#define UDP_CHANNEL_KEEPALIVE_PORT  (MAX_UDP_PACKET_CHANNEL+3)   
 //UDP 离开
#define UDP_CHANNEL_BYE				(MAX_UDP_PACKET_CHANNEL+4)     


typedef struct _StruUdpResendRequest
{
	UINT16 iSeq;
	UINT16 iSSeq;
	UINT16 iCounts;
} ATTRIBUTE_PACKED StruUdpResendRequest;


#define UDP_TRANSPORT_ON_RTP 0x00000001

typedef struct _StruUdpSetup
{	
	 //内容为SDP
	INT32 iContentLen;
	BYTE m_vContent[1];
} ATTRIBUTE_PACKED StruUdpSetupRequest;

typedef struct _StruUdpSetupRespone
{
	//内容为SDP
	INT32 iErrno;
	INT32 iContentLen;
	BYTE m_vContent[1];
} ATTRIBUTE_PACKED StruUdpSetupRespone;


#ifdef _WIN32
#pragma pack( pop )
#endif


}



using namespace GSP;

#endif
