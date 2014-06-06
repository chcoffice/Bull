#include "ServerChannel.h"

using namespace NetServiceLib;

INT	CClientChannel::CreateChannel(CBaseSocket* pclsSocket)
{
	return pclsSocket->Visitor(this);
}
INT	CClientChannel::CreaterTcpChannel(CBaseSocket* pclsSocket)
{
	//按照客户端的步骤建立tcp通道
	pclsSocket->CreateSocket(m_szLocalIP, m_unLocalPort);
	return pclsSocket->Connect(m_szRemoteHost, m_unRemotePort);
}
INT	CClientChannel::CreaterUdpChannel(CBaseSocket* pclsSocket)
{
	//按照服务器端的步骤建立UDP通道
	return pclsSocket->CreateSocket(m_szLocalIP, m_unLocalPort);
}

