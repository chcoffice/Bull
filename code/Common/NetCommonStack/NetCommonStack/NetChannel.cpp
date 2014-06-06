#include "GSCommon.h"
#include "NetChannel.h"
#include "NetMessage.h"
#include <assert.h>
#ifdef _WIN32
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include "Log.h"

CNetChannel::CNetChannel()
{
	m_pRecvBuf = NULL;
	m_pSocketChn = NULL;
	m_pNetService = NULL;

	m_pRecvPackBuf = NULL;
	m_iRecvPackBufSize = DEFAULT_MSG_SIZE * 2;
	m_iRecvPackLen = 0;
	m_pExtRecvPackBuf = NULL;
	m_iExtRecvPackBufSize = 0;

	m_pSendPackBuf = NULL;
	m_iSendPackBufSize = DEFAULT_MSG_SIZE * 2;
	m_iSendPackLen = 0;
	m_pExtSendPackBuf = NULL;
	m_iExtSendPackBufSize = 0;

	m_iNetChnStatus = NET_CHANNEL_STATUS_INVALID;
	m_iCurRecvPos = 0;

	m_iNetChnType = NET_CHANNEL_TYPE_CLIENT;

	m_iSendPulseCount = 0;
	m_iReConnectCount = 0; 
	m_iNetRecvDataCount = 0;

	m_pBufferArray = NULL;
	memset(&m_PrePacketHeader, 0, sizeof(StruCommuHeader));

	m_iSessionID = 0;
	m_bSendPulse = 0;

	m_bSendDisconnectEvent = 0;
}

CNetChannel::~CNetChannel()
{
	ReleaseNetChannel();
}

INetConfig* CNetChannel::GetChannelCfg()
{
	return this;
}

void CNetChannel::SetChannelCfg(INetConfig* pCfg)
{
	CNetConfig::SetNetConfig(pCfg);
}

INT32 CNetChannel::InitNetChannel(CBufferArray* pBufferArray)
{
	INT32 iMsgSize;
	INT32 iRet = 0;

	iMsgSize = GetMaxMsgSize();
	if(NULL == m_pRecvBuf)
	{
		m_pRecvBuf = (char*)malloc(iMsgSize * 2);
		if(NULL == m_pRecvBuf)
		{
            LOG2_FATAL( "Malloc %d fail.\n", iMsgSize * 2);
			return -1;
		}
		memset(m_pRecvBuf, 0, iMsgSize * 2);
	}
	if(NULL == m_pRecvPackBuf)
	{
		m_pRecvPackBuf = (char*)malloc(iMsgSize * 2);
		if(NULL == m_pRecvPackBuf)
		{
            LOG2_FATAL("Malloc %d fail.\n", iMsgSize * 2);

			return -1;
		}
		memset(m_pRecvPackBuf, 0, iMsgSize * 2);
		m_iRecvPackBufSize = iMsgSize * 2;
	}
	if(NULL == m_pSendPackBuf)
	{
		m_pSendPackBuf = (char*)malloc(iMsgSize * 2);
		if(NULL == m_pSendPackBuf)
		{
            LOG2_FATAL("Malloc %d fail.\n", iMsgSize * 2);

			return -1;
		}
		memset(m_pSendPackBuf, 0, iMsgSize * 2);
		m_iSendPackBufSize = iMsgSize * 2;
	}
	m_pBufferArray = pBufferArray;

	iRet = m_MgrSendBuffer.InitBufferMgr(pBufferArray);
	if(iRet < 0)
	{
        LOG2_ERROR( "m_MgrSendBuffer.InitBufferMgr fail.\n");
		return -1;
	}
	iRet = m_MgrRecvBuffer.InitBufferMgr(pBufferArray);
	return iRet;
}

