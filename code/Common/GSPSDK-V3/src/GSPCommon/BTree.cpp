#include "BTree.h"
#include "GSPMemory.h"

using namespace  GSP;


CBTree::StruGSPTreeNode *
CBTree::_TreeNodeNext (CBTree::StruGSPTreeNode *node)
{
    CBTree::StruGSPTreeNode *tmp;

    tmp = node->right;

    if (node->right_child)
        while (tmp->left_child)
            tmp = tmp->left;
    return tmp;
}

CBTree::StruGSPTreeNode *
CBTree::_TreeNodePrevious (CBTree::StruGSPTreeNode *node)
{
    StruGSPTreeNode *tmp;

    tmp = node->left;

    if (node->left_child)
        while (tmp->right_child)
            tmp = tmp->right;

    return tmp;
}


CBTree::StruGSPTreeNode *
CBTree::_TreeNodeRotateLeft (CBTree::StruGSPTreeNode *node)
{
    StruGSPTreeNode *right;
    int a_bal;
    int b_bal;

    right = node->right;

    if (right->left_child)
        node->right = right->left;
    else
    {
        node->right_child = FALSE;
        node->right = right;
        right->left_child = TRUE;
    }
    right->left = node;

    a_bal = node->balance;
    b_bal = right->balance;

    if (b_bal <= 0)
    {
        if (a_bal >= 1)
            right->balance = b_bal - 1;
        else
            right->balance = a_bal + b_bal - 2;
        node->balance = a_bal - 1;
    }
    else
    {
        if (a_bal <= b_bal)
            right->balance = a_bal - 2;
        else
            right->balance = b_bal - 1;
        node->balance = a_bal - b_bal - 1;
    }

    return right;
}

CBTree::StruGSPTreeNode*
CBTree::_TreeNodeRotateRight (CBTree::StruGSPTreeNode *node)
{
    StruGSPTreeNode *left;
    int a_bal;
    int b_bal;

    left = node->left;

    if (left->right_child)
        node->left = left->right;
    else
    {
        node->left_child = FALSE;
        node->left = left;
        left->right_child = TRUE;
    }
    left->right = node;

    a_bal = node->balance;
    b_bal = left->balance;

    if (b_bal <= 0)
    {
        if (b_bal > a_bal)
            left->balance = b_bal + 1;
        else
            left->balance = a_bal + 2;
        node->balance = a_bal - b_bal + 1;
    }
    else
    {
        if (a_bal <= -1)
            left->balance = b_bal + 1;
        else
            left->balance = a_bal + b_bal + 2;
        node->balance = a_bal + 1;
    }

    return left;
}

CBTree::StruGSPTreeNode *
CBTree::_TreeNodeBalance (CBTree::StruGSPTreeNode *node)
{
    if (node->balance < -1)
    {
        if (node->left->balance > 0)
            node->left = _TreeNodeRotateLeft (node->left);
        node = _TreeNodeRotateRight (node);
    }
    else if (node->balance > 1)
    {
        if (node->right->balance < 0)
            node->right = _TreeNodeRotateRight (node->right);
        node = _TreeNodeRotateLeft (node);
    }

    return node;
}




CBTree::CBTree(FuncPtrCompares fnKeyCmp)
:CGSPObject()
,m_struRoot(NULL)	
,m_fnKeyCmp(fnKeyCmp)
,m_iNodes(0)
,m_csWRMutex()
,m_fnKeyFree(NULL)
,m_fnDataFree(NULL)
,m_iSize(0)
{
    GS_ASSERT_EXIT(fnKeyCmp!=NULL,-1);
}

CBTree::~CBTree(void)
{
    Clear();
}




void CBTree::SetFreeFunction(FuncPtrFree fnKeyFree, FuncPtrFree fnDataFree)
{
    m_fnKeyFree = fnKeyFree;
    m_fnDataFree = fnDataFree;
}

CBTree::CIterator  CBTree::Insert( void *key, void *data, BOOL bLock )
{
    CBTree::CIterator csIt(this);


    if( bLock )
    {
        m_csWRMutex.LockWrite();
    }

    InsertInternal( csIt, key, data);
    if( csIt.Second() )
    {
        m_iSize++;
    }
    if( bLock )
    {
        m_csWRMutex.UnlockWrite();
    }

    return csIt;
}


