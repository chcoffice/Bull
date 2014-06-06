#include "StreamPackConvert.h"
#include "List.h"
#include "SIP/ES2PS.h"
#include "RTP/RtpStru.h"
#include <time.h>
#include "StreamPackDecoder.h"
#include "StreamPackEncoder.h"


using namespace GSP;
using namespace GSP::SIP;
using namespace GSP::RTP;



namespace GSP
{



	/*
	*********************************************************************
	*
	*@brief : CStPkCnvBase CStreamPackDecoder 的基本实现
	*
	*********************************************************************
	*/
	static void _FreeListMember(CFrameCache *p)
	{
		p->UnrefObject();
	}

	class CStPkCnvBase : public CStreamPackConvert
	{
	protected :
		CList m_listFrameCache; // 存储 CFrameCache * 分析完的结果
		EnumStreamPackingType m_eSrcPktType;
		EnumStreamPackingType m_eDestPktType;
		BOOL m_bInsertGSFH;
	public :

		virtual EnumErrno Init(BOOL bInsertGSFH)
		{
			m_bInsertGSFH = bInsertGSFH ? 1 : 0;
			return eERRNO_SUCCESS;
		}		
		virtual CFrameCache *Get(void)
		{
			void *p = NULL;
			m_listFrameCache.RemoveFront(&p);
			return (CFrameCache*)p;
		}

		virtual EnumStreamPackingType GetSrcPackType( void) const
		{
			return m_eSrcPktType;
		}
		virtual EnumStreamPackingType GetDestPackType( void) const
		{
			return m_eDestPktType;
		}


	protected :
		CStPkCnvBase(EnumStreamPackingType eSrcPktType,
			EnumStreamPackingType eDestPktType) : CStreamPackConvert()
		{
			m_eSrcPktType = eSrcPktType;
			m_eDestPktType = eDestPktType;
			m_listFrameCache.SetFreeCallback((FuncPtrFree)_FreeListMember);			
			m_bInsertGSFH = FALSE;

		}
		virtual ~CStPkCnvBase(void)
		{
			m_listFrameCache.Clear();
		}		
	};




	/*
	*********************************************************************
	*
	*@brief : 空转码转码
	*
	*********************************************************************
	*/
	class CStPkCnvEmpty  : public CStPkCnvBase
	{
	private :	
		CMediaInfo m_csMediaInfo;
	public :
		CStPkCnvEmpty(EnumStreamPackingType eSrcPktType,
			EnumStreamPackingType eDestPktType) 
			: CStPkCnvBase(eSrcPktType, eDestPktType)
		{

		}

		virtual ~CStPkCnvEmpty(void)
		{		


		}


		//必须增加的函数
		static CStreamPackConvert *Create( EnumStreamPackingType eSrcPktType,
			EnumStreamPackingType eDestPktType )
		{
			return new CStPkCnvEmpty(eSrcPktType, eDestPktType);
		}



		virtual EnumGSCodeID GetCodeId( UINT iChnNo )
		{
			return CMediaInfo::TestGsCodeId4StreamPkt(m_eDestPktType);
			// 			const CIMediaInfo::StruMediaChannelInfo *p = m_csMediaInfo.GetChannel((INT)iChnNo);
			// 			if( p)
			// 			{
			// 				return (EnumGSCodeID) p->stDescri.unDescri.struVideo.eCodeID;
			// 			}
			// 			return GS_CODEID_NONE;

		}

		virtual EnumGSMediaType GetMeidaType(UINT iChnNo )
		{
			return GS_MEDIA_TYPE_VIDEO;
			// 			const CIMediaInfo::StruMediaChannelInfo *p = m_csMediaInfo.GetChannel((INT)iChnNo);
			// 			if( p)
			// 			{
			// 				return (EnumGSMediaType) p->stDescri.eMediaType;
			// 			}
			// 			return GS_MEDIA_TYPE_NONE;
		}



		virtual EnumErrno Conver( CFrameCache *pFrame, BOOL bExistGSFH )
		{			
			if( bExistGSFH )
			{
				bExistGSFH = 1;
			}

			if( m_bInsertGSFH^bExistGSFH )
			{
				//要增增加或去掉GS 头


				if( m_bInsertGSFH )
				{
					//增加头

					StruGSFrameHeader stHeader;
					bzero(&stHeader, sizeof(stHeader));	
					if( pFrame->m_stFrameInfo.eMediaType )
					{
						stHeader.eMediaType = pFrame->m_stFrameInfo.eMediaType;
						stHeader.bKey = pFrame->m_stFrameInfo.bKey;
						stHeader.iTimeStamp = (UINT32) pFrame->m_stFrameInfo.iTimestamp;
					}
					else
					{
						stHeader.eMediaType = GS_MEDIA_TYPE_VIDEO;
						stHeader.iTimeStamp = (UINT32)time(NULL);
						pFrame->m_stFrameInfo.eMediaType = GS_MEDIA_TYPE_VIDEO;
						pFrame->m_stFrameInfo.iTimestamp = stHeader.iTimeStamp;
						pFrame->m_stFrameInfo.iChnNo = 0;


					}

					stHeader.iMagic = GS_FRAME_HEADER_MAGIC;					
					stHeader.iLenght = pFrame->GetBuffer().m_iDataSize;
					StruBaseBuf vTmp;
					vTmp.iSize = sizeof(stHeader);
					vTmp.pBuffer = &stHeader;
					CFrameCache *pTemp = pFrame->MergeFront(&vTmp, 1 );
					if( !pTemp )
					{
						GS_ASSERT(0);						
						return eERRNO_SYS_ENMEM;
					}
					if( m_listFrameCache.AddTail(pTemp))
					{			
						GS_ASSERT(0);
						pTemp->UnrefObject();
						return eERRNO_SYS_ENMEM;
					}					
				}
				else
				{
					//减少头
					CFrameCache *pTemp  = dynamic_cast<CFrameCache *>(pFrame->Clone());
					if( !pTemp )
					{
						GS_ASSERT(0);						
						return eERRNO_SYS_ENMEM;
					}
					pTemp->CutFront(sizeof(StruGSFrameHeader));
					if( !pFrame->m_stFrameInfo.eMediaType )
					{
						pFrame->m_stFrameInfo.eMediaType = GS_MEDIA_TYPE_VIDEO;
						pFrame->m_stFrameInfo.iTimestamp = (long) time(NULL);
						pFrame->m_stFrameInfo.iChnNo = 0;
					}

					if( m_listFrameCache.AddTail(pTemp))
					{			
						GS_ASSERT(0);
						pTemp->UnrefObject();
						return eERRNO_SYS_ENMEM;
					}

				}
				return eERRNO_SUCCESS;
			}

			if( m_listFrameCache.AddTail(pFrame))
			{
				return eERRNO_SYS_ENMEM;
			}
			pFrame->RefObject();

			return eERRNO_SUCCESS;
		}

