/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : REFOBJVECTOR.H
Author :  邹阳星
Version : 1.0.0.1
Date: 2013/5/15 15:03
Description: 数组的模板类
********************************************
*/

#ifndef _GS_H_REFOBJVECTOR_H_
#define _GS_H_REFOBJVECTOR_H_
#include "GSPObject.h"
#include <vector>

namespace GSP
{

template <class T>
class CRefObjVector :
	public CGSPObject
{
private :
	std::vector<T > m_vObj;
public:
	CRefObjVector(void) : CGSPObject(), m_vObj()
	{

	}
	CRefObjVector( CRefObjVector<T> &csDest ) : CGSPObject(), m_vObj
	{
		*this = csDest;
	}
	virtual ~CRefObjVector(void)
	{
		for( UINT i = 0; i<m_vObj.size(); i++ )
		{
			m_vObj[i]->UnrefObject();
		}
	}

	const std::vector<T> &GetStdVector(void)
	{
		return m_vObj;
	}

	UINT size(void)
	{
		return m_vObj.size();
	}

	T operator[](INT i)
	{
		return m_vObj[i];
	}

	CRefObjVector<T> operator=(CRefObjVector<T> &csDest)
	{
		if( this != &csDest )
		{
			m_vObj = csDest.m_vObj;
			for( UINT i = 0; i<m_vObj.size(); i++ )
			{
				m_vObj[i]->RefObject();
			}
		}
		return *this;

	}

	UINT max_size(void)
	{
		return m_vObj.size();
	}

	bool empty(void)
	{
		return m_vObj.empty();
	}

	void push_back( T pRefObj )
	{
		m_vObj.push_back(pRefObj);		
		pRefObj->RefObject();
	}

	void clear(void)
	{
		for( UINT i = 0; i<m_vObj.size(); i++ )
		{
			m_vObj[i]->UnrefObject();
		}
		m_vObj.clear();
	}

	void erase( UINT iIndex )
	{
		m_vObj[iIndex]->UnrefObject();
		m_vObj.erase(m_vObj.begin()+iIndex);
	}
};

} //end namespace GSP

#endif //end _GS_H_REFOBJVECTOR_H_
