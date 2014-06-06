
#include "ThreadIOCP.h"

#if OPERATING_SYSTEM
#elif _WIN32

using namespace NetServiceLib;

CThreadDealNetEvent::CThreadDealNetEvent(void)
{
	
}

CThreadDealNetEvent::~CThreadDealNetEvent(void) 
{
	
}

/*************************************************
  Function:       ThreadAcceptData
  Description:    接收数据，并回调通知上层。
  Input:		  enumEvent 传入的参数必须是ACCEPTDATA；pObject没用到，可以为null.
  Output:         
  Return:         
  Note:			1、判断连接是否断开、2、判断是否接收动作RECV_POSTED 3、回调通知用户层 4、发起下一次接收	
*************************************************/
INT CThreadDealNetEvent::ThreadAcceptData(enumThreadEventType enumEvent, void* pObject)
{
	//pObject可以是null
	if (enumEvent != ACCEPTDATA)
	{
		return ERROR_NET_PARAM_WRONG;
	}

	// 重叠IO
	LPOVERLAPPED lpOverlapped = NULL;	
	// 传输字节数
	DWORD BytesTransferred = 0;
	// 套接字结构体
	LPPER_HANDLE_DATA PerHandleData = NULL;
	// IO结构体
	LPPER_IO_OPERATION_DATA PerIoData = NULL;  

	m_GSMutexExit.Lock();
	if ( !m_bIsExitAcceptData )
	{
		m_GSMutexExit.Unlock();
		return ERROR_NET_PARAM_WRONG;
	}
	m_GSMutexExit.Unlock();

	DWORD	dtTick = DoGetTickCount();
	char szRemoteIP[16]={0};
	UINT16 uiRemotePort = 0;
	char szLocalPort[16]={0};
	UINT16 uiLocalPort = 0;

	CTRLPRINTF( m_clsLogPtr," 完成端口开始接收数据 \n");

	while( m_bIsExitAcceptData )	//什么时候退出循环
	{

		//收数据
		PerHandleData = NULL;
		PerIoData = NULL;
		lpOverlapped = NULL;
		BytesTransferred = 0;

		BOOL bRet = GetQueuedCompletionStatusEx(&BytesTransferred, (LPDWORD)&PerHandleData, (LPOVERLAPPED*)&lpOverlapped, (LPPER_IO_OPERATION_DATA*)&PerIoData);
		

		if ( bRet > 0 && BytesTransferred > 0 )
		{
			if(PerIoData->OptionType == RECV_POSTED && NULL != PerHandleData )
			{

				//收到数据
				CSocketChannel*  pclsSocketChannel = (CSocketChannel*)PerHandleData->pUserData;

				if (LISTEN_CHANNEL == pclsSocketChannel->GetChannelType() && NET_PROTOCOL_UDP == pclsSocketChannel->GetNetProtocolType())
				{
					//也许这里会让你看晕
					//如果是UDP方式SERVER通道 判断是不是新的客户端发来的数据

					UINT16  unPort = 0;				
					unPort = (UINT16)ntohs( PerIoData->struSockAddrFrom.sin_port);
					//查找该通道是否已经存在
					CSocketChannel*  pclsNewSocketChannel = GetExistSocketChannel( PerIoData->struSockAddrFrom.sin_addr.s_addr, unPort );
					if ( NULL == pclsNewSocketChannel)
					{
						// 判断是否达到最大通道数
						if ( !IfMaxChannelCount())
						{
							//通道不存在 创建
							pclsNewSocketChannel = CreateUdpChannel(pclsSocketChannel, PerIoData);

							if ( pclsNewSocketChannel  )
							{		
								//3、回调通知用户层
								ISocketChannel *pSocketChnl = (ISocketChannel*)pclsNewSocketChannel;
								//邹工要求分2次回调，先通知上层有接入 再发数据
								OnEventCallBack( pSocketChnl, NET_ACCEPT, NULL, 0 );
								
								//保存到所有通道队列 
								SaveSocketChannel(pclsNewSocketChannel);  

								OnEventCallBack( pSocketChnl, NET_READ, PerIoData->Buffer, BytesTransferred );
							}

							
							
						}						

					}//end if ( NULL == pclsNewSocketChannel)
					else
					{
						//收到数据，更新通道活动时间

						if (m_unActiveTime > 0)
						{
							pclsNewSocketChannel->SetLastActiveTime();
							pclsNewSocketChannel->SetChannelStatus( CHANNEL_NORMAL );
						}
						
						//3、回调通知用户层
						//3、回调通知用户层
						if (pclsNewSocketChannel->GetChannelStatus() != CHANNEL_CLOSE)
						{
							pclsNewSocketChannel->AddRefCount();
							ISocketChannel *pSocketChnl = (ISocketChannel*)pclsNewSocketChannel;
							OnEventCallBack( pSocketChnl,NET_READ, PerIoData->Buffer, BytesTransferred );
							pclsNewSocketChannel->SubRefCount();
						}
						else
						{						
							CTRLPRINTF( m_clsLogPtr," 通道%p在收到数据的时被关闭 \n", pclsSocketChannel);
							pclsSocketChannel->SetIOCPNoUse( FALSE );
						}

					}

					//4、发起下一次接收	 UDP方式收还得是它收,不是新的通道去收
					pclsSocketChannel->RecvData();


				}
				else
				{
					//收到数据，更新通道活动时间
					if (m_unActiveTime > 0)
					{
						pclsSocketChannel->SetLastActiveTime();
						pclsSocketChannel->SetChannelStatus( CHANNEL_NORMAL );
					}
					
					//3、回调通知用户层
					if (pclsSocketChannel->GetChannelStatus() != CHANNEL_CLOSE)
					{
						pclsSocketChannel->m_csCallBackGSMutex.Lock();
						if ( pclsSocketChannel->GetChannelStatus() != CHANNEL_CLOSE )
						{
							pclsSocketChannel->AddRefCount();
							pclsSocketChannel->m_uiChnIng = 1;
							ISocketChannel *pSocketChnl = (ISocketChannel*)pclsSocketChannel;
							int i=0;
							//if (DoGetTickCount() - dtTick > 10*1000)
							{
								memset(szRemoteIP,0,16);
								uiRemotePort = 0;								
								pclsSocketChannel->GetReMoteIPPort(szRemoteIP,uiRemotePort);								
								memset(szLocalPort,0,16);
								uiLocalPort = 0;
								pclsSocketChannel->GetLocalIPPort(szLocalPort,uiLocalPort);
								CTRLPRINTF_D( m_clsLogPtr,"网络库:通道%p回调数据包,szRemoteIP:%s, uiRemotePort:%u, uiLocalPort:%u \n",pclsSocketChannel, szRemoteIP, uiRemotePort,uiLocalPort );
								dtTick = DoGetTickCount();
								i=1;

							}
							OnEventCallBack( pSocketChnl, NET_READ, PerIoData->Buffer, BytesTransferred );
							//if (i==1)
							{
								CTRLPRINTF_D( m_clsLogPtr,"网络库:通道%p回调数据包完毕,szRemoteIP:%s, uiRemotePort:%u, uiLocalPort:%u \n",pclsSocketChannel, szRemoteIP, uiRemotePort,uiLocalPort );
							}
							pclsSocketChannel->m_uiChnIng = 0;
							//4、发起下一次接收	
							int iRet = pclsSocketChannel->RecvData();
							if (  1 != iRet  )
							{
								memset(szRemoteIP,0,16);
								uiRemotePort = 0;
								pclsSocketChannel->GetReMoteIPPort(szRemoteIP,uiRemotePort);
								memset(szLocalPort,0,16);
								uiLocalPort = 0;
								pclsSocketChannel->GetLocalIPPort(szLocalPort,uiLocalPort);
								CTRLPRINTF( m_clsLogPtr,"接收数据失败,szRemoteIP:%s, uiRemotePort:%u, uiLocalPort:%u \n",szRemoteIP, uiRemotePort,uiLocalPort );

							}
							else 
							{
								memset(szRemoteIP,0,16);
								uiRemotePort = 0;
								pclsSocketChannel->GetReMoteIPPort(szRemoteIP,uiRemotePort);
								memset(szLocalPort,0,16);
								uiLocalPort = 0;
								pclsSocketChannel->GetLocalIPPort(szLocalPort,uiLocalPort);
								//CTRLPRINTF( m_clsLogPtr,"成功将接收事件投递到完成端口 szRemoteIP:%s, uiRemotePort:%u, uiLocalPort:%u \n",szRemoteIP, uiRemotePort,uiLocalPort );
							}

							pclsSocketChannel->SubRefCount();
						}
						else
						{
							memset(szRemoteIP,0,16);
							uiRemotePort = 0;
							pclsSocketChannel->GetReMoteIPPort(szRemoteIP,uiRemotePort);
							memset(szLocalPort,0,16);
							uiLocalPort = 0;
							pclsSocketChannel->GetLocalIPPort(szLocalPort,uiLocalPort);
							CTRLPRINTF( m_clsLogPtr," 通道在收到数据的时被关闭,szRemoteIP:%s, uiRemotePort:%u, uiLocalPort:%u \n",szRemoteIP, uiRemotePort,uiLocalPort );
							pclsSocketChannel->SetIOCPNoUse( FALSE );

						}
						pclsSocketChannel->m_csCallBackGSMutex.Unlock();
						
					}
					else
					{						
						
						memset(szRemoteIP,0,16);
						uiRemotePort = 0;
						pclsSocketChannel->GetReMoteIPPort(szRemoteIP,uiRemotePort);
						memset(szLocalPort,0,16);
						uiLocalPort = 0;
						pclsSocketChannel->GetLocalIPPort(szLocalPort,uiLocalPort);
						CTRLPRINTF( m_clsLogPtr," 通道在收到数据的时被关闭 ,szRemoteIP:%s, uiRemotePort:%u, uiLocalPort:%u \n",szRemoteIP, uiRemotePort,uiLocalPort );
						pclsSocketChannel->SetIOCPNoUse( FALSE );
					}
					
				}



			}//end if(PerIoData->OptionType == RECV_POSTED)
			else if (PerIoData->OptionType == SEND_POSTED)
			{
				//发送数据  程序没有投掷这个请求，所以不应该跑到这里来
				assert(0);
				continue;
			}
		}
		else if (FALSE == bRet && 0 == BytesTransferred)
		{
			//无论TCP客户端还是UDP服务器端，主动断开都进此.
			//对方直接点击关闭按钮也是进此
	
			if ( NULL == PerHandleData )
			{
				//完成端口收到PostQueuedCompletionStatus消息退出
				INT32 iRet = GetLastError();
				
				CTRLPRINTF( m_clsLogPtr,"完成端口收到退出消息 FALSE == bRet\n");


				return ERROR_BASE_SUCCESS;
			}

			INT32 iError = GetLastError();
			
			CSocketChannel*  pclsSocketChannel = (CSocketChannel*)PerHandleData->pUserData;

			if ( !IfExistSocketChannel( pclsSocketChannel) )
			{
				
				CTRLPRINTF( m_clsLogPtr," 通道%p在通道队列中不存在 FALSE == bRet \n", pclsSocketChannel);
				continue;
			}


			if (ERROR_NETNAME_DELETED == iError)
			{
				//处理中断
				memset(szRemoteIP,0,16);
				uiRemotePort = 0;
				pclsSocketChannel->GetReMoteIPPort(szRemoteIP,uiRemotePort);
				memset(szLocalPort,0,16);
				uiLocalPort = 0;
				pclsSocketChannel->GetLocalIPPort(szLocalPort,uiLocalPort);
				CTRLPRINTF( m_clsLogPtr," 通道%p中断，指定的网络名不再可用 ,error = %d  ,szRemoteIP:%s, uiRemotePort:%u, uiLocalPort:%u \n",pclsSocketChannel, ERROR_NETNAME_DELETED, szRemoteIP, uiRemotePort, uiLocalPort );
				DealDisnChannel(pclsSocketChannel);

				
			}
			
			// 完成端口不使用该通道
			pclsSocketChannel->SetIOCPNoUse( FALSE );

			CTRLPRINTF( m_clsLogPtr," 完成端口不使用该通道%p FALSE == bRet \n", pclsSocketChannel);

			continue;
		}//end if(FALSE == bRet)

		//UDP方式 如果对端主动断开，这边不会有任何反应。只能依赖于发送数据是否成功来判断

	    else if ( TRUE == bRet && 0 == BytesTransferred)
		{
			//TCP 只要是对端主动断开，进此。无论对方是服务器端还是客户端

			if ( NULL == PerHandleData )
			{
				//完成端口收到PostQueuedCompletionStatus消息退出
				CTRLPRINTF( m_clsLogPtr,"完成端口收到退出消息\n");
				return ERROR_BASE_SUCCESS;
			}

			CSocketChannel*  pclsSocketChannel = (CSocketChannel*)PerHandleData->pUserData;

			if ( !IfExistSocketChannel( pclsSocketChannel) )
			{
				CTRLPRINTF( m_clsLogPtr," 通道%p在通道队列中不存在\n", pclsSocketChannel);
				continue;
			}

			DealDisnChannel(pclsSocketChannel);

			// 完成端口不使用该通道
			pclsSocketChannel->SetIOCPNoUse( FALSE );

			CTRLPRINTF( m_clsLogPtr," 完成端口不使用该通道%p\n", pclsSocketChannel);
			
			continue;
		}//end if ( TRUE == bRet && 0 == BytesTransferred)

		else if  ( FALSE == bRet && BytesTransferred > 0 )
		{
			//通道关闭瞬间收到的数据,这是有可能的,

			INT32 iError = GetLastError();
			CTRLPRINTF( m_clsLogPtr," BytesTransferred=%u, iError = %d \n", BytesTransferred, iError);

			if ( NULL == PerHandleData )
			{
				//完成端口收到PostQueuedCompletionStatus消息退出
				CTRLPRINTF( m_clsLogPtr,"完成端口收到退出消息 && BytesTransferred > 0\n");
				return ERROR_BASE_SUCCESS;
			}

			CSocketChannel*  pclsSocketChannel = (CSocketChannel*)PerHandleData->pUserData;

			if ( !IfExistSocketChannel( pclsSocketChannel) )
			{
				CTRLPRINTF( m_clsLogPtr," 通道%p在通道队列中不存在 && BytesTransferred > 0\n", pclsSocketChannel);
				continue;
			}

			DealDisnChannel(pclsSocketChannel);

			// 完成端口不使用该通道
			pclsSocketChannel->SetIOCPNoUse( FALSE );

			CTRLPRINTF( m_clsLogPtr," 完成端口不使用该通道%p && BytesTransferred > 0\n", pclsSocketChannel);

			continue;
		}
		else
		{
			INT32 iError = GetLastError();
			CTRLPRINTF( m_clsLogPtr," BytesTransferred=%u, bRet=%d, iError = %d \n", BytesTransferred, bRet, iError);

			if ( NULL == PerHandleData )
			{
				//完成端口收到PostQueuedCompletionStatus消息退出
				CTRLPRINTF( m_clsLogPtr,"完成端口收到退出消息 \n");
				return ERROR_BASE_SUCCESS;
			}

			CSocketChannel*  pclsSocketChannel = (CSocketChannel*)PerHandleData->pUserData;

			if ( !IfExistSocketChannel( pclsSocketChannel) )
			{
				CTRLPRINTF( m_clsLogPtr," 通道%p在通道队列中不存在 else \n", pclsSocketChannel);
				continue;
			}

			DealDisnChannel(pclsSocketChannel);

			// 完成端口不使用该通道
			pclsSocketChannel->SetIOCPNoUse( FALSE );

			CTRLPRINTF( m_clsLogPtr," 完成端口不使用该通道%p \n", pclsSocketChannel);
		}

		

		

		
	}

	CTRLPRINTF( m_clsLogPtr, "完成端口收到退出消息\n");

	return ERROR_BASE_SUCCESS;
}