	protected :	

		virtual void BindChannelInfo( const CMediaInfo &csMediaInfo )
		{			
			m_csMediaInfo = csMediaInfo;

		}
	private :			
	};


	/*
	*********************************************************************
	*
	*@brief : 
	*
	*********************************************************************
	*/
	class CStPkCnvGSIPC2PS : public CStPkCnvBase
	{
	private :	
#define GSIPC_STREAM_MAGIC (('H'<<24) | ('X'<<16) | ('X'<<8) | 'G')

		typedef struct StruGSIPCHeadInfo
		{
			INT32 dwMagicCode;           //模数：'G','X','X','H'
			INT32 dwSecCapTime;          //采集时间 的秒部分
			INT32 dwUSecCapTime;         //采集时间 的微秒部分			
		}StruGSIPCHeadInfo ,*StruGSIPCHeadInfoPtr;

#define GSIPCHEADER_SIZE 12

		std::map<EnumGSMediaType, EnumGSCodeID > m_mapBindInfo;
		BOOL m_bFirstFrame;
		BOOL m_bIsPSStream;
		CStreamPackEncoder *m_pEncoder;		
		CMediaInfo m_csMediaInfo;
	public :
		CStPkCnvGSIPC2PS(void) 
			: CStPkCnvBase(eSTREAM_PKG_GSIPC, eSTREAM_PKG_28181PS)
		{
			m_bFirstFrame = FALSE;
			m_bIsPSStream = TRUE;
			m_pEncoder = NULL;
			m_mapBindInfo.insert( make_pair(GS_MEDIA_TYPE_VIDEO, GS_CODEID_ST_H264));
			m_mapBindInfo.insert(make_pair(GS_MEDIA_TYPE_AUDIO, GS_CODEID_AUDIO_ST_G711A));
		}

		virtual ~CStPkCnvGSIPC2PS(void)
		{		

			if( m_pEncoder )
			{
				delete m_pEncoder;
			}
		}


		//必须增加的函数
		static CStreamPackConvert *Create( EnumStreamPackingType eSrcPktType,
			EnumStreamPackingType eDestPktType )
		{
			if( eSrcPktType==eSTREAM_PKG_GSIPC &&
				eDestPktType == eSTREAM_PKG_28181PS )
			{
				return new CStPkCnvGSIPC2PS();
			}
			GS_ASSERT(0);
			return NULL;
		}

		virtual EnumGSCodeID GetCodeId( UINT iChnNo )
		{
			return GS_CODEID_GS_VIPC;
		}

		virtual EnumGSMediaType GetMeidaType(UINT iChnNo )
		{
			return GS_MEDIA_TYPE_VIDEO;
		}

