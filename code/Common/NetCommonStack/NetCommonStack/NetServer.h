#ifndef NET_SERVER_DEF_H
#define NET_SERVER_DEF_H

#include "NetCommonStack.h"
#include "NetCommuBase.h"
#include "INetService.h"

class CNetServer : public INetServer, public CNetCommuBase
{
public:
	CNetServer();
	virtual ~CNetServer();

public:
	virtual void SetNetConfig(INetConfig* pCfg);
	virtual INetConfig* GetNetConfig();

	virtual INT32 StartListen(char* pszBindingIPAddr, UINT16 uiBindingPort, char* pszLogPath = NULL);
	virtual INT32 StopListen();

protected:
	static INT32 SocketEventCB(ISocketChannel* pChn, void* pCaller, enumNetEventType iEvent, void* pData, UINT32 iDataLen); 
protected:
	INetService* m_pISockSvr;
	vector<ISocketChannel*> m_vecListenChnl;
};


#endif