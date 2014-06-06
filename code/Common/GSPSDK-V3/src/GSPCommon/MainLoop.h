#ifndef GSS_MAINLOOP_DEF_H
#define GSS_MAINLOOP_DEF_H


/*
******************************************
Copyright (C), 2010-2011, GOSUN
Filename : GSPMAINLOOP.H
Author :  zouyx
Version : 0.1.0.0
Date: 2010/5/28 11:27
Description: 处理GSP事件主循环
********************************************
*/

#include "ThreadPool.h"
#include "List.h"
#include "OSThread.h"

namespace GSP
{

class  CMainLoop;

typedef INT32 TimerID_t;


class CWatchTimer;

typedef void (CGSPObject::*FuncPtrTimerCallback)( CWatchTimer *pTimer );




class CWatchTimer : public CGSPObject
{      
public :
    friend class CMainLoop;

   
	BOOL IsReady(void);

    void Stop(void);

    void Start(void);

    BOOL AlterTimer( UINT32 iNewInterval );

	INLINE TimerID_t GetID(void) const
	{
		return m_iTimerID;
	}


	CWatchTimer(void); 

	void Init(CGSPObject *pCbFuncOwner,  FuncPtrTimerCallback fnOnEvent,
		TimerID_t iTimerID, UINT32 iInterval,
		BOOL bInitStart = FALSE, CMainLoop *pMainLoop = NULL);

	~CWatchTimer(void);


private :
	void OnTaskEvent( CObjThreadPool *pcsPool, void *pTaskData );
	void Test( UINT64 iCurTime ); 

	FuncPtrTimerCallback m_fnCallback;
	CGSPObject *m_pCbOwner;
	  
	UINT32 m_iInterval;
    UINT64 m_iLastTime; 
    GSAtomicInter  m_eStatus;
	CGSPThreadPool m_csTask;
	CMainLoop *m_pMainLoop;
	TimerID_t m_iTimerID;
 
};


/*
*********************************************
ClassName : CMainLoop
DateTime : 2010/5/20 10:30
Description : 实现系统TIMER事件的类
Note :
*********************************************
*/




class CMainLoop : public CGSPObject
{

private :  
    CGSPThreadPool  m_csThPool;
    CList m_csTimerList;
	CGSWRMutex m_csListMutex;
    BOOL m_bRun;
	BOOL m_bReady;
    UINT64 iCurTime;
	static CMainLoop *s_pGMainLoop;
protected :
	friend class CWatchTimer;

	void OnThreadEntry( CObjThreadPool *pcsPool, void *pTaskData );


	void RemoveTimer( CWatchTimer *pTimer );
	void AddTimer( CWatchTimer *pTimer );
	static CMainLoop *Global(void);

 public :
	CMainLoop(INT iThreadNum = 1);
	virtual ~CMainLoop(void);
	static void InitModule(void);
	static void UninitModule(void);

};



};


#endif
