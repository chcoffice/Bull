#include "GSCommon.h"
#include "NetMessage.h"

CNetMessage::CNetMessage()
{
	m_iMsgLen = 0;
	m_pMsg = NULL;
	m_uiSessionID = 0;
	m_uiCommandID = 0;
	memset(&m_SrcID, 0, sizeof(m_SrcID));
	memset(&m_DstID, 0, sizeof(m_DstID));
}

CNetMessage::~CNetMessage()
{
}

INT32 CNetMessage::GetMSGLen()
{
	return m_iMsgLen;
}
void* CNetMessage::GetMSG()
{
	return m_pMsg;
}
UINT16 CNetMessage::GetSessionID()
{
	return m_uiSessionID;
}
UINT32 CNetMessage::GetCommandID()
{
	return m_uiCommandID;
}

StruID* CNetMessage::GetSrcModule()
{
	return &m_SrcID;
}

StruID* CNetMessage::GetDstModule()
{
	return &m_DstID;
}


void CNetMessage::SetMSGLen(INT32 iLen)
{
	m_iMsgLen = iLen;
}

void CNetMessage::SetMSG(void* pMsg)
{
	m_pMsg = pMsg;
}

void CNetMessage::SetSessionID(UINT16 uiSID)
{
	m_uiSessionID = uiSID;
}

void CNetMessage::SetCommandID(UINT32 uiCID)
{
	m_uiCommandID = uiCID;
}

void CNetMessage::SetSrcID(StruID* pID)
{
	if(NULL == pID)
	{
		return;
	}
	m_SrcID.uiPlatformID = pID->uiPlatformID;
	m_SrcID.uiSysID = pID->uiSysID;
}

void CNetMessage::SetDstID(StruID* pID)
{
	if(NULL == pID)
	{
		return;
	}
	m_DstID.uiPlatformID = pID->uiPlatformID;
	m_DstID.uiSysID = pID->uiSysID;
}