#include "StreamPackDecoder.h"

#include "SIP/ES2PS.h"
#include "RTP/RtpStru.h"
#include "SIP/PSAnalyzer.h"
#include <time.h>

using namespace GSP;
using namespace GSP::SIP;
using namespace GSP::RTP;



namespace GSP
{
	

/*
*********************************************************************
*
*@brief : CStPkDdBase CStreamPackDecoder 的基本实现
*
*********************************************************************
*/
	static void _FreeListMember(CFrameCache *p)
	{
		p->UnrefObject();
	}


	 EnumErrno CStPkDdBase::Init(BOOL bOutFactorStream)
	{			
		m_bOutFactorStream = bOutFactorStream;
		return eERRNO_SUCCESS;

	}		
	 CFrameCache *CStPkDdBase::Get(void)
	{
		void *p = NULL;
		m_listFrameCache.RemoveFront(&p);
		return (CFrameCache*)p;
	}

	 void CStPkDdBase::BindChannelInfo( UINT iChnNo, EnumGSMediaType eMType)
	{
		GS_ASSERT(iChnNo<GSP_MAX_MEDIA_CHANNELS );
		if( iChnNo<GSP_MAX_MEDIA_CHANNELS )
		{

			m_vInfoCtx[iChnNo].eMType = eMType;
		}

	}

	 void CStPkDdBase::BindChannelInfo( const CMediaInfo &csMediaInfo )
	{
		UINT iCnts = csMediaInfo.GetChannelNums();
		for( UINT16 i=0; i<iCnts; i++ )
		{
			const CIMediaInfo::StruMediaChannelInfo *p = csMediaInfo.GetChannel(i);
			if( p )
			{
				BindChannelInfo(p->iSubChannel,
					(EnumGSMediaType) p->stDescri.eMediaType);
			}
		}

	}
	 EnumGSCodeID CStPkDdBase::GetCodeId( UINT iChnNo )
	{
		GS_ASSERT(iChnNo<GSP_MAX_MEDIA_CHANNELS );
		if( iChnNo<GSP_MAX_MEDIA_CHANNELS )
		{
			return m_vInfoCtx[iChnNo].eCodeId;
		}
		return GS_CODEID_NONE;
	}

	 EnumGSMediaType CStPkDdBase::GetMeidaType(UINT iChnNo )
	{
		GS_ASSERT(iChnNo<GSP_MAX_MEDIA_CHANNELS );
		if( iChnNo<GSP_MAX_MEDIA_CHANNELS )
		{
			return m_vInfoCtx[iChnNo].eMType;
		}
		return GS_MEDIA_TYPE_NONE;
	}


	CStPkDdBase::CStPkDdBase(EnumStreamPackingType eSrcPktType) : CStreamPackDecoder()
	{
		m_bOutFactorStream = FALSE;
		m_eSrcPktType = eSrcPktType;
		m_listFrameCache.SetFreeCallback((FuncPtrFree)_FreeListMember);
		bzero(m_vInfoCtx, sizeof(m_vInfoCtx));
		for(UINT i=0; i<GSP_MAX_MEDIA_CHANNELS; i++ )
		{
			m_vInfoCtx[i].iChnNo = i;
		}


	}
	CStPkDdBase::~CStPkDdBase(void)
	{
		m_listFrameCache.Clear();
	}

	UINT16 CStPkDdBase::GetMediaChannel(EnumGSMediaType eMType )
	{
		for(UINT i = 0; i<GSP_MAX_MEDIA_CHANNELS; i++)
		{
			if(m_vInfoCtx[i].eMType == eMType )
			{
				return i;
			}

		}
		GS_ASSERT(0);
		return MAX_UINT16;
	}

	void CStPkDdBase::SetMediaCodeId(EnumGSMediaType eMType, EnumGSCodeID eCodeId )
	{
		for(UINT i = 0; i<GSP_MAX_MEDIA_CHANNELS; i++)
		{
			if(m_vInfoCtx[i].eMType == eMType )
			{
				m_vInfoCtx[i].eCodeId = eCodeId;
				return;
			}

		}
		GS_ASSERT(0);
	}
	/*
	*********************************************************************
	*
	*@brief : GSPS => 标准
	*
	*********************************************************************
	*/
	class CStPkDdGSPS  : public CStPkDdBase
	{
	private :

