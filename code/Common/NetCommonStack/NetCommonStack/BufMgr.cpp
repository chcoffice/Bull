#include "GSCommon.h"
#include "BufMgr.h"
#include "Log.h"

CBufMgr::CBufMgr()
{
	m_pGlobBufferArray = NULL; //全局空闲缓存数组
	m_pBufferArray = NULL;
	m_iBufferArraySize = 0;
	m_iWritePos = 0;
	m_iReadPos = 0;
}

CBufMgr::~CBufMgr()
{
	INT32 i;
	CBufferObj* pObj;
	if(m_pBufferArray)
	{
		for(i = 0; i < m_iBufferArraySize; i++)
		{
			pObj = m_pBufferArray[i];
			if(m_pGlobBufferArray && pObj)
			{
				m_pGlobBufferArray->FreeBuffer(pObj);
			}
		}
		delete m_pBufferArray;
		m_pBufferArray = NULL;
	}
}

INT32 CBufMgr::InitBufferMgr(CBufferArray *pGlobBufArray, INT32 iLocalBufArraySize)
{
	m_pGlobBufferArray = pGlobBufArray;

	if(iLocalBufArraySize <= 0)
	{
		iLocalBufArraySize = DEFAULT_LOCAL_BUFFER_ARRAY_SIZE;
	}
	m_iBufferArraySize = iLocalBufArraySize;

	m_pBufferArray = new CBufferObj*[m_iBufferArraySize];
	if(NULL == m_pBufferArray)
	{
        LOG2_FATAL( "New object ptr size:%d fail.\n", m_iBufferArraySize);
		return -1;
	}
	memset(m_pBufferArray, 0, sizeof(CBufferObj*) * m_iBufferArraySize);
	return 0;
}

INT32 CBufMgr::PushBuffer(CBufferObj *pBufObj)
{
	CBufferObj** pArray;
	CBufferObj* pBufPos;
	INT32 iExtBufSize = 0;
	INT32 i;

	if(NULL == pBufObj || NULL == m_pBufferArray)
	{
        LOG2_ERROR("Args invalid.\n");
		return -1;
	}

	pBufPos = m_pBufferArray[m_iWritePos];
	if(NULL != pBufPos) // 需要重新分配
	{
		m_MutexRelloc.Lock();
		iExtBufSize = m_iBufferArraySize;
		pArray = new CBufferObj*[m_iBufferArraySize + DEFAULT_LOCAL_BUFFER_ARRAY_SIZE];
		if(NULL == pArray)
		{
            LOG2_FATAL("New object ptr size:%d fail.\n", m_iBufferArraySize + DEFAULT_LOCAL_BUFFER_ARRAY_SIZE);

			m_MutexRelloc.Unlock();
			return -2;
		}
		m_iBufferArraySize += DEFAULT_LOCAL_BUFFER_ARRAY_SIZE;
		memset(pArray, 0, sizeof(CBufferObj*) * m_iBufferArraySize);

		for(i = 0; i < iExtBufSize; i++)
		{
			pArray[i] = m_pBufferArray[(m_iReadPos + i) % iExtBufSize];
		}
		delete m_pBufferArray;
		m_pBufferArray = pArray;
		m_iReadPos = 0;
		m_iWritePos = iExtBufSize;
		m_MutexRelloc.Unlock();
	}
	m_pBufferArray[m_iWritePos] = pBufObj;
	m_iWritePos++;
	if(m_iWritePos >= m_iBufferArraySize)
	{
		m_iWritePos = 0;
	}
	return 0;
}

CBufferObj* CBufMgr::PopBuffer()
{
	CBufferObj* pBufObj;

	m_MutexRelloc.Lock();
	pBufObj = m_pBufferArray[m_iReadPos];
	if(NULL == pBufObj)
	{
        LOG2_INFO("Cur %d is NULL.\n", m_iReadPos );
		m_MutexRelloc.Unlock();
		return NULL;
	}
	m_pBufferArray[m_iReadPos] = NULL;
	m_iReadPos++;
	if(m_iReadPos >= m_iBufferArraySize)
	{
		m_iReadPos = 0;
	}
	m_MutexRelloc.Unlock();
	return pBufObj;
}

void CBufMgr::ClearBuffer()
{
	INT32 i;
	CBufferObj* pBufObj;
	if(NULL != m_pGlobBufferArray)
	{
		for(i = 0; i < m_iBufferArraySize; i++)
		{
			pBufObj = m_pBufferArray[i];
			if(NULL != pBufObj)
			{
				m_pGlobBufferArray->FreeBuffer(pBufObj);
				m_pBufferArray[i] = NULL;
			}
		}
	}
	m_iReadPos = m_iWritePos = 0;
}

INT32 CBufMgr::GetDataLen()
{
	INT32 i;
	CBufferObj* pBufObj;
	INT32 iTotalLen = 0;

	for(i = 0; i < m_iBufferArraySize; i++)
	{
		pBufObj = m_pBufferArray[i];
		if(NULL != pBufObj)
		{
			iTotalLen += pBufObj->GetDataSize();
		}
	}
	return iTotalLen;
}

INT32 CBufMgr::GetData(char* pDstBuf)
{
	INT32 i;
	CBufferObj* pBufObj;
	INT32 iTotalLen = 0;

	for(i = 0; i < m_iBufferArraySize; i++)
	{
		pBufObj = m_pBufferArray[i];
		if(NULL != pBufObj)
		{
			memcpy(pDstBuf + iTotalLen, pBufObj->GetData(), pBufObj->GetDataSize());
			iTotalLen += pBufObj->GetDataSize();
		}
	}
	return iTotalLen;
}