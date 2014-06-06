#include "NetService.h"

#include "BaseSocket.h"

#if _LINUX
#include "LinuxSocket.h"
#include <signal.h>
#endif

using namespace NetServiceLib;




#define		MAX_CNETSERVICE_COUNT		3 //CNetService对象创建最大个数

#define		MAX_MEMORYOBJECT_COUNT		10 //内存池的能容纳的对象个数

INetService* CreateNetService()
{
	return new CNetService();
}

//类的静态成员初始化
UINT16		CNetService::s_unRefCount = 0;

CNetService::CNetService(void)
{
	m_bIsInit = false;	
	m_bIsExit = FALSE;
	m_SocketChannelMemoryPtr = NULL;
}

CNetService::~CNetService(void)
{
	m_clsClientChannel.CloseChannel();
	m_clsServerChannel.CloseChannel();
	if ( !m_bIsExit)
	{
		StopNetService();
	}
	
}
/*************************************************
  Function:		InitNetService    
  Description:  初始化网络  
  Input:		无
  Output:         
  Return:         
  Note:		//1、启动网络  //2、启动线程  //3、引用计数加1	4、创建完成端口或poll 5、创建cpu数目的线程准备接收数据	
*************************************************/
INT CNetService::InitNetService()
{
	return InitNetLib();


}
INT CNetService::InitSimpleNetService()
{
	return InitNetLib(2);
}