INT32 CNetChannel::ReleaseNetChannel()
{
	if(m_pSocketChn)
	{
        LOG2_DEBUG( "ChnType(%d) Close Socket: %s:%d => %s:%d .\n",
                        GetChnType(),                       
                        GetLocalIPAddrString(),GetLocalPort(),
                        GetRemoteIPAddrString(),GetRemotePort()); 
		m_pSocketChn->CloseChannelEx();
		m_pSocketChn = NULL;
	}

	if(m_pRecvBuf)
	{
		free(m_pRecvBuf);
		m_pRecvBuf = NULL;
	}
	if(m_pRecvPackBuf)
	{
		free(m_pRecvPackBuf);
		m_pRecvPackBuf = NULL;
	}
	if(m_pExtRecvPackBuf)
	{
		free(m_pExtRecvPackBuf);
		m_pExtRecvPackBuf = NULL;
	}
	if(m_pSendPackBuf)
	{
		free(m_pSendPackBuf);
		m_pSendPackBuf = NULL;
	}
	if(m_pExtSendPackBuf)
	{
		free(m_pExtSendPackBuf);
		m_pExtSendPackBuf = NULL;
	}

	m_bSendDisconnectEvent = 0;
	return 0;
}

char* CNetChannel::GetRemoteIPAddrString()
{
	UINT16 uiPort;
	if(NULL == m_pSocketChn)
	{
		return m_szRemoteIPAddr;
	}
	m_pSocketChn->GetReMoteIPPort(m_szRemoteIPAddr, uiPort);
	return m_szRemoteIPAddr;
}
UINT32 CNetChannel::GetRemoteIPAddr()
{
	UINT16 uiPort;
	if(NULL == m_pSocketChn)
	{
		return m_uiRemoteIPAddr;
	}
	m_pSocketChn->GetReMoteIPPort(m_szRemoteIPAddr, uiPort);
	return inet_addr(m_szRemoteIPAddr);
}
char* CNetChannel::GetLocalIPAddrString()
{
	UINT16 uiPort;
	if(NULL == m_pSocketChn)
	{
		return m_szBindingIPAddr;
	}
	m_pSocketChn->GetLocalIPPort(m_szBindingIPAddr, uiPort);
	return m_szBindingIPAddr;
}
UINT32 CNetChannel::GetLocalIPAddr()
{
	UINT16 uiPort;
	if(NULL == m_pSocketChn)
	{
		return m_uiLocalIPAddr;
	}
	m_pSocketChn->GetLocalIPPort(m_szBindingIPAddr, uiPort);
	return inet_addr(m_szBindingIPAddr);
}
UINT16 CNetChannel::GetRemotePort()
{
	UINT16 uiPort;
	if(NULL == m_pSocketChn)
	{
		return m_uiRemotePort;
	}
	m_pSocketChn->GetReMoteIPPort(m_szRemoteIPAddr, uiPort);
	return uiPort;
}
UINT16 CNetChannel::GetLocalPort()
{
	UINT16 uiPort;
	if(NULL == m_pSocketChn)
	{
		return m_uiLocalPort;
	}
	m_pSocketChn->GetLocalIPPort(m_szBindingIPAddr, uiPort);
	return uiPort;
}

