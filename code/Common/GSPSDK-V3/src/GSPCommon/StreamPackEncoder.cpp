#include "StreamPackEncoder.h"
#include "List.h"
#include "SIP/ES2PS.h"
#include "RTP/RtpStru.h"
#include <time.h>


using namespace GSP;
using namespace GSP::SIP;
using namespace GSP::RTP;



namespace GSP
{

	static UINT32 MakeTime(UINT uiYear, UINT uiMonth, UINT uiDay, UINT uiHour = 0, 
		UINT uiMinute = 0, UINT uiSecond = 0)
	{
		struct tm t; 
		t.tm_year = uiYear - 1900;
		t.tm_mon = uiMonth - 1; 
		t.tm_mday = uiDay; 
		t.tm_hour = uiHour; 
		t.tm_min = uiMinute; 
		t.tm_sec = uiSecond; 
		t.tm_isdst = 0; 
		return (UINT32) ::mktime(&t);
	}

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

	class CStPkEdBase : public CStreamPackEncoder
	{
	protected :
		CList m_listFrameCache; // 存储 CFrameCache * 分析完的结果
		typedef struct _StruInfoCtx
		{
			EnumGSMediaType eMType;
			EnumGSCodeID eCodeId;
			UINT iChnNo;
			float fFrameRate;
		}StruInfoCtx;

		StruInfoCtx m_vInfoCtx[GSP_MAX_MEDIA_CHANNELS];

		EnumStreamPackingType m_eDestPktType;
		BOOL m_bInsertGSFH;
	public :

		virtual EnumErrno Init(BOOL bInsertGSFH)
		{
			m_bInsertGSFH = bInsertGSFH ? TRUE : FALSE;
			return eERRNO_SUCCESS;
		}		
		virtual CFrameCache *Get(void)
		{
			void *p = NULL;
			m_listFrameCache.RemoveFront(&p);
			return (CFrameCache*)p;
		}

		virtual void BindChannelInfo( UINT iChnNo, EnumGSMediaType eMType,
			EnumGSCodeID eGSCodeId, float fFrameRate)
		{
			GS_ASSERT(iChnNo<GSP_MAX_MEDIA_CHANNELS );
			if( iChnNo<GSP_MAX_MEDIA_CHANNELS )
			{

				m_vInfoCtx[iChnNo].eMType = eMType;
				m_vInfoCtx[iChnNo].eCodeId = eGSCodeId;
				if( fFrameRate>0 )
				{
					m_vInfoCtx[iChnNo].fFrameRate = fFrameRate;
				}
			}

		}

		virtual void BindChannelInfo( const CMediaInfo &csMediaInfo )
		{
			UINT iCnts = csMediaInfo.GetChannelNums();
			for( UINT16 i=0; i<iCnts; i++ )
			{
				const CIMediaInfo::StruMediaChannelInfo *p = csMediaInfo.GetChannel(i);
				if( p )
				{
					BindChannelInfo(p->iSubChannel,
						(EnumGSMediaType) p->stDescri.eMediaType,
						(EnumGSCodeID)p->stDescri.unDescri.struVideo.eCodeID,
						p->stDescri.unDescri.struVideo.iFrameRate);
				}
			}

		}
		virtual EnumGSCodeID GetCodeId( UINT iChnNo )
		{
			GS_ASSERT(iChnNo<GSP_MAX_MEDIA_CHANNELS );
			if( iChnNo<GSP_MAX_MEDIA_CHANNELS )
			{
				return m_vInfoCtx[iChnNo].eCodeId;
			}
			return GS_CODEID_NONE;
		}

		virtual EnumGSMediaType GetMeidaType(UINT iChnNo )
		{
			GS_ASSERT(iChnNo<GSP_MAX_MEDIA_CHANNELS );
			if( iChnNo<GSP_MAX_MEDIA_CHANNELS )
			{
				return m_vInfoCtx[iChnNo].eMType;
			}
			return GS_MEDIA_TYPE_NONE;
		}

	protected :
		CStPkEdBase(EnumStreamPackingType eDestPktType) : CStreamPackEncoder()
		{
			m_eDestPktType = eDestPktType;
			m_listFrameCache.SetFreeCallback((FuncPtrFree)_FreeListMember);
			bzero(m_vInfoCtx, sizeof(m_vInfoCtx));
			for(UINT i=0; i<GSP_MAX_MEDIA_CHANNELS; i++ )
			{
				m_vInfoCtx[i].iChnNo = i;
				m_vInfoCtx[i].fFrameRate = 25;
			}
			m_bInsertGSFH = FALSE;

		}
		virtual ~CStPkEdBase(void)
		{
			m_listFrameCache.Clear();
		}

