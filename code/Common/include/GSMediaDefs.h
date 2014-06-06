/*
******************************************
Copyright (C), 2010-2011, GOSUN
Filename : GSMEDIADEFS.H
Author :  zouyx
Version : 0.1.0.0
Date: 2010/7/22 9:15
Description: 定义多媒体用到的一些定义
********************************************
*/
#ifndef GSP_GSMEDIADEFS_DEF_H
#define GSP_GSMEDIADEFS_DEF_H

#include <GSCommon.h>

//结构为单字节对齐
#ifdef _WIN32
#define GS_MEDIA_ATTRIBUTE_PACKED 
#pragma pack( push,1 )
#else
#define GS_MEDIA_ATTRIBUTE_PACKED    __attribute__ ((packed)) 
#endif


/*
****************************************
brief : 定义编码类型
****************************************
*/
typedef enum
{
    GS_CODEID_NONE = 0, //无编码类型
    GS_CODEID_BINARY,   //二进制编码

	GS_CODEID_GS_PS,	// GOSUN PS 动态码流


    //标准视频编码 
    GS_CODEID_ST_MP4        = 0x00000010,       //标准的 MP4 编码
    GS_CODEID_ST_H264,                          //标准的 H264 编码
	GS_CODEID_ST_SVAC,                           //标准的 SVAC 编码
 
    //标准音频编码
    GS_CODEID_ST_MP3        = 0x00000300,       //标准MP4
	GS_CODEID_AUDIO_ST_PCM,	
	GS_CODEID_AUDIO_ST_G711A,
	GS_CODEID_AUDIO_ST_G711U,
	GS_CODEID_AUDIO_ST_G721,
	GS_CODEID_AUDIO_ST_G722,
	GS_CODEID_AUDIO_ST_G723,
	GS_CODEID_AUDIO_ST_G729,
	GS_CODEID_AUDIO_ST_H263,
	GS_CODEID_AUDIO_ST_SVAC,

	GS_CODEID_PS = 0x00000400,


    //标准图像编码
    GS_CODEID_ST_JPEG       = 0x00000600,         //标准JPG
    GS_CODEID_ST_BMP,                             //标准JPG
    GS_CODEID_ST_YUV420P,                         //标准YUV420P


    //按厂商进行的特殊编码



    //=====高新新兴
    GS_CODEID_GS_V2160I     = 0x00001000,       // GOSUN 2160I 视频编码
    GS_CODEID_GS_A2160I,                        // GOSUN 2160I 音频编码
    GS_CODEID_GS_V462C,                         //GOSUN BASS462C 视频编码
    GS_CODEID_GS_A462C,                         //GOSUN BASS462C 音频编码
    GS_CODEID_GS_V461C,                         //GOSUN BASS461C 视频编码
    GS_CODEID_GS_A461C,                         //GOSUN BASS461C 音频编码
	GS_CODEID_GS_V461A,                         //GOSUN BASS461A 视频编码
	GS_CODEID_GS_V2160IV,						// GOSUN 2160I_V 视频编码
	GS_CODEID_GS_A2160IV,                       // GOSUN 2160I_V 音频编码
	GS_CODEID_GS_VIPC,							// GOSUN IPC	 视频编码
	GS_CODEID_GS_AIPC,							// GOSUN IPC	 音频编码
	

    //===== 海狮 
    GS_CODEID_HI_VDEFAULT   = 0x00001200,        //海狮的视频通用编码
    GS_CODEID_HI_ADEFAULT,                      //海狮的音频通用编码
    GS_CODEID_HI_VH264,                         //海狮 H264
    GS_CODEID_HI_VMP4,                          //海狮 H264
    GS_CODEID_HI_COMPLEX,                          //海狮复合通用编码


    //===== 海康
    GS_CODEID_HK_VDEFAULT    = 0x00001300,      //海康的视频通用编码
    GS_CODEID_HK_ADEFAULT,                      //海康的音频通用编码
    GS_CODEID_HK_VMP4,                          //海康 MP4
    GS_CODEID_HK_VH264,                         //海康 H264
    GS_CODEID_HK_COMPLEX,                       //海康复合通用编码




    //===== 大华
    GS_CODEID_DH_VDEFAULT    = 0x00001400,      //大华的视频通用编码
    GS_CODEID_DH_ADEFAULT ,                     //大华的音频通用编码
    GS_CODEID_DH_VMP4,                          //大华 MP4
    GS_CODEID_DH_VH264,                         //大华 H264
    GS_CODEID_DH_COMPLEX,                       //大华复合通用编码

    //===== 恒益
    GS_CODEID_HY_VDEFAULT   = 0x00001500,       //恒益的视频通用编码
    GS_CODEID_HY_ADEFAULT,                      //恒益的音频通用编码
    GS_CODEID_HY_VMP4,                          //恒益 MP4
    GS_CODEID_HY_VH264,                         //恒益 H264
    GS_CODEID_HY_COMPLEX,                       //恒益复合通用编码


	//=====手机
	GS_CODEID_WM_XVID		= 0x00001600,		//手机客户端解码1,谭志添加测试

	// ===== 黄河
	GS_COIDEID_HH_VDEFULT   = 0x00001700,       //黄河的视频通用编码

	// ===== 中本
	GS_CODEID_ZBEN_VDEFULT  = 0x00001800,       //中本的视频编码
	GS_CODEID_ZBEN_ADEFULT,                     //中本的音频编码

	// ===== 高凯视
	GS_CODEID_CALSYS_VDEFULT = 0x00001900,         //高凯视的视频通用编码 
	GS_CODEID_CALSYS_ADEFULT,                      //高凯视的音频通用编码

    // ==== 安联锐视
	GS_COIDEID_ALRS_VDEFULT   = 0x00002000,       //安联锐视的视频通用编码

	// ====  讯美
    GS_CODEID_XM_VDEFULT   = 0x00002100,          //讯美视频通用编码

   // ====  高凯视GS76xx系列
    GS_CODEID_CALSYSGS76XX_VDEFULT = 0x00002200,         //高凯视GS76XX的视频通用编码 

    GS_CODEID_END,   //放在结尾 

}EnumGSCodeID;








