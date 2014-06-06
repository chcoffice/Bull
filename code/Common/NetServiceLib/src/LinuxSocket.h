#if !defined (LinuxSocket_DEF_H)
#define LinuxSocket_DEF_H
/********************************************************************
	Copyright (C), 1997-2010, gosun Tech. Co., Ltd.
	FileName: 		LinuxSocket.h
	Author:        	陈聪
	Version :       v1.0   
	Date:			2010/05/12
	Description:     // 模块描述      

*********************************************************************/
#if _LINUX

#include "BaseSocket.h"

namespace NetServiceLib
{


class CLinuxTcpSocket :
	public CBaseSocket
{
public:
	CLinuxTcpSocket(void);
	virtual ~CLinuxTcpSocket(void);
public:
	INT		Visitor(ICreater* pclsVisitor);
	INT		CreateSocket(const char* pszHost,UINT16 unPort, INT iExclusive=0);//创建socket 并绑定IP端口
	INT		CloseSocket();//关闭接口
	INT		Connect(const char *pszDesHost, UINT16 unDesPort);//连接主机
	INT		Listen();//监听
	SOCKETHANDLE		AcceptConnect(struct sockaddr* pstruAddr, INT* piAddrLen);
	INT		SendData(void*	pData, UINT	uiLen);
	//TCP发送数据
	virtual INT		SendDataEx(void*	pData, UINT	uiLen);
	INT		SendDataTo(void*	pData, UINT	uiLen, char* pszDesIP, UINT16 unDesPort);
	INT		RecvData(void* pstruRecv);
protected:
	virtual	INT		SetSockOptionSec();



};

class CLinuxUdpSocket :
	public CBaseSocket
{
public:
	CLinuxUdpSocket(void);
	virtual ~CLinuxUdpSocket(void);
public:
	INT		Visitor(ICreater* pclsVisitor);
	INT		CreateSocket(const char* pszHost,UINT16 unPort);//创建socket 并绑定IP端口
	INT		CloseSocket();//关闭接口
	INT		Connect(const char *pszDesHost, UINT16 unDesPort);//连接主机
	INT		Listen();//监听
	SOCKETHANDLE		AcceptConnect(struct sockaddr* pstruAddr, INT* piAddrLen);
	INT		SendData(void*	pData, UINT	uiLen);
	//TCP发送数据
	virtual INT		SendDataEx(void*	pData, UINT	uiLen);
	INT		SendDataTo(void*	pData, UINT	uiLen, char* pszDesIP, UINT16 unDesPort);
	INT		RecvData(void* pstruRecv);
protected:
	virtual	INT		SetSockOptionSec();

};
#endif	//end

}

#endif	//end #if !defined (LinuxSocket_DEF_H)	