INT	CNetService::InitNetLib(UINT iThreadNum)
{
	if(m_bIsInit)
	{
		//已经初始化，禁止重复初始化
		return ERROR_NET_REPEAT_INIT;

	}

	CGSAutoMutex		GSAutoMutex(&m_GSMutex);


	if(m_bIsInit)
	{
		//已经初始化，禁止重复初始化
		return ERROR_NET_REPEAT_INIT;

	}

	if (g_clsLogPtr == NULL)
	{
		m_clsThreadDealNetEvent.SetLogPath( GetApplicationPath().c_str() );

		m_clsThreadDealNetEvent.m_clsThreadPool.SetLogInstancePtr( m_clsThreadDealNetEvent.GetLogInstancePtr());
	}
	else
	{
		m_clsThreadDealNetEvent.SetLogInstancePtr(g_clsLogPtr);
		m_clsThreadDealNetEvent.m_clsThreadPool.SetLogInstancePtr(g_clsLogPtr);
	}

	m_bIsExit = FALSE;

	

	if ( s_unRefCount >= MAX_CNETSERVICE_COUNT )
	{
		return ERROR_NET_MAX_CNETSERVICE_COUNT;
	}	

	INT16 iThreadCount = 0;

	m_clsThreadDealNetEvent.Init();
#if OPERATING_SYSTEM
	INT iRet = m_clsThreadDealNetEvent.m_clsThreadPool.Initialize(5);//开iThreadCount个线程

	if ( ERROR_BASE_SUCCESS != iRet)
	{
		return iRet;
	}

	CTRLPRINTF(m_clsThreadDealNetEvent.GetLogInstancePtr(),"操作系统为OPERATING_SYSTEM \n");


#elif _WIN32
	if (iThreadNum > 0)
	{
		iThreadCount = iThreadNum; //一般用于客户端
	}
	else
	{
		iThreadCount = (INT16)m_clsThreadDealNetEvent.GetNumberOfProcessors() * 2 + 2;
	}

	m_iIocpThreadCount = iThreadCount;
	
	INT iRet = m_clsThreadDealNetEvent.m_clsThreadPool.Initialize(iThreadCount);//开iThreadCount个线程去接收数据

	if ( ERROR_BASE_SUCCESS != iRet)
	{
		return iRet;
	}
	CTRLPRINTF(m_clsThreadDealNetEvent.GetLogInstancePtr(),"操作系统为_WIN32 \n");
#else
	INT iRet = m_clsThreadDealNetEvent.m_clsThreadPool.Initialize(10);//开10个线程去接收数据

	if ( ERROR_BASE_SUCCESS != iRet)
	{
		return iRet;
	}
	CTRLPRINTF(m_clsThreadDealNetEvent.GetLogInstancePtr(),"操作系统为_LINUX \n");
#endif

#if _LINUX

	signal(SIGPIPE, &CNetService::ReceiveSignal);
#endif





#if _WIN32
	if ( 0 == s_unRefCount )
	{
		//第一次启动
		WSADATA wsaData;
		INT iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
		if (iResult != NO_ERROR)
		{
			return ERROR_NET_WSASTARTUP_FAIL;
		}		
	}

#endif

#if OPERATING_SYSTEM

	iThreadCount = 1; // 仅需要一个线程去检测select事件
	for (INT i=0; i<iThreadCount; i++)
	{
		pstruThreadPoolTask	 pTask = (pstruThreadPoolTask) malloc(sizeof(struThreadTask));//

		if ( NULL == pTask )
		{

			return ERROR_NET_UNKNOWN;
		}

		memset(pTask,0x0,sizeof(struThreadTask));
		pTask->pFunction = &CCommunicationManager::SelectEvent;
		pTask->enumEvent = EPOLLEVENTWAIT;
		pTask->pObject = (CCommunicationManager *)&m_clsThreadDealNetEvent;
		pTask->pObject2 = (CCommunicationManager *)&m_clsThreadDealNetEvent;
		m_clsThreadDealNetEvent.m_clsThreadPool.AssignTask(pTask);
	}

#elif _WIN32

	m_clsThreadDealNetEvent.CreateIoCompletionPortEx();//创建完成端口	


	//创建cpu*2+2个一样的接收数据任务，以便使用同样的线程数目去接收数据

	for (INT i=0; i<iThreadCount; i++)
	{
		pstruThreadPoolTask	 pTask = (pstruThreadPoolTask) malloc(sizeof(struThreadTask));//

		if ( NULL == pTask )
		{

			return ERROR_NET_UNKNOWN;
		}

		memset(pTask,0x0,sizeof(struThreadTask));
		pTask->pFunction = &CCommunicationManager::ThreadAcceptData;
		pTask->enumEvent = ACCEPTDATA;
		pTask->pObject = (CCommunicationManager *)&m_clsThreadDealNetEvent;
		pTask->pObject2 = NULL;
		m_clsThreadDealNetEvent.m_clsThreadPool.AssignTask(pTask);
		m_clsThreadDealNetEvent.m_clsThreadPool.WakeUpThread();
	}

#else //_LINUX
	//启动epoll
	m_clsThreadDealNetEvent.CreateEpoll();
	iThreadCount = 1; //EPOLL只需要一个线程去执行
	for (INT i=0; i<iThreadCount; i++)
	{
		pstruThreadPoolTask	 pTask = (pstruThreadPoolTask) malloc(sizeof(struThreadTask));//

		if ( NULL == pTask )
		{

			return ERROR_NET_UNKNOWN;
		}

		memset(pTask,0x0,sizeof(struThreadTask));
		pTask->pFunction = &CCommunicationManager::EpollWaitEvent;
		pTask->enumEvent = EPOLLEVENTWAIT;
		pTask->pObject = (CCommunicationManager *)&m_clsThreadDealNetEvent;
		pTask->pObject2 = (CCommunicationManager *)&m_clsThreadDealNetEvent;
		m_clsThreadDealNetEvent.m_clsThreadPool.AssignTask(pTask);
	}
#endif

	s_unRefCount++;  //引用计数加1

	//通道故障处理任务
	pstruThreadPoolTask	 pTask2 = (pstruThreadPoolTask) malloc(sizeof(struThreadTask));//

	if ( NULL == pTask2 )
	{

		return ERROR_NET_UNKNOWN;
	}

	memset(pTask2,0x0,sizeof(struThreadTask));
	pTask2->pFunction = &CCommunicationManager::ThreadChannelStatus;
	pTask2->enumEvent = CHANNELSTATUS;
	pTask2->pObject = (CCommunicationManager *)&m_clsThreadDealNetEvent;
	pTask2->pObject2 = NULL;
	m_clsThreadDealNetEvent.m_clsThreadPool.AssignTask(pTask2);
	m_clsThreadDealNetEvent.m_clsThreadPool.WakeUpThread();

	//通知上层新连接到达
	pstruThreadPoolTask	 pTask3 = (pstruThreadPoolTask) malloc(sizeof(struThreadTask));//

	if ( NULL == pTask2 )
	{

		return ERROR_NET_UNKNOWN;
	}

	memset(pTask3,0x0,sizeof(struThreadTask));
	pTask3->pFunction = &CCommunicationManager::NoticeUpNewConnect;
	pTask3->enumEvent = ACCEPTNOTICE;
	pTask3->pObject = (CCommunicationManager *)&m_clsThreadDealNetEvent;
	pTask3->pObject2 = NULL;
	m_clsThreadDealNetEvent.m_clsThreadPool.AssignTask(pTask3);
	m_clsThreadDealNetEvent.m_clsThreadPool.WakeUpThread();

	m_bIsInit = true;

	return ERROR_BASE_SUCCESS;
}
/*************************************************
  Function:      AddServerChannel 
  Description:   建立通讯通道，
  Input:  
  Output:         
  Return:         
  Note:			对于	
*************************************************/
INT CNetService::AddServerChannel(const char* pszBindIP, UINT16 unPort, 
					 enumNetProtocolType eProtocolType, ISocketChannel** pSocketChnl)
{
	CGSAutoMutex		GSAutoMutex(&m_GSMutex);

	if ( !m_bIsInit )
	{
		return ERROR_NET_UNKNOWN;
	}

	// 为了区分协议栈和gsp，得这么写
	/*if (!g_clsLogPtr)
	{
		m_clsThreadDealNetEvent.SetLogPath( GetApplicationPath().c_str() );

		m_clsThreadDealNetEvent.m_clsThreadPool.SetLogInstancePtr( m_clsThreadDealNetEvent.GetLogInstancePtr());
	}*/	


	if (true == m_clsThreadDealNetEvent.IfMaxChannelCount())
	{
		return ERROR_NET_MAX_CHANNEL_COUNT;//已达到最大连接数
	}
	
	if (unPort < 0) //绑定的IP为null时 表示绑定本机任意IP
	{
		return ERROR_NET_PARAM_WRONG;
	}

	//建立socketchannel
	CSocketChannel *pclsSocketChnl = new CSocketChannel();//此指针保存到队列中，直到通道需要删除时才释放.在~CNetInterfaceCommData(void)里释放
	
	if ( NULL == pclsSocketChnl )
	{
		return ERROR_NET_UNKNOWN;
	}
	
	//设置通道属性
	pclsSocketChnl->SetLocalIPPort(pszBindIP, unPort);
	pclsSocketChnl->SetServerType( SERVER );
	pclsSocketChnl->SetNetProtocolType( eProtocolType );
	pclsSocketChnl->SetChannelType( LISTEN_CHANNEL );//不管是UDP还是tcp 凡是AddServerChannel add的通道都认为是监听通道

	
	//建立basesocket
	CBaseSocket* pBaseSocket = NULL;//生命周期和pclsSocketChnl一样，
#if _WIN32
	if (eProtocolType == NET_PROTOCOL_TCP)
	{
		pBaseSocket = new CWinTcpSocket();
		
	}
	else
	{
		pBaseSocket = new CWinUdpSocket();
	}
#endif
#if _LINUX
	if (eProtocolType == NET_PROTOCOL_TCP)
	{
		pBaseSocket = new CLinuxTcpSocket();
	}
	else
	{
		pBaseSocket = new CLinuxUdpSocket();

	}
#endif

	if ( NULL == pBaseSocket )
	{
		if ( pclsSocketChnl )
		{
			pclsSocketChnl->CloseChannel();
			delete pclsSocketChnl;
			pclsSocketChnl = NULL;
		}
		return ERROR_NET_UNKNOWN;
	}
	 
	pBaseSocket->SetBlockMode( m_clsThreadDealNetEvent.GetBlockMode() );
	m_clsServerChannel.SetLocalIPPort(pszBindIP, unPort);
	//创建通道
	if (m_clsServerChannel.CreateChannel(pBaseSocket) != ERROR_BASE_SUCCESS)
	{
		//创建 通道失败
		pclsSocketChnl->CloseChannel();
		delete pclsSocketChnl;
		pclsSocketChnl = NULL;

		delete pBaseSocket;
		pBaseSocket = NULL;

		return ERROR_NET_CREATE_CHANNEL_FAIL;
	}


	//保存CBaseSocket
	pclsSocketChnl->SetCbaseSocketPoint(pBaseSocket);

	//将socketchannel存入vector容器
    m_clsThreadDealNetEvent.SaveSocketChannel(pclsSocketChnl);

	pclsSocketChnl->SetLogInstancePtr(m_clsThreadDealNetEvent.GetLogInstancePtr());

#if OPERATING_SYSTEM

	if (eProtocolType == NET_PROTOCOL_TCP)
	{
		pstruThreadPoolTask	 pTask = (pstruThreadPoolTask) malloc(sizeof(struThreadTask));//在任务结束时或程序退出时的在线程池的Uninitialize()中释放

		if ( NULL == pTask )
		{
			return ERROR_NET_UNKNOWN;
		}

		memset(pTask,0x0,sizeof(struThreadTask));
		pTask->pFunction = &CCommunicationManager::Listen;
		pTask->enumEvent = LISTEN;
		pTask->pObject = (CCommunicationManager*)&m_clsThreadDealNetEvent;
		pTask->pObject2 = pclsSocketChnl;
		m_clsThreadDealNetEvent.m_clsThreadPool.AssignTask(pTask);
		m_clsThreadDealNetEvent.m_clsThreadPool.WakeUpThread();

	}
	

#elif _WIN32
	
	if (eProtocolType == NET_PROTOCOL_UDP)
	{
		//加入完成端口
		m_clsThreadDealNetEvent.AddToIoCompletionPort((HANDLE)pBaseSocket->GetSocket(), (DWORD)pclsSocketChnl->GetIOCPHandleData());
		//发起接收
		pclsSocketChnl->RecvData();
	}
	else
	{
		//m_clsThreadDealNetEvent.m_clsThreadPool.AddThread(1);//增加一个线程去完成下面创建的任务，即监听

		pstruThreadPoolTask	 pTask = (pstruThreadPoolTask) malloc(sizeof(struThreadTask));//在任务结束时或程序退出时的在线程池的Uninitialize()中释放

		if ( NULL == pTask )
		{
			return ERROR_NET_UNKNOWN;
		}

		memset(pTask,0x0,sizeof(struThreadTask));
		pTask->pFunction = &CCommunicationManager::Listen;
		pTask->enumEvent = LISTEN;
		pTask->pObject = (CCommunicationManager*)&m_clsThreadDealNetEvent;
		pTask->pObject2 = pclsSocketChnl;
		m_clsThreadDealNetEvent.m_clsThreadPool.AssignTask(pTask);
		m_clsThreadDealNetEvent.m_clsThreadPool.WakeUpThread();
	}
	

#else	//linux
	
	if (eProtocolType == NET_PROTOCOL_UDP)
	{
		//加入EPOLL
		m_clsThreadDealNetEvent.EpollCtrl( EPOLL_CTL_ADD,pBaseSocket->GetSocket(),pclsSocketChnl->GetEpollEvent());

	}
	else
	{

		pstruThreadPoolTask	 pTask = (pstruThreadPoolTask) malloc(sizeof(struThreadTask));//在任务结束时或程序退出时的在线程池的Uninitialize()中释放

		if ( NULL == pTask )
		{
			return ERROR_NET_UNKNOWN;
		}

		memset(pTask,0x0,sizeof(struThreadTask));
		pTask->pFunction = &CCommunicationManager::Listen;
		pTask->enumEvent = LISTEN;
		pTask->pObject = (CCommunicationManager*)&m_clsThreadDealNetEvent;
		pTask->pObject2 = pclsSocketChnl;
		m_clsThreadDealNetEvent.m_clsThreadPool.AssignTask(pTask);
	}
#endif

	//转换
	*pSocketChnl = (ISocketChannel*)pclsSocketChnl;

	return ERROR_BASE_SUCCESS;
}
/********************************************************************************
  Function:		AddClientChannel
  Description:	增加一个客户端
  Input:  		pszhost 主机地址	unDesPort 目的端口	pszBindIP 本地IP地址 可以是0 表示本机的任意IP	localport 本地端口 可以为0 系统自动分配
  Output:		pSocketChnl   成功返回通道指针 
  Return:  		       
  Note:					
  Author:        	CHC
  Date:				2010/08/31
********************************************************************************/
INT CNetService::AddClientChannel( const char *pszhost, UINT16 unDesPort,const char *pszBindIP, 
									UINT16 localport,enumNetProtocolType eProtocolType,  ISocketChannel** pSocketChnl)
{
	if ( m_bIsExit )
	{
		return ERROR_NET_UNKNOWN;
	}

	if ( !m_bIsInit )
	{
		return ERROR_NET_UNKNOWN;
	}

	CGSAutoMutex		GSAutoMutex(&m_GSMutex);

	// 为了区分协议栈和gsp，得这么写
	/*if (!g_clsLogPtr)
	{
	m_clsThreadDealNetEvent.SetLogPath( GetApplicationPath().c_str() );

	m_clsThreadDealNetEvent.m_clsThreadPool.SetLogInstancePtr( m_clsThreadDealNetEvent.GetLogInstancePtr());
	}*/

	if (true == m_clsThreadDealNetEvent.IfMaxChannelCount())
	{
		return ERROR_NET_MAX_CHANNEL_COUNT;//已达到最大连接数
	}

	if (pszhost == NULL)		
	{
		return ERROR_NET_PARAM_WRONG;
	}

	//建立socketchannel
	CSocketChannel *pclsSocketChnl = new CSocketChannel();//此指针保存到队列中，直到通道需要删除时才释放，比如程序退出，或者上层请求关闭通道
	if ( NULL == pclsSocketChnl )
	{
		// 没申请到内存
		return ERROR_NET_UNKNOWN;
	}
	pclsSocketChnl->SetLocalIPPort(pszBindIP, localport);
	pclsSocketChnl->SetReMoteIPPort(pszhost, unDesPort);
	pclsSocketChnl->SetNetProtocolType( eProtocolType );
	pclsSocketChnl->SetServerType( CLIENT );
	pclsSocketChnl->SetChannelType( COMM_CHANNEL );//客户端建立的都是通讯通道

	//建立basesocket
	CBaseSocket* pBaseSocket = NULL;//生命周期和pclsSocketChnl一样，
#if _WIN32
	if (eProtocolType == NET_PROTOCOL_TCP)
	{
		pBaseSocket = new CWinTcpSocket();
	}
	else
	{
		pBaseSocket = new CWinUdpSocket();

	}
#endif

#if _LINUX
	if (eProtocolType == NET_PROTOCOL_TCP)
	{
		pBaseSocket = new CLinuxTcpSocket();
	}
	else
	{
		pBaseSocket = new CLinuxUdpSocket();

	}
#endif

	if ( NULL == pBaseSocket )
	{
		// 没申请到内存
		return ERROR_NET_UNKNOWN;
	}

	pBaseSocket->SetBlockMode( m_clsThreadDealNetEvent.GetBlockMode() );
	m_clsClientChannel.SetLocalIPPort(pszBindIP, localport);
	m_clsClientChannel.SetReMoteIPPort(pszhost,unDesPort);


	//创建通道
	if (m_clsClientChannel.CreateChannel(pBaseSocket) != ERROR_BASE_SUCCESS)
	{
		//创建 通道失败
		pclsSocketChnl->CloseChannel();
		delete pclsSocketChnl;
		pclsSocketChnl = NULL;

		delete pBaseSocket;
		pBaseSocket = NULL;	

		char szRemoteIP[16]={0};
		UINT16 uiRemotePort = 0;
		char szLocalIP[16]={0};
		UINT16 uiLocalPort = 0;
		m_clsClientChannel.GetLocalIPPort(szLocalIP,uiLocalPort);
		m_clsClientChannel.GetReMoteIPPort(szRemoteIP,uiRemotePort);
		CTRLPRINTF( g_clsLogPtr,"创建通道失败传入通道参数,pszhost:%s, unDesPort:%u, pszBindIP:%s, localport:%u \n",pszhost, unDesPort,pszBindIP,localport );
		CTRLPRINTF( g_clsLogPtr,"创建通道失败客户端通道参数,szRemoteIP:%s, uiRemotePort:%u, szLocalIP:%s, uiLocalPort:%u \n",szRemoteIP, uiRemotePort,szLocalIP,uiLocalPort );
		return ERROR_NET_CREATE_CHANNEL_FAIL;
	}

	sockaddr_in struLocalAddr;
#if _WIN32
	INT32   iLocalAddrLen = sizeof(sockaddr_in);
#endif
#if _LINUX
	socklen_t   iLocalAddrLen = sizeof(sockaddr_in);
#endif
	getsockname( pBaseSocket->GetSocket(), (sockaddr*)&struLocalAddr, &iLocalAddrLen );
	pclsSocketChnl->SetLocalIPPort(inet_ntoa(struLocalAddr.sin_addr), (UINT16) ntohs(struLocalAddr.sin_port));

	//保存CBaseSocket
	pclsSocketChnl->SetCbaseSocketPoint(pBaseSocket);

	//将socketchannel存入vector容器
	m_clsThreadDealNetEvent.SaveSocketChannel(pclsSocketChnl);

	pclsSocketChnl->SetLogInstancePtr(m_clsThreadDealNetEvent.GetLogInstancePtr());

	CTRLPRINTF(m_clsThreadDealNetEvent.GetLogInstancePtr(), "通道%p创建成功, new socket %d, szRemoteIP:%s, \
		uiRemotePort:%u, szLocalIP:%s, uiLocalPort:%u\n ",pclsSocketChnl,pBaseSocket->GetSocket(),pszhost,unDesPort,
		inet_ntoa(struLocalAddr.sin_addr), (UINT16) ntohs(struLocalAddr.sin_port));

#if OPERATING_SYSTEM

	// select 什么都不需要做

#elif _WIN32

	//加入完成端口
	HANDLE hHandle = m_clsThreadDealNetEvent.AddToIoCompletionPort((HANDLE)pBaseSocket->GetSocket(), (DWORD)pclsSocketChnl->GetIOCPHandleData());
	if (hHandle == NULL)
	{
		pclsSocketChnl->CloseChannel();
		delete pclsSocketChnl;
		pclsSocketChnl = NULL;

		delete pBaseSocket;
		pBaseSocket = NULL;	
		CTRLPRINTF(m_clsThreadDealNetEvent.GetLogInstancePtr(), "通道%p加入完成端口失败, socket %d\n ",pclsSocketChnl,pBaseSocket->GetSocket());
		return ERROR_NET_CREATE_CHANNEL_FAIL;
	}
	//投掷第一次接收数据请求
	pclsSocketChnl->RecvData();

#else
	//linux

	//加入epoll
	m_clsThreadDealNetEvent.EpollCtrl( EPOLL_CTL_ADD,pBaseSocket->GetSocket(),pclsSocketChnl->GetEpollEvent());

	pclsSocketChnl->AddRefCount();

#endif
	//转换
	*pSocketChnl = (ISocketChannel*)pclsSocketChnl;


	return ERROR_BASE_SUCCESS;
}
/********************************************************************************
  Function:       StopNetService
  Description:    
  Input:  
  Output:         
  Return:         
  Note:			  主要过程：1、停止线程 2、关闭所有socket 3、释放所有通道 4、释放线程和任务 5、引用计数减一
							6、根据引用计数判断是否释放网络资源WSACleanup
********************************************************************************/
INT CNetService::StopNetService()//调用此函数停止网络服务，将不会调用回调通知上层
{
	
	UINT64	uiTick = DoGetTickCount();

	CGSAutoMutex		GSAutoMutex(&m_GSMutex);

	if ( m_bIsExit )
	{
		return ERROR_BASE_SUCCESS;
	}

	m_bIsExit = TRUE;

	m_clsThreadDealNetEvent.SetExitChannelStatus();	////设置退出通道状态处理线程标志

	m_clsThreadDealNetEvent.FreeAllChannel();//关闭通道	

#if OPERATING_SYSTEM
#elif _WIN32
	m_clsThreadDealNetEvent.ExitAcceptData(m_iIocpThreadCount);//退出接收数据的while循环
#else
#endif


	m_clsThreadDealNetEvent.m_clsThreadPool.SetIsExitWorkerThreadProc();//让所有线程都退出循环。其实，正在执行任务的线程要等到下一步才能真正退出

	//一、让所有线程执行的所有任务都停止，其实就是结束各个while循环 //只是设个标志 没有判断循环是否真正结束，这样不稳妥的
	m_clsThreadDealNetEvent.SetExitListen();//退出监听 
	m_clsThreadDealNetEvent.SetExitAcceptData();//退出接收数据
	m_clsThreadDealNetEvent.SetExitActiveTest();//退出连接活动检测
	m_clsThreadDealNetEvent.SetExitAcceptUpNotice();//退出退出通知上层新连接到达标志
	

#if OPERATING_SYSTEM
	m_clsThreadDealNetEvent.SetExitSelectEvent();
#elif _WIN32
	//m_clsThreadDealNetEvent.ExitAcceptData();//退出接收数据的while循环
	//m_clsThreadDealNetEvent.PostQueuedCompletionStatusEx();//退出完成端口
#else
	m_clsThreadDealNetEvent.SetExitEpollEventWait();
#endif

	//停止全部线程  退掉线程绝对不是一件简单的事 
	//1、锁定任务队列，并删除所有未执行的任务，避免线程将其执行
	//2、停止所有线程的while循环
	//3、释放线程资源
	//4、解掉任务队列锁
	m_clsThreadDealNetEvent.m_clsThreadPool.Uninitialize();

	if ( m_SocketChannelMemoryPtr )
	{
		delete m_SocketChannelMemoryPtr;
		m_SocketChannelMemoryPtr = NULL;
	}

	//置未初始化标志
	m_bIsInit = false;

	//引用计数减一
	s_unRefCount--;
	if (s_unRefCount == 0)
	{
#if _WIN32
		WSACleanup();
#endif

	}

	CTRLPRINTF(m_clsThreadDealNetEvent.GetLogInstancePtr(),"网络库退出耗时 %u 毫秒\n", DoGetTickCount() - uiTick);

	return ERROR_BASE_SUCCESS;
}
/********************************************************************************
  Function:      CloseChannel 
  Description:   关闭通道 
  Input:  
  Output:         
  Return:         
  Note:			
********************************************************************************/
INT CNetService::CloseChannel(ISocketChannel* pSocketChnl)
{
	CGSAutoMutex		GSAutoMutex(&m_GSMutex);

	if ( pSocketChnl == NULL )
	{
		return ERROR_NET_PARAM_WRONG;
	}
	CSocketChannel*  pclsSocketChannel = dynamic_cast<CSocketChannel*>(pSocketChnl);
	if ( NULL == pclsSocketChannel )
	{
		return ERROR_NET_PARAM_WRONG;
	}

	//想个法子检查指针的合法性，或者指针对象是否还存在
	enumChannelType			enumCnvType;
	try
	{
		enumCnvType = pclsSocketChannel->GetChannelType();
		if ( COMM_CHANNEL != enumCnvType && LISTEN_CHANNEL != enumCnvType )
		{
			return ERROR_NET_INVALID_CHANNEL_POINT;
		}
		
	}
	catch ( ...)
	{
		return ERROR_NET_CHANNEL_NOT_EXIST;
	}

	pclsSocketChannel->CloseChannel();

	return ERROR_BASE_SUCCESS;
}
INT CNetService::SetReConnect(bool bReConnect)
{
	CGSAutoMutex		GSAutoMutex(&m_GSMutex);

	m_clsThreadDealNetEvent.SetReConnect(bReConnect);
	
	return ERROR_BASE_SUCCESS;

}
INT CNetService::SetMsgBuffFlag(bool bMsgBuffFlag)
{
	CGSAutoMutex		GSAutoMutex(&m_GSMutex);

	m_clsThreadDealNetEvent.SetMsgBufFlag(bMsgBuffFlag);

	return ERROR_BASE_SUCCESS;

}
INT CNetService::SetActiveTime( UINT16 unTime )
{
	CGSAutoMutex		GSAutoMutex(&m_GSMutex);
	if (unTime <= 0)
	{
		return ERROR_NET_PARAM_WRONG;
	}

	m_clsThreadDealNetEvent.SetActiveTime(unTime);

	return ERROR_BASE_SUCCESS;

}

