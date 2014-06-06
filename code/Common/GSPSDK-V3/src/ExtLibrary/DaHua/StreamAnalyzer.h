/*************************************************************************
**                           streamAnalyzer 

**      (c) Copyright 1992-2004, ZheJiang Dahua Technology Stock Co.Ltd.
**                         All Rights Reserved
**
**	File  Name		: StreamAnalyzer.h
**	Description		: streamAnalyzer header
**	Modification	: 2010/07/13	yeym	Create the file
**************************************************************************/
#ifndef __STREAM_ANALYZER_H
#define __STREAM_ANALYZER_H
#define INOUT
#define IN
#define OUT


#ifndef _WIN32
#include <unistd.h>
#include <time.h>
#define __stdcall
#else
#include <windows.h>
#endif

#ifndef NULL
#define NULL
#endif

#ifdef SUPPORT_TYPE_DEFINE
#include "platform.h"
#include "platformsdk.h"
#else

#if !defined(uint8)
typedef unsigned char       uint8;
#endif
#if !defined(uint16)
typedef unsigned short     uint16;
#endif
#if !defined(uint32)
typedef unsigned int        uint32;
#endif
#if !defined(int32)
typedef int                 int32;
#endif
#if !defined(__int64) && !defined(_WIN32)
typedef long long                 __int64;
#endif
#endif



#ifdef WIN32
#ifdef STREAMANALYZER_EXPORTS
#define ANALYZER_API __declspec(dllexport)

//#define SUPPORT_AES
#else
#define ANALYZER_API __declspec(dllimport)
#endif
#else
#define ANALYZER_API
#endif

#ifndef NOERROR
#define NOERROR	0
#endif

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
// 帧类型
typedef enum _ANA_MEDIA_TYPE
{
	FRAME_TYPE_UNKNOWN = 0,			//帧类型不可知
	FRAME_TYPE_VIDEO,				//帧类型是视频帧
	FRAME_TYPE_AUDIO,				//帧类型是音频帧
	FRAME_TYPE_DATA					//帧类型是数据帧
}ANA_MEDIA_TYPE;

// 子类型
typedef enum _FRAME_SUB_TYPE
{
	TYPE_DATA_INVALID = -1,				//数据无效
	TYPE_VIDEO_I_FRAME = 0 ,			// I帧
	TYPE_VIDEO_P_FRAME,				// P帧
	TYPE_VIDEO_B_FRAME,				// B帧
	TYPE_VIDEO_S_FRAME,				// S帧
	TYPE_WATERMARK_TEXT,			//水印数据为TEXT类型
	TYPE_WATERMARK_JPEG,			//水印数据为JPEG类型
	TYPE_WATERMARK_BMP,				//水印数据为BMP类型
	TYPE_DATA_INTL,					//智能分析帧
	TYPE_VIDEO_JPEG_FRAME,
	TYPE_DATA_ITS,				//its信息帧
	TYPE_DATA_GPS,
	TYPE_DATA_INTLEX,

	
	TYPE_DATA_MONITOR_CODER,
	TYPE_DATA_MONITOR_STANDARD,

    TYPE_DATA_RAW = 255,
	TYPE_DATA_FILEHEAD,
	TYPE_DATA_INDEX,
	TYPE_DATA_MOTION,
	TYPE_DATA_MARK,
	TYPE_DATA_SCREEN,
	TYPE_DATA_EVENT,
	TYPE_DATA_END
}FRAME_SUB_TYPE;						

// 编码类型
typedef enum _ENCODE_TYPE
{
	ENCODE_VIDEO_UNKNOWN = 0,		//视频编码格式不可知
	ENCODE_VIDEO_MPEG4 ,			//视频编码格式是MPEG4
	ENCODE_VIDEO_HI_H264,			//视频编码格式是海思H264
	ENCODE_VIDEO_JPEG,				//视频编码格式是标准JPEG
	ENCODE_VIDEO_DH_H264,			//视频编码格式是大华码流H264
	ENCODE_VIDEO_INVALID,			//视频编码格式无效

	ENCODE_AUDIO_PCM = 7,			//音频编码格式是PCM8
	ENCODE_AUDIO_G729,				//音频编码格式是G729
	ENCODE_AUDIO_IMA,				//音频编码格式是IMA
	ENCODE_PCM_MULAW,				//音频编码格式是PCM MULAW
	ENCODE_AUDIO_G721,				//音频编码格式是G721
	ENCODE_PCM8_VWIS,				//音频编码格式是PCM8_VWIS
	ENCODE_MS_ADPCM,				//音频编码格式是MS_ADPCM
	ENCODE_AUDIO_G711A,				//音频编码格式是G711A
	ENCODE_AUDIO_AMR,				//音频编码格式是AMR
	ENCODE_AUDIO_PCM16,				//音频编码格式是PCM16
	ENCODE_AUDIO_G711U = 22,		//音频编码格式是G711U
	ENCODE_AUDIO_G723,				//音频编码格式是G723
	ENCODE_AUDIO_AAC = 26,			//音频编码格式是AAC
	ENCODE_AUDIO_TALK = 30,
	ENCODE_VIDEO_H263,
	ENCODE_VIDEO_PACKET
}ENCODE_TYPE;
		
