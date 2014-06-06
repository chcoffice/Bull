#include "GSPAnalyer.h"
#include "../crc.h"
#include "../md5.h"
#include "../Log.h"

using namespace GSP;

/*
*********************************************************************
*
*@brief : CGspProPacket 实现
*
*********************************************************************
*/
CGspProPacket::CGspProPacket(void) : CProPacket()
{
	bzero(&m_unHeader, sizeof(m_unHeader));
	m_pPBuffer = NULL;
	m_iPBufMaxSize = 0;
	m_iWholeSize = 0;
	m_iAnalyseErrno = 0;
	m_unHeader.stHeader.iVersion = GSP_VERSION;
	m_pPasteBuf = NULL;


}

CGspProPacket::~CGspProPacket(void)
{
	if( m_pPBuffer )
	{
		CMemoryPool::Free(m_pPBuffer);
		m_pPBuffer = NULL;
		m_iPBufMaxSize = 0;
	}
	SAFE_DESTROY_REFOBJECT(&m_pPasteBuf);
}



BOOL CGspProPacket::InitOfPaste(CRefObject *pPasteBuf, 
								const BYTE **ppPlayload, UINT &iSize, INT iVersion)
{
	//Reset();
	m_pPasteBuf = pPasteBuf;
	m_pPasteBuf->RefObject();
	m_unHeader.stHeader.iLen = 0;
	UINT iUsed = GSP_PACKET_SIZE-GSP_PACKET_HEADER_LEN-GSP_MAX_CHK_CRC_LEN;
	if( iVersion == GSP_VERSION_V1  )
	{
		iUsed = GSP_PACKET_V1_SIZE-GSP_PACKET_HEADER_LEN-GSP_MAX_CHK_CRC_LEN;
	}

	iUsed = MIN(iUsed, iSize);

	const BYTE *p = *ppPlayload;
	m_iWholeSize = iUsed;
	m_stPktParser.bHeader = m_unHeader.bBuffer;
	m_stPktParser.iHeaderSize = GSP_PACKET_HEADER_LEN;
	m_stPktParser.iPlayloadSize = iUsed;
	m_stPktParser.bPlayload = (BYTE*)p;
	p += iUsed;
	iSize -= iUsed;	
	*ppPlayload = p;
	return TRUE;


}

CGspProPacket *CGspProPacket::Create(CRefObject *pPasteBuf, 
									 const BYTE **ppPlayload, UINT &iSize, INT iVersion )
{
	CGspProPacket *pRet = new CGspProPacket();
	GS_ASSERT_RET_VAL(pRet, NULL);
	if( pRet->InitOfPaste(pPasteBuf, ppPlayload, iSize, iVersion) )
	{
		return pRet;
	}
	pRet->UnrefObject();
	return NULL;

}

CGspProPacket *CGspProPacket::Create( UINT iMaxWholeSize )
{
	GS_ASSERT_RET_VAL( iMaxWholeSize>=(GSP_PACKET_HEADER_LEN+GSP_MAX_CHK_CRC_LEN), NULL );
	iMaxWholeSize -= (GSP_PACKET_HEADER_LEN+GSP_MAX_CHK_CRC_LEN);
	CGspProPacket *pRet = new CGspProPacket();
	GS_ASSERT(pRet);
	if( pRet )
	{
		if( iMaxWholeSize )
		{
			pRet->m_pPBuffer = (BYTE*) CMemoryPool::Malloc(iMaxWholeSize+GSP_MAX_CHK_CRC_LEN);
			if( pRet->m_pPBuffer == NULL )
			{
				GS_ASSERT(0);
				pRet->UnrefObject();
				return NULL;
			}
			pRet->m_iPBufMaxSize = iMaxWholeSize;
		}
	}
	return pRet;
}

EnumErrno CGspProPacket::Analyse( const BYTE **ppData, UINT &iSize )
{

	GS_ASSERT_RET_VAL(iSize>0, eERRNO_SUCCESS);
	GS_ASSERT_RET_VAL(m_iAnalyseErrno==0, eERRNO_SYS_ESTATUS );

	GS_ASSERT(m_iPBufMaxSize);
	const BYTE *p = *ppData;
	INT iTemp;



	if( m_iWholeSize < GSP_PACKET_HEADER_LEN )
	{
		iTemp = MIN( (UINT)(GSP_PACKET_HEADER_LEN-m_iWholeSize), iSize);
		::memcpy(m_unHeader.bBuffer+m_iWholeSize, p, iTemp );
		m_iWholeSize += iTemp;
		p += iTemp;
		iSize -= iTemp;
		if( iSize<1 )
		{
			*ppData = p;
			return eERRNO_SUCCESS;
		}

		if( m_unHeader.stHeader.iLen>(m_iPBufMaxSize+GSP_PACKET_HEADER_LEN+GSP_MAX_CHK_CRC_LEN) )
		{
			m_iAnalyseErrno = -5;
			GS_ASSERT(0);
			*ppData = p;
			return eERRNO_SYS_EPRO;
		}
		GS_ASSERT(m_unHeader.stHeader.iLen>m_iWholeSize);

		
	}

	GS_ASSERT(m_iWholeSize >= GSP_PACKET_HEADER_LEN );

	


	if( m_iWholeSize<m_unHeader.stHeader.iLen )
	{

		iTemp = MIN( (UINT)(m_unHeader.stHeader.iLen-m_iWholeSize) , iSize);

		::memcpy( m_pPBuffer+m_iWholeSize-GSP_PACKET_HEADER_LEN, p, iTemp);
		m_iWholeSize += iTemp;
		p += iTemp;
		iSize -= iTemp;
	}

	if( m_iWholeSize==m_unHeader.stHeader.iLen )
	{
		m_stPktParser.iHeaderSize = GSP_PACKET_HEADER_LEN;
		m_stPktParser.bHeader = m_unHeader.bBuffer;
		switch( m_unHeader.stHeader.iCRC )
		{
		case CRC_TYPE_NONE :
			m_stPktParser.iTailerSize = 0;
			break;
		case CRC_TYPE_CRC16 :
			m_stPktParser.iTailerSize = 2;
			break;
		case CRC_TYPE_CRC32 :
			m_stPktParser.iTailerSize = 4;
			break;
		case CRC_TYPE_MD5 :
			m_stPktParser.iTailerSize = MD5_LEN;
			break;
		default :
			{
				m_iAnalyseErrno = -5;
				GS_ASSERT(0);
				*ppData = p;
				return eERRNO_SYS_ECRC;
			}
			break;
		}
		m_stPktParser.iPlayloadSize = m_iWholeSize-m_stPktParser.iHeaderSize-m_stPktParser.iTailerSize;
		m_stPktParser.bPlayload = m_pPBuffer;

		if( m_stPktParser.iTailerSize )
		{
			m_stPktParser.bTailer = m_bTBuffer;
			::memcpy(m_stPktParser.bTailer ,
				m_stPktParser.bPlayload+m_stPktParser.iPlayloadSize,
				m_stPktParser.iTailerSize);
		}

		//完成
		if( !CheckCRC() )
		{
			m_iAnalyseErrno = -5;
			GS_ASSERT(0);
			*ppData = p;
			return eERRNO_SYS_ECRC;
		}
	}
	*ppData = p;
	return eERRNO_SUCCESS;
}


INT CGspProPacket::AnalyseResult(void)
{
	if( m_iAnalyseErrno )
	{
		return m_iAnalyseErrno;
	}
	if( m_iWholeSize==m_unHeader.stHeader.iLen )
	{
		return 0;
	}
	if( m_iWholeSize<GSP_PACKET_HEADER_LEN )
	{
		return -2;
	}
	return m_unHeader.stHeader.iLen-m_iWholeSize;
}

