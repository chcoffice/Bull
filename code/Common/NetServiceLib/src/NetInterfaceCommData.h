#if !defined (NetInterfaceCommData_DEF_H)
#define NetInterfaceCommData_DEF_H
/********************************************************************
	Copyright (C), 1997-2010, gosun Tech. Co., Ltd.
	FileName: 		NetInterfaceCommData.h
	Author:        	陈聪
	Version :       v1.0   
	Date:			2010/05/12
	Description:     网络接口的公共数据 ,所有通道都存储在此，以及公共参数

*********************************************************************/


#include "NetServiceDataType.h"

#include "SocketChanel.h"

namespace NetServiceLib
{


typedef vector<CSocketChannel*>		VectorChannelPoint;

class CNetInterfaceCommData
{
public:
	CNetInterfaceCommData(void);
	virtual ~CNetInterfaceCommData(void);
	void	Init();
	INT		OnEventCallBack(ISocketChannel* pSocketChnl,  enumNetEventType enumType, void* pData, UINT32 uiDataSize);	
public:
	//设置是否重连
	INT		SetReConnect(bool bReConnect){m_bReConnect = bReConnect;return ERROR_BASE_SUCCESS;};
	//保存通道至队列
	INT		SaveSocketChannel(CSocketChannel* pSocketChannel);//保存通道
	//从队列中删除通道，和释放通道资源不是一回事
	INT		DeleteSocketChannel( CSocketChannel* pSocketChannel );
	// 获取存在的通道,不存在返回NULL
	CSocketChannel*		GetExistSocketChannel( DWORD dwRemoteIP, UINT16 unRemotePort );
	// 根据通道指针判断通道是否存在。存在返回TRUE，不存在返回FALSE
	BOOL	IfExistSocketChannel( CSocketChannel* pclsSocketChannel );
	//设置用户数据和回调函数
	INT		SetOnEventCallBack(void* pUserData, pOnEventCallBack OnEventCallBack);
	//设置是否支持消息缓冲
	INT		SetMsgBufFlag(bool bMsgBufFlag){m_bMsgBufFlag = bMsgBufFlag; return ERROR_BASE_SUCCESS;};
	//设置连接活动监测时间
	INT		SetActiveTime(UINT16 unActiveTime){m_unActiveTime = unActiveTime; return ERROR_BASE_SUCCESS;};
	//设置最大通道数
	INT		SetMaxChannel(UINT16 unMaxChannel);
	//获取所有通道数目
	INT		GetAllChannelNum();
	//存入故障通道队列
	INT		SaveToFaultVector(CSocketChannel* pSocketChannel);
	//从故障通道队列中删除通道，和释放通道资源不是一回事
	INT		DeleteoFromFaultVector( CSocketChannel* pSocketChannel );
	// 设置阻塞或非阻塞模式
	void	SetBlockMode( BOOL bMode){  m_bBlockMode = bMode; };
	// 获取m_bBlockMode值
	BOOL	GetBlockMode(){ return m_bBlockMode; };
	// 设置socket发送缓冲区大小
	INT		SetSendBuf(INT iBufSize);
	// 设置socket接收缓冲区大小
	INT		SetRcvBuf(INT iBufSize);

	ILogLibrary* GetLogInstancePtr(){ return m_clsLogPtr; };

	// 设置日志库指针，对于同时初始化多个服务端、客户端的情况
	void SetLogInstancePtr(ILogLibrary* pLog);

	// 设置日志路径
	void	SetLogPath( const char*	czPathName );


public: 
	//活动检测
	INT		TestActiveTime();
	//最大连接数检查
	bool	TestMaxChannelCount();
protected:
	//true:重连 false：无需重连   默认重连
	bool			m_bReConnect;	

	//消息缓冲 true：使用消息缓冲 false不使用 默认不使用
	bool			m_bMsgBufFlag;	

	//通道最大的连续未通讯时间，单位秒
	UINT16			m_unActiveTime;	

	//最大连接数 
	UINT16			m_unMaxChannel;		

	//模式	TRUE: 非阻塞	FALSE: 非阻塞 默认FALSE
	BOOL			m_bBlockMode;

	// socket发送缓冲区大小 如果不设置 则采用系统的默认值
	UINT16			m_unSocketSendBuf;

	// socket接收缓冲区大小 如果不设置 则采用系统的默认值
	UINT16			m_unSocketRcvBuf;
	
	//回调
	pOnEventCallBack	m_pfnOnEventCallBack;//回调指针

	//用户传入，回调时传出给用户
	void*				m_pUserData;		
	//通道队列 存放所有通道
	VectorChannelPoint		m_vectorSocketChannel;	

	//读写锁 该锁针对m_vectorSocketChannel队列而使用。
	CGSWRMutex			m_GSWRMutexVectorChannel;	
	CGSMutex			m_GSMutexVerChnl;

	//故障通道队列	存放有故障的通道队列，如中断、关闭、超时等等
	VectorChannelPoint		m_vectorFaultChannel;	

	//读写锁 该锁针对m_vectorFaultChannel队列而使用。
	CGSWRMutex			m_GSWRMutexVecFaultChannel;	
	CGSMutex			m_GSMutexVerFaultChnl;

	// 日志指针
	ILogLibrary*		m_clsLogPtr;


};

}

#endif