enum
{
	DH_ERROR_NOFIND_HEADER = -20, 	//数据信息不足,如没有帧头
	DH_ERROR_FILE, 					//文件操作失败
	DH_ERROR_MM, 					//内存操作失败
	DH_ERROR_NOOBJECT, 				//不存在相应的对象
	DH_ERROR_ORDER, 				//接口调用次序不正确
	DH_ERROR_TIMEOUT, 				//处理超时
	DH_ERROR_EXPECPT_MODE, 			//接口调用不正确
	DH_ERROR_PARAMETER, 			//参数出错
	DH_ERROR_NOKNOWN, 				//错误原因未明
	DH_ERROR_NOSUPPORT, 			//不提供实现
	DH_ERROR_OVER,
	DH_ERROR_LOCKTIMEOUT,
	DH_NOERROR = NOERROR 			//没有错误
};


typedef enum _STREAM_TYPE
{
	DH_STREAM_UNKNOWN = 0,
		DH_STREAM_MPEG4,		
		DH_STREAM_DHPT =3,
		DH_STREAM_NEW,
		DH_STREAM_HB,
		DH_STREAM_AUDIO,
		DH_STREAM_PS,
	DH_STREAM_DHSTD,
	DH_STREAM_ASF,
	DH_STREAM_3GPP,
	DH_STREAM_RAW,	
	DH_STREAM_TS,
}STREAM_TYPE;

enum
{
	E_STREAM_NOERROR = 0,		//数据校验无误
	E_STREAM_TIMESTAND,			//时间错误，未实现
	E_STREAM_LENGTH,			//长度信息出错
	E_STREAM_HEAD_VERIFY,		//未实现
	E_STREAM_VERIFY,			//数据校验失败
	E_STREAM_HEADER,			//数据没有帧头
	E_STREAM_NOKNOWN,			//不可知错误，未实现
	E_STREAM_LOSTFRAME,
	E_STREAM_WATERMARK,
	E_STREAM_CONTEXT,
	E_STREAM_NOSUPPORT,
	E_STREAM_BODY
};

enum
{
	DEINTERLACE_PAIR = 0,
	DEINTERLACE_SINGLE,
	DEINTERLACE_NONE,
};
////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct _DHTIME								
{
	uint32 second		:6;					//	秒	1-60		
	uint32 minute		:6;					//	分	1-60		
	uint32 hour			:5;					//	时	1-24		
	uint32 day			:5;					//	日	1-31		
	uint32 month			:4;					//	月	1-12		
	uint32 year			:6;					//	年	2000-2063	
}DHTIME,*pDHTIME;
// 帧信息
typedef struct __ANA_FRAME_INFO
{
	ANA_MEDIA_TYPE		nType;			// 帧类型
	FRAME_SUB_TYPE	nSubType;			// 子类型
	STREAM_TYPE		nStreamType;		// 数据打包协议类型（DHAV）
	ENCODE_TYPE		nEncodeType;		// 编码类型

	uint8*	        pHeader;			// 包含帧头的数据指针
	uint32			nLength;			// 包含帧头的数据长度
	uint8*	        pFrameBody;			// 裸数据指针
	uint32			nBodyLength;		// 裸数据长度
	uint32			dwFrameNum;			// 帧序号－丢帧判断

	uint32			nDeinterlace;		// 解交错
	uint8			nFrameRate;			// 帧率
	uint8			nMediaFlag;			// 码流类型标记，h264解码用(0：大华码流；2：海思码流)
	uint16			nWidth;				// 分辨率
	uint16			nHeight;

	uint16			nSamplesPerSec;		// 采样率
	uint8			nBitsPerSample;		// 位数
	uint8			nChannels;

	// 时间日期
	uint16			nYear;	
	uint16			nMonth;
	uint16			nDay;
	uint16			nHour;
	uint16			nMinute;
	uint16			nSecond;
	uint32			dwTimeStamp;		// 时间戳 mktime返回数值
	uint16			nMSecond;			// 毫秒
	uint16			nReserved;
	uint32			Reserved[3];		//
	uint16			sReverved[2];		
	uint32			bValid;				// 
} ANA_FRAME_INFO;


