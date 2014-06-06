#include "CommunicationManager.h"

using namespace NetServiceLib;

CCommunicationManager::CCommunicationManager(void)
{
	m_bIsExitLinsten = true;
	m_bIsExitActiveTest = true;
	m_bIsExitAcceptData = true;
	m_bIsExitActiveTest = true;
	m_bIsExitChannelStatus = true;
	m_bIsExitAcceptUpNotice = true;
	m_bIsSleep = false;
#if OPERATING_SYSTEM
	m_bIsExitSelectEvent = true;
#endif
#if _LINUX
	m_bIsExitEpollEventWait = true;
#endif
}

CCommunicationManager::~CCommunicationManager(void)
{

}
/********************************************************************************
  Function:       Init
  Description:    初始化
  Input:  
  Output:         
  Return:         
  Note:				
********************************************************************************/
INT	CCommunicationManager::Init()
{
	CNetInterfaceCommData::Init();

	return ERROR_BASE_SUCCESS;
}

/********************************************************************************
  Function:       ChannelActiveTest
  Description:    通道活动检测
  Input:  
  Output:         
  Return:         
  Note:				
********************************************************************************/
INT	CCommunicationManager::ChannelActiveTest(enumThreadEventType enumEvent, void* pObject)
{
	if (enumEvent != ACTIVETEST)
	{
		return ERROR_NET_PARAM_WRONG;
	}
	m_GSMutexExit.Lock();
	if ( !m_bIsExitActiveTest )
	{
		m_GSMutexExit.Unlock();
		return ERROR_NET_PARAM_WRONG;
	}
	m_GSMutexExit.Unlock();
	
	while (m_bIsExitActiveTest)//什么时候退出该循环
	{
		if (m_unActiveTime > 0)	// 根据秦海和邹工的意见修改 默认不进行通道超时检查 除非用户设置了该值
		{
			WAITFOREVENT_SLEEP( ACTIVE_TEST_TIME );

			TestActiveTime();
		}
		else
		{
			
			MSLEEP(10);
		}
		
	}

	return ERROR_BASE_SUCCESS;
}

/********************************************************************************
  Function:       WaitForEvent
  Description:    线程等待时间 可能是休眠 也可能是退出
  Input:  
  Output:         
  Return:         
  Note:				bool IsExitFlag 这样传值达不到目的
********************************************************************************/
INT	CCommunicationManager::WaitForEvent( DWORD dwSleep, DWORD dwSpace )
{
	if (dwSleep <1)
	{
		dwSleep=1;
	}
	if (dwSpace<1)
	{
		dwSpace=1;
	}

	for (DWORD i=0; i<dwSleep; i+=dwSpace)
	{		
		// 延时:防止暂停大量的占用CPU时间

		if ( m_bIsExitActiveTest == false )
		{
			return -1;
		}

		MSLEEP(dwSpace);		
	}

	return ERROR_BASE_SUCCESS;
}


//释放所有附加通道
INT	CCommunicationManager::FreeAllExtraChannel(CSocketChannel* pclsSocketChannel)
{
	if ( NULL == pclsSocketChannel )
	{
		return ERROR_NET_PARAM_WRONG;
	}
	
	m_GSMutexVerChnl.Lock();

	VectorChannelPoint::size_type i=0;
	bool bExitFlag = true;
	while ( bExitFlag )
	{
		bExitFlag = false;	//找不到匹配的就退出

		//要验证这里删除对不对
		for ( i=0; i< m_vectorSocketChannel.size(); i++)
		{
			if ( m_vectorSocketChannel[i]->GetListenChannel() == pclsSocketChannel)
			{
				vector<CSocketChannel*>::iterator pIter;
				pIter = find(m_vectorSocketChannel.begin(), m_vectorSocketChannel.end(), m_vectorSocketChannel[i]);
				delete m_vectorSocketChannel[i];
				m_vectorSocketChannel.erase(pIter);
				bExitFlag = true;	//有匹配的 则继续while循环
				break;
			}
			
		}

	}
	
	m_GSMutexVerChnl.Unlock();

	return ERROR_BASE_SUCCESS;
}