void  CBTree::InsertInternal (  CBTree::CIterator &csIt,
                              void  *key,void  *  data)
{
    StruGSPTreeNode *node;
    StruGSPTreeNode *path[GSS_MAX_GTREE_HEIGHT];
    int idx;


    if (!m_struRoot)
    {
        m_struRoot = GetTreeNode ();
        m_struRoot->key = key;
        m_struRoot->data = data;
        m_iNodes++;
        csIt.m_bSecond = TRUE;
        csIt.m_pFirst = data;
        csIt.m_pKey = key;
        csIt.m_pNode = m_struRoot;        
        return;
    }

    idx = 0;
    path[idx++] = NULL;
    node = m_struRoot;

    while (1)
    {
        int cmp = m_fnKeyCmp(key, node->key);

        if (cmp == 0)
        {
            //已经存在 
            csIt.m_bSecond = FALSE;
            csIt.m_pFirst = node->data;
            csIt.m_pKey = node->key;
            csIt.m_pNode = node;   
            return ;
        }
        else if (cmp < 0) 
        {

            if (node->left_child)  
            {
                path[idx++] = node;
                node = node->left;
            }
            else  
            {
                StruGSPTreeNode *child = GetTreeNode ();
                child->key = key;
                child->data = data;

                child->left = node->left;
                child->right = node;
                node->left = child;
                node->left_child = TRUE;
                node->balance -= 1;
                m_iNodes++; 

                csIt.m_bSecond = TRUE;
                csIt.m_pFirst = node->data;
                csIt.m_pKey = node->key;
                csIt.m_pNode = node;  

                break;
            }
        } 
        else        
        {

            if (node->right_child)            
            {
                path[idx++] = node;
                node = node->right;
            }
            else       
            {
                StruGSPTreeNode *child = GetTreeNode ();
                child->key = key;
                child->data = data;

                child->right = node->right;
                child->left = node;
                node->right = child;
                node->right_child = TRUE;
                node->balance += 1;
                m_iNodes++; 
                csIt.m_bSecond = TRUE;
                csIt.m_pFirst = node->data;
                csIt.m_pKey = node->key;
                csIt.m_pNode = node;  
                break;
            }
        }
    }

    /* restore balance. This is the goodness of a non-recursive
    implementation, when we are done with balancing we 'break'
    the loop and we are done. */
    while (1)
    {
        StruGSPTreeNode *bparent = path[--idx];
        BOOL left_node = (bparent && node == bparent->left);
        GS_ASSERT_RET(!bparent || bparent->left == node || bparent->right == node);

        if (node->balance < -1 || node->balance > 1)
        {
            node = _TreeNodeBalance (node);
            if (bparent == NULL)
                m_struRoot = node;
            else if (left_node)
                bparent->left = node;
            else
                bparent->right = node;
        }

        if (node->balance == 0 || bparent == NULL)
            break;

        if (left_node)
            bparent->balance -= 1;
        else
            bparent->balance += 1;
        node = bparent;
    }     
}

CBTree::StruGSPTreeNode *CBTree::GetTreeNode(void)
{
	StruGSPTreeNode * p = (StruGSPTreeNode *) CMemoryPool::Malloc(sizeof( StruGSPTreeNode));
    if( p )
    {
        bzero(p, sizeof( StruGSPTreeNode));
    }
    return p;
}

void CBTree::FreeTreeNode(StruGSPTreeNode *pNode, BOOL bFreeData, BOOL bFreeKey)
{
    if(pNode)
    {
         if(bFreeKey && m_fnKeyFree )
        {   
            m_fnKeyFree(pNode->key);            
            
        }
        pNode->key = NULL;

        if( bFreeData && m_fnDataFree )
        {
            m_fnDataFree(pNode->data);
         }
         pNode->data = NULL;
		 CMemoryPool::Free(pNode);	
    }
}


CBTree::CIterator CBTree::Find( const void *key, BOOL bLock )
{
    StruGSPTreeNode *node;
    int cmp;
    CBTree::CIterator csIt(this);

    if( bLock )
    {
        m_csWRMutex.LockReader();
    }
    node = m_struRoot;
    if (node)   
    {
        while (1)
        {
            cmp = m_fnKeyCmp (key,  node->key );
            if (cmp == 0)
            {
                csIt.m_bSecond = TRUE;
                csIt.m_pKey = node->key;
                csIt.m_pFirst = node->data;
                csIt.m_pNode = node;
                break;
            }
            else if (cmp < 0)
            {
                if (!node->left_child)
                {
                    break;
                }
                node = node->left;
            }
            else
            {
                if (!node->right_child)
                {
                    break;
                }
                node = node->right;
            }
        } //end while(1)

    } //end if(node)
    if( bLock )
    {
        m_csWRMutex.UnlockReader();
    }
    return csIt;
}

