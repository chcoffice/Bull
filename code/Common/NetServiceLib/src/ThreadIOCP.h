#if !defined (ThreadIOCP_DEF_H)
#define ThreadIOCP_DEF_H

#include "IOCP.h"
#include "CommunicationManager.h"
#include "ThreadPoolModule.h"

#if OPERATING_SYSTEM

#elif _WIN32


namespace NetServiceLib
{

class CNetService;
class CThreadDealNetEvent :
	public CIOCP, public CCommunicationManager
{
public:
	CThreadDealNetEvent(void);
	virtual ~CThreadDealNetEvent(void); 
	//处理接收数据
	INT ThreadAcceptData(enumThreadEventType enumEvent, void* pObject);	
	// 处理退出
	INT	ExitAcceptData(INT32 iThreadCount);
private:
	// 增加事件响应 继承自CCommunicationManager
	virtual	void OnEventModel( CSocketChannel* pclsSocketChannel );
public:
	CThreadPoolModule	m_clsThreadPool;

	
};

}

#endif



#endif //#if !defined (ThreadIOCP_DEF_H)