typedef	struct  __ANA_INDEX_INFO
{
	uint32	filePos;						//关键帧在文件中的偏移
	uint32	dwFrameNum;						//关键帧帧号
	uint32 dwFrameLen;						//关键帧帧长
	uint32 frameRate;						//帧率
	uint32	frameTime;						//关键帧时间
}ANA_INDEX_INFO;


#define MAX_IVSOBJ_NUM 200
//智能信息结构
#define  MAX_TRACKPOINT_NUM 10

typedef struct _ANA_IVS_POINT
{
	uint16 		x; 
	uint16 		y; 
	uint16		xSize;
	uint16		ySize;
	//轨迹点是物体外接矩形的中心，根据X，Y及XSize，YSize计算出的物体外接矩形坐标（left，top，right，bottom）：
	//RECT=(X-XSize, Y-YSize, X+XSize, Y+YSize)
	
}ANA_IVS_POINT; 

typedef struct _ANA_IVS_OBJ
{
	uint32				obj_id;						// 物体id
	uint32				enable;						 // 0 表示删除物体  1表示物体轨迹信息有效
	ANA_IVS_POINT 	track_point[MAX_TRACKPOINT_NUM]; 
	uint32				trackpt_num;				// 物体个数，即track_point有效个数
}ANA_IVS_OBJ;

typedef struct _ANA_IVS_OBJ_EX
{
	int				decode_id;
	int				obj_id;
	int				enable;			//无用
	ANA_IVS_POINT 	track_point[MAX_TRACKPOINT_NUM]; 
	int				trackpt_num;
	int				operator_type;	//操作分为三类：新增物体(1), 增加物体轨迹点(2)，删除物体(3)，隐藏物体轨迹(4),其它值无效
	int				frame_part_id;	//0表示一帧的开始，依次递增
	char			color;
	char			alarmCount;
	char			reserved[122];	//保留
	int				nRenderNum;
}ANA_IVS_OBJ_EX;


typedef struct _ANA_IVS_PrePos
{
	uint32				nPresetCount;			//预置点信息个数，单位: 1字节
	uint8*				pPresetPos;				//预置点信息指针
}ANA_IVS_PrePos;

typedef enum  _IVS_METHOD
{
	IVS_track,									//分析智能帧物体移动轨迹信息
	IVS_Preset,									//分析云台预置点信息
	IVS_trackEx									
}IVS_METHOD;

enum CHECK_ERROR_LEVEL
{
	CHECK_NO_LEVEL = 0,
	CHECK_PART_LEVEL,							//如果是用于服务器程序的话而不直接解码，请选用这个选项
	CHECK_COMPLETE_LEVEL						//如果解析出来帧直接用于解码的话，请选用这个一个选项
};

enum
{
	SAMPLE_FREQ_4000 = 1,
	SAMPLE_FREQ_8000,
	SAMPLE_FREQ_11025,
	SAMPLE_FREQ_16000,
	SAMPLE_FREQ_20000,
	SAMPLE_FREQ_22050,
	SAMPLE_FREQ_32000,
	SAMPLE_FREQ_44100,
	SAMPLE_FREQ_48000
};

typedef	void*		    ANA_HANDLE;
typedef ANA_HANDLE*		PANA_HANDLE;

/************************************************************************
 ** 接口定义
 ***********************************************************************/

//------------------------------------------------------------------------
// 函数: ANA_CreateStream
// 描述: 创建码流分析器
// 参数: dwSize：为内部缓冲区的长度，如果为0，内部自适应。pHandle：返回的码流分析器句柄
// 返回:<0 表示失败，==0 表示成功。
//------------------------------------------------------------------------
ANALYZER_API int32	__stdcall ANA_CreateStream(IN int dwSize,OUT PANA_HANDLE pHandle);

//------------------------------------------------------------------------
// 函数: ANA_Destroy
// 描述: 销毁码流分析器
// 参数: hHandle：通过ANA_CreateStream 或者ANA_CreateFile返回的句柄。
// 返回:无
//------------------------------------------------------------------------
ANALYZER_API void	__stdcall ANA_Destroy(IN ANA_HANDLE hHandle);

