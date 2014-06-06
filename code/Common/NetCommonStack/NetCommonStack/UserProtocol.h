#ifndef USER_PROTOCOL_DEF_H
#define USER_PROTOCOL_DEF_H

#include "NetCommonStack.h"
#include "BaseProtocol.h"
#include "NetMessage.h"

typedef struct StruUserProtocolHead
{
	UINT16 uiVersion;
	UINT16 uiSession;
	StruID SrcID;
	StruID DstID;
	UINT32 uiCMDID;
	UINT32 uiDataLen;
}StruUserProtocolHead;

typedef struct StruUserProtocol
{
	StruUserProtocolHead Head;
	char sData[4];
}StruUserProtocol;

#define CURRENT_PROTOCOL_VERSION 0

class CUserProtocol : public CBaseProtocol
{
public:
	CUserProtocol();
	virtual ~CUserProtocol();

public:
	INT32 PacketUserMsg(UINT32 uiCMDID,
		void* pMSG, 
		INT32 iMSGLen, 
		StruID* pSrcID,
		StruID* pDstID,
		UINT16 uiSessionID,
		char* pDstBuf);

	INT32 DePacketUserMsg(char* pDataBuf, CNetMessage* pUserMsg);
};


#endif