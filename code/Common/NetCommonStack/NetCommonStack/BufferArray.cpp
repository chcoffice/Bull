#include "GSCommon.h"
#include "BufferArray.h"
#include "Log.h"

CBufferArray::CBufferArray()
{
	m_BufArray = NULL;
	m_iBufArraySize = 0;
	m_iArrayGetPos = 0;
	m_iArraySetPos = 0;

	m_iBufferSize = DEFAULT_BUFFER_SIZE;
}

CBufferArray::~CBufferArray()
{
	DestroyArray();
}
INT32 CBufferArray::GetBuferSize()
{
	return m_iBufferSize;
}
INT32 CBufferArray::DestroyArray()
{
	if(NULL != m_BufArray)
	{
		FreeArrayBuffer(m_BufArray, m_iBufArraySize);
		delete m_BufArray;
		m_BufArray = NULL;
	}
	return 0;
}

void CBufferArray::FreeArrayBuffer(CBufferObj** pArray, INT32 iArraySize)
{
	INT32 i;
	CBufferObj* pObj;

	if(NULL == pArray || iArraySize <= 0)
	{
		return;
	}
	
	for(i = 0; i < iArraySize; i++)
	{
		pObj = pArray[i];
		if(NULL != pObj)
		{
			pArray[i] = NULL;
			delete pObj;
		}
	}
}

CBufferObj** CBufferArray::AllocArray(INT32 iArraySize)
{
	CBufferObj** pArray;

	if(iArraySize <= 0)
	{
		return NULL;
	}
	pArray = new CBufferObj*[iArraySize];
	if(NULL == pArray)
	{
        LOG2_FATAL("New object ptr fail. %d\n", iArraySize);
		return NULL;
	}
	memset(pArray, 0, sizeof(CBufferObj*) * iArraySize);
	return pArray;
}

INT32 CBufferArray::AllocArrayBuffer(CBufferObj** pArray, INT32 iArraySize, INT32 iBufSize)
{
	INT32 i;
	CBufferObj* pObj;
	INT32 iRet;

	if(NULL == pArray || iArraySize <= 0 || iBufSize <= 0)
	{
        LOG2_ERROR("Args invalid.\n");

		return -1;
	}
	for(i = 0; i < iArraySize; i++)
	{
		pObj = new CBufferObj;
		if(NULL == pObj)
		{
            LOG2_FATAL( "New CBufferObj object fail.\n"); 
			FreeArrayBuffer(pArray, iArraySize);
			return -2;
		}
		iRet = pObj->BufferObjInit(iBufSize);
		if(iRet < 0)
		{
            LOG2_ERROR("CBufferObj BufferObjInit fail ret:%d.\n", iRet); 
			FreeArrayBuffer(pArray, iArraySize);
			return -3;
		}
		pArray[i] = pObj;
	}
	return 0;
}

INT32 CBufferArray::InitBufferArray(INT32 iBufSize)
{
	INT32 iRet;

	if(m_BufArray)
	{
		return 0;
	}
	if(iBufSize < 0)
	{
		iBufSize = DEFAULT_BUFFER_SIZE;
	}
	m_iBufferSize = iBufSize;

	m_BufArray = AllocArray(DEFAULT_ARRAY_SIZE);
	if(NULL == m_BufArray)
	{
		DestroyArray();
		return -1;
	}
	m_iBufArraySize = DEFAULT_ARRAY_SIZE;

	iRet = AllocArrayBuffer(m_BufArray, m_iBufArraySize, m_iBufferSize);
	if(iRet < 0)
	{
 		DestroyArray();
		return -2;
	}
	return 0;
}

CBufferObj* CBufferArray::GetFreeBuffer()
{
	CBufferObj* pObj = NULL;
	CBufferObj** pArray;
	INT32 iExtSize;
	INT32 iRet;

	m_MutexArray.Lock();
	pObj = m_BufArray[m_iArrayGetPos];
	//printf("获取内存片 %d\n", m_iArrayGetPos);
	if(NULL == pObj) //缓冲区没有了，需要重新分配
	{
		m_MutexRellocArray.Lock();
        LOG2_DEBUG( "重新分配内存片数组\n"); 
		//////////////////////////////////////////////////////////////////////////
		//分配空闲内存数组
		iExtSize = m_iBufArraySize;
		pArray = AllocArray(m_iBufArraySize + DEFAULT_ARRAY_SIZE);
		if(NULL == pArray)
		{
			m_MutexRellocArray.Unlock();
			m_MutexArray.Unlock();
			return NULL;
		}
		m_iBufArraySize += DEFAULT_ARRAY_SIZE;
		delete m_BufArray;
		m_BufArray = pArray;

		//分配内存片
		iRet = AllocArrayBuffer(m_BufArray + iExtSize, DEFAULT_ARRAY_SIZE, m_iBufferSize);
		if(iRet < 0)
		{
            LOG2_ERROR("AllocArrayBuffer fail.\n");
			m_iBufArraySize = iExtSize;
			m_MutexRellocArray.Unlock();
			m_MutexArray.Unlock();
			return NULL;
		}
		m_iArrayGetPos = iExtSize;
		m_iArraySetPos = 0;
		pObj = m_BufArray[m_iArrayGetPos];

		m_MutexRellocArray.Unlock();
	}
	m_BufArray[m_iArrayGetPos] = NULL;
	m_iArrayGetPos++;
	if(m_iArrayGetPos >= m_iBufArraySize)
	{
		m_iArrayGetPos = 0;
	}
	m_MutexArray.Unlock();
	return pObj;
}

void CBufferArray::FreeBuffer(CBufferObj* pBufObj)
{
	if(NULL == pBufObj || NULL == m_BufArray)
	{
		return;
	}
	m_MutexRellocArray.Lock();
	if(NULL != m_BufArray[m_iArraySetPos])
	{
		delete pBufObj;
		m_MutexArray.Unlock();
		return;
	}
	//printf("回收内存片 %d\n", m_iArraySetPos);
	m_BufArray[m_iArraySetPos] = pBufObj;
	m_iArraySetPos++;
	if(m_iArraySetPos >= m_iBufArraySize)
	{
		m_iArraySetPos = 0;
	}
	m_MutexRellocArray.Unlock();
}