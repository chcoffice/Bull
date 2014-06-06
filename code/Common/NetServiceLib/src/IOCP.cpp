
#include "IOCP.h"

#if OPERATING_SYSTEM

#elif _WIN32

using namespace NetServiceLib;

CIOCP::CIOCP()
{
	m_CompletionPort = NULL;
}

CIOCP::~CIOCP()
{
	if (m_CompletionPort != NULL)
	{
		CloseHandle(m_CompletionPort);
	}

}

HANDLE CIOCP::CreateIoCompletionPortEx()
{
	if (m_CompletionPort == NULL)
	{
		//m_CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);//GetNumberOfProcessors());//应该设置为cpu*2+2个为佳
		m_CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, GetNumberOfProcessors() * 2 + 2);//应该设置为cpu*2+2个为佳
		return m_CompletionPort;
	}
	else
	{
		return NULL;	//已经创建
	}
	
}

//关联完成端口
HANDLE CIOCP::AddToIoCompletionPort(HANDLE hHandle, DWORD CompletionKey)
{
	if (m_CompletionPort == NULL)
	{
		return NULL;
	}

	HANDLE hDle = NULL;
	hDle = CreateIoCompletionPort(hHandle, m_CompletionPort, CompletionKey, 0);

	int n = GetLastError();

	if ( hDle == NULL)
	{
		return NULL;
	}
	
	return hDle;
}

/*
功能：查询完成端口是否有数据到达，有则返回收到的数据
参数：lpIOData
*/
BOOL CIOCP::GetQueuedCompletionStatusEx(LPDWORD		BytesTransferred,
										LPDWORD		PerHandleData,										
										LPOVERLAPPED* lpOverlapped,
										LPPER_IO_OPERATION_DATA* lpIOData)
{
	if (m_CompletionPort == NULL)
	{
		return FALSE;
	}


	BOOL	bRet = TRUE;
	bRet = GetQueuedCompletionStatus(m_CompletionPort,BytesTransferred, PerHandleData, lpOverlapped,INFINITE);

	
	// 检查成功的返回，这儿要注意使用这个宏CONTAINING_RECORD
	//LPPER_IO_OPERATION_DATA IOData;
	//IOData = (LPPER_IO_OPERATION_DATA)CONTAINING_RECORD(*lpOverlapped, PER_IO_OPERATION_DATA, Overlapped);
    *lpIOData = (LPPER_IO_OPERATION_DATA)CONTAINING_RECORD(*lpOverlapped, PER_IO_OPERATION_DATA, Overlapped);

	return bRet;

	 /*if (BytesTransferred == 0)
	 {
		 return FALSE;
	 }
	 
	 return TRUE;*/
}

BOOL CIOCP::PostQueuedCompletionStatusEx()
{
	for (int i=0; i < GetNumberOfProcessors()*2 + 2; i++)
	{
		PostQueuedCompletionStatus(m_CompletionPort, 0, (DWORD) NULL, NULL);
	}
	
	return TRUE;
}

typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
DWORD CIOCP::GetNumberOfProcessors()
{
#define Max_Accept_ThreadNum 10
	//固定用10条线程来接收数据
	return Max_Accept_ThreadNum;
	//SYSTEM_INFO si;
	//// Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
	//PGNSI pfnGNSI = (PGNSI) GetProcAddress(GetModuleHandle((LPCTSTR)"kernel32.dll"), "GetNativeSystemInfo");
	//if(pfnGNSI)
	//{
	//	pfnGNSI(&si);
	//}
	//else
	//{
	//	GetSystemInfo(&si);
	//}
	//return si.dwNumberOfProcessors;
}

#else//linux
//linux 编译此文件

#endif	//end #if _WIN32

