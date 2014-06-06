#include "ExtPackDecoder.h"
#include <time.h>
#ifdef ENABLE_MANUFACTURERS_STREAM_ANALYZER
#include "../ExtLibrary/DaHua/StreamAnalyzer.h"
#include "../ExtLibrary/HiK/AnalyzeDataNewInterface.h"
#endif

namespace GSP
{


#ifdef ENABLE_MANUFACTURERS_STREAM_ANALYZER
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
	*@brief : 大华 ==> 标准
	*
	*********************************************************************
	*/
	class CStPkDdDaHua : CStPkDdBase
	{
	private :
		ANA_HANDLE m_hANA;
		EnumGSCodeID m_vGsCodeId[2];
		int m_vDHCodeId[2];
		UINT32 m_iLastTime;
	public :
		CStPkDdDaHua(void) : CStPkDdBase(eSTREAM_PKG_DaHua)
		{
			m_hANA = NULL;
			m_vGsCodeId[0] = GS_CODEID_NONE;
			m_vGsCodeId[1] = GS_CODEID_NONE;
			m_vDHCodeId[0] =  -1;
			m_vDHCodeId[1] =  -1;

			m_iLastTime = (UINT32) time(NULL);
		}

		virtual ~CStPkDdDaHua(void)
		{
			if( m_hANA )
			{
				ANA_Destroy(m_hANA);
				m_hANA = NULL;
			}
		}

		//必须增加的函数
		static CStreamPackDecoder *Create( EnumStreamPackingType eSrcPktType )
		{
			return new CStPkDdDaHua();	
		}

		

		virtual EnumErrno Init(BOOL bOutFactorStream)
		{
			CStPkDdBase::Init(bOutFactorStream);
			int iRet =  ANA_CreateStream(0,(PANA_HANDLE) &m_hANA);
			if( iRet )
			{
				GS_ASSERT(0);
				m_hANA = NULL;
				return eERRNO_EUNKNOWN;
			}
			return eERRNO_SUCCESS;
		}

		virtual EnumErrno Decode( CFrameCache *pFrame, BOOL bExistGSFH ) 
		{
			UINT iOffsetHeader = 0;
			CGSPBuffer &csBuf = pFrame->GetBuffer();
			if( bExistGSFH )
			{
				iOffsetHeader = sizeof(StruGSFrameHeader);
			}


			if( csBuf.m_iDataSize<=(iOffsetHeader) )
			{
				GS_ASSERT(0);			
				return eERRNO_SYS_EINVALID;
			}
			const BYTE *p = csBuf.m_bBuffer+iOffsetHeader;
			UINT iSize = csBuf.m_iDataSize-iOffsetHeader;
			EnumErrno eRet = Analyze(p, iSize);		
			return eRet;
		}

	private :

