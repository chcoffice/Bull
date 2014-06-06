#include "List.h"
#include "GSPMemory.h"

using namespace GSP;






static INLINE void InitGSPListNode(StruGSListNode *pNode)
{

   pNode->Init();
}

static INLINE StruGSListNode *NewGSSListNode(void)
{
StruGSListNode *pRet;
	pRet = (StruGSListNode *)CMemoryPool::Malloc(sizeof(StruGSListNode));
    if( pRet )
    {
        InitGSPListNode(pRet);
    }
    return pRet;
}

static INLINE void  FreeGSSListNode(StruGSListNode *pNode)
{
   CMemoryPool::Free(pNode);
}





CList::CList(void)
:CGSPObject()
,m_fnFree(NULL)
,m_iSize(0)
{
   
  m_stHeader.Init();

}


CList::~CList(void)
{
    Clear();

}

void CList::FreeListNode(StruGSListNode *pNode, BOOL bFreeData)
{
    if( pNode )
    {

        if( m_fnFree && bFreeData )
        {
            m_fnFree( pNode->Data );
        }
        pNode->Data = NULL;
        FreeGSSListNode(pNode);
    }
}


EnumErrno CList::Prepend( StruGSListNode *pIterator, void *pData )
{
	if(  !pIterator )
	{
		GS_ASSERT(0);
		return eERRNO_SYS_ESTATUS;
	}

	StruGSListNode *pNode = NewGSSListNode();
	if( !pNode )
	{
		GS_ASSERT(0);
		return eERRNO_SYS_ENMEM;
	}
	pNode->Data = pData;

	StruGSListNode::AddInsert(pNode, pIterator->prev, pIterator);
	m_iSize++;
	return eERRNO_SUCCESS;
}

EnumErrno CList::Append( StruGSListNode *pIterator, void *pData)
{
	if(  !pIterator )
	{
		GS_ASSERT(0);
		return eERRNO_SYS_ESTATUS;
	}

	StruGSListNode *pNode =  NewGSSListNode();
	if( !pNode )
	{
		GS_ASSERT(0);
		return eERRNO_SYS_ENMEM;
	}
	pNode->Data = pData;

	StruGSListNode::AddInsert(pNode, pIterator,pIterator->next);
	m_iSize++;
	return eERRNO_SUCCESS;
}


EnumErrno CList::AddFirst(void *pData)
{
	
StruGSListNode *pNode =  NewGSSListNode();
	if( !pNode )
	{
		GS_ASSERT(0);
		return eERRNO_SYS_ENMEM;
	}
	pNode->Data = pData;

	StruGSListNode::AddFirst(pNode, &m_stHeader);
	m_iSize++;
	return eERRNO_SUCCESS;
	
}

void CList::Swap(CList &csDest )
{
	if( this == &csDest )
	{
		return;
	}

StruGSListNode stTemp;
UINT iTemp = m_iSize;	

	//本地缓冲中间值
	 if( iTemp == 0)
	 {
		 stTemp.Init();
	 }
	 else
	 {
		 stTemp.next = m_stHeader.next;
		 stTemp.prev = m_stHeader.prev;
	 }

	 //目标设到本地
	 m_iSize = csDest.m_iSize;
	 if( 0==m_iSize )
	 {
		m_stHeader.Init();
	 }
	 else
	 {
		m_stHeader.next = csDest.m_stHeader.next;
		m_stHeader.prev  = csDest.m_stHeader.prev;	

		m_stHeader.next->prev = &m_stHeader;
		m_stHeader.prev->next= &m_stHeader;
	 }

	//中间值 设到目标
	 csDest.m_iSize = iTemp;
	 if( iTemp )
	 {
		 csDest.m_stHeader.next = stTemp.next;
		 csDest.m_stHeader.prev = stTemp.prev;

		 csDest.m_stHeader.next->prev = &csDest.m_stHeader;
		 csDest.m_stHeader.prev->next= &csDest.m_stHeader;
	 }
	 else
	 {
		csDest.m_stHeader.Init();
	 }


}

void CList::Sort( FuncPtrCompares fnCmp )
{
	if( m_stHeader.Empty() )
	{
		return;
	}
StruGSListNode stTemp;
	StruGSListNode::Swap(m_stHeader, stTemp);
StruGSListNode *pNode, *p;
	while(! stTemp.Empty() )
	{
		pNode = StruGSListNode::RemoveFront(&stTemp );
		for( p=m_stHeader.next; p!=&m_stHeader; p = p->next )
		{
			if( fnCmp(p->Data, pNode->Data)< 0 )
			{
				StruGSListNode::AddInsert(pNode,p->prev, p );
				break;
			}
		}
		if( p==&m_stHeader )
		{
			StruGSListNode::AddTail(pNode, &m_stHeader );
		}
	}


}

EnumErrno CList::AddTail(void *pData)
{


	StruGSListNode *pNode =  NewGSSListNode();
	if( !pNode )
	{
		GS_ASSERT(0);
		return eERRNO_SYS_ENMEM;
	}
	pNode->Data = pData;
	StruGSListNode::AddTail(pNode, &m_stHeader);
	m_iSize++;
	return eERRNO_SUCCESS;
}

EnumErrno CList::RemoveFront( void **pData)
{

	StruGSListNode *pNode = StruGSListNode::RemoveFront(&m_stHeader );
	if( !pNode )
	{
		return eERRNO_SYS_ENEXIST;
	}
	if( pData )
	{
		*pData = pNode->Data;
	}
	pNode->Unlink();
	m_iSize--;
	FreeListNode(pNode, FALSE);
	return eERRNO_SUCCESS;
}


StruGSListNode *CList::Find( void *pKey )
{
StruGSListNode *pNode;
	for( pNode = m_stHeader.next; pNode != &m_stHeader; pNode = pNode->next )
	{
		if(pNode->Data==pKey )
		{
			return pNode;
		}
	}
	return NULL;
}




EnumErrno CList::Erase(StruGSListNode *pNode )
{
	if( pNode )
	{
		pNode->Unlink();
		m_iSize--;
		FreeListNode(pNode, TRUE);
	}
	return eERRNO_SUCCESS;
}


EnumErrno CList::Erase( void *pKey )
{
StruGSListNode *pNode = Find(pKey);
	if( pNode )
	{
		return Erase(pNode);
	}
	return eERRNO_SYS_ENEXIST;
}



EnumErrno CList::Remove( void *pKey )
{
	StruGSListNode *pNode = Find(pKey);
	if( pNode )
	{
		return Remove(pNode);
	}
	return eERRNO_SYS_ENEXIST;
}



EnumErrno CList::Remove( StruGSListNode *pNode)
{
	if( pNode )
	{
		m_iSize--;
		pNode->Unlink();
		FreeListNode(pNode, FALSE);
	}
	return eERRNO_SUCCESS;
}


void CList::Clear(void)
{
StruGSListNode *pNode;
	while((pNode=StruGSListNode::RemoveFront(&m_stHeader)))
	{
		m_iSize--;
		pNode->Unlink();
		FreeListNode(pNode, TRUE);
	}
	GS_ASSERT(m_iSize==0);
}