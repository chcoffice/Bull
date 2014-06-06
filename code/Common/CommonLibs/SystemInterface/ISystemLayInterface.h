#ifndef ISYSTEMLAYINTERFACE_DEF_H
#define ISYSTEMLAYINTERFACE_DEF_H

#if !defined(_DEBUG) && !defined(DEBUG)
#ifndef NDEBUG
#	define NDEBUG
#endif
#endif

#include <assert.h>


#include "ISystemDataType.h"
#ifdef WINCE
#include <windows.h>
#elif	_WIN32	
#include <process.h>
#endif






/********************************************************************************************
  Copyright (C), 2010-2011, GOSUN 
  File name 	: ISYSTEMLAYINTERFACE.H      
  Author 		: hf      
  Version 		: Vx.xx        
  DateTime 		: 2010/6/9 16:31
  Description 	:     // 实现系统封装
						1）线程		CGSThread类
						2）普通锁	CGSMutex类
						3）自动锁	CGSAutoMutex类
						4）进程锁	CGSProcessMutex类
						5）读写锁	CGSWRMutex类
						6）信号量	CGSSem类
						7）条件变量	CGSCond类
						8）目录枚举 CGSDir类
						9）枚举磁盘	GSGetSysDisk（）
						10）时间函数 DoGetTickCount()， DoGetLocalTime（）
********************************************************************************************/

/*******************************************************************************
功能说明：线程

使用说明：线程函数在调用时，采用如下方式：
void threadfun(CGSThread *gsThreadHandle, void *pUserParam)
{
	while(!gsThreadHandle->TestExit()&&...)
	{
		...
	}
}
*******************************************************************************/
class CGSThread;
class CGSCond;
class CGSMutex;

/********************************************************************************************
  Function		: 线程函数的定义    
  DateTime		: 2010/6/9 16:43	
  Description	: 线程函数定义
  Input			: CGSThread *gsThreadHandle：线程句柄
					void *pUserParam：		用户参数
  Output		: NULL
  Return		: NULL
  Note			: NULL	
********************************************************************************************/
typedef void (*GSThreadCallbackFunction)(CGSThread *gsThreadHandle, void *pUserParam );

/*#ifdef _WIN32
extern DWORD WINAPI CGSThreadProcFunc(HANDLE param);
#elif _LINUX
extern void *CGSThreadProcFunc(HANDLE param);
#endif*/ 

class GS_CLASS CGSThread
{
//public:
//#ifdef _WIN32
//	friend DWORD WINAPI CGSThreadProcFunc(HANDLE param);
//#elif _LINUX
//	friend void *CGSThreadProcFunc(HANDLE param);
//#endif 
	
public:
	CGSThread();
	virtual ~CGSThread(void);
	virtual BOOL Start(GSThreadCallbackFunction fnOnEvent=NULL, void *pUserParam=NULL);	 //开始线程，成功返回TRUE,失败返回FALSE	
	virtual BOOL Stop();	//停止线程，成功返回TRUE,失败返回FALSE	
	virtual BOOL TestExit();	//判断线程是否退出，在线程函数的循环中调用，执行暂停和唤醒操作	
	BOOL Join();	//等待线程结束，成功返回TRUE,失败返回FALSE	
	BOOL Join(INT mseconds);	//等待一段时间，让线程结束,成功返回TRUE,失败返回FALSE	
	BOOL Suspend();		//线程暂停,成功返回TRUE,失败返回FALSE	
	BOOL Resume();		//唤醒线程,成功返回TRUE,失败返回FALSE	
	BOOL IsRunning ();	//返回线程运行状态，TRUE为正在运行，FALSE为未运行
	void Cancel();		//强制退出
	GSThread GetThreadHandle(); //获取线程句柄，返回获取的句柄，若句柄不存在，返回NULL
	void UnInitData();	//释放线程句柄

protected:
	void Kill(void);	//强制线程退出	
	void Detach(void);	//linux下分离线程	

public:
	BOOL m_bRunningState;
	BOOL m_bExit;
	GSThreadCallbackFunction m_fnUser;
	void *m_pFnUserParam;
	CGSMutex *m_GSMutexUnit;
	BOOL m_bMutexLock;
	CGSCond  *m_GSCond;	

	BOOL	m_bthreadfinish;
	CGSMutex *m_GSMutexthreadfinish;

protected:
	GSThread m_GSThread;
	CGSCond	 *m_GSCondPause;
	BOOL m_bExiting;
	BOOL m_bPause;
	BOOL m_bJoin;
	BOOL m_bDetached;
	CGSMutex *m_GSMutexPause;

};




/*******************************************************************************
功能说明：普通锁
*******************************************************************************/
class GS_CLASS CGSMutex
{
public:
	CGSMutex();
	~CGSMutex(void);
	BOOL	Lock();		//加锁，成功返回TRUE，失败返回FALSE  
	void	Unlock();	//解锁，成功返回TRUE,失败返回FALSE
	BOOL	TryLock();	//非阻塞的加锁，成功返回TRUE,失败返回FALSE

public:
	GSMutex m_GSMutex;
};

