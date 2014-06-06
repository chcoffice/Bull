/*****************************************************
Copyright 2008-2011 Hikvision Digital Technology Co., Ltd.

FileName: AnalyzeDataNewInterface.h

Description: 码流分析库新接口

current version: 4.0.0.1
 	 
Modification History: 2010/5/27 辛安民
*****************************************************/

#ifndef _ANALYZEDATA_NEW_INTERFACE_H_
#define _ANALYZEDATA_NEW_INTERFACE_H_

#if defined(_WINDLL)
	#define ANALYZEDATA_API  extern "C" __declspec(dllexport) 
#else 
	#define ANALYZEDATA_API  extern "C" __declspec(dllimport) 
#endif

//数据包类型
#define FILE_HEAD			            0
#define VIDEO_I_FRAME		            1
#define VIDEO_B_FRAME		            2
#define VIDEO_P_FRAME		            3
#define AUDIO_PACKET		            10
#define PRIVT_PACKET                    11

//E帧标记
#define HIK_H264_E_FRAME        	    (1 << 6)

//错误码		    
#define ERROR_NOOBJECT                  -1//无效句柄
#define ERROR_NO                        0//没有错误
#define ERROR_OVERBUF                   1//缓冲溢出
#define ERROR_PARAMETER                 2//参数错误
#define ERROR_CALL_ORDER                3//调用顺序错误
#define ERROR_ALLOC_MEMORY              4//申请缓冲失败
#define ERROR_OPEN_FILE                 5//打开文件失败
#define ERROR_MEMORY                    11//内存错误
#define ERROR_SUPPORT                   12//不支持
#define ERROR_UNKNOWN                   99//未知错误

typedef struct _PACKET_INFO
{
	int		nPacketType;    /*包类型 
						    0  - 文件头
	                        1  -	I帧	
							2  -	B帧	
							3  -	P帧
							10 - 音频包
							11 -私有包*/

	char*	pPacketBuffer;//当前帧数据的缓存区地址
	DWORD	dwPacketSize;//包的大小
	
	int		nYear;	//时标：年
	int		nMonth; //时标：月
	int		nDay;//时标：日
	int		nHour;//时标：时
	int		nMinute;//时标：分
	int		nSecond;//时标：秒

	DWORD   dwTimeStamp;//时间戳：毫秒

} PACKET_INFO;

typedef struct _PACKET_INFO_EX
{
	int		nPacketType;    /*包类型 
						    0  - 文件头
	                        1  -	I帧	
							2  -	B帧	
							3  -	P帧
							10 - 音频包
							11 -私有包*/

	char*	pPacketBuffer;//当前帧数据的缓存区地址
	DWORD	dwPacketSize;//包的大小
	
	int		nYear;	//时标：年
	int		nMonth; //时标：月
	int		nDay;//时标：日
	int		nHour;//时标：时
	int		nMinute;//时标：分
	int		nSecond;//时标：秒

	DWORD   dwTimeStamp;//时间戳低32位，单位毫秒

	DWORD          dwFrameNum;//帧号
	DWORD          dwFrameRate;//帧率
	unsigned short uWidth;//宽度
	unsigned short uHeight;//高度
	DWORD		   dwTimeStampHigh;//时间戳高32位，单位毫秒
	DWORD          dwFlag;//E帧标记，是E帧 dwFlag = 64,否则为0
	DWORD          Reserved[5];//保留字

} PACKET_INFO_EX;

/***************************************************************************************
功  能： 流式打开并获取分析句柄
输入参数：DWORD dwSize --设置分析缓冲大小
          ≤CIF：≤200KB; 
          ≤4CIF：≤1MB
         PBYTE pHeader--头数据

输出参数：无
返回值： 分析句柄
****************************************************************************************/	
ANALYZEDATA_API HANDLE __stdcall HIKANA_CreateStreamEx(IN DWORD dwSize, IN PBYTE pHeader);
/***************************************************************************************
功  能： 销毁分析句柄
输入参数：HANDLE hHandle -- 分析句柄
输出参数：无
返回值： 无
****************************************************************************************/
	
ANALYZEDATA_API void   __stdcall HIKANA_Destroy(IN HANDLE hHandle);
/***************************************************************************************
功  能：向库中塞入数据
输入参数：HANDLE hHandle -分析句柄
          PBYTE pBuffer-塞入数据地址
		  DWORD dwSize-塞入数据长度
输出参数：
返回值：成功与否
****************************************************************************************/
	
ANALYZEDATA_API BOOL   __stdcall HIKANA_InputData(IN HANDLE hHandle, IN PBYTE pBuffer, IN DWORD dwSize);
/***************************************************************************************
功  能：从SDK缓冲区中获取PACKET_INFO包，在使用AnalyzeDataInputData输入数据成功后请反复调用HIKANA_GetOnePacketEx直到返回非0值，以确保获取缓存中所有有效数据。
输入参数：HANDLE hHandle -分析句柄
输出参数：PACKET_INFO* pstPacket -库与外部交换数据的结构
返回值：成功返回0，否则返回错误码
****************************************************************************************/

ANALYZEDATA_API int	   __stdcall HIKANA_GetOnePacketEx(IN HANDLE hHandle, OUT PACKET_INFO_EX* pstPacket);
/***************************************************************************************
功  能：清空缓存
输入参数：HANDLE hHandle -分析句柄
输出参数：无
返回值：成功与否
****************************************************************************************/
ANALYZEDATA_API BOOL   __stdcall HIKANA_ClearBuffer(IN HANDLE hHandle);
/***************************************************************************************
功  能：获取输入缓冲中的残余数据
输入参数：HANDLE hHandle -分析句柄
          PBYTE pData-残余数据地址
		  DWORD *pdwSize-残余数据大小指针
输出参数：
返回值：成功与否
****************************************************************************************/
ANALYZEDATA_API int	   __stdcall HIKANA_GetRemainData(IN HANDLE hHandle, IN PBYTE pData, OUT DWORD* pdwSize);
/***************************************************************************************
功  能：获取错误码
输入参数：HANDLE hHandle -分析句柄
输出参数：无
返回值：返回错误码
****************************************************************************************/
ANALYZEDATA_API DWORD  __stdcall HIKANA_GetLastErrorH(IN HANDLE hHandle);

#endif