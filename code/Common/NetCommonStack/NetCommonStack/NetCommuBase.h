#ifndef NET_BASE_DEF_H
#define NET_BASE_DEF_H

#include "NetConfig.h"
#include "NetChannel.h"
#include "BufferArray.h"


class CNetCommuBase : public CNetConfig
{
public:
	CNetCommuBase();
	virtual ~CNetCommuBase();

public:
	INT32 StartNetBase();
	INT32 StopNetBase();

public:
	CNetChannel* InsertNetChannel(ISocketChannel* pChn, INT32 iNetChnStatus);
	CNetChannel* FindNetChannelBySockChn(ISocketChannel* pChn);

protected:
	
	INT32 OnSocketEvent(ISocketChannel* pChn, INT32 iEvent, void* pData, INT32 iDataLen);
#ifdef _WIN32 
	static INT32 WINAPI ChannelEventCB(void* pCaller, INetChannel* pChn, INT32 iEvent, INetMessage* pNetMessage);
#else
	static INT32 ChannelEventCB(void* pCaller, INetChannel* pChn, INT32 iEvent, INetMessage* pNetMessage);
#endif
	INT32 OnChannelEvent(CNetChannel* pChn, INT32 iEvent, INetMessage* pNetMessage);

	INT32 ClearAllChn();
protected:
	static INT32 ThreadPopMsg(void* pParam);
	INT32 PopMsg();

	static void ThreadDemonNetChn(CGSThread *gsTheadHandle,void* pParam);
	INT32 NetChnDemon();

protected:
	CNetChannel** m_pNetChannelArray;
	CGSThread m_GSPulseThread;
	CBufferArray m_BufferSlickArray;
	//CBufferStack m_BufStack;
	//ICommonThreadPool* m_pThreadScanMsg;
	//ICommonThread* m_pThreadChannelRoutine;
};


#endif