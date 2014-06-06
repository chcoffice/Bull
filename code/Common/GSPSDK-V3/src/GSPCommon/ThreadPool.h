#ifndef GSS_THREADPOOL_DEF_H
#define GSS_THREADPOOL_DEF_H




/*
******************************************
Copyright (C), 2010-2011, GOSUN
Filename : GSPTHREADPOOL.H
Author :  zouyx
Version : 0.1.0.0
Date: 2010/5/18 16:38
Description: 线程池的实现
********************************************
*/


#include "GSPObject.h"




namespace GSP
{

	class CThreadPool;


	/*
	*********************************************
	Function :   GSPThreadPoolFunctionPtr
	DateTime : 2010/5/19 8:38
	Description :   线程池的工作函数回调
	Input : pcsPool 返回所属的线程池对象
	Input : pUserData 线程的用户参数
	Output :
	Return :
	Note :
	*********************************************
	*/




	class CTaskThread;


	class CGSPThreadPool : public CObjThreadPool
	{		
	public :
		CGSPThreadPool( const char *czName=NULL );
	
		virtual ~CGSPThreadPool(void);

		static void InitModule(void);
		static void UninitModule(void);
		static INLINE CGSThreadGlobal *MyGlobal(void)
		{
			GS_ASSERT(s_pThreadGlobal);
			return s_pThreadGlobal;
		}
	private :
		static CGSThreadGlobal *s_pThreadGlobal;
		CGSString m_strName;

	};



};

#endif
