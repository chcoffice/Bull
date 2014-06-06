#ifndef GSP_LIST_DEF_H
#define GSP_LIST_DEF_H



/*
******************************************
Copyright (C), 2010-2011, GOSUN
Filename : GSPLIST.H
Author :  zouyx
Version : 0.1.0.0
Date: 2010/6/8 14:21
Description: ¡¥±Ì¿‡
********************************************
*/

#include "GSPObject.h"



namespace GSP
{

class  CList : public CGSPObject
{


    
public:	
	template <class T >
	class CIterator
	{
	private :
		friend class CList;
		StruGSListNode *m_pCursor;
		const StruGSListNode *m_pH;

		CIterator(StruGSListNode *pCursor,const StruGSListNode *pH)
		{
			m_pCursor = pCursor;
			m_pH = pH;
		}
	public :
		CIterator(const CIterator &csDest )
		{
			m_pCursor = csDest.m_pCursor;
			m_pH = csDest.m_pH;
		}
		CIterator(void)
		{
			m_pCursor = NULL;
			m_pH = NULL;
		}

		~CIterator(void)
		{
			m_pCursor = NULL;
			m_pH = NULL;
		}
	
		CIterator &operator=(const CIterator &csDest )
		{
			if( this!=&csDest )
			{
				m_pCursor = csDest.m_pCursor;
				m_pH = csDest.m_pH;
			}
			return *this;
		}

		bool operator==(const CIterator &csDest) const
		{
			return (m_pH==csDest.m_pH && m_pCursor == csDest.m_pCursor);
		}


		INLINE T Data(void) const
		{
			GS_ASSERT(m_pH);
			return ((T)m_pCursor->Data);
		}

		INLINE void *Pointer(void) const
		{
			GS_ASSERT(m_pH);
			return ((T)m_pCursor->Data);
		}

		INLINE StruGSListNode *Node(void) const
		{
			return m_pCursor;
		}

		INLINE BOOL IsValid(void) const
		{
			const StruGSListNode *p = m_pH;
			if( !p )
			{
				return FALSE;
			}
			do
			{
				if( p==m_pCursor )
				{
					return TRUE;
				}
				p = p->next;
			} while( p!=m_pH );
			return FALSE;
		}

		INLINE BOOL IsOk(void) const
		{
			return m_pCursor!=m_pH;
		}

		INLINE void Next(void) 
		{
			m_pCursor = m_pCursor->next;
		}

		INLINE void Previous(void) 
		{
			m_pCursor = m_pCursor->prev;
		}

		INLINE BOOL IsLast(void) const
		{
			return m_pCursor && m_pCursor->next==m_pH;
		}

		INLINE BOOL IsFirst(void) const
		{
			return m_pCursor && m_pCursor->prev==m_pH;
		}
	};

	typedef CIterator<void*> CPointerIterator;

private :

	FuncPtrFree m_fnFree;
	UINT32 m_iSize;
	StruGSListNode m_stHeader;

public :

	CList(void);

	~CList(void);

	void SetFreeCallback( FuncPtrFree fnFree )
	{
		m_fnFree = fnFree;
	}

	INLINE UINT32 Size(void) const
	{
		return m_iSize;
	}

	INLINE BOOL IsEmpty(void) const
	{
		return m_stHeader.Empty();
	}

	EnumErrno AddFirst(void *pData);
	EnumErrno AddTail(void *pData);

	EnumErrno Prepend( StruGSListNode *pIterator, void *pData );
	EnumErrno Append( StruGSListNode *pIterator, void *pData);

	

	StruGSListNode *Find( void *pKey );
	template<class T> CList::CIterator<T> FindIterator( void *pKey );

	
	template<class T>  CList::CIterator<T>  Erase( CList::CIterator<T>  &csIt );
	EnumErrno Erase(StruGSListNode *pNode );	
	EnumErrno Erase( void *pKey );

	template<class T> CList::CIterator<T> Remove( CList::CIterator<T>  &csIt );

	EnumErrno Remove( void *pKey );	
	EnumErrno Remove( StruGSListNode *pNode);
	EnumErrno RemoveFront( void **pData);

	void Clear(void);

	INLINE void *FirstData(void) const
	{
		if( m_stHeader.Empty() )
		{
			return NULL;
		}
		return m_stHeader.next->Data;
	}

	INLINE void *LastData(void) const
	{
		if( m_stHeader.Empty() )
		{
			return NULL;
		}
		return m_stHeader.prev->Data;
	}


	void Sort( FuncPtrCompares fnCmp );

	void Swap(CList &csDest );



	template<class T> CList::CIterator<T>  First(void) const;
	template<class T> CList::CIterator<T> Last(void) const;
private :
	void FreeListNode(StruGSListNode *pNode, BOOL bFreeData);    
	
};

template<class T> CList::CIterator<T> CList::FindIterator( void *pKey )
{
	{
		StruGSListNode *p;
		p = Find(pKey);
		if( p )
		{
			return CIterator<T>(p,&m_stHeader);
		}
		return CIterator<T>(&m_stHeader,&m_stHeader);
	}
}

template<class T>  CList::CIterator<T>  CList::Erase( CList::CIterator<T>  &csIt )
{
	GS_ASSERT_RET_VAL(csIt.IsOk(), csIt);

	 CList::CIterator<T> csRet(csIt);
	csRet.m_pCursor = csRet.m_pH->prev;
	Erase(csIt.m_pCursor);
	csIt = csRet;
	return csRet;
}

template<class T> CList::CIterator<T> CList::Remove( CList::CIterator<T>  &csIt )	
{
	GS_ASSERT_RET_VAL(csIt.IsOk(), csIt);
	CIterator<T> csRet(csIt);
	csRet.m_pCursor = csRet.m_pH->prev;
	Remove(csIt.m_pCursor);
	csIt = csRet;
	return csRet;
}

template<class T> CList::CIterator<T>  CList::First(void) const
{
	return CList::CIterator<T>( m_stHeader.next, &m_stHeader);
}

template<class T> CList::CIterator<T> CList::Last(void) const
{
	return CList::CIterator<T>(m_stHeader.prev, &m_stHeader);
}

}; // end GSP

#endif
