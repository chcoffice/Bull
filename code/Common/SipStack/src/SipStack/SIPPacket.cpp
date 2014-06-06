#include "SIPPacket.h"
#include "SipStack.h"

using namespace GSSIP;

#define MyMemoryFree(x) free(x)
#define MyMemoryAlloc(size) malloc(size)


/*
*********************************************************************
*
*@brief : CRefObject
*
*********************************************************************
*/
void CRefObject::Ref(void)
{
	AtomicInterInc(m_iRefs);
}

void CRefObject::Unref(void)
{
	if( AtomicInterDec(m_iRefs) < 1 )
	{
		delete this;
	}
}


/*
*********************************************************************
*
*@brief : CPacket
*
*********************************************************************
*/
CPacket::CPacket( EnumDirect eDirect, EnumBodyType eBodyType )
:CRefObject()
{
	m_eDirect = eDirect;
	m_eBodyType = eBodyType;
	m_bBody = NULL;
	m_iBodyLength = 0;
	m_iBodyBufSize = 0;


	m_iSequence = 0;
	m_iCmdId = 0;
	m_iSubexpires = 0;
	m_eMethod = SIP_METHOD_INFOEX;
}

CPacket::~CPacket(void)
{
	if ( m_bBody != NULL )
	{
		MyMemoryFree(m_bBody);
		m_bBody = NULL;
		m_iBodyLength = 0;
		m_iBodyBufSize = 0;
	}
}

BOOL	CPacket::SetBody( const CGSString &strBody )
{
	GS_ASSERT_RET_VAL(m_eDirect==eBODY_STRING, FALSE );
	m_strBody = strBody;
	return TRUE;
}

BOOL   CPacket::SetBody( BYTE *bBuf, int iSize)
{
	GS_ASSERT_RET_VAL(m_eDirect==eBODY_BINARY, FALSE );
	m_iBodyLength = 0;
	if( m_iBodyBufSize<iSize )
	{
		if( m_bBody )
		{
			MyMemoryFree(m_bBody);
			m_iBodyBufSize = 0;
		}
		m_bBody = (BYTE*) MyMemoryAlloc(iSize);
		GS_ASSERT_RET_VAL(m_bBody, FALSE );
		m_iBodyBufSize = iSize;
	}
	::memcpy(m_bBody, bBuf, iSize);
	m_iBodyLength = iSize;
	return TRUE;
}

