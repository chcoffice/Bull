#if !defined (NetService_DEF_H)
#define NetService_DEF_H

/********************************************************************
	Copyright (C), 1997-2010, gosun Tech. Co., Ltd.
	FileName: 		NetService.h
	Author:        	陈聪
	Version :       v1.0   
	Date:			2010/04/29
	Description:     网络服务主类，由于此类可能有多个，请谨慎使用成员对象中的静态变量，建议不使用      

*********************************************************************/
#include "INetService.h"
#include "ServerChannel.h"
#include "MemoryPool.h"
#if OPERATING_SYSTEM
#include "ThreadSelect.h"
#elif _WIN32
#include "ThreadIOCP.h"
#else //LINUX
#include "ThreadEpoll.h"
#endif

namespace NetServiceLib
{

class CNetService :
	public INetService
{
public:
	CNetService(void);
	virtual ~CNetService(void);
public:
	//继承接口中的函数
	//新的网络接口
	//初始化网络服务，准备好一些资源 比如线程 
	virtual INT InitNetService();
	//初始化简单网络服务
	virtual INT InitSimpleNetService();
	virtual INT AddServerChannel(const char* pszBindIP, UINT16 unPort, 	enumNetProtocolType eProtocolType, ISocketChannel** pSocketChnl);
	virtual INT AddClientChannel( const char *pszHost, UINT16 unDesPort,const char *pszBindIP, 
								UINT16 unLocalport,enumNetProtocolType eProtocolType,  ISocketChannel** pSocketChnl);
	virtual INT StopNetService();
	virtual INT CloseChannel(ISocketChannel* pSocketChnl);
	//以下的设置是针对该对象产生的所有通道均有效
	//设置是否支持重连 客户端设置  
	virtual INT SetReConnect(bool bReConnect);
	//设置是否支持消息缓冲
	virtual INT SetMsgBuffFlag(bool bMsgBuffFlag);
	//设置连接活动检测时间间隔，单位秒
	virtual INT SetActiveTime(UINT16 unTime);
	//设置连接数限制
	virtual INT SetMaxChannel(UINT16 unCount);
	/********************************************************************************
	Function:		SetNetBlockMode
	Description:	设置网络模式。阻塞或非阻塞
	Input:  		bMode	TRUE:非阻塞		FALSE:阻塞
	Output:      	   
	Return:  		       
	Note:			网络库默认为阻塞方式		
	Author:        	CHC
	Date:				2010/09/08
	********************************************************************************/
	virtual INT	SetNetBlockMode( BOOL bMode);
	//设置回调函数
	virtual INT SetOnEventCallBack(void* pUserData, pOnEventCallBack OnEventCallBack);

	/********************************************************************************
	Function:		SetSendBuf
	Description:	设置系统socket的发送缓冲区大小
	Input:  		iBufSize 缓冲区值，单位Byte
	Output:      	   
	Return:  		ERROR_BASE_SUCCESS：表示成功。其它为错误，详见错误码定义。        
	Note:			设置成功后，对本对象的所有新建立的socket有效。这点和ISocketChannel不同
	Author:        	CHC
	Date:				2010/09/26
	********************************************************************************/
	virtual	INT		SetSendBuf(INT iBufSize);
	/********************************************************************************
	Function:		SetRcvBuf
	Description:	设置系统socket的接收缓冲区大小
	Input:  		iBufSize 缓冲区值，单位Byte
	Output:      	   
	Return:  		ERROR_BASE_SUCCESS：表示成功。其它为错误，详见错误码定义。        
	Note:			设置成功后，对本对象的所有新建立的socket有效。	这点和ISocketChannel不同	
	Author:        	CHC
	Date:				2010/09/26
	********************************************************************************/
	virtual	INT		SetRcvBuf(INT iBufSize);

	/********************************************************************************
	Function:		GetAllChannelNum
	Description:	获取当前网络库中所有的通道数目
	Input:  		
	Output:      	   
	Return:  		返回通道数目       
	Note:			这个函数貌似没有什么实际作用，只是我在测试时需要验证demo中的通道数目是否和网络库一直		
	********************************************************************************/
	virtual	INT	GetAllChannelNum();
	/********************************************************************************
	Function:       InitLog
	Description:    初始化日志
	Input:			czPathName	日志路径
	Output:         
	Return:         ERROR_BASE_SUCCESS：表示成功。其它为错误，详见错误码定义。
	Note:			  	
	********************************************************************************/
	virtual INT		InitLog(const char *czPathName);

private:

	/********************************************************************************
	Function:       InitNetLib
	Description:	初始化网络库
	Input:			iThreadCount	线程数. 默认值0意义为由系统自行决定
	Output:         
	Return:         ERROR_BASE_SUCCESS：表示成功。其它为错误，详见错误码定义。
	Note:			  	
	********************************************************************************/
	INT	InitNetLib(UINT iThreadNum = 0);

#if _LINUX
	// 接收信号，但不做处理，按linux的要求做
	static void		ReceiveSignal( INT32 iSignalNum );

#endif
	
private:
	//该对象创建最多不能超过3个，也算是引用计数吧
	static UINT16			s_unRefCount;		

	//是否已经初始化
	bool					m_bIsInit;
	
	//完成端口或epoll
	CThreadDealNetEvent		m_clsThreadDealNetEvent;

	// 保证线程安全
	CGSMutex				m_GSMutex;

	// 客户端创建者
	CClientChannel			m_clsClientChannel;

	// 服务器端创建者		
	CServerChannel			m_clsServerChannel;

	// 退出标志 TRUE 退出 FALSE 
	BOOL					m_bIsExit;

	// SocketChannel内存
	CMemoryPool<CSocketChannel>*	m_SocketChannelMemoryPtr;

	// 完成端口线程数
	INT32					m_iIocpThreadCount;



};
}
#endif