/*
****************************************
brief : 定义媒体类型
****************************************
*/
typedef enum 
{
    GS_MEDIA_TYPE_NONE = 0,           //未知媒体类型
    GS_MEDIA_TYPE_VIDEO = 0x01,   //视频流, 属性描述  StruVideoDescri
    GS_MEDIA_TYPE_AUDIO,            //音频流, 属性描述  StruAudioDescri
    GS_MEDIA_TYPE_PICTURE,          //图片数据,属性描述 StruPictureDescri
    GS_MEDIA_TYPE_OSD,              //OSD 数据,属性描述 StruOSDDescri
    GS_MEDIA_TYPE_BINARY,          //二进制流, 属性描述  StruBinaryDescri   
	GS_MEDIA_TYPE_PROGRESS,        //进度，属性描述 StruPlayStatus
	GS_MEDIA_TYPE_RECORD_INFO,     //录像信息，属性描述 GSStruRecordInfo
	GS_MEDIA_TYPE_IFRAME_INDEX,   //I帧索引， 属性为二进制数， 结构参考 GMFDefs.h 定义
	GS_MEDIA_TYPE_FILE_TO_END,    //文件结束

	GS_MEDIA_TYPE_USER_END,		   //无用信息， 用以统计
    GS_MEDIA_TYPE_SYSHEADER = 0x40,         //信息头
}EnumGSMediaType;


/*
****************************************
brief :  录像信息描述
****************************************
*/
typedef struct _GSStruRecordInfo
{
	UINT32 iType;              // 1开始 0结束
	unsigned char szTime[16];  // 时间: 20111013194000 
}GS_MEDIA_ATTRIBUTE_PACKED GSStruRecordInfo;
#define TYPE_RECORD_END   0
#define TYPE_RECORD_START 1

/*
****************************************
brief :  视频流信息描述
****************************************
*/

typedef struct _StruVideoDescri
{
    UINT32 eCodeID;  //编码类型   EnumGSCodeID
    UINT16 iWidth;         //图像宽         0 表示无效
    UINT16 iHeight;       //图像高          0 表示无效
    UINT8 iFrameRate;    //帧率 整数部分    0 表示无效
    UINT8 iFrameRate2;    //帧率 小数部分  
    UINT16 iReserver;      //保留字
}GS_MEDIA_ATTRIBUTE_PACKED StruVideoDescri;

/*
****************************************
brief :  音频流信息描述
****************************************
*/
typedef struct _StruAudioDescri
{
    UINT32 eCodeID;    //编码类型   EnumGSCodeID
    UINT32 iSample;    //采用频率 Hz         0 表示无效
    UINT16 iBits;      //采样位数  0 表示无效
    UINT16 iChannels;  //通道数     0 表示无效
}GS_MEDIA_ATTRIBUTE_PACKED StruAudioDescri;

/*
****************************************
brief : 图片信息描述
****************************************
*/
typedef struct _StruPictureDescri
{
    UINT32 eCodeID;    //编码类型   EnumGSCodeID
}GS_MEDIA_ATTRIBUTE_PACKED StruPictureDescri;


