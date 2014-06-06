#include "GSCommon.h"
#include "UserProtocol.h"

CUserProtocol::CUserProtocol()
{

}

CUserProtocol::~CUserProtocol()
{

}

INT32 CUserProtocol::PacketUserMsg(UINT32 uiCMDID,
					void* pMSG, 
					INT32 iMSGLen, 
					StruID* pSrcID,
					StruID* pDstID,
					UINT16 uiSessionID,
					char* pDstBuf)
{
	StruUserProtocol* pUP;

	if(NULL == pDstBuf)
	{
		return -1;
	}
	if(NULL == pSrcID)
	{
		return -2;
	}
	
	pUP = (StruUserProtocol*)pDstBuf;

	pUP->Head.uiVersion = CURRENT_PROTOCOL_VERSION;
	pUP->Head.uiSession = uiSessionID;
	pUP->Head.uiCMDID = uiCMDID;
	pUP->Head.uiDataLen = iMSGLen;
	memcpy(&pUP->Head.SrcID, pSrcID, sizeof(StruID));
	if(NULL != pDstID)
	{
		memcpy(&pUP->Head.DstID, pDstID, sizeof(StruID));
	}
	else
	{
		memset(&pUP->Head.DstID, 0, sizeof(StruID));
	}
	
	if(NULL != pMSG && iMSGLen > 0)
	{
		memcpy(pUP->sData, pMSG, iMSGLen);
	}
	
	return 0;
}

INT32 CUserProtocol::DePacketUserMsg(char* pDataBuf, CNetMessage* pUserMsg)
{
	StruUserProtocol* pUserProtocol;
	if(NULL == pDataBuf || NULL == pUserMsg)
	{
		return -1;
	}
	
	pUserProtocol = (StruUserProtocol*)pDataBuf;
	pUserMsg->SetCommandID(pUserProtocol->Head.uiCMDID);
	pUserMsg->SetMSG(pUserProtocol->sData);
	pUserMsg->SetMSGLen(pUserProtocol->Head.uiDataLen);
	pUserMsg->SetSessionID(pUserProtocol->Head.uiSession);
	pUserMsg->SetSrcID(&pUserProtocol->Head.SrcID);
	pUserMsg->SetDstID(&pUserProtocol->Head.DstID);
	return 0;
}