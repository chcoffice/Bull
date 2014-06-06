/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : PROSIPCHANNEL.H
Author :  邹阳星
Version : 1.0.0.1
Date: 2012/11/13 16:17
Description: SIP 协议通道 
********************************************
*/

#ifndef _GS_H_PROSIPCHANNEL_H_
#define _GS_H_PROSIPCHANNEL_H_



#include "../IProChannel.h"
#include "ISocket.h"
#include "MainLoop.h"
#include "RTP/RtpStru.h"
#include "SipStack.h"
#include "RTP/RtpNet.h"
#include "RTP/SdpParser.h"
#include "SIP/SIPParser.h"
#include "SIP/ES2PS.h"
#include "GSPMemory.h"
#include "StreamPackConvert.h"

using namespace GSP::RTP;
namespace GSP
{
	class  CClientChannel;
	class  CClientSection;
	

	namespace SIP
	{
//#define TEST_SERVER

	class  CProSipClientService;
	class  CStreamEncoder;
#define CLICHN_KEEPALIVE_TIMER_ID 1
#define CLICHN_ACTIVE_TIMER_ID  2
#define CLICHN_CLOSE_TIMER_ID  3

		typedef UINT32 SipClientConnecter;  //sip 通信		
		#define INVALID_SIP_CLIENT_CNNER 0
		

class CSipChannel :
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
	static GSAtomicInter s_iYSSRCSeq;
	CClientChannel *m_pParent;
	CProSipClientService *m_pSipSrv;
	const UINT32 m_iAutoID;
	UINT32 m_iYSSRC;
	
	
	GSAtomicInter  m_iCTagSequence; //命令CSeq
	

	
	INT m_iMaxKeepaliveTimeouts;

	CWatchTimer m_csSendKeepaliveTimer; //发送keepalive 

	CWatchTimer m_csKeepaliveTestTimer; //心跳检测

	INT m_iKeepalivePlugs; //心跳检测计数

	CGSWRMutex m_csWRMutex;



	C28181ParserSubject m_csSubject;
	
	CRtpUdpReader *m_pRtpReader;
	CSdpParser m_csSdp; //sdp 描述

	BOOL m_bStreamStart; //是否开始流接收

	BOOL m_bSendAck;


	CGSString m_strUsername;
	CGSString m_strDevId;

	UINT16 m_iKpSN;

	EnumRTPPayloadType m_eRtpPT; 

	EnumGSCodeID m_eInputCodeId;

 	
	
	INT64  m_iBeginTimestamp; //当前开始计算的时间戳， 单位 秒,utc时间
	UINT32 m_iTimestamp; //相对 m_iBeginTimestamp 的偏移,单位毫秒

	UINT64 m_iLastPTS; //PS 流时，最后一帧的时间戳

	INT64 m_iFileLengthTv; //文件时长
	INT64 m_iFileStartTv; //文件开始时间, 回放或下载
	INT64 m_iLastPos; //最后一次的进度

	BOOL m_bWouldSendByte; //需要发送Byte

	StruSipDialogKey m_stInviteDlgKey;

	int m_iPSSampleRate; 

	double m_fSpeed;

	CStreamPackConvert *m_pPkgCvt;

	EnumStreamPackingType m_eInputPkgType;
	EnumStreamPackingType m_eOutputPkgType;  //默认等于 eSTREAM_PKG_GSPS

	CStreamEncoder *m_pDecoder; //媒体封装重定义

	BOOL m_bTestRtcp; //检测RTCP 包
public :
	SipClientConnecter m_hCliCnner;
	CGSString m_strInviteCallId;
	BOOL m_bTransProxy; //传输代理模式
#ifdef TEST_SERVER
	bool m_bTEST_SERVER;
#endif

public  :
    //以下是接口实现
	INLINE CClientChannel* GetParent(void)
	{
		return m_pParent;
	}
	virtual EnumErrno Open(const CUri &csUri, BOOL bAsyn, INT iTimeouts);
	virtual EnumErrno Ctrl(const StruGSPCmdCtrl &stCtrl, BOOL bAsync,INT iTimeouts);
	virtual EnumErrno CtrlOfManstrsp(const char *czMansrtsp, BOOL bAsync,INT iTimeouts, StruGSPCmdCtrl &stGspCtrl);
	virtual void DestoryBefore(void);

	virtual EnumErrno FlowCtrl( BOOL bStart );


	virtual CGSString GetSdp(void)
	{
		if( !m_bSendAck )
		{
			return CGSString();
		}
		return m_csSdp.Serial();
	}

	/*
	*********************************************************************
	*
	*@brief : 新增加接口
	*
	*********************************************************************
	*/
	CSipChannel(CClientChannel *pParent);

	virtual ~CSipChannel(void);


	//网络断开
	void OnDisconnectEvent(void); 

	// SIP 服务器 数据包
	void HandleSipData(StruSipData *pData); 




private :


	



	//定时器回调
	void OnTimerEvent( CWatchTimer *pTimer );


	//RTP 接受数据回调
	void OnRtpReaderEvent( EnumRtpNetEvent eEvt, void *pEvtArgs );

	EnumErrno SendAck(void); //发送 Ack 命令
	EnumErrno SendMANSRTSP( const StruGSPCmdCtrl &stCtrl ); //发送 MANSRTSP
	EnumErrno SendBye(void); //发送 Bye 命令


	//出错是处理Sip 数据包
	void ResponseBadRequest(StruSipData *pData,int iSipErrno, const char *czError  );

