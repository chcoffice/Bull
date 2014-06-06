#include "GSCommon.h"
#include "BaseChannel.h"
#ifdef _WIN32
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

CBaseChannel::CBaseChannel()
{
	m_uiRemoteIPAddr = 0;
	m_uiLocalIPAddr = 0;
	m_uiRemotePort = 0;
	m_uiLocalPort = 0;

	strcpy(m_szRemoteIPAddr, "0.0.0.0");
	strcpy(m_szBindingIPAddr, "0.0.0.0");

	memset(m_szLogicRemoteIPAddr,0x0,16);
}

CBaseChannel::~CBaseChannel()
{

}


char* CBaseChannel::BaseGetRemoteIPAddrString()
{
	return m_szRemoteIPAddr;
}
UINT32 CBaseChannel::BaseGetRemoteIPAddr()
{
	return inet_addr(m_szRemoteIPAddr);
}
char* CBaseChannel::BaseGetLocalIPAddrString()
{
	return m_szBindingIPAddr;
}
UINT32 CBaseChannel::BaseGetLocalIPAddr()
{
	return inet_addr(m_szBindingIPAddr);
}
UINT16 CBaseChannel::BaseGetRemotePort()
{
	return m_uiRemotePort;
}
UINT16 CBaseChannel::BaseGetLocalPort()
{
	return m_uiLocalPort;
}






void CBaseChannel::SetRemoteIPAddrString(char* pszIPAddr)
{
	if(NULL != pszIPAddr)
	{
		strcpy(m_szRemoteIPAddr, pszIPAddr);
	}
}
void CBaseChannel::SetRemoteIPAddr(UINT32 uiIPAddr)
{
	m_uiRemoteIPAddr = uiIPAddr;
}
void CBaseChannel::SetLocalIPAddrString(char* pszIPAddr)
{
	if(NULL != pszIPAddr)
	{
		strcpy(m_szBindingIPAddr, pszIPAddr);
	}
}
void CBaseChannel::SetLocalIPAddr(UINT32 uiIPAddr)
{
	m_uiLocalIPAddr = uiIPAddr;
}
void CBaseChannel::SetRemotePort(UINT16 uiPort)
{
	m_uiRemotePort = uiPort;
}
void CBaseChannel::SetLocalPort(UINT16 uiPort)
{
	m_uiLocalPort = uiPort;
}
void CBaseChannel::SetLogicRemoteIPAddrString(const char* pszIPAddr)
{
	if(NULL != pszIPAddr)
	{
		memset(m_szLogicRemoteIPAddr,0x0,16);
		strncpy(m_szLogicRemoteIPAddr,pszIPAddr,16);
		m_szLogicRemoteIPAddr[15] = '\0';
	}
	
}
char* CBaseChannel::GetLogicRemoteIPAddrString()
{
	return m_szLogicRemoteIPAddr;

}