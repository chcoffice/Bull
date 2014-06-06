#include "GSCommon.h"
#include "NetServer.h"
#include "Log.h"

CNetServer::CNetServer()
{
	m_pISockSvr = NULL;
	m_vecListenChnl.clear();
}

CNetServer::~CNetServer()
{
	StopListen();
}

void CNetServer::SetNetConfig(INetConfig* pCfg)
{
	CNetCommuBase::SetNetConfig(pCfg);
}
INetConfig* CNetServer::GetNetConfig()
{
	return CNetCommuBase::GetNetConfig();
}

INT32 CNetServer::SocketEventCB(ISocketChannel* pChn, void* pCaller, enumNetEventType iEvent, void* pData, UINT32 iDataLen)
{
	CNetServer* pSvr;

	pSvr = (CNetServer*)pCaller;
	if(NULL == pSvr)
	{
		return -1;
	}

	return pSvr->OnSocketEvent(pChn, iEvent, pData, iDataLen);
}

INT32 CNetServer::StartListen(char* pszBindingIPAddr, UINT16 uiBindingPort, char* pszLogPath)
{
	//ISocketConfig* pSockCfg;
	INT32 iRet;
	enumNetProtocolType euPType;
	ISocketChannel* pChn = NULL;

	if(NULL != m_pISockSvr)
	{
		return 0;
	}

	iRet = StartNetBase();
	if(iRet < 0)
	{
		StopListen();
		return -1;
	}

	m_pISockSvr = CreateNetService();
	//m_pISockSvr = CreateSocketServer();
	if(NULL == m_pISockSvr)
	{
		StopListen();
		return -2;
	}
	if(pszLogPath)
	{
		m_pISockSvr->InitLog(pszLogPath);
	}
	m_pISockSvr->SetOnEventCallBack(this, SocketEventCB);
	iRet = m_pISockSvr->InitNetService();
	if(iRet != ERROR_BASE_SUCCESS)
	{
		m_pISockSvr->StopNetService();
		delete m_pISockSvr;
		m_pISockSvr = NULL;
		StopListen();
		return -3;
	}

	if(GetProtocolType() == PROTOCOL_TYPE_TCP)
	{
		euPType = NET_PROTOCOL_TCP;
	}
	else
	{
		euPType = NET_PROTOCOL_UDP;
	}
	iRet = m_pISockSvr->AddServerChannel(pszBindingIPAddr, uiBindingPort, euPType, &pChn);
    LOG2_DEBUG("Cmm Server %s:%d  Start %s\n",
        pszBindingIPAddr, uiBindingPort, iRet==ERROR_BASE_SUCCESS ? "OK" : "Fail");
	if (pChn != NULL && iRet==ERROR_BASE_SUCCESS  )
	{
		m_vecListenChnl.push_back(pChn);
	}
	return iRet;
}
INT32 CNetServer::StopListen()
{
	if(m_pISockSvr)
	{
		for (size_t i=0; i<m_vecListenChnl.size(); i++)
		{
			m_vecListenChnl[i]->CloseChannel();
		}
		m_vecListenChnl.clear();

		ClearAllChn();
		m_pISockSvr->StopNetService();
		delete m_pISockSvr;
		m_pISockSvr = NULL;
	}
	StopNetBase();
	return 0;
}