CBTree::CIterator CBTree::Remove( const void *key,  BOOL bLock)
{
    CBTree::CIterator csIt(this);
    StruGSPTreeNode *node = NULL;
    if(bLock)
    {
        m_csWRMutex.LockWrite();
    }
    node =  RemoveInternal (key);

    if( node &&  m_iSize )
    {
        m_iSize--;
    }

    if(bLock)
    {

        m_csWRMutex.UnlockWrite();
    }
    if( node ){ 
        csIt.m_bSecond = TRUE;
        csIt.m_pKey = node->key;
        csIt.m_pFirst = node->data; 
        csIt.m_pNode = NULL;
      
        FreeTreeNode(node, FALSE, FALSE);


    }     
    return csIt;

}

void  CBTree::Remove(  CBTree::CIterator &csIt, BOOL bLock)
{
    StruGSPTreeNode *node = NULL;

    GS_ASSERT_RET(csIt.m_pOwner==this);

    if( csIt.m_pNode == NULL)
    {
        return;
    }

    if(bLock)
    {
        m_csWRMutex.LockWrite();
    }
    node =  RemoveInternal (csIt.m_pKey);

    if( node &&  m_iSize )
    {
        m_iSize--;
    }

    if(bLock)
    {

        m_csWRMutex.UnlockWrite();
    }
    if( node ){
        GS_ASSERT(csIt.m_pNode==node);

        csIt.m_bSecond  = FALSE;
        FreeTreeNode(node, FALSE, FALSE);
        csIt.m_pNode = NULL;

    }     
}

void CBTree::Erase( const void *key, BOOL bLock)
{
    StruGSPTreeNode *node = NULL;
    if(bLock)
    {
        m_csWRMutex.LockWrite();
    }
    node =  RemoveInternal (key);

    if( node &&  m_iSize )
    {
        m_iSize--;
    }

    if(bLock)
    {
        m_csWRMutex.UnlockWrite();
    }
    if( node ){ 
        FreeTreeNode(node, TRUE, TRUE);
    }     


}
void CBTree::Erase(  CBTree::CIterator &csIt, BOOL bLock  )
{
    StruGSPTreeNode *node = NULL;

    GS_ASSERT_RET(csIt.m_pOwner==this);

    if( csIt.m_pNode == NULL)
    {
        return;
    }

    if(bLock)
    {
        m_csWRMutex.LockWrite();
    }
    node =  RemoveInternal (csIt.m_pKey);

    if( node &&  m_iSize )
    {
        m_iSize--;
    }

    if(bLock)
    {
        m_csWRMutex.UnlockWrite();
    }
    if( node ){
        GS_ASSERT(csIt.m_pNode==node);         
        FreeTreeNode(node, TRUE, TRUE);
        csIt.m_pKey = NULL;
        csIt.m_pFirst = NULL;
        csIt.m_bSecond  = FALSE;
        csIt.m_pNode = NULL;
    }     
}

UINT CBTree::Size(void)
{
    return m_iSize;
}