	void ResponseSimple( StruSipData *pData,int iSipErrno, const char *czInfo  );

	void SendKeepalive( BOOL bResponse );

	//处理流
	void HandleRtpStreamFrame(CFrameCache *pFrame);


	UINT64 IntervalOfPTS(UINT64 iStart, UINT64 iNow);


	EnumErrno ConvertFrameInfo(CFrameCache *pFrame,  StruFrameInfo &stInfo );

};


//Sip 客户端 服务
class CProSipClientService : public CGSPObject
{
public :
	CClientSection *m_pParent;
	SipServiceHandle m_hSipSrv;
private :
	CGSString m_strSipUdpBindIp;
	INT m_iSipUdpPort;
	CGSString m_strSipServerName;
	INT m_iRtpUdpPortBegin;
	INT m_iRtpUdpPortEnd;

	BOOL m_bForceSendRTCP;  //使用流保活， 默认不使用

	BOOL m_bReady;

	typedef struct _StruMyChnInfo
	{
		CSipChannel *pChn;		
		StruSipDialogKey stInviteDlgKey;
		_StruMyChnInfo(void)
		{
			pChn = NULL;			
			bzero(&stInviteDlgKey, sizeof(stInviteDlgKey));
		}
	}StruMyChnInfo;


	typedef std::map<CGSString  /*call-id*/ , StruMyChnInfo > CMapMyChannel;
	typedef struct _StruMyCnnerInfo
	{
		CGSString strRemoteHost;
		int iRemotePort;
		CGSString strUserName;
		CGSString strPassword;

		time_t tvLoginTv;
		UINT64 tvLastKeepalive;
		SipSessionHandle hHandle;
		SipClientConnecter hCnner;
		CMapMyChannel mapMyChannels;
		BOOL m_bRegistered;
		BOOL m_iRegistering;
		GSAtomicInter m_iBusy;
#ifdef TEST_SERVER
		bool m_bTEST_SERVER;
#endif
		_StruMyCnnerInfo() : hHandle(SIP_INVALID_HANDLE), hCnner(INVALID_SIP_CLIENT_CNNER)
		{
			mapMyChannels.clear();
			m_bRegistered = FALSE;
			tvLoginTv = 0;
			iRemotePort = -1;
			tvLastKeepalive = 0;
			m_iRegistering = FALSE;
			m_iBusy = 0;
#ifdef TEST_SERVER
			m_bTEST_SERVER = false;
#endif
		}
	}StruMyCnnerInfo;


	CGSWRMutex m_mutexSipClients;
	SipClientConnecter m_seqConnecter;
	typedef std::map<SipClientConnecter , StruMyCnnerInfo> CMapSipClient;
	CMapSipClient m_mapSipClients;
	CMapMyChannel m_mapSipClientsProxy; //代理模式


	CWatchTimer m_csKeepaliveTestTimer; //心跳检测

public :


	CProSipClientService(CClientSection *pParent );
	virtual ~CProSipClientService(void);

	EnumErrno Init( CConfigFile &cfCfg);

	EnumErrno Start(void);

	EnumErrno Stop(void);

	EnumErrno Uninit(void);

	BOOL IsForceRTCPKeepalive(void) const
	{
		return m_bForceSendRTCP;
	}


	void SipDisconnectChannel(CSipChannel *pChannel);
	EnumErrno SipCreateConnecter( CSipChannel *pChannel,const CGSString &strDevId,
								  const CGSString &strRemoteHost,int iRemotePort,
								  const CGSString &strUserName, 
								  const  CGSString &strPassword,
								  const StruSipData *pInviteReq, 
								  StruSipData *pResInviteRes);

	EnumErrno SessionSendSipData( CSipChannel *pChannel, 
									const StruSipData *pSendData, StruSipData *pRes,
									int iTimeouts,  StruSipDialogKey *pOutDlgKey );

	EnumErrno OnTransRecvData(const StruSipData *pSendData);

	void SipConnecterNetError(CSipChannel *pChannel, const CGSString &strDevId,
		const CGSString &strRemoteHost,int iRemotePort,
		const CGSString &strUserName, 
		const  CGSString &strPassword, INT eSipErrno);

private :
	//收包
	static void   SipSIPPacketCallback(SipServiceHandle hService, 
		SipSessionHandle hSession,
		StruSipData *pData);
	static EnumSipErrorCode SipClientConnectCallback( SipServiceHandle hService,
		SipSessionHandle hNewSession );
	//断开
	static void SipClientDisconnectCallback(SipServiceHandle hService, SipSessionHandle hSession);


	EnumSipErrorCode OnSipClientConnectEvent(SipSessionHandle hNewSession );
	void OnSipClientDisconnectEvent(SipSessionHandle hSession);
	void OnSipPacketEvent(SipSessionHandle hSession, StruSipData *pData);

	StruMyCnnerInfo* RefCnnerInfo(SipClientConnecter hCnner );
	INLINE void UnrefCnnerInfo( StruMyCnnerInfo*pInfo)
	{
		if( pInfo)
		{
			AtomicInterDec(pInfo->m_iBusy);
		}
	}

	//定时器回调
	void OnTimerEvent( CWatchTimer *pTimer );

	EnumErrno SessionSendProxySipData( CSipChannel *pChannel, 
		const StruSipData *pSendData, StruSipData *pRes,
		int iTimeouts,  StruSipDialogKey *pOutDlgKey );

	

};


} //end namespace SIP

} //end namespace GSP



#endif //end _GS_H_PROSIPCHANNEL_H_
