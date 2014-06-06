#include "ThreadEpoll.h"
#if OPERATING_SYSTEM

#elif  defined(_LINUX)



using namespace NetServiceLib;

//线程执行函数 用于轮询linux的epoll事件
/********************************************************************************
  Function:		EpollWaitEvent
  Description:	等待socket的epoll事件发生，发生的事件加入线程池任务列表，由线程池处理
  Input:  		enumEvent 传入的参数必须是EPOLLEVENTWAIT；pObject 指向CCommunicationManager对象
  Output:      	   
  Return:  		正确为ERROR_BASE_SUCCESS，其它错误码见定义       
  Note:					
********************************************************************************/
INT CThreadDealNetEvent::EpollWaitEvent(enumThreadEventType enumEvent, void* pObject)
{
	if ( enumEvent != EPOLLEVENTWAIT || pObject == NULL )
	{
		return ERROR_NET_PARAM_WRONG;
	}

	struct		epoll_event events[10];			//回传代处理的事件的数组
	int			nfds;							//epoll_wait的返回值
	int			iIndex;							//数组索引
	CSocketChannel*  pclsSocketChannel = NULL;

	m_GSMutexExit.Lock();
	if ( !m_bIsExitEpollEventWait )
	{
		m_GSMutexExit.Unlock();
		return ERROR_NET_PARAM_WRONG;
	}
	m_GSMutexExit.Unlock();

	while( m_bIsExitEpollEventWait )	
	{
		nfds = EpollWait( events, 10, 1 );			

		if (nfds == -1) 
		{
			printf("nfds == -1, %d, %s\n", errno, strerror(errno));
			if( errno == EINTR ) //有中断发生 
			{
				continue;
			}
			//否则就退出程序，为什么
			//exit(-1);			
		}
		else if( nfds == 0 ) 
		{
			//超时
			continue;
		}	

		if ( nfds < -1 )
		{
			printf("epoll error, %d, %s\n", errno, strerror(errno));
		}

		for (iIndex = 0; iIndex < nfds; ++iIndex) 
		{			
			if( !events[iIndex].data.ptr )  
			{
				//不能出现该情形
				assert(0);
				continue;
			}

			pclsSocketChannel = (CSocketChannel*)events[iIndex].data.ptr;


			if( events[iIndex].events & EPOLLIN ) 
			{ //读取操作
				//邹工在此加了锁 我不知道是为什么 所以先不加
				pclsSocketChannel->DelEpollEvent( ~EPOLLIN );
				EpollCtrl( EPOLL_CTL_MOD,pclsSocketChannel->GetCbaseSocketPoint()->GetSocket(), pclsSocketChannel->GetEpollEvent());

				//将任务加入线程池，由线程池读写
				pstruThreadPoolTask	 pTask = (pstruThreadPoolTask) malloc(sizeof(struThreadTask));//

				if ( NULL == pTask )
				{
					assert(0);
					continue;
				}

				pclsSocketChannel->AddRefCount();

				memset(pTask,0x0,sizeof(struThreadTask));
				pTask->pFunction = &CCommunicationManager::ThreadAcceptData;
				pTask->enumEvent = ACCEPTDATA;
				pTask->pObject = pObject;	//pObject指向CCommunicationManager
				pTask->pObject2 = pclsSocketChannel;
				m_clsThreadPool.AssignTask(pTask);
				if ( NET_PROTOCOL_UDP == pclsSocketChannel->GetNetProtocolType()  )
				{
					// UDP方式主动唤醒线程，这样效率高多了
					m_clsThreadPool.WakeUpThread();
				}
				
			}

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

					//从EPOLL中去掉
					EpollCtrl( EPOLL_CTL_DEL, pclsSocketChannel->GetCbaseSocketPoint()->GetSocket(), pclsSocketChannel->GetEpollEvent());

					DealDisnChannel( pclsSocketChannel );


					
				}
				else
				{
					//释放通道资源

					//从EPOLL中去掉
					EpollCtrl( EPOLL_CTL_DEL, pclsSocketChannel->GetCbaseSocketPoint()->GetSocket(), pclsSocketChannel->GetEpollEvent());

				}

				pclsSocketChannel->SubRefCount();

				return ERROR_BASE_SUCCESS; //直接退出该函数, 跟break的流程是不一样的
				
			}

			pclsSocketChannel->SubRefCount();			

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

	if ( NULL != pclsSocketChannel)
	{
		if (  CHANNEL_NORMAL == pclsSocketChannel->GetChannelStatus()  )
		{
			pclsSocketChannel->AddEpollEvent( EPOLLIN );
			EpollCtrl( EPOLL_CTL_MOD, pclsSocketChannel->GetCbaseSocketPoint()->GetSocket(), pclsSocketChannel->GetEpollEvent());

		}
		

	}
	
	return ERROR_BASE_SUCCESS;

}

// 增加事件响应 继承自CCommunicationManager
void CThreadDealNetEvent::OnEventModel( CSocketChannel* pclsSocketChannel )
{
	EpollCtrl( EPOLL_CTL_ADD, pclsSocketChannel->GetCbaseSocketPoint()->GetSocket(), pclsSocketChannel->GetEpollEvent());

	pclsSocketChannel->AddRefCount();
}


#endif //_LINUX


