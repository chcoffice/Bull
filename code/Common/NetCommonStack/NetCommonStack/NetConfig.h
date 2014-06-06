#ifndef NET_CONFIG_DEF_H
#define NET_CONFIG_DEF_H

#include "NetCommonStack.h"


class CNetConfig : public INetConfig
{
public:
	CNetConfig();
	virtual ~CNetConfig();

public:
	void SetProtocolType(INT32 iProtocolType);
	INT32 GetProtocolType();

	void SetMaxChnNum(INT32 iMaxChnNum);
	INT32 GetMaxChnNum();

	void SetSupportMsgBuf(INT32 iSupport);
	INT32 GetSupportMsgBuf();
	void SetMaxMsgBufSize(INT32 iBufSize);
	INT32 GetMaxMsgBufSize();

	void SetMaxMsgSize(INT32 iMsgSize);
	INT32 GetMaxMsgSize();

	void SetNetPulseTime(INT32 iPulseTime);
	INT32 GetNetPulseTime();

	void SetSupportReConnect(INT32 iSupport);
	INT32 GetSupportReConnect();
	void SetReConnectTime(INT32 iReConnTime);
	INT32 GetReConnectTime();

	void SetNetEventCB(void* pCaller, fnNetEvent EventEntry);
	void* GetNetEventCaller();
	fnNetEvent GetNetEventEntry();

	INetConfig* GetNetConfig();
	void SetNetConfig(INetConfig* pCfg);

	virtual void SetNetDataTimeout(INT32 iTimeOut);
	virtual INT32 GetNetDataTimeout();

	virtual void SetReuseNetPortFlag(INT32 iReuse);
	virtual INT32 GetReuseNetPortFlag();

protected:
	INT32 m_iProtocolType;
	INT32 m_iMaxChnNum;

	INT32 m_iSupportMsgBuf;
	INT32 m_iMaxMsgBufSize;

	INT32 m_iMaxMsgSize;

	INT32 m_iNetPulseTime;

	INT32 m_iSupportReConnect;
	INT32 m_iReConnectTime;

	fnNetEvent m_fnNetEvent;
	void* m_pEventCaller;

	INT32 m_iNetDataTimeout;

	INT32 m_iReuseNetPortFlag;
};


#endif