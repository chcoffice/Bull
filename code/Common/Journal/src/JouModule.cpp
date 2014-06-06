#include "JouModule.h"
using namespace  JOU;

CJouModule::CJouModule( const STRING &strModuleName)
:CJouObj()
,m_pSrv(NULL)
,m_strModuleName(strModuleName)
{
	m_eStatus = eST_UNINIT;
}

CJouModule::~CJouModule(void)
{

}

EnumJouErrno CJouModule::Init( CService *pServer )
{
	GS_ASSERT(m_eStatus==eST_UNINIT );
	m_pSrv = pServer;
	m_eStatus = eST_INIT;
	return eJOU_R_SUCCESS;
}

void CJouModule::Uninit(void)
{
	GS_ASSERT(m_eStatus!=eST_UNINIT );
	m_eStatus = eST_UNINIT;	
}

EnumJouErrno CJouModule::Start(void* pData)
{
	GS_ASSERT_RET_VAL(m_eStatus==eST_INIT, eJOU_E_NINIT);
	GS_ASSERT_RET_VAL(m_pSrv, eJOU_E_NINIT);
	m_eStatus = eST_RUNNING;	
	return eJOU_R_SUCCESS;
}

void CJouModule::Stop(void)
{
	GS_ASSERT_RET(m_eStatus==eST_RUNNING);
	m_eStatus = eST_INIT;
}