void CGspProPacket::Reset(void)
{
	bzero(&m_unHeader, sizeof(m_unHeader));
	m_iWholeSize = 0;
	bzero(&m_stPktParser, sizeof(m_stPktParser));
	m_unHeader.stHeader.iVersion = GSP_VERSION;
	SAFE_DESTROY_REFOBJECT(&m_pPasteBuf);
}

EnumErrno CGspProPacket::Packet(const StruGSPPacketHeader &stHeader)
{
	if( m_iWholeSize==0 )
	{
		GS_ASSERT(0);
		return eERRNO_SYS_ESTATUS;
	}

	if( m_unHeader.stHeader.iLen )
	{
		//已经打包
		GS_ASSERT(0);
		return eERRNO_SYS_EEXIST; 
	}
	memcpy(&m_unHeader.stHeader, &stHeader, sizeof(stHeader));
	m_unHeader.stHeader.iLen =  0;
	m_stPktParser.iHeaderSize = GSP_PACKET_HEADER_LEN;
	m_stPktParser.bHeader = m_unHeader.bBuffer;
	m_stPktParser.iPlayloadSize = m_iWholeSize;
	if( m_pPBuffer )
	{
		m_stPktParser.bPlayload = m_pPBuffer;
	}
	GS_ASSERT(m_stPktParser.bPlayload );

	m_stPktParser.iTailerSize = 0;
	switch( m_unHeader.stHeader.iCRC )
	{
	case CRC_TYPE_NONE :
		m_stPktParser.iTailerSize = 0;
		break;
	case CRC_TYPE_CRC16 :
		m_stPktParser.iTailerSize = 2;
		break;
	case CRC_TYPE_CRC32 :
		m_stPktParser.iTailerSize = 4;
		break;
	case CRC_TYPE_MD5 :
		m_stPktParser.iTailerSize = MD5_LEN;
		break;
	default :
		{
			GS_ASSERT(0);
		}
		break;
	}
	if( m_stPktParser.iTailerSize > 0 )
	{

		if( (m_iWholeSize+m_stPktParser.iHeaderSize+m_stPktParser.iTailerSize)>(m_iPBufMaxSize+GSP_MAX_CHK_CRC_LEN) )
		{
			GS_ASSERT(0);
			m_stPktParser.iTailerSize = 0;
		}
		else
		{
			m_stPktParser.bTailer = m_bTBuffer;
			m_iWholeSize += m_stPktParser.iTailerSize;
		}		
		m_iWholeSize += m_stPktParser.iHeaderSize;
		m_unHeader.stHeader.iLen = m_iWholeSize;

		MakeCRC();
	}
	else
	{
		m_iWholeSize += m_stPktParser.iHeaderSize;
		m_unHeader.stHeader.iLen = m_iWholeSize;
	}
	return eERRNO_SUCCESS;

}

UINT CGspProPacket::GetFreeBuffSize(void) const
{
	if( m_unHeader.stHeader.iLen )
	{
		//不能再添加数据了
		return 0;
	}
	//当使用引用时 m_iPBufMaxSize = 0
	if( m_iWholeSize<m_iPBufMaxSize )
	{
		return m_iPBufMaxSize-m_iWholeSize-GSP_PACKET_HEADER_LEN;
	}
	return 0;
}

EnumErrno CGspProPacket::AppendPlayload( const BYTE *pData, UINT iSize )
{
	if(m_unHeader.stHeader.iLen )
	{
		//已经封闭
		GS_ASSERT(0);
		return eERRNO_SYS_ESTATUS;
	}

	if( (iSize+m_iWholeSize)>m_iPBufMaxSize )
	{
		//已经溢出
		GS_ASSERT(0);
		return eERRNO_SYS_EFLOWOUT;
	}
	::memcpy( m_pPBuffer+m_iWholeSize, pData, iSize);
	m_iWholeSize += iSize;
	return eERRNO_SUCCESS;
}

EnumErrno CGspProPacket::AppendPlayloadInMaxC( const BYTE **pData, UINT &iSize )
{

	if(m_unHeader.stHeader.iLen )
	{
		//已经封闭
		GS_ASSERT(0);
		return eERRNO_SYS_ESTATUS;
	}
	const BYTE *p = *pData;
	UINT iMax = MIN( (m_iPBufMaxSize-m_iWholeSize), iSize );

	if( iMax )
	{
		::memcpy( m_pPBuffer+m_iWholeSize, p, iMax);
		m_iWholeSize += iMax;
		p += iMax;
		iSize -= iMax;
	}
	*pData = p;
	return eERRNO_SUCCESS;
}


static UINT16 _MakeCRC16( const BYTE *v1, UINT iV1Len, const BYTE *v2, UINT iV2Len )
{
	UINT iTemp;
	iTemp = mk_crc16((void*)v1, iV1Len);
	iTemp = mk_crc16((void*)v2, iV2Len, iTemp);
	return iTemp;
}

static UINT32 _MakeCRC32( const BYTE *v1, UINT iV1Len, const BYTE *v2, UINT iV2Len )
{
	UINT iTemp;
	iTemp = mk_crc32((void*)v1, iV1Len);
	iTemp = mk_crc32((void*)v2, iV2Len, iTemp);
	return iTemp;
}

static void _MakeMD5( const BYTE *v1, UINT iV1Len, const BYTE *v2, UINT iV2Len, BYTE *pOutVal )
{
	MD5_CTX stCtx;
	MD5Init(&stCtx)	;
	::MD5Update( &stCtx, (unsigned char *)v1, iV1Len);		
	::MD5Update( &stCtx, (unsigned char *)v2, iV2Len);
	MD5Final(&pOutVal[0], &stCtx);		
}


BOOL CGspProPacket::CheckCRC(void)
{
	switch( m_unHeader.stHeader.iCRC )
	{
	case CRC_TYPE_NONE :
		return TRUE;
		break;
	case CRC_TYPE_CRC16 :
		{
			UINT16 iRcvVal ;
			memcpy( &iRcvVal, m_stPktParser.bTailer, sizeof(iRcvVal));
			UINT16 iCRCVal = _MakeCRC16(m_stPktParser.bHeader, m_stPktParser.iHeaderSize,
				m_stPktParser.bPlayload, m_stPktParser.iPlayloadSize );
			return iRcvVal== iCRCVal;
		}		
		break;
	case CRC_TYPE_CRC32 :
		{
			UINT32 iRcvVal ;
			memcpy( &iRcvVal, m_stPktParser.bTailer, sizeof(iRcvVal));
			UINT32 iCRCVal = _MakeCRC32(m_stPktParser.bHeader, m_stPktParser.iHeaderSize,
				m_stPktParser.bPlayload, m_stPktParser.iPlayloadSize );
			return iRcvVal== iCRCVal;
		}
		break;
	case CRC_TYPE_MD5 :
		{
			BYTE vRcvVal[MD5_LEN];
			memcpy( &vRcvVal, m_stPktParser.bTailer, sizeof(vRcvVal));
			BYTE vCRCVal[MD5_LEN];
			_MakeMD5(m_stPktParser.bHeader, m_stPktParser.iHeaderSize,
				m_stPktParser.bPlayload, m_stPktParser.iPlayloadSize,vCRCVal );
			return 0==::memcmp(&vRcvVal,&vCRCVal, sizeof(vRcvVal) );
		}
		break;
	default :
		{
			GS_ASSERT(0);
		}
		break;
	}
	return FALSE;
}