//释放全部通道
INT	CCommunicationManager::FreeAllChannel()
{
	m_GSMutexVerChnl.Lock();

	VectorChannelPoint::size_type i=0;
	for ( i=0; i< m_vectorSocketChannel.size(); i++)
	{
		m_vectorSocketChannel[i]->CloseChannel();

	}

	m_GSMutexVerChnl.Unlock();

	m_GSMutexVerFaultChnl.Lock();

	for ( i=0; i< m_vectorFaultChannel.size(); i++)
	{
		m_vectorFaultChannel[i]->CloseChannel();

	}

	m_GSMutexVerFaultChnl.Unlock();

	return ERROR_BASE_SUCCESS;
}

//重新连接
INT	CCommunicationManager::ReConnectChannel( CSocketChannel* pclsSocketChannel )
{
	if (m_bReConnect)
	{
		char	szIP[256] = {0};
		UINT16	unPort = 0;
		pclsSocketChannel->GetReMoteIPPort( szIP, unPort );
		CBaseSocket* pBaseSocket = pclsSocketChannel->GetCbaseSocketPoint();
		pBaseSocket->CloseSocket();
		char szLocalIP[256] = {0};
		UINT16 unLocalPort = 0;
		pclsSocketChannel->GetLocalIPPort( szLocalIP, unLocalPort );
		pBaseSocket->CreateSocket( szLocalIP, unLocalPort );
		return pBaseSocket->Connect( szIP, unPort );
	}

	return ERROR_NET_PARAM_WRONG;
}


// 检查正常通道队列
void CCommunicationManager::CheckNormalChannelDeque()
{
	m_GSMutexVerChnl.Lock();
	for (VectorChannelPoint::size_type i=0; i<m_vectorSocketChannel.size(); i++)
	{
		if (m_vectorSocketChannel[i]->GetChannelStatus() == CHANNEL_NORMAL)
		{
			continue;
		}

		else if ( m_vectorSocketChannel[i]->GetChannelStatus() == CHANNEL_CLOSE)
		{
			//关闭

			CSocketChannel* pclsSocketChannel = m_vectorSocketChannel[i];

			//m_GSMutexVerChnl.Unlock();

			SaveToFaultVector( pclsSocketChannel );
			//DeleteSocketChannel(pclsSocketChannel);
			vector<CSocketChannel*>::iterator pIter;
			pIter = find(m_vectorSocketChannel.begin(), m_vectorSocketChannel.end(), pclsSocketChannel);

			if (pIter != m_vectorSocketChannel.end())
			{
				m_vectorSocketChannel.erase(pIter);
			}
			else
			{
				assert(0);
			}


			//m_GSMutexVerChnl.Lock();

			i=-1;	//重新开始检查
			continue;
			//break;

		}
		else if ( m_vectorSocketChannel[i]->GetChannelStatus() == CHANNEL_DISCNN)
		{
			CSocketChannel* pclsSocketChannel = m_vectorSocketChannel[i];

			//m_GSMutexVerChnl.Unlock();

			pclsSocketChannel->m_csCallBackGSMutex.Lock();
			if ( m_vectorSocketChannel[i]->GetChannelStatus() == CHANNEL_DISCNN)
			{
				pclsSocketChannel->AddRefCount();
				pclsSocketChannel->m_uiChnIng = 2;

				char szRemoteIP[16]={0};
				UINT16 uiRemotePort = 0;
				char szLocalPort[16]={0};
				UINT16 uiLocalPort = 0;

				memset(szRemoteIP,0,16);
				uiRemotePort = 0;
				pclsSocketChannel->GetReMoteIPPort(szRemoteIP,uiRemotePort);
				memset(szLocalPort,0,16);
				uiLocalPort = 0;
				pclsSocketChannel->GetLocalIPPort(szLocalPort,uiLocalPort);

				CTRLPRINTF(m_clsLogPtr, "通知上层通道%p中断, szRemoteIP:%s, uiRemotePort:%u, uiLocalPort:%u \n ",\
					pclsSocketChannel,szRemoteIP,uiRemotePort,uiLocalPort);

				//通知上层，远端主动断开。如果本方是服务器端监听产生的连接通道，上层应停止发送数据，并CloseChannel。
				OnEventCallBack( pclsSocketChannel, NET_REMOTE_DISCNN, NULL, 0);
				CTRLPRINTF(m_clsLogPtr, "通知上层通道%p中断,通知完毕, szRemoteIP:%s, uiRemotePort:%u, uiLocalPort:%u \n ",\
					pclsSocketChannel,szRemoteIP,uiRemotePort,uiLocalPort);


				pclsSocketChannel->m_uiChnIng = 0;
				pclsSocketChannel->SubRefCount();

			}
			pclsSocketChannel->m_csCallBackGSMutex.Unlock();			


			//如果本方是客户端，网络库将根据重连参数进行操作
			if ( CLIENT == pclsSocketChannel->GetServerType() && m_bReConnect )
			{
				ChannelReconnect(pclsSocketChannel);

			}

			//m_GSMutexVerChnl.Lock();

			i=-1;	//重新开始检查

			if ( pclsSocketChannel->GetChannelStatus() == CHANNEL_DISCNN)
			{
				//m_GSMutexVerChnl.Unlock();

				SaveToFaultVector( pclsSocketChannel );
				//DeleteSocketChannel(pclsSocketChannel);
				vector<CSocketChannel*>::iterator pIter;
				pIter = find(m_vectorSocketChannel.begin(), m_vectorSocketChannel.end(), pclsSocketChannel);

				if (pIter != m_vectorSocketChannel.end())
				{
					m_vectorSocketChannel.erase(pIter);
				}
				else
				{
					assert(0);
				}


				//m_GSMutexVerChnl.Lock();

				continue;

			}

		}
		else
		{
			//不应该出现此状态
		}

	}

	m_GSMutexVerChnl.Unlock();
}

