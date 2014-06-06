#ifndef NET_CHANNEL_DEF_H
#define NET_CHANNEL_DEF_H

#include "NetCommonStack.h"
#include "INetService.h"
#include "NetConfig.h"
#include "BufferArray.h"

#include "BaseChannel.h"
#include "UserProtocol.h"
#include "BufMgr.h"


#define NET_CHANNEL_TYPE_CLIENT 0
#define NET_CHANNEL_TYPE_SERVER 1

class CNetChannel : public INetChannel, 
					public CNetConfig, 
					public CBaseChannel,
					public CUserProtocol
{
public:
	CNetChannel();
	virtual ~CNetChannel();

public:
	INetConfig* GetChannelCfg();
	void SetChannelCfg(INetConfig* pCfg);

	INT32 InitNetChannel(CBufferArray* pBufferArray = NULL);
	INT32 ReleaseNetChannel();

	virtual char* GetRemoteIPAddrString();
	virtual UINT32 GetRemoteIPAddr();
	virtual char* GetLocalIPAddrString();
	virtual UINT32 GetLocalIPAddr();
	virtual UINT16 GetRemotePort();
	virtual UINT16 GetLocalPort();
	virtual INT32 ConnectServer(char* pszRemoteIPAddr, UINT16 uiRemotePort,
		char* pszBindingIPAddr, UINT16 uiBindingPort);
	virtual INT32 CloseChannel();
	virtual INT32 CloseChannelEx();//关闭通道时，调用网络库的CloseChannel。加了回调锁.
	virtual INT32 GetChnStatus();
	virtual INT32 SendRemoteMsg(UINT32 uiCMDID,
		void* pMSG, 
		INT32 iMSGLen, 
		StruID* pSrcID,
		StruID* pDstID,
		UINT16 uiSessionID = 0);
	void SetLogicRemoteIPAddrString(const char* pszIPAddr);
	char* GetLogicRemoteIPAddrString();

public:
	ISocketChannel* GetSocketChn();
	void SetSocketChn(ISocketChannel* pChn, INT32 iChnStatus);
	INetService* GetNetService();
	void SetNetService(INetService* pNS);

	void SetChnStatus(INT32 iChnStatus);
	INT32 GetChnType();
	void SetChnType(INT32 iChnType);
	INT32 OnDataArrive(void* pData, INT32 iDataLen);
	INT32 OnCheckStatus();

protected:
	void OnRecvPulse();
	void SendPulse();
	INT32 RecvPacketSlice(StruCommuPacket* pCommuPacket);
	INT32 SendRemoteData(char* pData, INT32 iDataLen);
	
protected:
	char* m_pRecvBuf;
	INT32 m_iCurRecvPos;
	ISocketChannel* m_pSocketChn;
	INetService* m_pNetService;

	char* m_pRecvPackBuf;
	INT32 m_iRecvPackBufSize;
	INT32 m_iRecvPackLen;
	char* m_pExtRecvPackBuf;
	INT32 m_iExtRecvPackBufSize;

	char* m_pSendPackBuf;
	INT32 m_iSendPackBufSize;
	INT32 m_iSendPackLen;
	char* m_pExtSendPackBuf;
	INT32 m_iExtSendPackBufSize;

	CBufMgr m_MgrSendBuffer;
	CBufMgr m_MgrRecvBuffer;

	INT32 m_iNetChnStatus;
	INT32 m_iNetChnType;
	CGSMutex m_Mutex;

	INT32 m_iSendPulseCount;
	INT32 m_iReConnectCount;
	INT32 m_iNetRecvDataCount;

	CBufferArray* m_pBufferArray;
	StruCommuHeader m_PrePacketHeader;

	INT32 m_iSessionID;
	CGSMutex m_MutexSend;

	INT m_bSendPulse;

	INT m_bSendDisconnectEvent; 
};



#endif