		INT m_iVideoGSStreamType;
		INT m_iAudioGSStreamType;
		BOOL m_bInvalid;
		INT m_iVideoChnNo;
		INT m_iAudioChnNo;
		UINT m_iTrysCounts;
	public :

		CStPkDdGSPS(void) : CStPkDdBase(eSTREAM_PKG_GSPS)
		{
			m_iVideoGSStreamType = -1;
			m_iAudioGSStreamType = -1;	
			m_bInvalid = FALSE;
			m_iVideoChnNo = 0;
			m_iAudioChnNo = 1;
			m_iTrysCounts = 0;

		}

		virtual ~CStPkDdGSPS(void)
		{			
		}


		//必须增加的函数
		static CStreamPackDecoder *Create( EnumStreamPackingType eSrcPktType )
		{
			return new CStPkDdGSPS();
		}



		virtual EnumErrno Decode( CFrameCache *pFrame, BOOL bExistGSFH  ) 
		{
			if( m_bInvalid )
			{
				return eERRNO_SYS_ECODEID;
			}
			UINT iOffsetHeader = 0;
			CGSPBuffer &csBuf = pFrame->GetBuffer();
			if( bExistGSFH )
			{
				iOffsetHeader = sizeof(StruGSFrameHeader);
			}

			
			if( csBuf.m_iDataSize<=(iOffsetHeader+4) )
			{
				GS_ASSERT(0);				
				return eERRNO_SYS_EINVALID;
			}
			const BYTE *p = csBuf.m_bBuffer+iOffsetHeader;
			UINT iSize = csBuf.m_iDataSize-iOffsetHeader;
			EnumErrno eRet = Parser(p, iSize);			
			return eRet;
		}
	private :

		EnumErrno InitStreamInfo( const StruGSPSHeader stGsH ) 
		{
			EnumGSCodeID eGsCodeId = CMediaInfo::GetGsCodeId4GSPSStreamType(stGsH.iCodeType);
			if( eGsCodeId==GS_CODEID_NONE)
			{
				GS_ASSERT(0);
				m_bInvalid = TRUE;
				return eERRNO_SYS_ECODEID;
			}
			EnumGSMediaType eMType = CodeID2MediaType(eGsCodeId);
			if( eMType == GS_MEDIA_TYPE_VIDEO )
			{
				SetMediaCodeId(GS_MEDIA_TYPE_VIDEO, eGsCodeId);
				m_iVideoChnNo = GetMediaChannel(GS_MEDIA_TYPE_VIDEO);
				m_iVideoGSStreamType = stGsH.iCodeType;
			}
			else if( eMType == GS_MEDIA_TYPE_AUDIO )
			{
				SetMediaCodeId(GS_MEDIA_TYPE_AUDIO, eGsCodeId);
				m_iAudioChnNo = GetMediaChannel(GS_MEDIA_TYPE_AUDIO);
				m_iAudioGSStreamType = stGsH.iCodeType;
			}
			else
			{				
				m_iTrysCounts++;
				if( m_iVideoGSStreamType == -1 )
				{
					if(  m_iTrysCounts>100 )
					{

						m_bInvalid = TRUE;
					}
				} 				
				GS_ASSERT(0);
				return eERRNO_SRC_EUNUSED;
			}
			return eERRNO_SUCCESS;
		}