// 检查故障通道队列
void CCommunicationManager::CheckFaultChannelDeque()
{
	m_GSMutexVerFaultChnl.Lock();
	for (VectorChannelPoint::size_type i=0; i<m_vectorFaultChannel.size(); i++)
	{
		if ( m_vectorFaultChannel[i]->GetChannelStatus() == CHANNEL_CLOSE )
		{

			if ( m_vectorFaultChannel[i]->GetCloseTick() == 0)
			{
				continue;
			}

			// 大于60秒就不需要考虑其它条件 直接执行下面的代码 释放通道
			UINT64 iTick = DoGetTickCount() - m_vectorFaultChannel[i]->GetCloseTick(); 

			if ( iTick < 30*1000 )	// 10秒
			{
				if (m_vectorFaultChannel[i]->GetRefCount() > 0 )
				{
					// 直到引用计数等于0才删除

					continue;
				}

				if (m_vectorFaultChannel[i]->GetIOCPNoUse())
				{
					// 要等到完成端口不使用了才删除
					continue;
				}

			}
			else
			{
				if ( m_vectorFaultChannel[i]->GetRefCount() != 0 || m_vectorFaultChannel[i]->m_uiChnIng != 0)
				{
					char szRemoteIP[16]={0};
					UINT16 uiRemotePort = 0;
					char szLocalPort[16]={0};
					UINT16 uiLocalPort = 0;

					memset(szRemoteIP,0,16);
					uiRemotePort = 0;
					m_vectorFaultChannel[i]->GetReMoteIPPort(szRemoteIP,uiRemotePort);
					memset(szLocalPort,0,16);
					uiLocalPort = 0;
					m_vectorFaultChannel[i]->GetLocalIPPort(szLocalPort,uiLocalPort);
					CTRLPRINTF(m_clsLogPtr, "通道%p的引用计数或回调标志未能归零 m_uiRefCount:%u, m_bIOCPNoUse:%d,m_uiChnIng:%u, szRemoteIP:%s, uiRemotePort:%u, uiLocalPort:%u \n",m_vectorFaultChannel[i],
						m_vectorFaultChannel[i]->GetRefCount(), m_vectorFaultChannel[i]->GetIOCPNoUse(),m_vectorFaultChannel[i]->m_uiChnIng, szRemoteIP, uiRemotePort, uiLocalPort );
					assert(0);
				}

			}

			CSocketChannel* pclsSocketChannel = m_vectorFaultChannel[i];

			//m_GSMutexVerFaultChnl.Unlock();

			//DeleteoFromFaultVector( pclsSocketChannel );

			VectorChannelPoint::iterator pIter;
			pIter = find(m_vectorFaultChannel.begin(), m_vectorFaultChannel.end(), pclsSocketChannel);
			if (pIter != m_vectorFaultChannel.end())
			{
				m_vectorFaultChannel.erase(pIter);

			}
			else
			{
				assert(0);
			}

			CTRLPRINTF(m_clsLogPtr, "释放通道%p内存\n ",pclsSocketChannel);

			delete pclsSocketChannel;

			//m_GSMutexVerFaultChnl.Lock();

			i=-1;	//重新开始检查
			continue;
			//break;
		}
		else if ( m_vectorFaultChannel[i]->GetChannelStatus() == CHANNEL_ADD_IOCPOREPOLL)
		{
			//加入epoll or iocp
			CSocketChannel* pclsSocketChannel = m_vectorFaultChannel[i];
			pclsSocketChannel->SetChannelStatus( CHANNEL_NORMAL );				

			//m_GSMutexVerFaultChnl.Unlock();

			//DeleteoFromFaultVector( pclsSocketChannel );
			VectorChannelPoint::iterator pIter;
			pIter = find(m_vectorFaultChannel.begin(), m_vectorFaultChannel.end(), pclsSocketChannel);

			if (pIter != m_vectorFaultChannel.end())
			{
				m_vectorFaultChannel.erase(pIter);

			}
			else
			{
				assert(0);
			}

			SaveSocketChannel( pclsSocketChannel );

			OnEventModel( pclsSocketChannel );


			//m_GSMutexVerFaultChnl.Lock();

			i=-1;	//重新开始检查
			continue;
			//break;

		}			
		else
		{
			//其它状态不处理
		}

	}

	m_GSMutexVerFaultChnl.Unlock();
}