		UINT16 GetMediaChannel(EnumGSMediaType eMType )
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
	};


	/*
	*********************************************************************
	*
	*@brief : 标准流  => 标准
	*
	*********************************************************************
	*/
	class CStPkEdStandStream  : public CStPkEdBase
	{
	public :
		CStPkEdStandStream(void) : CStPkEdBase(eSTREAM_PKG_Standard)
		{
			
		}

		virtual ~CStPkEdStandStream(void)
		{			
		}


		//必须增加的函数
		static CStreamPackEncoder *Create( EnumStreamPackingType eDestPktType )
		{
			return new CStPkEdStandStream();
		}


		virtual EnumErrno Encode( CFrameCache *pFrame, BOOL bExistGSFH ) 
		{
			if( pFrame->m_stFrameInfo.iChnNo>=GSP_MAX_MEDIA_CHANNELS ||
				!m_vInfoCtx[pFrame->m_stFrameInfo.iChnNo].eCodeId )
			{
				GS_ASSERT(0);
				return eERRNO_SYS_EINVALID;
			}

			if( bExistGSFH )
			{
				bExistGSFH = TRUE;
			}
			else
			{
				bExistGSFH = FALSE;
			}

			if( m_bInsertGSFH == bExistGSFH )
			{
				pFrame->RefObject();
				if( m_listFrameCache.AddTail(pFrame) )
				{
					pFrame->UnrefObject();
					return eERRNO_SYS_ENMEM;
				}
				return eERRNO_SUCCESS;
			}

			UINT iOffsetHeader = 0;
			CGSPBuffer &csBuf = pFrame->GetBuffer();
			const BYTE *p = csBuf.m_bBuffer;
			INT iSize = csBuf.m_iDataSize;
			CFrameCache *pNewFrame = NULL;
			if( bExistGSFH )
			{
				
				

				iSize -= sizeof(StruGSFrameHeader);
				p += sizeof(StruGSFrameHeader);
				//去掉头
				StruBaseBuf vBuf[1];		
				vBuf[0].iSize = iSize;
				vBuf[0].pBuffer = (void*) p;
				pNewFrame = pNewFrame->Create(vBuf, 1);
				if( !pNewFrame )
				{
					GS_ASSERT(0);										
					return eERRNO_SYS_ENMEM;
				}
				memcpy( &pNewFrame->m_stFrameInfo, &pFrame->m_stFrameInfo,
						sizeof(pFrame->m_stFrameInfo) );
				if( m_listFrameCache.AddTail(pNewFrame) )
				{
					//加到缓冲队列
					GS_ASSERT(0);					
					SAFE_DESTROY_REFOBJECT(&pNewFrame);		
					return eERRNO_SYS_ENMEM;
				}
			}
			else
			{
				//增加头
				StruGSFrameHeader stGSFrameH;
				StruBaseBuf vBuf[2];	
				bzero(&stGSFrameH, sizeof(stGSFrameH));
				stGSFrameH.eMediaType = m_vInfoCtx[pFrame->m_stFrameInfo.iChnNo].eMType;
				stGSFrameH.bKey = stGSFrameH.bKey;
				stGSFrameH.iMagic = GS_FRAME_HEADER_MAGIC;
				stGSFrameH.iLenght = iSize;
				stGSFrameH.iTimeStamp = stGSFrameH.iTimeStamp;
				vBuf[0].iSize= sizeof(stGSFrameH);
				vBuf[0].pBuffer =  &stGSFrameH;
				vBuf[1].iSize = iSize;
				vBuf[1].pBuffer = (void*) p;
				pNewFrame = pNewFrame->Create(vBuf, 2);
				if( !pNewFrame )
				{
					GS_ASSERT(0);										
					return eERRNO_SYS_ENMEM;
				}
				memcpy( &pNewFrame->m_stFrameInfo, &pFrame->m_stFrameInfo,
					sizeof(pFrame->m_stFrameInfo) );
				if( m_listFrameCache.AddTail(pNewFrame) )
				{
					//加到缓冲队列
					GS_ASSERT(0);					
					SAFE_DESTROY_REFOBJECT(&pNewFrame);		
					return eERRNO_SYS_ENMEM;
				}
			}
			return eERRNO_SUCCESS;
		}

	};


