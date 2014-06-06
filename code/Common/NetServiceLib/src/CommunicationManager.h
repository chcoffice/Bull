#if !defined (CommunicationManager_DEF_H)
#define CommunicationManager_DEF_H

/***************************************************************************************************
	Copyright (C), 1997-2010, gosun Tech. Co., Ltd.
	FileName: 		CommunicationManager.h
	Author:        	陈聪
	Version :       v1.0   
	Date:			2010/05/24
	Description:    通讯管理类，即管理接收数据、TCP监听、连接活动监测功能、关闭通道。
					按照我的设计，请谨慎使用静态成员，否则你会崩溃

****************************************************************************************************/
#include "NetInterfaceCommData.h"
#include "BaseSocket.h"
#include <queue>

#if _LINUX

#include "LinuxSocket.h"

#endif

namespace NetServiceLib
{

// 宏定义
#define WAITFOREVENT_SLEEP_SPACE(dwSleep,dwSpace) if(WaitForEvent(dwSleep,dwSpace)==-1){return -1;}
#define WAITFOREVENT_SLEEP(dwSleep) WAITFOREVENT_SLEEP_SPACE(dwSleep,10)
//#define WAITFOREVENT() WAITFOREVENT_SLEEP(10)

#define	ACTIVE_TEST_TIME		2*1000	//通道活动测试时间间隔

typedef struct	STRUSOCKETINFO
{
	SOCKETHANDLE		iSocket;
	CSocketChannel*		pListenChannel;	
	INT					nLen;
	sockaddr_in			ClientAddr;
	STRUSOCKETINFO():iSocket(NULL),pListenChannel(NULL)
	{
		nLen = sizeof(sockaddr_in);
		memset(&ClientAddr, 0x0, nLen);		
	}

}StruSocketInfo,*StruSocketInfoPtr;

typedef	queue<StruSocketInfoPtr>	SocketInfoList;

class CCommunicationManager
	: public CNetInterfaceCommData
{
public:
	CCommunicationManager(void);
	virtual ~CCommunicationManager(void);

	INT		Init();

	//处理接收数据
	virtual	INT ThreadAcceptData(enumThreadEventType enumEvent, void* pObject)=0;

	//线程执行函数 TCP
	INT  Listen(enumThreadEventType enumEvent, void* pObject);

	INT	ThreadChannelStatus(enumThreadEventType enumEvent, void* pObject);

	// 通知上层新连接到达
	INT	NoticeUpNewConnect(enumThreadEventType enumEvent, void* pObject);

#if OPERATING_SYSTEM

	virtual INT  SelectEvent(enumThreadEventType enumEvent, void* pObject)=0;

	//设置退出活动检测线程标志
	inline	void	SetExitSelectEvent(){ m_GSMutexExit.Lock(); m_bIsExitSelectEvent = false; m_GSMutexExit.Unlock(); };
#elif _LINUX

	//线程执行函数 用于轮询linux的epoll事件
	virtual	INT  EpollWaitEvent(enumThreadEventType enumEvent, void* pObject)=0;

	//设置退出活动检测线程标志
	inline	void	SetExitEpollEventWait(){ m_GSMutexExit.Lock(); m_bIsExitEpollEventWait = false;m_GSMutexExit.Unlock(); };

#endif

	//通道活动检测
	INT	ChannelActiveTest(enumThreadEventType enumEvent, void* pObject);

	//判断最大连接数
	bool	IfMaxChannelCount(){return TestMaxChannelCount();};

	//设置退出监听线程标志
	inline	void	SetExitListen(){ m_GSMutexExit.Lock(); m_bIsExitLinsten = false; m_GSMutexExit.Unlock(); };

	//设置退出监听线程标志
	inline	void	SetExitAcceptData(){ m_GSMutexExit.Lock();m_bIsExitAcceptData = false; m_GSMutexExit.Unlock();};

	//设置退出活动检测线程标志
	inline	void	SetExitActiveTest(){ m_GSMutexExit.Lock(); m_bIsExitActiveTest = false; m_GSMutexExit.Unlock(); };

	//设置退出通道状态处理线程标志
	inline	void	SetExitChannelStatus(){ m_GSMutexExit.Lock(); m_bIsExitChannelStatus = false; m_GSCond.Signal(); m_GSMutexExit.Unlock(); };

	//设置退出通知上层新连接到达标志
	inline	void	SetExitAcceptUpNotice(){ m_GSMutexExit.Lock(); m_bIsExitAcceptUpNotice = false; m_GSCondAcceptUpNotice.Signal(); m_GSMutexExit.Unlock(); };

	//释放所有附加通道
	INT		FreeAllExtraChannel(CSocketChannel* pclsSocketChannel);

	//释放全部通道
	INT		FreeAllChannel();

	//重新连接
	INT		ReConnectChannel( CSocketChannel* pclsSocketChannel );

protected:

	// UDP,根据父通道创建新通道，并设置相关属性 失败返回NULL
	CSocketChannel*		CreateUdpChannel( CSocketChannel* pParentSocketChannel, LPPER_IO_OPERATION_DATA PerIoData );

	INT		WaitForEvent( DWORD dwSleep, DWORD dwSpace);
	
	// 检查正常通道队列
	void	CheckNormalChannelDeque();

	// 检查故障通道队列
	void	CheckFaultChannelDeque();

	// 事件响应模型
	virtual	void OnEventModel( CSocketChannel* pclsSocketChannel )=0;

	//通道重连
	void	ChannelReconnect(CSocketChannel*	pclsSocketChannel);
	//断开处理
	void	DealDisnChannel( CSocketChannel*	pclsSocketChannel);

protected:
	//退出监听线程标志
	bool		m_bIsExitLinsten;
	//退出接收数据线程标志
	bool		m_bIsExitAcceptData;
	//退出活动检测线程标志
	bool		m_bIsExitActiveTest;
	//退出通道状态处理线程标志
	bool		m_bIsExitChannelStatus;
	// 退出通知上层新连接到达
	bool		m_bIsExitAcceptUpNotice;
	// 锁 此锁专职做退出标志的判断
	CGSMutex	m_GSMutexExit;
	CGSCond		m_GSCond;

	SocketInfoList		m_SocketInfoList;
	//用于m_SocketInfoList队列锁
	CGSMutex	m_GSMutexSocketInfoList;
	CGSCond		m_GSCondAcceptUpNotice;
	// 通知上层新连接到达线程是否属于休眠状态 true:休眠
	bool		m_bIsSleep;

#if _LINUX
	//线程执行函数 用于轮询linux的epoll事件
	bool		m_bIsExitEpollEventWait;
#endif
#if OPERATING_SYSTEM
	//退出selectevent线程函数
	bool		m_bIsExitSelectEvent;
#endif
private:
	
};

typedef INT (CCommunicationManager::*pThreadPoolExecute_Fn)(enumThreadEventType enumEvent, void* pObject);

}




#endif //end #if !defined (CommunicationManager_DEF_H)