		virtual EnumErrno Conver( CFrameCache *pFrame, BOOL bExistGSFH )
		{			

			StruGSIPCHeadInfo stIPCHeader;		
			int iCutSize = 0;
			CGSPBuffer *pBuf = &pFrame->GetBuffer();
			if( bExistGSFH )
			{
				iCutSize += sizeof( StruGSFrameHeader );
				memcpy(&stIPCHeader,pBuf->GetData()+sizeof( StruGSFrameHeader ), GSIPCHEADER_SIZE );				
			}
			else
			{
				memcpy(&stIPCHeader,pBuf->GetData(), GSIPCHEADER_SIZE );
			}


#if 0
			if( m_bFirstFrame )
			{				
				const BYTE *pp = NULL;
				INT iDataSize = 0;
				INT iPSType = CES2PS::TestPSStream(pBuf->GetData()+iCutSize,
					MIN(pBuf->GetDataSize()-iCutSize, 256), &pp, iDataSize );
				if( iPSType != PS_HEADER_NONE )
				{
					m_bIsPSStream = TRUE;
				}
				m_bFirstFrame = FALSE;
			}
#endif

			if( stIPCHeader.dwMagicCode == GSIPC_STREAM_MAGIC )
			{
				iCutSize += GSIPCHEADER_SIZE;
			}
#if 0
			else if( !m_bIsPSStream )
			{
				//非PS 流 有 8 个字节头
				iCutSize += 8;
			}
#endif

			CFrameCache *pNewFrame = NULL;
			if( iCutSize >0 )
			{
				pNewFrame =  dynamic_cast<CFrameCache *>(pFrame->Clone());
				if( !pNewFrame )
				{
					GS_ASSERT(0);						
					return eERRNO_SYS_ENMEM;
				}
				pNewFrame->m_stFrameInfo = pFrame->m_stFrameInfo;
				pNewFrame->CutFront(iCutSize);
				pFrame = pNewFrame;

			}

			bExistGSFH = 0;

			if( !m_bIsPSStream )
			{
				if( !(pFrame->m_stFrameInfo.eMediaType==GS_MEDIA_TYPE_VIDEO ||
					pFrame->m_stFrameInfo.eMediaType==GS_MEDIA_TYPE_AUDIO ) )
				{
					SAFE_DESTROY_REFOBJECT(&pNewFrame);
					return eERRNO_SUCCESS;
				}
				//非PS 流
				if( !m_mapBindInfo.empty() &&
					m_mapBindInfo.end() != m_mapBindInfo.find(pFrame->m_stFrameInfo.eMediaType) )
				{
					//告诉 编码器 帧类型

					const CIMediaInfo::StruMediaChannelInfo *pInfo = m_csMediaInfo.GetSubChannel(pFrame->m_stFrameInfo.iChnNo);
					if( pInfo )
					{
						m_pEncoder->BindChannelInfo(pFrame->m_stFrameInfo.iChnNo, 
							pFrame->m_stFrameInfo.eMediaType,
							m_mapBindInfo[pFrame->m_stFrameInfo.eMediaType], -1);
						m_mapBindInfo.erase(pFrame->m_stFrameInfo.eMediaType);	
					}							
				}
				EnumErrno eRet = m_pEncoder->Encode(pFrame, FALSE);			
				if( eRet)
				{
					GS_ASSERT(0);
					SAFE_DESTROY_REFOBJECT(&pNewFrame);
					return eRet;
				}
				CFrameCache *pTemp;
				while( (pTemp=m_pEncoder->Get())  )
				{
					if( m_listFrameCache.AddTail(pTemp))
					{			
						GS_ASSERT(0);
						pTemp->UnrefObject();
						SAFE_DESTROY_REFOBJECT(&pNewFrame);
						return eERRNO_SYS_ENMEM;
					}	
				}
				SAFE_DESTROY_REFOBJECT(&pNewFrame);
				return eERRNO_SUCCESS;
			}
			else if( m_bInsertGSFH )
			{
				//增加头

				StruGSFrameHeader stHeader;
				bzero(&stHeader, sizeof(stHeader));	
				if( pFrame->m_stFrameInfo.eMediaType )
				{
					stHeader.eMediaType = pFrame->m_stFrameInfo.eMediaType;
					stHeader.bKey = pFrame->m_stFrameInfo.bKey;
					stHeader.iTimeStamp = (UINT32) pFrame->m_stFrameInfo.iTimestamp;
				}
				else
				{
					stHeader.eMediaType = GS_MEDIA_TYPE_VIDEO;
					stHeader.iTimeStamp = (UINT32)time(NULL);
					pFrame->m_stFrameInfo.eMediaType = GS_MEDIA_TYPE_VIDEO;
					pFrame->m_stFrameInfo.iTimestamp = stHeader.iTimeStamp;
					pFrame->m_stFrameInfo.iChnNo = 0;
				}
				stHeader.iMagic = GS_FRAME_HEADER_MAGIC;					
				stHeader.iLenght = pFrame->GetBuffer().m_iDataSize;
				StruBaseBuf vTmp;
				vTmp.iSize = sizeof(stHeader);
				vTmp.pBuffer = &stHeader;
				CFrameCache *pTemp = pFrame->MergeFront(&vTmp, 1 );
				if( !pTemp )
				{
					GS_ASSERT(0);		
					SAFE_DESTROY_REFOBJECT(&pNewFrame);
					return eERRNO_SYS_ENMEM;
				}
				if( m_listFrameCache.AddTail(pTemp))
				{			
					GS_ASSERT(0);
					pTemp->UnrefObject();
					SAFE_DESTROY_REFOBJECT(&pNewFrame);
					return eERRNO_SYS_ENMEM;
				}					
			}


			if( m_listFrameCache.AddTail(pFrame))
			{
				SAFE_DESTROY_REFOBJECT(&pNewFrame);
				return eERRNO_SYS_ENMEM;
			}
			pFrame->RefObject();
			SAFE_DESTROY_REFOBJECT(&pNewFrame);
			return eERRNO_SUCCESS;
		}

	protected :


		virtual void BindChannelInfo( const CMediaInfo &csMediaInfo )
		{			
			m_csMediaInfo = csMediaInfo;
			if( m_pEncoder )
			{
				m_pEncoder->BindChannelInfo(csMediaInfo);
			}

		}


		virtual EnumErrno Init(BOOL bInsertGSFH)
		{
			CStPkCnvBase::Init(bInsertGSFH);		
			m_pEncoder = m_pEncoder->Make(m_eDestPktType);
			if( !m_pEncoder )
			{
				GS_ASSERT(0);
				return eERRNO_SYS_ECODEID;
			}
			EnumErrno eRet;			
			eRet = m_pEncoder->Init(bInsertGSFH);
			if( eRet )
			{
				GS_ASSERT(0);
				return eRet;
			}

			return eERRNO_SUCCESS;
		}	

	};


	/*
	*********************************************************************
	*
	*@brief : 
	*
	*********************************************************************
	*/
	class CStPkCnvNormal  : public CStPkCnvBase
	{
	private :	
		CStreamPackEncoder *m_pEncoder;
		CStreamPackDecoder *m_pDecoder;		
		std::map<EnumGSMediaType, BOOL > m_mapBindInfo;
	public :
		CStPkCnvNormal(EnumStreamPackingType eSrcPktType,
			EnumStreamPackingType eDestPktType) 
			: CStPkCnvBase(eSrcPktType, eDestPktType)
		{
			m_pEncoder = NULL;
			m_pDecoder = NULL;
			m_mapBindInfo.insert( make_pair(GS_MEDIA_TYPE_VIDEO, TRUE));
			m_mapBindInfo.insert(make_pair(GS_MEDIA_TYPE_AUDIO, TRUE));

		}

		virtual ~CStPkCnvNormal(void)
		{		
			SAFE_DELETE_OBJECT(&m_pEncoder);
			SAFE_DELETE_OBJECT(&m_pDecoder);

		}


		//必须增加的函数
		static CStreamPackConvert *Create( EnumStreamPackingType eSrcPktType,
			EnumStreamPackingType eDestPktType )
		{
			return new CStPkCnvNormal(eSrcPktType, eDestPktType);
		}


		virtual CFrameCache *Get(void)
		{
			GS_ASSERT(m_pEncoder);
			return m_pEncoder->Get();
		}

		virtual EnumGSCodeID GetCodeId( UINT iChnNo )
		{
			return m_pDecoder->GetCodeId(iChnNo);

		}

		virtual EnumGSMediaType GetMeidaType(UINT iChnNo )
		{
			return m_pDecoder->GetMeidaType(iChnNo);
		}



