#include "GSCommon.h"
#include "NetClient.h"

CNetClient::CNetClient()
{
	m_pSockCli = NULL;
	m_bIsExit = FALSE;
}

CNetClient::~CNetClient()
{
	StopNetService();
}

void CNetClient::SetNetConfig(INetConfig* pCfg)
{
	CNetCommuBase::SetNetConfig(pCfg);
}
INetConfig* CNetClient::GetNetConfig()
{
	return CNetCommuBase::GetNetConfig();
}

INetChannel* CNetClient::AddNetChannel(char *pszRemoteIPAddr, 
									   UINT16 uiRemotePort, 
									   char *pszBindingIPAddr, 
									   UINT16 uiBindingPort)
{
	ISocketChannel* pSockChn = NULL;
	CNetChannel* pNetChn;
	INT32 iRet;
	enumNetProtocolType euPType;
	INT32 iStatus;

	if(NULL == m_pSockCli)
	{
		return NULL;
	}

	if(GetProtocolType() == PROTOCOL_TYPE_TCP)
	{
		euPType = NET_PROTOCOL_TCP;
	}
	else
	{
		euPType = NET_PROTOCOL_UDP;
	}
	iRet = m_pSockCli->AddClientChannel(pszRemoteIPAddr, 
											uiRemotePort, 
											pszBindingIPAddr,
											uiBindingPort, euPType, &pSockChn);
	if(NULL == pSockChn || iRet != ERROR_BASE_SUCCESS)
	{
		iStatus = NET_CHANNEL_STATUS_SHUTDOWN;
	}
	else
	{
		iStatus = NET_CHANNEL_STATUS_ACTIVE;
	}
	
	pNetChn = InsertNetChannel(pSockChn, iStatus);
	if(NULL == pNetChn)
	{
		if(pSockChn)
		{
			pSockChn->CloseChannelEx();
		}
		return NULL;
	}
	pNetChn->SetChnType(NET_CHANNEL_TYPE_CLIENT);
	pNetChn->SetNetService(m_pSockCli);
	pNetChn->SetRemoteIPAddrString(pszRemoteIPAddr);
	pNetChn->SetRemotePort(uiRemotePort);
	pNetChn->SetLocalIPAddrString(pszBindingIPAddr);
	pNetChn->SetLocalPort(uiBindingPort);
	//iRet = pNetChn->ConnectServer(pszRemoteIPAddr, uiRemotePort, pszBindingIPAddr, uiBindingPort);
	return pNetChn;
}

INT32 CNetClient::SocketEventCB(ISocketChannel* pChn, void* pCaller, enumNetEventType iEvent, void* pData, UINT32 iDataLen)
{
	CNetClient* pCli;

	pCli = (CNetClient*)pCaller;
	if(NULL == pCli)
	{
		return -1;
	}

	return pCli->OnSocketEvent(pChn, iEvent, pData, iDataLen);
}

INT32 CNetClient::StartNetService(char* pszLogPath)
{
	INT32 iRet;

	if(NULL != m_pSockCli)
	{
		return 0;
	}

	iRet = StartNetBase();
	if(iRet < 0)
	{
		StopNetService();
		return -1;
	}

	//m_pSockCli = CreateSocketClient();
	m_pSockCli = CreateNetService();
	if(NULL == m_pSockCli)
	{
		StopNetService();
		return -2;
	}
	m_pSockCli->InitLog(pszLogPath);
	m_pSockCli->SetReConnect(FALSE);
	m_pSockCli->SetOnEventCallBack(this, SocketEventCB);
	iRet = m_pSockCli->InitSimpleNetService();// ³Â´ÏÐÞ¸Ä 20120521
	if(iRet != ERROR_BASE_SUCCESS)
	{
		StopNetService();
		return -4;
	}
	return 0;
}

INT32 CNetClient::StopNetService()
{
	if ( m_bIsExit ) return 0;

	CGSAutoMutex		GSAutoMutex(&m_GSMutex);

	if ( m_bIsExit ) return 0;

	m_bIsExit = TRUE;

	if(m_pSockCli)
	{
		ClearAllChn();
		m_pSockCli->StopNetService();
		delete m_pSockCli;
		m_pSockCli = NULL;
	}
	StopNetBase();
	return 0;
}