CBTree::StruGSPTreeNode *CBTree::RemoveInternal (const void *  key)
{
    StruGSPTreeNode *node, *parent, *balance;
    StruGSPTreeNode *path[GSS_MAX_GTREE_HEIGHT];
    int idx;
    BOOL left_node;



    if ( !m_struRoot)
        return NULL;

    idx = 0;
    path[idx++] = NULL;
    node = m_struRoot;

    while (1)
    {
        int cmp = m_fnKeyCmp (key, node->key);

        if (cmp == 0)
            break;
        else if (cmp < 0)
        {
            if (!node->left_child)
                return NULL;

            path[idx++] = node;
            node = node->left;
        }
        else
        {
            if (!node->right_child)
                return NULL;

            path[idx++] = node;
            node = node->right;
        }
    }

    /* the following code is almost equal to g_tree_remove_node,
    except that we do not have to call g_tree_node_parent. */
    balance = parent = path[--idx];
    GS_ASSERT (!parent || parent->left == node || parent->right == node);
    left_node = (parent && node == parent->left);

    if (!node->left_child)
    {
        if (!node->right_child)
        {
            if (!parent)
            {
                m_struRoot = NULL;
            }
            else if (left_node)
            {
                parent->left_child = FALSE;
                parent->left = node->left;
                parent->balance += 1;
            }
            else
            {
                parent->right_child = FALSE;
                parent->right = node->right;
                parent->balance -= 1;
            }
        }
        else /* node has a right child */
        {
            StruGSPTreeNode *tmp = _TreeNodeNext (node);
            tmp->left = node->left;

            if (!parent)
            {
                m_struRoot = node->right;
            }
            else if (left_node)
            {
                parent->left = node->right;
                parent->balance += 1;
            }
            else
            {
                parent->right = node->right;
                parent->balance -= 1;
            }
        }
    }
    else /* node has a left child */
    {
        if (!node->right_child)
        {
            StruGSPTreeNode *tmp =_TreeNodePrevious(node);
            tmp->right = node->right;

            if (parent == NULL)
            {
                m_struRoot = node->left;
            }
            else if (left_node)
            {
                parent->left = node->left;
                parent->balance += 1;
            }
            else
            {
                parent->right = node->left;
                parent->balance -= 1;
            }
        }
        else /* node has a both children (pant, pant!) */
        {
            StruGSPTreeNode *prev = node->left;
            StruGSPTreeNode *next = node->right;
            StruGSPTreeNode *nextp = node;
            int old_idx = idx + 1;
            idx++;

            /* path[idx] == parent */
            /* find the immediately next node (and its parent) */
            while (next->left_child)
            {
                path[++idx] = nextp = next;
                next = next->left;
            }

            path[old_idx] = next;
            balance = path[idx];

            /* remove 'next' from the tree */
            if (nextp != node)
            {
                if (next->right_child)
                    nextp->left = next->right;
                else
                    nextp->left_child = FALSE;
                nextp->balance += 1;

                next->right_child = TRUE;
                next->right = node->right;
            }
            else
                node->balance -= 1;

            /* set the prev to point to the right place */
            while (prev->right_child)
                prev = prev->right;
            prev->right = next;

            /* prepare 'next' to replace 'node' */
            next->left_child = TRUE;
            next->left = node->left;
            next->balance = node->balance;

            if (!parent)
            {
                m_struRoot = next;
            }
            else if (left_node)
            {
                parent->left = next;
            }
            else
            {
                parent->right = next;
            }
        }
    }

    /* restore balance */
    if (balance)
        while (1)
        {
            StruGSPTreeNode *bparent = path[--idx];
            GS_ASSERT (!bparent || bparent->left == balance || bparent->right == balance);
            left_node = (bparent && balance == bparent->left);

            if(balance->balance < -1 || balance->balance > 1)
            {
                balance = _TreeNodeBalance(balance);
                if (!bparent)
                {
                    m_struRoot = balance;
                }
                else if (left_node)
                {
                    bparent->left = balance;
                }
                else
                {
                    bparent->right = balance;
                }
            }

            if (balance->balance != 0 || !bparent)
                break;

            if (left_node)
                bparent->balance += 1;
            else
                bparent->balance -= 1;

            balance = bparent;
        }
        m_iNodes--;
        return node;
}

void CBTree::Clear( BOOL bLock)
{
    StruGSPTreeNode *node;
    StruGSPTreeNode *next;


    if( bLock )
    {
        m_csWRMutex.LockWrite();
    }

    node = GetFirstNode ();

    while (node){
        next = _TreeNodeNext (node);     
        FreeTreeNode( node, TRUE, TRUE );
        node = next;
    }
    m_struRoot = NULL;
    m_iSize = 0;
    if( bLock )
    {
        m_csWRMutex.UnlockWrite();
    }

}

CBTree::StruGSPTreeNode* CBTree::GetFirstNode(void)
{
    StruGSPTreeNode *tmp;

    if (!m_struRoot)
    {
        return NULL;
    }

    tmp = m_struRoot;

    while (tmp->left_child)
    {
        tmp = tmp->left;
    }

    return tmp;
}

