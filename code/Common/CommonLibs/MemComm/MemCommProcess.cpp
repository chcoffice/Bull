#include "MemCommProcess.h"

#ifdef _LINUX
#include <errno.h>
extern int errno;
#endif

CMemCommProcess::CMemCommProcess(void)
{

	m_pCGSProcessMutex = NULL;
#ifdef _WIN32
	m_pMemHandle = NULL;
#endif

#ifdef _LINUX
	m_iShareMemID = 0;
#endif

}

CMemCommProcess::~CMemCommProcess(void)
{
	
}


BOOL CMemCommProcess::Free(void)
{

	Lock();

#ifdef _WIN32
	if (m_pHead != NULL)//内存句柄
	{
		UnmapViewOfFile(m_pHead);
		m_pHead = NULL;
		m_pStruCookie = NULL;
	}
	if (m_pMemHandle != NULL)
	{
		CloseHandle(m_pMemHandle);
		m_pMemHandle = NULL;
	}

	m_pMemHandle = NULL;
#endif

#ifdef _LINUX

	if (m_pHead != NULL)
	{
		/*if(shmdt(m_pHead)==-1)//仅仅是删除进程与共享内存的的联系
		{
			Unlock();
			return FALSE;
		}*/
		m_pHead = NULL;
	}

    if(shmctl(m_iShareMemID,IPC_RMID,0)==-1)//从系统中删除共享内存
	{
		Unlock();
		return FALSE;
	}
	m_iShareMemID = 0;

#endif

	if (m_pCGSProcessMutex != NULL)
	{
		Unlock();
		delete m_pCGSProcessMutex;
		m_pCGSProcessMutex = NULL;
		return TRUE;
	}
	Unlock();
	return TRUE;
}

BOOL CMemCommProcess::Lock()
{

	if (m_pStruCookie != NULL)
	{
		if (m_pStruCookie->uiLock == 1)
		{
			if (m_pCGSProcessMutex != NULL)
			{
				return m_pCGSProcessMutex->LockProcess();
			}
		}
	}
	return TRUE;

}
void CMemCommProcess::Unlock()
{
	if (m_pStruCookie != NULL)
	{
		if (m_pStruCookie->uiLock == 1)
		{
			if (m_pCGSProcessMutex != NULL)
			{
				m_pCGSProcessMutex->UnlockProcess();
			}
		}
	}

}
BOOL CMemCommProcess::Open(UINT32 dwSize, BOOL bLock, string &strName)
{

	dwSize = (dwSize + MEMCOMM_ALIGN_BYTE_NUM -1) / MEMCOMM_ALIGN_BYTE_NUM * MEMCOMM_ALIGN_BYTE_NUM + MEMCOMM_ALIGN_BYTE_NUM;
	dwSize = sizeof(StruMemCookie) + dwSize;

#ifdef _WIN32
	m_pMemHandle = CreateFileMapping(HANDLE(-1), NULL, PAGE_READWRITE, 0, dwSize, (LPCWSTR)strName.c_str());
	if (m_pMemHandle == NULL)
		return FALSE;

	if (GetLastError() != ERROR_ALREADY_EXISTS)//不存在
	{
		CloseHandle(m_pMemHandle);
		m_pMemHandle = NULL;
		return FALSE;
	}

	//映射共享内存
	m_pHead = MapViewOfFile(m_pMemHandle, FILE_MAP_READ|FILE_MAP_WRITE, 0, 0, dwSize);//创建视
	CloseHandle(m_pMemHandle);
	if (m_pHead == NULL)
	{
		return FALSE;
	}

#endif


#ifdef _LINUX
	
	//获取已经存在共享内存ID
	m_iShareMemID = shmget((key_t)(*(int *)strName.c_str()),0,0);
	if(m_iShareMemID == -1)//表明是重新创建的
	{
		m_pHead = NULL;
		Free();
		return FALSE;
	}

	//获取共享内存地址
	m_pHead = shmat(m_iShareMemID, (void *)0, 0);
	if (m_pHead == (void *) -1)
	{
		m_pHead = NULL;
		Free();
		return FALSE;
	}

#endif

	m_pStruCookie = (StruMemCookie *)m_pHead;

	//创建锁
	if (m_pStruCookie->uiLock == 1)
	{

		if (m_pCGSProcessMutex == NULL)
		{
			m_pCGSProcessMutex = new CGSProcessMutex(m_pStruCookie->chMutexName);
			if (m_pCGSProcessMutex == NULL)
			{

				Free();
				return FALSE;
			}

		}
	}

	return TRUE;
}
BOOL CMemCommProcess::ReCreate(UINT32 dwSize, BOOL bLock, string &strName)
{
	//创建共享内存
	UINT32 uiAlignSize;
	uiAlignSize = (dwSize + MEMCOMM_ALIGN_BYTE_NUM -1) / MEMCOMM_ALIGN_BYTE_NUM * MEMCOMM_ALIGN_BYTE_NUM + MEMCOMM_ALIGN_BYTE_NUM * 2;//其中1单位用于存放长度，1单位用于区分缓冲区是否为空
	dwSize = sizeof(StruMemCookie) + dwSize;

#ifdef _WIN32
	m_pMemHandle = CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, dwSize, (LPCWSTR)strName.c_str());
	if (m_pMemHandle == NULL)
	{
		return FALSE;
	}

	//获取共享内存地址
	m_pHead = MapViewOfFile(m_pMemHandle, FILE_MAP_READ|FILE_MAP_WRITE, 0, 0, dwSize);

	if (m_pHead == NULL)
	{
		CloseHandle(m_pMemHandle);
		m_pMemHandle = NULL;
		return FALSE;
	}
