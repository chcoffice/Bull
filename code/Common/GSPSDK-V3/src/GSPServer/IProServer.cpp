#include "IProServer.h"
#include "IServer.h"

using namespace GSP;

/*
*********************************************************************
*
*@brief : CISrvSession µÄÊµÏÖ
*
*********************************************************************
*/

GSAtomicInter CISrvSession::s_iAutoIDSequence = 0;

CISrvSession::CISrvSession(CIProServer *pProServer)
:CGSPObject()
,m_iAutoID((UINT32)AtomicInterInc(s_iAutoIDSequence))
,m_strClientIPInfo("")
,m_pProServer(pProServer)
{
	bzero( &m_stClientInfo, sizeof(m_stClientInfo));
	m_stClientInfo.eProtocol = pProServer->Protocol();
}
