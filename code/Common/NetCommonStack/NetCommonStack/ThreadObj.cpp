#include "GSCommon.h"
#include "ThreadObj.h"

CThreadObj::CThreadObj()
{
	m_pUserFunc = NULL;
	m_pUserParam = NULL;
}

CThreadObj::~CThreadObj()
{

}

void CThreadObj::SetUserFunc(UserFunc pFunc, void *pParam)
{
	m_pUserFunc = pFunc;
	m_pUserParam = pParam;
}

void CThreadObj::UserFuncFinished()
{
	m_pUserFunc = NULL;
	m_pUserParam = NULL;
}

void CThreadObj::ThreadCommonDemon(CGSThread *gsTheadHandle,void* pParam)
{
	CThreadObj* pTh;

	pTh = (CThreadObj*)pParam;
	if(NULL == pTh)
	{
		return;
	}

	while(TRUE)
	{

	}
}