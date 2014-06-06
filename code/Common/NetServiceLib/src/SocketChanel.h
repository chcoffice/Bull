#if !defined (SocketChanel_DEF_H)
#define SocketChanel_DEF_H

/********************************************************************
	Copyright (C), 1997-2010, gosun Tech. Co., Ltd.
	FileName: 		SocketChanel.h
	Author:        	陈聪
	Version :       v1.0   
	Date:			2010/04/28
	Description:    每个socket的信息，每个socket就是一个通道

*********************************************************************/


#include "NetServiceDataType.h"

#include "INetService.h"
#include "BaseSocket.h"
#include "IOCP.h"

namespace NetServiceLib
{

class CSocketChannel : public ISocketChannel
{
public:
	CSocketChannel(void);	
	virtual ~CSocketChannel(void);
public:
	INT		GetLocalIPPort(char* pszIP, UINT16& unPort);
	INT		GetReMoteIPPort(char* pszIP, UINT16& unPort);
	INT		SendData(void* pData, UINT unDataLen);
	INT		CloseChannel();

	ISocketChannel*		GetListenChannel();
	enumNetProtocolType	GetNetProType();
	//设置用户数据
	inline	void	SetUserData( void* pUserData ){ m_pUserData = pUserData; };
	//返回用户数据指针
	inline	void*	GetUserData(){ return m_pUserData; };
	//人工重连，仅限于TCP客户端调用	
	virtual INT		ReConnect();
	/********************************************************************************
	Function:			CloseChannelEx
	Description:		关闭通道
	Input:  
	Output:         
	Return:			ERROR_BASE_SUCCESS：表示成功。其它为错误，详见错误码定义。
	Note:			和CloseChannel不同的地方是此CloseChannelEx不对该通道加回调锁	
	********************************************************************************/
	virtual INT		CloseChannelEx();
	/********************************************************************************
	Function:		SetSendBuf
	Description:	设置系统socket的发送缓冲区大小
	Input:  		iBufSize 缓冲区值，单位Byte
	Output:      	   
	Return:  		ERROR_BASE_SUCCESS：表示成功。其它为错误，详见错误码定义。        
	Note:			这个设置仅针对该通道的socket，不影响其它通道的socket缓冲区		
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
	Note:			这个设置仅针对该通道的socket，不影响其它通道的socket缓冲区		
	Author:        	CHC
	Date:				2010/09/26
	********************************************************************************/
	virtual	INT		SetRcvBuf(INT iBufSize);
	/********************************************************************************
	Function:		SetNetBlockMode
	Description:	设置网络模式。阻塞或非阻塞
	Input:  		bMode	TRUE:非阻塞		FALSE:阻塞
	Output:      	   
	Return:  		       
	Note:			网络库默认为阻塞方式. 	可针对单个通道设置阻塞或非阻塞方式	
	Author:        	CHC
	Date:				2010/09/08
	********************************************************************************/
	virtual INT	SetNetBlockMode( BOOL bMode);
	/********************************************************************************
	Function:       SendDataEx
	Description:    发送数据。网络库将直接调用API发送数据
	Input:		  pData：数据指针，	unDataLen：数据长度
	Output:         
	Return:         ERROR_BASE_SUCCESS：表示成功。其它为错误，详见错误码定义。
	Note:			  注意，发送失败时，此函数不会返回常见的API错误码，而是网络库中定义的错误码
	此函数和SendData()的区别是不调用select函数来测试socket是否能写数据
	********************************************************************************/
	virtual INT		SendDataEx(void* pData, UINT unDataLen);
/********************************************************************************
	Function:       GetSocketHandle
	Description:    获取socket的handle
	Input:		  
	Output:         
	Return:         返回handle  NULL为无效值
	Note:			  	
********************************************************************************/
	virtual INT		GetSocketHandle(void);
public:
	INT		RecvData();
	INT		SetLocalIPPort(const char* pszIP, UINT16 unPort);
	INT		SetReMoteIPPort(const char* pszIP, UINT16 unPort);
	INT		SetCbaseSocketPoint(CBaseSocket* pBaseSocket);
	INT		SetListenSocketChannel( CSocketChannel*	pSocketChannel );
	//关闭socket
	INT		CloseHandle();//必须显式调用
	inline	void	SetNetProtocolType(enumNetProtocolType  enumNetProType){ m_enumNetProType = enumNetProType;};
	inline	void	SetServerType( enumServerType enumSvrType ){ m_enumServerType = enumSvrType; };
	inline	enumServerType	GetServerType(){ return m_enumServerType; };
	inline enumNetProtocolType	GetNetProtocolType(){return m_enumNetProType;};
	inline  CBaseSocket* GetCbaseSocketPoint(){return m_pBaseSocket;};
	inline	void	SetChannelType( enumChannelType enumCnlType ){ m_enumChannelType = enumCnlType; };
	inline	enumChannelType GetChannelType(){ return m_enumChannelType; };
	inline	INT64	GetLastActiveTime(){ CGSAutoMutex	GSAutoMutex(&m_GSMutex); return m_uiLastActiveTime;};
	void	SetLastActiveTime();
	inline	void	SetDWORDRemoteIP( DWORD dwRemoteIP){ m_dwRemoteIP = dwRemoteIP; };
	void		GetDWORDRemoteIPPort(DWORD& dwRemoteIP, UINT16& unPort){ dwRemoteIP = m_dwRemoteIP; unPort = m_unRemotePort; };
	void	SetChannelStatus(enumChannelStatus enumStatus);
	inline	enumChannelStatus	GetChannelStatus(){ CGSAutoMutex	GSAutoMutex(&m_GSMutex); return m_enumChannelStatus; };

