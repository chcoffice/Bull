#ifndef GSS_BTREE_DEF_H
#define GSS_BTREE_DEF_H


/*
******************************************
Copyright (C), 2010-2011, GOSUN
Filename : GSPBTREE.H
Author :  zouyx
Version : 0.1.0.0
Date: 2010/6/8 17:03
Description: BTree   right>node>left
********************************************
*/

#include "GSPObject.h"

namespace GSP
{




#define GSS_MAX_GTREE_HEIGHT 50

GS_API int BTreePointerCmp( const void *key1, const void *key2);
GS_API int BTreeStringCmp(const void *key1, const void *key2);
GS_API int BTreeUINT32Cmp( const void *key1, const void *key2);

class CBTree  : public CGSPObject
{
	
public :	//
	typedef struct _StruGSPTreeNode 
	{
		void  * key;         /* key for this node */
		void  *data;       /* value stored at this node */
		_StruGSPTreeNode *left;        /* left subtree */
		_StruGSPTreeNode *right;       /* right subtree */
		INT16      balance;     /* height (left) - height (right) */
		BYTE     left_child;  
		BYTE     right_child;

		_StruGSPTreeNode(void)
		{
			bzero( this, sizeof(struct _StruGSPTreeNode) );
		}

		~_StruGSPTreeNode(void)
		{
			bzero( this, sizeof(struct _StruGSPTreeNode) );
		}
	};

	typedef struct _StruGSPTreeNode StruGSPTreeNode;
    typedef enum
    {
        NONE,
        PRE_ORDER,  // node->left->right;
        IN_ORDER,    //left->node->right;  由小到大
        POST_ORDER,  //left->right->node
        OUT_ORDER,    //right->node->left;  由大到小
    }EnumTravelMode;

    class CIterator  : public CGSPObject
    {
       
    private :
        friend class CBTree;
        CBTree *m_pOwner;        
        StruGSPTreeNode *m_pNode;       
        void *m_pFirst;
        BOOL m_bSecond;
        void *m_pKey;
        EnumTravelMode m_eTrMode;
        StruGSPTreeNode *m_pTrPath[GSS_MAX_GTREE_HEIGHT];
        short m_iTrPath;  
        short m_bHadTr[GSS_MAX_GTREE_HEIGHT];

    public :

        CIterator(void);
        CIterator( const CIterator &csDest);
        ~CIterator( void );

        const CIterator &operator++(int);
        CIterator &operator=(const CIterator &csDest); 
        bool operator==(const CIterator &csDest) const;
        BOOL SetFirst(void *pData);

        INLINE BOOL Second(void) const
        {
            return m_bSecond;
        }

        INLINE void *First(void)
        {
            return  m_pFirst;
        }
        
        INLINE const void *Key(void)
        {
            return m_pKey;
        }

        INLINE CBTree *BTree(void)
        {
            return m_pOwner;
        }

        
    private :
        CIterator(CBTree *pObject,EnumTravelMode eTrMode=CBTree::NONE);
    };

private :
	friend class CIterator;
	StruGSPTreeNode *m_struRoot;	
	FuncPtrCompares m_fnKeyCmp;
	UINT32 m_iNodes;
	CGSWRMutex m_csWRMutex;

	FuncPtrFree m_fnKeyFree;
	FuncPtrFree m_fnDataFree;
	UINT m_iSize;
public :

    CBTree(FuncPtrCompares fnKeyCmp = BTreePointerCmp);
    virtual ~CBTree(void);

    void SetFreeFunction(FuncPtrFree fnKeyFree, FuncPtrFree fnDataFree); 

     CBTree::CIterator  Insert( void *key, void *data, BOOL bLock = TRUE );

     CBTree::CIterator Find( const void *key, BOOL bLock  = TRUE );

    void Remove(  CBTree::CIterator &csIt, BOOL bLock  = TRUE);

    CBTree::CIterator Remove( const void *key, BOOL bLock  = TRUE);

    void Erase( const void *key, BOOL bLock  = TRUE);

    void Erase(  CBTree::CIterator &csIt, BOOL bLock  = TRUE);

    
    void Clear( BOOL bLock  = TRUE);  
    
    CBTree::CIterator  Travel( EnumTravelMode eMode=IN_ORDER );

    void Lock(BOOL bRead = FALSE);

    void Unlock(BOOL bRead = FALSE);

    UINT Size(void);


	CBTree::StruGSPTreeNode *Root(void)
	{
		return m_struRoot;
	}

	CBTree::StruGSPTreeNode *NodeLeft(CBTree::StruGSPTreeNode *pNode )
	{
		return pNode->left;
	}

	CBTree::StruGSPTreeNode *NodeRight(CBTree::StruGSPTreeNode *pNode )
	{
		return pNode->right;
	}

	void *NodeData(CBTree::StruGSPTreeNode *pNode )
	{
		return pNode->data;
	}

	void *NodeKey(CBTree::StruGSPTreeNode *pNode)
	{
		return pNode->key;
	}

private :
    void  InsertInternal (  CBTree::CIterator &csIt,
                                void  *key,void  *  data);

    StruGSPTreeNode* RemoveInternal (const void *  key );

    StruGSPTreeNode *GetTreeNode(void);

    void FreeTreeNode(StruGSPTreeNode *pNode, BOOL bFreeData, BOOL bFreeKey);

    StruGSPTreeNode* GetFirstNode(void);

	 INLINE StruGSPTreeNode *_TreeNodeNext ( StruGSPTreeNode *node);
	 INLINE StruGSPTreeNode *_TreeNodePrevious ( StruGSPTreeNode *node);
	 StruGSPTreeNode *_TreeNodeRotateLeft ( StruGSPTreeNode *node);
	 StruGSPTreeNode *_TreeNodeRotateRight ( StruGSPTreeNode *node);
	 StruGSPTreeNode *_TreeNodeBalance ( StruGSPTreeNode *node);
};

};

#endif
