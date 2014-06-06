#include "Frame.h"
#include "GSPMemory.h"


namespace GSP
{
namespace CPAT
{

/*
*********************************************************************
*
*@brief : CPAT::CRefBuffer й╣ож
*
*********************************************************************
*/
CRefBuffer::~CRefBuffer(void)
{
	SAFE_DESTROY_REFOBJECT( &m_pData);
}

CRefBuffer::CRefBuffer(void)
{
	m_pData = NULL;
}

UINT CRefBuffer::GetDataSize(void) const
{
	if( m_pData )
	{
		return m_pData->m_iDataSize;
	}
	return 0;
}

void *CRefBuffer::GetData(void) const
{
	if( m_pData )
	{
		return (void*) m_pData->m_bBuffer;
	}
	return NULL;
}

void CRefBuffer::SetData(GSP::CGSPBuffer *pData)
{
	SAFE_DESTROY_REFOBJECT(&m_pData);
	m_pData = pData;
	if( m_pData )
	{
		m_pData->RefObject();
	}
}

CPAT::CRefBuffer &CRefBuffer::operator=(const CPAT::CRefBuffer &csDest)
{
	if( this!= &csDest )
	{
		SAFE_DESTROY_REFOBJECT(&m_pData);
		m_pData = csDest.m_pData;
		if( m_pData )
		{
			m_pData->RefObject();
		}
	}
	return *this;
}


	/*
	*********************************************************************
	*
	*@brief : CMyCpatFrame
	*
	*********************************************************************
	*/
UINT CMyCpatFrame::GetFrameSize(void)
{
	if( m_pMerge)
	{
		return m_pMerge->m_iDataSize;
	}
	return 0;
}

BOOL CMyCpatFrame::GetFrameData( CRefBuffer &csBuffer )
{
	csBuffer.SetData(m_pMerge);
	return m_pMerge!=NULL;
}

CMyCpatFrame::~CMyCpatFrame(void)
{
	SAFE_DESTROY_REFOBJECT((GSP::CGSPBuffer**)&m_pMerge);
}

CMyCpatFrame::CMyCpatFrame(void)
:CFrame()
{
	m_pMerge = NULL;
}

BOOL CMyCpatFrame::Set(GSP::CFrameCache *pFrame)
{
	SAFE_DESTROY_REFOBJECT( &m_pMerge);
	m_eMediaType = pFrame->m_stFrameInfo.eMediaType;
	m_iChnID = pFrame->m_stFrameInfo.iChnNo;
	m_pMerge = &pFrame->GetBuffer();
	m_pMerge->RefObject();
	
	return m_pMerge!=NULL;
}


} //end namespace GSP

} //end namespace GSP
