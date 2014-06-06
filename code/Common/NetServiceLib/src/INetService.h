#if !defined (INETSERVICE_DEF_H)
#define INETSERVICE_DEF_H
/********************************************************************
	Copyright (C), 1997-2010, gosun Tech. Co., Ltd.
	FileName: 		INetService.h
	Author:        	陈聪
	Version :       v1.0   
	Date:			2010/04/28
	Description:    此文件为网络库的唯一的对外接口。上层最多可以创建3个网络服务类，每个网络服务类都是完全独立的，
					可以设置单独的属性和回调函数。最多只允许创建3个网络服务类是因为每个网络服务类至少拥有2*CPU
					内核个数的线程，网络库不希望创建过多的线程.
					ISocketChannel是通道虚基类：	上层通过此虚基类提供的接口进行数据收发和获取通道相关属性。
													通道不能直接在外部调用delete ISocketChannel 进行释放资源，
													而必须显式调用CloseChannel()函数进行释放。
					INetService是网络服务虚基类：	上层通过此虚基类提供的接口初始化网络服务以及增加、删除通道。
													设置网络服务的相关属性。

*********************************************************************/
#include "DataType.h"

using namespace NetServiceLib;

namespace NetServiceLib
{

class ISocketChannel
{
public:
/********************************************************************************
  Function:       GetLocalIPPort
  Description:    获取通道的本地端口
  Input:			
  Output:         pszIP：IP地址		unPort：端口
  Return:         ERROR_BASE_SUCCESS：表示成功。其它为错误，详见错误码定义。
  Note:				
********************************************************************************/
	virtual	INT		GetLocalIPPort(char* pszIP, UINT16& unPort)=0;
/********************************************************************************
  Function:       GetReMoteIPPort
  Description:    获取通道的远程端口
  Input:			
  Output:         pszIP：IP地址		unPort：端口
  Return:         ERROR_BASE_SUCCESS：表示成功。其它为错误，详见错误码定义。
  Note:				
********************************************************************************/
	virtual INT		GetReMoteIPPort(char* pszIP, UINT16& unPort)=0;
/********************************************************************************
  Function:       SendData
  Description:    发送数据。网络库将直接调用API发送数据
  Input:		  pData：数据指针，	unDataLen：数据长度
  Output:         
  Return:         ERROR_BASE_SUCCESS：表示成功。其它为错误，详见错误码定义。
  Note:			  注意，发送失败时，此函数不会返回常见的API错误码，而是网络库中定义的错误码
********************************************************************************/
	virtual INT		SendData(void* pData, UINT unDataLen)=0;
	//采用此函数关闭通道,跟INetService中的CloseChannel是一样的。
/********************************************************************************
  Function:			CloseChannel
  Description:		关闭通道
  Input:  
  Output:         
  Return:			ERROR_BASE_SUCCESS：表示成功。其它为错误，详见错误码定义。
  Note:				
********************************************************************************/
	virtual INT		CloseChannel()=0;
/********************************************************************************
	Function:			CloseChannelEx
	Description:		关闭通道
	Input:  
	Output:         
	Return:			ERROR_BASE_SUCCESS：表示成功。其它为错误，详见错误码定义。
	Note:			和CloseChannel不同的地方是此CloseChannelEx不对该通道加回调锁	
********************************************************************************/
	virtual INT		CloseChannelEx()=0;
/********************************************************************************
  Function:       GetListenChannel
  Description:    获取通道的监听通道。TCP或UDP均可以。
  Input:  
  Output:         
  Return:         如果有，将返回监听通道的指针ISocketChannel*，如果没有将返回NULL。
  Note:			  不是每个通道都有监听通道。比如客户端通道就没有。
********************************************************************************/
	virtual ISocketChannel*		GetListenChannel()=0;
/********************************************************************************
  Function:       GetNetProType
  Description:    获取网络协议类型
  Input:  
  Output:         
  Return:         详见enumNetProtocolType
  Note:				
********************************************************************************/
	virtual enumNetProtocolType	GetNetProType()=0;
public:
/********************************************************************************
  Function:		SetUserData
  Description:	设置用户数据。上层可以为每个通道单独设置用户数据
  Input:  		pUserData: void*指针 用户数据
  Output:      	   
  Return:  		       
  Note:					
********************************************************************************/
	virtual void	SetUserData( void* pUserData )=0;
/********************************************************************************
  Function:		GetUserData
  Description:	返回用户数据指针
  Input:  		
  Output:      	   
  Return:  		如果有，将返回void*，如果没有设置用户数据，返回NULL       
  Note:					
********************************************************************************/
	virtual void*	GetUserData()=0;
/********************************************************************************
  Function:		ManReConnect
  Description:	人工重连，仅限于TCP客户端调用
  Input:  		
  Output:      	   
  Return:  		ERROR_BASE_SUCCESS：表示成功。其它为错误，详见错误码定义。       
  Note:					
********************************************************************************/
	virtual INT		ReConnect()=0;
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
	virtual	INT		SetSendBuf(INT iBufSize)=0;
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
	virtual	INT		SetRcvBuf(INT iBufSize)=0;
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
	virtual INT	SetNetBlockMode( BOOL bMode)=0;
/********************************************************************************
	Function:       SendDataEx
	Description:    发送数据。网络库将直接调用API发送数据
	Input:		  pData：数据指针，	unDataLen：数据长度
	Output:         
	Return:         ERROR_BASE_SUCCESS：表示成功。其它为错误，详见错误码定义。
	Note:			  注意，发送失败时，此函数不会返回常见的API错误码，而是网络库中定义的错误码
					此函数和SendData()的区别是不调用select函数来测试socket是否能写数据
********************************************************************************/
	virtual INT		SendDataEx(void* pData, UINT unDataLen)=0;
/********************************************************************************
	Function:       GetSocketHandle
	Description:    获取socket的handle
	Input:		  
	Output:         
	Return:         返回handle  NULL为无效值
	Note:			  	
********************************************************************************/
	virtual INT		GetSocketHandle(void)=0;

protected:
	ISocketChannel(void){};	
	virtual ~ISocketChannel(void){};

};

class INetService
{

public:
/********************************************************************************
  Function:		InitNetService
  Description:	初始化网络服务。
  Input:  		
  Output:      	   
  Return:  		ERROR_BASE_SUCCESS：表示成功。其它为错误，详见错误码定义。       
  Note:			主要过程：	1、启动网络  2、启动线程  3、引用计数加1	4、创建完成端口或poll 
							5、创建cpu数目的线程准备接收数据			
********************************************************************************/
	virtual INT InitNetService()=0;
/********************************************************************************
  Function:		InitSimpleNetService
  Description:	初始化简单网络服务。
  Input:  		
  Output:      	   
  Return:  		ERROR_BASE_SUCCESS：表示成功。其它为错误，详见错误码定义。       
  Note:			主要过程：	1、启动网络  2、启动线程  3、引用计数加1	4、创建完成端口或poll 
							5、创建cpu数目的线程准备接收数据
				InitSimpleNetService 和 InitNetService 个主要区别是只启用1个线程去接收完成端口数据，
				减少线程数量，适合作为客户端时初始化使用。两者只能调用其中一个。
********************************************************************************/
	virtual INT InitSimpleNetService()=0;

/********************************************************************************
  Function:		AddServerChannel
  Description:	增加服务器通道
  Input:  		pszBindIP：绑定IP，即服务器端IP。格式必须形如"127.0.0.1"。	如果要使用默认IP，可填NULL或"0"。unPort：端口。
				eProtocolType：协议类型。
  Output:      	pSocketChnl：二级指针，指向通道的指针。   
  Return:  		ERROR_BASE_SUCCESS：表示成功。其它为错误，详见错误码定义。           
  Note:			上层应保存通道指针pSocketChnl，以便通过该指针进行相关操作。		
********************************************************************************/
	virtual INT AddServerChannel(const char* pszBindIP, UINT16 unPort, enumNetProtocolType eProtocolType, 
								ISocketChannel** pSocketChnl)=0;
/********************************************************************************
  Function:		AddClientChannel
  Description:	增加客户端通道
  Input:  		pszhost：主机地址，即对端服务器的IP。格式必须形如"127.0.0.1",或者为主机串。	unDesPort：对方端口
				pszBindIP：绑定IP，即本地IP。如果要使用默认IP，可填NULL或"0"。unLocalPort：本地端口。eProtocolType：协议类型。
  Output:      	pSocketChnl：二级指针，指向通道的指针。   
  Return:  		ERROR_BASE_SUCCESS：表示成功。其它为错误，详见错误码定义。           
  Note:			上层应保存通道指针pSocketChnl，以便通过该指针进行相关操作。		
********************************************************************************/
	virtual INT AddClientChannel( const char *pszHost, UINT16 unDesPort,const char *pszBindIP, 
								UINT16 unLocalPort,enumNetProtocolType eProtocolType,  ISocketChannel** pSocketChnl)=0;
/********************************************************************************
  Function:		StopNetService
  Description:	停止网络服务。这将释放该网络服务产生的所有通道。调用此函数完毕后，上层程序必须释放网络库对象，
				而不能直接调用InitNetService()又重新使用网络库。
  Input:  		
  Output:      	   
  Return:  		ERROR_BASE_SUCCESS：表示成功。其它为错误，详见错误码定义。        
  Note:			主要过程：1、停止线程 2、关闭所有socket 3、释放所有通道 4、释放线程和任务 5、引用计数减一
  6、根据引用计数判断是否释放网络资源WSACleanup		
********************************************************************************/
	virtual INT StopNetService()=0;
/********************************************************************************
  Function:		CloseChannel
  Description:	关闭通道，并释放通道。
  Input:  		pSocketChnl:需要关闭的通道指针
  Output:      	   
  Return:  		ERROR_BASE_SUCCESS：表示成功。其它为错误，详见错误码定义。        
  Note:			关闭通道后，pSocketChnl指针将失效。上层应注意。	
				这个函数作用和ISocketChannel::CloseChannel()完全相同
********************************************************************************/
	virtual INT CloseChannel(ISocketChannel* pSocketChnl)=0;
	//以下的设置是针对该对象产生的所有通道均有效 
/********************************************************************************
  Function:		SetReConnect
  Description:	设置重连标志。网络库默认为重连
  Input:  		bReConnect:值为true表示重连，false不重连。
  Output:      	   
  Return:  		ERROR_BASE_SUCCESS：表示成功。其它为错误，详见错误码定义。       
  Note:			重连仅是通道内部重新建立socket，通道指针未改变，重连成功后，上层可以继续使用该通道指针。
				重连失败或成功将回调通知上层。
********************************************************************************/
	virtual INT SetReConnect(bool bReConnect)=0;
	//设置是否支持消息缓冲
/********************************************************************************
  Function:		SetMsgBuffFlag
  Description:	设置是否支持消息缓冲.网络库默认为支持
  Input:  		bMsgBuffFlag:值为true表示支持，false不支持。
  Output:      	   
  Return:  		ERROR_BASE_SUCCESS：表示成功。其它为错误，详见错误码定义。        
  Note:			目前网络库暂未实现此功能		
********************************************************************************/
	virtual INT SetMsgBuffFlag(bool bMsgBuffFlag)=0;
	
/********************************************************************************
  Function:		SetActiveTime
  Description:	设置通道活动检测时间间隔.网络库默认为10秒
  Input:  		unTime:时间，单位秒
  Output:      	   
  Return:  		ERROR_BASE_SUCCESS：表示成功。其它为错误，详见错误码定义。           
  Note:			通道建立后，在unTime时间内至少通讯一次，否则网络库认为该通道超时。将通知上层。		
********************************************************************************/
	virtual INT SetActiveTime( UINT16 unTime )=0;
	
/********************************************************************************
  Function:		SetMaxChannel
  Description:	设置通道数限制.网络库默认为200个。
  Input:  		unCount:最大通道数
  Output:      	   
  Return:  		ERROR_BASE_SUCCESS：表示成功。其它为错误，详见错误码定义。           
  Note:			超过最大通道数网络库自动关闭新接入的连接。		
********************************************************************************/
	virtual INT SetMaxChannel( UINT16 unCount )=0;
/********************************************************************************
  Function:		SetOnEventCallBack
  Description:	设置回调函数
  Input:  		pUserData:用户数据,可以为NULL。	OnEventCallBack：回调函数指针。
  Output:      	   
  Return:  		ERROR_BASE_SUCCESS：表示成功。其它为错误，详见错误码定义。           
  Note:				
********************************************************************************/
	virtual INT SetOnEventCallBack(void* pUserData, pOnEventCallBack OnEventCallBack)=0;
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
	virtual INT	SetNetBlockMode( BOOL bMode)=0;
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
	virtual	INT		SetSendBuf(INT iBufSize)=0;
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
	virtual	INT		SetRcvBuf(INT iBufSize)=0;
/********************************************************************************
  Function:		GetAllChannelNum
  Description:	获取当前网络库中所有的通道数目
  Input:  		
  Output:      	   
  Return:  		返回通道数目       
  Note:			这个函数貌似没有什么实际作用，只是我在测试时需要验证demo中的通道数目是否和网络库一直		
********************************************************************************/
	virtual	INT	GetAllChannelNum()=0;
/********************************************************************************
	Function:       InitLog
	Description:    初始化日志
	Input:			czPathName	日志路径
	Output:         
	Return:         ERROR_BASE_SUCCESS：表示成功。其它为错误，详见错误码定义。
	Note:			  	
********************************************************************************/
	virtual INT		InitLog(const char *czPathName)=0;
protected:
	INetService(void){};
public:
	virtual ~INetService(void){ };
};

/********************************************************************************
  Function:		CreateNetService
  Description:	创建CNetService对象的全局函数。创建成功后，必须调用InitNetService进行初始化
  Input:  		
  Output:      	   
  Return:  		创建成功返回INetService指针，失败返回NULL。       
  Note:					
********************************************************************************/


}

EXPORT_API INetService* CreateNetService();

#endif