CBTree::CIterator  CBTree::Travel( CBTree::EnumTravelMode eMode)
{
    CIterator csIt(this,eMode);
    StruGSPTreeNode *pNode = NULL;


    if (!m_struRoot )
    {          
        return csIt;
    }


    switch (eMode)
    {

    case IN_ORDER: 
        {
            pNode =  m_struRoot;

            if( pNode->left_child )
            {                   
                while( pNode->left_child  )
                {
                    if( csIt.m_iTrPath==GSS_MAX_GTREE_HEIGHT)
                    {

                        GS_ASSERT(0);
                        return CIterator(this,eMode);
                    }
                    csIt.m_pTrPath[csIt.m_iTrPath] = pNode;
                    csIt.m_bHadTr[csIt.m_iTrPath] = 1;
                    csIt.m_iTrPath++; 
                    pNode = pNode->left;
                }
                csIt.m_pTrPath[csIt.m_iTrPath] = pNode;
                csIt.m_bHadTr[csIt.m_iTrPath] = 0;
                csIt.m_iTrPath++;
            }
            else if(pNode->right_child )
            {
                csIt.m_pTrPath[csIt.m_iTrPath] = pNode;
                csIt.m_bHadTr[csIt.m_iTrPath] = 0;
                csIt.m_iTrPath++;                
            }

        }
        break;
    case OUT_ORDER: 
        {
            pNode =  m_struRoot;

            if( pNode->right_child )
            {                   
                while( pNode->right_child  )
                {
                    if( csIt.m_iTrPath==GSS_MAX_GTREE_HEIGHT)
                    {

                        GS_ASSERT(0);
                        return CIterator(this,eMode);
                    }
                    csIt.m_pTrPath[csIt.m_iTrPath] = pNode;
                    csIt.m_bHadTr[csIt.m_iTrPath] = 1;
                    csIt.m_iTrPath++; 
                    pNode = pNode->right;
                }
                csIt.m_pTrPath[csIt.m_iTrPath] = pNode;
                csIt.m_bHadTr[csIt.m_iTrPath] = 0;
                csIt.m_iTrPath++;
            }
            else if(pNode->left_child )
            {
                csIt.m_pTrPath[csIt.m_iTrPath] = pNode;
                csIt.m_bHadTr[csIt.m_iTrPath] = 0;
                csIt.m_iTrPath++;                
            }

        }
        break;
    case PRE_ORDER:
        {        
            //中序遍历

            pNode =  m_struRoot;
            if( pNode->right_child || pNode->left_child )
            {           
                csIt.m_pTrPath[0] = pNode;
                csIt.m_bHadTr[0] = 0;             
                csIt.m_iTrPath = 1;
            }

        }
        break;
    case POST_ORDER:
        {       
            pNode =  m_struRoot;

            while( pNode->left_child || pNode->right_child)
            {
                while( pNode->left_child  )
                {
                    if( csIt.m_iTrPath==GSS_MAX_GTREE_HEIGHT)
                    {

                        GS_ASSERT(0);
                        return CIterator(this,eMode);
                    }
                    csIt.m_pTrPath[csIt.m_iTrPath] = pNode;
                    csIt.m_bHadTr[csIt.m_iTrPath] = 1;
                    csIt.m_iTrPath++; 
                    pNode = pNode->left;
                }                    

                if( pNode->right_child )
                {
                    csIt.m_pTrPath[csIt.m_iTrPath] = pNode;
                    csIt.m_bHadTr[csIt.m_iTrPath] = 2;
                    csIt.m_iTrPath++; 
                    pNode = pNode->right;
                }
            }
        }
        break;
    default :
        return csIt;
        ;
    }
    csIt.m_bSecond = TRUE;
    csIt.m_pFirst = pNode->data;
    csIt.m_pKey = pNode->key;
    csIt.m_pNode = pNode;     
    return csIt;

}

void CBTree::Lock(BOOL bRead)
{
    if( bRead )
    {
        m_csWRMutex.LockReader();
    }
    else
    {
        m_csWRMutex.LockWrite();
    }
}

void CBTree::Unlock(BOOL bRead)
{
    if( bRead )
    {
        m_csWRMutex.UnlockReader();
    }
    else
    {
        m_csWRMutex.UnlockWrite();
    }
}


/*
****************************************
brief : CIterator 的实现
****************************************
*/


CBTree::CIterator::CIterator(void)
:m_pOwner(NULL) 
,m_pNode(NULL)
,m_pFirst(NULL)
,m_bSecond(FALSE)
,m_pKey(NULL)
,m_eTrMode(CBTree::NONE)
,m_iTrPath(0)

{

}