void CGspProPacket::MakeCRC(void)
{
	switch( m_unHeader.stHeader.iCRC )
	{
	case CRC_TYPE_NONE :
		return;
		break;
	case CRC_TYPE_CRC16 :
		{


			UINT16 iCRCVal = _MakeCRC16(m_stPktParser.bHeader, m_stPktParser.iHeaderSize,
				m_stPktParser.bPlayload, m_stPktParser.iPlayloadSize );
			memcpy(  m_stPktParser.bTailer,&iCRCVal, sizeof(iCRCVal));
		}		
		break;
	case CRC_TYPE_CRC32 :
		{

			UINT32 iCRCVal = _MakeCRC32(m_stPktParser.bHeader, m_stPktParser.iHeaderSize,
				m_stPktParser.bPlayload, m_stPktParser.iPlayloadSize );
			memcpy(  m_stPktParser.bTailer,&iCRCVal, sizeof(iCRCVal));
		}
		break;
	case CRC_TYPE_MD5 :
		{		
			BYTE vCRCVal[MD5_LEN];
			_MakeMD5(m_stPktParser.bHeader, m_stPktParser.iHeaderSize,
				m_stPktParser.bPlayload, m_stPktParser.iPlayloadSize,vCRCVal );
			::memcpy(  m_stPktParser.bTailer,vCRCVal, sizeof(vCRCVal));		}
		break;
	default :
		{
			GS_ASSERT(0);
		}
		break;
	}
}


/*
*********************************************************************
*
*@brief : CGspCommand 实现
*
*********************************************************************
*/



CGspCommand::CGspCommand(void) : CGSPObject()
{
	m_iPlayloadSize = 0;
	m_iWholeSize = GSP_PRO_COMMAND_HEADER_LENGTH;
	bzero(&m_unData.stCmd, sizeof(m_unData.stCmd));

	m_iGspVersion = -1;
	m_iGspDataType = -1;
	m_iGspSeq = MAX_UINT16;
	m_iGspSubChn = (UINT8)-1;
	m_iGspExtraVal = (UINT8)-1;
}

CGspCommand::~CGspCommand(void)
{

}

EnumErrno CGspCommand::Parser(CGspProFrame *pProFrame)
{

	GS_ASSERT(pProFrame!=NULL);
	CProPacket **vPkts = NULL;
	UINT iNumbs = pProFrame->GetPackets(&vPkts);
	GS_ASSERT_RET_VAL( iNumbs>0 , eERRNO_SYS_EINVALID);
	Reset();
	m_iGspVersion = pProFrame->m_iGspVersion;
	m_iGspDataType = pProFrame->m_iGspDataType;
	m_iGspSeq = pProFrame->m_iGspSeq;
	m_iGspSubChn = pProFrame->m_iGspSubChn;
	m_iGspExtraVal = pProFrame->m_iGspExtraVal;
	EnumErrno eRet = eERRNO_SUCCESS;
	m_iWholeSize = 0;	
	for( UINT i = 0; i<iNumbs; i++ )
	{
		const StruPktInfo &stT = vPkts[i]->GetParser();
		GS_ASSERT(stT.iPlayloadSize);
		if( (stT.iPlayloadSize+m_iWholeSize)<=sizeof(m_unData) )
		{

			memcpy(&m_unData.bBuffer[m_iWholeSize], stT.bPlayload, stT.iPlayloadSize);
			m_iWholeSize += stT.iPlayloadSize;
			m_iPlayloadSize += stT.iPlayloadSize;
		}
		else
		{
			GS_ASSERT(0);
			Reset();
			return eERRNO_SYS_EFLOWOUT;
		}

	}
	if( m_iWholeSize<GSP_PRO_COMMAND_HEADER_LENGTH )
	{
		GS_ASSERT(0);
		Reset();
		return eERRNO_SYS_EINVALID;
	}
	return eRet;
}

EnumErrno CGspCommand::Parser( const char *pData, UINT iSize )
{
	if( iSize<GSP_PRO_COMMAND_HEADER_LENGTH )
	{
		GS_ASSERT(0);
		return eERRNO_SYS_EINVALID;
	}

	GS_ASSERT_RET_VAL((iSize+m_iWholeSize)<=sizeof(m_unData), eERRNO_SYS_EFLOWOUT);
	memcpy(&m_unData, pData, iSize);
	m_iWholeSize = iSize;
	m_iPlayloadSize = m_iWholeSize-GSP_PRO_COMMAND_HEADER_LENGTH;

	return eERRNO_SUCCESS;
}

void CGspCommand::Reset(void)
{
	m_iPlayloadSize = 0;
	m_iWholeSize = GSP_PRO_COMMAND_HEADER_LENGTH;
	bzero(&m_unData.stCmd, sizeof(m_unData.stCmd));
	m_iGspVersion = -1;
	m_iGspDataType = -1;
	m_iGspSeq = MAX_UINT16;
	m_iGspSubChn = (UINT8)-1;
	m_iGspExtraVal =  (UINT8)-1;
}

EnumErrno CGspCommand::AddCommandPlayload(const void *pData, UINT iSize )
{
	GS_ASSERT_RET_VAL((iSize+m_iWholeSize)<=sizeof(m_unData), eERRNO_SYS_EFLOWOUT);
	memcpy(&m_unData.bBuffer[m_iWholeSize], pData, iSize);
	m_iWholeSize += iSize;
	m_iPlayloadSize += iSize;
	return eERRNO_SUCCESS;
}


/*
*********************************************************************
*
*@brief : CGspProFrame
*
*********************************************************************
*/

CGspProFrame::CGspProFrame(void) : CProFrame()
{
	m_iGspVersion = -1;
	m_iGspDataType = -1;
	m_iGspSeq= MAX_UINT16;
	m_iGspSubChn = (UINT8)-1;
	m_iGspExtraVal =  (UINT8)-1;
}

CGspProFrame::~CGspProFrame(void)
{

}


//检测是否有效
BOOL CGspProFrame::CheckValid(void)
{


	if( m_iPktNumber < 1 )
	{
		return FALSE;
	}
	UINT i = 0;
	CGspProPacket *p = dynamic_cast<CGspProPacket *>(m_ppPkts[i]);
	i++;
	UINT16 iSSeq = p->GetHeader().iSSeq;

	if( iSSeq != 0 )
	{
		GS_ASSERT(0);
		return FALSE;
	}
	else
	{

		StruGSPPacketHeader &stHeader = p->GetHeader();
		m_iGspVersion = stHeader.iVersion;
		m_iGspDataType = stHeader.iDataType;
		m_iGspSeq=  stHeader.iSeq;
		m_iGspSubChn = stHeader.iSubChn;
		m_iGspExtraVal=  stHeader.iExtraVal;
	}

	for( ; i<m_iPktNumber; i++ )
	{
		p = dynamic_cast<CGspProPacket *>(m_ppPkts[i]);
		if( p->GetHeader().iSSeq!=(iSSeq+1) )
		{
			GS_ASSERT(0);
			return FALSE;
		}
		iSSeq = p->GetHeader().iSSeq;
	}

	if( !p->GetHeader().bEnd )
	{
		GS_ASSERT(0);
		return FALSE;
	}

	return TRUE;
}


/*
*********************************************************************
*
*@brief : 协议解析器
*
*********************************************************************
*/

static void _FreeGspTcpDecoderFinishListMember( CGspProFrame *pFrame )
{
	SAFE_DESTROY_REFOBJECT(&pFrame);
}

CGspTcpDecoder::CGspTcpDecoder(void)
:CGSPObject()
,m_csFinishList()
{
	m_pCurPacket = NULL;
	m_pCurFrame = NULL; //当前包内容
	m_csFinishList.SetFreeCallback((FuncPtrFree)_FreeGspTcpDecoderFinishListMember);
	m_iMaxPacketSize = GSP_PACKET_SIZE;
	m_iVersion = -1;
	m_eAssert = eERRNO_SUCCESS;
}


