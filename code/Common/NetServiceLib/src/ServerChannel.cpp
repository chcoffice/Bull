#include "ServerChannel.h"

using namespace NetServiceLib;

INT	CServerChannel::CreateChannel(CBaseSocket* pclsSocket)
{
	return pclsSocket->Visitor(this);
	
}
INT	CServerChannel::CreaterTcpChannel(CBaseSocket* pclsSocket)
{
	//按照服务器端的步骤建立tcp通道
	//CTRLPRINTF("CreaterTcpChannel\n");
	return	pclsSocket->CreateSocket(m_szLocalIP, m_unLocalPort,1);
	
}
INT	CServerChannel::CreaterUdpChannel(CBaseSocket* pclsSocket)
{
	//按照服务器端的步骤建立UDP通道
	return pclsSocket->CreateSocket(m_szLocalIP, m_unLocalPort);	
}




