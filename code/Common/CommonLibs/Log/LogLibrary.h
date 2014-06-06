#pragma once

#include <stdio.h>
#include <string.h>
#include "ILogLibrary.h"

#define LOG_MAX_SIZE				2*1024*1024		//日志大小
#define LOG_MEDIUM_NUM				2				//日志媒介数目（控制台或文件）
#define LOG_LEVEL_NUM				10				//日志等级数目
#define LOG_DIR_NAME_NUM			512				//路径最大长度
#define LOG_DOCUMENT_NAME_NUM		32				//日志文件夹名最大长度
#define LOG_FILE_NAME_NUM			64				//日志文件名最大长度
#define LOG_EX_NAME_NUM				32				//日志文件扩展名
class CLogLibrary:public ILogLibrary
{
public:
	CLogLibrary(void);
	virtual ~CLogLibrary(void);

public:


	/*各媒质对应等级信息*/
	typedef struct 
	{
		UINT uiMedium;//媒质
		BOOL bFlag;//是否有效
		UINT uiLevel;//对应等级
	}StruLogWay;

private:
	FILE *m_pfLog;//日志文件句柄

	char m_strDir[LOG_DIR_NAME_NUM];//日志路径,由用户设定
	char m_strDocu[LOG_DOCUMENT_NAME_NUM];//当前日志所在文件夹，是根据当前日期自动生成,型如"log2010_12_31"
	char m_strFileName[LOG_FILE_NAME_NUM];//日志文件名
	char m_strFileEx[LOG_EX_NAME_NUM];	 //日志文件扩展名
    char m_strWholeName[1024];//日志全路径
	void *m_pCaller;//用户参数
	void ( *m_pfnDealCB)(void *pCaller,const char *pszLevel,const char *pszPrefix,const char *pszMsg);
	UINT64 m_uiLogSize;//日志大小
	UINT64 m_uiCurSize; // 日志当前大小
	UINT m_uiWay;//文件策略，回卷和新建
	StruLogWay m_stLogWay[LOG_MEDIUM_NUM];//记录日志各种输出方式及相关等级信息
	CGSMutex *m_pGSMutex;//锁，支持多线程
	BOOL bDiskReady;// 磁盘准备好标志
public:

	INT  SetLogDir(const char *pszDir,const char *pszExtName);//设置日志路径
	INT  SetLogSize(UINT uiWay,UINT uiSize);// 设置日志大小及是否回卷
	INT  SetLogLevel(UINT uiMedium,UINT uiLevel);//设置日志输出方式（控制台或文件）及对应日志级别
	void SetLogCB(void *pCaller,void ( *pfnDealCB)(void *pCaller,const char *pszLevel,const char *pszPrefix,const char *pszMsg));//设置日志回调函数；
	INT  Log(UINT uiLevel,char *pszPrefix,char  *pszFormat,...);//支持变长
private:
	INT GenerateLogDir(const char *pszDir);
	INT WriteToFile(const char *pszMsg);
	string GetTimeStr();
	INT CreatLogDir(char *pszDir);
	BOOL CountDiskFreeSpace(const char *pDir,UINT64 &uiTotalFreeSize);// 获取磁盘可用剩余空间

};
