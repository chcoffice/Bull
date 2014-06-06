#ifndef BUFFER_MANAGER_DEF_H
#define BUFFER_MANAGER_DEF_H

#include "BufferObj.h"
#include "BufferArray.h"

#ifdef WINCE
	#define DEFAULT_LOCAL_BUFFER_ARRAY_SIZE 2
#else
	#define DEFAULT_LOCAL_BUFFER_ARRAY_SIZE 128
#endif


class CBufMgr
{
public:
	CBufMgr();
	virtual ~CBufMgr();

public:
	INT32 InitBufferMgr(CBufferArray* pGlobBufArray, 
						INT32 iLocalBufArraySize = DEFAULT_LOCAL_BUFFER_ARRAY_SIZE);
	INT32 PushBuffer(CBufferObj* pBufObj);
	CBufferObj* PopBuffer();
	void ClearBuffer();
	INT32 GetDataLen();
	INT32 GetData(char* pDstBuf);
protected:
	CBufferArray* m_pGlobBufferArray; //全局空闲缓存数组

	CBufferObj** m_pBufferArray;
	INT32 m_iBufferArraySize;
	INT32 m_iReadPos;
	INT32 m_iWritePos;
	CGSMutex m_MutexRelloc;
};

#endif