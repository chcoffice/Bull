#include "ThreadSelect.h"

#if OPERATING_SYSTEM

using namespace NetServiceLib;

//线程执行函数 用于轮询linux的epoll事件
/********************************************************************************
Function:		SelectEvent
Description:	等待Select事件发生，发生的事件加入线程池任务列表，由线程池处理
Input:  		enumEvent 传入的参数必须是EPOLLEVENTWAIT；pObject 指向CCommunicationManager对象
Output:      	   
Return:  		正确为ERROR_BASE_SUCCESS，其它错误码见定义       
Note:					
********************************************************************************/
INT CThreadDealNetEvent::SelectEvent(enumThreadEventType enumEvent, void* pObject)
{
	timeval timeout; 
	int rt;
	fd_set fd;
	//fd_set efd;
	SOCKETHANDLE	sockMax;	// 最大的sockMax值

	int iFlag = 0;

	m_GSMutexExit.Lock();
	if ( !m_bIsExitSelectEvent )
	{
		m_GSMutexExit.Unlock();
		return ERROR_NET_PARAM_WRONG;
	}
	m_GSMutexExit.Unlock();
	while ( m_bIsExitSelectEvent )
	{
		// 1、复制socket到FD
		//2、select
		//3、判断是否超时、是否异常、有收到数据
		//4、超时继续循环
		//5、异常:如果是close socket 那么需要进行数据同步、其它异常？
		//6、有收到数据，从FD移出该SOCKET，加入线程池收数据
		//7、重复循环

		iFlag = 0;
		sockMax = -1;

		FD_ZERO(&fd);
		//FD_ZERO(&efd);
		timeout.tv_sec=1;	//1秒
		timeout.tv_usec=0;

		m_GSMutexVerChnl.Lock();

		// 1、复制socket到FD
		for (VectorChannelPoint::size_type i=0; i<m_vectorSocketChannel.size(); i++)
		{
			if ( m_vectorSocketChannel[i]->GetChannelStatus() != CHANNEL_NORMAL )
			{
				continue;
			}
			else if ( m_vectorSocketChannel[i]->GetChannelType() == LISTEN_CHANNEL
				&& m_vectorSocketChannel[i]->GetNetProtocolType() == NET_PROTOCOL_TCP )
			{
				// TCP监听通道不参与
				continue;
			}
			else if ( m_vectorSocketChannel[i]->GetIOCPNoUse())
			{
				// TRUE 正在读数据
				continue;
			}

			SOCKETHANDLE sockValue = m_vectorSocketChannel[i]->GetCbaseSocketPoint()->GetSocket();

			if ( sockValue <= 0 )
			{
				assert(0);
				continue;
			}

			sockMax = sockValue > sockMax ? sockValue : sockMax;
			FD_SET(sockValue,&fd);//读
			iFlag++;
		}

		m_GSMutexVerChnl.Unlock();

		if ( iFlag == 0)
		{
			m_GSCond.WaitTimeout(10);
			continue;
		}

		//2、select
		rt = select(sockMax+1,&fd,NULL,NULL,&timeout);

		//3、判断是否超时、是否异常、有收到数据
		if ( rt > 0 )
		{
			// 读响应
			// 如果是对方关闭 或者网线被拔 返回读 但读不到数据
			m_GSMutexVerChnl.Lock();

			// 1、检查哪个socket读响应
			for (VectorChannelPoint::size_type i=0; i<m_vectorSocketChannel.size(); i++)
			{
				if ( m_vectorSocketChannel[i]->GetChannelStatus() != CHANNEL_NORMAL )
				{
					continue;
				}
				else if ( m_vectorSocketChannel[i]->GetChannelType() == LISTEN_CHANNEL
					&& m_vectorSocketChannel[i]->GetNetProtocolType() == NET_PROTOCOL_TCP )
				{
					// TCP监听通道不参与
					continue;
				}
				else if ( m_vectorSocketChannel[i]->GetIOCPNoUse())
				{
					// TRUE 正在读数据
					continue;
				}

				SOCKETHANDLE sockValue = m_vectorSocketChannel[i]->GetCbaseSocketPoint()->GetSocket();

				if ( sockValue <= 0 )
				{
					assert(0);
					continue;
				}

				if ( FD_ISSET( sockValue,&fd) )
				{
					// 读到
					// 设置为正在读数据
					m_vectorSocketChannel[i]->SetIOCPNoUse( TRUE );
					//将任务加入线程池，由线程池读写
					pstruThreadPoolTask	 pTask = (pstruThreadPoolTask) malloc(sizeof(struThreadTask));//

					if ( NULL == pTask )
					{
						assert(0);
						continue;
					}

					memset(pTask,0x0,sizeof(struThreadTask));
					pTask->pFunction = &CCommunicationManager::ThreadAcceptData;
					pTask->enumEvent = ACCEPTDATA;
					pTask->pObject = pObject;	//pObject指向CCommunicationManager
					pTask->pObject2 = m_vectorSocketChannel[i];
					m_clsThreadPool.AssignTask(pTask);
					if ( NET_PROTOCOL_UDP == m_vectorSocketChannel[i]->GetNetProtocolType()  )
					{
						// UDP方式主动唤醒线程，这样效率高多了
						m_clsThreadPool.WakeUpThread();
					}

					// 检查是否已经找完
					rt--;
					if ( 0 == rt )
					{
						// 已经找完
						break;
					}

				}

				
			}

			m_GSMutexVerChnl.Unlock();
		}
		else if ( 0 == rt )
		{
			// 超时 继续循环
			continue;
		}
		else
		{
			// 异常 根据我的设计 异常不需处理 再次循环时异常的socket的不会加入FDSET
			// 如果是本方关闭 rt == -1
			continue;
			
		}

	}
	
	return ERROR_BASE_SUCCESS;
}
/********************************************************************************
Function:		ThreadAcceptData
Description:	接收数据，并回调通知上层.
Input:  		enumEvent 传入的参数必须是ACCEPTDATA；pObject 必须指向CSocketChannel对象
Output:      	   
Return:  		正确为ERROR_BASE_SUCCESS，其它错误码见定义       
Note:			无数据接收函数才返回。这个函数的执行和EpollWaitEvent函数紧密联系
********************************************************************************/
INT CThreadDealNetEvent::ThreadAcceptData(enumThreadEventType enumEvent, void* pObject)
{
	if ( enumEvent != ACCEPTDATA || pObject == NULL )
	{
		return ERROR_NET_PARAM_WRONG;
	}

	//1、接收数据
	//2、第一次接收就没收到数据，说明是对方关掉socket，设置通道中断标志 ，继续循环
	//3、收到数据，上报通知
	//4、数据接收完成，将socket加入队列，线程返回

	//收数据
	CSocketChannel* pclsSocketChannel = (CSocketChannel*)pObject;;
	INT		nDataLen = 0;
	UINT64	dwTick = DoGetTickCount();
	BOOL	bFirstCircle = TRUE;

	m_GSMutexExit.Lock();
	if ( !m_bIsExitAcceptData )
	{
		m_GSMutexExit.Unlock();
		return ERROR_NET_PARAM_WRONG;
	}
	m_GSMutexExit.Unlock();

	while( m_bIsExitAcceptData )	//什么时候退出循环,直到无数据
	{
		//收数据
		nDataLen = pclsSocketChannel->RecvData();

		if ( nDataLen <= 0 )
		{
			if ( bFirstCircle )
			{
				//第一次收数据没收到 说明这是连接断开
				//如果是udp的服务器端断开 是不是所有附加通道都通知上层说通讯中断？？
				if (pclsSocketChannel->GetChannelStatus() != CHANNEL_CLOSE )
				{
					//不是主动断开 通知上层
					DealDisnChannel( pclsSocketChannel );

				}				

			}

			// 不再读数据, 本来是完成端口的标志 现在也用到select模型中了。乱了点,变量名和意义不匹配
			pclsSocketChannel->SetIOCPNoUse( FALSE );
			break;
		}

		bFirstCircle = FALSE;

		LPPER_IO_OPERATION_DATA  PerIoData = pclsSocketChannel->GetIORecvData();	

		//收到数据
		if (LISTEN_CHANNEL == pclsSocketChannel->GetChannelType() && NET_PROTOCOL_UDP == pclsSocketChannel->GetNetProtocolType())
		{
			//也许这里会让你看晕
			//如果是UDP方式SERVER通道 判断是不是新的客户端发来的数据

			UINT16  unPort = 0;				
			unPort = ntohs( PerIoData->struSockAddrFrom.sin_port);
			//查找该通道是否已经存在
			CSocketChannel*  pclsNewSocketChannel = GetExistSocketChannel( PerIoData->struSockAddrFrom.sin_addr.s_addr, unPort );
			if ( NULL == pclsNewSocketChannel)
			{
				// 判断是否达到最大通道数
				if ( !IfMaxChannelCount())
				{
					//通道不存在 创建
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

						OnEventCallBack( pSocketChnl, NET_READ, PerIoData->Buffer, nDataLen );
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
				if (pclsNewSocketChannel->GetChannelStatus() != CHANNEL_CLOSE)
				{
					pclsNewSocketChannel->AddRefCount();
					pclsSocketChannel->m_uiChnIng = 1;
					ISocketChannel *pSocketChnl = (ISocketChannel*)pclsNewSocketChannel;
					OnEventCallBack( pSocketChnl,NET_READ, PerIoData->Buffer, nDataLen );
					pclsSocketChannel->m_uiChnIng = 0;
					pclsNewSocketChannel->SubRefCount();
				}

			}
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
					OnEventCallBack( pSocketChnl, NET_READ, PerIoData->Buffer, nDataLen );
					pclsSocketChannel->m_uiChnIng = 0;
					pclsSocketChannel->SubRefCount();
				}
				else
				{
					CTRLPRINTF( m_clsLogPtr," 通道%p在收到数据的时被关闭 \n", pclsSocketChannel);

				}
				pclsSocketChannel->m_csCallBackGSMutex.Unlock();

			}
			else
			{						
				CTRLPRINTF( m_clsLogPtr," 通道%p在收到数据的时被关闭 \n", pclsSocketChannel);
			}

		}

	}//end while

	return ERROR_BASE_SUCCESS;

}


// 增加事件响应 继承自CCommunicationManager
void CThreadDealNetEvent::OnEventModel( CSocketChannel* pclsSocketChannel )
{
	m_GSCond.Signal();

	return;
}

#endif 