#include "GSCommon.h"
#include "NetCommuBase.h"
#include "Log.h"

CNetCommuBase::CNetCommuBase()
{
	m_pNetChannelArray = NULL;
	//m_pThreadScanMsg = NULL;
	//m_pThreadChannelRoutine = NULL;
}

CNetCommuBase::~CNetCommuBase()
{
	StopNetBase();
}

#include "time.h" 
CNetChannel* CNetCommuBase::InsertNetChannel(ISocketChannel* pChn, INT32 iNetChnStatus)
{
	CNetChannel* pNetChn;
	INT32 i;
	int iChnNum;
	INT32 iRet;
	INetConfig* pCfg;
	INetConfig* pChnCfg;

	pCfg = GetNetConfig();

	
	if(NULL == m_pNetChannelArray)
	{
		return NULL;
	}

	iChnNum = GetMaxChnNum();
	for(i = 0; i < iChnNum; i++)
	{
		pNetChn = m_pNetChannelArray[i];
		if(NULL == pNetChn)
		{
			pNetChn = new CNetChannel;
			if(NULL == pNetChn)
			{
				return NULL;
			}
			pNetChn->SetNetConfig(pCfg);
			pChnCfg = pNetChn->GetNetConfig();
			if(SUPPORT_MESSAGE_BUF == pChnCfg->GetSupportMsgBuf())
			{
				pChnCfg->SetNetEventCB(this, ChannelEventCB);
			}
			pNetChn->SetNetConfig(pChnCfg);
			iRet = pNetChn->InitNetChannel(&m_BufferSlickArray);
			if(iRet != 0)
			{
				return NULL;
			}
			m_pNetChannelArray[i] = pNetChn;
		}
		if(NET_CHANNEL_STATUS_INVALID != pNetChn->GetChnStatus())
		{
			continue;
		}
        LOG2_INFO("NetChn: %p On Index: %d \n ",pNetChn, i );
		pNetChn->SetSocketChn(pChn, iNetChnStatus);
		return pNetChn;
	}
    LOG2_WARN("Not free,flowout: %d\n", iChnNum);
	return NULL;
}


CNetChannel* CNetCommuBase::FindNetChannelBySockChn(ISocketChannel* pChn)
{
	CNetChannel* pNetChn;
	INT32 i;
	int iChnNum;

	if(NULL == m_pNetChannelArray || NULL == pChn)
	{
		return NULL;
	}
	iChnNum = GetMaxChnNum();
	for(i = 0; i < iChnNum; i++)
	{
		pNetChn = m_pNetChannelArray[i];
		if(NULL == pNetChn)
		{
			continue;
		}
		if(NET_CHANNEL_STATUS_INVALID == pNetChn->GetChnStatus())
		{
			continue;
		}
		if(pChn == pNetChn->GetSocketChn())
		{
			return pNetChn;
		}
	}
	return NULL;
}


