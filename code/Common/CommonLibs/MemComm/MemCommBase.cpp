#include "MemCommBase.h"

CMemCommBase::CMemCommBase(void)
{
	m_pStruCookie = NULL;
	m_pHead = NULL;
}

CMemCommBase::~CMemCommBase(void)
{

	if (m_pStruCookie != NULL)
	{
		m_pStruCookie = NULL;
	}
}
void CMemCommBase::Reset(void)
{
	Lock();
	if (m_pStruCookie != NULL)
	{
		m_pStruCookie->uiWritePos = 0;
		m_pStruCookie->uiReadPos = 0;

	}
	Unlock();
}
BOOL CMemCommBase::Create(UINT32 dwSize, BOOL bLock, EnumOperType opType, string &strName)
{
	switch(opType)
	{
	case MEM_COMM_OPEN://打开
		{
			return Open(dwSize,bLock,strName);
		}
	case MEM_COMM_CREATE://创建
		{
			return ReCreate(dwSize,bLock,strName);
		}
	case MEM_COMM_OPEN_CREATE://尝试打开后创建
		{
			if (Open(dwSize,bLock,strName) == TRUE)	
			{
				return TRUE;
			}
			return ReCreate(dwSize,bLock,strName);
		}
	default:
		return FALSE;
	}
	return FALSE;

}

