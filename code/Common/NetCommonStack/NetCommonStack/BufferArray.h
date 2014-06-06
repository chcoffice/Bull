#ifndef BUFFER_ARRAY_DEF_H
#define BUFFER_ARRAY_DEF_H

#include "BufferObj.h"

#define DEFAULT_ARRAY_SIZE 1024
#define DEFAULT_BUFFER_SIZE 1500

class CBufferArray
{
public:
	CBufferArray();
	virtual ~CBufferArray();

	INT32 InitBufferArray(INT32 iBufSize);
	INT32 DestroyArray();
	CBufferObj* GetFreeBuffer();
	void FreeBuffer(CBufferObj* pBufObj);
	INT32 GetBuferSize();

protected:
	CBufferObj** AllocArray(INT32 iArraySize);
	INT32 AllocArrayBuffer(CBufferObj** pArray, INT32 iArraySize, INT32 iBufSize);
	void FreeArrayBuffer(CBufferObj** pArray, INT32 iArraySize);

protected:
	CBufferObj** m_BufArray;
	INT32 m_iBufArraySize;
	INT32 m_iArrayGetPos;
	INT32 m_iArraySetPos;
	
	INT32 m_iBufferSize;

	CGSMutex m_MutexArray;
	CGSMutex m_MutexRellocArray;

};

#endif