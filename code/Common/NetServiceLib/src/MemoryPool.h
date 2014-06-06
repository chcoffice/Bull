#if !defined (MemoryPool_DEF_H)
#define MemoryPool_DEF_H

/***************************************************************************************************
	Copyright (C), 1997-2010, gosun Tech. Co., Ltd.
	FileName: 		MemoryPool.h
	Author:        	陈聪
	Version :       v1.0   
	Date:			2010/08/25
	Description:    内存池模板类
					实现任意类型的对象或结构体的内存池。
					内存池的每一块的结构为TYPE|Index
					Index的意义是索引，代表下一个可用的内存块的位置

****************************************************************************************************/

#include "NetServiceDataType.h"

namespace NetServiceLib
{

#define	MAX_TYPE_COUNT		100*10000				// 最大的类型TYPE数目
#define	MIN_TYPE_COUNT		10						// 最小的类型TYPE数目


template<class TYPE>
class CMemoryPool
{
public:
	CMemoryPool();
	virtual ~CMemoryPool(void);

	// 初始化内存池
	BOOL	Init(INT32 iTYPECount );

	// 分配一个TYPE的内存
	TYPE*	Allocate();

	// 释放一个TYPE内存
	BOOL	DeAllocate(void* pType);

	// 销毁内存池
	BOOL	Destroy();

private:
	// 判断指针是否合法
	BOOL	IsValidPtr( void* pType);

	// 追加内存  
	BOOL	ReAllocate( INT32 iTYPECount );

private:
	//	模板类型TYPE的大小 sizeof(TYPE)
	INT32				m_iTYPESize;

	// 模板类型TYPE的数目. 意义为内存池大小=m_iTYPECount*（m_iTYPESize + sizeof（INT32））
	INT32				m_iTYPECount;

	// 当前的可用块索引
	INT32				m_iAvailableBlockIndex;

	// 可用的块数目
	INT32				m_iAvailableBlockCount;

	// 内存池起始地址
	BYTE*				m_pMemStartAddress;

