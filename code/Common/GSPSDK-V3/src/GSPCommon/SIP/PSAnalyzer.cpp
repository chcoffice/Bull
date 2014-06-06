#include "PSAnalyzer.h"

using namespace GSP;
using namespace GSP::SIP;

/*
*********************************************************************
*
*@brief : CPSProPacket
*
*********************************************************************
*/

CPSSlice::CPSSlice(void) : CRefObject()
{
	m_iPSHeaderType = PS_HEADER_NONE;
	m_pRefHeader = NULL;
	m_pRefPES = NULL;
	m_bPSHeader = NULL;
	m_iPSHeaderSize = 0;
	bzero(&m_stPesBody, sizeof(m_stPesBody));

}

CPSSlice::~CPSSlice(void)
{
	SAFE_DESTROY_REFOBJECT(&m_pRefHeader);
	SAFE_DESTROY_REFOBJECT(&m_pRefPES);
}

CPSSlice *CPSSlice::Create(const BYTE **pIOData, UINT &iIOSize)
{
	CPSSlice *pRet = new CPSSlice();
	GS_ASSERT(pRet);
	if( pRet )
	{
		const BYTE *p = *pIOData;
		UINT iS = iIOSize;
		INT iRet;
		iRet = pRet->Parser(p, iS);
		if( iRet<0 )
		{
			GS_ASSERT(0);
			pRet->UnrefObject();
			return NULL;
		}
		p += iRet;
		iS -= iRet;
		*pIOData = p;
		iIOSize = iS;
	}
	return pRet;
}

CPSSlice *CPSSlice::Create(CRefObject *pRefBuf, const BYTE **pIOData, UINT &iIOSize)
{
	CPSSlice *pRet = new CPSSlice();
	GS_ASSERT(pRet);
	if( pRet )
	{
		const BYTE *p = *pIOData;
		UINT iS = iIOSize;
		INT iRet;
		iRet = pRet->Parser(pRefBuf, p, iS);
		if( iRet<=0 )
		{
		//	GS_ASSERT(iRet==0);
			pRet->UnrefObject();
			return NULL;
		}
		p += iRet;
		iS -= iRet;
		*pIOData = p;
		iIOSize = iS;
	}
	return pRet;
}

INT  CPSSlice::Parser( const BYTE *pInputData, UINT iInputSize )
{
	INT iDataSize = 0;
	const BYTE *pp = NULL;
	const BYTE *p = pInputData;
	INT iSize = iInputSize;

	CDynamicBuffer *pBuf = NULL;
	

	m_iPSHeaderType = CES2PS::TestPSStream(p, iSize, &pp, iDataSize );
	if( PS_HEADER_NONE == m_iPSHeaderType )
	{
		return 0;
	}

	if( ((pp-pInputData)+iDataSize) > iInputSize  )
	{
		GS_ASSERT(0);
		return  -1;
	}

	pBuf = pBuf->Create(iDataSize);
	if( !pBuf )
	{
		return -1;
	}
	m_pRefHeader = pBuf;
	pBuf->SetData(pp, iDataSize);
	m_bPSHeader = pBuf->m_bBuffer;
	m_iPSHeaderSize = pBuf->m_iDataSize;
	GS_ASSERT(m_iPSHeaderSize==iDataSize);

	if( PS_HEADER_PES == m_iPSHeaderType )
	{
		//有内容
		iDataSize = 0;		 
		iSize = iInputSize-(pp-p);
		p = pp;		
		if( !CES2PS::PSStreamGetPES(p, iSize, &pp, iDataSize, 
			m_stPesBody.iPSStreamID, m_stPesBody.iPTS, m_stPesBody.iDTS   ) )
		{
			return -1;
		}
		pBuf = pBuf->Create(iDataSize);
		if( !pBuf )
		{
			return -1;
		}
		m_pRefPES = pBuf;
		pBuf->SetData(pp, iDataSize);
		m_stPesBody.pBody = pBuf->m_bBuffer;
		m_stPesBody.iBodySize = pBuf->m_iDataSize;
		GS_ASSERT(m_stPesBody.iBodySize==iDataSize);
	}

	if( ((pp-pInputData)+iDataSize) > iInputSize  )
	{
		return  -1;
	}
	return (pp-pInputData)+iDataSize;
}

INT  CPSSlice::Parser(CRefObject *pRefBuf, const BYTE *pInputData, UINT iInputSize )
{
INT iDataSize = 0;
const BYTE *pp = NULL;
const BYTE *p = pInputData;
INT iSize = iInputSize;
	
	 m_iPSHeaderType = CES2PS::TestPSStream(p, iSize, &pp, iDataSize );
	 if( PS_HEADER_NONE == m_iPSHeaderType )
	 {
		return 0;
	 }

	 if( ((pp-pInputData)+iDataSize) > iInputSize  )
	 {
		 GS_ASSERT(0);
		 return  -1;
	 }

	 m_bPSHeader = pp;
	 m_iPSHeaderSize = iDataSize;
	 if( PS_HEADER_PES == m_iPSHeaderType )
	 {
		 //有内容
		 iDataSize = 0;		 
		 iSize = iInputSize-(pp-p);
		 p = pp;		
		 if( !CES2PS::PSStreamGetPES(p, iSize, &pp, iDataSize, 
				m_stPesBody.iPSStreamID, m_stPesBody.iPTS, m_stPesBody.iDTS   ) )
		 {
			// GS_ASSERT(0);
			 return -1;
		 }
		 m_stPesBody.pBody = pp;
		 m_stPesBody.iBodySize = iDataSize;		
	 }
	 m_pRefHeader = pRefBuf;
	 pRefBuf->RefObject();

	 if( ((pp-pInputData)+iDataSize) > iInputSize  )
	 {
		 GS_ASSERT(0);
		 return  -1;
	 }

	 return (pp-pInputData)+iDataSize;
}

EnumErrno CPSSlice::ParserPSM( std::vector<StruPSStreamInfo> &vStreamInfo)
{
	GS_ASSERT_RET_VAL(m_iPSHeaderType == PS_HEADER_PSM, eERRNO_SYS_EINVALID);
	if( CES2PS::ParserPSM(m_bPSHeader, m_iPSHeaderSize, vStreamInfo) )
	{
		return eERRNO_SUCCESS;
	}
	return eERRNO_SYS_ECODEID;
}


/*
*********************************************************************
*
*@brief : CPSSliceParser
*
*********************************************************************
*/

EnumErrno CPSSliceParser::Slice( CFrameCache *pPSFrame, BOOL bExistGSFHeader, CVectorPSSlice &vResult )
{
	vResult.clear();
	const BYTE *p = (const BYTE*) pPSFrame->GetBuffer().m_bBuffer;
	UINT iSize = pPSFrame->GetBuffer().m_iDataSize;

	if( bExistGSFHeader )
	{
		p += sizeof( StruGSFrameHeader );
		iSize -= sizeof( StruGSFrameHeader );
	}
	if( iSize<4 )
	{
		return eERRNO_SYS_EINVALID;
	}
	CPSSlice *pSlice;
	do 
	{
		pSlice = pSlice->Create(pPSFrame, &p, iSize);
		if( pSlice )
		{
			vResult.push_back(pSlice);
			pSlice->UnrefObject();	
		}
		
	}while( iSize>4 && pSlice );

	return vResult.empty() ?  eERRNO_SYS_ENMEM : eERRNO_SUCCESS;
}