	/*
	*********************************************************************
	*
	*@brief : 标准流  => 标准 PS
	*
	*********************************************************************
	*/
	class CStPkEdStand28181Ps  : public CStPkEdBase
	{
	private :		
		CES2PS m_csEs2Ps;
		INT m_iVideoSipStreamType;
		INT m_iAudioSipStreamType;
		BYTE m_bPesBuffer[1024];
	public :
		CStPkEdStand28181Ps(void) : CStPkEdBase(eSTREAM_PKG_28181PS)
		{
			m_iVideoSipStreamType = stream_type_none;
			m_iAudioSipStreamType = stream_type_none;
		}

		virtual ~CStPkEdStand28181Ps(void)
		{			
		}


		//必须增加的函数
		static CStreamPackEncoder *Create( EnumStreamPackingType eDestPktType )
		{
			return new CStPkEdStand28181Ps();
		}


		virtual EnumErrno Encode( CFrameCache *pFrame, BOOL bExistGSFH ) 
		{
			if( pFrame->m_stFrameInfo.iChnNo>=GSP_MAX_MEDIA_CHANNELS ||
				!m_vInfoCtx[pFrame->m_stFrameInfo.iChnNo].eCodeId )
			{
				GS_ASSERT(0);
				return eERRNO_SYS_EINVALID;
			}

			UINT iOffsetHeader = 0;
			CGSPBuffer &csBuf = pFrame->GetBuffer();
			const BYTE *p = csBuf.m_bBuffer;
			INT iSize = csBuf.m_iDataSize;

			if( bExistGSFH )
			{
				iSize -= sizeof(StruGSFrameHeader);
				p += sizeof(StruGSFrameHeader);
			}

			
			if( iSize<1 )
			{
				GS_ASSERT(0);
				return eERRNO_SYS_EINVALID;
			}
		


			EnumErrno eRet = PacketPS(p, iSize, pFrame->m_stFrameInfo);
		
			return eRet;
		}


	private :
		int GetPSStreamType( UINT iChnNo )
		{
			switch( m_vInfoCtx[iChnNo].eCodeId )
			{
			case GS_CODEID_ST_MP4 :
				return stream_type_mpeg4;
			break;
			case GS_CODEID_ST_H264 :
				return stream_type_h264;
			break;
			case GS_CODEID_ST_SVAC :
				return stream_type_svac;
			case GS_CODEID_AUDIO_ST_G711A :
			case GS_CODEID_AUDIO_ST_G711U :
				return stream_type_audio_g711;
				break;
			case GS_CODEID_AUDIO_ST_G722 :
				return stream_type_audio_g722;
				break;
			case GS_CODEID_AUDIO_ST_G723 :	
				return stream_type_audio_g723;
				break;
			case GS_CODEID_AUDIO_ST_G729  :
				return stream_type_audio_g729;
				break;
			case GS_CODEID_AUDIO_ST_SVAC  :
				return stream_type_audio_svac;
				break;
			default :
				{
					
				}
			break;
			}
			return stream_type_none;
		}

