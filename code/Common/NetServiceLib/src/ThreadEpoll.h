#if !defined (ThreadEpoll_DEF_H)
#define ThreadEpoll_DEF_H

#if OPERATING_SYSTEM

#elif _LINUX

#include "LinuxEpoll.h"
#include "CommunicationManager.h"
#include "ThreadPoolModule.h"

namespace NetServiceLib
{

class CThreadDealNetEvent :
	public CLinuxEpoll, public CCommunicationManager
{
public:
	CThreadDealNetEvent(void){};
	virtual ~CThreadDealNetEvent(void){};
	//处理接收数据
	INT ThreadAcceptData(enumThreadEventType enumEvent, void* pObject);		
	//线程执行函数 用于轮询linux的epoll事件
	INT  EpollWaitEvent(enumThreadEventType enumEvent, void* pObject);
private:
	// 增加事件响应 继承自CCommunicationManager
	virtual	void OnEventModel( CSocketChannel* pclsSocketChannel );
public:
	CThreadPoolModule			m_clsThreadPool;

};

}
#endif

#endif