INT32 CNetChannel::ConnectServer(char* pszRemoteIPAddr, UINT16 uiRemotePort,
							char* pszBindingIPAddr, UINT16 uiBindingPort)
{
   
	INT32 iRet;
	enumNetProtocolType euPType;

    LOG2_INFO("ChnType(%d) Connect %s:%d bind:%s:%d", 
        GetChnType(),
        pszRemoteIPAddr, uiRemotePort, 
        pszBindingIPAddr ? pszBindingIPAddr : "0" , uiRemotePort );
	if(m_pSocketChn)
	{
        LOG2_DEBUG("ChnType(%d)  Close Socket: %s:%d => %s:%d .\n",
            GetChnType(),
            GetLocalIPAddrString(),GetLocalPort(),
            GetRemoteIPAddrString(),GetRemotePort()); 
        m_MutexSend.Lock();
        ISocketChannel *pSocket = m_pSocketChn;
        m_pSocketChn = NULL;
        m_MutexSend.Unlock();
        if( pSocket )
        {
            pSocket->CloseChannelEx();
        }
	}

	if(NULL == m_pSocketChn)
	{
		if(m_pNetService)
		{
			if(GetProtocolType() == PROTOCOL_TYPE_TCP)
			{
				euPType = NET_PROTOCOL_TCP;
			}
			else
			{
				euPType = NET_PROTOCOL_UDP;
			}
            ISocketChannel *pSocket = NULL;
			iRet = m_pNetService->AddClientChannel(m_szRemoteIPAddr, 
													uiRemotePort, 
													pszBindingIPAddr, 
													uiBindingPort, 
													euPType, 
													&pSocket);
			if(iRet < 0 || NULL == pSocket)
			{
                LOG2_ERROR("ChnType(%d) Connect %s:%d bind:%s:%d protype:%d fail.", 
                    GetChnType(),
                    pszRemoteIPAddr, uiRemotePort, 
                    pszBindingIPAddr ? pszBindingIPAddr : "0" , uiRemotePort,(int) euPType );
				return -1;
			}
            SetSocketChn( pSocket, NET_CHANNEL_STATUS_ACTIVE );

            LOG2_DEBUG("ChnType(%d)  Connect Socket: %s:%d => %s:%d  OK.\n",
                GetChnType(),
                GetLocalIPAddrString(),GetLocalPort(),
                GetRemoteIPAddrString(),GetRemotePort()); 
		}
		else
		{
            CMM_ASSERT(0);
			return -1;
		}
	}
	else
	{
		iRet = m_pSocketChn->ReConnect();
	}	
	return iRet;
}
/***************************************************************************
  Function:        	SendRemoteData
  DATE:				2010-8-5   20:59
  Description: 		向远端发送数据   
  Input:  			pData:发送数据地址
					iDataLen:发送数据长度
  Output:     		    
  Return:   		发送字节数      
  Note:						
****************************************************************************/
INT32 CNetChannel::SendRemoteData(char* pData, INT32 iDataLen)
{
	INT32 iPacketLen = 0;
	INT32 iRet;
	INT32 iBufSize;
	INT32 iSendPos = 0;
	INT32 iTotal = 0;
	INT32 iCur = 0;
	char* pDstBuf;
	CBufferObj* pFreeBuf; 
	INT32 i;
	INT32 iSendLen;
	INT32 iResLen = 0;
	INT32 iSessionID;
	CBufferObj* pObjBuf;

	if(NULL == m_pSocketChn)
	{
        LOG2_ERROR("m_pSocketChn is null\n");
		return -1;
	}
	if(NULL == m_pBufferArray)
	{
        LOG2_ERROR("m_pBufferArray is null\n");
		return -2;
	}

	if(iDataLen > 0)
	{
		m_iSessionID++;
		if(0 == m_iSessionID)
		{
			m_iSessionID = 1;
		}
		iSessionID = m_iSessionID;
	}
	else
	{
		iSessionID = 0;
	}

	
	//每个包的大小
	iBufSize = m_pBufferArray->GetBuferSize();
	if(iBufSize > GetMaxMsgSize())
	{
		iBufSize = GetMaxMsgSize();
	}
	iTotal = iDataLen / (iBufSize - sizeof(StruCommuHeader));
	if(0 == iDataLen)
	{
		iTotal = 1;
	}
	else if(iDataLen % (iBufSize - sizeof(StruCommuHeader)))
	{
		iTotal++;
	}

	iResLen = iDataLen;
	for(i = 0; i < iTotal; i++)
	{
		pFreeBuf = m_pBufferArray->GetFreeBuffer();
		if(NULL == pFreeBuf)
		{
            LOG2_ERROR("GetFreeBuffer fail. i:%d\n", i );
			return -3;
		}
		pDstBuf = pFreeBuf->GetData();
		if(NULL == pDstBuf)
		{
             LOG2_ERROR("GetData fail. i:%d\n", i );
			return -4;
		}
		iCur++;
		if(iResLen < (INT32)(iBufSize - sizeof(StruCommuHeader)))
		{
			iSendLen = iResLen;
		}
		else
		{
			iSendLen = iBufSize - sizeof(StruCommuHeader);
		}
		
		PackData(pData + iSendPos, iSendLen, 
			pDstBuf, &iPacketLen, 
			iSessionID, iTotal, iCur);
		iSendPos += iSendLen;
		iResLen -= iSendLen;
		pFreeBuf->SetDataSize(iPacketLen);

		m_MgrSendBuffer.PushBuffer(pFreeBuf);
	}

	while(TRUE)
	{
		pObjBuf = m_MgrSendBuffer.PopBuffer();
		if(NULL == pObjBuf)
		{
			break;
		}
		int iLen = pObjBuf->GetDataSize();
		iRet = m_pSocketChn->SendData(pObjBuf->GetData(), iLen);
		m_pBufferArray->FreeBuffer(pObjBuf);
        if(iRet != ERROR_BASE_SUCCESS  )
        {
            LOG2_ERROR("ChnType(%d) Socket: %s:%d => %s:%d  SendData fail, Ret:%d\n",
                GetChnType(),
                GetLocalIPAddrString(),GetLocalPort(),
                GetRemoteIPAddrString(),GetRemotePort(), iRet); 

			m_bSendDisconnectEvent = 1;
			SetChnStatus(NET_CHANNEL_STATUS_SOCKET_ERROR); //禁止再发送


            return -5;
        }
	}

	return 0;
}