#endif

#ifdef _LINUX
	//创建共享内存
	
	m_iShareMemID = shmget((key_t)(*(int *)strName.c_str()), dwSize, 0666|IPC_CREAT|IPC_EXCL);//只有当共享内存不存在时，重新创建成功，才会返回成功，即保证是重新创建。
	if (m_iShareMemID == -1)
	{
		if (errno != EEXIST)//如果已经存在，删除掉，重新创建
		{
			return FALSE;
		}
		shmctl(m_iShareMemID,IPC_RMID,0);
		m_iShareMemID = shmget((key_t)(*(int *)strName.c_str()), dwSize, 0666|IPC_CREAT|IPC_EXCL);
		if (m_iShareMemID == -1)
		{
			return FALSE;
		}	
	}

	//获取共享内存地址
	m_pHead = shmat(m_iShareMemID,(void *)0,0);

	if (m_pHead == (void *) -1)
	{
		m_pHead = NULL;
		Free();
		return FALSE;
	}

#endif
    printf("Recreate Mem!\n");
	//  创建锁
	if (bLock)
	{
		if (m_pCGSProcessMutex == NULL)
		{
			strName += "ProcessMutex";
			m_pCGSProcessMutex = new CGSProcessMutex(strName.c_str());//TODO：根据事件名产生一个进程锁
			if (m_pCGSProcessMutex == NULL)
			{
				Free();
				return FALSE;
			}

		}

	}

	//写入内存cookie

	m_pStruCookie = (StruMemCookie *)m_pHead;

	m_pStruCookie->uiMemId = MEMCOMM_SHARE_ID;
	m_pStruCookie->uiOffSet = sizeof(StruMemCookie);
	m_pStruCookie->uiTotalSize = (dwSize - sizeof(StruMemCookie))/MEMCOMM_ALIGN_BYTE_NUM;
	m_pStruCookie->uiWritePos = 0;
	m_pStruCookie->uiReadPos = 0;
	if (bLock)
	{
		m_pStruCookie->uiLock = 1;
	}
	else
	{
		m_pStruCookie->uiLock = 0;
	}

	strcpy(m_pStruCookie->chMutexName,strName.c_str()); 
	strcpy(m_pStruCookie->chEventName,strName.c_str());
	return TRUE;
}