//通道重连
void CCommunicationManager::ChannelReconnect(CSocketChannel*	pclsSocketChannel)
{
	//重连
	if( ERROR_BASE_SUCCESS != ReConnectChannel( pclsSocketChannel ))
	{
		//重连失败
		OnEventCallBack( pclsSocketChannel, NET_RECONNECT_FAIL, NULL, 0 );

	}
	else
	{

		//重连成功
		pclsSocketChannel->SetChannelStatus( CHANNEL_NORMAL );

		OnEventCallBack( pclsSocketChannel, NET_RECONNECT_SUCCESS, NULL, 0 );

		OnEventModel( pclsSocketChannel );

	}
}

//断开处理
/********************************************************************************
Function:		DealDisnChannel
Description:	断开处理
Input:  		pclsSocketChannel
Output:      	   
Return:  		       
Note:					
Author:        	CHC
Date:				2010/08/11
********************************************************************************/
void CCommunicationManager::DealDisnChannel( CSocketChannel*	pclsSocketChannel)
{
	if (pclsSocketChannel == NULL || pclsSocketChannel->GetChannelStatus() == CHANNEL_CLOSE)
	{
		return;
	}

	pclsSocketChannel->SetChannelStatus( CHANNEL_DISCNN );

}

/********************************************************************************
Function:		ThreadChannelStatus
Description:	处理通道的状态、如中断、关闭、等等
Input:  		
Output:      	   
Return:  		       
Note:					
********************************************************************************/
INT	CCommunicationManager::ThreadChannelStatus(enumThreadEventType enumEvent, void* pObject)

