#include "RTPAnalyer.h"
#include "RtpProFrame.h"

using namespace GSP;
using namespace GSP::RTP;

//#define MY_DEBUG printf
#define MY_DEBUG(...)

#if 0
static  void MY_DEBUG(const char * lpFormat, ...)
{
	char szBuf[1024];
	va_list marker;

	va_start( marker, lpFormat );
	int i = vsnprintf( szBuf,1024,lpFormat, marker );    
	va_end( marker );
	if( i< 1)
	{
		return;
	}
	szBuf[1023] = '\0';
	OutputDebugString((LPCTSTR) szBuf );
}
#endif


/*
*********************************************************************
*
*@brief :  CRtpPacketMerger
*
*********************************************************************
*/


namespace GSP
{

namespace RTP
{

static  void _FreeListMember_ProPacket( CRtpProPacket *p )
{
	p->UnrefObject();
}
static  void _FreeListMember_ProFrame( CRtpProFrame *p )
{
	p->UnrefObject();
}

static  void _FreeListMember_FrameCache( CFrameCache *p )
{
	p->UnrefObject();
}

INLINE static  INT _CmpTimestamp(UINT32 iNew, UINT32 iOld )
{
	if( iNew==iOld )
	{
		return 0;
	}
	if( iNew>iOld )
	{
		return iNew-iOld;
	}
	if( (iOld-iNew) > 3000000 /*0x7fffffff*/ )
	{
		//已经循环
		return 1;
	}
	return -1;
}

INLINE static INT _CountSeqCmp(UINT16 iNew, UINT16 iOld )
{
	if( iNew==iOld )
	{
		return 0;
	}
	if( iNew>iOld )
	{
		return iNew-iOld;
	}
	if( (iOld-iNew) > 0x7fff )
	{
		//循环 
		return ((int)iNew)+0xffff-iOld+1;
	}
	return ((int)iNew-(int)iOld);
}

INLINE static INT _CountSeq(UINT16 iNew, UINT16 iOld )
{
	if( iNew==iOld )
	{
		return 0;
	}
	if( iNew>iOld )
	{
		return iNew-iOld;
	}
	return ((int)iNew)+0xffff-iOld+1;
}


} //end namespace RTP

} //end namespace GSP

CRtpPacketMerger::CRtpPacketMerger(void) : CGSPObject()
{
	m_listFinish.SetFreeCallback((FuncPtrFree)_FreeListMember_ProFrame);
	for( int i = 0; i<GSP_MAX_MEDIA_CHANNELS; i++ )
	{
		m_vChannel[i].Init((FuncPtrFree)_FreeListMember_ProPacket);
	}
	
}

CRtpPacketMerger::~CRtpPacketMerger(void)
{	
	m_listFinish.Clear();
	for( int i = 0; i<GSP_MAX_MEDIA_CHANNELS; i++ )
	{
		m_vChannel[i].Clear();
	}
	

}


