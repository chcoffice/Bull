#include "GSCommon.h"
#include "BufferObj.h"
#include "Log.h"

CBufferObj::CBufferObj()
{
	m_pObjBuf = NULL;
	m_iBufSize = 0;
	m_iContentSize = 0;
	m_iDataValid = BUFFER_DATA_INVALID;
	m_pUserData = NULL;
}

CBufferObj::~CBufferObj()
{
	if(m_pObjBuf)
	{
		free(m_pObjBuf);
		m_pObjBuf = NULL;
	}
}

INT32 CBufferObj::BufferObjInit(INT32 iBufferSize)
{
	if(iBufferSize <= 0)
	{
		return -1;
	}
	if(m_pObjBuf)
	{
		free(m_pObjBuf);
		m_pObjBuf = NULL;
	}

	m_pObjBuf = (char*)malloc(iBufferSize);
	if(NULL == m_pObjBuf)
	{
        LOG2_FATAL( "Malloc size:%d fail.\n", iBufferSize);
		return -2;
	}
	m_iBufSize = iBufferSize;
	memset(m_pObjBuf, 0, iBufferSize);
	return 0;
}

INT32 CBufferObj::GetBufSize()
{
	return m_iBufSize;
}
INT32 CBufferObj::GetDataSize()
{
	return m_iContentSize;
}

char* CBufferObj::GetData()
{
	return m_pObjBuf;
}

void* CBufferObj::GetUserData()
{
	return m_pUserData;
}

void CBufferObj::SetDataSize(INT32 iDataSize)
{
	m_iContentSize = iDataSize;
}

INT32 CBufferObj::SetData(void *pUserData, char *pData, INT32 iDataLen)
{
	if(NULL == pData || iDataLen <= 0)
	{
        LOG2_ERROR( "Args invalid.\n");
		return -1;
	}
	m_pUserData = pUserData;
	if(iDataLen > m_iBufSize)
	{
        LOG2_ERROR( "iDataLen(%d) > m_iBufSiz(%d).\n", iDataLen, m_iBufSize);
		return -2;
	}
	if(NULL == m_pObjBuf)
	{
        LOG2_ERROR("m_pObjBuf is NULL.\n");

		return -3;
	}
	memcpy(m_pObjBuf, pData, iDataLen);
	m_iContentSize = iDataLen;
	return 0;
}

INT32 CBufferObj::GetBufValid()
{
	return m_iDataValid;
}

void CBufferObj::SetBufValid(INT32 iValid)
{
	if(iValid < BUFFER_DATA_INVALID || iValid > BUFFER_DATA_VALID)
	{
		m_iDataValid = BUFFER_DATA_INVALID;
		return;
	}
	m_iDataValid = iValid;
}

INT32 CBufferObj::CopyBufObj(CBufferObj *pObj)
{
	if(NULL == pObj)
	{
        LOG2_ERROR( "Args invalid.\n");

		return -1;
	}
	if(pObj->GetDataSize() > m_iBufSize)
	{
        LOG2_ERROR("GetDataSize(%d) > m_iBufSiz(%d).\n", pObj->GetDataSize() , m_iBufSize);

		return -2;
	}

	SetData(pObj->GetUserData(), pObj->GetData(), pObj->GetDataSize());
	SetBufValid(pObj->GetBufValid());
	
	return 0;
}