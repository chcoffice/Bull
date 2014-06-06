/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : RTPCONTENT.H
Author :  zouyx
Version : 0.0.0.0
Date: 2012/4/23 14:41
Description: 
********************************************
*/

#ifndef _GS_H_RTPCONTENT_H_
#define _GS_H_RTPCONTENT_H_
#include "ISocket.h"
#include "../MainLoop.h"
#include "../RTPAnalyer.h"


namespace GSP
{
	
#define RTP_CMD_CHANNEL_ID  (0x7F-1)  //CMD 通信使用的信道， X

class CRtpContent : public CGSPObject
{
public :
	typedef enum
	{
		eEVT_RCV_FRAME = 1, //接收到数据， 参数 CSliceFrame *
		eEVT_RCV_LOST_FRAME, //丢帧， 参数为NULL
		eEVT_RCV_NAL_PACKET, //收到NAL 包
		eEVT_RCV_RETRY_RESEND, //请求重传， 参数 为 INT ， 表示重传的个数
	}EnumEvent;

	/*
	 *********************************************
	 Function : FuncPtrEventCallback
	 DateTime : 2012/4/25 14:52
	 Description :  事件回调
	 Input :  eEvt 事件类型参考 EnumEvent
	 Input : pEvtArg 事件参数 参考 EnumEvent 说明
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
	typedef void (CGSPObject::*FuncPtrEventCallback)(EnumEvent eEvt, void *pEvtArg);

	typedef struct _StruRtpCoreData
	{	
		RTP::StruRTPHeader stRtpHeader; //发送数据的Rtp头
		UINT16 iRtpSeq; //发送数据的Rtp头的SEQ
		INT16 iCompletes; //收到结束包的个数
		UINT32  iTimestamp; //发送数据的Rtp头的TP
		CList lRcvPacket; //接受到得数据 存储对象为 RTSP::CRtpRecvBuffer *
		
		UINT32 iRcvDiscardTimestamp; //最后失效帧的时间


		std::vector<RTP::CRtpSliceFrame *> vFrameCache;
		INT iCacheW; // Cache 写指针
		INT iMaxCaches;
		void Init( FuncPtrFree fnFreeRcvPacket )
		{
			
			INIT_DEFAULT_RTH_HEADER(&stRtpHeader);
			iRtpSeq = 0;
			iTimestamp = 0;
			iCompletes = 0;
			lRcvPacket.Clear();
			iRcvDiscardTimestamp = 0;
			iCacheW = 0;
			iMaxCaches = 0;
			
			if( fnFreeRcvPacket )
			{
				lRcvPacket.SetFreeCallback(fnFreeRcvPacket);
			}
			for( INT i = 0; i<5; i++ )
			{
				RTP::CRtpSliceFrame *p = new RTP::CRtpSliceFrame();
				if( p )
				{
					vFrameCache.push_back(p);
				}
				
			}
			iMaxCaches = vFrameCache.size();
			GS_ASSERT_EXIT(iMaxCaches>0, -1);
		}
		void Clear(void)
		{
			lRcvPacket.Clear();
			for( UINT i = 0; i<vFrameCache.size(); i++ )
			{
				delete vFrameCache[i];
			}
			vFrameCache.clear();
		}
		
	}StruRtpCoreData;




	
	StruLenAndSocketAddr m_stRemoteAddr; //对端的网地址信息
	StruRtpCoreData m_vRtpData[GSP_MAX_MEDIA_CHANNELS]; //RTP 头使用的数据
	StruRtpCoreData m_stCmdRtpData; //Command 通道 RTP 头使用的数据
	UINT16 m_iLocalSSRC;  //SSRC 本地部分
	UINT16 m_iRemoteSSRC; //SSRC 远端 部分
	UINT32 m_iSSRC; // SSRC
	StruLenAndSocketAddr m_stNalRemoteAddr; //进行NAL 发送的包
	BOOL m_bNalEnable; //释放使能 NAL通信
	BOOL m_bConnect; //当前是否连接
	BOOL m_bNalInit; //是已经初始化NAL

	CWatchTimer m_csNalTimer; // 发送NAL 的定时器
	INT m_iNalInterval; //NAL 发送时间间隔， 单位秒

	CGSString  m_strRemoteIP; // 对端IP
	INT m_iRemotePort; //对端通信端口

	CMyID *m_pMyID;

	BOOL m_bTestConnect; //正在检查测试

	CGSMutex m_csMutex;
	CGSCondEx m_csCond; //用以连接测试等待

	BOOL m_bServerType; //是否为服务器端

	CGSPObject    *m_pFuncOwner; //事件回调 所属对象
	FuncPtrEventCallback m_fnCallback; //回调函数

	INT m_iSendNalTicks;  //发送NAL 计数

	BOOL m_bInit;
private :
	CRtpSocket *m_pSocket;
public :

	CRtpContent(void);

	virtual ~CRtpContent(void);

	INLINE INT LocalPort(void)
	{
		CGSAutoMutex locker(&m_csMutex);
		if( m_pSocket )
		{
			return m_pSocket->Socket().LocalPort();
		}
		return 0;
	}


	void SetEventCallback( CGSPObject *pFuncOwner, FuncPtrEventCallback fnCallback );



	/*
	 *********************************************
	 Function : Init
	 DateTime : 2012/4/24 19:05
	 Description :  初始化
	 Input :  pSocket 通信使用的RTP SOCKET
	 Output : 
	 Return :  返回错误码
	 Note :   
	 *********************************************
	 */
	EnumErrno Init( CRtpSocket *pSocket );

