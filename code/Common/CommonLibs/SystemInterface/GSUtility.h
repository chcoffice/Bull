/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : GSUTILITY.H
Author :  zouyx
Version : 0.0.0.0
Date: 2011/9/14 9:44
Description: 一些供的功能函数和模块
********************************************
*/

#ifndef _GS_H_GSUTILITY_H_
#define _GS_H_GSUTILITY_H_

#include "GSType.h"
#include <sstream>
#include <algorithm>
#include <vector>
#include <string>
#include "ISystemLayInterface.h"

#ifdef LINUX
#include <stdarg.h>
#endif

/*
*********************************************************************
*
*@brief :  链表操作
*
*********************************************************************
*/
template<class T>
struct _StruGSList
{
	struct _StruGSList *next; 
	struct _StruGSList *prev;
	T Data;

	_StruGSList(void)
	{
		Init();
	}
	~_StruGSList(void)
	{
		Init();
	}

	BOOL Empty(void) const
	{
		return (next==this && prev==this);
	}

	INLINE void Init(void)
	{
		next = this;
		prev = next;		
	}

	INLINE void Unlink(void)
	{
		if( !Empty() )
		{                     
			next->prev = prev;
			prev->next = next;
			next = this;
			prev = this;
		}
	}



	/*
	*********************************************
	Function : RemoveFront
	DateTime : 2011/9/14 9:30
	Description :  移除第一个链表
	Input :  head 链表头
	Output : 
	Return : 链表为空返回 NULL
	Note :   
	*********************************************
	*/
	static INLINE struct _StruGSList<T> *RemoveFront( struct _StruGSList<T> *head )
	{
		if( head->Empty() )
		{
			return NULL;
		}
		struct _StruGSList<T> *pRet = head->next;
		pRet->Unlink();
		return pRet;
	}

	/*
	*********************************************
	Function : Add
	DateTime : 2011/9/14 9:32
	Description :  添加数据到 prev 和 next 之间
	Input :  
	Output : 
	Return : 
	Note :   
	*********************************************
	*/
	static INLINE void Add( struct _StruGSList<T> *pnew,
		struct _StruGSList<T> *prev,
		struct _StruGSList<T> *next)
	{
		next->prev = pnew;
		pnew->next = next;
		pnew->prev = prev;
		prev->next = pnew;
	}

	/*
	*********************************************
	Function : AddInsert
	DateTime : 2011/9/14 9:33
	Description :   添加数据到 prev 和 next 之间
	Input :  
	Output : 
	Return : 
	Note :   
	*********************************************
	*/
	static INLINE void AddInsert(struct _StruGSList<T> *pnew,
		struct _StruGSList<T> *prev,
		struct _StruGSList<T> *next)
	{
		Add(pnew, prev, next);
	}


	/**
	*@brief 把新节点pnew 加到链表head的开始位置
	*/
	static INLINE void AddFirst(struct _StruGSList<T> *pnew, struct _StruGSList<T> *head)
	{
		Add(pnew, head, head->next);
	}

	/**
	*@brief 把新节点pnew 加到链表head的结尾
	*/	
	static INLINE void AddTail(struct _StruGSList<T> *pnew, struct _StruGSList<T> *head)
	{
		Add(pnew, head->prev, head);
	}

	static INLINE void Swap( struct _StruGSList<T> &stSrcHead, 
							struct _StruGSList<T> &stDestHead )
	{
		struct _StruGSList<T> stTemp;
		stTemp.Init();

		if( !stSrcHead.Empty() )
		{
			stTemp.prev = stSrcHead.prev;
			stTemp.next = stSrcHead.next;
		}
		
		if( stDestHead.Empty() )
		{
			stSrcHead.Init();
		}
		else
		{
			stSrcHead.next = stDestHead.next;
			stSrcHead.next->prev = &stSrcHead;
			stSrcHead.prev = stDestHead.prev;
			stSrcHead.prev->next = &stSrcHead;
		}

		if( stTemp.Empty() )
		{
			stDestHead.Init();
		}
		else
		{
			stDestHead.next = stTemp.next;
			stDestHead.next->prev = &stDestHead;
			stDestHead.prev = stTemp.prev;
			stDestHead.prev->next = &stDestHead;
		}


	}


}; //end _StruGSList



typedef _StruGSList<void*> StruGSListNode;


template<class T>
class CGSListMemCache
{


public :
	typedef _StruGSList<T> StruListNode;

