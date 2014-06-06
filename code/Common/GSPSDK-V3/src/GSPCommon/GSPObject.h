#ifndef GSP_GSPOBJECT_DEF_H
#define GSP_GSPOBJECT_DEF_H
#include "GSPConfig.h"
#include "GSPErrno.h"


/*
******************************************
Copyright (C), 2010-2011, GOSUN
Filename : GSPOBJECT.H
Author :  zouyx
Version : 0.1.0.0
Date: 2010/5/14 14:17
Description: 在GSP SDK 中使用的公共基础类， 其他类都是该类的子类
********************************************
*/





/*
*********************************************
ClassName :  CGSPObject
DateTime : 2010/5/28 9:06
Description :   GSP 协议栈用到的基本类
Note :
*********************************************
*/

namespace GSP
{




	class CGSPObject : public CGSObject
	{
	public:
		static BOOL g_bModuleRunning;
		static CGSMutex g_csMutex;
		CGSPObject(void);
		virtual ~CGSPObject(void);

		void *operator new(size_t iSize);
		void operator delete(void *pBuffer);

	};




	class CRefObject :
		public CGSPObject
	{


	private :
		GSAtomicInter m_iRef; //引用计数        
	protected :
		CRefObject()
			: CGSPObject()            
			,m_iRef(1)
		{

		}    
		virtual ~CRefObject(void)
		{

		}
	public :
#if defined(DEBUG) || defined(_DEBUG)
		INT64 m_iiTest;
#endif
		/*
		*********************************************
		Function : GetRefCounts
		DateTime : 2010/8/17 14:21
		Description : 获取对象被引用的次数
		Input :
		Output :
		Return :  返回应用的次数
		Note :
		*********************************************
		*/
		virtual UINT GetRefCounts(void) const
		{
			return m_iRef;
		}

		/*
		*********************************************
		Function :  RefObject
		DateTime : 2010/8/17 14:23
		Description :  引用对象
		Input :
		Output :
		Return :  将增加引用
		Note :   调用该函数将引发 OnRefObjectEvent 
		*********************************************
		*/
		virtual void RefObject(void)
		{

			//增加引用怎么可能<= 1 ???
			GS_ASSERT_EXIT(AtomicInterInc(m_iRef)>1, -1);
			OnRefObjectEvent();
		}


		/*
		*********************************************
		Function :  UnrefObject
		DateTime : 2010/8/17 14:25
		Description : 减少对象引用
		Input :
		Output :
		Return :
		Note :  当对象没有被人引用时对象间被释放
		调用该函数将引发 OnUnrefObjectEvent 
		*********************************************
		*/
		virtual void UnrefObject(void)
		{
			OnUnrefObjectEvent();
			INT iRef = AtomicInterDec(m_iRef);           
			GS_ASSERT_EXIT(iRef>-1, -1); //出现负数 ？？？？
			if( iRef<1 ) 
			{
				delete this;       
			}
		}




		/*
		*********************************************
		Function :  Clone
		DateTime : 2010/8/17 14:25
		Description : 返回对象的拷贝,所有的内容将被复制
		Input :
		Output :
		Return : 返回新的对象
		Note :
		*********************************************
		*/
		virtual CRefObject *Clone(void) const
		{
			return NULL;
		}
	protected :
		virtual void OnRefObjectEvent(void)
		{

		}

		virtual void OnUnrefObjectEvent(void)
		{

		}
	private :
		CRefObject(CRefObject &csDest);
		CRefObject &operator=(CRefObject &csDest);


	};

#if defined(DEBUG) || defined(_DEBUG)

#define REF_OBJ_SET_TEST_TICKS( pObj ) do{ pObj->m_iiTest = DoGetTickCount(); }while(0)
#define REF_OBJ_GET_TEST_TICKS( pObj)  ((pObj)->m_iiTest)

#else
#define REF_OBJ_SET_TEST( pObj ) do{}while(0)
#define REF_OBJ_GET_TEST_TICKS( pObj)  (0xFFFFFFFFLL)
#endif


#define SAFE_DESTROY_REFOBJECT(ppObj) \
	do{ if( *(ppObj) ){ (*(ppObj))->UnrefObject(); *(ppObj)=NULL ; } } while(0)

#define SAFE_DELETE_OBJECT(ppObj) \
	do{ if( *(ppObj) ){  delete (*(ppObj)); *(ppObj)=NULL ; } } while(0)

};


#define CLASS_NO_COPY(classname ) \
	private : \
		classname( const classname &csDest ); \
		classname &operator=( const classname &csDest );

#endif