{
	if (enumEvent != CHANNELSTATUS )
	{

		return ERROR_NET_PARAM_WRONG;
	}

	UINT64 uiTick = DoGetTickCount();

	m_GSMutexExit.Lock();
	if ( !m_bIsExitChannelStatus )
	{
		m_GSMutexExit.Unlock();
		return ERROR_NET_PARAM_WRONG;
	}
	m_GSMutexExit.Unlock();
	while ( m_bIsExitChannelStatus )
	{
		int iRet = m_GSCond.WaitTimeout(10);
		if (iRet == 0)
		{
			break;
		}

		// 检查正常通道状态队列
		CheckNormalChannelDeque();

		CheckFaultChannelDeque();

		// 输出日志

		if ( DoGetTickCount() - uiTick > 20 *1000 )
		{
			// 每30秒钟输出1次网络库状态			
#ifdef _DEBUG

			CTRLPRINTF(m_clsLogPtr,"------------当前网络库状态参数如下------------\n");
			m_GSMutexVerChnl.Lock();
			m_GSMutexVerFaultChnl.Lock();
			INT16 iNum = m_vectorSocketChannel.size() + m_vectorFaultChannel.size();
			CTRLPRINTF(m_clsLogPtr,"当前网络库中通道数目是 %d \n", iNum);
			char szRemoteIP[16]={0};
			UINT16 uiRemotePort = 0;
			char szLocalPort[16]={0};
			UINT16 uiLocalPort = 0;
			for (VectorChannelPoint::size_type i=0; i<m_vectorSocketChannel.size(); i++)
			{
				memset(szRemoteIP,0,16);
				uiRemotePort = 0;
				m_vectorSocketChannel[i]->GetReMoteIPPort(szRemoteIP,uiRemotePort);
				memset(szLocalPort,0,16);
				uiLocalPort = 0;
				m_vectorSocketChannel[i]->GetLocalIPPort(szLocalPort,uiLocalPort);
				CTRLPRINTF( m_clsLogPtr,"通道参数N szRemoteIP:%s, uiRemotePort:%u, uiLocalPort:%u, ChannelStatus：%d,m_uiRefCount:%u, m_bIOCPNoUse:%d,m_uiChnIng:%u \n",\
					szRemoteIP, uiRemotePort,uiLocalPort,m_vectorSocketChannel[i]->GetChannelStatus(),m_vectorSocketChannel[i]->GetRefCount(), m_vectorSocketChannel[i]->GetIOCPNoUse(),m_vectorSocketChannel[i]->m_uiChnIng );
			}

			for (VectorChannelPoint::size_type i=0; i<m_vectorFaultChannel.size(); i++)
			{
				memset(szRemoteIP,0,16);
				uiRemotePort = 0;
				m_vectorFaultChannel[i]->GetReMoteIPPort(szRemoteIP,uiRemotePort);
				memset(szLocalPort,0,16);
				uiLocalPort = 0;
				m_vectorFaultChannel[i]->GetLocalIPPort(szLocalPort,uiLocalPort);
				CTRLPRINTF( m_clsLogPtr,"通道参数F szRemoteIP:%s, uiRemotePort:%u, uiLocalPort:%u, ChannelStatus：%d,m_uiRefCount:%u, m_bIOCPNoUse:%d,m_uiChnIng:%u \n",\
					szRemoteIP, uiRemotePort,uiLocalPort,m_vectorFaultChannel[i]->GetChannelStatus(),m_vectorFaultChannel[i]->GetRefCount(), m_vectorFaultChannel[i]->GetIOCPNoUse(),m_vectorFaultChannel[i]->m_uiChnIng );
			}

			

			m_GSMutexVerFaultChnl.Unlock();
			m_GSMutexVerChnl.Unlock();
#endif
			
			uiTick = DoGetTickCount();

		}	




	}

	return ERROR_BASE_SUCCESS;


}
// UDP,根据父通道创建新通道，并设置相关属性 失败返回NULL
CSocketChannel*	CCommunicationManager::CreateUdpChannel( CSocketChannel* pParentSocketChannel, LPPER_IO_OPERATION_DATA PerIoData )
{
	if ( NULL == pParentSocketChannel )
	{
		return NULL;
	}

	//创建新通道
	CSocketChannel* pclsNewSocketChannel = NULL;
	pclsNewSocketChannel = new CSocketChannel();
	if ( pclsNewSocketChannel )
	{
		pclsNewSocketChannel->SetCbaseSocketPoint(pParentSocketChannel->GetCbaseSocketPoint());
		pclsNewSocketChannel->SetListenSocketChannel(pParentSocketChannel);
		pclsNewSocketChannel->SetChannelType( COMM_CHANNEL );
		pclsNewSocketChannel->SetNetProtocolType( NET_PROTOCOL_UDP );
		pclsNewSocketChannel->SetServerType( SERVER );		

		char szIP[16] = { 0 };		
		UINT16 unPort = 0;
		pParentSocketChannel->GetLocalIPPort( szIP, unPort );
		pclsNewSocketChannel->SetLocalIPPort( szIP, unPort );

		memset( szIP, 0x0 , 16 );
		strcpy(szIP, inet_ntoa(PerIoData->struSockAddrFrom.sin_addr));
		pclsNewSocketChannel->SetReMoteIPPort( szIP, unPort );
		pclsNewSocketChannel->SetDWORDRemoteIP( PerIoData->struSockAddrFrom.sin_addr.s_addr );

		pclsNewSocketChannel->SetLogInstancePtr(m_clsLogPtr);	

		//收到数据，更新通道活动时间
		pclsNewSocketChannel->SetLastActiveTime();

		
	}
	else
	{
		// 申请内存失败
		assert(0);
		return NULL;
	}

	return pclsNewSocketChannel;
}
/********************************************************************************
Function:		Listen
Description:	监听。
Input:  		enumEvent 传入的参数必须是LISTEN；pObject 必须指向CSocketChannel对象,表明是哪个通道监听
Output:      	   
Return:  		正确为ERROR_BASE_SUCCESS，其它错误码见定义       
Note:			
********************************************************************************/
INT  CCommunicationManager::Listen(enumThreadEventType enumEvent, void* pObject)
{
	if (enumEvent != LISTEN || pObject == NULL)
	{
		//1 写到日志 发生错误了 可能是传错参数
		//2 返回
		return ERROR_NET_PARAM_WRONG;
	}

	CSocketChannel* pclsSocketChannel = (CSocketChannel*)pObject;

	CBaseSocket* pclsCBaseSocket = pclsSocketChannel->GetCbaseSocketPoint();
	if ( NULL == pclsCBaseSocket)
	{
		return ERROR_NET_UNKNOWN;
	}
	INT iRet = pclsCBaseSocket->Listen();

#ifdef _WIN32
	if (iRet == SOCKET_ERROR)
	{
		INT iErr = GetLastError();
		CTRLPRINTF(m_clsLogPtr,"listen fail %d \n", iErr);
	}
#endif

	sockaddr_in ClientAddr;
	INT nLen = sizeof(ClientAddr);	
	memset(&ClientAddr, 0x0, nLen);
	SOCKETHANDLE ClientAccept;

	m_GSMutexExit.Lock();
	if ( !m_bIsExitLinsten )
	{
		m_GSMutexExit.Unlock();
		return ERROR_NET_PARAM_WRONG;
	}
	m_GSMutexExit.Unlock();

	pclsSocketChannel->AddRefCount();
	while (m_bIsExitLinsten == true)//要考虑这个循环什么时候退出
	{		
		ClientAccept = pclsCBaseSocket->AcceptConnect((sockaddr*)&ClientAddr, &nLen);

		if ( ClientAccept <= 0 )
		{
			if( pclsSocketChannel->GetChannelStatus() == CHANNEL_CLOSE )
			{	
				CTRLPRINTF(m_clsLogPtr, "网络库:关闭监听通道 %p\n", pclsSocketChannel);
				break;	
			}
#ifdef _LINUX
			MSLEEP(1);
#endif
			continue;//未关闭 继续接收监听
		}

		StruSocketInfoPtr pStruSocketInfo = new STRUSOCKETINFO();
		if (pStruSocketInfo == NULL)
		{
			CTRLPRINTF(m_clsLogPtr, "网络库:分配内存失败\n");	
#ifdef _WIN32
			closesocket( ClientAccept );//这样写破坏了封装性
#endif
#ifdef _LINUX
			close( ClientAccept );//这样写破坏了封装性
#endif
			memset(&ClientAddr, 0x0, nLen);
			ClientAccept = NULL;
			continue;
		}

		pStruSocketInfo->iSocket = ClientAccept;
		pStruSocketInfo->pListenChannel = pclsSocketChannel;
		pStruSocketInfo->ClientAddr = ClientAddr;
		pStruSocketInfo->nLen = nLen;

		m_GSMutexSocketInfoList.Lock();
		pclsSocketChannel->AddRefCount();
		m_SocketInfoList.push(pStruSocketInfo);
		m_GSMutexSocketInfoList.Unlock();
		if (m_bIsSleep)
		{
			m_GSCondAcceptUpNotice.Signal();
		}

	

	}

	pclsSocketChannel->SubRefCount();

	return ERROR_BASE_SUCCESS;
}