//------------------------------------------------------------------------
// 函数: ANA_InputData
// 描述: 输入数据流
// 参数: hHandle：通过ANA_CreateStream返回的句柄。pBuffer：数据流地址，dwSize：数据流长度。
// 返回:>=0表示成功。DH_ERROR_NOFIND_HEADER 这个错误应该可以忽略。
//------------------------------------------------------------------------
ANALYZER_API int32	__stdcall ANA_InputData(IN ANA_HANDLE hHandle, IN uint8* pBuffer, IN int dwSize);

//------------------------------------------------------------------------
// 函数: ANA_GetNextFrame
// 描述: 码流分析器获得除音频帧之外其他帧信息并填充到pstFrameInfo里面
// 参数: hHandle：通过ANA_CreateStream或者ANA_CreateFile返回的句柄。pstFrameInfo 外部ANA_FRAME_INFO的一个结构地址。
// 返回:(-1：失败；0：成功；1：缓冲区已空)
//------------------------------------------------------------------------
ANALYZER_API int32	__stdcall ANA_GetNextFrame(IN ANA_HANDLE hHandle, OUT ANA_FRAME_INFO* pstFrameInfo);

// 获取码流分析器中缓冲剩余数据 hHandle：ANA_CreateStream返回的句柄 pData：外部缓冲区 pSize：如果pData==NULL就返回内部缓冲区的长度，pSize的内存值为
//------------------------------------------------------------------------
// 函数: ANA_ClearBuffer
// 描述: 清除码流分析器内部的缓冲。
// 参数: hHandle：通过ANA_CreateStream返回的句柄
// 返回:>=0表示成功
//------------------------------------------------------------------------
ANALYZER_API int	__stdcall ANA_ClearBuffer(IN ANA_HANDLE hHandle);
//------------------------------------------------------------------------
// 函数: ANA_GetRemainData
// 描述: 获取码流分析器中缓冲剩余数据
// 参数: hHandle：通过ANA_CreateStream返回的句柄。pData：外部缓冲区 pSize：如果pData==NULL就返回内部缓冲区的长度，pSize的内存值为返回出来pData的长度。
// 返回:>=0表示成功。
//------------------------------------------------------------------------
ANALYZER_API int32	__stdcall ANA_GetRemainData(IN ANA_HANDLE hHandle, IN uint8* pData, INOUT int* pSize);

//------------------------------------------------------------------------
// 函数: ANA_GetLastError
// 描述: 获得码流分析库错误码
// 参数: hHandle：通过ANA_CreateStream返回的句柄。
// 返回: 错误值。
//------------------------------------------------------------------------
ANALYZER_API int32	__stdcall ANA_GetLastError(IN ANA_HANDLE hHandle);

//------------------------------------------------------------------------
// 函数: ANA_ParseIvsFrame
// 描述: 解析IVS数据帧。
// 参数: pstFrameInfo 数据帧的信息 buffer：数据帧的缓冲地址。len 数据帧的长度。type判断是巡航还是云台。
// 返回: 0表示成功。
//------------------------------------------------------------------------
ANALYZER_API int32	__stdcall ANA_ParseIvsFrame(IN ANA_FRAME_INFO* pstFrameInfo,INOUT unsigned char* buffer,IN int len,IN IVS_METHOD type);
//------------------------------------------------------------------------
// 函数: ANA_CreateFileIndexEx
// 描述: 用于创建文件索引。
// 参数: hHandle：ANA_CreateFile返回的分析器句柄。tv 索引的时间限制，默认为无限等待，直到创建完成或失败。flag ==0 只解析出I帧，其它的值解析出所有的帧。
// 返回: >0：表示多少条。DH_ERROR_TIMEOUT 应该再次调用ANA_WaitForCreateIndexComplete。其他的值表示失败。
//------------------------------------------------------------------------
ANALYZER_API int32	__stdcall ANA_CreateFileIndexEx(IN ANA_HANDLE hHandle,IN struct timeval* tv = NULL,IN int flag = 0);
//------------------------------------------------------------------------
// 函数: ANA_WaitForCreateIndexComplete
// 描述: 用于等待文件索引返回。
// 参数: hHandle：ANA_CreateFile返回的分析器句柄。
// 返回: >0：表示多少条。其他的值表示失败。
//------------------------------------------------------------------------
ANALYZER_API int32	__stdcall ANA_WaitForCreateIndexComplete(IN ANA_HANDLE hHandle);
//------------------------------------------------------------------------
// 函数: ANA_CreateFile
// 描述: 创建文件解析器。
// 参数:filePath:全路径文件名 pHandle：返回的码流分析器句柄
// 返回:0 表示成功。
//------------------------------------------------------------------------
ANALYZER_API int32	__stdcall ANA_CreateFile(const char* filePath,OUT PANA_HANDLE pHandle);

