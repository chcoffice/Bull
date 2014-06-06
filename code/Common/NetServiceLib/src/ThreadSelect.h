#if !defined (ThreadSelect_DEF_H)
#define ThreadSelect_DEF_H

/***************************************************************************************************
	Copyright (C), 1997-2010, gosun Tech. Co., Ltd.
	FileName: 		ThreadSelect.h
	Author:        	陈聪
	Version :       v1.0   
	Date:			2010/11/22
	Description:    select模型，可以适用于win mobile 、win32、linux系统

****************************************************************************************************/
#include "CommunicationManager.h"
#include "ThreadPoolModule.h"

#if OPERATING_SYSTEM


namespace NetServiceLib
{

	class CThreadDealNetEvent :
		public CCommunicationManager
	{
	public:
		CThreadDealNetEvent(void){};
		virtual ~CThreadDealNetEvent(void){};
		//处理接收数据
		INT ThreadAcceptData(enumThreadEventType enumEvent, void* pObject);			
		//线程执行函数 用于轮询linux的epoll事件
		INT  SelectEvent(enumThreadEventType enumEvent, void* pObject);
		
	private:
		// 增加事件响应 继承自CCommunicationManager
		virtual	void OnEventModel( CSocketChannel* pclsSocketChannel );
	public:
		CThreadPoolModule			m_clsThreadPool;
		// 信号量
		CGSCond						m_GSCond;

	};

}

#endif


#endif


