#ifndef MEMCOMMLOCAL_H_
#define MEMCOMMLOCAL_H_

#include "MemCommBase.h"

class CMemCommLocal :
	public CMemCommBase
{
public:
	CGSMutex *m_pCGSMutex ;//±¾µØËø
	CMemCommLocal(void);
	virtual ~CMemCommLocal(void);
public:

	BOOL Free(void);

protected:
	 BOOL Lock();
	 void Unlock();
     BOOL Open(UINT32 dwSize, BOOL bLock, string &strName);
	 BOOL ReCreate(UINT32 dwSize, BOOL bLock, string &strName);
};

#endif