	/*
	 *********************************************
	 Function : Unint
	 DateTime : 2012/4/24 19:21
	 Description :  卸载
	 Input :  
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
	void Unint(void);

	/*
	 *********************************************
	 Function : Connect
	 DateTime : 2012/4/24 19:06
	 Description :  连接对端
	 Input :  szRemoteIP 对端IP
	 Input :  iRemotePort 对端端口
	 Input : iRemoteSSRC 对端SSRC
	 Input : szNalIP NAL IP， NULL 不起用 NAL
	 Input : iNalPort NAL 端口， < 1  不起用 NAL
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
	EnumErrno Connect(const char *szRemoteIP, UINT16 iRemotePort,UINT16 iRemoteSSRC, 
						const char *szNalIP, INT iNalPort );

	/*
	 *********************************************
	 Function : EnableNal
	 DateTime : 2012/4/24 19:09
	 Description :  使能NAL 发送
	 Input :  bEnable 使能
	 Input :  iInterval 发送间隔， 单位： 秒
	 Output : 
	 Return : TRUE/FALSE
	 Note :   
	 *********************************************
	 */
	BOOL EnableNal(BOOL bEnable, INT iInterval = 250);
	
	/*
	 *********************************************
	 Function : SendFrame
	 DateTime : 2012/4/24 19:10
	 Description :  发送媒体数据
	 Input :  pFrame 媒体数据帧
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
	EnumErrno SendFrame(CSliceFrame *pFrame );

	/*
	 *********************************************
	 Function : ConnectTest
	 DateTime : 2012/4/24 19:11
	 Description :  检测连接是否可以
	 Input :  iTimeouts 超时时间， 单位毫秒
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
	EnumErrno ConnectTest(INT iTimeouts);
private :

	/*
	 *********************************************
	 Function : OnTimerEvent
	 DateTime : 2012/4/24 19:11
	 Description :  定时器回调
	 Input :  
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
	void OnTimerEvent( CWatchTimer *pTimer );


	/*
	 *********************************************
	 Function : OnRtpRecvEvent
	 DateTime : 2012/4/24 19:30
	 Description :  RTP SOCKET 接收数据回调，参考 CRtpSocket::FuncPtrRtpDataRecvEvent
	 Input :  
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
	BOOL OnRtpRecvEvent(const StruLenAndSocketAddr &stRemoteAddr, RTP::CRtpRecvBuffer *pPacket );

	/*
	 *********************************************
	 Function : SendTestConnectCommand
	 DateTime : 2012/4/24 20:21
	 Description :  发送连接测试命令
	 Input :  
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
	BOOL SendTestConnectCommand(void); 

	/*
	 *********************************************
	 Function : HandleCommand
	 DateTime : 2012/4/24 20:35
	 Description :  处理通道命令
	 Input :  
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
	BOOL HandleCommand(const StruLenAndSocketAddr &stRemoteAddr, 
		RTP::CRtpRecvBuffer *pPacket);


	/*
	*********************************************
	Function : HandleCommand
	DateTime : 2012/4/24 20:35
	Description :  处理流数据
	Input :  
	Output : 
	Return : 
	Note :   
	*********************************************
	*/
	BOOL HandleStream(const StruLenAndSocketAddr &stRemoteAddr, 
		RTP::CRtpRecvBuffer *pPacket);


	void SendEvent( EnumEvent eEvt, void *pArgs )
	{
		if( m_fnCallback )
		{
			(m_pFuncOwner->*m_fnCallback)(eEvt, pArgs);
		}
	}

};

} //end namespace GSP

#endif //end _GS_H_RTPCONTENT_H_