CGspTcpDecoder::~CGspTcpDecoder(void)
{	
	SAFE_DESTROY_REFOBJECT(&m_pCurPacket);
	SAFE_DESTROY_REFOBJECT(&m_pCurFrame);
	m_csFinishList.Clear();
}

EnumErrno CGspTcpDecoder::Decode( CGSPBuffer *pInBuffer)
{
	if( m_eAssert )
	{
		return eERRNO_SYS_ESTATUS;
	}
	UINT iSize = pInBuffer->m_iDataSize;
	const BYTE *pData = pInBuffer->m_bBuffer;
	EnumErrno &eRet = m_eAssert;
	INT iRet;

	while( iSize>0 ) {

		if( m_pCurPacket == NULL )
		{
			m_pCurPacket = m_pCurPacket->Create(m_iMaxPacketSize);
			if( m_pCurPacket == NULL )
			{
				GS_ASSERT(0);	
				MY_LOG_FATAL(g_pLog, _GSTX("分配内存失败!!\n"));
				eRet = eERRNO_SYS_ENMEM;

				return eRet;
			}
		}

		eRet = m_pCurPacket->Analyse(&pData, iSize);
		if(  eRet )
		{
			GS_ASSERT(0);	
			MY_LOG_FATAL(g_pLog, _GSTX("协议解析失败A: %s!\n"), GetError(eRet));		
			return eRet;
		}


		iRet = m_pCurPacket->AnalyseResult();
		if( iRet == 0 )
		{
			if( m_iVersion == -1 )
			{
				m_iVersion = m_pCurPacket->GetHeader().iVersion;
				if( m_iVersion == GSP_VERSION_V1 )
				{
					m_iMaxPacketSize = GSP_PACKET_V1_SIZE;
				}
			}
			//完成数据分析
			if( m_pCurFrame == NULL )
			{
				m_pCurFrame = m_pCurFrame->Create();
				if( NULL == m_pCurFrame )
				{
					GS_ASSERT(0);	
					MY_LOG_FATAL(g_pLog, _GSTX("分配内存失败!!\n"));
					eRet = eERRNO_SYS_ENMEM;

					SAFE_DESTROY_REFOBJECT(&m_pCurPacket);
					return eRet;
				}
			}
			if( m_pCurFrame->AppendBack(m_pCurPacket) )
			{
				GS_ASSERT(0);	
				MY_LOG_FATAL(g_pLog, _GSTX("分配内存失败!!\n"));
				eRet = eERRNO_SYS_ENMEM;

				SAFE_DESTROY_REFOBJECT(&m_pCurPacket);
				SAFE_DESTROY_REFOBJECT(&m_pCurFrame);
				return eRet;
			}

			iRet = m_pCurPacket->GetHeader().bEnd;
			SAFE_DESTROY_REFOBJECT(&m_pCurPacket);
			if( iRet )
			{

				//结束帧
				if( !m_pCurFrame->CheckValid() )
				{
					//丢包
					GS_ASSERT(0);	
					MY_LOG_FATAL(g_pLog, _GSTX("序列不对!!\n"));
					eRet = eERRNO_SYS_ECRC;
					SAFE_DESTROY_REFOBJECT(&m_pCurFrame);
					return eRet;
				}



				// 一帧完成
				if(	eERRNO_SUCCESS == m_csFinishList.AddTail(m_pCurFrame))
				{								
					m_pCurFrame = NULL;
				}
				else
				{
					GS_ASSERT(0);
					eRet =  eERRNO_SYS_ENMEM;
					MY_LOG_FATAL(g_pLog, _GSTX("分配内存失败!!\n"));					
					SAFE_DESTROY_REFOBJECT(&m_pCurFrame);
					return eRet;
				}

			}
		} 
		else if( iRet < 0 && iRet!=-2  )
		{
			GS_ASSERT(0);	
			MY_LOG_FATAL(g_pLog, _GSTX("协议解析失败B: %d !\n"), iRet);
			eRet = eERRNO_SYS_EPRO;
			return eRet;			
		}		
	} //end while( iSize>0 )
	return eRet;
}

CGspProFrame *CGspTcpDecoder::Pop( void )
{
	void *pData = NULL;
	if( eERRNO_SUCCESS != m_csFinishList.RemoveFront( &pData ) )
	{
		return NULL;
	}
	return (CGspProFrame*)pData;
}

void CGspTcpDecoder::Reset(void)
{
	m_csFinishList.Clear();
	SAFE_DESTROY_REFOBJECT(&m_pCurPacket);
	SAFE_DESTROY_REFOBJECT(&m_pCurFrame);
}


/*
*********************************************************************
*
*@brief : CGspTcpEncoder GSP TCP 协议打包
*
*********************************************************************
*/

CGspTcpEncoder::CGspTcpEncoder(void)
:CGSPObject()
{	
}

CGspTcpEncoder::~CGspTcpEncoder(void)
{

}

CGspProFrame * CGspTcpEncoder::Encode( CGSPBuffer *pInBuffer, StruGSPPacketHeader &stHeader )
{

	GS_ASSERT_RET_VAL(pInBuffer->m_iDataSize>0, NULL);
	CGspProFrame *pRet = pRet->Create();
	if( pRet == NULL  )
	{
		GS_ASSERT(0);
		return NULL;
	}
	CGspProPacket *pPkt = NULL;
	const BYTE *pp;
	UINT iSize;
	pp = pInBuffer->m_bBuffer;
	iSize = pInBuffer->m_iDataSize;

	pRet->m_iGspVersion = stHeader.iVersion;
	pRet->m_iGspDataType = stHeader.iDataType;
	pRet->m_iGspSeq = stHeader.iSeq;
	pRet->m_iGspSubChn = stHeader.iSubChn;	
	pRet->m_iGspExtraVal = stHeader.iExtraVal;
	stHeader.bEnd = 0;
	while( iSize>0 )
	{


		pPkt = pPkt->Create(pInBuffer, &pp, iSize, stHeader.iVersion);
		if( !pPkt )
		{
			GS_ASSERT(0);
			pRet->UnrefObject();
			return NULL;
		}
		if( iSize<1 )
		{
			//结束帧
			stHeader.bEnd = 1;
		}

		if( pPkt->Packet(stHeader) )
		{

			GS_ASSERT(0);
			pPkt->UnrefObject();
			pRet->UnrefObject();
			return NULL;
		}
		stHeader.iSSeq++;
		if( pRet->AppendBack(pPkt) )
		{
			//出错
			GS_ASSERT(0);
			pPkt->UnrefObject();
			pRet->UnrefObject();
			return NULL;
		}		
		SAFE_DESTROY_REFOBJECT(&pPkt);
	}
	return pRet;
}