		EnumErrno Analyze(const BYTE *pData, INT iSize )
		{
			INT iRet;
			ANA_FRAME_INFO stInfo;
			CFrameCache *pFrame = NULL;
			EnumGSCodeID eCodeId =  GS_CODEID_NONE;
			EnumGSMediaType eMediaType = GS_MEDIA_TYPE_NONE;

			BOOL bKey = FALSE;


			while( iSize>0 )
			{
				iRet = ANA_InputData(m_hANA, 
					(uint8 *)pData, 
					iSize);

				if( iRet>=0  )
				{
					iSize -= iRet;
					pData += iRet;
				}
				else
				{
					iSize = 0;
					//	ANA_Reset(m_hANA, 0);
				}

				while(  0==(iRet = ANA_GetMediaFrame(m_hANA, &stInfo) ) )
				{
					eCodeId = GS_CODEID_NONE;				
					eMediaType = GS_MEDIA_TYPE_NONE;
					bKey = FALSE;
					if( stInfo.nType == FRAME_TYPE_VIDEO  )
					{				
						eMediaType = GS_MEDIA_TYPE_VIDEO;

						if( stInfo.nSubType == TYPE_VIDEO_I_FRAME)
						{
							bKey = TRUE;
						}

						switch( stInfo.nEncodeType )
						{
						case ENCODE_VIDEO_MPEG4 :
							{
								eCodeId = GS_CODEID_ST_MP4;							
							}
							break;
						case ENCODE_VIDEO_DH_H264 :
						case ENCODE_VIDEO_HI_H264 :
							{
								eCodeId = GS_CODEID_ST_H264;							
							}
							break;
						case ENCODE_VIDEO_JPEG :
							{
								eCodeId = GS_CODEID_ST_MP4;							
							}
							break;
						default :
							{
								eCodeId = GS_CODEID_NONE;
							}
							break;
						} // end switch

					}
					else if( stInfo.nType == FRAME_TYPE_AUDIO  )
					{
						eMediaType = GS_MEDIA_TYPE_AUDIO;
						bKey = TRUE;
						switch( stInfo.nEncodeType )
						{
						case ENCODE_AUDIO_PCM :
							{
								eCodeId = GS_CODEID_AUDIO_ST_PCM;
							}
							break;
						case ENCODE_AUDIO_G729 :						
							{
								eCodeId = GS_CODEID_AUDIO_ST_G729;							
							}
							break;
						case ENCODE_AUDIO_G721 :
							{
								eCodeId = GS_CODEID_AUDIO_ST_G721;

							}
							break;
						case ENCODE_AUDIO_G711A :
							{
								eCodeId = GS_CODEID_AUDIO_ST_G711A;

							}
							break;
						case ENCODE_AUDIO_G711U :
							{
								eCodeId = GS_CODEID_AUDIO_ST_G711U;							
							}
							break;
						case ENCODE_AUDIO_G723 :
							{
								eCodeId = GS_CODEID_AUDIO_ST_G723;							
							}
							break;
						case ENCODE_VIDEO_H263 :
							{
								eCodeId = GS_CODEID_AUDIO_ST_H263;
							}
							break;
						default :
							{
								eCodeId = GS_CODEID_NONE;
							}
							break;
						} // end switch
					}
					else
					{
						//其他数据不需要 
						continue;
					}

					UINT iChn = GetMediaChannel(eMediaType);
					if( iChn>GSP_MAX_MEDIA_CHANNELS || eCodeId == GS_CODEID_NONE /*|| !stInfo.bValid*/ )
					{
						/*GS_ASSERT(!stInfo.bValid);*/
						continue;
					}
					if( eCodeId == m_vInfoCtx[iChn].eCodeId  )
					{ 

					}
					else if( m_vInfoCtx[iChn].eCodeId == GS_CODEID_NONE )
					{
						if( eMediaType == GS_MEDIA_TYPE_AUDIO )
						{
							UINT iTempCh = GetMediaChannel(GS_MEDIA_TYPE_VIDEO);
							if( iTempCh>=GSP_MAX_MEDIA_CHANNELS 
								|| m_vInfoCtx[iTempCh].eCodeId == GS_CODEID_NONE)
							{
								//等待视频后再设置
								continue;
							}
						}
						m_vInfoCtx[iChn].eCodeId = eCodeId;					
					}
					else 
					{
						//中间改变了编码
						GS_ASSERT(0);
						return eERRNO_SYS_ECODEID;
					}

					StruBaseBuf stTemp;
					bzero(&stTemp, sizeof(stTemp));
					if( m_bOutFactorStream )
					{
						//厂商流
						stTemp.iSize = stInfo.nLength;
						stTemp.pBuffer =  (BYTE*)stInfo.pHeader; 
					}
					else
					{
						//裸标准流
						stTemp.iSize = stInfo.nBodyLength;
						stTemp.pBuffer =  (BYTE*)stInfo.pFrameBody; 
					}
					pFrame = pFrame->Create(&stTemp, 1);

					GS_ASSERT(pFrame);
					if( pFrame )
					{
						pFrame->m_stFrameInfo.iChnNo = iChn;
						pFrame->m_stFrameInfo.bKey = bKey;
						pFrame->m_stFrameInfo.bSysHeader = (eMediaType == GS_MEDIA_TYPE_SYSHEADER);
						pFrame->m_stFrameInfo.eMediaType = eMediaType;
						if ( stInfo.nYear > 0 )
						{
							m_iLastTime = MakeTime(stInfo.nYear,stInfo.nMonth,
								stInfo.nDay,stInfo.nHour,
								stInfo.nMinute,stInfo.nSecond);
						}
						pFrame->m_stFrameInfo.iTimestamp  = m_iLastTime;
						if( m_listFrameCache.AddTail(pFrame) )
						{
							GS_ASSERT(0);
							pFrame->UnrefObject();
						}
					}
				} // end while(  0==(iRet
			} // end while(iSize>0)
			return eERRNO_SUCCESS;
		}


	};




	/*
	*********************************************************************
	*
	*@brief : 海康 ==> 分析
	*
	*********************************************************************
	*/
	class CStPkDdHiK : CStPkDdBase
	{
	private :
		HANDLE m_hANA;		
		UINT32 m_iLastTime;
		INT m_iInitTrys;
	public :
		CStPkDdHiK(void) : CStPkDdBase(eSTREAM_PKG_HiKVS)
		{
			m_hANA = NULL;			
			m_iInitTrys = 0;
			m_iLastTime = (UINT32) time(NULL);
		}

		virtual ~CStPkDdHiK(void)
		{
			if( m_hANA )
			{
				HIKANA_Destroy(m_hANA);
				m_hANA = NULL;
			}
		}

		//必须增加的函数
		static CStreamPackDecoder *Create( EnumStreamPackingType eSrcPktType )
		{
			return new CStPkDdHiK();	
		}

		virtual EnumGSCodeID GetCodeId( UINT iChnNo )
		{
			return GS_CODEID_HK_COMPLEX;
		}

		virtual EnumGSMediaType GetMeidaType(UINT iChnNo )
		{
			if( iChnNo == 0 )
			{
				return GS_MEDIA_TYPE_SYSHEADER;
			}
			else if( iChnNo == 2 )
			{
				return GS_MEDIA_TYPE_AUDIO;
			}
			return GS_MEDIA_TYPE_VIDEO;
		}