		EnumErrno PacketPS( const BYTE *pData, UINT iDataSize,
			const StruFrameInfo &stFrameInfo )
		{
			//每个 PES 包的大小
#define MAX_PES_SIZE   (1024*31)  

			int isVideo = TRUE;
			bool bAudio = false;
			UINT iChn = stFrameInfo.iChnNo;
			if( m_vInfoCtx[iChn].eMType == GS_MEDIA_TYPE_AUDIO )
			{
				isVideo = FALSE;
				bAudio = true;		
			}

			if( m_vInfoCtx[iChn].eMType == GS_MEDIA_TYPE_VIDEO )
			{
				if( m_iVideoSipStreamType == stream_type_none )
				{
					m_iVideoSipStreamType = GetPSStreamType(iChn);
					m_csEs2Ps.SetConvertParam( (UINT) m_vInfoCtx[iChn].fFrameRate,
						m_iVideoSipStreamType);	
				}				
			}
			else if( m_vInfoCtx[iChn].eMType == GS_MEDIA_TYPE_AUDIO)
			{
				if( m_iVideoSipStreamType == stream_type_none )
				{
					return eERRNO_SUCCESS;
				}

				if( m_iAudioSipStreamType == stream_type_none )
				{
					m_iAudioSipStreamType = GetPSStreamType(iChn);
					m_csEs2Ps.SetAudioType(m_iAudioSipStreamType);
				}
			}	
			else
			{
				GS_ASSERT(0);
				return eERRNO_SUCCESS;
			}
			GS_ASSERT(m_iVideoSipStreamType != stream_type_none);
			

			const BYTE *p = pData;
			INT iSize = iDataSize;
			INT iTemp;
			INT iPesHeaderLen;
		
			StruBaseBuf vBuf[5];
			StruGSFrameHeader stGSFrameH;
			CProFrameCache *pCache = NULL;

			pCache = pCache->Create();
			GS_ASSERT_RET_VAL(pCache,eERRNO_SYS_ENMEM );


			bzero(&stGSFrameH, sizeof(stGSFrameH));
			stGSFrameH.iLenght = 0;
			

			while( iSize>0 )
			{
				iTemp = MIN( iSize, MAX_PES_SIZE);
				if( isVideo )
				{
					isVideo = 0;			
					iPesHeaderLen = m_csEs2Ps.ESConvertToPs(&p,&iTemp, iTemp,m_bPesBuffer, 
						sizeof(m_bPesBuffer) );				
				}
				else
				{
					iPesHeaderLen = m_csEs2Ps.ESConvertMakePES(iTemp,m_bPesBuffer, 
						sizeof(m_bPesBuffer) , bAudio );
				}
				


				if( iPesHeaderLen< 1 )
				{
					GS_ASSERT(0);	
					SAFE_DESTROY_REFOBJECT(&pCache);	
					return eERRNO_SYS_EINVALID;		
				}
				int n = 0;

				stGSFrameH.iLenght += (iTemp+iPesHeaderLen);

				
				
				vBuf[0].iSize= iPesHeaderLen;
				vBuf[0].pBuffer =  m_bPesBuffer;
				vBuf[1].iSize = iTemp;
				vBuf[1].pBuffer = (void*) p;

				if( pCache->AddBack(vBuf, 2) )
				{
					GS_ASSERT(0);					
					SAFE_DESTROY_REFOBJECT(&pCache);		
					return eERRNO_SYS_ENMEM;
				}		
				iSize -= iTemp;
				p += iTemp;
			} // end for

			if( m_bInsertGSFH )
			{
				stGSFrameH.eMediaType = m_vInfoCtx[iChn].eMType;
				stGSFrameH.bKey = stFrameInfo.bKey;
				stGSFrameH.iMagic = GS_FRAME_HEADER_MAGIC;
				stGSFrameH.iTimeStamp = stFrameInfo.iTimestamp;	
				if( pCache->AddFront((BYTE*)&stGSFrameH, sizeof(stGSFrameH) ) )
				{
					GS_ASSERT(0);					
					SAFE_DESTROY_REFOBJECT(&pCache);		
					return eERRNO_SYS_ENMEM;
				}
			}

			CFrameCache *pNewFrame = pNewFrame->Create(pCache);
			SAFE_DESTROY_REFOBJECT(&pCache);
			GS_ASSERT_RET_VAL(pNewFrame, eERRNO_SYS_ENMEM);

			memcpy( &pNewFrame->m_stFrameInfo, &stFrameInfo, sizeof(stFrameInfo) );
			if( m_listFrameCache.AddTail(pNewFrame) )
			{
				//加到缓冲队列
				GS_ASSERT(0);					
				SAFE_DESTROY_REFOBJECT(&pNewFrame);		
				return eERRNO_SYS_ENMEM;
			}			
			return eERRNO_SUCCESS;
		}
	};



	/*
	*********************************************************************
	*
	*@brief : GS PS 流封装
	*
	*********************************************************************
	*/

	class CStPkEdGosunPS  : public CStPkEdBase
	{
	private :		
		INT m_vGSPSStreamType[GSP_MAX_MEDIA_CHANNELS];
	public :
		CStPkEdGosunPS(void) : CStPkEdBase(eSTREAM_PKG_GSPS)
		{
			for( int i = 0; i<GSP_MAX_MEDIA_CHANNELS; i++ )
			{
				m_vGSPSStreamType[i] = -1;
			}
		}
		virtual ~CStPkEdGosunPS(void)
		{			
		}