		virtual EnumErrno Conver( CFrameCache *pFrame, BOOL bExistGSFH )
		{
			EnumErrno eRet = m_pDecoder->Decode(pFrame, bExistGSFH);
			if( eRet )
			{
				GS_ASSERT(0);
				return eRet;
			}

			CFrameCache *p;
			while( (p=m_pDecoder->Get()) )
			{
				if( !m_mapBindInfo.empty() &&
					m_mapBindInfo.end() != m_mapBindInfo.find(p->m_stFrameInfo.eMediaType) )
				{
					//告诉 编码器 帧类型
					for( UINT i = 0; i<GSP_MAX_MEDIA_CHANNELS; i++ )
					{
						EnumGSCodeID eCodeId = m_pDecoder->GetCodeId(i);
						if(eCodeId )
						{
							m_pEncoder->BindChannelInfo(i, 
								m_pDecoder->GetMeidaType(i),
								eCodeId, -1);
						}
					}
					m_mapBindInfo.erase(p->m_stFrameInfo.eMediaType);
				}
				eRet = m_pEncoder->Encode(p, FALSE);
				p->UnrefObject();
				if( eRet)
				{
					GS_ASSERT(0);
					return eRet;

				}
			}
			return eRet;
		}		
	protected :
		virtual EnumErrno Init(BOOL bInsertGSFH)
		{
			CStPkCnvBase::Init(bInsertGSFH);

			m_pDecoder = m_pDecoder->Make(m_eSrcPktType);
			if( !m_pDecoder )
			{
				GS_ASSERT(0);
				return eERRNO_SYS_ECODEID;
			}
			m_pEncoder = m_pEncoder->Make(m_eDestPktType);
			if( !m_pEncoder )
			{
				GS_ASSERT(0);
				return eERRNO_SYS_ECODEID;
			}
			EnumErrno eRet;
			eRet = m_pDecoder->Init(FALSE);
			if( eRet )
			{
				GS_ASSERT(0);
				return eRet;
			}
			eRet = m_pEncoder->Init(bInsertGSFH);
			if( eRet )
			{
				GS_ASSERT(0);
				return eRet;
			}

			return eERRNO_SUCCESS;
		}	

		virtual void BindChannelInfo( const CMediaInfo &csMediaInfo )
		{			
			if( m_pEncoder )
			{
				m_pEncoder->BindChannelInfo(csMediaInfo);
			}

			if( m_pDecoder )
			{
				m_pDecoder->BindChannelInfo(csMediaInfo);
			}

		}
	private :			
	};

	/*
	*********************************************************************
	*
	*@brief : 海康标准流
	*
	*********************************************************************
	*/

	class CStPkCnvHk2Hk : public CStPkCnvBase
	{
	private :
		CStreamPackDecoder *m_pDecoder;		
	public :
		CStPkCnvHk2Hk(EnumStreamPackingType eSrcPktType,
			EnumStreamPackingType eDestPktType) 
			: CStPkCnvBase(eSrcPktType, eDestPktType)
		{
			m_pDecoder =  NULL;

		}

		virtual ~CStPkCnvHk2Hk(void)
		{		
			SAFE_DELETE_OBJECT(&m_pDecoder);
		}


		//必须增加的函数
		static CStreamPackConvert *Create( EnumStreamPackingType eSrcPktType,
			EnumStreamPackingType eDestPktType )
		{
			return   new CStPkCnvHk2Hk(eSrcPktType, eDestPktType);
		}

		virtual EnumErrno Conver( CFrameCache *pFrame, BOOL bExistGSFH )
		{
			EnumErrno eRet = m_pDecoder->Decode(pFrame, bExistGSFH);
			if( eRet )
			{
				GS_ASSERT(0);
				return eRet;
			}

			CFrameCache *p;
			while( (p=m_pDecoder->Get()) )
			{				
				if( m_bInsertGSFH )
				{
					//修养增加GXX 头
					StruGSFrameHeader stHeader;
					bzero(&stHeader, sizeof(stHeader));	

					stHeader.eMediaType = p->m_stFrameInfo.eMediaType;
					stHeader.bKey = p->m_stFrameInfo.bKey;
					stHeader.iMagic = GS_FRAME_HEADER_MAGIC;
					stHeader.iTimeStamp = (UINT32) p->m_stFrameInfo.iTimestamp;
					stHeader.iLenght = p->GetBuffer().m_iDataSize;
					StruBaseBuf vTmp;
					vTmp.iSize = sizeof(stHeader);
					vTmp.pBuffer = &stHeader;
					CFrameCache *pTemp = p->MergeFront(&vTmp, 1 );
					p->UnrefObject();
					if( !pTemp )
					{
						GS_ASSERT(0);						
						return eERRNO_SYS_ENMEM;
					}
					if( m_listFrameCache.AddTail(pTemp))
					{			
						GS_ASSERT(0);
						pTemp->UnrefObject();						
						return eERRNO_SYS_ENMEM;
					}					
				}
				else
				{
					if( m_listFrameCache.AddTail(p))
					{			
						GS_ASSERT(0);
						p->UnrefObject();
						return eERRNO_SYS_ENMEM;
					}	
				}
			}
			return eRet;
		}		


		virtual EnumGSCodeID GetCodeId( UINT iChnNo )
		{
			return GS_CODEID_HK_COMPLEX;

		}

		virtual EnumGSMediaType GetMeidaType(UINT iChnNo )
		{
			if( iChnNo == 1 )
			{
				return GS_MEDIA_TYPE_VIDEO;
			}
			else if( iChnNo == 2 )
			{
				return GS_MEDIA_TYPE_AUDIO;
			}
			return GS_MEDIA_TYPE_SYSHEADER;			
		}

	protected :
		virtual EnumErrno Init(BOOL bInsertGSFH)
		{
			CStPkCnvBase::Init(bInsertGSFH);

			m_pDecoder = m_pDecoder->Make(eSTREAM_PKG_HiKVS);
			if( !m_pDecoder )
			{
				GS_ASSERT(0);
				return eERRNO_SYS_ECODEID;
			}			
			EnumErrno eRet;
			eRet = m_pDecoder->Init(TRUE);
			if( eRet )
			{
				GS_ASSERT(0);
				return eRet;
			}			
			return eERRNO_SUCCESS;
		}	

		virtual void BindChannelInfo( const CMediaInfo &csMediaInfo )
		{	
			if( m_pDecoder )
			{
				m_pDecoder->BindChannelInfo(0, GS_MEDIA_TYPE_SYSHEADER );
				m_pDecoder->BindChannelInfo(1, GS_MEDIA_TYPE_VIDEO );
				m_pDecoder->BindChannelInfo(2, GS_MEDIA_TYPE_AUDIO );
			}
		}


	};