/*
****************************************
brief :  OSD信息描述
****************************************
*/
//OSD 的数据类型  是字符串
#define GS_OSD_TYPE_STRING  0x01   
//OSD 的数据类型  是RGB数据
#define GS_OSD_TYPE_RBG     0x02   

typedef struct _StruOSDDescri
{
    UINT16 iPosX; //开始的X坐标, 相对于图片左上角
    UINT16 iPosY; //开始的Y坐标, 相对于图片左上角
    UINT8 iDataType; //数据类型     参考 GS_OSD_TYPE_xxx
    UINT8 iTransparency; //透明度  0~100    0 表示完全透明， 100 表示完全不透明
}GS_MEDIA_ATTRIBUTE_PACKED StruOSDDescri;

/*
****************************************
brief : 二进制数据描述
****************************************
*/
typedef struct _StruBinaryDescri
{
    UINT64 iSize;  //整个二进制数的长度， 0 表示无限长
}GS_MEDIA_ATTRIBUTE_PACKED StruBinaryDescri;

/*
****************************************
brief : 信息头数据描述
****************************************
*/
typedef struct _StruSysHeader
{
    UINT32 iSize; //表示信息头的长度， 0 表示未知 
}GS_MEDIA_ATTRIBUTE_PACKED StruSysHeaderDescri;

/*
****************************************
brief : 媒体描述封装
****************************************
*/
typedef union 
{
    StruVideoDescri struVideo;
    StruAudioDescri struAudio;
    StruPictureDescri struPicture;
    StruOSDDescri struOsd;
    StruBinaryDescri struBinary;
    StruSysHeaderDescri struSysHeader;
}UnionMediaDescri;


/*
****************************************
brief :  多媒体描述封装
****************************************
*/
typedef struct _StruGSMediaDescri
{
    UINT32  eMediaType;  //媒体类型 EnumGSMediaType
    UnionMediaDescri unDescri;  //媒体描述
}GS_MEDIA_ATTRIBUTE_PACKED StruGSMediaDescri;


/*
****************************************
brief :  传输媒体、存储帧格式封装
**************************************** 
长度(3Bytes)|是否关键帧(1bytes)|媒体类型(7bytes)|时戳（4Bytes)| 数据... 
*/
#define INVALID_TIME_STAMP ((UINT32)-1)
#define GS_FRAME_HEADER_MAGIC  (('G'<<24) | ('s'<<16) | ('c'<<8) | 'N') 
   
typedef struct _StruGSFrameHeader
{
    UINT32 iMagic;        //魔术字， 用于标识每帧的开头
    UINT32 iLenght : 24;  //数据长度， 不包括本包头 , 指 aFrameData长度
    UINT32 bKey : 1;      //是否是关键帧
    UINT32 eMediaType : 7;  //媒体类型
    UINT32 iTimeStamp;      //时间戳   (-1) 为无效时间戳 INVALID_TIME_STAMP // (从1970年1月1日0时0分0秒到此时的秒数) modified by jiangsx 20121213
    //   unsigned char aFrameData[];  //帧数据
}GS_MEDIA_ATTRIBUTE_PACKED StruGSFrameHeader;

/*
*********************************************************************
*
*@brief : GS_CODEID_GS_PS 流封装的帧头
*
*********************************************************************
*/
typedef struct _StruGSPSFrameHeader
{
#define GSPSFRAMEHEADER_MAGIC  0x10000000
#define GSPSFRAMEHEADER_MARK1  (('G'<<0) | ('S'<<8))
#define GSPSFRAMEHEADER_MARK2  (('P'<<0) | ('S'<<8))

#define GSPS_CODETYPE_NONE 0x00


#define GSPS_CODETYPE_V_H264	0x1B
#define GSPS_CODETYPE_V_MP4		0x10
#define GSPS_CODETYPE_V_SVAC	0x80

#define GSPS_CODETYPE_A_G711	0X90
#define GSPS_CODETYPE_A_G722	0X92
#define GSPS_CODETYPE_A_G723 0X93
#define GSPS_CODETYPE_A_G729 0X99
#define GSPS_CODETYPE_A_SVAC 0X9B

	//等于 GSPSFRAMEHEADER_MAGIC 
	UINT32 iMagic;
	 // 等于 0xFFFF
	UINT16 iLabel;
	 //等  GSPSFRAMEHEADER_MARK1
	UINT16 iGSMark1;
	//等   GSPSFRAMEHEADER_MARK2
	UINT16 iGSMark2; 
	//编码类型, 参考 GB28181 PS 封装定义
	UINT8  iCodeType; 
	//等于 是否关键帧
	UINT8  bKey : 1;  
	//等于 0xF7	
	UINT8  iReserver : 7;  
	//
	UINT64 iTimestamp;
} GS_MEDIA_ATTRIBUTE_PACKED StruGSPSHeader;

