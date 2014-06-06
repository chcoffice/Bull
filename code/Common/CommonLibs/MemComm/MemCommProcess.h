#ifndef MEMCOMMPROCESS_H_
#define MEMCOMMPROCESS_H_

#include "MemCommBase.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef _LINUX
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#endif

class CMemCommProcess :
	public CMemCommBase
{
public:
	CMemCommProcess(void);
	virtual ~CMemCommProcess(void);
public:
	CGSProcessMutex *m_pCGSProcessMutex;
#ifdef _WIN32
	HANDLE m_pMemHandle;	//共享内存的句柄，当m_Local为FALSE时使用
#endif

#ifdef _LINUX
	INT m_iShareMemID;
#endif


public:

	BOOL Free(void);
protected:
	BOOL Lock();
	void Unlock();
	BOOL Open(UINT32 dwSize, BOOL bLock, string &strName);
	BOOL ReCreate(UINT32 dwSize, BOOL bLock, string &strName);
};

#endif


