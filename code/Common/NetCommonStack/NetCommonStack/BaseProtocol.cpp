#include "GSCommon.h"
#include "BaseProtocol.h"
#include "Log.h"

CBaseProtocol::CBaseProtocol()
{
	m_pProtocolData = NULL;
	m_iDataSize = 0;
}

CBaseProtocol::~CBaseProtocol()
{
	if(m_pProtocolData)
	{
		free(m_pProtocolData);
		m_pProtocolData = NULL;
	}
	m_iDataSize = 0;
}

/***************************************************************************
  Function:        	PackData
  DATE:				2010-5-12   12:01
  Description: 		打包函数,将用户数据按协议组包   
  Input:  			pSrcData: 用户数据
					iSrcLen: 用户数据长度
					pDstAddr:打包后的地址
					pLen:打包后的长度
  Output:     		    
  Return:   		0：打包成功，其他值表示打包失败      
  Note:						
****************************************************************************/
INT32 CBaseProtocol::PackData(void* pSrcData, INT32 iSrcLen, 
							void* pDstAddr, INT32 *pLen, 
							INT32 iSessionID, INT16 iTotalPack, INT16 iCurPacket)
{
	StruCommuPacket* pCommuPacket;

	if(iSrcLen < 0 || 
		NULL == pDstAddr ||
		NULL == pLen)
	{
        LOG2_ERROR(  "Args invalid.\n");
		return -1;
	}

	pCommuPacket = (StruCommuPacket*)pDstAddr;
	pCommuPacket->Header.uiHeader = COMMU_PACK_HEADER;
	pCommuPacket->Header.iLength = iSrcLen;
	pCommuPacket->Header.iSessionID = iSessionID;
	pCommuPacket->Header.iTotalPacket = iTotalPack;
	pCommuPacket->Header.iCurPacket = iCurPacket;
	if(iSrcLen > 0 && NULL != pSrcData)
	{
		memcpy(pCommuPacket->Payload, pSrcData, iSrcLen);
	}
	
	*pLen = sizeof(StruCommuHeader) + iSrcLen;
	return 0;
}

/***************************************************************************
  Function:        	DePackData
  DATE:				2010-5-11   11:38
  Description: 		解包函数   
  Input:  			pData:		解包数据缓冲区
					iDataLen:	数据区长度
					pHeadPos:	解包后包头位置
					pPacketLen: 包长度
  Output:     		    
  Return:   		0:解包成功，pHeadPos 和 pPacketLen 都正确赋值
					1:pHeadPos被正确赋值，但pPacketLen未被赋值
					2:缓冲区中没有包头
					3:pHeadPos 和 pPacketLen 都正确赋值但数据没收全
					4:数据收齐，但校验码错误
					-1:包头没收完
					-2:参数错误
  Note:						
****************************************************************************/
INT32 CBaseProtocol::DePackData(void* pData, INT32 iDataLen, void **pHeadPos, INT32 *pPacketLen, void** pCopyData)
{
	INT32 i;
	char* pHeader;
	INT32 iTotalLen;
	INT32 iContentLen;
	INT32 iHeadPos = 0;
	StruCommuPacket *pCommuPacket;

	if(iDataLen < sizeof(DWORD))
	{
		//如果包头还没收完，直接返回        
		return -1;
	}
	if(NULL == pData || NULL == pHeadPos || NULL == pPacketLen)
	{
         LOG2_ERROR(  "Args invalid.\n");
		return -2;
	}

	*pHeadPos = NULL;
	*pPacketLen = 0;
	pHeader = (char*)pData;
	//寻找包头
	for(i = 0; i < iDataLen - 4; i++)
	{
		if(0xAB == (pHeader[i] & 0x0ff) &&
			0x00 == (pHeader[i + 1] & 0x0ff) &&
			0x00 == (pHeader[i + 2] & 0x0ff) &&
			0xAB == (pHeader[i + 3] & 0x0ff))
		{
			//找到包头
			*pHeadPos = pHeader + i;
			iHeadPos = i;
			break;
		}
	}
	//缓冲区中无包头
	if(NULL == *pHeadPos)
	{
        LOG2_WARN( "Not found packet header, size:%d.\n",
            iDataLen);
		return 2;
	}
	//长度字段还没收完
	if(iDataLen - iHeadPos < sizeof(StruCommuHeader))
	{
		return 1;
	}
	if(NULL == m_pProtocolData)
	{
		m_pProtocolData = (char*)malloc(iDataLen);
		if(NULL == m_pProtocolData)
		{
            LOG2_FATAL(  "Malloc buffer, size:%d fail.\n",
                        iDataLen); 
			return 2;
		}
		m_iDataSize = iDataLen;
	}
	else
	{
		if(m_iDataSize < iDataLen)
		{
			free(m_pProtocolData);
			m_pProtocolData = (char*)malloc(iDataLen);
			if(NULL == m_pProtocolData)
			{
                LOG2_FATAL( "Malloc buffer, size:%d fail.\n",
                    iDataLen);  
				return 2;
			}
			m_iDataSize = iDataLen;
		}
	}
	memcpy(m_pProtocolData, *pHeadPos, iDataLen - i);
	*pCopyData = m_pProtocolData;
	pCommuPacket = (StruCommuPacket *)m_pProtocolData;
	//长度字段
	iContentLen = pCommuPacket->Header.iLength;
	if(iContentLen < 0)
	{
        LOG2_FATAL("Packet invalid of size: %d < 0 .\n",
            iContentLen); 

		return 2;
	}
	
	//包总长应该是 头 + 长度字段 + 内容长度
	iTotalLen = iContentLen + sizeof(StruCommuHeader);
	*pPacketLen = iTotalLen;
	if(iDataLen - iHeadPos < iTotalLen)
	{        
		return 3;
	}
	return 0;
}