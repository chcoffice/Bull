/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : PROGSPCHANNEL.H
Author :  zouyx
Version : 0.0.0.0
Date: 2012/3/28 17:35
Description: GSP 客户端通道协议的实现
********************************************
*/

#ifndef _GS_H_PROGSPCHANNEL_H_
#define _GS_H_PROGSPCHANNEL_H_

#include "../IProChannel.h"
#include "ISocket.h"
#include "MainLoop.h"
#include "RTP/RtpNet.h"


using namespace GSP::RTP;

namespace GSP
{

class  CClientChannel;
class  CClientSection;
class  CGspSyncWaiter;
class CGspTcpDecoder;
class CGspCommand;


#define CLICHN_KEEPALIVE_TIMER_ID 1
#define CLICHN_ACTIVE_TIMER_ID  2
#define CLICHN_CLOSE_TIMER_ID  3



class CGspChannel :
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
	const static LONG INVALID_COMMAND_TAG = -1; //无效命令序号
	
	CClientChannel *m_pParent;     //执行管理的通道
	CISocket *m_pTcpSocket;				//客户端的连接SOCKET
	const UINT32 m_iAutoID;				//改对象的自增长ID， 调试使用
	StruGSPPacketHeader m_stProCmdHeader; //命令的协议头
	GSAtomicInter m_iCmdTagSequence;    //命令包序号计数

	CGSWRMutex m_csAsyncWRMutex; // m_csAsyncCmdList 同步锁
	CList m_csAsyncCmdList;  //存储的是 CGspSyncWaiter*
	

	BOOL m_bTcpSending;    //标准TCP 是否正在发送
	CList m_csTcpWaitSendQueue;  //Tcp等待发送数据的队列, 数据为 CIPacket *

	BOOL m_bWaitSendFinish;

	CGSPThreadPool m_csCommandTask; //处理命令线程
	CGspTcpDecoder *m_pTcpDecoder; //TCP 端协议解析

	

	INT m_iMaxKeepaliveTimeouts; //通信最大超时时间

	CWatchTimer m_csSendKeepaliveTimer; //发送keepalive 定时器

	CWatchTimer m_csAsyncWaiterTimer; //同步检测定时器

	CWatchTimer m_csKeepaliveTestTimer; //心跳检测

	INT m_iKeepalivePlugs; //心跳检测计数

	CGSWRMutex m_csWRMutex; //全局同步锁




	GSAtomicInter m_iSendKeySeq;

	INT m_iSysHeaderDataChn;

	INT m_iGspVersion;

	GSAtomicInter m_iSkTest;


	INT m_eGspStreamTranMode;  //流传输模式
	CTCPClientSocket *m_pTcpStreamReader;
	RTP::CRtpUdpReader *m_pRtpUdpReader;

	CGspTcpDecoder *m_pStreamTcpDecoder; //流TCP 端协议解析
		

public  :
    //以下是接口实现
	virtual EnumErrno Open(const CUri &csUri, BOOL bAsyn, INT iTimeouts);
	virtual EnumErrno Ctrl(const StruGSPCmdCtrl &stCtrl, BOOL bAsync,INT iTimeouts);
	
	virtual void DestoryBefore(void);

	virtual EnumErrno FlowCtrl( BOOL bStart );

	/*
	*********************************************************************
	*
	*@brief : 新增加接口
	*
	*********************************************************************
	*/
	CGspChannel(CClientChannel *pParent);

	virtual ~CGspChannel(void);

	/*
	 *********************************************
	 Function : OnAsyncWaiterEvent
	 DateTime : 2012/4/24 8:45
	 Description :  处理异步命令事件
	 Input :  pWaiter 异步事件同步对象
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
	void OnAsyncWaiterEvent(CGspSyncWaiter *pWaiter);

private :
	/*
	 *********************************************
	 Function : OnTcpSocketEvent
	 DateTime : 2012/4/24 8:46
	 Description : TCP 连接Socket 处理回调函数  
	 Input :  
	 Output : 
	 Return : 
	 Note :   参考 <<MySocket.h>> FuncPtrSocketEvent 描述
	 *********************************************
	 */
	void *OnTcpSocketEvent(	CISocket *pSocket, 
							EnumSocketEvent iEvt,
							void *pParam, void *pParamExt );


	//流TCP Socket 时间回调
	void *OnStreamTcpSocketEvent(	CISocket *pSocket, 
		EnumSocketEvent iEvt,
		void *pParam, void *pParamExt );

	//UDP RTP Socket 接受数据回调
	void OnRtpReaderEvent( EnumRtpNetEvent eEvt, void *pEvtArgs );


	/*
	 *********************************************
	 Function : HandleTcpSocketWriteEvent
	 DateTime : 2012/4/24 8:48
	 Description :  处理TCP 写事件
	 Input :  pPacket 返回的已经完成的数据包
	 Output : 
	 Return : 返回接着写入的数据包  参考 <<MySocket.h>> FuncPtrSocketEvent 描述
	 Note :   
	 *********************************************
	 */
	void *HandleTcpSocketWriteEvent( const StruAsyncSendEvent *pEvt );