		EnumErrno Parser(const BYTE *p, UINT iSize )
		{
			StruGSPSHeader stGsH;
			GS_ASSERT_RET_VAL(iSize>=sizeof(stGsH), eERRNO_SYS_EINVALID);
			memcpy( &stGsH,p, sizeof(stGsH) );

			if( !IS_GSPSHEADER(stGsH) )
			{
				GS_ASSERT(0);
				return eERRNO_SYS_ECODEID;
			}
			StruBaseBuf vTemp;
			vTemp.iSize = iSize-sizeof(stGsH);
			vTemp.pBuffer = (void*)(p+sizeof(stGsH));
			StruFrameInfo stFrameInfo;
			bzero(&stFrameInfo, sizeof(stFrameInfo));
			while(1)
			{
				if( m_iVideoGSStreamType == stGsH.iCodeType )
				{
					stFrameInfo.eMediaType = GS_MEDIA_TYPE_VIDEO;
					stFrameInfo.bKey = stGsH.bKey;
					stFrameInfo.bSysHeader = FALSE;
					stFrameInfo.iChnNo = m_iVideoChnNo;
					stFrameInfo.iTimestamp = stGsH.iTimestamp;				
					break;
				} 
				else if( m_iVideoGSStreamType == stGsH.iCodeType )
				{
					stFrameInfo.eMediaType = GS_MEDIA_TYPE_AUDIO;
					stFrameInfo.bKey = TRUE;
					stFrameInfo.bSysHeader = FALSE;
					stFrameInfo.iChnNo = m_iAudioChnNo;
					stFrameInfo.iTimestamp = stGsH.iTimestamp;
					break;
				}
				else if( m_iVideoGSStreamType == -1 )
				{
					if( InitStreamInfo(stGsH) )
					{
						return eERRNO_SUCCESS;
					}
				}
				else if( m_iAudioGSStreamType ==-1 )
				{
					if(  m_iVideoGSStreamType != -1 )
					{
						if( InitStreamInfo(stGsH) )
						{
							return eERRNO_SUCCESS;
						}
					}
					else
					{
						return eERRNO_SUCCESS;
					}
				}
				else 
				{
					//码流类型已经改变
					GS_ASSERT(0);
					m_bInvalid = TRUE;
					return eERRNO_SYS_ECODEID;

				}
			}

			CFrameCache *pFrame = pFrame->Create(&vTemp, 1);
			if( !pFrame )
			{
				GS_ASSERT(0);
				return eERRNO_SYS_ENMEM;
			}
			memcpy(&pFrame->m_stFrameInfo, &stFrameInfo, sizeof(stFrameInfo));
			if( m_listFrameCache.AddTail(pFrame))
			{
				GS_ASSERT(0);
				pFrame->UnrefObject();
				return eERRNO_SYS_ENMEM;
			}
			return eERRNO_SUCCESS;
		}

	};


/*
*********************************************************************
*
*@brief : 标准 PS 28181 == > 标准
*
*********************************************************************
*/
class CStPkDdStand28181Ps  : public CStPkDdBase
{
private :
	StruIDPair m_vID[GSP_MAX_MEDIA_CHANNELS];
	CVectorPSSlice m_vCurVideo;
	CVectorPSSlice m_vCurAudio;
	CList m_listExist;
	StruPSStreamInfo m_stAStreamType;
	StruPSStreamInfo m_stVStreamType;

	StruFrameInfo m_stVFrameInfo;
	StruFrameInfo m_stAFrameInfo;
public :
	CStPkDdStand28181Ps(void) : CStPkDdBase(eSTREAM_PKG_28181PS)
	{
		bzero(m_vID, sizeof(m_vID));			


		bzero(&m_stAStreamType, sizeof(m_stAStreamType));
		m_stAStreamType.iStreamId  = -1;
		m_stAStreamType.iStreamType = -1;

		bzero(&m_stVStreamType, sizeof(m_stVStreamType));
		m_stVStreamType.iStreamId  = -1;
		m_stVStreamType.iStreamType = -1;
	}

	virtual ~CStPkDdStand28181Ps(void)
	{
		m_vCurVideo.clear();
		m_vCurAudio.clear();
	}


	//必须增加的函数
	static CStreamPackDecoder *Create( EnumStreamPackingType eSrcPktType )
	{
		return new CStPkDdStand28181Ps();
	}