CGspProFrame *  CGspTcpEncoder::Encode(CFrameCache *pFrameCache, 
									   StruGSPPacketHeader &stHeader )
{
	CGspProFrame *pRet = pRet->Create();
	if( pRet == NULL  )
	{
		GS_ASSERT(0);
		return NULL;
	}
	CGspProPacket *pPkt = NULL;	
	CGSPBuffer *pRefBuf;
	const BYTE *p;
	UINT iSize;

	UINT iMaxSize = GSP_PACKET_SIZE;
	if( stHeader.iVersion == GSP_VERSION_V1 )
	{
		iMaxSize = GSP_PACKET_V1_SIZE;
	}

	pRefBuf = &pFrameCache->GetBuffer();
	p  = pRefBuf->m_bBuffer;
	iSize = pRefBuf->m_iDataSize;

	stHeader.iSSeq = 0;
	stHeader.bEnd = 0;

	pRet->m_iGspVersion = stHeader.iVersion;
	pRet->m_iGspDataType = stHeader.iDataType;
	pRet->m_iGspSeq = stHeader.iSeq;
	pRet->m_iGspSubChn = stHeader.iSubChn;	
	pRet->m_iGspExtraVal = stHeader.iExtraVal;



	while( iSize>0 )
	{	
		pPkt = CGspProPacket::Create(pRefBuf, &p, iSize, stHeader.iVersion );
		if( !pPkt )
		{
			GS_ASSERT(0);
			pRet->UnrefObject();
			return NULL;
		}
		if( iSize==0 )
		{
			stHeader.bEnd = TRUE;
		}
		
		if( pPkt->Packet(stHeader) )
		{
			GS_ASSERT(0);
			SAFE_DESTROY_REFOBJECT(&pPkt);
			pRet->UnrefObject();
			return NULL;
		}	
		stHeader.iSSeq++;

		if( pRet->AppendBack(pPkt ) )
		{
			GS_ASSERT(0);
			SAFE_DESTROY_REFOBJECT(&pPkt);
			pRet->UnrefObject();
			return NULL;
		}
		else
		{
				SAFE_DESTROY_REFOBJECT(&pPkt);
		}
	} // end while	
	return pRet;	
}

CGspProFrame *  CGspTcpEncoder::Encode( const StruBaseBuf *vBuf, INT iBufNums, 
									   StruGSPPacketHeader &stHeader )
{
	CGspProFrame *pRet = pRet->Create();
	if( pRet == NULL  )
	{
		GS_ASSERT(0);
		return NULL;
	}
	CGspProPacket *pPkt = NULL;
	const BYTE *p;
	UINT iSize, iTemp;
	EnumErrno eRet;


	UINT iMaxSize = GSP_PACKET_SIZE;
	if( stHeader.iVersion == GSP_VERSION_V1 )
	{
		iMaxSize = GSP_PACKET_V1_SIZE;
	}

	stHeader.iSSeq = 0;
	stHeader.bEnd = 0;

	pRet->m_iGspVersion = stHeader.iVersion;
	pRet->m_iGspDataType = stHeader.iDataType;
	pRet->m_iGspSeq = stHeader.iSeq;
	pRet->m_iGspSubChn = stHeader.iSubChn;	
	pRet->m_iGspExtraVal = stHeader.iExtraVal;



	for( INT i = 0; i<iBufNums; i++ )
	{
		p = (const BYTE*) vBuf[i].pBuffer;
		iSize = vBuf[i].iSize;
		GS_ASSERT(iSize);
		while( iSize>0 )
		{

			if( pPkt == NULL )
			{
				pPkt = pPkt->Create(iMaxSize);
				if( !pPkt )
				{
					GS_ASSERT(0);
					pRet->UnrefObject();
					return NULL;
				}

			}	

			iTemp = iSize;
			eRet = pPkt->AppendPlayloadInMaxC( &p, iSize);
			if( eRet )
			{
				GS_ASSERT(0);
				pPkt->UnrefObject();
				pRet->UnrefObject();
				return NULL;
			}

			if( iSize<1 && (i+1) == iBufNums )
			{
				//结束
				stHeader.bEnd = 1;
			}


			if( iTemp == iSize ||  stHeader.bEnd )
			{
				//没有了空间
				if( pPkt->Packet(stHeader) )
				{	
					GS_ASSERT(0);
					pPkt->UnrefObject();
					pRet->UnrefObject();
					return NULL;
				}
				if(  pRet->AppendBack(pPkt) )
				{
					GS_ASSERT(0);
					pPkt->UnrefObject();
					pRet->UnrefObject();
					return NULL;
				}
				SAFE_DESTROY_REFOBJECT(&pPkt);	
				stHeader.iSSeq++;
			}

		} // end while
	} //end for	

	GS_ASSERT(pPkt==NULL);
	return pRet;	
}

/*
*********************************************************************
*
*@brief : CGspMediaFormat
*
*********************************************************************
*/




EnumGSMediaType CGspMediaFormat::FormatSection( const   StruMediaInfoTable *pMediaInfo,StruGSMediaDescri &stDescri, INT &iChn, UINT iIdxStart, UINT iIdxEnd)
{
	EnumGSMediaType eType = GS_MEDIA_TYPE_NONE;
	iChn = -1;
	bzero( &stDescri, sizeof(stDescri));

	//查找该域的媒体类型和通道号
	for( UINT i=iIdxStart;  i<iIdxEnd; i++ )
	{
		if( pMediaInfo->aRows[i].iNname == GSP_MEDIA_ATTRI_NAME_MEDIATYPE )
		{
			eType = (EnumGSMediaType)pMediaInfo->aRows[i].iValue;
			break;
		}else if( pMediaInfo->aRows[i].iNname == GSP_MEDIA_ATTRI_NAME_SECTION_ID )
		{
			//获取通道号
			iChn = pMediaInfo->aRows[i].iValue;
		}
	}

	stDescri.eMediaType =(UINT32)eType;

	switch (eType )
	{
	case GS_MEDIA_TYPE_VIDEO :
		{    //视频数据
			FormatVideo(pMediaInfo, stDescri.unDescri.struVideo, iIdxStart, iIdxEnd);
		}
		break;
	case GS_MEDIA_TYPE_AUDIO :
		{
			//音频流, 属性描述
			FormatAudio(pMediaInfo, stDescri.unDescri.struAudio, iIdxStart, iIdxEnd);
		}
		break;
	case GS_MEDIA_TYPE_PICTURE:
		{
			//图片数据,属性描述 
			FormatPicture(pMediaInfo, stDescri.unDescri.struPicture, iIdxStart, iIdxEnd);
		}
		break;   

	case      GS_MEDIA_TYPE_OSD:
		{
			//OSD 数据,属性描述
			FormatOSD(pMediaInfo,  stDescri.unDescri.struOsd, iIdxStart, iIdxEnd);
		}
		break;
	case     GS_MEDIA_TYPE_BINARY :
		{         
			//二进制流, 属性描述 
			FormatBin(pMediaInfo,  stDescri.unDescri.struBinary, iIdxStart, iIdxEnd);
		}
		break;
	case       GS_MEDIA_TYPE_SYSHEADER :
		{
			//信息头
			FormatSysHeader(pMediaInfo,  stDescri.unDescri.struSysHeader, iIdxStart, iIdxEnd);
		}
		break;
	default :
		break;
	}
	return eType;
}

void CGspMediaFormat::FormatSysHeader( const StruMediaInfoTable *pMediaInfo,StruSysHeaderDescri &stSys, UINT iIdxStart, UINT iIdxEnd)
{
	UINT32 iTemp = 0;
	stSys.iSize = 0;
	for( UINT i=iIdxStart;  i<iIdxEnd; i++ )
	{
		switch ( pMediaInfo->aRows[i].iNname )
		{
		case GSP_MEDIA_ATTRI_NAME_SYSHEADER_LEN :
			{
				//获取 长的 1~16 位
				iTemp = pMediaInfo->aRows[i].iValue;
				stSys.iSize |= iTemp;
			}
			break;
		case GSP_MEDIA_ATTRI_NAME_SYSHEADER_LEN1 :
			{
				//获取 长的 16~32 位
				iTemp = pMediaInfo->aRows[i].iValue;
				iTemp <<= 16;
				stSys.iSize |= iTemp;
			}
			break;
		case GSP_MEDIA_ATTRI_NAME_MEDIATYPE :
		case GSP_MEDIA_ATTRI_NAME_SECTION_ID :
			break;
		default :
			{
				MY_LOG_WARN(g_pLog,  "GSP sysheader section exist unknown attri name %d\n", 
					pMediaInfo->aRows[i].iNname );
			}
			break;
		}
	}
}


