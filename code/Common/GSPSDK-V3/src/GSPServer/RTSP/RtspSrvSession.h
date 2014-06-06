/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : RTSPSRVSESSION.H
Author :  zouyx
Version : 0.0.0.0
Date: 2012/4/18 10:31
Description: RTSP 会话
********************************************
*/

#ifndef _GS_H_RTSPSRVSESSION_H_
#define _GS_H_RTSPSRVSESSION_H_



#include "../IProServer.h"
#include "ThreadPool.h"
#include "MainLoop.h"
#include "ISocket.h"
#include "../RefSource.h"
#include "RTSP/RTSPStru.h"
#include "RTSP/RTSPAnalyer.h"
#include "RTP/SdpParser.h"
#include "MediaInfo.h"
#include "StreamPackConvert.h"
#include "RTP/RtpNet.h"

namespace GSP
{




namespace RTSP
{

class CRtspServer;


class CRtspSrvSession :
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

	CGSString m_strSessionID;

	CRtspServer *m_pRtspServer;
	CRtspTcpDecoder *m_pRtspTcpDecoder;





	BOOL m_bRtspTcpSending;    //标准TCP 是否正在发送
	CList m_csRtspTcpWaitSendQueue;  //Tcp等待发送数据的队列, 数据为 CIPacket *
	BOOL m_bRtspTcpWaitSendFinish; //等待发送完成
	INT64 m_iWaitSendSize; //等待发送的数据大小

	EnumHTTPState m_eHttpStatus;
	EnumTransportMode m_eTransport;
	EnumRTSPComandMask m_eCurAllowMask; //当前运行的命令

	
	

	CGSWRMutex m_csWRMutex;
	
	INT m_iKeepalivePlugs;
	CWatchTimer m_csSendKeepaliveTimer; //发送Keepalive Timer 
	CISocket *m_pRtspTcpSocket;
	
	BOOL m_bStopStream;

	EnumSignal m_eSignal;
	
	CWatchTimer m_csKeepaliveTimer; //活动检查定时器	
	StruPlayStatus m_stPlayStatus;  //当前播放状态
	INT32 m_eTransModel; //传输模式 GSP 的播放模式


	CRefSource *m_pRefSource; //引用的数据源


	CSdpParser m_csSdp; //sdp 描述


	EnumRTPPayloadType m_eOutRtpPlayloadType; 	

	CStreamPackConvert   *m_pPkgCvt;  //格式转换器
	EnumStreamPackingType m_eInputStreamPkt; //接受流的封装类型
	EnumStreamPackingType m_eOutputStreamPkt; //输出打包厂商类型	
	double m_fSpeed; //当前速度
	BOOL m_bFirstFrame;

	UINT m_iSNSeq; //SN 计数
	INT64 m_iFileBegin; //文件时长
	INT64 m_iFileEnd; //文件时长

	CGSString m_strRemoteRtpIp; //对端 Rtp Ip
	int m_iRemoteRtpPort; //对端 rtp port
	CRtpUdpSender *m_pRtpSender;
	
	UINT32 m_iSendKeySeq;

	BOOL m_bCtrlIng;

	BOOL m_bPlayEnd;

	CGSString m_strUserAgent;

	BOOL m_bSteup;
public:
	CRtspSrvSession( CRtspServer *pProServer);
	virtual ~CRtspSrvSession(void);

	
	EnumErrno Init( CISocket *pRtspTcpSocket );


	/*
	*********************************************************************
	*
	*@brief :  接口
	*
	*********************************************************************
	*/
	virtual void DeleteBefore(void); //删除前调用
	virtual void Start(void);

	

	void OnRtpContentKeepalive(void); //给RTP Content 调用
private :
	//RTSP TCP 线程池 回调
//	void OnRtspTcpTaskPoolEvent( CObjThreadPool *pTkPool, void *pData );
	
	// RTSP TCP 连接Socket 处理回调 
	void *OnRtspTcpSocketEvent(	CISocket *pSocket, 
		EnumSocketEvent iEvt,
		void *pParam, void *pParamExt );

	void OnTimerEvent( CWatchTimer *pTimer );
	
	//处理TCP 写事件
	BOOL HandleRtspTcpSocketWriteEvent( StruAsyncSendEvent *pEvt );

	//处理TCP 读事件
	BOOL HandleRtspTcpSocketReadEvent(CGSPBuffer *pBuffer);


// 	//命令处理
 	void HandleRtspTcpCommand( CRtspProPacket *pPacket );


	void CloseOfBadRequest(void);
	void HandleOptions( CRtspHeader *pHeader, CRtspProPacket *pRcvPacket );
	void HandleDescribe( CRtspHeader *pHeader , CRtspProPacket *pRcvPacket );
	void HandleTeardown(CRtspHeader *pHeader, CRtspProPacket *pRcvPacket );
	void HandleSetup( CRtspHeader *pHeader, CRtspProPacket *pRcvPacket );
	void HandlePlay( CRtspHeader *pHeader, CRtspProPacket *pRcvPacket );
	void HandlePause(CRtspHeader *pHeader, CRtspProPacket *pRcvPacket );
	void HandleGetParameter( CRtspHeader *pHeader, CRtspProPacket *pRcvPacket );
	void HandleSetParameter(CRtspHeader *pHeader, CRtspProPacket *pRcvPacket);
	void HandleInvalidCmmand(CRtspHeader *pHeader, CRtspProPacket *pRcvPacket );


	void ResponseError(const CRtspHeader *pHeader, EnumResponseStatus eErrno );



	void Signal(EnumSignal eSig );


	//停止说有操作
	void StopAllAction(void);

	//数据源事件
	EnumErrno OnSourceEvent(CRefSource *pRefSource,
							CRefSource::EnumRefSourceEvent eEvt, void *pParam);

	//发送数据帧
	EnumErrno SendStreamFrame( CFrameCache* pFrame );


	void SendRtspCmdPacket( CRtspProPacket *pPacket );

	//建立数据源
	EnumErrno BuildSource(const CUri &csUri, const CRtspHeader *pHeader, const CGSString &strReqSdp  );

	//获取数据源的SDP 描述
	CGSString GetSDPDescription( CUri &csUri );

};

} //end namespace RTSP

} //end namespace GSP



#endif //end _GS_H_RTSPSRVSESSION_H_