	virtual EnumErrno Decode( CFrameCache *pFrame, BOOL bExistGSFH  ) 
	{
		CVectorPSSlice vSlice;

		EnumErrno eRet = CPSSliceParser::Slice(pFrame, bExistGSFH, vSlice);		
		GS_ASSERT_RET_VAL(!eRet, eRet);
		UINT iNums = vSlice.size();
		CPSSlice  *pSlice;
		INT iPSHType;
		const CPSSlice::StruPESBody *pPES;
		UINT64 iTs = 0;
		BOOL bIFrame = FALSE;
		for( UINT i = 0; i<iNums; i++ )
		{
			pSlice = vSlice[i];
			iPSHType = pSlice->GetPSHeaderType();
			if( iPSHType == PS_HEADER_PES )
			{
				//获取数据片
				pPES = &pSlice->GetPESBody();
				CProFrameCache *ppCache = NULL;
				if( pPES->iPSStreamID == m_stVStreamType.iStreamId )
				{
					//视频
					if( m_vCurVideo.size() == 0 )
					{
						m_stVFrameInfo.iTimestamp = iTs;	
						m_stVFrameInfo.bKey = bIFrame;
						m_stVFrameInfo.eMediaType = GS_MEDIA_TYPE_VIDEO;
						m_stVFrameInfo.iChnNo = GetMediaChannel(GS_MEDIA_TYPE_VIDEO);
					}
					m_vCurVideo.push_back(pSlice);
					
				}
				else if( pPES->iPSStreamID ==m_stAStreamType.iStreamId )
				{
					//音频
					if( m_vCurAudio.size() == 0 )
					{
						m_stAFrameInfo.iTimestamp = iTs;	
						m_stAFrameInfo.bKey = bIFrame;
						m_stAFrameInfo.eMediaType = GS_MEDIA_TYPE_AUDIO;
						m_stAFrameInfo.iChnNo = GetMediaChannel(GS_MEDIA_TYPE_AUDIO);
					}
					m_vCurAudio.push_back(pSlice);
				}
				else
				{
					continue;
				}
			}
			else if( iPSHType == PS_HEADER_PK_START )
			{
				//新的一帧开始
				FlushCacheToList();				
			}
			else if( iPSHType == PS_HEADER_SMH )
			{
				//系统头
				bIFrame = TRUE;
				//不使用, 跳过				
			}
			else if( iPSHType == PS_HEADER_PSM )
			{
				bIFrame = TRUE;	
				if( AnalzePSM(pSlice ) )
				{
					GS_ASSERT(0);
					Reset();
					return eERRNO_SYS_ECODEID;
				}
				
			}
		} // end for
		//FlushCacheToList();		//// ???
		return  eERRNO_SUCCESS;
	}

private :
	EnumErrno AnalzePSM( CPSSlice *pSlice  )
	{
		std::vector<StruPSStreamInfo> vStreamInfo;
		//信息头
		//获取流的类型
		if( pSlice->ParserPSM(vStreamInfo) )
		{
			GS_ASSERT(0);				
			return eERRNO_SYS_EPRO;
		}
		GS_ASSERT(!vStreamInfo.empty());				

		StruPSStreamInfo stTypeVideo;
		StruPSStreamInfo stTypeAudio;
		bzero(&stTypeVideo, sizeof(stTypeVideo));
		bzero(&stTypeAudio, sizeof(stTypeAudio));
		EnumGSCodeID eVID = GS_CODEID_NONE, eAID = GS_CODEID_NONE;
		for( UINT i = 0; i<vStreamInfo.size(); i++ )
		{
			StruPSStreamInfo stType = vStreamInfo[i];

			if( stType.iStreamType == stream_type_h264 )
			{
				eVID = GS_CODEID_ST_H264;
				stTypeVideo = stType;
			}
			else if( stType.iStreamType  == stream_type_mpeg4 )
			{
				eVID = GS_CODEID_ST_MP4;		
				stTypeVideo = stType;

			}
			else if(  (stType.iStreamType &0xf0) == 0x90 )
			{

				switch( stType.iStreamType )
				{
				case  stream_type_audio_g711:
					eAID = GS_CODEID_AUDIO_ST_G711A;	
					stTypeAudio = stType;		
					break;
				case  stream_type_audio_g722:
					eAID =  GS_CODEID_AUDIO_ST_G711A;	
					stTypeAudio = stType;		
					break;
				case  stream_type_audio_g723:
					eAID = GS_CODEID_AUDIO_ST_G723;	
					stTypeAudio = stType;	
					break;
				case  stream_type_audio_g729:
					eAID = GS_CODEID_AUDIO_ST_G729;		
					stTypeAudio = stType;	
					break;
				default :
					{

					}
					break;
				}						
			} 
		} // end for

		if( stTypeVideo.iStreamType==0 && stTypeAudio.iStreamType==0 )
		{
			//未知到的编码
			GS_ASSERT(0);			
			return eERRNO_SYS_EPRO;
		}
		else
		{
			if(  stTypeVideo.iStreamType )
			{
				if(	m_stVStreamType.iStreamType==-1 ) 						
				{
					m_stVStreamType = stTypeVideo;								
					if( eVID==GS_CODEID_NONE )
					{
						return eERRNO_SYS_ECODEID;
					}
					SetMediaCodeId(GS_MEDIA_TYPE_VIDEO, eVID);
				}
				else if( m_stVStreamType.iStreamType != stTypeVideo.iStreamType )
				{
					//编码已经改变
					GS_ASSERT(0);				
					return eERRNO_SYS_ECODEID;
				}
			}

			if( stTypeAudio.iStreamType )
			{
				if(	m_stAStreamType.iStreamType ==-1 )
				{
					m_stAStreamType = stTypeAudio;								
					if( eVID==GS_CODEID_NONE )
					{
						//不使用音频????
						//return eERRNO_SYS_ECODEID;
					}
					else
					{
						SetMediaCodeId(GS_MEDIA_TYPE_AUDIO, eAID);
					}
				}
				else if	(m_stAStreamType.iStreamType != stTypeAudio.iStreamType )
				{
					//编码已经改变
					GS_ASSERT(0);				
					return eERRNO_SYS_ECODEID;
				}
			}
		}
		return eERRNO_SUCCESS;
	}