#define GSPSHEADER_INIT(h, streamtype, xbKey) \
do { \
 (h).iMagic = GSPSFRAMEHEADER_MAGIC; \
 (h).iLabel = 0xFFFF; \
 (h).iGSMark1 = GSPSFRAMEHEADER_MARK1; \
 (h).iGSMark2 = GSPSFRAMEHEADER_MARK2; \
 (h).iCodeType = (streamtype); \
 (h).bKey = ((xbKey) ? 1 : 0); \
 (h).iReserver = 0xF7; (h).iTimestamp=(UINT64)-1; }while(0)

#define IS_GSPSHEADER(h) \
	((h).iMagic==GSPSFRAMEHEADER_MAGIC && (h).iLabel==0xFFFF && \
	 (h).iGSMark1==GSPSFRAMEHEADER_MARK1 && (h).iGSMark2==GSPSFRAMEHEADER_MARK2)

#ifdef _WIN32
#pragma pack( pop )
#endif


INLINE static  EnumGSMediaType CodeID2MediaType( EnumGSCodeID codeID)
{
    switch( codeID)
    {
    case GS_CODEID_NONE :
        return GS_MEDIA_TYPE_NONE;
        break;
    case GS_CODEID_BINARY :
        return GS_MEDIA_TYPE_BINARY;
        break;

        //视频编码
    case GS_CODEID_GS_V2160I:
    case GS_CODEID_GS_V462C:
    case GS_CODEID_GS_V461C:
	case GS_CODEID_GS_V461A:
	case GS_CODEID_GS_V2160IV:
	case GS_CODEID_GS_VIPC:

    case GS_CODEID_HI_VDEFAULT:
    case GS_CODEID_HI_VMP4:
    case GS_CODEID_HI_VH264:
    case GS_CODEID_HI_COMPLEX :

    case GS_CODEID_HK_VDEFAULT:
    case GS_CODEID_HK_VMP4:
    case GS_CODEID_HK_VH264:
    case GS_CODEID_HK_COMPLEX :

    case GS_CODEID_DH_VDEFAULT:
    case GS_CODEID_DH_VMP4:
    case GS_CODEID_DH_VH264:
    case GS_CODEID_DH_COMPLEX :

    case GS_CODEID_HY_VDEFAULT:
    case GS_CODEID_HY_VMP4:
    case GS_CODEID_HY_VH264:
    case GS_CODEID_HY_COMPLEX :

	case GS_COIDEID_HH_VDEFULT :

	case GS_CODEID_ZBEN_VDEFULT:

	case GS_CODEID_CALSYS_VDEFULT:

	case GS_COIDEID_ALRS_VDEFULT:

	case GS_CODEID_XM_VDEFULT:

	case GS_CODEID_PS:
	case GS_CODEID_GS_PS:

        return GS_MEDIA_TYPE_VIDEO;
        break;

        //音频编码
    case GS_CODEID_GS_A2160I:
    case GS_CODEID_GS_A462C:
    case GS_CODEID_GS_A461C:
    case GS_CODEID_GS_A2160IV:
	case GS_CODEID_GS_AIPC:

    case GS_CODEID_HI_ADEFAULT: 

    case GS_CODEID_HK_ADEFAULT :

    case GS_CODEID_DH_ADEFAULT :

    case GS_CODEID_HY_ADEFAULT :

	case GS_CODEID_ZBEN_ADEFULT:

	case GS_CODEID_CALSYS_ADEFULT:


        return GS_MEDIA_TYPE_AUDIO;
        break;

    default :
        if( codeID>=GS_CODEID_ST_MP4 && codeID<GS_CODEID_ST_MP3)
        {
            return GS_MEDIA_TYPE_VIDEO;
        }
        else if( codeID>=GS_CODEID_ST_MP3 && codeID<GS_CODEID_ST_JPEG )
        {
            return GS_MEDIA_TYPE_AUDIO;
        }
        else if( codeID>=GS_CODEID_ST_JPEG && codeID<GS_CODEID_GS_V2160I )
        {
            return GS_MEDIA_TYPE_PICTURE;
        }
        break;
    }
    return GS_MEDIA_TYPE_NONE;
}


#endif