INT32 CNetCommuBase::OnSocketEvent(ISocketChannel* pChn, INT32 iEvent, void* pData, INT32 iDataLen)
{
	fnNetEvent OnNetEventCB;
	CNetChannel* pNetChn;

	OnNetEventCB = GetNetEventEntry();

	switch(iEvent)
	{
	case NET_ACCEPT:
		/*if(NULL != pChn)
		{
			char szIPAddr[20];
			WORD wPort = 0;
			pChn->GetReMoteIPPort(szIPAddr, wPort);
			if(strcmp(szIPAddr, "192.168.15.46") == 0 &&
				10087 == wPort)
			{
				printf("192.168.15.46:10087 连接到服务器\n");
			}
		}*/
		pNetChn = InsertNetChannel(pChn, NET_CHANNEL_STATUS_ACTIVE);
		if(NULL == pNetChn)
		{
			pChn->CloseChannelEx();
            LOG2_WARN("插入新通道失败.\n"); 		
			break;
		}
		pNetChn->SetChnType(NET_CHANNEL_TYPE_SERVER);
		if(OnNetEventCB)
		{
			OnNetEventCB(GetNetEventCaller(), pNetChn, NET_EVENT_CONNECT, NULL);
		}
		break;
	case NET_READ:

		/*if(NULL != pChn)
		{
			char szIPAddr[20];
			WORD wPort = 0;
			pChn->GetReMoteIPPort(szIPAddr, wPort);
			if(strcmp(szIPAddr, "192.168.15.46") == 0 &&
				10087 == wPort)
			{
				printf("192.168.15.46:10087 发送数据\n");
			}
		}*/
		pNetChn = FindNetChannelBySockChn(pChn);
		if(NULL == pNetChn)
		{
			LOG2_WARN("服务端无此通道记录.\n"); 			
			CMM_ASSERT(0);
			break;
		}
		pNetChn->OnDataArrive(pData, iDataLen);
		break;
	case NET_TIMEOUT:
		break;
	case NET_REMOTE_DISCNN:
	case NET_CLOSE:

		/*if(NULL != pChn)
		{
			char szIPAddr[20];
			WORD wPort = 0;
			pChn->GetReMoteIPPort(szIPAddr, wPort);
			if(strcmp(szIPAddr, "192.168.15.46") == 0 &&
				10087 == wPort)
			{
				printf("192.168.15.46:10087 断开连接\n");
			}
		}*/

		pNetChn = FindNetChannelBySockChn(pChn);
		if(NULL == pNetChn)
		{
            LOG2_WARN("服务端无此通道记录.\n"); 			
            CMM_ASSERT(0);
			break;
		}

        LOG2_DEBUG("ChnType(%d) %s:%d=>%s:%d Disconnect of socket event:%d\n", 
            pNetChn->GetChnType(),
            pNetChn->GetLocalIPAddrString(),pNetChn->GetLocalPort(),
            pNetChn->GetRemoteIPAddrString(),pNetChn->GetRemotePort(),             
            (int)iEvent);

		if(NET_CHANNEL_TYPE_CLIENT == pNetChn->GetChnType())
		{
			pNetChn->SetChnStatus(NET_CHANNEL_STATUS_NO_CONNECT);
		}
		if(OnNetEventCB)
		{
			OnNetEventCB(GetNetEventCaller(), pNetChn, NET_EVENT_CLOSE, NULL);
		}
		else
		{
			pNetChn->CloseChannel();
		}
		break;
	case NET_RECONNECT_SUCCESS:
		pNetChn = FindNetChannelBySockChn(pChn);
		if(NULL == pNetChn)
		{
            LOG2_WARN("服务端无此通道记录.\n"); 			
            CMM_ASSERT(0);
		}
		pNetChn->SetChnType(NET_CHANNEL_TYPE_CLIENT);
		pNetChn->SetChnStatus(NET_CHANNEL_STATUS_ACTIVE); 
		if(OnNetEventCB)
		{
			OnNetEventCB(GetNetEventCaller(), pNetChn, NET_EVENT_CONNECT_SERVER, NULL);
		}
		break;
	default:
		break;
	}
	return 0;
}

INT32 CNetCommuBase::ClearAllChn()
{
	INT32 iMaxChnNum;
	CNetChannel* pChn;
	INT32 i;
	iMaxChnNum = GetMaxChnNum();

	if(NULL == m_pNetChannelArray)
	{
		return -1;
	}
	for(i = 0; i < iMaxChnNum; i++)
	{
		pChn = m_pNetChannelArray[i];
		if(NULL == pChn)
		{
			continue;
		}
		pChn->CloseChannelEx();
	}
	return 0;
}

INT32 CNetCommuBase::ThreadPopMsg(void* pParam)
{
	CNetCommuBase* pBase;

	pBase = (CNetCommuBase*)pParam;
	if(NULL == pBase)
	{
		return -1;
	}
	pBase->PopMsg();
	return 0;
}

INT32 CNetCommuBase::PopMsg()
{
#if 0
	CBufferObj* pObj;
	fnNetEvent OnNetEventCB;

	pObj = m_BufStack.PopData();
	if(NULL == pObj)
	{
		return -1;
	}
	OnNetEventCB = GetNetEventEntry();
	if(NULL != OnNetEventCB)
	{
		OnNetEventCB(GetNetEventCaller(), 
			(INetChannel*)pObj->GetUserData(),
			NET_EVENT_READ, 
			pObj->GetData(), 
			pObj->GetDataSize());
	}
	pObj->SetBufValid(BUFFER_DATA_INVALID);
#endif 
	return 0;
}


void CNetCommuBase::ThreadDemonNetChn(CGSThread *gsTheadHandle, void *pParam)
{
	CNetCommuBase* pBase;

	pBase = (CNetCommuBase*)pParam;
	if(NULL == pBase || NULL == gsTheadHandle)
	{
		return;
	}

	while(!gsTheadHandle->TestExit())
	{
		pBase->NetChnDemon();
		MSLEEP(1000);		
	}
}

INT32 CNetCommuBase::NetChnDemon()
{
	INT32 i;
	CNetChannel* pChn;

	for(i = 0; i < GetMaxChnNum(); i++)
	{
		pChn = m_pNetChannelArray[i];
		if(NULL == pChn)
		{
			continue;
		}
		if(NET_CHANNEL_STATUS_INVALID == pChn->GetChnStatus())
		{
			continue;
		}
		pChn->OnCheckStatus();
	}
	return 0;
}