INT32 CNetChannel::SendRemoteMsg(UINT32 uiCMDID, 
								 void* pMSG, 
								 INT32 iMSGLen, 
								 StruID* pSrcID, 
								 StruID* pDstID, 
								 UINT16 uiSessionID)
{
	INT32 iRet;
	INT32 iTotalDataLen;

   

	m_MutexSend.Lock();

	if( m_iNetChnStatus != NET_CHANNEL_STATUS_ACTIVE )
	{
		m_MutexSend.Unlock();
		return -64;
	}

	iTotalDataLen = iMSGLen + sizeof(StruUserProtocolHead);
	if(iTotalDataLen > m_iSendPackBufSize) //需要重新分配发送缓冲区大小
	{
		m_pExtSendPackBuf = (char*)malloc(iTotalDataLen);
		if(NULL == m_pExtSendPackBuf)
		{
			m_MutexSend.Unlock();
			return -1;
		}
		memset(m_pExtSendPackBuf, 0, iTotalDataLen);
	}
	else
	{
		m_pExtSendPackBuf = m_pSendPackBuf;
	}

	iRet = PacketUserMsg(uiCMDID, pMSG, iMSGLen, pSrcID, pDstID, uiSessionID, m_pExtSendPackBuf);
	if(iRet < 0)
	{
        LOG2_ERROR("ChnType(%d)  %s:%d => %s:%d  PacketUserMsg fail, Ret:%d\n",
            GetChnType(),
            GetLocalIPAddrString(),GetLocalPort(),
            GetRemoteIPAddrString(),GetRemotePort(), iRet); 
		m_MutexSend.Unlock();
		return -2;
	}
	iRet = SendRemoteData(m_pExtSendPackBuf, iTotalDataLen);
	if(iTotalDataLen > m_iSendPackBufSize)
	{
		free(m_pExtSendPackBuf);
	}
    if( iRet < 0 )
    {
        LOG2_ERROR("ChnType(%d)  %s:%d => %s:%d  SendRemoteData %d fail, Ret:%d\n",
            GetChnType(),
            GetLocalIPAddrString(),GetLocalPort(),
            GetRemoteIPAddrString(),GetRemotePort(),iTotalDataLen, iRet); 
    }
	m_pExtSendPackBuf = NULL;
	m_MutexSend.Unlock();
	return iRet;
}


INT32 CNetChannel::CloseChannel()
{
	if(m_pSocketChn)
	{
        LOG2_DEBUG( "ChnType(%d) Close Socket: %s:%d => %s:%d .\n",
            GetChnType(),           
            GetLocalIPAddrString(),GetLocalPort(),
            GetRemoteIPAddrString(),GetRemotePort()); 
        m_MutexSend.Lock();
        ISocketChannel *pSocket = m_pSocketChn;
        m_pSocketChn = NULL;
        m_MutexSend.Unlock();
        if( pSocket )
        {
            pSocket->CloseChannelEx();
        }
	}
	CGSAutoMutex locker( &m_MutexSend);
	m_iSendPulseCount = 0;
	m_iReConnectCount = 0; 
	m_iNetRecvDataCount = 0;
	m_iCurRecvPos = 0;
	m_iNetChnStatus = NET_CHANNEL_STATUS_INVALID;
	m_bSendPulse = 0;
	m_bSendDisconnectEvent = 0;
	return 0;
}

