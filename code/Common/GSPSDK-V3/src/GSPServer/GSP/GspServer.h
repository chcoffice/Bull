/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : GSPSERVER.H
Author :  zouyx
Version : 0.0.0.0
Date: 2012/3/20 9:45
Description: GSP 协议服务
********************************************
*/

#ifndef _GS_H_GSPSERVER_H_
#define _GS_H_GSPSERVER_H_
#include "../IProServer.h"
#include "ISocket.h"
#include "ThreadPool.h"
#include "AsyncTcpSrvSocket.h"

namespace GSP
{
class CGspSrvSession;

class CGspServer :
	public CIProServer
{
public :
	typedef struct _StruGspServerConfig
	{
		INT iKeepaliveTimeouts;
		BOOL bEnableUDP;

		BOOL bEnableRtp;
		INT iRtpUdpPortBegin;
		INT iRtpUdpPortEnd;
	}StruGspServerConfig;
	
	
	StruGspServerConfig m_stConfig;
private :
		
	CTCPServerSocket *m_pListenSocket;	

	CGSPString m_strTcpBindIP; //TCP 监听绑定的IP
	INT  m_iTCPPort;  //TCP 监听端口

	BOOL m_bUDP; //使能 UDP 协议
	CGSPString m_strUdpBindIP; //UDP 监听绑定的IP
	INT m_iUDPPort;    //UDP 监听端口


	CGSPThreadPool m_csTaskDestorySession; //异步释放Session 线程

	CAsyncTcpSrvEventContext *m_pAsyncTcpEvtCtx; //当使用对立TCP流传输时， 建立连接端口
public:
	CGspServer(void);
	virtual ~CGspServer(void);

	/*
	*********************************************************************
	*
	*@brief : 接口实现
	*
	*********************************************************************
	*/
	virtual EnumErrno Init( CServer *pServer );
	virtual void Unint(void);

	virtual EnumErrno Start(void);

	virtual void Stop(void);	


	/*
	*********************************************************************
	*
	*@brief : 增加接口
	*
	*********************************************************************
	*/
	void AsyncDestroySession( CGspSrvSession *pSession );

	CAsyncTcpSrvSocket *CreateAsyncTcpSrvSocket(void);

private :
	void *OnTcpListenSocketEvent(CISocket *pSocket, EnumSocketEvent iEvt,void *pParam, void *pParamExt);


	void OnAsyncDelSessionTaskPoolEvent( CObjThreadPool *pTkPool, void *pData );


	
};

} //end namespace GSP

#endif //end _GS_H_GSPSERVER_H_
