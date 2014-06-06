#if !defined (BaseSocket_DEF_H)
#define BaseSocket_DEF_H

/********************************************************************
	Copyright (C), 1997-2010, gosun Tech. Co., Ltd.
	FileName: 		BaseSocket.h
	Author:        	陈聪
	Version :       v1.0   
	Date:			2010/05/07
	Description:     描述windows linux中sokcet通讯的共有方法和属性，之后再派生不同的平台类。
					其实也就设置异步和关闭shutdown不一样
					 产生tcp或者udp端口，即可能是服务器端、也可能是客户端、由访问者决定

*********************************************************************/

#include "NetServiceDataType.h"
#include "ICreater.h"

#if _WIN32
#include "IOCP.h"
#else
#include "LinuxEpoll.h"
#endif

namespace NetServiceLib
{



class CBaseSocket
{
public:
	CBaseSocket(void);
	virtual ~CBaseSocket(void);
public:
	//派发接口
	virtual INT		Visitor(ICreater* pclsVisitor)=0;

	//创建socket 并绑定IP端口
	virtual INT		CreateSocket(const char* pszHost,UINT16 unPort, INT iExclusive=0)=0;

	virtual	INT		CloseSocket()=0;
	//连接主机
	virtual INT		Connect(const char *pszDesHost, UINT16 unDesPort)=0;

	//监听
	virtual INT		Listen()=0;

	virtual	SOCKETHANDLE		AcceptConnect(struct sockaddr* pstruAddr, INT* piAddrLen)=0;

	//TCP发送数据
	virtual INT		SendData(void*	pData, UINT	uiLen)=0;

	//TCP发送数据
	virtual INT		SendDataEx(void*	pData, UINT	uiLen)=0;

	//UDP发送数据
	virtual INT		SendDataTo(void* pData, UINT	uiLen, char* pszDesIP, UINT16 unDesPort)=0;

	virtual	INT		RecvData(void* pstruRecv)=0;

public:
	SOCKETHANDLE	GetSocket();

	void			SetSocket(SOCKETHANDLE hSocket);

	//设置socket选项
	INT				SetSockOption(INT iExclusive=0);	

	// 设置阻塞方式或非阻塞方式 FALSE 阻塞  TRUE非阻塞 这个相当于保存参数
	inline	void			SetBlockMode( BOOL bMode){ m_bBlock = bMode; };

	// 设置阻塞方式或非阻塞方式 FALSE 阻塞  TRUE非阻塞
	void			SetBlockModeEx( BOOL bMode);

	// 测试sokect是否正常 只有非阻塞模式才需要测试, 返回 0x1ffff 表示成功， 0 表示超时，  < 0 表示错误
	INT				TestSocket(INT iFlag);

	// 设置发送缓冲区大小
	INT			SetSendBuf(INT iBufSize);

	// 设置接收缓冲区大小
	INT			SetRcvBuf(INT iBufSize);

	//iTimeOut单位为秒, 返回 0x1ffff 表示成功， 0 表示超时，  < 0 表示错误
	INT			TestSocketEx(INT iFlag, INT iTimeOut, INT& iErrNo);
protected:
	virtual	INT		SetSockOptionSec()=0;

protected:
	//socket句柄
	SOCKETHANDLE		m_hSocket;	

	//只有UDP的服务器端才需要加锁，因为可能多个通道同时发信息
	CGSMutex			m_GSMutex;		

	// 非阻塞或阻塞方式	TRUE 非阻塞模式  FALSE 阻塞方式
	BOOL				m_bBlock;		
};

#if _WIN32

class CWinTcpSocket :
	public CBaseSocket
{
public:
	CWinTcpSocket(void);
	virtual ~CWinTcpSocket(void);
public:
	virtual	INT		Visitor(ICreater* pclsVisitor);
	virtual	INT		CreateSocket(const char* pszHost,UINT16 unPort, INT iExclusive=0);//创建socket 并绑定IP端口
	//绑定
	virtual	INT				BindSocket(sockaddr_in& service);

	virtual	INT		CloseSocket();//关闭接口
	virtual	INT		Connect(const char *pszDesHost, UINT16 unDesPort);//连接主机
	virtual	INT		Listen();//监听
	virtual	SOCKETHANDLE		AcceptConnect(struct sockaddr* pstruAddr, INT* piAddrLen);
	virtual	INT		SendData(void*	pData, UINT	uiLen );
	//TCP发送数据
	virtual INT		SendDataEx(void*	pData, UINT	uiLen);
	virtual	INT		SendDataTo(void*	pData, UINT	uiLen, char* pszDesIP, UINT16 unDesPort);
	virtual	INT		RecvData(void* pstruRecv);
protected:
	virtual	INT		SetSockOptionSec();
	static	INT		m_iCount;

};

class CWinUdpSocket :
	public CBaseSocket
{
public:
	CWinUdpSocket(void);
	virtual ~CWinUdpSocket(void);
public:
	INT		Visitor(ICreater* pclsVisitor);
	INT		CreateSocket(const char* pszHost,UINT16 unPort, INT iExclusive);//创建socket 并绑定IP端口
	//绑定
	INT				BindSocket(sockaddr_in& service);

	INT		CloseSocket();//关闭接口
	INT		Connect(const char *pszDesHost, UINT16 unDesPort);//连接主机
	INT		Listen();//监听
	SOCKETHANDLE		AcceptConnect(struct sockaddr* pstruAddr, INT* piAddrLen);
	INT		SendData(void*	pData, UINT	uiLen );
	//TCP发送数据
	virtual INT		SendDataEx(void*	pData, UINT	uiLen);
	INT		SendDataTo(void*	pData, UINT	uiLen, char* pszDesIP, UINT16 unDesPort);
	INT		RecvData(void* pstruRecv );
protected:
	virtual	INT		SetSockOptionSec();

};

#endif //end #if _WIN32

}

#endif



