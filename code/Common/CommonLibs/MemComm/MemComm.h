#ifndef MEMCOMM_H_
#define MEMCOMM_H_


#include "IMemComm.h"
#include "MemCommBase.h"



class CMemComm :
	public IMemComm
{
public:
	CMemComm(void);
	virtual ~CMemComm(void);
	BOOL Create(UINT32 dwSize, BOOL bLock, BOOL bLocal, EnumOperType opType, string &strName);
	void Reset(void);
	BOOL Free(void);
	INT32 Read(void *pBuf, UINT32 dwMaxLen, UINT32 *pRealLen, UINT32 dwMilliSeconds);
	INT32 Write(void *pBuf, UINT32 dwLen, UINT32 dwMilliSeconds);
	INT32 WriteUrgent(void *pBuf, UINT32 dwLen, UINT32 dwMilliSeconds);
private:
	CMemCommBase* m_pCMemCom;
};

#endif


