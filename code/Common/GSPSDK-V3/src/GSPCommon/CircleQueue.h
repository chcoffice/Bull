/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : CIRCLEQUEUE.H
Author :  zouyx
Version : 0.0.0.0
Date: 2012/3/31 10:45
Description: 环形队列， 对 一读一写的模式 线程安全
********************************************
*/

#ifndef _GS_H_CIRCLEQUEUE_H_
#define _GS_H_CIRCLEQUEUE_H_

#include "GSPObject.h"
#include <vector>

namespace GSP
{


template <class T >
class CCircleQueue : public CGSPObject
{
private :
	std::vector<T> m_csQueue;
	UINT m_iQueueSize;
	UINT m_iW;
	UINT m_iR;
	FuncPtrFree m_fnFreeMember;
	const T m_tDefaultValue;
public:

	CCircleQueue(UINT iSize, const T tDefaultValue)
		:CGSPObject()
		,m_csQueue()	
		,m_iQueueSize(0)
		,m_iW(0)
		,m_iR(0)
		,m_fnFreeMember(NULL)
		,m_tDefaultValue(tDefaultValue)
	{
		m_csQueue.resize(iSize,tDefaultValue);
		m_iQueueSize = m_csQueue.size();
		GS_ASSERT(m_iQueueSize>2);

	}
	
	void SetFreeMemberFunction( FuncPtrFree fnFree)
	{
		m_fnFreeMember = fnFree;
	}

	virtual ~CCircleQueue(void)
	{
		Clear();
	}

	void Clear(void)
	{
		if( m_fnFreeMember)
		{
			T pValue;
			while( eERRNO_SUCCESS==Pop(&pValue) )
			{
				m_fnFreeMember((void*)pValue);
			}
		}
		m_iR = 0;
		m_iW = 0;
	}

	EnumErrno Write(const T tValue )
	{
		INT iIndex = m_iW+1;
		INT iFree = iIndex-m_iR;
		if( iFree<0 )
		{
			// ---- w -- r--			
			m_csQueue[iIndex] = tValue;
			m_iW = iIndex;
		} 
		else if( iFree>0 )
		{
			// --- r -- w --				
			if( iIndex==m_iQueueSize )
			{
				if( m_iR==0  )
				{
					//没有空间					
					return eERRNO_SYS_EFLOWOUT;
					
				}
				iIndex = 0;
			}
			m_csQueue[iIndex] = tValue;
			m_iW = iIndex;
		} 
		else 
		{
			//没有空间
			//   -- r/w--			
			return eERRNO_SYS_EFLOWOUT;
		}
		return eERRNO_SUCCESS;
	}

	EnumErrno Read(T *pValue)
	{
		INT iFree = m_iR-m_iW;
		if( iFree==0 )
		{
			return eERRNO_SYS_EFLOWOUT;
		}
		else 
		{
			// ---- w -- r--	
			// ---- r -- w--	
			iFree = m_iR+1;					
			if( iFree == m_iQueueSize )
			{
				iFree = 0;
			}
			*pValue = m_csQueue[iFree];				
		}		
		return eERRNO_SUCCESS;
	}

	EnumErrno Pop( T *pValue )
	{
		INT iFree = m_iR-m_iW;
		if( iFree==0 )
		{
			return eERRNO_SYS_EFLOWOUT;
		}
		else 
		{
			// ---- w -- r--	
			// ---- r -- w--	
			iFree = m_iR+1;					
			if( iFree == m_iQueueSize )
			{
				iFree = 0;
			}
			*pValue = m_csQueue[iFree];	
			m_csQueue[iFree] = m_tDefaultValue;
			m_iR = iFree;
		}		
		return eERRNO_SUCCESS;
	}

	UINT Free(void) const
	{
		INT iFree = m_iR-m_iW;
		if( iFree>0 )
		{
			// ---- w -- r--
			return iFree;
		}
		return m_iQueueSize+iFree;
	}

	UINT Size(void) const
	{
		INT iFree = m_iR-m_iW;
		if( iFree>0 )
		{
			// ---- w -- r--
			return m_iQueueSize-iFree;
		}
		return -iFree;
	}

private :
	CCircleQueue(const CCircleQueue &csDest)
	{

	}
	CCircleQueue &operator=(const CCircleQueue &csDest)
	{
		return *this;
	}

};

// 
// class CCircleBuffer
// {
// private :
// 	BOOL m_bBegin;
// 	typedef struct _StruNode
// 	{
// 		unsigned int iSize;
// 		unsigned int iDataSize;
// 		unsigned char bBuf[1];
// 	}StruNode;
// 	
// 	unsigned char *m_pBuffer;
// 	unsigned char *m_pR;
// 	unsigned char *m_pW;
// 	UINT m_iBufSize;
// 
// 	static const unsigned int INVALID_NODE = ( unsigned int)-1;
// public :
// 	CCircleBuffer(void)
// 	{
// 		m_pBuffer = NULL;
// 		m_pR = NULL;
// 		m_pW = NULL;
// 	}
// 
// 	~CCircleBuffer(void)
// 	{
// 		if( m_pBuffer )
// 		{
// 			::free(m_pBuffer);
// 		}
// 	}
// 
// 	BOOL Init( UINT  iBufSize) const
// 	{
// 		if( m_pBuffer != NULL)
// 		{
// 			GS_ASSERT(0);
// 			return FALSE;
// 		}
// 
// 		if(  iBufSize< sizeof(StruNode)*2 )
// 		{
// 			return FALSE;
// 		}
// 
// 		m_pBuffer = ::malloc( iBufSize);
// 		if( !m_pBuffer )
// 		{
// 			return FALSE;
// 		}
// 		m_iBufSize = iBufSize;
// 		m_pR = m_pW = m_pBuffer;
// 	}
// 
// 
// 
// 	UINT BufferSize(void) const;
// 	
// 	UINT FreeSize(void);
// 
// 	//=============== Write Function ====================
// 	void *Begin( UINT iSize );
// 	void Commint( void *pHandle);
// 	void Callback(void *pHandle);
// 	BOOL Push(const void *pData, UINT iSize );
// 	
// 
// 	//==================Reader Function=================
// 	void *Read(UINT &iSize );
// 	void *Pop(UINT &iSize);
// 	void Pop(void);
// 
// 	
// };


} //end namespace GSP

#endif //end _GS_H_CIRCLEQUEUE_H_
