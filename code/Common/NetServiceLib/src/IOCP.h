#if !defined (IOCP_DEF_H)
#define IOCP_DEF_H

#include "NetServiceDataType.h"

namespace NetServiceLib
{
#ifndef _LINUX
typedef struct 
{
   OVERLAPPED Overlapped;
   WSABUF DataBuf; 
   char	 Buffer[DATA_BUFSIZE];
   INT	OptionType;					//操作类型
   sockaddr_in	struSockAddrFrom;	//UDP接收数据时的地址
   INT			iAddrFromLen;		//UDP接收数据时的地址的长度

} PER_IO_OPERATION_DATA, * LPPER_IO_OPERATION_DATA;

#endif

#if OPERATING_SYSTEM
#elif _WIN32
//
//	定义操作句柄结构，
//  该结构用于和IOCP关联起来
//
typedef struct
{
	void*	pUserData;		//在这里我让其指向通道指针
   
} PER_HANDLE_DATA,  *LPPER_HANDLE_DATA;

class CIOCP  
{
public:
	CIOCP();
	virtual ~CIOCP();
public:
	BOOL PostQueuedCompletionStatusEx();
	//创建完成端口
	HANDLE	CreateIoCompletionPortEx();
	//关联完成端口
	HANDLE	AddToIoCompletionPort(HANDLE hHandle, DWORD CompletionKey);
	//查询完成端口是否有数据到达
	BOOL	GetQueuedCompletionStatusEx(LPDWORD		BytesTransferred,
										LPDWORD		PerHandleData,										
										LPOVERLAPPED* lpOverlapped,
										LPPER_IO_OPERATION_DATA* lpIOData
										);
	HANDLE	GetCompletionPort(){ return m_CompletionPort;}
	DWORD	GetNumberOfProcessors();

protected:
	HANDLE					m_CompletionPort;						// 完全端口句柄
};

#else//linux
//linux 编译此文件

#endif	//end #if _WIN32

}

#endif


