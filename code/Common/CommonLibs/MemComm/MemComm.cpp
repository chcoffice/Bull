#include "MemComm.h"

#include "MemCommLocal.h"
#include "MemCommProcess.h"

CMemComm::CMemComm(void)
{
	m_pCMemCom = NULL;
	
}

CMemComm::~CMemComm(void)
{
	if (m_pCMemCom != NULL)
	{
		delete m_pCMemCom;
	}
}
BOOL CMemComm::Create(UINT32 dwSize, BOOL bLock, BOOL bLocal, EnumOperType opType, string &strName)
{
	if (bLocal)
	{
		m_pCMemCom = new CMemCommLocal();
	}
	else
	{
		m_pCMemCom = new CMemCommProcess();
	}
	if (m_pCMemCom == NULL)
	{
		return FALSE;
	}

	return m_pCMemCom->Create(dwSize,bLock,opType,strName);

}

void CMemComm::Reset(void)
{
	return m_pCMemCom->Reset();
}
BOOL CMemComm::Free(void)
{
	return m_pCMemCom->Free();
}
INT32 CMemComm::Read(void *pBuf, UINT32 dwMaxLen, UINT32 *pRealLen, UINT32 dwMilliSeconds)
{
	return m_pCMemCom->Read(pBuf,dwMaxLen, pRealLen, dwMilliSeconds);
}
INT32 CMemComm::Write(void *pBuf, UINT32 dwLen, UINT32 dwMilliSeconds)
{
	return m_pCMemCom->Write(pBuf,dwLen,dwMilliSeconds);
}
INT32 CMemComm::WriteUrgent(void *pBuf, UINT32 dwLen, UINT32 dwMilliSeconds)
{
	return m_pCMemCom->WriteUrgent(pBuf,dwLen,dwMilliSeconds);
}

IMemComm *GetMemInstance()
{
	IMemComm *pMemComm = new CMemComm;

	if (pMemComm != NULL)
	{
		return pMemComm;
	}
	return NULL;
}

void ClearMemInstance(IMemComm *pMemComm)
{
	if (pMemComm != NULL)
	{
		delete pMemComm;
	}
}