	void Reset(void)
	{
		m_vCurAudio.clear();
		m_vCurVideo.clear();
	}

	CFrameCache *CreateFrame( CVectorPSSlice &vSlice )
	{
		UINT i;
		UINT iNums = vSlice.size();
		UINT iTotals = 0;
		for(  i = 0; i<iNums; i++ )
		{
			iTotals += vSlice[i]->GetPESBody().iBodySize;
		}
		if( 0== iTotals )
		{
			GS_ASSERT(0);
			return NULL;
		}

		CFrameCache *pTemp = pTemp->Create( iTotals );
		GS_ASSERT_RET_VAL(pTemp, NULL);

		CGSPBuffer &csBuf = pTemp->GetBuffer();
		const CPSSlice::StruPESBody *pPes;

		for(  i = 0; i<iNums; i++ )
		{
			pPes = &vSlice[i]->GetPESBody();
			GS_ASSERT(pPes->iBodySize);
			csBuf.AppendData( pPes->pBody, pPes->iBodySize);
		}
		return pTemp;
	}

	void FlushCacheToList(void)
	{
		//新的一帧	
		if( !m_vCurVideo.empty())
		{
			//有数据 
			CFrameCache *pTemp = CreateFrame(m_vCurVideo);
			if( pTemp )
			{
				memcpy( &pTemp->m_stFrameInfo, &m_stVFrameInfo, sizeof(m_stVFrameInfo));
				if(m_listFrameCache.AddTail(pTemp) )
				{
					GS_ASSERT(0);	
					SAFE_DESTROY_REFOBJECT(&pTemp);
				}
			}
			m_vCurVideo.clear();
			
		}
			

		if( !m_vCurAudio.empty() )
		{
			//有数据 
			CFrameCache *pTemp = CreateFrame(m_vCurAudio);
			if( pTemp )
			{
				memcpy( &pTemp->m_stFrameInfo, &m_stAFrameInfo, sizeof(m_stAFrameInfo));
				if( m_listFrameCache.AddTail(pTemp) )
				{
					GS_ASSERT(0);	
					SAFE_DESTROY_REFOBJECT(&pTemp);
				}
			}
			m_vCurAudio.clear();
		}
	}


};

/*
*********************************************************************
*
*@brief : 动态分析 == > 标准
*
*********************************************************************
*/
class CStPkDdStandNormal  : public CStPkDdBase
{
private :
	CES2PS m_csEs2Ps;
	EnumGSCodeID m_vGsCodeId[2];	
	int m_vStreamType[2];
	int m_iGeuessCounts;
	BOOL m_bValid;
public :

