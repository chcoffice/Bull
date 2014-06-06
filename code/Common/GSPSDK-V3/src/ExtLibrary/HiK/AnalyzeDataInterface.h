/*****************************************************
Copyright 2008-2011 Hikvision Digital Technology Co., Ltd.

FileName: AnalyzeDataInterface.h

Description: 码流分析库接口

current version: 4.0.0.1
 	 
Modification History: 2010/4/13 辛安民
*****************************************************/

#ifndef _ANALYZEDATA_INTERFACE_H_
#define _ANALYZEDATA_INTERFACE_H_

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
#define ERROR_NOOBJECT                  -1
#define ERROR_NO                        0
#define ERROR_OVERBUF                   1
#define ERROR_PARAMETER                 2
#define ERROR_CALL_ORDER                3
#define ERROR_ALLOC_MEMORY              4
#define ERROR_OPEN_FILE                 5
#define ERROR_MEMORY                    11
#define ERROR_SUPPORT                   12
#define ERROR_UNKNOWN                   99

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

	DWORD   dwTimeStamp;//时间戳低32位，毫秒

	DWORD          dwFrameNum;//帧号
	DWORD          dwFrameRate;//帧率
	unsigned short uWidth;//宽度
	unsigned short uHeight;//高度
	DWORD		   dwTimeStampHigh;//时间戳高32位，毫秒
	DWORD          dwFlag;//帧标记
	DWORD		   dwFilePos;//数据帧在文件中的位置，流模式下无效
	DWORD          Reserved[4];//保留字

} PACKET_INFO_EX;
/***************************************************************************************
功  能： 获取可用句柄号
输入参数：无
输出参数：无
返回值： 0~499  有效
        -1失败，已经无可用句柄
****************************************************************************************/
ANALYZEDATA_API int   __stdcall AnalyzeDataGetSafeHandle();

/***************************************************************************************
功  能： 流方式打开
输入参数：LONG lHandle -合法句柄号
          PBYTE pHeader -头部数据
          
输出参数：无
返回值： 成功与否       
****************************************************************************************/
ANALYZEDATA_API BOOL  __stdcall	AnalyzeDataOpenStreamEx(IN LONG lHandle, IN PBYTE pHeader);
/***************************************************************************************
功  能：释放AnalyzeDataOpenStreamEx ()打开的句柄
输入参数：LONG lHandle- 合法句柄号
输出参数： 无
返回值：  无
****************************************************************************************/

ANALYZEDATA_API void  __stdcall	AnalyzeDataClose(IN LONG lHandle);
/***************************************************************************************
功  能：向库中塞入数据
输入参数：LONG lHandle -合法句柄号
          PBYTE pBuffer-塞入数据地址
		  DWORD dwSize-塞入数据长度
输出参数：
返回值：成功与否
****************************************************************************************/
	
ANALYZEDATA_API BOOL  __stdcall	AnalyzeDataInputData(IN LONG lHandle, IN PBYTE pBuffer, IN DWORD dwSize);
/***************************************************************************************
功  能：从SDK缓冲区中获取PACKET_INFO包，在使用AnalyzeDataInputData输入数据成功后请反复调用AnalyzeDataGetPacket直到返回非0值，以确保获取缓存中所有有效数据。
输入参数：LONG lHandle -合法句柄号
输出参数：PACKET_INFO* pstPacket -库与外部交换数据的结构
返回值：成功返回0，否则返回错误码
****************************************************************************************/
	
ANALYZEDATA_API int	  __stdcall AnalyzeDataGetPacket(IN LONG lHandle, OUT PACKET_INFO* pstPacket);
/***************************************************************************************
功  能：从SDK缓冲区中获取PACKET_INFO包，在使用AnalyzeDataInputData输入数据成功后请反复调用AnalyzeDataGetPacket直到返回非0值，以确保获取缓存中所有有效数据。
输入参数：LONG lHandle -合法句柄号
输出参数：PACKET_INFO* pstPacket -库与外部交换数据的结构
返回值：成功返回0，否则返回错误码
****************************************************************************************/

ANALYZEDATA_API int	  __stdcall AnalyzeDataGetPacketEx(IN LONG lHandle, OUT PACKET_INFO_EX* pstPacket);
/***************************************************************************************
功  能：获取输入缓存中的残余数据
输入参数：lHandle-合法句柄号
输出参数：pData-残余数据地址指针
          pdwSize - 残余数据大小指针

返回值：成功与否
****************************************************************************************/

ANALYZEDATA_API BOOL  __stdcall AnalyzeDataGetTail(IN LONG lHandle, OUT BYTE** pData, OUT DWORD* pdwSize);
/***************************************************************************************
功  能：获取错误码
输入参数：lHandle-合法句柄号
输出参数：无
返回值：错误码
****************************************************************************************/

ANALYZEDATA_API DWORD __stdcall AnalyzeDataGetLastError(IN LONG lHandle);

#endif