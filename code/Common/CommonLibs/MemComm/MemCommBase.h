#ifndef MEMCOMMBASE_H_
#define MEMCOMMBASE_H_

#include "IMemComm.h"
#define MEMCOMM_ALIGN_BYTE_NUM 4 //四字节对齐
#define MEMCOMM_ALIGN_BYTE_NUM 4  //标识长度的字节数
#define MEMCOMM_SHARE_ID *(int *)"DAPM"//缓冲区标识ID

typedef struct
{
	UINT32 uiMemId;//缓冲区标识
	UINT32 uiOffSet;//有效缓冲区起始点
	UINT32 uiTotalSize;//缓冲区总大小
	//UINT32 uiFreeSize;//缓冲区剩余空间
	UINT32 uiLock;// 是否加锁，0不加锁，1加锁
	UINT32 uiWritePos;//写位置
	UINT32 uiReadPos;//读位置
	char chMutexName[32];//锁名字
	char chEventName[32];//事件名字，用于读写通知
}StruMemCookie;
class CMemCommBase
{
public:
	CMemCommBase(void);
	virtual ~CMemCommBase(void);
protected:
	
	void * m_pHead;			//消息缓冲区的开始地址
	StruMemCookie *m_pStruCookie;
public:
	BOOL Create(UINT32 dwSize, BOOL bLock, EnumOperType opType, string &strName);
	void Reset(void);
	virtual BOOL Free(void)=0;
	INT32 Read(void *pBuf, UINT32 dwMaxLen, UINT32 *pRealLen, UINT32 dwMilliSeconds);
	INT32 Write(void *pBuf, UINT32 dwLen, UINT32 dwMilliSeconds);
	INT32 WriteUrgent(void *pBuf, UINT32 dwLen, UINT32 dwMilliSeconds);
protected:
	virtual BOOL Lock()=0;
	virtual void Unlock()=0;
	virtual BOOL Open(UINT32 dwSize, BOOL bLock, string &strName) = 0;
	virtual BOOL ReCreate(UINT32 dwSize, BOOL bLock, string &strName) = 0;
};

#endif


