/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : RTPSENDER.H
Author :  邹阳星
Version : 1.0.0.1
Date: 2012/10/30 14:29
Description: RTP 的网络操作
********************************************
*/

#ifndef _GS_H_RTPNET_H_
#define _GS_H_RTPNET_H_


#include "../GSPObject.h"
#include "RTPStru.h"
#include "../ISocket.h"
#include "../GSPMemory.h"
#include "../MainLoop.h"
#include "RTPAnalyer.h"
#include <list>


namespace GSP
{
	
	

namespace RTP
{
	typedef struct _StruRtpSn
	{
		UINT32 iSSRC;
		UINT32 iTimeStamp;
		UINT16  iSeq;
		UINT16  iSeqNumber;
		UINT  iPT;				
	}StruRtpSn;

	typedef enum
	{
		eEVT_RTPNET_STREAM_FRAME = 1,  // pEvtArgs 等于 CFrameCache *
		eEVT_RTPNET_RTCP_PULSE,       //  pEvtArgs 无用 NULL
		eEVT_RTPNET_LOST_FRAME,      //  pEvtArgs 等于 StruRtpSn *
		eEVT_RTPNET_RTCP_BYE,		//收到RTCP 的Bye 命令 pEvtArgs 无用 NULL
	}EnumRtpNetEvent;

	typedef void (CGSPObject::*FuncPtrRtpNetEvent)(EnumRtpNetEvent eEvt, void *pEvtArgs);

#define  iCONST_RTCP_BUFFER  1400


class CRtpUdpNet : public CGSPObject
{
protected :
	CUDPSocket *m_pRtpSocket;
	CUDPSocket *m_pRtcpSocket;

	union
	{
		StruRtcpPacket stPacket;
		char pRtcpBuffer[iCONST_RTCP_BUFFER];	
	}m_unionRtcpBuf;

	/* rtcp sender statistics receive */
	INT64 m_last_rtcp_ntp_time;    
	INT64 m_first_rtcp_ntp_time;  

	/* rtcp sender statistics */
	UINT m_packet_count;     

	UINT m_octet_count;      

	UINT m_last_octet_count; 

	INT m_first_packet;

	UINT32 m_base_timestamp;

	INT m_num_frames;

	CRtpHeader m_csRtpHeader;

	
	CGSMutex m_csMutex;
	UINT32 m_iRcvRtcpCounts;

	UINT m_iChannelId;
	UINT8 m_iRtpPlayloadType;
	UINT32 m_iCurTimestamp;
	UINT32 m_iSSRC;
	
	BOOL m_bMulticast;
	CGSString m_strLocalIp;
	INT m_iLocalRtpPort;
	INT m_iLocalRtcpPort;
	BOOL m_bSendOnly;
	BOOL m_bConnected;

	

	FuncPtrRtpNetEvent m_fnEvtCb;
	CGSPObject *m_pEvtReceiver;
	const BOOL m_bService;
public :
	CRtpUdpNet(BOOL bService );
	virtual ~CRtpUdpNet(void);
	

	INLINE CUDPSocket *GetRtpSocket(void)
	{
		return m_pRtpSocket;
	}

	INLINE CUDPSocket *GetRtcpSocket(void)
	{
		return m_pRtcpSocket;
	}
	
	INLINE UINT32 GetSSRC(void) const
	{
		return m_iSSRC;
	}

	INLINE EnumErrno SetSSRC(UINT32 iSSRC)
	{
		m_iSSRC = iSSRC;
		m_csRtpHeader.SetSyncSource(m_iSSRC);
		return eERRNO_SUCCESS;
	}

	INLINE BOOL IsMulticast(void) const
	{
		return m_bMulticast;
	}

	INT GetPlayloadType( void ) const;

	INT GetChannelID(void) const;

	EnumErrno SetPlayloadType( UINT8 iPlayloadType, UINT16 iChannelID);


	INLINE UINT32 GetRcvRtcpCount(void) const
	{
		return m_iRcvRtcpCounts;
	}

	INLINE UINT32 GetCurTimestamp(void) const
	{
		return m_iCurTimestamp;
	}

	EnumErrno SetRemoteAddress(  const CGSPString &strRemoteHost, int iRtpPort, int iRtcpPort);

	void ClearRemoteAddress( void );
	