	/*
	*********************************************************************
	*
	*@brief : 
	*
	*********************************************************************
	*/
	class CStPkCnvDaH2DaH : public CStPkCnvBase
	{
	private :
		CStreamPackDecoder *m_pDecoder;		
	public :
		CStPkCnvDaH2DaH(EnumStreamPackingType eSrcPktType,
			EnumStreamPackingType eDestPktType) 
			: CStPkCnvBase(eSrcPktType, eDestPktType)
		{
			m_pDecoder =  NULL;

		}

		virtual ~CStPkCnvDaH2DaH(void)
		{		
			SAFE_DELETE_OBJECT(&m_pDecoder);
		}


		//必须增加的函数
		static CStPkCnvDaH2DaH *Create( EnumStreamPackingType eSrcPktType,
			EnumStreamPackingType eDestPktType )
		{
			return   new CStPkCnvDaH2DaH(eSrcPktType, eDestPktType);
		}

		virtual EnumGSCodeID GetCodeId( UINT iChnNo )
		{
			return GS_CODEID_DH_COMPLEX;

		}

		virtual EnumGSMediaType GetMeidaType(UINT iChnNo )
		{
			if( iChnNo == 1 )
			{
				return GS_MEDIA_TYPE_VIDEO;
			}
			else if( iChnNo == 2 )
			{
				return GS_MEDIA_TYPE_AUDIO;
			}
			return GS_MEDIA_TYPE_SYSHEADER;			
		}

		virtual EnumErrno Conver( CFrameCache *pFrame, BOOL bExistGSFH )
		{
			EnumErrno eRet = m_pDecoder->Decode(pFrame, bExistGSFH);
			if( eRet )
			{
				GS_ASSERT(0);
				return eRet;
			}

			CFrameCache *p;
			while( (p=m_pDecoder->Get()) )
			{				
				if( m_bInsertGSFH )
				{
					//修养增加GXX 头
					StruGSFrameHeader stHeader;
					bzero(&stHeader, sizeof(stHeader));	

					stHeader.eMediaType = p->m_stFrameInfo.eMediaType;
					stHeader.bKey = p->m_stFrameInfo.bKey;
					stHeader.iMagic = GS_FRAME_HEADER_MAGIC;
					stHeader.iTimeStamp = (UINT32) p->m_stFrameInfo.iTimestamp;
					stHeader.iLenght = p->GetBuffer().m_iDataSize;
					StruBaseBuf vTmp;
					vTmp.iSize = sizeof(stHeader);
					vTmp.pBuffer = &stHeader;
					CFrameCache *pTemp = p->MergeFront(&vTmp, 1 );
					p->UnrefObject();
					if( !pTemp )
					{
						GS_ASSERT(0);						
						return eERRNO_SYS_ENMEM;
					}
					if( m_listFrameCache.AddTail(pTemp))
					{			
						GS_ASSERT(0);
						pTemp->UnrefObject();						
						return eERRNO_SYS_ENMEM;
					}					
				}
				else
				{
					if( m_listFrameCache.AddTail(p))
					{			
						GS_ASSERT(0);
						p->UnrefObject();
						return eERRNO_SYS_ENMEM;
					}	
				}
			}
			return eRet;
		}		

	protected :
		virtual EnumErrno Init(BOOL bInsertGSFH)
		{
			CStPkCnvBase::Init(bInsertGSFH);

			m_pDecoder = m_pDecoder->Make(eSTREAM_PKG_DaHua);
			if( !m_pDecoder )
			{
				GS_ASSERT(0);
				return eERRNO_SYS_ECODEID;
			}			
			EnumErrno eRet;
			eRet = m_pDecoder->Init(TRUE);
			if( eRet )
			{
				GS_ASSERT(0);
				return eRet;
			}			
			return eERRNO_SUCCESS;
		}	

		virtual void BindChannelInfo( const CMediaInfo &csMediaInfo )
		{	
			if( m_pDecoder )
			{
				m_pDecoder->BindChannelInfo(0, GS_MEDIA_TYPE_SYSHEADER );
				m_pDecoder->BindChannelInfo(1, GS_MEDIA_TYPE_VIDEO );
				m_pDecoder->BindChannelInfo(2, GS_MEDIA_TYPE_AUDIO );
			}
		}


	};


	/*
	*********************************************************************
	*
	*@brief : 高兴新兴转码
	*
	*********************************************************************
	*/
	class CStPkCnvGXX  : public CStPkCnvBase
	{
	private :	
		CMediaInfo m_csMediaInfo;
	public :
		CStPkCnvGXX(EnumStreamPackingType eSrcPktType,
			EnumStreamPackingType eDestPktType) 
			: CStPkCnvBase(eSrcPktType, eDestPktType)
		{

		}

		virtual ~CStPkCnvGXX(void)
		{		


		}


		//必须增加的函数
		static CStreamPackConvert *Create( EnumStreamPackingType eSrcPktType,
			EnumStreamPackingType eDestPktType )
		{
			return new CStPkCnvGXX(eSrcPktType, eDestPktType);
		}



		virtual EnumGSCodeID GetCodeId( UINT iChnNo )
		{

			const CIMediaInfo::StruMediaChannelInfo *p = m_csMediaInfo.GetChannel((INT)iChnNo);
			if( p)
			{
				return (EnumGSCodeID) p->stDescri.unDescri.struVideo.eCodeID;
			}
			return GS_CODEID_NONE;
		}

		virtual EnumGSMediaType GetMeidaType(UINT iChnNo )
		{

			const CIMediaInfo::StruMediaChannelInfo *p = m_csMediaInfo.GetChannel((INT)iChnNo);
			if( p)
			{
				return (EnumGSMediaType) p->stDescri.eMediaType;
			}
			return GS_MEDIA_TYPE_NONE;
		}



