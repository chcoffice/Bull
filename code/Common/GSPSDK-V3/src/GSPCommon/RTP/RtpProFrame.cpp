#include "RtpProFrame.h"


using namespace GSP;
using namespace GSP::RTP;


/*
*********************************************************************
*
*@brief : CRtpProPacket
*
*********************************************************************
*/
CRtpProPacket::CRtpProPacket(void) : CProPacket()
{
	m_iPriBufDataSize = 0;
	m_pRefBuf = NULL;
	m_bPriBuffer = NULL;
	m_iPriMaxBuf = 0;
}

CRtpProPacket::~CRtpProPacket(void)
{
	SAFE_DESTROY_REFOBJECT(&m_pRefBuf);
	if( m_bPriBuffer )
	{
		CMemoryPool::Free(m_bPriBuffer);
		m_bPriBuffer = NULL;
		m_iPriMaxBuf = 0;
	}

}

CRtpProPacket *CRtpProPacket::Create( CGSPBuffer *pBuf )
{
	const BYTE *pPacket = pBuf->m_bBuffer;
	INT iPacketLen = pBuf->m_iDataSize;


	CRtpProPacket *pRet = new CRtpProPacket();
	GS_ASSERT(pRet);
	if( pRet )
	{
		

		INT iHLen = pRet->m_csRtpHeader.Init(pPacket, iPacketLen);
		if( iHLen<0 || iPacketLen<iHLen )
		{
			GS_ASSERT(0);
			pRet->UnrefObject();
			return NULL;
		}
		pRet->m_iPriMaxBuf = iPacketLen;

		iPacketLen -= iHLen;
		pPacket += iHLen;	

		
		
		pRet->m_iPriBufDataSize = iPacketLen;
		pRet->m_stPktParser.bHeader = (BYTE*) pRet->m_csRtpHeader.GetHeaderBuffer();
		pRet->m_stPktParser.iHeaderSize = iHLen;
		pRet->m_stPktParser.bPlayload = (BYTE*) pPacket;
		pRet->m_stPktParser.iPlayloadSize = pRet->m_iPriBufDataSize ;

		pRet->m_pRefBuf = pBuf;
		pBuf->RefObject();
	}
	return pRet;
}

CRtpProPacket *CRtpProPacket::Create( const BYTE *pPacket, int iPacketLen)
{
	GS_ASSERT_RET_VAL(iPacketLen>=RTP_PACKET_HEADER_LENGTH, NULL);
	CRtpProPacket *pRet = new CRtpProPacket();
	GS_ASSERT(pRet);
	if( pRet )
	{
		pRet->m_bPriBuffer = (BYTE*) CMemoryPool::Malloc(iPacketLen);
		if( pRet->m_bPriBuffer == NULL )
		{
			GS_ASSERT(0);
			pRet->UnrefObject();
			return NULL;
		}
		pRet->m_iPriMaxBuf = iPacketLen;

		INT iHLen = pRet->m_csRtpHeader.Init(pPacket, iPacketLen);
		if( iHLen<0 || iPacketLen<iHLen )
		{
			GS_ASSERT(0);
			pRet->UnrefObject();
			return NULL;
		}
		iPacketLen -= iHLen;
		pPacket += iHLen;


		memcpy( pRet->m_bPriBuffer, pPacket, iPacketLen );
		pRet->m_iPriBufDataSize = iPacketLen;

		pRet->m_stPktParser.bHeader = (BYTE*) pRet->m_csRtpHeader.GetHeaderBuffer();
		pRet->m_stPktParser.iHeaderSize = iHLen;
		pRet->m_stPktParser.bPlayload = pRet->m_bPriBuffer;
		pRet->m_stPktParser.iPlayloadSize = pRet->m_iPriBufDataSize;
	}
	return pRet;
}

// CRtpProPacket *CRtpProPacket::Create( const StruRTPHeader &stHeader )
// {
// 	CRtpProPacket *pRet = new CRtpProPacket();
// 	GS_ASSERT(pRet);
// 	if( pRet )
// 	{
// 		pRet->m_bPriBuffer =(BYTE*) CMemoryPool::Malloc(iMAX_PACKET_SIZE);
// 		if( pRet->m_bPriBuffer == NULL )
// 		{
// 			GS_ASSERT(0);
// 			pRet->UnrefObject();
// 			return NULL;
// 		}
// 
// 		INT iHLen = pRet->m_csRtpHeader.Init(stHeader);		
// 		pRet->m_stPktParser.bHeader = (BYTE*) pRet->m_csRtpHeader.GetHeaderBuffer();
// 		pRet->m_stPktParser.iHeaderSize = iHLen;
// 	}
// 	return pRet;
// }