	CGSListMemCache( UINT iSize = 10000 )	
		:m_csMutex()
	{
		m_pCacheMem = NULL;
		m_pMemBegin = NULL;
		m_pMemEnd = NULL;

	
		m_pMemBegin = new StruListNode[iSize+1];
		if( m_pMemBegin )
		{
			m_pMemEnd = &m_pMemBegin[iSize];
			for( UINT i=0; i<iSize; i++  )
			{			
				m_pMemBegin[i].next = m_pCacheMem;
				m_pCacheMem =&m_pMemBegin[i];
			}
		}

	}

	~CGSListMemCache(void)
	{
		m_csMutex.Lock();
		if( m_pMemBegin )
		{
			delete[] m_pMemBegin;
		}
		m_pCacheMem = NULL;
		m_pMemBegin = NULL;
		m_pMemEnd = NULL;
		m_csMutex.Unlock();
	}
	

	StruListNode *Get(void)
	{
		StruListNode *pRet = NULL;
		m_csMutex.Lock();
		if( m_pCacheMem )
		{
			pRet = m_pCacheMem;
			m_pCacheMem = pRet->next;
			pRet->Init();
		}
		else
		{
			pRet = new StruListNode;
			if( pRet )
			{
				pRet->Init();
			}
		}
		m_csMutex.Unlock();
		return pRet;
	}

	void Free( StruListNode *pNode )
	{
		
		m_csMutex.Lock();
		if( pNode>=m_pMemBegin && pNode<m_pMemEnd )
		{
			pNode->Init();
			pNode->next = m_pCacheMem;
			m_pCacheMem = pNode;
		}
		else
		{
			delete pNode;
		}
		m_csMutex.Unlock();


	}
private :
	CGSMutex m_csMutex;
	StruListNode *m_pCacheMem; //缓冲内存池
	StruListNode *m_pMemBegin;
	StruListNode *m_pMemEnd;
	
};




/*
*********************************************************************
*
*@brief : 字符串操作功能
*
*********************************************************************
*/



namespace  GSStrUtil
{


	// 删除字符串左边空白
	CGSString TrimLeft(const CGSString& str);

	// 删除字符串右边空白
	CGSString TrimRight(const CGSString& str);

	// 删除字符串左右两边空白
	CGSString Trim(const CGSString& str);

	// 转换为小写字符串
	CGSString ToLower(const CGSString& str);

	// 转换为大写字符串
	CGSString ToUpper(const CGSString& str); 

	// 分解字符串
	void Split(std::vector<CGSString> &csResult, const CGSString& str, const CGSString& delimiters);

	//不分大小写的比较
	BOOL EqualsIgnoreCase( const CGSString& strSrc, const CGSString& strDest);


	// 字符串类型转换模板函数
	// 字符串类型转换模板函数
	template<class T> T ToNumber(const CGSString& str);
	template<class T> T ToHexNumber(const CGSString& str);
	template<class T> CGSString ToString(const T value);
	template<class T> CGSString ToHexString(const T value);



	/*
	*********************************************
	Function : ToNumber
	DateTime : 2011/1/20 15:23
	Description : 将十进制字符串转换为数值
	Input :
	Output :
	Return :
	Note :
	*********************************************
	*/
	template<class T> 
	T ToNumber(const CGSString& str)
	{
		T value;
		std::istringstream iss(str.c_str());
		iss >> value;
		return value;
	}

	/*
	*********************************************
	Function :
	DateTime : 2011/1/20 15:23
	Description : 将十六进制字符串转换为数值
	Input :
	Output :
	Return :
	Note :
	*********************************************
	*/
	template<class T> 
	T ToHexNumber(const CGSString& str) 
	{
		T value;
		std::istringstream iss(str.c_str());
		iss >> hex >> value;
		return value;
	}


	template<class T> 
	CGSString ToString(const T value) 
	{
		std::ostringstream oss;
		oss << value;
		return oss.str();
	}


	template<class T> 
	CGSString ToHexString(const T value) 
	{
		std::ostringstream oss;
		oss << "0x" << hex << value;
		return oss.str();
	}

	BOOL Format(CGSString &strOString, const char* czFormat, ...);
	BOOL VFormat(CGSString &strOString,const char *czFormat, va_list ap );
	BOOL AppendWithFormat(CGSString &strIOString, const char* czFormat, ...);

	template<class T> 
	INT CheckValueRange(T &vValue, const T vMin, const T vMax)
	{
		if( vValue<vMin )
		{
			vValue = vMin;
			return -1;
		}
		else if( vValue > vMax )
		{
			vValue = vMax;
			return 1;
		}
		return 0;
	}

};

#endif //end _GS_H_GSUTILITY_H_