INT32 CMemCommBase::Read(void *pBuf, UINT32 dwMaxLen, UINT32 *pRealLen, UINT32 dwMilliSeconds)
{
	if (m_pStruCookie == NULL || pBuf == NULL)//无合法缓冲区
	{
		return ERROR_MEMCOMM_BUFFER_NOT_EXIST;
	}

	if (m_pStruCookie->uiMemId != MEMCOMM_SHARE_ID)//缓冲区标识不对
	{
		return ERROR_MEMCOMM_BUFFERID_NOT_FIT;
	}

	Lock();
	if (m_pStruCookie->uiReadPos == m_pStruCookie->uiWritePos)//缓冲区无数据，TODO:等待超时
	{
		Unlock();
		return ERROR_MEMCOMM_BUFFER_IS_EMPTY;
	}

	char *pMemPos = (char *)(m_pHead) + m_pStruCookie->uiOffSet;
	UINT32 uiReadSize = 0;

	UINT32 dwLen = *(UINT32 *)(pMemPos + m_pStruCookie->uiReadPos * MEMCOMM_ALIGN_BYTE_NUM); //读取长度字节
	
	if (dwMaxLen < dwLen)//pBuf大小不够
	{
		*pRealLen = 0;
		Unlock();
		return ERROR_MEMCOMM_BUFFER_NOT_ENOUGH;
	}

	m_pStruCookie->uiReadPos++;

	UINT dwRightSize = m_pStruCookie->uiTotalSize - m_pStruCookie->uiReadPos;//计算缓冲区读指针右侧空间大小

	if (dwRightSize * MEMCOMM_ALIGN_BYTE_NUM < dwLen)
	{
		uiReadSize = dwRightSize * MEMCOMM_ALIGN_BYTE_NUM;
		memcpy(pBuf,pMemPos + m_pStruCookie->uiReadPos * MEMCOMM_ALIGN_BYTE_NUM,uiReadSize);//拷贝数据前半段
		m_pStruCookie->uiReadPos = 0;
		uiReadSize = dwLen - uiReadSize;
		memcpy((char *)pBuf + dwRightSize * MEMCOMM_ALIGN_BYTE_NUM,pMemPos,uiReadSize);//拷贝数据后半段
		m_pStruCookie->uiReadPos =(uiReadSize + MEMCOMM_ALIGN_BYTE_NUM - 1) / MEMCOMM_ALIGN_BYTE_NUM;
	}
	else
	{
		uiReadSize = dwLen;
		memcpy(pBuf, pMemPos + m_pStruCookie->uiReadPos * MEMCOMM_ALIGN_BYTE_NUM,uiReadSize);
		m_pStruCookie->uiReadPos = m_pStruCookie->uiReadPos + (uiReadSize + MEMCOMM_ALIGN_BYTE_NUM - 1) / MEMCOMM_ALIGN_BYTE_NUM;
		if (m_pStruCookie->uiReadPos == m_pStruCookie->uiTotalSize)
		{
			m_pStruCookie->uiReadPos = 0;
		}
	}

	*pRealLen = dwLen;
	Unlock();

	return ERROR_MEMCOMM_OPER_SUCCESS;
}
INT32 CMemCommBase::Write(void *pBuf, UINT32 dwLen, UINT32 dwMilliSeconds)
{
	if (m_pStruCookie == NULL || pBuf == NULL)//无合法缓冲区
	{
		return ERROR_MEMCOMM_BUFFER_NOT_EXIST;
	}

	if (m_pStruCookie->uiMemId != MEMCOMM_SHARE_ID)//缓冲区标识不对
	{
		return ERROR_MEMCOMM_BUFFERID_NOT_FIT;
	}

	UINT32 dwLenAlign = (dwLen + MEMCOMM_ALIGN_BYTE_NUM -1) / MEMCOMM_ALIGN_BYTE_NUM + 1;//其中一字节用于保存数据长度

	Lock();

	UINT32 uiFreeSize;//空闲缓冲区大小
	if (m_pStruCookie->uiReadPos == m_pStruCookie->uiWritePos)//缓冲区为空
	{
		uiFreeSize = m_pStruCookie->uiTotalSize;
	}
	else
		if (m_pStruCookie->uiReadPos > m_pStruCookie->uiWritePos)
		{
			uiFreeSize =m_pStruCookie->uiReadPos - m_pStruCookie->uiWritePos;
		}
		else
		{
			uiFreeSize = m_pStruCookie->uiTotalSize - m_pStruCookie->uiWritePos + m_pStruCookie->uiReadPos;
		}

		if (uiFreeSize < dwLenAlign + 1)//缓冲区空间不够，保留一个空间（4字节）用于区别缓冲区是否为空 TODO:等待超时
		{
			Unlock();
			return ERROR_MEMCOMM_BUFFER_NOT_ENOUGH;
		}

		char *pMemPos = (char *)m_pHead + m_pStruCookie->uiOffSet;//缓冲区起始位置
		UINT32 uiWriteSize = 0;//每次写入长度

		UINT32 dwRightSize = m_pStruCookie->uiTotalSize- m_pStruCookie->uiWritePos;//计算缓冲区写指针后空间大小

		if (dwRightSize < dwLenAlign)
		{

			*(UINT32 *)(pMemPos + m_pStruCookie->uiWritePos * MEMCOMM_ALIGN_BYTE_NUM) = dwLen;//保存写入长度

			m_pStruCookie->uiWritePos++;
			dwRightSize --;

			uiWriteSize = dwRightSize * MEMCOMM_ALIGN_BYTE_NUM;
			memcpy( pMemPos + m_pStruCookie->uiWritePos * MEMCOMM_ALIGN_BYTE_NUM,pBuf,uiWriteSize);//拷贝数据前半段

			m_pStruCookie->uiWritePos = 0;
			uiWriteSize = dwLen - uiWriteSize;
			memcpy(pMemPos,(char *)pBuf + dwRightSize * MEMCOMM_ALIGN_BYTE_NUM,uiWriteSize);//拷贝数据后半段
			m_pStruCookie->uiWritePos = dwLenAlign - 1 - dwRightSize;

		}
		else//整段拷贝
		{
			*(UINT32 *)(pMemPos + m_pStruCookie->uiWritePos * MEMCOMM_ALIGN_BYTE_NUM) = dwLen;//保存写入长度

			m_pStruCookie->uiWritePos ++;

			uiWriteSize = dwLen;
			memcpy(pMemPos + m_pStruCookie->uiWritePos * MEMCOMM_ALIGN_BYTE_NUM,pBuf,uiWriteSize);//拷贝数据

			m_pStruCookie->uiWritePos = m_pStruCookie->uiWritePos + dwLenAlign -1;

			if (m_pStruCookie->uiWritePos == m_pStruCookie->uiTotalSize)
			{
				m_pStruCookie->uiWritePos = 0;// 写位置置0
			}
		}

		Unlock();

		return ERROR_MEMCOMM_OPER_SUCCESS;
}
INT32 CMemCommBase::WriteUrgent(void *pBuf, UINT32 dwLen, UINT32 dwMilliSeconds)
{
	if (m_pStruCookie == NULL || pBuf == NULL)//无合法缓冲区
	{
		return ERROR_MEMCOMM_BUFFER_NOT_EXIST;
	}

	if (m_pStruCookie->uiMemId != MEMCOMM_SHARE_ID)//缓冲区标识不对
	{
		return ERROR_MEMCOMM_BUFFERID_NOT_FIT;
	}

	Lock();
	if (m_pStruCookie->uiLock != 1)//没加锁，则直接返回错误
	{
		Unlock();
		return ERROR_MEMCOMM_BUFFER_NOT_LOCK;
	}

	UINT32 dwLenAlign = (dwLen + MEMCOMM_ALIGN_BYTE_NUM -1) / MEMCOMM_ALIGN_BYTE_NUM + 1;//其中一字节用于保存数据长度

	UINT32 uiFreeSize;//空闲缓冲区大小
	if (m_pStruCookie->uiReadPos == m_pStruCookie->uiWritePos)//缓冲区为空
	{
		uiFreeSize = m_pStruCookie->uiTotalSize;
	}
	else
		if (m_pStruCookie->uiReadPos > m_pStruCookie->uiWritePos)
		{
			uiFreeSize =m_pStruCookie->uiReadPos - m_pStruCookie->uiWritePos;
		}
		else
		{
			uiFreeSize = m_pStruCookie->uiTotalSize - m_pStruCookie->uiWritePos + m_pStruCookie->uiReadPos;
		}

		if (uiFreeSize < dwLenAlign + 1)//缓冲区空间不够，保留一个空间（4字节）用于区别缓冲区是否为空 TODO:等待超时
		{
			Unlock();
			return ERROR_MEMCOMM_BUFFER_NOT_ENOUGH;
		}

		char *pMemPos = (char *)m_pHead + m_pStruCookie->uiOffSet;//缓冲区起始位置
		UINT32 uiWriteSize = 0;//每次写入长度

		UINT32 dwLeftSize = m_pStruCookie->uiReadPos;//计算缓冲区写指针后空间大小

		if (dwLeftSize < dwLenAlign)
		{
			
			if (m_pStruCookie->uiReadPos == 0)
			{
				m_pStruCookie->uiReadPos = m_pStruCookie->uiTotalSize -(dwLenAlign -dwLeftSize);
				*(UINT32 *)(pMemPos + m_pStruCookie->uiReadPos * MEMCOMM_ALIGN_BYTE_NUM) = dwLen;//保存写入长度
				uiWriteSize = dwLen;
				memcpy( pMemPos +( m_pStruCookie->uiReadPos + 1) * MEMCOMM_ALIGN_BYTE_NUM,pBuf,uiWriteSize);//拷贝整段数据到缓冲区尾部
			}
			else
			{
				m_pStruCookie->uiReadPos = m_pStruCookie->uiTotalSize -(dwLenAlign -dwLeftSize);
				*(UINT32 *)(pMemPos + m_pStruCookie->uiReadPos * MEMCOMM_ALIGN_BYTE_NUM) = dwLen;//保存写入长度

				uiWriteSize = (dwLenAlign -dwLeftSize -1) * MEMCOMM_ALIGN_BYTE_NUM;
				memcpy( pMemPos +( m_pStruCookie->uiReadPos + 1) * MEMCOMM_ALIGN_BYTE_NUM,pBuf,uiWriteSize);//拷贝数据前半段

				uiWriteSize = dwLen - uiWriteSize;
				memcpy(pMemPos,(char *)pBuf + (dwLenAlign -dwLeftSize -1) * MEMCOMM_ALIGN_BYTE_NUM,uiWriteSize);//拷贝数据后半段
			}
			
		}
		else//整段拷贝
		{
			m_pStruCookie->uiReadPos = m_pStruCookie->uiReadPos - dwLenAlign;

			*(UINT32 *)(pMemPos + m_pStruCookie->uiReadPos * MEMCOMM_ALIGN_BYTE_NUM) = dwLen;//保存写入长度

			uiWriteSize = dwLen;
			memcpy(pMemPos + (m_pStruCookie->uiReadPos + 1) * MEMCOMM_ALIGN_BYTE_NUM,pBuf,uiWriteSize);//拷贝数据
		}
		Unlock();
		return ERROR_MEMCOMM_OPER_SUCCESS;
}

