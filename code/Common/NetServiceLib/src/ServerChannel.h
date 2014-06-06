#if !defined (ServerChannel_DEF_H)
#define ServerChannel_DEF_H
/***************************************************************************************************
	Copyright (C), 1997-2010, gosun Tech. Co., Ltd.
	FileName: 		ServerChannel.h
	Author:        	陈聪
	Version :       v1.0   
	Date:			2010/05/25
	Description:     // 模块描述      

****************************************************************************************************/

#include "ICreater.h"
#include "SocketChanel.h"

namespace NetServiceLib
{
class CServerChannel : public CSocketChannel,public ICreater 
{
public:
	CServerChannel(){};
	virtual ~CServerChannel(){};

	INT		CreaterTcpChannel(CBaseSocket* pclsSocket);
	INT		CreaterUdpChannel(CBaseSocket* pclsSocket);
public:
	INT		CreateChannel(CBaseSocket* pclsSocket);


};

class CClientChannel : public ICreater, public CSocketChannel
{
public:
	CClientChannel(){};
	virtual ~CClientChannel(){};
	INT		CreateChannel(CBaseSocket* pclsSocket);
	INT		CreaterTcpChannel(CBaseSocket* pclsSocket);//按照客户端的步骤建立udp通道
	INT		CreaterUdpChannel(CBaseSocket* pclsSocket);//按照客户端的步骤建立UDP通道

};
}

#endif


