#ifndef NET_CLIENT_DEF_H
#define NET_CLIENT_DEF_H

#include "NetCommonStack.h"
#include "NetCommuBase.h"
#include "INetService.h"

class CNetClient : public INetClient, public CNetCommuBase
{
public:
	CNetClient();
	virtual ~CNetClient();

public:
	virtual void SetNetConfig(INetConfig* pCfg);
	virtual INetConfig* GetNetConfig();

	virtual INT32 StartNetService(char* pszLogPath = NULL);
	virtual INT32 StopNetService();

	virtual INetChannel* AddNetChannel(char* pszRemoteIPAddr, UINT16 uiRemotePort, 
		char* pszBindingIPAddr, UINT16 uiBindingPort);
protected:
	static INT32 SocketEventCB(ISocketChannel* pChn, void* pCaller, enumNetEventType iEvent, void* pData, UINT32 iDataLen); 
protected:
	INetService* m_pSockCli;
	// 保证线程安全
	CGSMutex				m_GSMutex;
	// 退出标志 TRUE 退出 FALSE 
	BOOL					m_bIsExit;
};



#endif