void CThreadDealNetEvent::OnEventModel( CSocketChannel* pclsSocketChannel )
{
	if ( NULL == pclsSocketChannel)
	{
		assert(0);
		return;
	}
	HANDLE hReturn = AddToIoCompletionPort((HANDLE)pclsSocketChannel->GetCbaseSocketPoint()->GetSocket(), (DWORD)pclsSocketChannel->GetIOCPHandleData());

	if (!hReturn)
	{
		char szRemoteIP[16]={0};
		UINT16 uiRemotePort = 0;

		pclsSocketChannel->GetReMoteIPPort(szRemoteIP,uiRemotePort);
		CTRLPRINTF( m_clsLogPtr,"网络库:通道%p加入完成端口失败,szRemoteIP:%s, uiRemotePort:%u \n",pclsSocketChannel, szRemoteIP, uiRemotePort );
		closesocket(pclsSocketChannel->GetCbaseSocketPoint()->GetSocket());
		DealDisnChannel(pclsSocketChannel);
	}

	//发起接收
	int iRet = pclsSocketChannel->RecvData();
	if (  1 != iRet  )
	{
		char szRemoteIP[16]={0};
		UINT16 uiRemotePort = 0;

		pclsSocketChannel->GetReMoteIPPort(szRemoteIP,uiRemotePort);
		CTRLPRINTF( m_clsLogPtr,"网络库:通道%p接收数据失败,szRemoteIP:%s, uiRemotePort:%u \n",pclsSocketChannel, szRemoteIP, uiRemotePort );
		DealDisnChannel(pclsSocketChannel);

	}
}

// 处理退出
INT	CThreadDealNetEvent::ExitAcceptData(INT32 iThreadCount)
{
	if (iThreadCount < 0 || iThreadCount > 1000)
	{
		return 0;
	}
#if OPERATING_SYSTEM
#elif _WIN32
	UINT64 uiTick = DoGetTickCount();
	//UINT16 iTatolIocpThreadCount = GetNumberOfProcessors() * 2 + 2;
	while( iThreadCount != m_clsThreadPool.m_ExitThreadCount)
	{
		PostQueuedCompletionStatusEx();//让完成端口线程退出
		if (DoGetTickCount() - uiTick > 20000)
		{
			CTRLPRINTF( m_clsLogPtr,"完成端口线程无法全部退出，仅退出%u条线程 \n",m_clsThreadPool.m_ExitThreadCount );
			break;
		}
		MSLEEP(20);
	}
	
#else
#endif

	return 0;
}

#endif //_win32

