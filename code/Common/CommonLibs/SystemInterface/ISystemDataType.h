#ifndef ISYSTEMDATATYPE_DEF_H
#define ISYSTEMDATATYPE_DEF_H

#include "GSDefine.h"
#include "GSType.h"

#ifdef _WIN32
#elif _LINUX
#include <semaphore.h>
#endif

/******************************************************************************/
/********************************数据结构定义*********************** ***/
/******************************************************************************/

#define GS_FILE_FIFO		0x1000  //管道文件
#define GS_FILE_OTHER		0x2000	//其他类型	
#define GS_FILE_DIRECTORY   0x4000	//目录
#define GS_FILE_COMMON		0x8000  //普通文件

/*文件信息结构*/
typedef struct _StruGSFileInfo
{
	std::string strFileName;		//文件名
	INT	  iFileType;				//文件类型 :目录，普通文件，管道文件，其他类型
}StruGSFileInfo, *StruGSFileInfoPtr;


/* 系统时间结构体 */
typedef struct _StruSysTime
{
	WORD wYear;
	WORD wMonth;
	WORD wDayOfWeek;
	WORD wDay;
	WORD wHour;
	WORD wMinute;
	WORD wSecond;
	WORD wMilliseconds;
}StruSysTime, *StruSysTimePtr;

/******************************************************************************/
/********************************网络信息获取数据结构定义*********************** ***/
/******************************************************************************/
//用来转换字节为1K
#define DIV 1024
#define	GS_MAX_NET_NAME_LEN	256
#define	MAX_NET_COUNTS		16

typedef struct StruGSNETSTATTable
{	
	char szName[GS_MAX_NET_NAME_LEN]; //网卡描述名称
	INT64 iRecv;					//总共接收字节数
	INT64 iTrans;					//总共传送字节数
	double dRecvSpeed;				//网络接收速度
	double dTransSpeed;				//网络传送速度

}StruGSNETSTATTable,*StruGSNETSTATTablePtr;

typedef struct StruGSNETSTAT
{	
	INT32				iNetNum;			//网卡数量
	StruGSNETSTATTable	stNetStatTable[1];	//网卡流量信息
}StruGSNETSTAT,*StruGSNETSTATPtr;
/******************************************************************************/
/********************************线程相关元素定义*********************** ***/
/******************************************************************************/

#ifdef _WIN32

#ifndef  GS_INIT_SEM_COUNT						
#define  GS_INIT_SEM_COUNT  (LONG)1
#endif
#ifndef	 GS_MAX_SEM_COUNT
#define  GS_MAX_SEM_COUNT   (LONG)1
#endif

#define CAN_NOT_READ_SIGN   0x80000000
#define CAN_NOT_WRITE_SIGN  0x40000000
#define READ_COUNT_MASK     0x3fffffff

#define GSThread			HANDLE				//线程类型
#define GSProcessMutex		HANDLE				//进程锁
#define GSMutex				CRITICAL_SECTION	//线程锁
#define GSRwmutex			CRITICAL_SECTION	//读写锁
#define GSCond				HANDLE				//条件变量
#define GSSem				HANDLE				//信号量

#elif _LINUX

#define GSThread			pthread_t
#define GSProcessMutex		INT
#define GSMutex				pthread_mutex_t
#define GSRwmutex			pthread_rwlock_t
#define GSCond				pthread_cond_t
#define GSSem				sem_t*


#endif


#endif // ISYSTEMDATATYPE_DEF_H
