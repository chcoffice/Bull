#include "MemCommLocal.h"

CMemCommLocal::CMemCommLocal(void)
{
	m_pCGSMutex = NULL;
}

CMemCommLocal::~CMemCommLocal(void)
{
	
}

BOOL CMemCommLocal::Free(void)
{
	Lock();
	if (m_pHead != NULL)
	{
		free(m_pHead);
		m_pHead = NULL;
		m_pStruCookie = NULL;
	}

	if (m_pCGSMutex != NULL)
	{
		Unlock();
		delete m_pCGSMutex;
		m_pCGSMutex = NULL;
		return TRUE;
	}

	Unlock();
	return TRUE;
}


BOOL CMemCommLocal::Lock()
{

	if (m_pCGSMutex != NULL)
	{
		m_pCGSMutex->Lock();
	}

	return FALSE;

}
void CMemCommLocal::Unlock()
{

	if (m_pCGSMutex != NULL)
	{
		m_pCGSMutex->Unlock();
	}

}
BOOL CMemCommLocal::Open(UINT32 dwSize, BOOL bLock, string &strName)
{

	if (m_pHead == NULL)
	{
		return FALSE;
	}
	return TRUE;
}
BOOL CMemCommLocal::ReCreate(UINT32 dwSize, BOOL bLock, string &strName)
{
	if (m_pHead != NULL)
	{
		Free();
	}
	dwSize = (dwSize + MEMCOMM_ALIGN_BYTE_NUM - 1) / MEMCOMM_ALIGN_BYTE_NUM * MEMCOMM_ALIGN_BYTE_NUM  + MEMCOMM_ALIGN_BYTE_NUM * 2;//其中1单位用于存放长度，1单位用于区分缓冲区是否为空
	dwSize = dwSize + sizeof(StruMemCookie);
	m_pHead = malloc(dwSize);//创建缓冲区

	if (m_pHead == NULL)
	{
		return FALSE;
	}

	if (bLock)//生成锁
	{
		if (m_pCGSMutex == NULL)
		{
			m_pCGSMutex = new CGSMutex;
			if (m_pCGSMutex == NULL)
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

	return TRUE;
}