	CStPkDdStandNormal(EnumStreamPackingType eSrcPktType) : CStPkDdBase(eSrcPktType)
	{
		m_vGsCodeId[0] = GS_CODEID_NONE;
		m_vGsCodeId[1] = GS_CODEID_NONE;

		m_vStreamType[0] = 0;
		m_vStreamType[1] = 0;



		m_iGeuessCounts =  200;
		m_bValid = TRUE;
	}
	virtual ~CStPkDdStandNormal(void)
	{

	}


	//必须增加的函数
	static CStreamPackDecoder *Create( EnumStreamPackingType eSrcPktType )
	{
		return new CStPkDdStandNormal(eSrcPktType);	
	}


	virtual EnumErrno Decode( CFrameCache *pFrame, BOOL bExistGSFH ) 
	{
		UINT iOffsetHeader = 0;
		CGSPBuffer &csBuf = pFrame->GetBuffer();
		if( bExistGSFH )
		{
			iOffsetHeader = sizeof(StruGSFrameHeader);
		}	
		if( csBuf.m_iDataSize<=(iOffsetHeader+4) )
		{
			GS_ASSERT(0);
			
			return eERRNO_SYS_EINVALID;
		}
		const BYTE *p = csBuf.m_bBuffer+iOffsetHeader;
		UINT iSize = csBuf.m_iDataSize-iOffsetHeader;
		EnumErrno eRet = Analyze(p, iSize, pFrame->m_stFrameInfo, &csBuf );
		
		return eRet;
	}
	
protected :
	EnumErrno Analyze( const BYTE *p, UINT iSize,const StruFrameInfo &stSrcFrmInfo
		,CGSPBuffer *pRefBuf )
	{
		if( !m_bValid )
		{
			return eERRNO_SYS_ECODEID;
		}
		if( stSrcFrmInfo.eMediaType != GS_MEDIA_TYPE_VIDEO )
		{
			return eERRNO_SUCCESS;
		}

		//测试码流类型
		if( m_vGsCodeId[0]== GS_CODEID_NONE )
		{
			int iType = m_csEs2Ps.TestStreamType(p, iSize);
			m_iGeuessCounts--;
			switch(iType)
			{
			case stream_type_h264 :
				m_vStreamType[0] = stream_type_h264;
				m_vGsCodeId[0] = GS_CODEID_ST_H264;

				break;
			case stream_type_mpeg4 :
				m_vStreamType[0] = stream_type_mpeg4; 
				m_vGsCodeId[0] = GS_CODEID_ST_MP4;
				break;
			default :
				{
					if( m_iGeuessCounts == 0 )
					{
						//不再检查
						m_bValid = FALSE;
					}					
					return eERRNO_SUCCESS;
				}
				break;
			}
			m_csEs2Ps.SetConvertParam(25,iType);

			UINT iChn = GetMediaChannel(GS_MEDIA_TYPE_VIDEO);
			if( iChn >=GSP_MAX_MEDIA_CHANNELS )
			{
				GS_ASSERT(0);				
				return eERRNO_SUCCESS;
			}	
			SetMediaCodeId(GS_MEDIA_TYPE_VIDEO, m_vGsCodeId[0]);
		}

		if( m_vStreamType[0] != stream_type_none )
		{
			const BYTE *pp = p;
			BOOL bRet = m_csEs2Ps.IsTestStreamType(p, iSize,m_vStreamType[0] , &pp );
			if( p != pp )
			{
				iSize -= (pp-p);
				p = pp;
			}
			if( iSize>0 )
			{
				CFrameCache *pRet = CFrameCache::Create(pRefBuf, p, iSize);
				if( pRet )
				{
					memcpy( &pRet->m_stFrameInfo, 
						&stSrcFrmInfo, sizeof(pRet->m_stFrameInfo));
					if( m_listFrameCache.AddTail(pRet) )
					{
						GS_ASSERT(0);						
						SAFE_DESTROY_REFOBJECT(&pRet);
						return eERRNO_SYS_ENMEM;
					}
				}
				else
				{
					GS_ASSERT(0);					
					return eERRNO_SYS_ENMEM;
				}
			}
		}


		return eERRNO_SUCCESS;
	}

	

};








} //end namespace GSP