	// 锁
	CGSMutex			m_GSMutex;

};

// 实现

template<class TYPE>
CMemoryPool<TYPE>::CMemoryPool() : 
m_iTYPESize(sizeof(TYPE)),
m_iTYPECount(0),
m_iAvailableBlockIndex(0),
m_iAvailableBlockCount(0),
m_pMemStartAddress(NULL)
{

}
template<class TYPE>
CMemoryPool<TYPE>::~CMemoryPool(void)
{
	Destroy();
}

/********************************************************************************
Function:		Init
Description:	初始化内存池
Input:  		
Output:      	   
Return:  		成功返回TRUE 失败返回FALSE       
Note:			根据传入的参数,分配内存		
Author:        	CHC
Date:				2010/08/25
********************************************************************************/
template<class TYPE>
BOOL CMemoryPool<TYPE>::Init(INT32 iTYPECount )
{
	CGSAutoMutex	GSAutoMutex(&m_GSMutex);

	if (iTYPECount<MIN_TYPE_COUNT || iTYPECount > MAX_TYPE_COUNT)
	{
		return FALSE;
	}
	if (m_iTYPESize < 0)
	{
		return FALSE;
	}

	// 分配iTYPECount个m_iTYPESize+Index索引的内存
	m_pMemStartAddress = (BYTE*)malloc( iTYPECount * (m_iTYPESize + sizeof(INT32)) );

	if ( NULL == m_pMemStartAddress )
	{
		return FALSE;
	}

	// 初始化数据

	m_iAvailableBlockIndex = 0;

	// 初始时 m_iAvailableBlockCount = m_iTYPECount
	m_iAvailableBlockCount = iTYPECount;
	m_iTYPECount = iTYPECount;

	// 初始化每个TYPE的索引 
	INT32		iIndex = 0;
	INT32*		pNext = NULL;
	BYTE*		pPos = m_pMemStartAddress;

	for ( ; iIndex != iTYPECount; pPos += ( m_iTYPESize + sizeof( INT32 )) )
	{
		// 初始化每一块内存TYPE|Index结构中的Index
		pNext = (INT32*)( pPos + m_iTYPESize );
		*pNext = ++iIndex;
	}

	return TRUE;


}

/********************************************************************************
Function:		Allocate
Description:	分配一个TYPE的内存
Input:  		
Output:      	   
Return:  		成功返回TYPE的指针，失败返回NULL       
Note:					
Author:        	CHC
Date:				2010/08/25
********************************************************************************/
template<class TYPE>
TYPE* CMemoryPool<TYPE>::Allocate()
{
	CGSAutoMutex	GSAutoMutex(&m_GSMutex);

	// 判断有无可用的块
	if ( m_iAvailableBlockCount <= 0 )
	{
		if ( !ReAllocate( m_iTYPECount*2 ))
		{
			return NULL;
		}
		
		
	}

	BYTE*	pResult = NULL;

	// 获取可用的内存块的指针
	pResult = m_pMemStartAddress + m_iAvailableBlockIndex * ( m_iTYPESize + sizeof( INT32 ) );
	//memset(pResult, 0x0, m_iTYPESize);	// 加上这个分配内存慢许多

	// 计算下一个可用的内存块
	m_iAvailableBlockIndex = *(INT32*)( pResult + m_iTYPESize );


	// 可用块计数减一
	m_iAvailableBlockCount--;

	return static_cast<TYPE*>((void*)pResult);

}

/********************************************************************************
Function:		DeAllocate
Description:	释放一个TYPE内存
Input:  		
Output:      	   
Return:  		成功返回TRUE，失败返回FALSE
Note:			相当于把内存归还给内存池		
Author:        	CHC
Date:				2010/08/25
********************************************************************************/
template<class TYPE>
BOOL CMemoryPool<TYPE>::DeAllocate(void* pType)
{
	CGSAutoMutex	GSAutoMutex(&m_GSMutex);

	// 判断内存合法
	if ( !IsValidPtr(pType))
	{
		return FALSE;
	}
	 

	BYTE*	pRelease = static_cast<BYTE*>(pType);

	// 释放的块内存的索引 TYPE|Index 赋值为当前的可用块索引
	*((INT32*)( pRelease + m_iTYPESize )) = m_iAvailableBlockIndex;

	// 然后计算当前的可用块索引
	m_iAvailableBlockIndex = ( pRelease - m_pMemStartAddress )/( m_iTYPESize + sizeof( INT32 ) );

	// 可用块计数加一
	m_iAvailableBlockCount++;

	return TRUE;


}

/********************************************************************************
Function:		Destroy
Description:	销毁内存池
Input:  		
Output:      	   
Return:  		       
Note:			成功返回TRUE，失败返回FALSE		
Author:        	CHC
Date:				2010/08/25
********************************************************************************/
template<class TYPE>
BOOL CMemoryPool<TYPE>::Destroy()
{
	CGSAutoMutex	GSAutoMutex(&m_GSMutex);

	// 销毁前判断是否所有的内存块都已经释放 
	if ( m_iAvailableBlockCount < m_iTYPECount )
	{
		return FALSE;
	}

	if ( m_pMemStartAddress )
	{
		free(m_pMemStartAddress);
		m_pMemStartAddress = NULL;
	}

	return TRUE;


}

/********************************************************************************
  Function:		IsValidPtr
  Description:	 判断指针是否合法
  Input:  		
  Output:      	   
  Return:  		TRUE 合法 FALSE 不合法       
  Note:					
  Author:        	CHC
  Date:				2010/09/13
********************************************************************************/
template<class TYPE>
BOOL CMemoryPool<TYPE>::IsValidPtr( void* pType)
{
	//内存不在这个里面。也不是他分配的。
	if(pType< m_pMemStartAddress || pType > (m_pMemStartAddress + (m_iTYPESize + sizeof( INT32 )) * m_iTYPECount ) ) 
	{
		return FALSE;
	}

	//指针没在blockSize边界上对齐．肯定不是由这个MemPool分配的
	INT32 iResult = ((BYTE*)pType - m_pMemStartAddress) % (m_iTYPESize + sizeof( INT32));
	if(iResult != 0) 
	{
		return FALSE;
	}

	return TRUE;
}

// 追加内存  
/********************************************************************************
  Function:		ReAllocate
  Description:	内存池内存不足时,追加内存
  Input:  		iTYPECount 需要增加的块数
  Output:      	   
  Return:  		TRUE 分配成功 FALSE 失败       
  Note:					
  Author:        	CHC
  Date:				2010/09/13
********************************************************************************/
template<class TYPE>
BOOL CMemoryPool<TYPE>::ReAllocate( INT32 iTYPECount )
{
	CGSAutoMutex	GSAutoMutex(&m_GSMutex);

	if ( m_iAvailableBlockCount > 0)
	{
		return TRUE;
	}

	if ( m_iTYPECount + iTYPECount<MIN_TYPE_COUNT || m_iTYPECount + iTYPECount > MAX_TYPE_COUNT)
	{
		return FALSE;
	}
	if (m_iTYPESize < 0)
	{
		return FALSE;
	}

	// 分配iTYPECount个m_iTYPESize+Index索引的内存
	BYTE* pAddress = (BYTE*)realloc( m_pMemStartAddress, iTYPECount * (m_iTYPESize + sizeof(INT32)) );
	

	if ( NULL == pAddress )
	{
		return FALSE;
	}

	// 新地址,新地址可能和旧地址相同
	m_pMemStartAddress = pAddress;

	// 初始化每个新申请的TYPE的索引 
	INT32		iIndex = m_iTYPECount;
	INT32*		pNext = NULL;
	BYTE*		pPos = m_pMemStartAddress + (m_iTYPESize + sizeof( INT32 )) * m_iTYPECount ;

	for ( ; iIndex != iTYPECount; pPos += ( m_iTYPESize + sizeof( INT32 )) )
	{
		// 初始化每一块内存TYPE|Index结构中的Index
		pNext = (INT32*)( pPos + m_iTYPESize );
		*pNext = ++iIndex;
	}

	// 现在的可用块索引
	m_iAvailableBlockIndex = m_iTYPECount + 1;
	// 现在 m_iAvailableBlockCount 最大可以等于 = m_iTYPECount
	m_iAvailableBlockCount = iTYPECount;
	m_iTYPECount += iTYPECount;

	return TRUE;
}

}

#endif