INT32 CNetChannel::CloseChannelEx()
{
	if(m_pSocketChn)
	{
		LOG2_DEBUG( "ChnType(%d) Close Socket: %s:%d => %s:%d .\n",
			GetChnType(),           
			GetLocalIPAddrString(),GetLocalPort(),
			GetRemoteIPAddrString(),GetRemotePort()); 
		m_MutexSend.Lock();
		ISocketChannel *pSocket = m_pSocketChn;
		m_pSocketChn = NULL;
		m_MutexSend.Unlock();
		if( pSocket )
		{
			pSocket->CloseChannel();
		}
	}
	CGSAutoMutex locker( &m_MutexSend);
	m_iSendPulseCount = 0;
	m_iReConnectCount = 0; 
	m_iNetRecvDataCount = 0;
	m_iCurRecvPos = 0;
	m_iNetChnStatus = NET_CHANNEL_STATUS_INVALID;
	m_bSendPulse = 0;
	m_bSendDisconnectEvent = 0;
	return 0;
}


INT32 CNetChannel::GetChnStatus()
{

	if(NET_CHANNEL_STATUS_INVALID == m_iNetChnStatus)
	{
		return m_iNetChnStatus;
	}
	if(NULL == m_pSocketChn)
	{
		m_iNetChnStatus = NET_CHANNEL_STATUS_SHUTDOWN;
		return m_iNetChnStatus;
	}
	return m_iNetChnStatus;
}

ISocketChannel* CNetChannel::GetSocketChn()
{
	return m_pSocketChn;
}

void CNetChannel::SetSocketChn(ISocketChannel* pChn,INT32 iChnStatus)
{
	if( m_pSocketChn )
	{
        if( pChn )
        {

            LOG2_DEBUG( "ChnType(%d) Exist Socket: %s:%d => %s:%d.\n",
                GetChnType(),             
                GetLocalIPAddrString(),GetLocalPort(),
                GetRemoteIPAddrString(),GetRemotePort()); 
            CMM_ASSERT(0);
        }
        m_MutexSend.Lock();
        ISocketChannel *pSocket = m_pSocketChn;
        m_pSocketChn = NULL;
        m_MutexSend.Unlock();
        if( pSocket )
        {
             pSocket->CloseChannelEx();
         }
	}
    CGSAutoMutex locker( &m_MutexSend);
	m_bSendPulse = 0;
	m_pSocketChn = pChn;
    m_iNetChnStatus = iChnStatus;
	m_bSendDisconnectEvent = 0;
}

void CNetChannel::SetChnStatus(INT32 iChnStatus)
{
	m_iNetChnStatus = iChnStatus;
}

INT32 CNetChannel::GetChnType()
{
	return m_iNetChnType;
}
void CNetChannel::SetChnType(INT32 iChnType)
{
	if(iChnType < NET_CHANNEL_TYPE_CLIENT || iChnType > NET_CHANNEL_TYPE_SERVER)
	{
		return;
	}
	m_iNetChnType = iChnType;
}

