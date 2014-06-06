#include "GSCommon.h"
#include "SendTask.h"

CSendTask::CSendTask()
{
	m_pSendChn = NULL;
	m_pBufArray = NULL;
	m_iBufArraySize =TASK_DEFAULT_BUFFER_NUM;
	m_iBufCount = 0;

	m_iReadPos = 0;
	m_iWritePos = 0;
}

CSendTask::~CSendTask()
{
	if(NULL != m_pBufArray)
	{
		delete m_pBufArray;
		m_pBufArray = NULL;
	}
}

INT32 CSendTask::InitSendTask(INT32 iBufArraySize)
{
	if(iBufArraySize <= 0)
	{
		iBufArraySize = TASK_DEFAULT_BUFFER_NUM;
	}
	m_iBufArraySize = iBufArraySize;

	m_pBufArray = new CBufferObj*[m_iBufArraySize];
	if(NULL == m_pBufArray)
	{
		return -1;
	}
	memset(m_pBufArray, 0, sizeof(CBufferObj*) * m_iBufArraySize);
	return 0;
}

INT32 CSendTask::AddSendTask(CNetChannel* pChn, CBufferObj** pBufArray, INT32 iBufCount)
{
	INT32 iFreeCount = 0;
	INT32 iReadPos = 0;
	INT32 iCurCount = 0;
	INT32 iTotalCount = 0;
	INT32 iExtArraySize;
	CBufferObj** pArray;
	INT32 i;

	m_MutexAdd.Lock();
	iReadPos = m_iReadPos;
	if(m_pSendChn != NULL)
	{
		if(m_pSendChn != pChn)
		{
			m_MutexAdd.Unlock();
			return -1;
		}
	}
	else
	{
		m_pSendChn = pChn;
	}
	iCurCount = (m_iBufArraySize + m_iWritePos - iReadPos) % m_iBufArraySize;
	if(iCurCount >= TASK_MAX_BUFFER_NUM)
	{
		//缓冲区太多包了，发不出去
		m_MutexAdd.Unlock();
		return -2;
	}
	iFreeCount = m_iBufArraySize - iCurCount;
	if(iBufCount > iFreeCount)//缓冲区不够大了
	{
		iExtArraySize = m_iBufArraySize;
		//总共要分配当前缓冲加上要加入的大小
		iTotalCount = iBufCount + m_iBufArraySize;

		pArray = new CBufferObj*[iTotalCount];
		if(NULL == pArray)
		{
			return -3;
		}
		memset(pArray, 0, sizeof(CBufferObj*) * iTotalCount);

		m_MutexRelloc.Lock();
		iReadPos = m_iReadPos;
		for(i = 0; i < ((iExtArraySize + m_iWritePos - iReadPos) % iExtArraySize); i++)
		{
			pArray[i] = m_pBufArray[(i + iReadPos)%iExtArraySize];
		}
		delete m_pBufArray;
		m_pBufArray = pArray;
		m_iReadPos = 0; 
		m_iWritePos = i;
		m_iBufArraySize = iTotalCount;
		m_MutexRelloc.Unlock();
	}
	for(i = 0; i < iBufCount; i++)
	{
		m_pBufArray[m_iWritePos] = pBufArray[i];
		m_iWritePos++;
		if(m_iWritePos >= m_iBufArraySize)
		{ 
			m_iWritePos = 0;
		} 
	}
	m_MutexAdd.Unlock();

	return 0;
}