CBTree::CIterator::CIterator(CBTree *pObject,EnumTravelMode eTrMode )
:m_pOwner(pObject) 
,m_pNode(NULL)
,m_pFirst(NULL)
,m_bSecond(FALSE)
,m_pKey(NULL)
,m_eTrMode(eTrMode)
,m_iTrPath(0) 

{

}

CBTree::CIterator::CIterator( const CBTree::CIterator &csDest)
:m_pOwner(csDest.m_pOwner) 
,m_pNode(csDest.m_pNode)
,m_pFirst(csDest.m_pFirst)
,m_bSecond(csDest.m_bSecond)
,m_pKey(csDest.m_pKey)
,m_eTrMode(csDest.m_eTrMode)
,m_iTrPath(csDest.m_iTrPath)
{
    memcpy( m_pTrPath, csDest.m_pTrPath, GSS_MAX_GTREE_HEIGHT*sizeof(StruGSPTreeNode*) ); 
    memcpy( m_bHadTr, csDest.m_bHadTr, GSS_MAX_GTREE_HEIGHT*sizeof(short) ); 
}
CBTree::CIterator::~CIterator( void )
{

}

const CBTree::CIterator &CBTree::CIterator::operator++(int)
{
    StruGSPTreeNode *pNext = NULL;
    if( m_bSecond && m_eTrMode != CBTree::NONE && m_pNode )
    {   


        switch (m_eTrMode)
        {


        case IN_ORDER:
            {   

                while( m_iTrPath )
                {
                    m_iTrPath--;
                    m_pNode  = m_pTrPath[m_iTrPath];

                    if( m_bHadTr[m_iTrPath] == 1 )
                    {
                        pNext = m_pNode;
                        m_bHadTr[m_iTrPath] = 0;
                        m_iTrPath++;                       
                        goto exit_fun;
                    }

                    if( m_bHadTr[m_iTrPath] == 0 )
                    {
                        m_bHadTr[m_iTrPath] = 2;
                        if( m_pNode->right_child  ) 
                        {
                            m_iTrPath++;
                            pNext = m_pNode->right;
                            while(pNext->left_child )
                            {
                                m_pTrPath[m_iTrPath] = pNext;
                                m_bHadTr[m_iTrPath ] = 1;
                                m_iTrPath++;
                                pNext = pNext->left;
                            }
                            m_pTrPath[m_iTrPath] = pNext;
                            m_bHadTr[m_iTrPath ]= 0;
                            m_iTrPath++;
                            goto exit_fun;
                        }
                    }
                }            
            }
            break;
        case OUT_ORDER :
            {   

                while( m_iTrPath )
                {
                    m_iTrPath--;
                    m_pNode  = m_pTrPath[m_iTrPath];

                    if( m_bHadTr[m_iTrPath] == 1 )
                    {
                        pNext = m_pNode;
                        m_bHadTr[m_iTrPath] = 0;
                        m_iTrPath++;                       
                        goto exit_fun;
                    }

                    if( m_bHadTr[m_iTrPath] == 0 )
                    {
                        m_bHadTr[m_iTrPath] = 2;
                        if( m_pNode->left_child  ) 
                        {
                            m_iTrPath++;
                            pNext = m_pNode->left;
                            while(pNext->right_child )
                            {
                                m_pTrPath[m_iTrPath] = pNext;
                                m_bHadTr[m_iTrPath ] = 1;
                                m_iTrPath++;
                                pNext = pNext->right;
                            }
                            m_pTrPath[m_iTrPath] = pNext;
                            m_bHadTr[m_iTrPath ]= 0;
                            m_iTrPath++;
                            goto exit_fun;
                        }
                    }
                }            
            }
            break;

        case PRE_ORDER:
            {   

                while( m_iTrPath )
                {
                    m_iTrPath--;
                    m_pNode  = m_pTrPath[m_iTrPath];
                    if( m_bHadTr[m_iTrPath]==0 )
                    {    
                        m_bHadTr[m_iTrPath] = 1;
                        if( m_pNode->left_child )
                        {

                            pNext = m_pNode->left;
                            m_iTrPath++;
                            m_pTrPath[m_iTrPath] = pNext;
                            m_bHadTr[m_iTrPath] = 0;
                            m_iTrPath++;
                            goto exit_fun;
                        }                      
                    }
                    if( m_bHadTr[m_iTrPath]==1 )
                    {
                        m_bHadTr[m_iTrPath] = 2;
                        if( m_pNode->right_child  )
                        {                                
                            pNext = m_pNode->right;                            
                            m_iTrPath++;
                            m_pTrPath[m_iTrPath] = pNext;
                            m_bHadTr[m_iTrPath] = 0;
                            m_iTrPath++;
                            goto exit_fun;
                        } 
                    }                  
                }            
            }
            break;

        case POST_ORDER :
            {   
                while( m_iTrPath )
                {
                    m_iTrPath--;
                    m_pNode  = m_pTrPath[m_iTrPath];                    

                    if( m_bHadTr[m_iTrPath]==1 )
                    {
                        m_bHadTr[m_iTrPath] = 2;
                        if( m_pNode->right_child  )
                        {      
                            m_iTrPath++;
                            m_pNode = m_pNode->right;

                            while( m_pNode->left_child || m_pNode->right_child)
                            {
                                while( m_pNode->left_child  )
                                {
                                    GS_ASSERT_EXIT(m_iTrPath!=GSS_MAX_GTREE_HEIGHT, -1); 
                                    m_pTrPath[m_iTrPath] = m_pNode;
                                    m_bHadTr[m_iTrPath] = 1;
                                    m_iTrPath++; 
                                    m_pNode = m_pNode->left;
                                }                    

                                if( m_pNode->right_child )
                                {
                                    m_pTrPath[m_iTrPath] = m_pNode;
                                    m_bHadTr[m_iTrPath] = 2;
                                    m_iTrPath++; 
                                    m_pNode = m_pNode->right;
                                }
                            }
                            pNext = m_pNode;
                            goto exit_fun;
                        } 
                    } 
                    if( m_bHadTr[m_iTrPath]==2 )
                    {
                        m_bHadTr[m_iTrPath] = 3;
                        pNext = m_pNode;                       
                        m_iTrPath++;                       
                        goto exit_fun;
                    }
                }            
            }
            break;
        default :
            ;
            break;
        } //end switch();


    }
exit_fun :
    if( pNext )
    {
        m_pFirst = pNext->data;
        m_pKey = pNext->key;
        m_bSecond = TRUE;
        m_pNode = pNext;
    }
    else
    {
        m_pFirst = NULL;
        m_pKey = NULL;
        m_bSecond = FALSE;
        m_pNode = NULL;
    }
    return *this;
}