/*******************************************************************************
功能说明：进程间的锁
*******************************************************************************/
class GS_CLASS CGSProcessMutex
{
public:
	CGSProcessMutex( const char *czKey);
	~CGSProcessMutex(void);
	BOOL	LockProcess();  //加锁，成功返回TRUE,失败返回FALSE  
	void	UnlockProcess(); //解锁，成功返回TRUE,失败返回FALSE
	BOOL	TryLockProcess();//非阻塞的加锁，成功返回TRUE,失败返回FALSE

private:
	GSProcessMutex m_GSProcessMutex;
};

/*******************************************************************************
功能说明：信号量
*******************************************************************************/
class GS_CLASS CGSSem
{
public:
	CGSSem( const char *czKey = NULL,BOOL bProcess=TRUE ); //输入参数：const char *czKey ：信号量的名称
															//BOOL bProcess：是否是进程间的信号量，TRUE:是，FALSE:否
	~CGSSem();
	BOOL IsValid( void ); //判断信号量是否有效,成功返回TRUE,失败返回FALSE
	BOOL Signal(); //发送信号量，成功返回TRUE,失败返回FALSE
	BOOL Wait(); //等待信号量，成功返回TRUE,失败返回FALSE
	BOOL Wait( UINT mSeconds ); //等待信号量一定时间，UINT mSeconds：等待的时间，成功返回TRUE,失败返回FALSE

private:
	GSSem m_GSSem;
	BOOL  m_bIsValid;
	
};

/*******************************************************************************
功能说明：读写锁
*******************************************************************************/
class GS_CLASS CGSWRMutex
{
public:
	CGSWRMutex();
	~CGSWRMutex(void);
	BOOL	LockReader(); //加锁读操作，成功返回TRUE,失败返回FALSE 
	BOOL	TryLockReader();  //非阻塞的加锁读操作，成功返回TRUE,失败返回FALSE  
	void	UnlockReader(); //解锁读操作
	BOOL	LockWrite(); //加锁写操作，成功返回TRUE,失败返回FALSE
	BOOL	TryLockWrite(); //非阻塞的加锁写操作，成功返回TRUE,失败返回FALSE
	void	UnlockWrite(); //解锁写操作

private:
#ifdef WINCE
	BOOL GSStopReadWaitSetWrite();
	HANDLE m_GSReadEvent;
	HANDLE m_GSWriteEvent;
	LONG m_readCount;
	LONG m_writeCount;
#elif _WIN32
	BOOL GSStopReadWaitSetWrite();
	HANDLE m_GSReadEvent;
	HANDLE m_GSWriteEvent;
	volatile LONG m_readCount;
	volatile LONG m_writeCount;
#ifdef _DEBUG
#define _MUTEX_DEBUG  
#endif

#ifdef _MUTEX_DEBUG
	volatile LONG m_iRRefDebug;
	volatile LONG m_iWRefDebug;
#endif
#elif _LINUX
	GSRwmutex	m_GSRwmutex;
#endif
};

class GS_CLASS CGSAutoReaderMutex
{
private :
	CGSWRMutex *m_pMutex;
public :
	CGSAutoReaderMutex(CGSWRMutex *pWRMutex )
		: m_pMutex(pWRMutex)
	{
		m_pMutex->LockReader();
	}
	~CGSAutoReaderMutex(void)
	{
		m_pMutex->UnlockReader();
	}

};

class GS_CLASS CGSAutoWriterMutex
{
private :
	CGSWRMutex *m_pMutex;
public :
	CGSAutoWriterMutex(CGSWRMutex *pWRMutex )
		: m_pMutex(pWRMutex)
	{
		m_pMutex->LockWrite();
	}
	~CGSAutoWriterMutex(void)
	{
		m_pMutex->UnlockWrite();
	}

};

/*******************************************************************************
功能说明：自动锁
*******************************************************************************/
class GS_CLASS CGSAutoMutex
{

public :
	CGSAutoMutex( CGSMutex *locker ); //加锁
	~CGSAutoMutex(void); //解锁

private:
	CGSMutex *m_locker;

};

/*******************************************************************************
功能说明：条件变量
*******************************************************************************/
class GS_CLASS CGSCond
{
public:
	CGSCond();
	~CGSCond(void);
	INT	Wait(); //等待条件变量， 返回0为成功，其他表示失败  
	INT	WaitTimeout(INT mseconds); //等待条件变量一定时间，mseconds：等待条件变量的时间，返回0为成功，其他表示失败
	INT	Signal(); //发送信号，返回0为成功，其他表示失败
	INT	BroadcastSignal(); //发送广播信号，返回0为成功，其他表示失败

private:
	GSCond m_GSCond;
#ifdef _WIN32
	HANDLE m_mutex; 
#elif _LINUX
	CGSMutex *m_CondMutex;
#endif


};


class GS_CLASS CGSCondEx
{
private :
#ifdef _WIN32
	HANDLE m_hEvent;
	INT m_iWaitConts; 
#else
	pthread_cond_t  m_hEvent;
#endif

public :
	//返回值
	static const INT R_SUCCESS = 0; //成功
	static const INT R_ESYSTEM = 1; //超时
	static const INT R_ETIMEOUT = 2; //系统函数调用错误



