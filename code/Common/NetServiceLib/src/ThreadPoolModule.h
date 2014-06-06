#if !defined (ThreadPoolModule_DEF_H)
#define ThreadPoolModule_DEF_H
/********************************************************************
	Copyright (C), 1997-2010, gosun Tech. Co., Ltd.
	FileName: 		ThreadPoolModule.h
	Author:        	陈聪
	Version :       v1.0   
	Date:			2010/04/30
	Description:    线程池模块。支持linux windows。

*********************************************************************/


#include "NetServiceDataType.h"

#include "CommunicationManager.h"


namespace NetServiceLib
{

#define		THREADPOOL_MAX_THREAD_COUNT		50		//最大线程数
//#define		USE_NETHREAD	1

#ifdef _WIN32

#define NetThread			HANDLE				//线程类型

#else

#define NetThread			pthread_t

#endif

class CNetThread;

typedef void (*ThreadCallbackFunction)(CNetThread* pclsThread, void *pUserParam );


//事件主要有监听、接受、发数据、收数据

//线程任务结构体
typedef struct ThreadPoolTask
{
	enumThreadEventType		enumEvent;					// 事件，监听、连接、接受、接收回调、发送，不同类型的事件线程池主线程将执行不同步骤
	pThreadPoolExecute_Fn	pFunction;					// 线程执行函数.
	void*					pObject;					// 指向CThreadIOCP对象.
	void*					pObject2;					// 由用户指定，也可以是NULL 。
}struThreadTask, *pstruThreadPoolTask;

class CNetThread
{
public:
	CNetThread();
	virtual ~CNetThread();

	// 设置线程运行指针
	BOOL Start(ThreadCallbackFunction fnOnEvent=NULL, void *pUserParam=NULL);
public:
	ThreadCallbackFunction		m_fnUser;
	NetThread					m_hThread;
	void*						m_pFnUserParam;

};
class CThreadPoolModule
{
public:
	CThreadPoolModule(void);
	virtual ~CThreadPoolModule(void);
public:
	// 初始线程池
	INT			Initialize( UINT uiThreads=2 );

	// 释放线程池
	void		Uninitialize();
	
	//增加任务
	bool		AssignTask(pstruThreadPoolTask pTask);

	// 线程池管理线程
	static void ManagerThreadProc(CNetThread* pclsThread, void * pInfo);
	
	//静态方法，线程执行函数.
	static void WorkerThreadProc(CNetThread* pclsThread, void * pInfo);

	

	// 设置线程池退出
	inline	void		SetIsExitWorkerThreadProc(){ m_bIsExitWorkerThreadProc = false;};

	// 唤醒线程 此函数特地提供给外部，由外部根据特殊需要主动唤醒线程。如果外部不唤醒，管理线程也会唤醒线程来执行任务，只不过是响应迟一点
	inline	void	WakeUpThread(){ m_GSCond.Signal(); }
public:

	// 线程池管理线程执行函数
	void		ManagerThreadProc(CNetThread* pclsThread);

	//线程真正的执行函数
	void		WorkerThreadProc(CNetThread* pclsThread);

	

	//以下是为了测试时取得数据而添加的函数
	//获取空闲线程数目
	INT16		GetIdleThreadCount();
	//获取忙碌线程数目
	INT16		GetBusyThreadCount(){ CGSAutoMutex	AutoMutex(&m_GSMutexDequeThread); return m_VecBusyThread.size(); };
	//获取所有线程数目
	INT16		GetAllThreadCount(){ CGSAutoMutex	AutoMutex(&m_GSMutexDequeThread); return m_VectorThread.size(); };
	//获取当前存在的任务数目
	INT16		GetTaskCount(){ CGSAutoMutex	AutoMutex(&m_GSMutexDequeTask); return m_DequeTask.size(); };
	//获取当前待删除的线程数
	INT16		GetWaitDelThreadNum(){ return m_iWaitDelThreadNum; };
	//获取当前删除队列中的线程数
	INT16		GetVecWaitDelThread(){ CGSAutoMutex	AutoMutex(&m_GSMutexDequeThread); return m_VecWaitDelThread.size(); };
	// 设置日志指针
	void		SetLogInstancePtr( ILogLibrary* clsLogPtr){ m_clsLogPtr = clsLogPtr; };
private:
	INT			AllocNewThreads(UINT uiThreadCount);
	INT			MoveToBusyVec(CNetThread* pclsThread);//移到忙碌队列
	INT			MoveToIdleVec(CNetThread* pclsThread);//移动空闲队列 
	// 释放线程
	INT			FreeThread();
	//增加线程
	INT			AddThread(UINT16 usThreads);
	//减少线程 
	INT			SubThread(UINT16 usThreads);
	//从队列移出线程
	INT			DeleteFromDeque( CNetThread* pclsThread );
private:	//变量
	deque<pstruThreadPoolTask>				m_DequeTask;			//任务队列
	vector<CNetThread*>						m_VectorThread;			//线程队列
	vector<CNetThread*>						m_VecBusyThread;		//忙碌线程队列
	vector<CNetThread*>						m_VecIdleThread;		//空闲线程队列
	// 等待删除的线程队列
	vector<CNetThread*>						m_VecWaitDelThread;

	UINT									m_uiMaxThreadCount;		//最大线程数
	UINT									m_uiCurThreadCount;		//当前线程数
	CGSMutex								m_GSMutexDequeTask;		//任务队列锁
	CGSMutex								m_GSMutexDequeThread;	//线程队列锁
	bool									m_bIsExitWorkerThreadProc;//置线程退出标志
	INT										m_iIdleThreadMaxNum;		//空闲线程数最大值 高于此值应减少线程
	INT										m_iIdleThreadMinNum;		//空闲线程数最小值 低于此值应增加线程

	
	
	// 待删除的线程计数
	INT										m_iWaitDelThreadNum;

	// 信号量
	CGSCond									m_GSCond;		

	// 管理线程
	CNetThread								m_clsManagerThread;

	// 退出计时
	UINT64									m_unExitTick;

	// 管理线程退出标志	TRUE:已退出   FALSE：未退出
	BOOL									m_bIsExitManangeThread;

	// 日志指针
	ILogLibrary*							m_clsLogPtr;

public:
	UINT16									m_ExitThreadCount;


};

}

#endif