INT CNetService::SetMaxChannel( UINT16 unCount )
{
	CGSAutoMutex		GSAutoMutex(&m_GSMutex);

	if (unCount <= 0)
	{
		return ERROR_NET_PARAM_WRONG;
	}

	m_clsThreadDealNetEvent.SetMaxChannel( unCount );

	return ERROR_BASE_SUCCESS;

}

INT CNetService::SetOnEventCallBack(void* pUserData, pOnEventCallBack OnEventCallBack)
{
	CGSAutoMutex		GSAutoMutex(&m_GSMutex);
	
	//pUserData是可以为null的 上层程序觉得不需要时
	if ( NULL == OnEventCallBack )
	{
		return ERROR_NET_PARAM_WRONG;
	}

	return m_clsThreadDealNetEvent.SetOnEventCallBack(pUserData, OnEventCallBack);
}

INT	CNetService::GetAllChannelNum()
{

	CGSAutoMutex		GSAutoMutex(&m_GSMutex);

	//把这个函数当成测试函数来写，显示一些网络库的数据
	printf("当前网络库中通道数目是 %d \n", m_clsThreadDealNetEvent.GetAllChannelNum());
	printf("当前网络库中所有线程数目是 %d \n", m_clsThreadDealNetEvent.m_clsThreadPool.GetAllThreadCount());
	printf("当前网络库中忙碌线程数目是 %d \n", m_clsThreadDealNetEvent.m_clsThreadPool.GetBusyThreadCount());
	printf("当前网络库中空闲线程数目是 %d \n", m_clsThreadDealNetEvent.m_clsThreadPool.GetIdleThreadCount());
	printf("当前网络库中任务数目是 %d \n", m_clsThreadDealNetEvent.m_clsThreadPool.GetTaskCount());
	printf("当前网络库中待删除的线程数目是 %d \n", m_clsThreadDealNetEvent.m_clsThreadPool.GetWaitDelThreadNum());
	printf("当前网络库中删除队列中的线程数目是 %d \n", m_clsThreadDealNetEvent.m_clsThreadPool.GetVecWaitDelThread());
	return	m_clsThreadDealNetEvent.GetAllChannelNum();
}