INT32 CNetChannel::RecvPacketSlice(StruCommuPacket* pCommuPacket)
{
	fnNetEvent pNetEventFn;
	void* pNetEventCaller;
	CBufferObj* pFreeBuf;
	INT32 iDone = 0;
	INT32 iTotalLen = 0;
	CNetMessage UserMsg;
	INT32 iRet;

	if(NULL == pCommuPacket)
	{
        CMM_ASSERT(0);
		return -1;
	}

	if(pCommuPacket->Header.iSessionID != m_PrePacketHeader.iSessionID)//切换Session
	{
		if(0 == m_PrePacketHeader.iSessionID) //第一个包
		{
		}
		else //Session 丢包了
		{
            LOG2_DEBUG( "ChnType(%d)  %s:%d => %s:%d. Lost packet.\n",
                GetChnType(),             
                GetLocalIPAddrString(),GetLocalPort(),
                GetRemoteIPAddrString(),GetRemotePort()); 
			//清空
			m_MgrRecvBuffer.ClearBuffer();
			//暂时不做重传处理
		}

		if(pCommuPacket->Header.iTotalPacket > 1)
		{
			memcpy(&m_PrePacketHeader, &pCommuPacket->Header, sizeof(StruCommuHeader));
		}
		else//只有1个包
		{
			iDone = 1;
		}
		pFreeBuf = m_pBufferArray->GetFreeBuffer();
		if(NULL == pFreeBuf)
		{
			return -2;
		}
		pFreeBuf->SetData(this, pCommuPacket->Payload, pCommuPacket->Header.iLength);
		m_MgrRecvBuffer.PushBuffer(pFreeBuf);
	}
	else
	{
		pFreeBuf = m_pBufferArray->GetFreeBuffer();
		if(NULL == pFreeBuf)
		{
            LOG2_FATAL( "GetFreeBuffer fail.\n");
			return -2;
		}
		pFreeBuf->SetData(this, pCommuPacket->Payload, pCommuPacket->Header.iLength);
		m_MgrRecvBuffer.PushBuffer(pFreeBuf);
		if(pCommuPacket->Header.iTotalPacket == pCommuPacket->Header.iCurPacket)
		{
			iDone = 1;
			memset(&m_PrePacketHeader, 0, sizeof(m_PrePacketHeader));
		}
	}


	if(iDone)
	{
		if(NULL == m_pRecvPackBuf || m_iRecvPackBufSize <= 0)
		{
			return -1;
		}
		iTotalLen = m_MgrRecvBuffer.GetDataLen();
		if(iTotalLen > m_iRecvPackBufSize)
		{
			m_pExtRecvPackBuf = (char*)malloc(iTotalLen);
			if(NULL == m_pExtRecvPackBuf)
			{
                LOG2_FATAL( "malloc %d fail.\n", iTotalLen);
				return -2;
			}
		}
		else
		{
			m_pExtRecvPackBuf = m_pRecvPackBuf;
		}
		m_MgrRecvBuffer.GetData(m_pExtRecvPackBuf);
		m_MgrRecvBuffer.ClearBuffer();

		iRet = DePacketUserMsg(m_pExtRecvPackBuf, &UserMsg);
		if(UserMsg.GetMSGLen() > iTotalLen)
		{
			char* pTempMsg = (char*)malloc(UserMsg.GetMSGLen());
			if(NULL == pTempMsg)
			{
                LOG2_FATAL( "malloc %d fail.\n", UserMsg.GetMSGLen());
				return -2;
			}
			memcpy(pTempMsg, m_pExtRecvPackBuf, iTotalLen);
			free(m_pExtRecvPackBuf);
			m_pExtRecvPackBuf = pTempMsg;
			iRet = DePacketUserMsg(m_pExtRecvPackBuf, &UserMsg);
		}

		pNetEventFn = GetNetEventEntry();
		pNetEventCaller = GetNetEventCaller();
		if(pNetEventFn)
		{			
			pNetEventFn(pNetEventCaller, this, NET_EVENT_READ, &UserMsg);
		}
		if(iTotalLen > m_iRecvPackBufSize)
		{
			free(m_pExtRecvPackBuf);
		}
		m_pExtRecvPackBuf = NULL;
	}
	return 0;
}