/*
*********************************************************************
*
*@brief : CStreamPackDecoder::Create(EnumStreamPackingType eSrcPktType,
EnumStreamPackingType eDestPktType)
*
*********************************************************************
*/
namespace GSP
{

typedef struct _StruRegPkDdIf
{
	EnumStreamPackingType eSupportSrcPktType;
	CStreamPackDecoder* (*Create)(EnumStreamPackingType eSrcPktType);
}StruRegPkDdIf;

#define IMP(e, classname )  {(EnumStreamPackingType)e, (CStreamPackDecoder*(*)(EnumStreamPackingType)) classname::Create}

static StruRegPkDdIf _vExistReg[] =
{
	IMP(eSTREAM_PKG_28181PS, CStPkDdStand28181Ps ),
	IMP(eSTREAM_PKG_GSPS, CStPkDdGSPS ),
	IMP(eSTREAM_PKG_GS461C, CStPkDdStandNormal ),
	IMP(eSTREAM_PKG_GS2160I, CStPkDdStandNormal ),
	IMP(eSTREAM_PKG_GS2160IV, CStPkDdStandNormal ),
	IMP(eSTREAM_PKG_Hi,  CStPkDdStandNormal ),
	IMP(eSTREAM_PKG_HengYi, CStPkDdStandNormal ),
	IMP(eSTREAM_PKG_ZBeng, CStPkDdStandNormal ),

	{eSTREAM_PKG_DaHua, NULL },
	{eSTREAM_PKG_HiKVS, NULL },
	{eSTREAM_PKG_NONE, NULL },
	{eSTREAM_PKG_NONE, NULL },
	{eSTREAM_PKG_NONE, NULL },
	{eSTREAM_PKG_NONE, NULL },
	{eSTREAM_PKG_NONE, NULL },
	{eSTREAM_PKG_NONE, NULL },
};

} //end namespace GSP

CStreamPackDecoder *CStreamPackDecoder::Make(EnumStreamPackingType eSrcPktType)
{


	for( UINT i=0; i<ARRARY_SIZE(_vExistReg); i++ )
	{
		if(_vExistReg[i].Create!=NULL &&  _vExistReg[i].eSupportSrcPktType == eSrcPktType )
		{
			return _vExistReg[i].Create(eSrcPktType);
		}
	}
	GS_ASSERT(0);
	return NULL;
}

void CStreamPackDecoder::ResigterExtDecoder(EnumStreamPackingType ePktType, 
											CStreamPackDecoder*(*CreateFunc)(EnumStreamPackingType ePktType) )
{
	
	for( UINT i=0; i<ARRARY_SIZE(_vExistReg); i++ )
	{
		if( _vExistReg[i].eSupportSrcPktType==ePktType )
		{
			if( _vExistReg[i].Create==NULL )
			{
				_vExistReg[i].Create =CreateFunc;
			}
			return;
		}
	}

	for( UINT i=0; i<ARRARY_SIZE(_vExistReg); i++ )
	{
		if( _vExistReg[i].eSupportSrcPktType==ePktType &&  _vExistReg[i].Create==NULL )
		{
			_vExistReg[i].Create =CreateFunc;			
			return;
		}
	}
	GS_ASSERT(0);
}