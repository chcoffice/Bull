#include "GSCommon.h"
#include "NetConfig.h"


CNetConfig::CNetConfig()
{
	m_iProtocolType = PROTOCOL_TYPE_TCP;
	m_iMaxChnNum = DEFAULT_NET_CHANNEL_NUM;
	m_iSupportMsgBuf = SUPPORT_MESSAGE_BUF;
	m_iMaxMsgBufSize = DEFAULT_MSG_BUF_SIZE;
	m_iMaxMsgSize = DEFAULT_MSG_SIZE;
	m_iNetPulseTime = DEFAULT_NET_PULSE_TIME;
	m_iSupportReConnect = SUPPORT_RECONNECT;
	m_iReConnectTime = DEFAULT_RECONNECT_TIME;
	m_fnNetEvent = NULL;
	m_pEventCaller = NULL;
	m_iNetDataTimeout = NET_DATA_TIMEOUT;
	m_iReuseNetPortFlag = REUSE_NETPORT_FLAG;
}

CNetConfig::~CNetConfig()
{

}

void CNetConfig::SetProtocolType(INT32 iProtocolType)
{
	if(iProtocolType < PROTOCOL_TYPE_TCP || 
		iProtocolType > PROTOCOL_TYPE_UDP)
	{
		return;
	}
	m_iProtocolType = iProtocolType;
}


INT32 CNetConfig::GetProtocolType()
{
	return m_iProtocolType;
}


void CNetConfig::SetMaxChnNum(INT32 iMaxChnNum)
{
	if(iMaxChnNum < 0)
	{
		iMaxChnNum = DEFAULT_NET_CHANNEL_NUM;
	}
	else if(iMaxChnNum > MAX_NET_CHANNEL_NUM)
	{
		iMaxChnNum = MAX_NET_CHANNEL_NUM;
	}
	m_iMaxChnNum = iMaxChnNum;
}
INT32 CNetConfig::GetMaxChnNum()
{
	return m_iMaxChnNum;
}

void CNetConfig::SetSupportMsgBuf(INT32 iSupport)
{
	if(SUPPORT_NO_MESSAGE_BUF == iSupport)
	{
		m_iSupportMsgBuf = SUPPORT_NO_MESSAGE_BUF;
	}
	else
	{
		m_iSupportMsgBuf = SUPPORT_MESSAGE_BUF;
	}
}
INT32 CNetConfig::GetSupportMsgBuf()
{
	return m_iSupportMsgBuf;
}


void CNetConfig::SetMaxMsgBufSize(INT32 iBufSize)
{
	if(iBufSize < 0)
	{
		iBufSize = DEFAULT_MSG_BUF_SIZE;
	}
	else if(iBufSize > MAX_MSG_BUF_SIZE)
	{
		iBufSize = MAX_MSG_BUF_SIZE;
	}
	m_iMaxMsgBufSize = iBufSize;
}
INT32 CNetConfig::GetMaxMsgBufSize()
{
	return m_iMaxMsgBufSize;
}

void CNetConfig::SetMaxMsgSize(INT32 iMsgSize)
{
	if(iMsgSize < 0)
	{
		iMsgSize = DEFAULT_MSG_SIZE;
	}
	else if(iMsgSize > MAX_MSG_SIZE)
	{
		iMsgSize = MAX_MSG_SIZE;
	}
	m_iMaxMsgSize = iMsgSize;
}
INT32 CNetConfig::GetMaxMsgSize()
{
	return m_iMaxMsgSize;
}

void CNetConfig::SetNetPulseTime(INT32 iPulseTime)
{
	if(iPulseTime < 0)
	{
		iPulseTime = DEFAULT_NET_PULSE_TIME;
	}
	m_iNetPulseTime = iPulseTime;
}
INT32 CNetConfig::GetNetPulseTime()
{
	return m_iNetPulseTime;
}

void CNetConfig::SetSupportReConnect(INT32 iSupport)
{
	if(SUPPORT_NO_RECONNECT == iSupport)
	{
		m_iSupportReConnect = SUPPORT_NO_RECONNECT;
	}
	else
	{
		m_iSupportReConnect = SUPPORT_RECONNECT;
	}
}
INT32 CNetConfig::GetSupportReConnect()
{
	return m_iSupportReConnect;
}
void CNetConfig::SetReConnectTime(INT32 iReConnTime)
{
	if(iReConnTime < 0)
	{
		iReConnTime = DEFAULT_RECONNECT_TIME;
	}
	m_iReConnectTime = iReConnTime;
}
INT32 CNetConfig::GetReConnectTime()
{
	return m_iReConnectTime;
}

void CNetConfig::SetNetEventCB(void* pCaller, fnNetEvent EventEntry)
{
	m_pEventCaller = pCaller;
	m_fnNetEvent = EventEntry;
}
void* CNetConfig::GetNetEventCaller()
{
	return m_pEventCaller;
}
fnNetEvent CNetConfig::GetNetEventEntry()
{
	return m_fnNetEvent;
}

void CNetConfig::SetNetDataTimeout(INT32 iTimeOut)
{
	m_iNetDataTimeout = iTimeOut;
}
INT32 CNetConfig::GetNetDataTimeout()
{
	return m_iNetDataTimeout;
}

INetConfig* CNetConfig::GetNetConfig()
{
	return this;
}

void CNetConfig::SetNetConfig(INetConfig* pCfg)
{
	if(this == pCfg)
	{
		return;
	}

	m_iProtocolType = pCfg->GetProtocolType();
	m_iMaxChnNum = pCfg->GetMaxChnNum();
	m_iSupportMsgBuf = pCfg->GetSupportMsgBuf();
	m_iMaxMsgBufSize = pCfg->GetMaxMsgBufSize();
	m_iMaxMsgSize = pCfg->GetMaxMsgSize();
	m_iNetPulseTime = pCfg->GetNetPulseTime();
	m_iSupportReConnect = pCfg->GetSupportReConnect();
	m_iReConnectTime = pCfg->GetReConnectTime();
	m_fnNetEvent = pCfg->GetNetEventEntry();
	m_pEventCaller = pCfg->GetNetEventCaller();
	m_iNetDataTimeout = pCfg->GetNetDataTimeout();
	m_iReuseNetPortFlag = pCfg->GetReuseNetPortFlag();
}
void CNetConfig::SetReuseNetPortFlag(INT32 iReuse)
{
	if (iReuse == REUSE_NETPORT_FLAG)
	{
		m_iReuseNetPortFlag = REUSE_NETPORT_FLAG;
	}
	else
	{
		m_iReuseNetPortFlag = 0;
	}
	
}
INT32 CNetConfig::GetReuseNetPortFlag()
{
	return m_iReuseNetPortFlag;
}