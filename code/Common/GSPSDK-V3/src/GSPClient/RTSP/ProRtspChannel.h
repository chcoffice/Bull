/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : PRORTSPCHANNEL.H
Author :  zouyx
Version : 0.0.0.0
Date: 2012/4/20 14:04
Description: RTSP 客户端通道协议的实现
********************************************
*/

#ifndef _GS_H_PRORTSPCHANNEL_H_
#define _GS_H_PRORTSPCHANNEL_H_


#include "../IProChannel.h"
#include "MySocket.h"
#include "MainLoop.h"
#include "RTSPStru.h"



namespace GSP
{
	class  CClientChannel;
	class  CClientSection;

	namespace RTSP
	{


		class CRtspTcpDecoder;
		class CRtspIPacket;
		class CRtspHeader;
		class CRtspSyncWaiter;

#define CLICHN_KEEPALIVE_TIMER_ID 1
#define CLICHN_ACTIVE_TIMER_ID  2
#define CLICHN_CLOSE_TIMER_ID  3



class CRtspChannel :
    public CIProChannel
{
public:
    typedef enum
    {
       MY_ST_INIT,
       MY_ST_READY,
       MY_ST_WREQUEST,
       MY_ST_PLAYING,
       MY_ST_ERR,
       MY_ST_ASSERT,
    }EnumMyStatus;

    typedef enum
    {
        CLOSE_NONE,
        CLOSE_SUCCESS,    //正常
        CLOSE_EINVALID_DATA,   //接收到非法数据
        CLOSE_EILLEGAL_OPERATION, //非法操作
        CLOSE_EIO, //IO出错
        CLOSE_EPERMIAT, //无权限
        CLOSE_ASSERT,
        CLOSE_REMOTE, //远端关闭
        CLOSE_EREQUEST,
        CLOSE_EKEEPALIVE, //KEEPALIVE 超时
    }EnumCloseType;

private :
		
	CClientChannel *m_pParent;
	CISocket *m_pRtspTcpSocket;
	const UINT32 m_iAutoID;
	
	GSAtomicInter  m_iCTagSequence; //命令CSeq

	CGSWRMutex m_csAsyncWRMutex; // m_csAsyncCmdList 同步锁
	CList m_csAsyncCmdList;  //存储的是 CRtspSyncWaiter*
	

	BOOL m_bRtspTcpSending;    //标准TCP 是否正在发送
	CList m_csRtspTcpWaitSendQueue;  //Tcp等待发送数据的队列, 数据为 CIPacket *

	BOOL m_bRtspWaitSendFinish;

	CGSPThreadPool m_csRtspTcpRcvTask; //处理接收到得数据线程
	CRtspTcpDecoder *m_pRtspTcpDecoder; //TCP 端协议解析

	

	INT m_iMaxKeepaliveTimeouts;

	CWatchTimer m_csSendKeepaliveTimer; //发送keepalive 

	CWatchTimer m_csAsyncWaiterTimer; //同步检测定时器

	CWatchTimer m_csKeepaliveTestTimer; //心跳检测

	INT m_iKeepalivePlugs; //心跳检测计数
	INT m_iCurSetupChn;
	CGSWRMutex m_csWRMutex;
	
public  :
    //以下是接口实现
	EnumErrno Open(const CUri &csUri, BOOL bAsyn, INT iTimeouts);
	EnumErrno Ctrl(const StruGSPCmdCtrl &stCtrl, BOOL bAsync,INT iTimeouts);
	virtual void DestoryBefore(void);

	virtual EnumErrno FlowCtrl( BOOL bStart );

	/*
	*********************************************************************
	*
	*@brief : 新增加接口
	*
	*********************************************************************
	*/
	CRtspChannel(CClientChannel *pParent);

	virtual ~CRtspChannel(void);


	void OnAsyncWaiterEvent(CGspSyncWaiter *pWaiter);

private :
	// TCP 连接Socket 处理回调 
	void *OnRtspTcpSocketEvent(	CISocket *pSocket, 
							EnumSocketEvent iEvt,
							void *pParam, void *pParamExt );

	//处理TCP 写事件
	CIPacket *HandleRtspTcpSocketWriteEvent( CIPacket *pPacket );

	//处理TCP 读事件
	BOOL HandleRtspTcpSocketReadEvent(CRefBuffer *pBuffer);


	//TCP 接收线程池 回调
	void OnRtspTcpRcvTaskPoolEvent( CObjThreadPool *pTkPool, void *pData );



	void WakeupAsync(EnumRTSPComandMask iCommandID, INT32 iCSeq, EnumErrno eErrno );

	//定时器回调
	void OnTimerEvent( CWatchTimer *pTimer );

    EnumErrno FlowCtrl( BOOL bStart );


	EnumErrno ParserRequest( EnumRTSPComandMask eCmdJD,  CRtspIPacket **ppResult, INT32 &iCSeq );

	EnumErrno SendRtspCmdPacket( CRtspIPacket *pPacket );

	EnumErrno ParserCtrl(const StruGSPCmdCtrl &stGssCtrl, CRtspIPacket **ppResult, INT32 &iCSeq);

	void HandleRtspTcpPacket(CRtspIPacket *pPacket);

	CRtspSyncWaiter *GetSyncWaiter( INT32 iCSeq );


	void HandleOptionsResponse(CRtspSyncWaiter *pAsync, CRtspHeader *PHeader);
	void HandleDescribeResponse(CRtspSyncWaiter *pAsync, CRtspHeader *PHeader, 
								const CGSString &strContent);
	void HandleSetupResponse(CRtspSyncWaiter *pAsync, CRtspHeader *pHeader);
};

} //end namespace GSP

} //end namespace RTSP

#endif //end _GS_H_PRORTSPCHANNEL_H_