void CGspMediaFormat::FormatBin( const StruMediaInfoTable *pMediaInfo,StruBinaryDescri &stBin, UINT iIdxStart, UINT iIdxEnd)
{
	UINT64 iTemp = 0;
	stBin.iSize = 0;
	for( UINT i=iIdxStart;  i<iIdxEnd; i++ )
	{
		switch ( pMediaInfo->aRows[i].iNname )
		{
		case GSP_MEDIA_ATTRI_NAME_BINARY_LEN :
			{
				//获取 长的 1~16 位
				iTemp = pMediaInfo->aRows[i].iValue;
				stBin.iSize |= iTemp;
			}
			break;
		case GSP_MEDIA_ATTRI_NAME_BINARY_LEN1 :
			{
				//获取 长的 16~32 位
				iTemp = pMediaInfo->aRows[i].iValue;
				iTemp <<= 16;
				stBin.iSize |= iTemp;
			}
			break;
		case GSP_MEDIA_ATTRI_NAME_BINARY_LEN2 :
			{
				//获取 长的 32~48 位
				iTemp = pMediaInfo->aRows[i].iValue;
				iTemp <<= 32;
				stBin.iSize |= iTemp;
			}
			break;
		case GSP_MEDIA_ATTRI_NAME_BINARY_LEN3 :
			{
				//获取 长的 48~64 位
				iTemp = pMediaInfo->aRows[i].iValue;
				iTemp <<= 48;
				stBin.iSize |= iTemp;
			}
			break;
		case GSP_MEDIA_ATTRI_NAME_MEDIATYPE :
		case GSP_MEDIA_ATTRI_NAME_SECTION_ID :
			break;
		default :
			{
				MY_LOG_WARN(g_pLog, "GSP binary section exist unknown attri name %d\n", 
					pMediaInfo->aRows[i].iNname );
			}
			break;
		}
	}
}


void CGspMediaFormat::FormatOSD( const StruMediaInfoTable *pMediaInfo,StruOSDDescri &stOSD, UINT iIdxStart, UINT iIdxEnd)
{
	for( UINT i=iIdxStart;  i<iIdxEnd; i++ )
	{
		switch ( pMediaInfo->aRows[i].iNname )
		{
		case GSP_MEDIA_ATTRI_NAME_OSD_X :
			{
				//获取OSD叠加的坐标 X
				stOSD.iPosX = pMediaInfo->aRows[i].iValue;
			}
			break;
		case GSP_MEDIA_ATTRI_NAME_OSD_Y :
			{
				///获取OSD叠加的坐标 Y
				stOSD.iPosY = pMediaInfo->aRows[i].iValue;
			}
			break;
		case GSP_MEDIA_ATTRI_NAME_OSD_DATA_TYPE :
			{
				//OSD叠加 数据类型
				stOSD.iDataType  = (UINT8)pMediaInfo->aRows[i].iValue;               
			}
			break;
		case GSP_MEDIA_ATTRI_NAME_OSD_TRANSPARENCY :
			{
				//OSD叠加 叠加的透明度
				stOSD.iTransparency  = (UINT8)pMediaInfo->aRows[i].iValue;               
			}
			break;
		case GSP_MEDIA_ATTRI_NAME_MEDIATYPE :
		case GSP_MEDIA_ATTRI_NAME_SECTION_ID :
			break;
		default :
			{
				MY_LOG_WARN(g_pLog, "GSP osd section exist unknown attri name %d\n", 
					pMediaInfo->aRows[i].iNname );
			}
			break;
		}
	}
}

void CGspMediaFormat::FormatPicture( const StruMediaInfoTable *pMediaInfo,StruPictureDescri &stPicture, UINT iIdxStart, UINT iIdxEnd)
{
	for( UINT i=iIdxStart;  i<iIdxEnd; i++ )
	{
		switch ( pMediaInfo->aRows[i].iNname )
		{  
		case GSP_MEDIA_ATTRI_NAME_CODE_ID :
			{
				//获取 编码类型
				stPicture.eCodeID = (EnumGSCodeID)pMediaInfo->aRows[i].iValue;
			}
			break;
		case GSP_MEDIA_ATTRI_NAME_MEDIATYPE :
		case GSP_MEDIA_ATTRI_NAME_SECTION_ID :
			break;
		default :
			{
				MY_LOG_WARN(g_pLog,  "GSP video section exist unknown attri name %d\n", 
					pMediaInfo->aRows[i].iNname );
			}
			break;
		}
	}
}

void CGspMediaFormat::FormatAudio( const StruMediaInfoTable *pMediaInfo,StruAudioDescri &stAudio, UINT iIdxStart, UINT iIdxEnd)
{
	for( UINT i=iIdxStart;  i<iIdxEnd; i++ )
	{
		switch ( pMediaInfo->aRows[i].iNname )
		{
		case GSP_MEDIA_ATTRI_NAME_CODE_ID :
			{
				//获取 编码类型
				stAudio.eCodeID = (EnumGSCodeID)pMediaInfo->aRows[i].iValue;
			}
			break;
		case GSP_MEDIA_ATTRI_NAME_AUDIO_SAMPLE :
			{
				//获取 采用频率
				stAudio.iSample = (EnumGSCodeID)pMediaInfo->aRows[i].iValue;
			}
			break;
		case GSP_MEDIA_ATTRI_NAME_AUDIO_BITS :
			{
				//获取音频的采样位数
				stAudio.iBits = pMediaInfo->aRows[i].iValue;
			}
			break;
		case GSP_MEDIA_ATTRI_NAME_AUDIO_CHANNELS :
			{
				///获取通道数
				stAudio.iChannels = pMediaInfo->aRows[i].iValue;
			}
			break;
		case GSP_MEDIA_ATTRI_NAME_MEDIATYPE :
		case GSP_MEDIA_ATTRI_NAME_SECTION_ID :
			break;
		default :
			{
				MY_LOG_WARN(g_pLog,  "GSP audio section exist unknown attri name %d\n", 
					pMediaInfo->aRows[i].iNname );
			}
			break;
		}
	}
}

void CGspMediaFormat::FormatVideo( const   StruMediaInfoTable *pMediaInfo,StruVideoDescri &stVideo, UINT iIdxStart, UINT iIdxEnd)
{
	for( UINT i=iIdxStart;  i<iIdxEnd; i++ )
	{
		switch ( pMediaInfo->aRows[i].iNname )
		{
		case GSP_MEDIA_ATTRI_NAME_CODE_ID :
			{
				//获取 编码类型
				stVideo.eCodeID = (EnumGSCodeID)pMediaInfo->aRows[i].iValue;
			}
			break;
		case GSP_MEDIA_ATTRI_NAME_VIDEO_WIDTH :
			{
				//获取图像宽
				stVideo.iWidth = pMediaInfo->aRows[i].iValue;
			}
			break;
		case GSP_MEDIA_ATTRI_NAME_VIDEO_HEIGHT :
			{
				///获取图像高
				stVideo.iHeight = pMediaInfo->aRows[i].iValue;
			}
			break;
		case GSP_MEDIA_ATTRI_NAME_VIDEO_FRAMERATE :
			{
				//获取帧率
				stVideo.iFrameRate = ((pMediaInfo->aRows[i].iValue>>8)&0xFF);
				stVideo.iFrameRate2 = pMediaInfo->aRows[i].iValue&0xFF;
			}
			break;
		case GSP_MEDIA_ATTRI_NAME_MEDIATYPE :
		case GSP_MEDIA_ATTRI_NAME_SECTION_ID :
			break;
		default :
			{
				MY_LOG_WARN(g_pLog,  "GSP video section exist unknown attri name %d\n", 
					pMediaInfo->aRows[i].iNname );
			}
			break;
		}
	}
}