		virtual EnumErrno Decode( CFrameCache *pFrame, BOOL bExistGSFH ) 
		{

			CGSPBuffer &csBuf = pFrame->GetBuffer();
			const BYTE *p = csBuf.m_bBuffer;
			INT iSize = csBuf.m_iDataSize;
			if( bExistGSFH )
			{
				iSize -= sizeof(StruGSFrameHeader);
				p +=  sizeof(StruGSFrameHeader);
			}
// 			if( iSize<40 )
// 			{
// 				GS_ASSERT(0);
// 				return eERRNO_SYS_ECODEID;
// 			}


			if( m_hANA == NULL )
			{
				if( m_iInitTrys > 500 )
				{
					GS_ASSERT(0);
					return eERRNO_SYS_ECODEID;
				}	
				m_hANA = HIKANA_CreateStreamEx(KBYTES*768, (BYTE*) p);
				if( m_hANA == NULL )
				{
					m_iInitTrys++;
					return eERRNO_SUCCESS;
				}
				m_iInitTrys = 0;				
				m_iLastTime = (UINT32) time(NULL);
			}	
			EnumErrno eRet = Analyze(p, iSize);		
			return eRet;
		}

	private :

		EnumErrno Analyze(const BYTE *pData, INT iSize )
		{
			INT iRet;
			PACKET_INFO_EX stInfo;
			CFrameCache *pFrame = NULL;
			EnumGSCodeID eCodeId =  GS_CODEID_NONE;
			EnumGSMediaType eMediaType = GS_MEDIA_TYPE_NONE;

			BOOL bKey = FALSE;
			UINT iChn = 1;

			while( iSize>0 )
			{
				iRet = HIKANA_InputData(m_hANA, 
					(uint8 *)pData, 
					iSize);
				iSize = 0;

				while(  0==(iRet = HIKANA_GetOnePacketEx(m_hANA, &stInfo) ) )
				{
					eCodeId = GS_CODEID_NONE;				
					eMediaType = GS_MEDIA_TYPE_NONE;
					bKey = FALSE;
					iChn = 1;
					if( stInfo.nPacketType == VIDEO_I_FRAME )
					{
						eMediaType = GS_MEDIA_TYPE_VIDEO;
						eCodeId = GS_CODEID_HK_COMPLEX;	
						bKey = TRUE;
					}
					else if( stInfo.nPacketType == AUDIO_PACKET  )
					{
						eMediaType = GS_MEDIA_TYPE_AUDIO;
						bKey = TRUE;
						eCodeId = GS_CODEID_HK_COMPLEX;		
						iChn = 0;
					}
					else if( stInfo.nPacketType == FILE_HEAD )
					{
						eMediaType = GS_MEDIA_TYPE_SYSHEADER;
						bKey = TRUE;
						eCodeId = GS_CODEID_HK_COMPLEX;	
						iChn = 1;
					}
					else
					{
						eMediaType = GS_MEDIA_TYPE_VIDEO;
						bKey = FALSE;						
						eCodeId = GS_CODEID_HK_COMPLEX;	
						
					}
					
					
					StruBaseBuf stTemp;
					bzero(&stTemp, sizeof(stTemp));
					stTemp.iSize = stInfo.dwPacketSize;
					stTemp.pBuffer =  (BYTE*)stInfo.pPacketBuffer; 
					pFrame = pFrame->Create(&stTemp, 1);

					GS_ASSERT(pFrame);
					if( pFrame )
					{
						pFrame->m_stFrameInfo.iChnNo = iChn;
						pFrame->m_stFrameInfo.bKey = bKey;
						pFrame->m_stFrameInfo.bSysHeader = (eMediaType == GS_MEDIA_TYPE_SYSHEADER);
						pFrame->m_stFrameInfo.eMediaType = eMediaType;
						if ( stInfo.nYear > 0 )
						{
							m_iLastTime = MakeTime(stInfo.nYear,stInfo.nMonth,
								stInfo.nDay,stInfo.nHour,
								stInfo.nMinute,stInfo.nSecond);
						}
						pFrame->m_stFrameInfo.iTimestamp  = m_iLastTime;
						if( m_listFrameCache.AddTail(pFrame) )
						{
							GS_ASSERT(0);
							pFrame->UnrefObject();
						}
					}
				} // end while(  0==(iRet
			} // end while(iSize>0)
			return eERRNO_SUCCESS;
		}


	};

	void ResigterExtPackDecoder(void)
	{
#define FUNC_DEF(f)  (CStreamPackDecoder*(*)(EnumStreamPackingType))(f)

		CStreamPackDecoder::ResigterExtDecoder(eSTREAM_PKG_DaHua,FUNC_DEF(CStPkDdDaHua::Create) );
		CStreamPackDecoder::ResigterExtDecoder(eSTREAM_PKG_HiKVS,FUNC_DEF(CStPkDdHiK::Create)  );
	}
#else

void ResigterExtPackDecoder(void)
{
}

#endif



} //end namespace GSP