	// 增加引用
	inline	void	AddRefCount(){ CGSAutoMutex	GSAutoMutex(&m_GSMutex);  m_uiRefCount++; };
	// 减少引用
	inline	void	SubRefCount(){ CGSAutoMutex	GSAutoMutex(&m_GSMutex);  m_uiRefCount--; };
	
	// 设置完成端口是否使用该通道
	inline	void	SetIOCPNoUse( BOOL bIOCPNoUse){ CGSAutoMutex	GSAutoMutex(&m_GSMutex); m_bIOCPNoUse = bIOCPNoUse; };
	// 获取m_bICOPNoUse
	inline	BOOL	GetIOCPNoUse(){ CGSAutoMutex	GSAutoMutex(&m_GSMutex); return m_bIOCPNoUse; };

	inline	UINT64	GetCloseTick(){ CGSAutoMutex	GSAutoMutex(&m_GSMutex); return m_uiCloseTick; };


	// 获得计数
	UINT16	GetRefCount(){ CGSAutoMutex	GSAutoMutex(&m_GSMutex); return m_uiRefCount; };

	// 设置日志指针
	void		SetLogInstancePtr( ILogLibrary* clsLogPtr){ m_clsLogPtr = clsLogPtr; };

	
public:
#if OPERATING_SYSTEM
	inline	LPPER_IO_OPERATION_DATA		GetIORecvData(){ return m_pIOData_Recv; };

#elif _WIN32
	inline	LPPER_HANDLE_DATA GetIOCPHandleData(){ return m_pIOCP_HdlData; };
	inline	void			InitIOCPHandleData(){ m_pIOCP_HdlData->pUserData = this; };
#else //_LINUX
	inline  epoll_event*	GetEpollEvent(){ return m_pstruEpollEvent; };
	inline	LPPER_IO_OPERATION_DATA		GetIORecvData(){ return m_pIOData_Recv; };
	void					InitEpollEvent();
	inline	void			AddEpollEvent( INT32	iEvent ){ m_pstruEpollEvent->events |= iEvent; };
	inline	void			DelEpollEvent( INT32	iEvent ){ m_pstruEpollEvent->events &= iEvent; };
#endif

	// 主要用于回调时锁住通道,注意和m_GSMutex的使用顺序，否则造成死锁
	CGSMutex				m_csCallBackGSMutex;

	// 测试标志  用来表示通道正在和上层交互什么 0:什么都没 1:正式回调数据 2:正在通知中断 暂时就三种
	UINT16					m_uiChnIng;

protected:
	CBaseSocket*			m_pBaseSocket;	//程序释放此对象时相当别扭，因为附加通道是不能delete的。我采取显式调用的方法
	//保存产生此通道的监听通道，只有TCP服务器端才有此情况
	CSocketChannel*			m_pListenSocketChannel;
	//通道类型，监听通道，数据通道
	enumChannelType			m_enumChannelType;
	//CGSWRMutex			m_GSWRMutex;	//发送数据或者修改此类的属性时用此锁
	CGSMutex				m_GSMutex;//改成普通锁算了
	//通道最近一次读写数据的时间，要不要读写成功才记录时间
	UINT64					m_uiLastActiveTime;
	//协议类型
	enumNetProtocolType		m_enumNetProType;
	//服务类型
	enumServerType			m_enumServerType;//客户端或服务器端
	//本地IP
	char					m_szLocalIP[16];
	//本地端口
	UINT16					m_unLocalPort;
	//远程IP或主机名
	char					m_szRemoteHost[256];
	//远程端口
	UINT16					m_unRemotePort;
	//远程IP地址的整数形式，目的是用于快速比较
	DWORD					m_dwRemoteIP;
	//用户数据指针，由上层程序指定。网络模块不做任何处理
	void*					m_pUserData;
	//通道状态	通道释放的条件之一
	enumChannelStatus		m_enumChannelStatus;

	// 使用计数 当该计数为0时才允许释放该通道 通道释放的条件之二
	UINT16					m_uiRefCount;

	// 完成端口不再使用该通道	TRUE IOCP使用 FALSE 不使用
	BOOL					m_bIOCPNoUse;

	// 关闭计时 如果上层请求CloseChannel 后3秒钟必须强行释放资源
	UINT64					m_uiCloseTick;

	// socket发送缓冲区大小 如果不设置 则采用系统的默认值
	UINT16					m_unSocketSendBuf;

	// socket接收缓冲区大小 如果不设置 则采用系统的默认值
	UINT16					m_unSocketRcvBuf;

	// 日志指针
	ILogLibrary*			m_clsLogPtr;
	

	LPPER_IO_OPERATION_DATA		m_pIOData_Recv;		//收到的数据
	
#if OPERATING_SYSTEM

#elif _WIN32
	LPPER_HANDLE_DATA			m_pIOCP_HdlData;			//完成端口句柄数据	
#else //_LINUX
	epoll_event*	m_pstruEpollEvent;
#endif
};

/*
释放通道内存的条件
正常情况
1、m_enumChannelStatus == CHANNEL_CLOSE
2、m_uiRefCount == 0
3、m_bICOPNoUse == FALSE (仅WIN32)
异常情况
1、m_enumChannelStatus == CHANNEL_CLOSE
2、与m_uiCloseTick比较 超时3秒
*/

}

#endif