		virtual EnumErrno Conver( CFrameCache *pFrame, BOOL bExistGSFH )
		{		
			const BYTE *p = pFrame->GetBuffer().GetData();
			INT iSize = pFrame->GetBuffer().GetDataSize();
			StruGSFrameHeader stHeader;
			if( iSize< sizeof(stHeader) )
			{
				GS_ASSERT(0);
				return eERRNO_SUCCESS;
			}
			memcpy( &stHeader, p, sizeof(stHeader));
			if( stHeader.iMagic != GS_FRAME_HEADER_MAGIC )
			{
				GS_ASSERT(0);
				return eERRNO_SUCCESS;
			}
			pFrame->m_stFrameInfo.bKey = stHeader.bKey;
			pFrame->m_stFrameInfo.eMediaType = (EnumGSMediaType) stHeader.eMediaType;
			pFrame->m_stFrameInfo.iTimestamp  = stHeader.iTimeStamp;
			pFrame->m_stFrameInfo.bSysHeader = pFrame->m_stFrameInfo.eMediaType == GS_MEDIA_TYPE_SYSHEADER;

			if( m_listFrameCache.AddTail(pFrame))
			{
				return eERRNO_SYS_ENMEM;
			}
			pFrame->RefObject();
			return eERRNO_SUCCESS;
		}

	protected :


		virtual void BindChannelInfo( const CMediaInfo &csMediaInfo )
		{			
			m_csMediaInfo = csMediaInfo;

		}
	private :			
	};





	/*
	*********************************************************************
	*
	*@brief : 海康标准流 =》 PS
	*
	*********************************************************************
	*/

	class CStPkCnvHk2PS : public CStPkCnvBase
	{
	private :
		CStreamPackDecoder *m_pDecoder;		
		CStreamPackEncoder *m_pPSEncoder; 
		int m_video_stream_type;
		int m_audio_stream_type;
		BYTE m_buf[1024];		
		bool m_bHadPSM;
		bool m_isPSStream;
		bool m_isTestPS;
		BYTE m_bufLastPsH[1024];	
		int m_iLastPsHSize;
	public :
		CStPkCnvHk2PS(EnumStreamPackingType eSrcPktType,
			EnumStreamPackingType eDestPktType) 
			: CStPkCnvBase(eSrcPktType, eDestPktType)
		{
			m_pDecoder =  NULL;
			m_video_stream_type  = 0 ;
			m_audio_stream_type = stream_type_audio_g711;			
			m_bHadPSM = false;
			m_isPSStream = false;
			m_isTestPS = false;
			m_pPSEncoder = NULL;
			m_iLastPsHSize = 0;

		}

		virtual ~CStPkCnvHk2PS(void)
		{		
			SAFE_DELETE_OBJECT(&m_pDecoder);	
			SAFE_DELETE_OBJECT(&m_pPSEncoder);	
		}


		//必须增加的函数
		static CStreamPackConvert *Create( EnumStreamPackingType eSrcPktType,
			EnumStreamPackingType eDestPktType )
		{
			return   new CStPkCnvHk2PS(eSrcPktType, eDestPktType);
		}




		virtual EnumErrno Conver( CFrameCache *pFrame, BOOL bExistGSFH )
		{
			EnumErrno eRet = m_pDecoder->Decode(pFrame, bExistGSFH);
			if( eRet )
			{
				GS_ASSERT(0);
				return eRet;
			}

			CFrameCache *p;

			while( (p=m_pDecoder->Get()) )
			{	
				if( GS_MEDIA_TYPE_SYSHEADER == p->m_stFrameInfo.eMediaType )
				{
					//去掉信息头
					SAFE_DESTROY_REFOBJECT(&p);
					continue;
				}

				if( !m_isTestPS )
				{
					if( !TestIsPSStream(p) )
					{
						SAFE_DESTROY_REFOBJECT(&p);
						continue;
					}

				}

				if( !m_isPSStream )
				{
					//非PS 流
					p->m_stFrameInfo.iChnNo = p->m_stFrameInfo.eMediaType == GS_MEDIA_TYPE_VIDEO ? 0 : 1;
					eRet = m_pPSEncoder->Encode(pFrame, FALSE);
					if( eRet)
					{
						GS_ASSERT(0);	
						SAFE_DESTROY_REFOBJECT(&p);
						return eRet;
					}

					CFrameCache *pTemp;
					while( (pTemp=m_pPSEncoder->Get())  )
					{
						if( m_listFrameCache.AddTail(pTemp))
						{			
							GS_ASSERT(0);
							pTemp->UnrefObject();
							SAFE_DESTROY_REFOBJECT(&p);
							return eERRNO_SYS_ENMEM;
						}	
					}
				}
				else 
				{
					//检测PS 头 
					if( !m_bHadPSM )
					{
// 						if( p->m_stFrameInfo.eMediaType == GS_MEDIA_TYPE_AUDIO )
// 						{ //增加音频帧头
// 							const BYTE *pBuf;				
// 							pBuf = p->GetBuffer().GetData();							
// 							if( pBuf[3] != 0xba && m_iLastPsHSize>0  )
// 							{
// 								//增加头
// 								StruBaseBuf stBuf;
// 								stBuf.iSize = m_iLastPsHSize;
// 								stBuf.pBuffer = m_bufLastPsH;
// 								CFrameCache *pNew = p->MergeFront(&stBuf, 1);
// 								if( pNew )
// 								{
// 									p->UnrefObject();
// 									p = pNew;
// 								}								
// 							}
// 						}
// 						else 
						if( !InsertPSM(p) )
						{
							GS_ASSERT(0);
							SAFE_DESTROY_REFOBJECT(&p);
							return eERRNO_SYS_ENMEM;
						}
					}

					if( m_bInsertGSFH )
					{

						//修养增加GXX 头
						StruGSFrameHeader stHeader;
						bzero(&stHeader, sizeof(stHeader));	

						stHeader.eMediaType = p->m_stFrameInfo.eMediaType;
						stHeader.bKey = p->m_stFrameInfo.bKey;
						stHeader.iMagic = GS_FRAME_HEADER_MAGIC;
						stHeader.iTimeStamp = (UINT32) p->m_stFrameInfo.iTimestamp;
						stHeader.iLenght = p->GetBuffer().m_iDataSize;
						StruBaseBuf vTmp;
						vTmp.iSize = sizeof(stHeader);
						vTmp.pBuffer = &stHeader;
						CFrameCache *pTemp = p->MergeFront(&vTmp, 1 );
						p->UnrefObject();
						if( !pTemp )
						{
							GS_ASSERT(0);						
							return eERRNO_SYS_ENMEM;
						}
						if( m_listFrameCache.AddTail(pTemp))
						{			
							GS_ASSERT(0);
							pTemp->UnrefObject();						
							return eERRNO_SYS_ENMEM;
						}					
					}
					else
					{
						if( m_listFrameCache.AddTail(p))
						{			
							GS_ASSERT(0);
							p->UnrefObject();
							return eERRNO_SYS_ENMEM;
						}	
					}
				}
			}
			return eRet;
		}		