	/*
	 *********************************************
	 Function : HandleTcpSocketReadEvent
	 DateTime : 2012/4/24 8:50
	 Description :  处理TCP 读事件
	 Input :  pPacket 读取到得数据
	 Output : 
	 Return :  TRUE/FALSE， TRUE 表示继续读取， FALSE 停止读取
					参考 <<MySocket.h>> FuncPtrSocketEvent 描述
	 Note :   
	 *********************************************
	 */
	BOOL HandleTcpSocketReadEvent(CGSPBuffer *pBuffer);

	//处理流TCP 接收数据
	BOOL HandleStreamTcpSocketReadEvent(CGSPBuffer *pBuffer);


	//处理流RTP-UDP 接收数据
	void HandleRtpStreamFrame(CFrameCache *pFrame);

	/*
	 *********************************************
	 Function : SendCommand
	 DateTime : 2012/4/24 8:51
	 Description :  发送命令
	 Input :  eCommandID 命令ID 参考 EnumGSPCommandID 说明
	 Input :  pCommandPlayload 命令包的内容， 指 StruGSPCommand::cPlayload
	 Input :  iSize pCommandPlayload 的长度
	 Input :  iTag 命令包的序号， 指 StruGSPCommand::iTag，
				如果为 INVALID_COMMAND_TAG 函数内部将自动生成一个序号
	 Output : 
	 Return : 参考 EnumErrno 说明
	 Note :   
	 *********************************************
	 */
	EnumErrno SendCommand(EnumGSPCommandID eCommandID,
						const void *pCommandPlayload,
						UINT iSize, 
					    UINT32 iTag = INVALID_COMMAND_TAG);

	
	/*
	 *********************************************
	 Function : HandleCommandResquestResponse
	 DateTime : 2012/4/24 8:55
	 Description :  发送Request 的回复命令
	 Input :  pCmd 回复的命令
	 Input : bAsync 是否 异步发送
	 Input : iTimeouts 超时时间， 单位毫秒
	 Output : 
	 Return : 参考 EnumErrno 说明
	 Note :   
	 *********************************************
	 */
	EnumErrno HandleCommandResquestResponse(const StruGSPCommand *pCmd, BOOL bAsync, INT iTimeouts );



	/*
	 *********************************************
	 Function : OnTcpRcvTaskPoolEvent
	 DateTime : 2012/4/24 8:57
	 Description : 命令处理线程池 回调, m_csCommandTask 的回调
	 Input :  
	 Output : 
	 Return : 
	 Note :   参考<<GSThreadPool.h>> FuncPtrObjThreadPoolEvent 说明
	 *********************************************
	 */
	void OnCommandTaskPoolEvent( CObjThreadPool *pTkPool, void *pData );


	/*
	 *********************************************
	 Function : HandleCommand
	 DateTime : 2012/4/24 9:01
	 Description :   命令处理
	 Input :  pCommand 被处理的命令体
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
	void HandleCommand( CGspCommand *pCommand );

	/*
	 *********************************************
	 Function : HandleKeepalive
	 DateTime : 2012/4/24 9:02
	 Description :  处理Keepalive 命令
	 Input :  pCmdHeader 命令包， 参考 StruGSPCommand 说明
	 Input : iPlayloadSize 指 命令体的长度, StruGSPCommand::cPlayload长度
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
	void HandleKeepalive( CGspCommand *pCommand );

	//
	/*
	 *********************************************
	 Function : HandleRequestResponse
	 DateTime : 2012/4/24 9:04
	 Description :  处理Request 命令
	 Input :   pCmdHeader 命令包， 参考 StruGSPCommand 说明
	 Input :  iPlayloadSize 指 命令体的长度, StruGSPCommand::cPlayload长度
	 Input :  iGSPVersion 接受命令的GSP 版本号
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
	void HandleRequestResponse( CGspCommand *pCommand );




	/*
	 *********************************************
	 Function : WakeupAsync
	 DateTime : 2012/4/24 9:06
	 Description :  唤醒异步同步对象
	 Input :   iCommandID 等待的命令
	 Input : iCmdTag 等待命令序号
	 Input : eErrno 唤醒结果
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
	void WakeupAsync(INT32 iCommandID, INT32 iCmdTag, EnumErrno eErrno );


	/*
	 *********************************************
	 Function : OnTimerEvent
	 DateTime : 2012/4/24 9:07
	 Description :  定时器回调
	 Input :  
	 Output : 
	 Return : 
	 Note :   参考 FuncPtrTimerCallback 说明
	 *********************************************
	 */
	void OnTimerEvent( CWatchTimer *pTimer );


	/*
	 *********************************************
	 Function : SendKeepalive
	 DateTime : 2012/4/24 9:08
	 Description :  发送Keepalive 命令
	 Input :  bResponse 指明该命令对端是否需要回复
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
	void SendKeepalive( BOOL bResponse );





	//唤醒所有异步等待者
	void WakeupAllAsyncWaiter(EnumErrno eErrno);


	INT GetMediaChannel(EnumGSMediaType eType);

    
};

} //end namespace GSP


#endif //end _GS_H_PROGSPCHANNEL_H_