BOOL CGspMediaFormat::StructToInfo( const StruMediaInfoTable *pTable, CMediaInfo &csResult )
{
	UINT iSectionBegin;
	UINT iSectionEnd;
	EnumGSMediaType eType;
	BOOL bRet = TRUE;
	csResult.Clear();
	StruGSMediaDescri stDescri;
	INT iChn;
	for( UINT i =0; i<pTable->iRows; i++ )
	{
		if( pTable->aRows[i].iNname == GSP_MEDIA_ATTRI_NAME_SECTION_BEGIN )
		{
			//开始一媒体描述
			i++;
			iSectionBegin = i;
			iSectionEnd = 0;
			eType = GS_MEDIA_TYPE_NONE;
			iChn = -1;
			for( i++;i<pTable->iRows; i++)
			{
				if( pTable->aRows[i].iNname == GSP_MEDIA_ATTRI_NAME_SECTION_END )
				{
					iSectionEnd = i;
					eType = FormatSection(pTable, stDescri,iChn, iSectionBegin, iSectionEnd);
					break;
				}
			}
			if( eType != GS_MEDIA_TYPE_NONE )
			{
				//获取到一个新的媒体属性
				if( !csResult.AddChannel( &stDescri, iChn, NULL ) )
				{
					bRet = FALSE;
				}
			}
		}           
	}
	return bRet;
}

BOOL CGspMediaFormat::InfoToStruct( const CMediaInfo &csInfo,  StruMediaInfoTable *pResBuf, INT iBufSize  )
{
	bzero( pResBuf, iBufSize);
	INT iRow = 0,iRowTemp;
	INT iCnts = csInfo.GetChannelNums();
	StruGSMediaDescri stDescri; 
	CMediaInfo *pInfo = const_cast<CMediaInfo*>(&csInfo);
	iBufSize -= sizeof(UINT32);
	const CIMediaInfo::StruMediaChannelInfo *pChnInfo;
	for( INT i = 0; i<iCnts; i++ )
	{

		bzero( &stDescri, sizeof( stDescri));

		pChnInfo = pInfo->GetChannel( i );
		if( !pChnInfo )
		{
			GS_ASSERT(0);
			continue;       
		}


		memcpy(&stDescri, &pChnInfo->stDescri, sizeof(stDescri) );
		iRowTemp = iRow;

		//开始一个Section  
		if( iBufSize < (INT)( (iRow+1)*sizeof(StruMediaAttribute) ) )
		{
			GS_ASSERT(0);
			return FALSE;
		}
		pResBuf->aRows[iRow].iNname = GSP_MEDIA_ATTRI_NAME_SECTION_BEGIN;
		pResBuf->aRows[iRow].iValue = 0;
		iRow++;

		//通道号
		if( iBufSize< (INT)( (iRow+1)*sizeof(StruMediaAttribute) ) )
		{
			GS_ASSERT(0);
			return FALSE;
		}
		pResBuf->aRows[iRow].iNname = GSP_MEDIA_ATTRI_NAME_SECTION_ID;
		pResBuf->aRows[iRow].iValue = i;
		iRow++;

		//媒体类型
		if( iBufSize< (INT)( (iRow+1)*sizeof(StruMediaAttribute) ) )
		{
			GS_ASSERT(0);
			return FALSE;
		}
		pResBuf->aRows[iRow].iNname = GSP_MEDIA_ATTRI_NAME_MEDIATYPE;
		pResBuf->aRows[iRow].iValue = stDescri.eMediaType;
		iRow++;



		switch( stDescri.eMediaType )
		{
		case GS_MEDIA_TYPE_VIDEO :
			{    //视频数据


				//编码类型 
				if( iBufSize< (INT)( (iRow+1)*sizeof(StruMediaAttribute) ) )
				{
					GS_ASSERT(0);
					return FALSE;
				}
				pResBuf->aRows[iRow].iNname = GSP_MEDIA_ATTRI_NAME_CODE_ID;
				pResBuf->aRows[iRow].iValue = stDescri.unDescri.struVideo.eCodeID;
				iRow++;

				//图像宽
				if( iBufSize<(INT)( (iRow+1)*sizeof(StruMediaAttribute) ) )
				{
					GS_ASSERT(0);
					return FALSE;
				}
				pResBuf->aRows[iRow].iNname = GSP_MEDIA_ATTRI_NAME_VIDEO_WIDTH;
				pResBuf->aRows[iRow].iValue = stDescri.unDescri.struVideo.iWidth;
				iRow++;

				///图像高
				if( iBufSize< (INT)( (iRow+1)*sizeof(StruMediaAttribute) ) )
				{
					GS_ASSERT(0);
					return FALSE;
				}
				pResBuf->aRows[iRow].iNname = GSP_MEDIA_ATTRI_NAME_VIDEO_HEIGHT;
				pResBuf->aRows[iRow].iValue = stDescri.unDescri.struVideo.iHeight;
				iRow++;


				//帧率
				if( iBufSize<(INT)( (iRow+1)*sizeof(StruMediaAttribute) ) )
				{
					GS_ASSERT(0);
					return FALSE;
				}
				pResBuf->aRows[iRow].iNname = GSP_MEDIA_ATTRI_NAME_VIDEO_FRAMERATE;
				UINT16 r1, r2;
				r1 = stDescri.unDescri.struVideo.iFrameRate;
				r2 = stDescri.unDescri.struVideo.iFrameRate2;
				pResBuf->aRows[iRow].iValue = (r1<<8)|(r2&0xFF);
				iRow++;
			}
			break;
		case GS_MEDIA_TYPE_AUDIO :
			{
				//音频流, 属性描述

				//获取 编码类型
				if( iBufSize<(INT)( (iRow+1)*sizeof(StruMediaAttribute) ) )
				{
					GS_ASSERT(0);
					return FALSE;
				}
				pResBuf->aRows[iRow].iNname = GSP_MEDIA_ATTRI_NAME_CODE_ID;
				pResBuf->aRows[iRow].iValue = stDescri.unDescri.struAudio.eCodeID;
				iRow++;


				//获取 采用频率
				if( iBufSize<(INT)( (iRow+1)*sizeof(StruMediaAttribute) ) )
				{
					GS_ASSERT(0);
					return FALSE;
				}
				pResBuf->aRows[iRow].iNname = GSP_MEDIA_ATTRI_NAME_AUDIO_SAMPLE;
				pResBuf->aRows[iRow].iValue = stDescri.unDescri.struAudio.iSample;
				iRow++;


				//获取音频的采样位数
				if( iBufSize<(INT)( (iRow+1)*sizeof(StruMediaAttribute) ) )
				{
					GS_ASSERT(0);
					return FALSE;
				}
				pResBuf->aRows[iRow].iNname = GSP_MEDIA_ATTRI_NAME_AUDIO_BITS;
				pResBuf->aRows[iRow].iValue = stDescri.unDescri.struAudio.iBits;
				iRow++;

				//获取通道数
				pResBuf->aRows[iRow].iNname = GSP_MEDIA_ATTRI_NAME_AUDIO_CHANNELS;
				pResBuf->aRows[iRow].iValue = stDescri.unDescri.struAudio.iChannels;
				iRow++;

			}
			break;
		case GS_MEDIA_TYPE_PICTURE:
			{
				//图片数据,属性描述 

				//获取 编码类型
				if( iBufSize<(INT)( (iRow+1)*sizeof(StruMediaAttribute) ) )
				{
					GS_ASSERT(0);
					return FALSE;
				}
				pResBuf->aRows[iRow].iNname = GSP_MEDIA_ATTRI_NAME_CODE_ID;
				pResBuf->aRows[iRow].iValue = stDescri.unDescri.struPicture.eCodeID;
				iRow++;
			}
			break;   

		case      GS_MEDIA_TYPE_OSD:
			{
				//OSD 数据,属性描述

				//获取OSD叠加的坐标 X
				if( iBufSize<(INT)( (iRow+1)*sizeof(StruMediaAttribute) ) )
				{
					GS_ASSERT(0);
					return FALSE;
				}
				pResBuf->aRows[iRow].iNname = GSP_MEDIA_ATTRI_NAME_OSD_X;
				pResBuf->aRows[iRow].iValue = stDescri.unDescri.struOsd.iPosX;
				iRow++;


				///获取OSD叠加的坐标 Y
				if( iBufSize<(INT)( (iRow+1)*sizeof(StruMediaAttribute) ) )
				{
					GS_ASSERT(0);
					return FALSE;
				}
				pResBuf->aRows[iRow].iNname = GSP_MEDIA_ATTRI_NAME_OSD_Y;
				pResBuf->aRows[iRow].iValue = stDescri.unDescri.struOsd.iPosY;
				iRow++;

				//OSD叠加 数据类型
				if( iBufSize<(INT)( (iRow+1)*sizeof(StruMediaAttribute) ) )
				{
					GS_ASSERT(0);
					return FALSE;
				}
				pResBuf->aRows[iRow].iNname = GSP_MEDIA_ATTRI_NAME_OSD_DATA_TYPE;
				pResBuf->aRows[iRow].iValue = stDescri.unDescri.struOsd.iDataType;
				iRow++;

				//OSD叠加 叠加的透明度
				if( iBufSize<(INT)( (iRow+1)*sizeof(StruMediaAttribute) ) )
				{
					GS_ASSERT(0);
					return FALSE;
				}
				pResBuf->aRows[iRow].iNname = GSP_MEDIA_ATTRI_NAME_OSD_TRANSPARENCY;
				pResBuf->aRows[iRow].iValue = stDescri.unDescri.struOsd.iTransparency;
				iRow++;


			}
			break;
		case     GS_MEDIA_TYPE_BINARY :
			{         
				//二进制流, 属性描述 

				//获取 长的 1~16 位
				if( iBufSize<(INT)( (iRow+1)*sizeof(StruMediaAttribute) ) )
				{
					GS_ASSERT(0);
					return FALSE;
				}
				pResBuf->aRows[iRow].iNname = GSP_MEDIA_ATTRI_NAME_BINARY_LEN;
				pResBuf->aRows[iRow].iValue = stDescri.unDescri.struBinary.iSize&0xFFFF;
				iRow++;

				//获取 长的 16~32 位
				if( iBufSize<(INT)( (iRow+1)*sizeof(StruMediaAttribute) ) )
				{
					GS_ASSERT(0);
					return FALSE;
				}
				pResBuf->aRows[iRow].iNname = GSP_MEDIA_ATTRI_NAME_BINARY_LEN1;
				pResBuf->aRows[iRow].iValue = (stDescri.unDescri.struBinary.iSize>>16)&0xFFFF;
				iRow++;

				//获取 长的 32~48 位
				pResBuf->aRows[iRow].iNname = GSP_MEDIA_ATTRI_NAME_BINARY_LEN2;
				pResBuf->aRows[iRow].iValue = (stDescri.unDescri.struBinary.iSize>>32)&0xFFFF;
				iRow++;


				//获取 长的 48~64 位
				if( iBufSize<(INT)( (iRow+1)*sizeof(StruMediaAttribute) ) )
				{
					GS_ASSERT(0);
					return FALSE;
				}
				pResBuf->aRows[iRow].iNname = GSP_MEDIA_ATTRI_NAME_BINARY_LEN3;
				pResBuf->aRows[iRow].iValue = (stDescri.unDescri.struBinary.iSize>>48)&0xFFFF;
				iRow++; 

			}
			break;

		case       GS_MEDIA_TYPE_SYSHEADER :
			{
				//信息头
				if( iBufSize<(INT)( (iRow+1)*sizeof(StruMediaAttribute) ) )
				{
					GS_ASSERT(0);
					return FALSE;
				}
				pResBuf->aRows[iRow].iNname = GSP_MEDIA_ATTRI_NAME_SYSHEADER_LEN;
				pResBuf->aRows[iRow].iValue = stDescri.unDescri.struSysHeader.iSize&0xFFFF;
				iRow++;

				if( iBufSize<(INT)( (iRow+1)*sizeof(StruMediaAttribute) ) )
				{
					GS_ASSERT(0);
					return FALSE;
				}
				pResBuf->aRows[iRow].iNname = GSP_MEDIA_ATTRI_NAME_SYSHEADER_LEN1;
				pResBuf->aRows[iRow].iValue = (stDescri.unDescri.struSysHeader.iSize>>16)&0xFFFF;
				iRow++;                 
			}
			break;
		default :
			{
				MY_LOG_WARN(g_pLog, " Unknown media type: %d\n",stDescri.eMediaType );
				iRow = iRowTemp;
				GS_ASSERT(0);
			}
			break;

		}
		if( iRowTemp != iRow )
		{
			//Section结束
			if( iBufSize<(INT)( (iRow+1)*sizeof(StruMediaAttribute) ) )
			{
				GS_ASSERT(0);
				return FALSE;
			}
			pResBuf->aRows[iRow].iNname = GSP_MEDIA_ATTRI_NAME_SECTION_END;
			pResBuf->aRows[iRow].iValue = 0;
			iRow++;
		}
	}
	pResBuf->iRows = iRow;
	return TRUE;

}