		virtual EnumGSCodeID GetCodeId( UINT iChnNo )
		{
			return GS_CODEID_PS;

		}

		virtual EnumGSMediaType GetMeidaType(UINT iChnNo )
		{
			return GS_MEDIA_TYPE_VIDEO;				
		}

	protected :

		BOOL TestCodeID(CFrameCache *p)
		{
			CES2PS ps;
			const BYTE *pp = NULL, *pBuf;
			INT iPkStartHSize = 0;
			INT iDataSize = 0;
	//		INT iPSType;
			INT iS;
			pBuf = p->GetBuffer().GetData();
			iS = p->GetBuffer().GetDataSize();

			
// 			do 
// 			{
// 				iPSType = CES2PS::TestPSStream(pBuf,
// 					MIN(iS, 1024), &pp, iDataSize );
// 				if( iPSType == PS_HEADER_NONE )
// 				{
// 					return FALSE;
// 				}
// 				else if( iPSType == PS_HEADER_PES )
// 				{
// 					iS -= (pp-pBuf);
// 					pBuf = pp;
// 					INT j;
// 					UINT64 x,y;
// 					if( !CES2PS::PSStreamGetPES(pBuf, iS, &pp, iDataSize, j,x,y ) )
// 					{
// 						GS_ASSERT(0);
// 						return FALSE;
// 					}
// 					pBuf = pp;
// 					iS -= (pp-pBuf);
// 					break;
// 				}
// 				iS -= (pp-pBuf+iDataSize);
// 				pBuf = pp+iDataSize;				
// 			}while( iS > 256 );
// 
// 			if( iS <32 )
// 			{
// 				GS_ASSERT(0);
// 				return FALSE;
// 			}


			int isH264 = ps.h264_probe(pBuf,  MIN(iS, 8L*1024) );
			int isMP4 = ps.mpeg4video_probe(pBuf,  MIN(iS, 8L*1024) );

			if( isH264>isMP4 )
			{
				m_video_stream_type = stream_type_h264;
			}
			else if( isMP4 > 1 )
			{
				m_video_stream_type = stream_type_mpeg4;
			}
			else
			{
				m_video_stream_type = stream_type_h264;
			}
			return TRUE;

		}

		BOOL InsertPSM(CFrameCache *p )
		{
			if( p->m_stFrameInfo.eMediaType == GS_MEDIA_TYPE_VIDEO &&
				p->m_stFrameInfo.bKey )
			{

				const BYTE *pp = NULL, *pBuf;
				INT iPkStartHSize = 0;
				INT iDataSize = 0;
				INT iPSType;
				INT iS;
				pBuf = p->GetBuffer().GetData();
				iS = p->GetBuffer().GetDataSize();				
				do 
				{
					iPSType = CES2PS::TestPSStream(pBuf,
						MIN(iS, 256), &pp, iDataSize );
					if( iPSType == PS_HEADER_PK_START )
					{
						if( iDataSize <=  sizeof(m_buf) )
						{								
							iPkStartHSize = iDataSize;
							memcpy(m_buf, pp, iPkStartHSize);
							memcpy(m_bufLastPsH, pp, iPkStartHSize);
							m_iLastPsHSize = iPkStartHSize;
						}
					}
					else if( iPSType==  PS_HEADER_PSM  || iPSType == PS_HEADER_SMH )
					{
						m_bHadPSM = true;
						break;
					}
					else if( iPSType==  PS_HEADER_NONE || iPSType==PS_HEADER_PES )
					{
						break;
					}
					iS -= (pp-pBuf+iDataSize);
					pBuf = pp;
				} while( iS > 8 );


				if( !m_bHadPSM )
				{
					//增加PSM 头
					
					if( m_video_stream_type == 0 )
					{
						//检测流类型
						if( !TestCodeID(p) )
						{
							return TRUE;
						}
						
					}
					CES2PS ps;



					BYTE *pSYSBuf=m_buf;
					
					ps.SetConvertParam(25, m_video_stream_type);					
					ps.SetAudioType(m_audio_stream_type);
					INT iExistHSize = iPkStartHSize;

					if( iPkStartHSize > 0 )
					{
						pSYSBuf += iPkStartHSize;
					}
					else
					{
						iS = ps.make_ps_packet_header(pSYSBuf, 0, 25);
						iPkStartHSize += iS;	
						pSYSBuf += iPkStartHSize;
					}

					iS = ps.make_sys_packet_header(pSYSBuf, sizeof(m_buf)-iPkStartHSize, 
						0, 25, AUDIO_STREAM_ID );
					pSYSBuf += iS;
					iPkStartHSize += iS;
					PSM_tag psm;
					psm.stream_type_video = m_video_stream_type;
					psm.stream_type_audio = m_audio_stream_type;
					memcpy( pSYSBuf, &psm, sizeof(psm));
					pSYSBuf += sizeof(psm);
					iPkStartHSize += sizeof(psm);

					CFrameCache *pSYSFrame = p->Create(iPkStartHSize);
					if( pSYSFrame )
					{
						pSYSFrame->m_stFrameInfo = p->m_stFrameInfo;
						pSYSFrame->GetBuffer().SetData(m_buf, iPkStartHSize);
						if( m_listFrameCache.AddTail(pSYSFrame))
						{			
							GS_ASSERT(0);
							pSYSFrame->UnrefObject();
							return FALSE;
						}	
					}

				}
			}
			return TRUE;
		}

