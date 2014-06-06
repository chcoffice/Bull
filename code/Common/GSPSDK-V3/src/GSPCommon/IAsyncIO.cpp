#include "IAsyncIO.h"
#include <stdlib.h>
#include <stdio.h>





using namespace GSP;


#define MAX_AIO_GLOBALS  90

CNetAsyncIOGlobal CNetAsyncIOGlobal::s_csGlobal;


CNetAsyncIOGlobal::CNetAsyncIOGlobal(void)
{
	m_iRefs = 0;
	m_iIDSeq = 0;
}

CNetAsyncIOGlobal::~CNetAsyncIOGlobal(void)
{
	GS_ASSERT(m_iRefs==0);
	if( m_iRefs>0 )
	{
		m_csMutex.Lock();
		m_iRefs=1;
		m_csMutex.Unlock();
		Uninit();
	}
}


CNetAsyncIOGlobal &CNetAsyncIOGlobal::Intance(void)
{
		return s_csGlobal;
}

void CNetAsyncIOGlobal::InitModule(void)
{
	s_csGlobal.Init();
}

void CNetAsyncIOGlobal::UninitModule(void)
{
	s_csGlobal.Uninit();
	GS_ASSERT(s_csGlobal.m_iRefs==0);
}

void CNetAsyncIOGlobal::Init(void)
{
	CGSAutoMutex locker(&m_csMutex);
	m_iRefs++;
	if( m_iRefs > 1 )
	{
		//已经初始化
		return;
	}
	
	CIAsyncIO *p = CreateAsyncIO(m_iIDSeq++);
	if( !p )
	{
		GS_ASSERT(0);
		m_iRefs--;		
		return;
	}
	if( !p->Init() )
	{
		GS_ASSERT(0);
		m_iRefs--;		
		delete p;		
		return;
	}
	m_vAIO.push_back(p);


}

void CNetAsyncIOGlobal::Uninit(void)
{
	CGSAutoMutex locker(&m_csMutex);
	GS_ASSERT(m_iRefs>0);
	m_iRefs--;
	if( m_iRefs==0 )
	{
		//卸载
		for(UINT i = 0; i<m_vAIO.size(); i++)
		{
			m_vAIO[i]->Uninit();
			delete m_vAIO[i];
		}
		m_vAIO.clear();
	}
}

CIAsyncIO *CNetAsyncIOGlobal::Register(CISocket *pSocket, CNetError &csError )
{
	CGSAutoMutex locker(&m_csMutex);
	if( m_iRefs==0 )
	{
		GS_ASSERT(0);
		csError.m_eErrno = eERRNO_SYS_ESTATUS;
		csError.m_strError = _GSTX("没有初始化");
		return NULL;
	}
	csError.m_eErrno = eERRNO_ENONE;
	for( UINT i = 0; i<m_vAIO.size(); i++ )
	{
		if( m_vAIO[i]->GetEmptyCounts() > 0 )
		{
			if( m_vAIO[i]->Register(pSocket, csError) )
			{
				csError.m_eErrno = eERRNO_SUCCESS;
				return m_vAIO[i];
			}
			else
			{
				GS_ASSERT(0);
			}
		}
	}
	if( csError.m_eErrno == eERRNO_ENONE  )
	{
		//不能容纳
		if( m_vAIO.size()>=MAX_AIO_GLOBALS )
		{
			csError.m_eErrno = eERRNO_SYS_EFLOWOUT;
			csError.m_iSysErrno = 0;
			csError.m_strError = _GSTX("超过管理能力");
		}
		else
		{
			CIAsyncIO *p = CreateAsyncIO(m_iIDSeq++);
			if( p )
			{
				if( p->Init() )
				{
					m_vAIO.push_back(p);
					if( p->Register(pSocket, csError) )
					{
						csError.m_eErrno = eERRNO_SUCCESS;
						return p;
					}
				}
				else
				{
					GS_ASSERT(0);				
					delete p;		
					csError.m_eErrno = eERRNO_SYS_EOPER;
					csError.m_iSysErrno = 0;
					csError.m_strError = _GSTX("系统操作错误");
				}
			}
			else
			{
				csError.m_eErrno = eERRNO_SYS_ENMEM;
				csError.m_iSysErrno = 0;
				csError.m_strError = _GSTX("分配内存失败");
			}			
		}
	}
	return NULL;
}