EnumErrno CRtpPacketMerger::Rcv( CGSPBuffer *pBuffer, std::vector<StruRtpPktSn> &vLoser )
{
	CRtpProPacket *pPkt, *pTempPkt;
	EnumErrno eRet = eERRNO_SUCCESS;
	CRtpHeader *pH;
	CList::CIterator<CRtpProPacket *> csIt;
	INT iCmp;
	INT iCmpSeq;
	BOOL bAddNew = TRUE;
	BOOL bNBreak=TRUE;
	UINT32 iCurTs;
	UINT16 iCurSeq, iTempSeq;
	INT iChn = -1; 

	UINT64 iTs;
	UINT32 iFirstTs;
	UINT32 iExistFrames;
	UINT32 iCmpFrames;
	INT bFirstCmp;
	UINT16 iOldSeq;
	BOOL bOk = TRUE;
	CRtpProFrame *pProFrame = NULL;
	INT iM;
	
	INT iTemp;
	pPkt = pPkt->Create(pBuffer);
	if( !pPkt )
	{
		return eERRNO_SYS_ENMEM;
	}
	pH = &pPkt->GetHeader();


	iCurTs = pH->GetTimestamp();
	iCurSeq = pH->GetSequenceNumber();
	
	//转换通道号
	for(int i = 0; i<GSP_MAX_MEDIA_CHANNELS; i++  )
	{
		if( m_vChannel[i].iPt == pH->GetPayloadType() )
		{
			iChn = i;
			if( m_vChannel[i].iSSRC != pH->GetSyncSource() )
			{
				m_vChannel[iChn].iLastTs=MAX_UINT64;
				m_vChannel[iChn].iLastSeq = MAX_UINT32;
			}
			break;
		}
		else if( m_vChannel[i].iPt==MAX_UINT32 )
		{
			//第一个 
			m_vChannel[i].iPt =  pH->GetPayloadType();
			m_vChannel[i].iSSRC = pH->GetSyncSource();
			iChn = i;
			break;
		}
	}
	
	
	


	if( iChn<0   )
	{
		//不合法的通道号
		GS_ASSERT(0);
		eRet =  eERRNO_SYS_EFLOWOUT;
		goto exit_func;
	}

	csIt = m_vChannel[iChn].lRcvPacket.First<CRtpProPacket *>();
	bAddNew = TRUE;

// 	MY_DEBUG("Rcv TS:%lu, Seq:%d, iM:%d.\n",
// 		(unsigned long) iCurTs, 
// 		(int) iCurSeq,
// 		(int)  pH->GetMarker() );

	if( csIt.IsOk() && m_vChannel[iChn].iLastTs!=MAX_UINT64 )
	{
		iCmp = _CmpTimestamp(iCurTs,(UINT32) m_vChannel[iChn].iLastTs );
		if( iCmp<=0 )
		{
			//过时的数据
			MY_DEBUG("TS:%ld, Seq:%d, iM:%d out of day.\n",
				(unsigned long) iCurTs, 
				(int) iCurSeq,
				(int)  pH->GetMarker() );
			eRet = eERRNO_SUCCESS;
			goto exit_func;
		}
	}
	else
	{
		if( iCurTs>1200)
		{
			m_vChannel[iChn].iLastTs = iCurTs-1200;
		}
		else
		{	
			m_vChannel[iChn].iLastTs = iCurTs>>2;
		}
	}


	csIt = m_vChannel[iChn].lRcvPacket.First<CRtpProPacket *>();

	eRet = eERRNO_ENONE;
	for(bNBreak=TRUE; bNBreak && csIt.IsOk(); csIt.Next() )
	{		
		pTempPkt = csIt.Data();
		iCmp = _CmpTimestamp(iCurTs, pTempPkt->GetHeader().GetTimestamp() );
		if( iCmp==0 )
		{
			//相等 比较 Seq
			bAddNew = FALSE;

			bNBreak = FALSE; //中断外部循环
			for( ;csIt.IsOk(); csIt.Next() )
			{
				pTempPkt = csIt.Data();
				iTempSeq = pTempPkt->GetHeader().GetSequenceNumber();
				if( iCurSeq== iTempSeq)
				{
					//已经存在
					MY_DEBUG("TS:%ld, Seq:%d, iM:%d exist.\n",
						(unsigned long) iCurTs, 
						(int) iCurSeq,
						(int)  pH->GetMarker() );
					eRet = eERRNO_SUCCESS;
					goto exit_func;
				}
				iCmpSeq = _CountSeqCmp(iCurSeq, iTempSeq);
				if( iCmpSeq< 0 
					|| iCurTs != pTempPkt->GetHeader().GetTimestamp() )
				{
					//加入到前面
					eRet = m_vChannel[iChn].lRcvPacket.Prepend(csIt.Node(), pPkt);
					
					if( iCurTs != pTempPkt->GetHeader().GetTimestamp() 
						&& iCmpSeq == -1 )
					{
						//结束包
						pH->SetMarker(1);
					}
					if( eERRNO_SUCCESS == eRet )
					{
						pPkt->RefObject();
					}
					break;
				}
			}
		}
		else if( iCmp<0 )
		{
			//加入到前面
			eRet = m_vChannel[iChn].lRcvPacket.Prepend(csIt.Node(), pPkt);
			bAddNew = FALSE;
			iCmpSeq = _CountSeqCmp(iCurSeq, pTempPkt->GetHeader().GetSequenceNumber());
			if( iCmpSeq == -1 )
			{
				//结束包
				pH->SetMarker(1);
			}
			if( eERRNO_SUCCESS == eRet )
			{
				pPkt->RefObject();
			}
			break;
		}
	}

	if( eRet == eERRNO_ENONE)
	{
		//加到后面
		pTempPkt = (CRtpProPacket *)m_vChannel[iChn].lRcvPacket.LastData();
		eRet = m_vChannel[iChn].lRcvPacket.AddTail(pPkt);
		if( pTempPkt && iCurTs != pTempPkt->GetHeader().GetTimestamp() &&
			_CountSeqCmp(iCurSeq, pTempPkt->GetHeader().GetSequenceNumber() )==1 )
		{
			pTempPkt->GetHeader().SetMarker(1);
		}
		if( eERRNO_SUCCESS == eRet )
		{
			pPkt->RefObject();
		}
	}


	if(eERRNO_SUCCESS!=eRet )
	{
		//丢帧	
		GS_ASSERT(0);		
		eRet =  eERRNO_SYS_ENMEM; //没有内存
		goto exit_func;
	}

	

	csIt = m_vChannel[iChn].lRcvPacket.First<CRtpProPacket *>();

	pTempPkt = csIt.Data();

	 iTs = pTempPkt->GetHeader().GetTimestamp();
	 iFirstTs = (UINT32) iTs;
	 iExistFrames = 1; //当前收到的帧数

	 iCmpFrames = 0; //完成的帧数
	 bFirstCmp = 0; //第一帧结束

	if( pTempPkt->GetHeader().GetMarker() )
	{
		bFirstCmp = TRUE;
		iCmp++;
	}


// 	MY_DEBUG("******************<<<<\n");
// 	MY_DEBUG("TS:%d, iM:%d, Seq:%d ???.\n",
// 		(int) pTempPkt->GetHeader().GetTimestamp(), 
// 		(int) pTempPkt->GetHeader().GetMarker(),
// 		(int) pTempPkt->GetHeader().GetSequenceNumber());



	//统计帧数据
	for( csIt.Next() ;csIt.IsOk(); csIt.Next() )
	{
		pTempPkt = csIt.Data();
		if( iTs != pTempPkt->GetHeader().GetTimestamp()  )
		{			
			iExistFrames++;
			iTs = pTempPkt->GetHeader().GetTimestamp() ;
		}
		if( pTempPkt->GetHeader().GetMarker()  )
		{
			iCmpFrames++;
			if( iTs==iFirstTs )
			{
				bFirstCmp = TRUE;
			}
		}

// 		MY_DEBUG("TS:%d, iM:%d, Seq:%d ???.\n",
// 			(int) pTempPkt->GetHeader().GetTimestamp(), 
// 			(int) pTempPkt->GetHeader().GetMarker(),
// 			(int) pTempPkt->GetHeader().GetSequenceNumber());
	}

// 	MY_DEBUG("******************exist: %d, cmp: %d<<<<\n", 
// 		iExistFrames, iCmpFrames);

	eRet = eERRNO_SUCCESS;

	if( iCmpFrames > 0 && bFirstCmp )
	{
		//检查第一帧是否完成

		//检查第一帧是否为结束包
		csIt = m_vChannel[iChn].lRcvPacket.First<CRtpProPacket *>();
		pTempPkt = csIt.Data();
		iFirstTs =  pTempPkt->GetHeader().GetTimestamp() ;
	
		if( m_vChannel[iChn].iLastSeq > MAX_UINT16 ) //上一帧的CSeq
		{
			//首帧
			iOldSeq = pTempPkt->GetHeader().GetSequenceNumber()-1;
		}
		else
		{
			iOldSeq = m_vChannel[iChn].iLastSeq;
		}

		bOk = TRUE;
		iCmp = _CountSeq(pTempPkt->GetHeader().GetSequenceNumber(), iOldSeq);
		if( iCmp == 1 )
		{
			iOldSeq = pTempPkt->GetHeader().GetSequenceNumber();
			for( csIt.Next() ;csIt.IsOk();  csIt.Next() )
			{		
				pTempPkt = csIt.Data();
				if( iFirstTs != pTempPkt->GetHeader().GetTimestamp() )
				{
					break;
				}
				iCmp = _CountSeq(pTempPkt->GetHeader().GetSequenceNumber(), iOldSeq);
				if( iCmp != 1 )
				{
					//丢帧
					bOk = FALSE;
					break;
				}
				iOldSeq = pTempPkt->GetHeader().GetSequenceNumber();
			}
		}
		else
		{
			//丢帧
			bOk = FALSE;				
		}

		if( bOk )
		{
			//有一帧数据
			pProFrame = pProFrame->Create();

			if( !pProFrame )
			{
				eRet = eERRNO_SYS_ENMEM;
				goto exit_func;
			}

			

			bOk = TRUE;
			iTemp = 0;
			bFirstCmp = FALSE;			
					MY_DEBUG("<<====FRM===\r\n");

				do 
				{
					pTempPkt = NULL;
					m_vChannel[iChn].lRcvPacket.RemoveFront((void**)&pTempPkt);
					if(!pTempPkt)
					{
						GS_ASSERT(0);						
						eRet = eERRNO_SYS_ESTATUS;
						bOk = FALSE;						
						break;
					}
					iTemp++;
					m_vChannel[iChn].iLastSeq = pTempPkt->GetHeader().GetSequenceNumber();
					m_vChannel[iChn].iLastTs =  pTempPkt->GetHeader().GetTimestamp();
					iM = pTempPkt->GetHeader().GetMarker();
					// 						if( iM )
					// 						{
					// 							//pFrame->m_bKey =   //TODO 关键帧
					// 						}

					MY_DEBUG("TS:0x%08x, iM:%d, Seq:%d ***. Size: %d\n",
						(int) m_vChannel[iChn].iLastTs, 
						(int) iM,(int) m_vChannel[iChn].iLastSeq,
						(int) pTempPkt->GetParser().GetTotalSize() );

					if(bOk && eERRNO_SUCCESS != pProFrame->AppendBack(pTempPkt) )
					{
						GS_ASSERT(0);
						//丢帧
						bOk = FALSE;							
						SAFE_DESTROY_REFOBJECT(&pTempPkt);
						eRet = eERRNO_SYS_ESTATUS;
						break;
					}
					SAFE_DESTROY_REFOBJECT(&pTempPkt);
				} while ( iM == 0 );	
					MY_DEBUG(">>====\r\n");

				GS_ASSERT(iTemp);
				if( bOk )
				{
					//一帧数据完成					
					if( m_listFinish.AddTail(pProFrame) )
					{
						SAFE_DESTROY_REFOBJECT(&pProFrame);
						eRet = eERRNO_SYS_ENMEM;	
					}
					else
					{
						iCmpFrames--;
						iExistFrames--;
						eRet = eERRNO_SUCCESS;
						pProFrame = NULL;
					}
				}
				else
				{
					//丢帧
					SAFE_DESTROY_REFOBJECT(&pProFrame);					
				}
			} 
		}


	if( (bFirstCmp && iCmpFrames>2 ) || iExistFrames>5 )
	{
		//丢帧
		if( eRet)
		{
			eRet = eERRNO_SYS_EFLOWOUT;
		}
		MY_DEBUG(" Flowout out lose frame.\r\n");
		m_vChannel[iChn].lRcvPacket.RemoveFront((void**)&pTempPkt);
		iExistFrames--;
		bFirstCmp = FALSE;
		if( pTempPkt )
		{
			iTs =  pTempPkt->GetHeader().GetTimestamp();
			m_vChannel[iChn].iLastSeq = pTempPkt->GetHeader().GetSequenceNumber();
			m_vChannel[iChn].iLastTs = iTs;			
		}
		SAFE_DESTROY_REFOBJECT(&pTempPkt);

		while( (pTempPkt = (CRtpProPacket *)m_vChannel[iChn].lRcvPacket.FirstData()) ) 
		{				
			if( pTempPkt->GetHeader().GetTimestamp() == iTs )
			{
				m_vChannel[iChn].iLastSeq = pTempPkt->GetHeader().GetSequenceNumber();
				m_vChannel[iChn].iLastTs = pTempPkt->GetHeader().GetTimestamp();
				m_vChannel[iChn].lRcvPacket.RemoveFront((void**)&pTempPkt);
				if( pTempPkt->GetHeader().GetMarker() )
				{
					iCmpFrames--;
				}
				SAFE_DESTROY_REFOBJECT(&pTempPkt);
				MY_DEBUG(" Flowout out lose frame.\r\n");
			}
			else
			{
				break;
			}			
		};	

	}

	if( bFirstCmp || iCmpFrames>1 || iExistFrames>2 )
	{
		//丢帧数据
		csIt = m_vChannel[iChn].lRcvPacket.First<CRtpProPacket *>();
		eRet = eERRNO_SYS_ELOSE;
		if( csIt.IsOk() )
		{

			pTempPkt  = csIt.Data();
			iFirstTs = pTempPkt->GetHeader().GetTimestamp();
			UINT16 iOldSeq;
			if( m_vChannel[iChn].iLastSeq > MAX_UINT16 ) //上一帧的CSeq
			{
				//首帧
				iOldSeq = pTempPkt->GetHeader().GetSequenceNumber()-1;
			}
			else
			{
				iOldSeq = m_vChannel[iChn].iLastSeq;
			}
			StruRtpPktSn stSn;
			stSn.iPT = pTempPkt->GetHeader().GetTimestamp();
			stSn.iSSRC =  pTempPkt->GetHeader().GetSyncSource();
			for( csIt.Next(); csIt.IsOk(); csIt.Next() )
			{
				iCmp = _CountSeq(pTempPkt->GetHeader().GetSequenceNumber(), iOldSeq);

				if( iCmp == 1 )
				{		
					iOldSeq = pTempPkt->GetHeader().GetSequenceNumber();
					continue;
				}

				while( iCmp >0 )
				{
					iOldSeq++;
					stSn.iSeq = iOldSeq;
					vLoser.push_back(stSn);
					iCmp--;
				}
			}
		}
	}
	

exit_func :
	SAFE_DESTROY_REFOBJECT(&pPkt);
	SAFE_DESTROY_REFOBJECT(&pProFrame);
	return eRet;
		

}