CBTree::CIterator &CBTree::CIterator::operator=(const CBTree::CIterator &csDest)
{
    m_pOwner = csDest.m_pOwner;
    m_pNode = csDest.m_pNode;
    m_pFirst = csDest.m_pFirst;
    m_bSecond = csDest.m_bSecond;
    m_pKey = csDest.m_pKey;
    m_eTrMode = csDest.m_eTrMode;
    m_iTrPath = csDest.m_iTrPath;

    memcpy( m_pTrPath, csDest.m_pTrPath, GSS_MAX_GTREE_HEIGHT*sizeof(StruGSPTreeNode*) );
    memcpy( m_bHadTr, csDest.m_bHadTr, GSS_MAX_GTREE_HEIGHT*sizeof(short) );
    return *this;
}

bool CBTree::CIterator::operator == (const CBTree::CIterator &csDest) const
{
    return  m_pOwner == csDest.m_pOwner &&
        m_pNode == csDest.m_pNode &&
        m_pFirst == csDest.m_pFirst &&
        m_bSecond == csDest.m_bSecond &&
        m_eTrMode == csDest.m_eTrMode &&
        m_iTrPath == csDest.m_iTrPath;

}


BOOL CBTree::CIterator::SetFirst(void *pData)
{
    if( !m_pNode )
    {
        return FALSE;
    }
    m_pNode->data = pData;
    m_pFirst = pData;
    return TRUE;
}

 namespace GSP
 {
     GS_API int BTreePointerCmp( const void *key1, const void *key2)
    {
        if( key1>key2 )
        {
            return 1;
        }
        if( key1<key2)
        {
            return -1;
        }
        return 0;
    }

    GS_API int BTreeStringCmp(const void *key1, const void *key2)
    {
        return strcmp((const char *)key1, (const char *)key2);
    }

    GS_API int  BTreeUINT32Cmp( const void *key1, const void *key2)
    {
        if((UINT32) key1>(UINT32)key2 )
        {
            return 1;
        }
        if( (UINT32)key1<(UINT32)key2)
        {
            return -1;
        }
        return 0;
    }

 }


 