INT CNetService::SetNetBlockMode( BOOL bMode)
{
	// 不能由上层来决定是阻塞或非阻塞模式，没有意义，因为select、epoll模型要求必须是非阻塞。
	/*CGSAutoMutex		GSAutoMutex(&m_GSMutex);
	
	m_clsThreadDealNetEvent.SetBlockMode( bMode );*/

	return ERROR_BASE_SUCCESS;
}



INT CNetService::SetSendBuf(INT iBufSize)
{
	CGSAutoMutex		GSAutoMutex(&m_GSMutex);

	if ( iBufSize <=0 )
	{
		return ERROR_NET_PARAM_WRONG;
	}

	if ( ERROR_BASE_SUCCESS != m_clsThreadDealNetEvent.SetSendBuf( iBufSize ) )
	{
		return ERROR_NET_PARAM_WRONG;
	}

	return ERROR_BASE_SUCCESS;
	
}

INT CNetService::SetRcvBuf(INT iBufSize)
{
	CGSAutoMutex		GSAutoMutex(&m_GSMutex);

	if ( iBufSize <=0 )
	{
		return ERROR_NET_PARAM_WRONG;
	}

	if ( ERROR_BASE_SUCCESS != m_clsThreadDealNetEvent.SetRcvBuf( iBufSize ) )
	{
		return ERROR_NET_PARAM_WRONG;
	}

	return ERROR_BASE_SUCCESS;
}

INT	CNetService::InitLog(const char *czPathName)
{
	if ( NULL == czPathName  )
	{
		return ERROR_NET_PARAM_WRONG;
	}

	//同意在initservice时建立日志
	/*CGSAutoMutex		GSAutoMutex(&m_GSMutex);

	m_clsThreadDealNetEvent.SetLogPath( czPathName );

	m_clsThreadDealNetEvent.m_clsThreadPool.SetLogInstancePtr( m_clsThreadDealNetEvent.GetLogInstancePtr());*/

	return ERROR_BASE_SUCCESS;
}

#if _LINUX

void CNetService::ReceiveSignal( INT32 iSignalNum )
{
	// 不需要做任何处理
	
	return;
}

#endif