	CGSCondEx(void);
	~CGSCondEx(void);
	INT	Wait( CGSMutex *pMutex ); //等待条件变量, 	 ;
	INT	WaitTimeout(CGSMutex *pMutex, INT mseconds); //等待条件变量一定时间，
	//mseconds：等待条件变量的时间，
	//超时返回 R_ETIMEOUT	
	INT	Signal(void); //发送信号，
	INT	BroadcastSignal(void); //发送广播信号，返回0为成功，其他表示失败
	
};


/******************************************************************************
功能说明：目录枚举
******************************************************************************/
class  GS_CLASS CGSDir
{
public:
	CGSDir();
	~CGSDir(void);
	BOOL	OpenDir(const char *czDirPath);   //打开指定路径的目录，成功返回TRUE,失败返回FALSE 
	void	CloseDir(void);						//关闭目录
	void	ReadDir(std::vector<StruGSFileInfo> &vectFileList); //读取目录信息，输入参数std::vector<StruGSFileInfo> &vectFileList

//	UINT64 GetDirSize(void); //获取目录大小， 打开后调用
private:
	HANDLE	m_GSDir;
	CGSString m_strPathName;
#ifdef WINCE
private:
	WCHAR	m_wczDirPath[MAX_PATH];
#endif

};

/******************************************************************************
功能说明：进程信息
******************************************************************************/
class  GS_CLASS CGSProcessInfo
{
public:
	CGSProcessInfo();
	~CGSProcessInfo(void);
	INT32	GSGetTotalCPUUsage(INT32 &iCPU);   //获取总的CPU使用率，返回CPU使用率，返回-1，获取失败
	INT32	GSGetTotalMemoryUsage(DWORD &dwMem);	//获取总的内存使用率

private:
#ifdef _WIN32
	INT64 CompareFileTime (FILETIME time1, FILETIME time2);
#endif
	//获得CPU的核数
	INT32	GSGetProcessorNumber(); 

private:
	//cpu数量
	INT32 m_processor_count;
};
/******************************************************************************
功能说明：网络流量获取
******************************************************************************/
class  GS_CLASS CGSNetInfo
{
public:
	CGSNetInfo();
	~CGSNetInfo(void);
	INT32	GSGetNetCount();	//获取网卡个数，返回网卡个数，失败返回0或者负数
	INT32	GSGetNetUsage(StruGSNETSTAT * pstNetStat,INT32 iBuffLen);		//获取网络流量参数
private:
	StruGSNETSTATTable* GetLastNetStat(const char* szName);
private:
	UINT64				m_uiLastTime;		//上一次获取流量的时间
	INT32				m_iNetCounts;		//网卡个数
	StruGSNETSTATTable	m_stLastNetStat[MAX_NET_COUNTS];//上一次各个网卡的流量
	char*				m_pDataBuf;
	INT32				m_iBufLen;
};
/*******************************************************************************
功能说明：枚举磁盘
*******************************************************************************/

  INT	GSGetSysDisk(std::vector<std::string> &vectDiskList); //获取磁盘信息，存放在磁盘表中，返回磁盘个数

/*******************************************************************************
功能说明：系统时间
*******************************************************************************/
  UINT64   DoGetTickCount(); //获取从操作系统启动到现在所经过的毫秒数，返回从操作系统启动到现在所经过的毫秒数

  void    DoGetLocalTime(StruSysTimePtr pLoaltime); //获取当地的当前系统日期和时间，存放在包含日期和时间信息的结构中

// 对iVal 进行递增操作， 返回增加后的值
 long AtomicInterInc( GSAtomicInter &iVal );
// 对iVal 进行递减操作， 返回递减后的值
 long AtomicInterDec( GSAtomicInter &iVal );

//比较iVal 释放 以iOldVal 相等， 如果相等，把 iVal 设定为 iNewVal, 并返回TRUE， 否则返回FALSE
 BOOL AtomicInterCompareExchange(GSAtomicInter &iVal, const long iOldVal, const long iNewVal);

//设置 iVal 为 iNewVal, 返回 设定前 iVal 的值
 long AtomicInterSet(GSAtomicInter &iVal, const long iNewVal );

//对 iVal 执行 AND 运算 iFlag, 返回 设定前 iVal 的值
 long AtomicInterAnd(GSAtomicInter &iVal, const long iFlag );

//对 iVal 执行 Or 运算 iFlag, 返回 设定前 iVal 的值
 long AtomicInterOr(GSAtomicInter &iVal, const long iFlag );



//返回当前引用程序的所在的目录
CGSString GSGetApplicationPath(void);

//格式化目录
void GSPathParser( CGSString &strPath);

//检测czPath 是否存在， 如果不存在 将创建改目录
BOOL GSTestAndCreateDir( const char *czPath);


#ifndef _WIN32
#define  localtime_s localtime_r
#endif

#endif // ISYSTEMLAYINTERFACE_DEF_H
