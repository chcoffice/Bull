/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : GSPSRVSESSION.H
Author :  zouyx
Version : 0.0.0.0
Date: 2012/3/20 10:56
Description: GSP 协议会话
********************************************
*/

#ifndef _GS_H_GSPSRVSESSION_H_
#define _GS_H_GSPSRVSESSION_H_

#include "../IProServer.h"
#include "ThreadPool.h"
#include "MainLoop.h"
#include "ISocket.h"
#include "../RefSource.h"
#include "GSPMemory.h"
#include "CircleQueue.h"
#include "RTP/RtpNet.h"
#include "AsyncTcpSrvSocket.h"


namespace GSP
{
class CGspServer;
class CGspTcpDecoder;
class CGspCommand;
class CIMediaInfo;



class CGspSrvSession :
	public CISrvSession
{
private :
	//控制信号
	typedef enum
	{
		eSIG_NONE = 0,
		eSIG_RELEASE, //删除
		eSIG_REMOTE_DISCONNECT, //对端关闭连接
		eSIG_ASSERT, //异常
		eSIG_REMOTE_CLOSE, //对端关闭
	}EnumSignal;

	//状态
	typedef enum
	{
		eST_NONE = 0,   //初始
		eST_WAIT_REQUEST, //等待URI请求
		eST_WAIT_START, //等待开始
		eST_PLAYING, //播放中
		eST_ASSERT, //异常
	}EnumStatus;

	static const UINT32 INVALID_COMMAND_TAG = MAX_UINT32;

	CGspServer *m_pGspServer;
	CGspTcpDecoder *m_pTcpDecoder; //TCP 端协议解析
//	CGSPThreadPool m_csTcpTask; // TCP数据处理线程池, 为 CRefBuffer *

	CGSWRMutex m_csWRMutex;
	CWatchTimer m_csKeepaliveTimer; //活动检查定时器	
	INT m_iKeepalivePlugs;
	CWatchTimer m_csSendKeepaliveTimer; //发送Keepalive Timer 
	CISocket *m_pTcpSocket;
	CGSPString m_strURI;

	
	BOOL m_bTcpSending;    //标准TCP 是否正在发送
	CList m_csTcpWaitSendQueue;  //Tcp等待发送数据的队列, 数据为CProFrame *
	INT64 m_iWaitSendSize; //等待发送的数据大小


	CList m_csStreamWaitSendQueue;  //流等待发送数据的队列, 数据为CProFrame *， 当使用TCP 模式有效	
	INT64 m_iWaitSendStreamSize;
	BOOL m_bStreamTcpSending;    //流TCP 是否正在发送

	EnumStatus m_eStatus;

	CRefSource *m_pRefSource; //引用的数据源

	StruPlayStatus m_stPlayStatus;  //当前播放状态

	INT32 m_eTransModel; //传输模式

	StruGSPPacketHeader m_stProCmdHeader; //命令的协议头
	StruGSPPacketHeader m_vProStreamHeader[GSP_MAX_MEDIA_CHANNELS]; //命令的协议头

	GSAtomicInter m_iTagSequence; //命令号序列

	BOOL m_bWaitSendFinish; //等待发送完成
	
	BOOL m_bStopStream;  //停了流
	
	EnumSignal m_eSignal;

	

	BOOL  m_bCtrlIng; //当前正在进行控制, 保证CTRL 返回前不发送流数据
	BOOL  m_bPlayCtrlIng; //当前正在进行播放控制, 保证CTRL 返回前不发送流数据

	INT m_eCtrlIngCmd; //当前控制命令

	

	BOOL m_bPlayEnd;

	UINT32 m_iSendKeySeq;

	INT m_iGspVersion; 


	INT m_eGspStreamTransMode; //流传输模式

	

	RTP::CRtpUdpSender  *m_pRtpUdpSender; //使用RTP_UDP 模式有效
	CISocket *m_pTcpStreamSender;  //使用RTP_TCP 时有效
	CAsyncTcpSrvSocket *m_pAsyncSrvSocket; //使用RTP_TCP 时有效
	BOOL m_bFirstFrame; //第一帧 
	

	CFrameCache* m_pSysFrame; //系统头
	//std::set<UINT32> m_setWaitingWrite;

	int m_bFlowCtrl; //正在流控 
public:
	CGspSrvSession( CGspServer *pProServer);
	virtual ~CGspSrvSession(void);

	
	EnumErrno Init( CISocket *pSocket );


	/*
	*********************************************************************
	*
	*@brief :  接口
	*
	*********************************************************************
	*/
	virtual void DeleteBefore(void); //删除前调用
	virtual void Start(void);

	
private :

	
	// TCP 连接Socket 处理回调 
	void *OnTcpSocketEvent(	CISocket *pSocket, 
		EnumSocketEvent iEvt,
		void *pParam, void *pParamExt );

	// Stream TCP 连接Socket 处理回调 
	void *OnStreamTcpSocketEvent(	CISocket *pSocket, 
		EnumSocketEvent iEvt,
		void *pParam, void *pParamExt );

	//Async  TCP Server 连接Socket 处理回调 
	void *OnATcpSrvSocketEvent(	CISocket *pSocket, 
		EnumSocketEvent iEvt,
		void *pParam, void *pParamExt );

	void OnTimerEvent( CWatchTimer *pTimer );
	
	//处理TCP 写事件
	void *HandleTcpSocketWriteEvent( const StruAsyncSendEvent *pEvt );

	//处理TCP 读事件
	BOOL HandleTcpSocketReadEvent(CGSPBuffer *pBuffer);


	//命令处理
	void HandleCommand( CGspCommand *pCommand );


	//处理 URI 请求命令 GSP_CMD_ID_REQUEST
	void HandleRequest( CGspCommand *pCommand  );
	
	//处理 RTP UDP 传输请求 GSP_UDP_SET_SETUP
	void HandleUdpSetupRequest( CGspCommand *pCommand );

	//处理Keepalive 命令 GSP_CMD_ID_KEEPAVLIE
	void HandleKeepalive( CGspCommand *pCommand   );

	//处理重传命令  GSP_CMD_ID_RESEND
	void HandleRequestResend(CGspCommand *pCommand  );  

	//处理控制命令  GSP_CMD_ID_CTRL
	void HandleCtrl(CGspCommand *pCommand   );

	//处理状态请求命令 GSP_CMD_ID_REQUEST_STATUS
	void HandleRequestStatus(CGspCommand *pCommand  );

	//处理关闭强求 GSP_CMD_ID_CLOSE
	void HandleRequestClose(CGspCommand *pCommand  );

	//处理异常关闭强求, 不用回复 GSP_CMD_ID_ASSERT_AND_CLOSE
	void HandleRemoteAssert(CGspCommand *pCommand  );

	//统一处理没有处理或不认识的命令
	void HandleUnknownCommand( CGspCommand *pCommand  );


	void SendCommand(EnumGSPCommandID eCommandID, const void *pCommandPlayload, UINT iSize, 
					 UINT32 iTag = INVALID_COMMAND_TAG);


	void SendRequestResponse( UINT16 iErrno,UINT32 iTag, UINT32 iAbilities = 0,
							const CIMediaInfo  *pMediaInfo = NULL);

	void Signal(EnumSignal eSig );


	//停止说有操作
	void StopAllAction(void);

	//数据源事件
	EnumErrno OnSourceEvent(CRefSource *pRefSource,
							CRefSource::EnumRefSourceEvent eEvt, void *pParam);

	//发送流数据帧
	EnumErrno SendStreamFrame( CFrameCache* pFrame );

	//复合通道发送流数据
	EnumErrno SendMultiTcpStreamFrame(CFrameCache* pFrame, int iChnNo);

	//RTPUDP 通道发送数据
	EnumErrno SendRtpUdpStreamFrame(CFrameCache* pFrame, int iChnNo);


	//独立 TCP 通道发送数据
	EnumErrno SendTcpStreamFrame(CFrameCache* pFrame, int iChnNo);

};

} //end namespace GSP

#endif //end _GS_H_GSPSRVSESSION_H_