INT32 CNetChannel::OnDataArrive(void* pData, INT32 iDataLen)
{
	INT32 iRet;
	char* pMSG = NULL;
	INT32 iMSGLen;
	INT32 iPayloadLen;
	char* pCopyData;

	INT32 iHeadPos;
	INT32 iFlag = 0;

	char* pHead;
	INT32 iResLen;

	m_Mutex.Lock();

	memcpy(m_pRecvBuf + m_iCurRecvPos, pData, iDataLen);
	m_iCurRecvPos += iDataLen;
	
	pHead = m_pRecvBuf;
	iResLen = m_iCurRecvPos;
	while(TRUE)
	{
		iRet = DePackData(pHead, iResLen, (void**)&pMSG, &iMSGLen, (void**)&pCopyData);
		switch(iRet)
		{
		case 0://成功收到包
			m_iNetRecvDataCount = 0;
			iPayloadLen = iMSGLen - sizeof(StruCommuHeader);
			if(0 == iPayloadLen)//心跳包
			{
                LOG2_DEBUG( "ChnType(%d)  %s:%d => %s:%d. Rcv Pulse packet On:%ld.\n",
                    GetChnType(),             
                    GetLocalIPAddrString(),GetLocalPort(),
                    GetRemoteIPAddrString(),GetRemotePort(), (long) time(NULL)); 
				OnRecvPulse();
			}
			else
			{
				StruCommuPacket* pCommuPacket;
				pCommuPacket = (StruCommuPacket*)pCopyData;
				RecvPacketSlice(pCommuPacket);
			}
			iHeadPos = pMSG - pHead;
			if(iHeadPos + iMSGLen < iResLen)
			{
				pHead = pHead + iHeadPos + iMSGLen;
				iResLen = iResLen - iHeadPos - iMSGLen;
				iFlag = 1;
			}
			else
			{
				m_iCurRecvPos = 0;
				iFlag = 0;
			}
			break;
		case 1://包没有收完
		case 3:
		case -1:
			iFlag = 0;
			if(pHead != m_pRecvBuf)
			{
				memcpy(m_pRecvBuf, pHead, iResLen);
				m_iCurRecvPos = iResLen;
			}
			break;
		case 2://无包头
			if(iResLen >= 4)
			{
				memcpy(m_pRecvBuf, pHead + iResLen -3, 3);
				m_iCurRecvPos = 3; //剩余3字节有可能是头的一部分
			}
			else
			{
				//never happend
                CMM_ASSERT(0);
				m_iCurRecvPos = 0;
			}
			iFlag = 0;
			break;
		default:
			iFlag = 0;
			break;
		}
		if(!iFlag)
		{
			break;
		}
	}
	m_Mutex.Unlock();
	return 0;
}

void CNetChannel::OnRecvPulse()
{
	if( NET_CHANNEL_TYPE_SERVER == GetChnType() )
	{
		m_bSendPulse = 2;
	}
}

void CNetChannel::SendPulse()
{
	if( m_bSendPulse && m_iNetChnStatus == NET_CHANNEL_STATUS_ACTIVE )
	{
		m_bSendPulse = 0;
		LOG2_DEBUG( "ChnType(%d)  %s:%d => %s:%d. Send Pulse packet On: %ld.\n",
			GetChnType(),             
			GetLocalIPAddrString(),GetLocalPort(),
			GetRemoteIPAddrString(),GetRemotePort(),
			(long)time(NULL) ); 

		
		m_MutexSend.Lock();
		SendRemoteData(NULL, 0);
		m_MutexSend.Unlock();
	}
}


