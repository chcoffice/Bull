#ifndef SEND_TASK_DEF_H
#define SEND_TASK_DEF_H

#include "NetChannel.h"
#include "BufferObj.h"

#define TASK_DEFAULT_BUFFER_NUM 256
//如果有500*1024个任务都无法完成，则认为队列阻塞了
#define TASK_MAX_BUFFER_NUM (1024*500)

class CSendTask
{
public:
	CSendTask();
	virtual ~CSendTask();

	INT32 InitSendTask(INT32 iBufArraySize);
	INT32 AddSendTask(CNetChannel* pChn, CBufferObj** pBufArray, INT32 iBufCount);
	INT32 GetSendTask();
	
protected:
	CNetChannel* m_pSendChn;
	CBufferObj** m_pBufArray;
	INT32 m_iBufArraySize;
	INT32 m_iBufCount;

	INT32 m_iWritePos;
	INT32 m_iReadPos;

	CGSMutex m_MutexAdd;
	CGSMutex m_MutexRelloc;
};


#endif