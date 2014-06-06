#ifndef NET_MESSAGE_DEF_H
#define NET_MESSAGE_DEF_H

#include "NetCommonStack.h"


class CNetMessage : public INetMessage
{
public:
	CNetMessage();
	virtual ~CNetMessage();

public:
	virtual INT32 GetMSGLen();
	virtual void* GetMSG();
	virtual UINT16 GetSessionID();
	virtual UINT32 GetCommandID();
	virtual StruID* GetSrcModule();
	virtual StruID* GetDstModule();

public:
	void SetMSGLen(INT32 iLen);
	void SetMSG(void* pMsg);
	void SetSessionID(UINT16 uiSID);
	void SetCommandID(UINT32 uiCID);
	void SetSrcID(StruID* pID);
	void SetDstID(StruID* pID);
protected:
	INT32 m_iMsgLen;
	void* m_pMsg;
	UINT16 m_uiSessionID;
	UINT32 m_uiCommandID;
	StruID m_SrcID;
	StruID m_DstID;
};


#endif