INT32 CNetChannel::OnCheckStatus()
{
	INT32 iNetDataTimeout;
	INT32 iRet;
	fnNetEvent OnNetEventCB;

	if(NET_CHANNEL_STATUS_INVALID == m_iNetChnStatus)
	{
		return 0;
	}

	SendPulse();

	
	//长期收不到网络包
	m_iNetRecvDataCount++;
	iNetDataTimeout = GetNetDataTimeout();

	if(m_iNetChnStatus != NET_CHANNEL_STATUS_ACTIVE)
	{
        LOG2_DEBUG( "ChnType(%d)  %s:%d => %s:%d. Set timeout.\n",
            GetChnType(),             
            GetLocalIPAddrString(),GetLocalPort(),
            GetRemoteIPAddrString(),GetRemotePort()); 
		m_iNetRecvDataCount = iNetDataTimeout + 1;
	}

	if( m_iNetRecvDataCount == iNetDataTimeout
		|| (m_bSendDisconnectEvent && m_iNetChnStatus != NET_CHANNEL_STATUS_ACTIVE) )
	{
		LOG2_DEBUG( "ChnType(%d)  %s:%d => %s:%d. Keeplavie timeout %d>=%d. IsDisCnn:%d\n",
            GetChnType(),             
            GetLocalIPAddrString(),GetLocalPort(),
            GetRemoteIPAddrString(),GetRemotePort(),
            m_iNetRecvDataCount,iNetDataTimeout, m_bSendDisconnectEvent ); 
		m_iNetRecvDataCount = 0;
		m_bSendDisconnectEvent = 0;
		OnNetEventCB = GetNetEventEntry();
		if(NULL != OnNetEventCB)
		{
			SetChnStatus(NET_CHANNEL_STATUS_SHUTDOWN);
			OnNetEventCB(GetNetEventCaller(), this, NET_EVENT_CLOSE, NULL);
		}
		return 0;
	}

	if( m_iNetRecvDataCount > iNetDataTimeout)
	{
		m_iNetRecvDataCount = iNetDataTimeout;
	}


	if(NET_CHANNEL_TYPE_CLIENT == GetChnType())
	{
		INT32 iPulseTime = GetNetPulseTime();
		//客户端需要主动发送心跳包，掉线需要重连
        if( m_iNetChnStatus == NET_CHANNEL_STATUS_ACTIVE )
        {
		    m_iSendPulseCount++;
        }
		if( m_iSendPulseCount * 1250 >= iPulseTime )
		{
            //发送心跳
//             m_MutexSend.Lock();
// 			SendRemoteData(NULL, 0);
//             m_MutexSend.Unlock();
			m_bSendPulse = 1;
			m_iSendPulseCount = 0;
		}
		//重连操作
		INT32 iReConnect = GetSupportReConnect();
		INT32 iNetStat = GetChnStatus();
		if( !m_bSendDisconnectEvent && 
			(NET_CHANNEL_STATUS_NO_CONNECT == iNetStat || 
			NET_CHANNEL_STATUS_SOCKET_ERROR == iNetStat ||
			NET_CHANNEL_STATUS_SHUTDOWN == iNetStat) )
		{
			if(SUPPORT_RECONNECT == iReConnect)
			{

				m_iReConnectCount++;
				if(m_iReConnectCount * 1000 >= GetReConnectTime())
				{
                    LOG2_ERROR( "ChnType(%d)  %s:%d => %s:%d. Reconnect status:%d  On: %ld.\n",
                        GetChnType(),             
                        GetLocalIPAddrString(),GetLocalPort(),
                        GetRemoteIPAddrString(),GetRemotePort(),
                        iNetStat,
                        (long)time(NULL) ); 

					m_iReConnectCount = 0;
					INT32 iReuse = GetReuseNetPortFlag();
					if (iReuse == REUSE_NETPORT_FLAG)
					{
						iRet = ConnectServer(GetRemoteIPAddrString(), GetRemotePort(), 
							GetLocalIPAddrString(), GetLocalPort());
					}
					else
					{
						iRet = ConnectServer(GetRemoteIPAddrString(), GetRemotePort(), 
							"", 0);
					}
					
					if(0 == iRet)
					{
                        LOG2_ERROR( "Connect %s:%d bind: %s:%d  fail.\n",
                                    GetRemoteIPAddrString(), GetRemotePort(), 
                                    GetLocalIPAddrString(), GetLocalPort() ); 
						m_iNetRecvDataCount = 0;
						OnNetEventCB = GetNetEventEntry();
						if(NULL != OnNetEventCB)
						{
							OnNetEventCB(GetNetEventCaller(), this, NET_EVENT_CONNECT, NULL);
						}
					}
				}
			}
		}
	}

	return 0;
}


INetService* CNetChannel::GetNetService()
{
	return m_pNetService;
}
void CNetChannel::SetNetService(INetService* pNS)
{
	m_pNetService = pNS;
}



void CNetChannel::SetLogicRemoteIPAddrString(const char* pszIPAddr)
{
	CBaseChannel::SetLogicRemoteIPAddrString(pszIPAddr);
}
char* CNetChannel::GetLogicRemoteIPAddrString()
{
	return CBaseChannel::GetLogicRemoteIPAddrString();
}