		UINT8 GetGSPSStreamType(UINT iChn )
		{
			if( iChn<GSP_MAX_MEDIA_CHANNELS )
			{
				if( m_vGSPSStreamType[iChn]!=-1 )
				{
					return m_vGSPSStreamType[iChn];
				}
				m_vGSPSStreamType[iChn] = CMediaInfo::GetGSPSStreamType4GsCodeId(m_vInfoCtx[iChn].eCodeId);
				GS_ASSERT(m_vGSPSStreamType[iChn]);
				return m_vGSPSStreamType[iChn];
			}
			GS_ASSERT(0);
			return GSPS_CODETYPE_NONE;
		}


		//必须增加的函数
		static CStreamPackEncoder *Create( EnumStreamPackingType eDestPktType )
		{
			return new CStPkEdGosunPS();
		}


		virtual EnumErrno Encode( CFrameCache *pFrame, BOOL bExistGSFH ) 
		{

			StruGSPSHeader stGsH;
 			GSPSHEADER_INIT(stGsH,
				GetGSPSStreamType(pFrame->m_stFrameInfo.iChnNo),
 				pFrame->m_stFrameInfo.bKey);


			StruBaseBuf vTmp[3];			
			CGSPBuffer *pBuf = &pFrame->GetBuffer();


			const BYTE *pPlayload = pBuf->m_bBuffer;
			INT iSize = pBuf->m_iDataSize;
			StruGSFrameHeader stGSFrameH;
			bzero(&stGSFrameH, sizeof(stGSFrameH));

			if( bExistGSFH )
			{
				iSize -= sizeof(stGSFrameH);				
				if( iSize<1 )
				{
					return eERRNO_SYS_ECODEID;
				}
				memcpy( &stGSFrameH, pPlayload, sizeof(stGSFrameH));
				pPlayload += sizeof(stGSFrameH);
			}

			int n = 0;			
			if( m_bInsertGSFH )
			{				
				stGSFrameH.eMediaType = pFrame->m_stFrameInfo.eMediaType;				
				stGSFrameH.iMagic = GS_FRAME_HEADER_MAGIC;
				stGSFrameH.iLenght = sizeof(stGsH)+iSize;
				if( 0==stGSFrameH.iTimeStamp )
				{
					stGSFrameH.bKey = pFrame->m_stFrameInfo.bKey;
					stGSFrameH.iTimeStamp = (UINT32) pFrame->m_stFrameInfo.iTimestamp;
				}
				vTmp[n].iSize= sizeof(stGSFrameH);
				vTmp[n].pBuffer =  &stGSFrameH;
				n++;
			}
	
			vTmp[n].iSize = sizeof(stGsH);
			vTmp[n].pBuffer = &stGsH;
			n++;

			vTmp[n].iSize = iSize;
			vTmp[n].pBuffer = (void*) pPlayload;
			n++;

			CFrameCache *pNewFrame = pFrame->Create(vTmp, n);
			memcpy( &pNewFrame->m_stFrameInfo, &pFrame->m_stFrameInfo, sizeof(pFrame->m_stFrameInfo));
			if( !pNewFrame)
			{
				GS_ASSERT(0);
				return eERRNO_SYS_ENMEM;
			}
			if( m_listFrameCache.AddTail(pNewFrame) )
			{
				GS_ASSERT(0);
				pNewFrame->UnrefObject();
				return eERRNO_SYS_ENMEM;
			}
			return eERRNO_SUCCESS;

		}
	};


} //end namespace GSP



/*
*********************************************************************
*
*@brief : CStreamPackEncoder
*
*********************************************************************
*/


CStreamPackEncoder *CStreamPackEncoder::Make( EnumStreamPackingType eDestPktType)
{
	typedef struct _StruRegPkEdIf
	{
		EnumStreamPackingType eSupportDestPktType;
		CStreamPackEncoder* (*Create)(EnumStreamPackingType eDestPktType);
	} StruRegPkEdIf;

#define FUNC(e, classname ) \
	{(EnumStreamPackingType)e, (CStreamPackEncoder*(*)(EnumStreamPackingType)) classname::Create }

	static StruRegPkEdIf _vEdExistReg[] =
	{
		FUNC(eSTREAM_PKG_28181PS, CStPkEdStand28181Ps ),	
		FUNC(eSTREAM_PKG_GSPS,CStPkEdGosunPS),
		FUNC(eSTREAM_PKG_Standard,CStPkEdStandStream),
	};

	for( UINT i=0; i<ARRARY_SIZE(_vEdExistReg); i++ )
	{
		if( _vEdExistReg[i].eSupportDestPktType == eDestPktType )
		{
			return _vEdExistReg[i].Create(eDestPktType);
		}
	}
	GS_ASSERT(0);
	return NULL;

}