namespace GSP
{

	struct _StruGsp2GssErrnoTable
	{
		const EnumErrno eGssRet;
		const INT iGspRet;
	};



	struct _StruGsp2GssErrnoTable s_vTable[] =
	{	
		{eERRNO_SUCCESS, GSP::GSP_PRO_RET_SUCCES},
		{eERRNO_SYS_EBUSY, GSP::GSP_PRO_RET_EBUSY},
		{eERRNO_SYS_ENMEM, GSP::GSP_PRO_RET_EBUSY},
		{eERRNO_SYS_EPERMIT, GSP::GSP_PRO_RET_EPERMIT},
		{eERRNO_SRC_ECLOSE, GSP::GSP_PRO_RET_ESTREAM_ASSERT},

		{eERRNO_SYS_EINVALID, GSP::GSP_PRO_RET_EINVALID},
		{eERRNO_SYS_ETIMEOUT, GSP::GSP_PRO_RET_EKP},
		{eERRNO_SYS_EPRO, GSP::GSP_PRO_RET_EPRO},


	};


	INT32 ErrnoLocal2Gsp(EnumErrno eGSSRet )
	{
		for( int i = 0; i<ARRARY_SIZE(s_vTable); i++ )
		{
			if( s_vTable[i].eGssRet==eGSSRet )
			{
				return s_vTable[i].iGspRet;
			}
		}
		return GSP_PRO_RET_EUNKNOWN;
	}


	EnumErrno ErrnoGsp2Local(INT32 iGspRet )
	{
		for( int i = 0; i<ARRARY_SIZE(s_vTable); i++ )
		{
			if( s_vTable[i].iGspRet ==iGspRet )
			{
				return s_vTable[i].eGssRet;
			}
		}
		return eERRNO_EUNKNOWN;
	}

} //end namespace GSP