//------------------------------------------------------------------------
// 函数: ANA_GetNextAudio
// 描述: 从码流分析器中获取AUDIO帧数据。
// 参数:pHandle：返回的码流分析器句柄 pstFrameInfo：帧结构信息地址
// 返回:-1：失败；0：成功；1：缓冲区已空
//------------------------------------------------------------------------
ANALYZER_API int32	__stdcall ANA_GetNextAudio(IN ANA_HANDLE hHandle, OUT ANA_FRAME_INFO* pstFrameInfo);

//------------------------------------------------------------------------
// 函数: ANA_ParseFile
// 描述: 开始分析文件。
// 参数:handle：通过调用ANA_CreateFile返回的码流分析器句柄
// 返回:<0：失败；>=0 表示成功。
//------------------------------------------------------------------------
ANALYZER_API int32	__stdcall ANA_ParseFile(IN ANA_HANDLE handle);
//------------------------------------------------------------------------
// 函数: ANA_GetIndexByOffset
// 描述: 开始获取索引信息
// 参数:handle：通过调用ANA_CreateFile返回的码流分析器句柄。offset 偏移位置，以0开始。pIndex 外部的索引结构地址。
// 返回:<0：失败；>=0 表示成功。
//------------------------------------------------------------------------
ANALYZER_API int32	__stdcall ANA_GetIndexByOffset(IN ANA_HANDLE handle,IN int offset,OUT ANA_INDEX_INFO* pIndex);
//------------------------------------------------------------------------
// 函数: ANA_GetMediaFrame
// 描述: 从码流分析器中获取所有的帧数据。
// 参数:pHandle：返回的码流分析器句柄 pstFrameInfo：帧结构信息地址
// 返回:-1：失败；0：成功；1：缓冲区已空
//------------------------------------------------------------------------
ANALYZER_API int32	__stdcall ANA_GetMediaFrame(IN ANA_HANDLE hHandle, OUT ANA_FRAME_INFO* pstFrameInfo);
//------------------------------------------------------------------------
// 函数: ANA_Reset
// 描述: 重置码流分析库。
// 参数:pHandle：ANA_CreateStream返回的码流分析器句柄 nLevel 1表示清理内部缓冲，0表示重新开始分析码流，并且清理内部缓冲。
// 返回:0：成功
//------------------------------------------------------------------------
ANALYZER_API int32	__stdcall ANA_Reset(IN ANA_HANDLE hHandle,IN int nLevel);

//------------------------------------------------------------------------
// 函数: ANA_EnableError
// 描述: 用于设置码流分析库对错误帧的检查力度（不调用，默认是CHECK_PART_LEVEL）
// 参数:pHandle：ANA_CreateStream返回的码流分析器句柄 nEnableLevel：CHECK_ERROR_LEVEL 里面其中的一个值
// 返回:0：成功
//------------------------------------------------------------------------
ANALYZER_API int32	__stdcall ANA_EnableError(IN ANA_HANDLE hHandle,IN int nEnableLevel);

ANALYZER_API int32 __stdcall ANA_GetFrameCount(IN ANA_HANDLE hHandle,IN ANA_MEDIA_TYPE mediaType);


typedef int (__stdcall *FrameCallBack)(ANA_FRAME_INFO*,ANA_INDEX_INFO*,void* nReserved);
ANALYZER_API int32	__stdcall ANA_CallBack(const char* szFileName,FrameCallBack cb,void*);

ANALYZER_API int32	__stdcall ANA_CallBackEx(const char* szFileName,FrameCallBack cb,void*,__int64 fileoffset);

typedef int (__stdcall *OPCallBack)(int eop,void* nReserved);

typedef enum  _eOP
{
	OP_AES
}eOP;

struct OP
{
	int			op;
	char*		szkey;
	int			nkeylen;
	void*		context;
	OPCallBack	opCb;;
};

ANALYZER_API int32	__stdcall ANA_CallBackAndAES(const char* szFileName,FrameCallBack cb,void*,__int64 fileoffset,struct OP* op);

#ifdef __cplusplus
}
#endif


#endif // __STREAM_ANALYZER_H