// 通知上层新连接到达
INT	CCommunicationManager::NoticeUpNewConnect(enumThreadEventType enumEvent, void* pObject)
{
	if (enumEvent != ACCEPTNOTICE )
	{
		//1 写到日志 发生错误了 可能是传错参数
		//2 返回
		return ERROR_NET_PARAM_WRONG;
	}


	m_GSMutexExit.Lock();
	if ( !m_bIsExitAcceptUpNotice )
	{
		m_GSMutexExit.Unlock();
		return ERROR_NET_PARAM_WRONG;
	}
	m_GSMutexExit.Unlock();

	SocketInfoList	listSocketInfo;
	SOCKETHANDLE ClientAccept;
	sockaddr_in ClientAddr;
	INT nLen = sizeof(ClientAddr);	
	memset(&ClientAddr, 0x0, nLen);

	CTRLPRINTF(m_clsLogPtr, "通知上层新连接到达线程开始运行\n");

	while (m_bIsExitAcceptUpNotice)//要考虑这个循环什么时候退出
	{

		// 这是清空上一次数据
		while(!listSocketInfo.empty())
		{
			listSocketInfo.pop();
		}

		// 这是取数据到局部变量
		m_GSMutexSocketInfoList.Lock();
		while(!m_SocketInfoList.empty())
		{
			StruSocketInfoPtr	pData = m_SocketInfoList.front();
			listSocketInfo.push(pData);
			m_SocketInfoList.pop();
		}
		m_GSMutexSocketInfoList.Unlock();

		if (listSocketInfo.empty())
		{
			m_bIsSleep = true;
			int iRet = m_GSCondAcceptUpNotice.WaitTimeout(100);
			m_bIsSleep = false;
			if (iRet == 0 && !m_bIsExitAcceptUpNotice )
			{
				break;
			}
			else
			{
				continue;
			}
		}

		

		// 通知上层
		while(!listSocketInfo.empty())
		{
			if (!m_bIsExitAcceptUpNotice)
			{
				return ERROR_BASE_SUCCESS;
			}
			StruSocketInfoPtr	pData = listSocketInfo.front();
			listSocketInfo.pop();

			ClientAccept = pData->iSocket;//sokcet
			CSocketChannel* pclsSocketChannel = pData->pListenChannel;//监听通道
			ClientAddr = pData->ClientAddr;
			nLen = pData->nLen;

			if ( ClientAccept <= 0 )
			{
				if( pclsSocketChannel->GetChannelStatus() == CHANNEL_CLOSE )
				{	
					CTRLPRINTF(m_clsLogPtr, "网络库:关闭监听通道\n");
				}
#ifdef _LINUX
				MSLEEP(1);
#endif
				CTRLPRINTF(m_clsLogPtr, "网络库:错误的socket %d\n",ClientAccept);
				pclsSocketChannel->SubRefCount();
				delete pData;
				continue;//错误的socket 继续处理下一个
			}



			//需要判断是不是已达到最大连接数，达到就关闭刚连接进来的通道
			if (IfMaxChannelCount())
			{
				//已经达到最大连接数
#ifdef _WIN32
				closesocket( ClientAccept );//这样写破坏了封装性
#endif
#ifdef _LINUX
				close( ClientAccept );//这样写破坏了封装性
#endif
				memset(&ClientAddr, 0x0, nLen);
				ClientAccept = NULL;
				CTRLPRINTF(m_clsLogPtr, "网络库:达到最大通道数\n");
				pclsSocketChannel->SubRefCount();
				delete pData;
				continue;
			}	


			//创建新通道
			CSocketChannel* pclsNewSocketChannel = new CSocketChannel();
			//设置远程IP端口
			CBaseSocket* pclsNewCBaseSocket = NULL;
#ifdef _WIN32

			pclsNewCBaseSocket = new CWinTcpSocket();

#endif

#ifdef _LINUX
			pclsNewCBaseSocket = new CLinuxTcpSocket();

#endif

			if ( NULL == pclsNewSocketChannel || NULL == pclsNewCBaseSocket)
			{
				//请求内存失败
				CTRLPRINTF(m_clsLogPtr,"CThreadDealNetEvent::Listen 请求内存失败 \n");
#ifdef _WIN32
				closesocket( ClientAccept );//这样写破坏了封装性
#endif
#ifdef _LINUX
				close( ClientAccept );//这样写破坏了封装性
#endif
				memset(&ClientAddr, 0x0, nLen);
				ClientAccept = NULL;
				CTRLPRINTF(m_clsLogPtr, "网络库: 请求内存失败\n");
				pclsSocketChannel->SubRefCount();
				delete pData;
				continue;
			}

			//设置sokcet 、CBaseSocket类、监听通道
			pclsNewCBaseSocket->SetSocket(ClientAccept);
			pclsNewCBaseSocket->SetBlockMode( m_bBlockMode );
			pclsNewCBaseSocket->SetSockOption();
			pclsNewSocketChannel->SetCbaseSocketPoint(pclsNewCBaseSocket);
			pclsNewSocketChannel->SetListenSocketChannel(pclsSocketChannel);
			pclsNewSocketChannel->SetChannelType( COMM_CHANNEL );
			pclsNewSocketChannel->SetNetProtocolType( NET_PROTOCOL_TCP );
			pclsNewSocketChannel->SetServerType( SERVER );
			pclsNewSocketChannel->SetDWORDRemoteIP( ClientAddr.sin_addr.s_addr );

			//设置通道的本地IP和远端IP
			pclsNewSocketChannel->SetReMoteIPPort(inet_ntoa(ClientAddr.sin_addr), (UINT16) ntohs(ClientAddr.sin_port));
			sockaddr_in struLocalAddr;
#ifdef _WIN32
			INT32   iLocalAddrLen = sizeof(sockaddr_in);
#endif
#ifdef _LINUX
			socklen_t   iLocalAddrLen = sizeof(sockaddr_in);
#endif

			getsockname( ClientAccept, (sockaddr*)&struLocalAddr, &iLocalAddrLen );
			pclsNewSocketChannel->SetLocalIPPort(inet_ntoa(struLocalAddr.sin_addr), (UINT16) ntohs(struLocalAddr.sin_port));

			pclsNewSocketChannel->SetLogInstancePtr(m_clsLogPtr);

			//6、回调通知用户
			pclsSocketChannel->m_csCallBackGSMutex.Lock();
			if ( pclsSocketChannel->GetChannelStatus() == CHANNEL_CLOSE  )
			{
				pclsNewSocketChannel->CloseChannelEx();
				delete pclsNewSocketChannel;
				pclsNewSocketChannel = NULL;
				pclsSocketChannel->m_csCallBackGSMutex.Unlock();
				CTRLPRINTF(m_clsLogPtr, "网络库: 通知上层之前被关闭\n");
				pclsSocketChannel->SubRefCount();
				delete pData;
				continue;
			}

			char szRemoteIP[16]={0};
			UINT16 uiRemotePort = 0;
			char szLocalPort[16]={0};
			UINT16 uiLocalPort = 0;

			memset(szRemoteIP,0,16);
			uiRemotePort = 0;
			pclsNewSocketChannel->GetReMoteIPPort(szRemoteIP,uiRemotePort);
			memset(szLocalPort,0,16);
			uiLocalPort = 0;
			pclsNewSocketChannel->GetLocalIPPort(szLocalPort,uiLocalPort);

			CTRLPRINTF(m_clsLogPtr, "通道%p创建成功, new socket %d, szRemoteIP:%s, uiRemotePort:%u, uiLocalPort:%u \n ",\
				pclsNewSocketChannel,ClientAccept,szRemoteIP,uiRemotePort,uiLocalPort);
			ISocketChannel *pSocketChnl = (ISocketChannel*)pclsNewSocketChannel;
			CTRLPRINTF(m_clsLogPtr, "回调通知上层连接建立,new socket %d\n ",ClientAccept);
			OnEventCallBack( pSocketChnl,NET_ACCEPT, NULL, 0 );
			CTRLPRINTF(m_clsLogPtr, "回调结束\n ");

			if (  NULL == pclsNewSocketChannel->GetCbaseSocketPoint()->GetSocket())
			{
				CTRLPRINTF(m_clsLogPtr, " 上层在NET_ACCEPT回调中关闭了 new socket %d \n ", ClientAccept );
				//pclsNewSocketChannel->CloseChannel();
				delete pclsNewSocketChannel;
				pclsNewSocketChannel = NULL;
				pclsSocketChannel->m_csCallBackGSMutex.Unlock();
				pclsSocketChannel->SubRefCount();
				delete pData;
				continue;
			}
			pclsSocketChannel->m_csCallBackGSMutex.Unlock();

			CTRLPRINTF(m_clsLogPtr, "通道%p NET_ACCEPT回调完成, new socket %d\n ",pclsNewSocketChannel,ClientAccept);

			memset(&ClientAddr, 0x0, nLen);
			ClientAccept = NULL;		

			pclsNewSocketChannel->m_csCallBackGSMutex.Lock();
			if ( pclsNewSocketChannel->GetChannelStatus() == CHANNEL_CLOSE )
			{
				CTRLPRINTF(m_clsLogPtr, "通道%p 被异常关闭 \n ",pclsNewSocketChannel);

				pclsNewSocketChannel->m_csCallBackGSMutex.Unlock();
				delete pclsNewSocketChannel;
				pclsNewSocketChannel = NULL;
				pclsSocketChannel->SubRefCount();
				delete pData;
				continue;

			}
			//3、保存通道
			SaveSocketChannel(pclsNewSocketChannel);  

			// 响应事件
			OnEventModel( pclsNewSocketChannel );

			pclsNewSocketChannel->m_csCallBackGSMutex.Unlock();

			pclsSocketChannel->SubRefCount();
			delete pData;

		}//end while
		
	}


	return ERROR_BASE_SUCCESS;
}