#ifndef BASE_PROTOCOL_DEF_H
#define BASE_PROTOCOL_DEF_H

//–≠“ÈÕ∑
#define COMMU_PACK_HEADER 0xAB0000AB

typedef struct StruCommuHeader
{
	UINT32 uiHeader;
	INT32 iLength;
	INT32 iSessionID;
	INT16 iTotalPacket;
	INT16 iCurPacket;
}StruCommuHeader;

typedef struct StruCommuPacket
{
	StruCommuHeader Header;
	char Payload[4];
}StruCommuPacket;


class CBaseProtocol
{
public:
	CBaseProtocol();
	virtual ~CBaseProtocol();

protected:
	INT32 DePackData(void* pData, INT32 iDataLen, void **pHeadPos, INT32 *pPacketLen, void** pCopyData);
	INT32 PackData(void* pSrcData, 
					INT32 iSrcLen, 
					void* pDstAddr, 
					INT32 *pLen, 
					INT32 iSessionID, 
					INT16 iTotalPack, 
					INT16 iCurPacket);
protected:
	char* m_pProtocolData;
	int m_iDataSize;
};


#endif