CRtpProPacket *CRtpProPacket::Create( const CRtpHeader &csHeader , UINT iMaxBuffer)
{
	CRtpProPacket *pRet = new CRtpProPacket();
	GS_ASSERT(pRet);
	if( pRet )
	{
		pRet->m_bPriBuffer = (BYTE*)CMemoryPool::Malloc(iMaxBuffer);
		if( pRet->m_bPriBuffer == NULL )
		{
			GS_ASSERT(0);
			pRet->UnrefObject();
			return NULL;
		}
		pRet->m_iPriMaxBuf = iMaxBuffer;
		pRet->m_csRtpHeader =  csHeader;
		INT iHLen = pRet->m_csRtpHeader.GetHeaderSize();	
		pRet->m_stPktParser.bHeader = (BYTE*) pRet->m_csRtpHeader.GetHeaderBuffer();
		pRet->m_stPktParser.iHeaderSize = iHLen;
	}
	return pRet;
}

INT CRtpProPacket::MaxAppendPlayload(const BYTE *pPlayload, UINT iSize )
{

	if( m_bPriBuffer == NULL )
	{
		GS_ASSERT(0);
		return -1;
	}

	
	INT iRet = MIN(iSize, m_iPriMaxBuf-m_iPriBufDataSize-m_csRtpHeader.GetHeaderSize());
	
	if( iRet>0 )
	{		
		memcpy( m_bPriBuffer+m_iPriBufDataSize, pPlayload, iRet);
		m_iPriBufDataSize += iRet;
		m_stPktParser.bPlayload = m_bPriBuffer;
		m_stPktParser.iPlayloadSize = m_iPriBufDataSize;
	}
	return iRet;
	
}


/*
*********************************************************************
*
*@brief : CRtpProFrame
*
*********************************************************************
*/
CRtpProFrame::CRtpProFrame(void)
{

	
}

CRtpProFrame::~CRtpProFrame(void)
{

}

CRtpProFrame *CRtpProFrame::Create(CFrameCache *pFrameData, 
								   CRtpHeader &csHeader,
								   BOOL bEnableMark  )
{
	CRtpProFrame *pRet = new CRtpProFrame();
	if( !pRet )
	{
		GS_ASSERT(0);
		return NULL;
	}
	CGSPBuffer *pRefBuf;
	const BYTE *p;
	UINT iSize;
	CRtpProPacket *pPro = NULL;
	INT iRet;
	UINT16 iSeq = csHeader.GetSequenceNumber();
	if( bEnableMark )
	{
		csHeader.SetMarker(0);
	}
	pRefBuf = &pFrameData->GetBuffer();

	p = pRefBuf->m_bBuffer;
	iSize = pRefBuf->m_iDataSize;


	while(iSize>0)
	{


		pPro = pPro->Create(csHeader, pPro->iDEFAULT_PACKET_SIZE);
		if( !pPro )
		{
			GS_ASSERT(0);
			pRet->UnrefObject();
			return NULL;
		}
		iRet =  pPro->MaxAppendPlayload(p, iSize);
		if( iRet<0 )
		{
			GS_ASSERT(0);
			pPro->UnrefObject();
			pRet->UnrefObject();
			return NULL;

		}
		iSize -= iRet;
		p += iRet;
		iSeq++;
		csHeader.SetSequenceNumber(iSeq);

		if( iSize == 0 )
		{
			if(bEnableMark)
			{
				pPro->GetHeader().SetMarker(1);
			}
		}

		//ÒÑ¾­Ð´Âú
		if( pRet->AppendBack(pPro) )
		{
			GS_ASSERT(0);
			pPro->UnrefObject();
			pRet->UnrefObject();
			return NULL;
		}
		pPro->UnrefObject();	
	}

	return pRet;
}

CRtpHeader *CRtpProFrame::GetFirstHeader(void)
{
	if( m_iPktNumber )
	{
		return &dynamic_cast<CRtpProPacket*>(m_ppPkts[0])->GetHeader();
	}
	return NULL;
}