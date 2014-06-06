#ifndef THREAD_OBJECT_DEF_H
#define THREAD_OBJECT_DEF_H

typedef INT32 (*UserFunc)(void* pParam);

class CThreadObj
{
public:
	CThreadObj();
	virtual ~CThreadObj();
public:
	INT32 StartThread();
	INT32 StopThread();
	void SetUserFunc(UserFunc pFunc, void* pParam);
	void UserFuncFinished();
protected:
	static void ThreadCommonDemon(CGSThread *gsTheadHandle,void* pParam);
protected:
	CGSThread m_ThreadHandle;
	CGSCond m_ActiveCondition;

	UserFunc m_pUserFunc;
	void* m_pUserParam;

};

#endif