/*
*********************************************************************
*
*@brief : CRtpDecoder
*
*********************************************************************
*/

CRtpDecoder::CRtpDecoder(void)
:CGSPObject()
{
	
}

CRtpDecoder::~CRtpDecoder(void)
{
	
}

EnumErrno CRtpDecoder::Decode( CGSPBuffer *pBuffer, std::vector<StruRtpPktSn> &vLoser )
{
	EnumErrno eRet = m_csMeger.Rcv(pBuffer, vLoser);
	return eRet;
}

CFrameCache *CRtpDecoder::Get(void)
{
	CRtpProFrame *pProFrame;
	CFrameCache *pRet;

	pProFrame = m_csMeger.Get();
	if( !pProFrame )
	{
		return NULL;
	}
	pRet = pRet->Create(pProFrame);
	if( !pRet )
	{
		pProFrame->UnrefObject();
		return NULL;
	}

	CRtpHeader *h = pProFrame->GetFirstHeader();
	if( !h )
	{
		GS_ASSERT(0);
		pRet->UnrefObject();
		pProFrame->UnrefObject();
		return NULL;
	}
	pRet->m_stFrameInfo.iChnNo = h->GetPayloadType();
	pRet->m_stFrameInfo.eMediaType = (EnumGSMediaType)h->GetSyncSource();
	pRet->m_stFrameInfo.iTimestamp = h->GetTimestamp();
	pProFrame->UnrefObject();

#if 0
static FILE *_pf = NULL;
		if( _pf == NULL )
		{
			_pf = fopen("D:\\test.ps", "wb+");
		}
		if( _pf )
		{
			fwrite( pRet->GetBuffer().m_bBuffer,1, pRet->GetBuffer().m_iDataSize,_pf );
		}
#endif
	return pRet;
}