		BOOL TestIsPSStream(CFrameCache *p)
		{
			if( p->m_stFrameInfo.eMediaType != GS_MEDIA_TYPE_VIDEO 
				|| !p->m_stFrameInfo.bKey )
			{
				return FALSE;
			}
			const BYTE *pBuf = NULL;				
			INT iPSType;
			INT iS;
			pBuf = p->GetBuffer().GetData();
			iS = p->GetBuffer().GetDataSize();

			iPSType = CES2PS::TestStreamType(pBuf, MIN(iS, 8L*1024) );
			m_isPSStream = false;
			if( iPSType == stream_type_none )
			{
				return FALSE;
			}
			if( iPSType == stream_type_ps )
			{
				m_isPSStream = true;
			}
			else 
			{
				if( iPSType == stream_type_h264 )
				{
					m_video_stream_type = stream_type_h264;
				}
				else if( iPSType == stream_type_mpeg4 )
				{
					m_video_stream_type = stream_type_mpeg4;
				}
				else
				{
					GS_ASSERT(0);
					return FALSE;
				}
				m_pPSEncoder = m_pPSEncoder->Make(eSTREAM_PKG_28181PS);
				if( m_pPSEncoder )
				{
					GS_ASSERT(0);
					return FALSE;
				}
				if( m_pPSEncoder->Init(m_bInsertGSFH) )
				{
					GS_ASSERT(0);
					SAFE_DELETE_OBJECT(&m_pPSEncoder);
					return FALSE;
				}
				else
				{
					m_pPSEncoder->BindChannelInfo(0, 
						GS_MEDIA_TYPE_VIDEO,
						m_video_stream_type == stream_type_mpeg4  ?  GS_CODEID_ST_MP4 : GS_CODEID_ST_H264, 25);
					m_pPSEncoder->BindChannelInfo(1, 
						GS_MEDIA_TYPE_AUDIO, GS_CODEID_AUDIO_ST_G711A, -1);
				}
			}
			m_isTestPS = true;
			return TRUE;
		}

		virtual EnumErrno Init(BOOL bInsertGSFH)
		{
			CStPkCnvBase::Init(bInsertGSFH);

			m_pDecoder = m_pDecoder->Make(eSTREAM_PKG_HiKVS);
			if( !m_pDecoder )
			{
				GS_ASSERT(0);
				return eERRNO_SYS_ECODEID;
			}			
			EnumErrno eRet;
			eRet = m_pDecoder->Init(TRUE);
			if( eRet )
			{
				GS_ASSERT(0);
				return eRet;
			}			
			return eERRNO_SUCCESS;
		}	

		virtual void BindChannelInfo( const CMediaInfo &csMediaInfo )
		{	
			if( m_pDecoder )
			{
				m_pDecoder->BindChannelInfo(0, GS_MEDIA_TYPE_VIDEO );
				m_pDecoder->BindChannelInfo(1, GS_MEDIA_TYPE_VIDEO );
				m_pDecoder->BindChannelInfo(2, GS_MEDIA_TYPE_VIDEO );
			}
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


CStreamPackConvert *CStreamPackConvert::Make(BOOL bRealPlay, 
											 BOOL bServer, EnumStreamPackingType eSrcPktType,
											 EnumStreamPackingType eDestPktType, 
											 const CMediaInfo &csSrcMediaInfo,BOOL bInsertGSFH)
{
	typedef struct _StruRegPkEdIf
	{
		EnumStreamPackingType eSupportSrcPktType;
		EnumStreamPackingType eSupportDestPktType;
		CStreamPackConvert* (*Create)(EnumStreamPackingType eSrcPktType,EnumStreamPackingType eDestPktType);
	} StruRegPkEdIf;

#define FUNC(src,dest, classname ) \
	{(EnumStreamPackingType)src, (EnumStreamPackingType)dest, (CStreamPackConvert*(*)(EnumStreamPackingType,EnumStreamPackingType)) classname::Create }

	static StruRegPkEdIf _vEdExistReg[] =
	{		
		FUNC(eSTREAM_PKG_NONE, eSTREAM_PKG_NONE, CStPkCnvNormal ),		
		FUNC(eSTREAM_PKG_GSIPC, eSTREAM_PKG_28181PS, CStPkCnvGSIPC2PS ),	
		FUNC(eSTREAM_PKG_HiKVS, eSTREAM_PKG_28181PS, CStPkCnvHk2PS ),
	};
	CStreamPackConvert *pCvt = NULL;
	if( eSrcPktType == eDestPktType  )
	{
		if( eSrcPktType == eSTREAM_PKG_GSC3MVIDEO )
		{
			pCvt = CStPkCnvGXX::Create(eSrcPktType, eDestPktType);
		} 
		else if( bServer )
		{
			pCvt = CStPkCnvEmpty::Create(eSrcPktType, eDestPktType);
		}
		else if( eSrcPktType == eSTREAM_PKG_HiKVS )
		{
			pCvt =  CStPkCnvHk2Hk::Create(eSrcPktType, eDestPktType);			
		}
		else if( eSrcPktType == eSTREAM_PKG_DaHua )
		{
			if( bRealPlay )
			{
				pCvt = CStPkCnvEmpty::Create(eSrcPktType, eDestPktType);

			}
			else
			{
				pCvt =  CStPkCnvDaH2DaH::Create(eSrcPktType, eDestPktType);
			}
		}	

	}


	if(  pCvt == NULL  )
	{
		for( UINT i=0; i<ARRARY_SIZE(_vEdExistReg); i++ )
		{
			if( _vEdExistReg[i].eSupportSrcPktType == eSrcPktType &&
				_vEdExistReg[i].eSupportDestPktType == eDestPktType  )
			{
				pCvt = _vEdExistReg[i].Create(eSrcPktType, eDestPktType);
				break;
			}
		}
	}
	if( pCvt == NULL )
	{
		//使用通用转换器尝试
		pCvt = _vEdExistReg[0].Create(eSrcPktType, eDestPktType);
	}

	GS_ASSERT(pCvt);
	if( pCvt )
	{
		EnumErrno eRet = pCvt->Init(bInsertGSFH);
		if( eRet )
		{
			GS_ASSERT(0);
			delete pCvt;
			return NULL;
		}
		pCvt->BindChannelInfo(csSrcMediaInfo);
	}
	return pCvt;
}
