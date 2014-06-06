/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : SIPSRVSESSION.H
Author :  邹阳星
Version : 1.0.0.1
Date: 2012/11/7 14:08
Description: SIP 会话
********************************************
*/

#ifndef _GS_H_SIPSRVSESSION_H_
#define _GS_H_SIPSRVSESSION_H_


#include "../IProServer.h"
#include "ThreadPool.h"
#include "MainLoop.h"
#include "ISocket.h"
#include "../RefSource.h"
#include "SipStack.h"
#include "RTP/RtpNet.h"
#include "RTP/SdpParser.h"
#include "SIP/SIPParser.h"
#include "SIP/ES2PS.h"
#include "SipServer.h"
#include "MediaInfo.h"
#include "StreamPackConvert.h"

using namespace GSP::RTP;

namespace GSP
{

namespace SIP
{



class CStreamEncoder;



class CSipSrvSession :
	public CISrvSession
{

private :
	typedef enum
	{
		eSIG_NONE = 0,
		eSIG_RELEASE, //删除
		eSIG_REMOTE_DISCONNECT, //对端关闭连接
		eSIG_ASSERT, //异常
		eSIG_REMOTE_CLOSE, //对端关闭
	}EnumSignal;

	CSipServer *m_pSipSrv;
	
	CGSPThreadPool m_csHandleSipDataTask; // 处理Sip 数据 ， 参数为  StruSipData *pData

	StruPlayStatus m_stPlayStatus;  //当前播放状态

	INT32 m_eTransModel; //传输模式

	CRefSource *m_pRefSource; //引用的数据源

	CSdpParser m_csSdp; //sdp 描述

	C28181ParserSubject m_csSubject;

	CRtpUdpSender *m_pRtpSender;

	EnumSignal m_eSignal;

	CGSWRMutex m_csWRMutex;

	UINT m_iKeepalivePlugs;
	CWatchTimer m_csKeepaliveTimer; //活动检查定时器
	
	CGSString m_strRemoteRtpIp; //对端 Rtp Ip
	int m_iRemoteRtpPort; //对端 rtp port

	UINT32 m_iPSSRC;
	UINT32 m_iYSSRC;

	EnumRTPPayloadType m_eOutRtpPlayloadType; 	
	


	CFrameCache *m_pSysHeaderFrame; //系统头
	INT m_iSendSysHeaderTicks; //发送信息头计数, UDP 发送时 每 90 帧发送一个


	StruSipDialogKey m_stInviteDlgKey; //对话的Key

	UINT m_iSNSeq; //SN 计数
	INT64 m_iFileBegin; //文件时长
	INT64 m_iFileEnd; //文件时长


	//解封装
	CStreamPackConvert   *m_pPkgCvt;  //格式转换器
	EnumStreamPackingType m_eInputStreamPkt; //接受流的封装类型
	EnumStreamPackingType m_eOutputStreamPkt; //输出打包厂商类型	


	double m_fSpeed; //当前速度

	BOOL m_bFirstFrame;

	BOOL m_bTestRtcp;
public :
	CGSString m_strInviteCallId;
	SipSessionConnecter m_hSipCnner;

	CSipSrvSession(CSipServer *pParent);
	virtual ~CSipSrvSession(void);

	/*
	*********************************************************************
	*
	*@brief :  接口
	*
	*********************************************************************
	*/
	virtual void DeleteBefore(void); //删除前调用
	virtual void Start(void);


	BOOL Init(SipSessionConnecter cnner, const CGSString &strInviteCallId,const StruSipConnnectInfo &stNetInfo );


	void OnDisconnectEvent(void); //网络断开


	void HandleSipData(StruSipData *pData);


private :

	//出错是处理Sip 数据包
	void HandleSipDataOfError(StruSipData *pData, const char *czError  );

	void SendBye(void);


	//处理SIP 的线程池回调
	void OnHandleSipDataTaskPoolEvent( CObjThreadPool *pTkPool, StruSipData *pData );


	//处理Invite 请求
	void HandleRequestInvite(StruSipData *pData);

	//处理Ack 请求
	void HandleRequestAck(StruSipData *pData);

	//处理Bye 请求
	void HandleRequestBye(StruSipData *pData);

	//处理MANRTSP 请求
	void HandleMANSRTSP(StruSipData *pData);

	//建立数据源
	BOOL BuildSource( const CGSString &strSourceID, CSdpParser *pRequestSdp ); 


	void Signal(EnumSignal eSig );


	//停止说有操作
	void StopAllAction(void);


	void OnTimerEvent( CWatchTimer *pTimer );

	EnumErrno OnSourceEvent(CRefSource *pRefSource,
		CRefSource::EnumRefSourceEvent eEvt, void *pParam);

	EnumErrno SendStreamFrame( CFrameCache* pFrame );
// 
// 	EnumErrno SendPSFrame( CFrameCache* pFrame );
// 
// 	EnumErrno SendNonePSFrame(CFrameCache* pFrame);
// 
// 	EnumErrno SendConvertToStandartFrame( CFrameCache* pFrame, int isNonePS );

	//发送结束命令				
	void SendEndMessage(void);

	//Rtp 接收线程池 回调
	void OnRtpReaderEvent( EnumRtpNetEvent eEvt, void *pEvtArgs );

};


} //end namespace SIP


} //end namespace GSP

#endif //end _GS_H_SIPSRVSESSION_H_