INT32 CNetCommuBase::ChannelEventCB(void* pCaller, INetChannel* pChn, INT32 iEvent, INetMessage* pNetMessage)
{
	CNetCommuBase* pNetCommuBase;
	INT32 iRet;

	pNetCommuBase = (CNetCommuBase*)pCaller;
	if(NULL == pNetCommuBase)
	{
		return -1;
	}
	iRet = pNetCommuBase->OnChannelEvent((CNetChannel*)pChn, iEvent, pNetMessage);
	return iRet;
}
INT32 CNetCommuBase::OnChannelEvent(CNetChannel* pChn, INT32 iEvent, INetMessage* pNetMessage)
{
	fnNetEvent OnNetEventCB;

	OnNetEventCB = GetNetEventEntry();

	if(NULL == pChn)
	{
		return -1;
	}
	switch(iEvent)
	{
	case NET_EVENT_CONNECT:
		if(OnNetEventCB)
		{
			OnNetEventCB(GetNetEventCaller(), pChn, iEvent, pNetMessage);
		}
		break;
	case NET_EVENT_READ:
		if(NULL == pNetMessage || pNetMessage->GetMSGLen() < 0)
		{
            LOG2_DEBUG("ChnType(%d) %s:%d=>%s:%d Disconnect of read invalid data.\n", 
                pChn->GetChnType(),
                pChn->GetLocalIPAddrString(),pChn->GetLocalPort(),
                pChn->GetRemoteIPAddrString(),pChn->GetRemotePort() );
			if(NET_CHANNEL_TYPE_CLIENT == pChn->GetChnType())
			{
                pChn->SetSocketChn( NULL, NET_CHANNEL_STATUS_NO_CONNECT); 		
			}
			if(NULL != OnNetEventCB)
			{
				OnNetEventCB(GetNetEventCaller(), (INetChannel*)pChn, NET_EVENT_CLOSE, NULL);
			}
			break;
		}

		if(SUPPORT_MESSAGE_BUF == GetSupportMsgBuf())
		{
			if(OnNetEventCB)
			{
				OnNetEventCB(GetNetEventCaller(), pChn, iEvent, pNetMessage);
			}
		}
		else
		{
			if(OnNetEventCB)
			{
				OnNetEventCB(GetNetEventCaller(), pChn, iEvent, pNetMessage);
			}
		}
		break;
	case NET_EVENT_CLOSE:
		if(OnNetEventCB)
		{
			OnNetEventCB(GetNetEventCaller(), pChn, iEvent, NULL);
		}
		break;
	}
	return 0;
}

/***************************************************************************
  Function:        	StartNetBase
  DATE:				2010-8-3   15:38
  Description: 		   
  Input:  			           
  Output:     		    
  Return:   		      
  Note:						
****************************************************************************/
INT32 CNetCommuBase::StartNetBase()
{
	INT32 iMaxChnNum;
	INT32 iRet;

	iMaxChnNum = GetMaxChnNum();

	m_pNetChannelArray = new CNetChannel* [iMaxChnNum];
	if(NULL == m_pNetChannelArray)
	{
		return -1;
	}
	memset(m_pNetChannelArray, 0, sizeof(CNetChannel*) * iMaxChnNum);

	m_GSPulseThread.Start(ThreadDemonNetChn, this);


	iRet = m_BufferSlickArray.InitBufferArray(DEFAULT_BUFFER_SIZE);
	if(iRet < 0)
	{
		StopNetBase();
		return -2;
	}

	return 0;
}

INT32 CNetCommuBase::StopNetBase()
{
	INT32 i;
	CNetChannel* pChn;
	
	if (m_GSPulseThread.GetThreadHandle() != NULL)
	{
		m_GSPulseThread.Join(10000);
		MSLEEP(100);
		if (m_GSPulseThread.GetThreadHandle() != NULL)
		{
			//m_GSPulseThread.Cancel();
		}
	}
	if(NULL != m_pNetChannelArray)
	{
		for(i = 0; i < GetMaxChnNum(); i++)
		{
			pChn = m_pNetChannelArray[i];
			if(NULL != pChn)
			{
				pChn->CloseChannelEx();
				pChn->ReleaseNetChannel();
				delete pChn;
				m_pNetChannelArray[i] = NULL;
			}
		}
		delete m_pNetChannelArray;
		m_pNetChannelArray = NULL;
	}

	m_BufferSlickArray.DestroyArray();
#if 0
	if(m_pThreadScanMsg)
	{
		m_pThreadScanMsg->StopThreadRoutine();
		delete m_pThreadScanMsg;
		m_pThreadScanMsg = NULL;
	}
	
	m_BufStack.DestroyStack();
#endif 	
	return 0;
}