	EnumErrno JoinMulticast(const CGSPString &strRemoteHost );

	EnumErrno LeaveMulticast(const CGSPString &strRemoteHost );

	EnumErrno    SetTtl(UINT16 timeToLive);

	EnumErrno    SetMulticastInterface(const CGSPString &strLocalIp );


	virtual EnumErrno Start(void) = 0;

	virtual void Stop(void) = 0;

	

	void SetEventListener(CGSPObject *pEvtReceiver, FuncPtrRtpNetEvent fnEvtCb );

protected :
	void Disconnect(void);
	EnumErrno Init( const char *czLocalIp, BOOL bSendOnly, BOOL bMulticast );
	EnumErrno Init(int iMinPort, int iMaxPort, const char *czLocalIp, BOOL bSendOnly, BOOL bMulticast );

	BOOL RunEventCallback(EnumRtpNetEvent eEvt, void *pEvtArgs)
	{
		if( m_pEvtReceiver && m_fnEvtCb )
		{
		   ((m_pEvtReceiver->*m_fnEvtCb)(eEvt, pEvtArgs));
		}
		return TRUE;
	}
};

class CRtpUdpSender : public CRtpUdpNet
{
private :
	UINT32 m_iTimestampInterval;
	CWatchTimer m_csSendRtcpTimer;
	
public :
	virtual ~CRtpUdpSender(void);

	static CRtpUdpSender *Create( const char *czLocalIp, BOOL bSendOnly, BOOL bMulticast );
	static CRtpUdpSender *Create( int iMinPort, int iMaxPort, const char *czLocalIp, BOOL bSendOnly, BOOL bMulticast );


	virtual EnumErrno Start(void);

	virtual void Stop(void);


	INLINE UINT32 GetTimestampInterval(void) const
	{
		return m_iTimestampInterval;
	}

	EnumErrno SetTimestampInterval( UINT32 iInvertal )
	{
		m_iTimestampInterval = iInvertal;
		return eERRNO_SUCCESS;
	}

	EnumErrno Send(CFrameCache *pFrame, UINT32 iTimestamp);

	EnumErrno Send(CFrameCache *pFrame);

	
	
protected :
	CRtpUdpSender(void);
private :
	void RtcpTimerTicks(void);
	void *OnRtcpSocketEvent(CISocket *pSocket, 
		EnumSocketEvent iEvt,void *pParam, void *pParamExt );
	INT MakeRtcpSendSR(void);

	void OnTimerEvent(CWatchTimer *pTimer);

	void RtcpSendBye(void);
};



class CRtpUdpReader : public CRtpUdpNet
{
private :
	StruLenAndSocketAddr m_stLimitRemoteAddr; //限定对端的IP, 否则全部接受
	UINT m_iLimitePort;

	UINT32 m_iTimestampInterval;
	CWatchTimer m_csSendRtcpTimer;

	CRtpDecoder m_rtpDecoder;

	EnumGSMediaType m_vBindMediaType[GSP_MAX_MEDIA_CHANNELS];
public:
	virtual ~CRtpUdpReader(void);


	void BindChannelMediaType(UINT iChnId,EnumGSMediaType eType, UINT8 iRtpPt );

	void SetLimitAddr( const CGSPString &strRemoteIp, UINT iPort );


	static CRtpUdpReader *Create( const char *czLocalIp, BOOL bRcvOnly, BOOL bMulticast  );

	static CRtpUdpReader *Create(int iMinPort, int iMaxPort, const char *czLocalIp, 
						BOOL bRcvOnly, BOOL bMulticast  );

	virtual EnumErrno Start(void);

	virtual void Stop(void);

protected :
	CRtpUdpReader(void);

	void OnTimerEvent(CWatchTimer *pTimer);

	void RtcpSendBye(void);

	void *OnRtcpSocketEvent(CISocket *pSocket, 
		EnumSocketEvent iEvt,void *pParam, void *pParamExt );
	void *OnRtpSocketEvent(CISocket *pSocket, 
		EnumSocketEvent iEvt,void *pParam, void *pParamExt );

	void RtcpTimerTicks(void);
	INT MakeRtcpRcvSR(void);
};


} //end namespace RTP

} //end namespace GSP

#endif //end _GS_H_RTPSENDER_H_
