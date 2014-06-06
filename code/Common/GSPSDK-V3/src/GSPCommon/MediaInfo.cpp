#include "MediaInfo.h"
#include "GSPProDebug.h"
#include "RTP/RtpStru.h"
using namespace  GSP;
using namespace GSP::RTP;




CMediaInfo::CMediaInfo(void)
:CGSPObject()
,CIMediaInfo()
{
	bzero(&m_vChannels[0], sizeof(m_vChannels));
	for( UINT i =0; i<GSP_MAX_MEDIA_CHANNELS; i++ )
	{
		m_vChannels[i].iSubChannel = CIMediaInfo::INVALID_CHANNEL_ID;
	}
	m_iChannels = 0;

}

CMediaInfo::CMediaInfo(const CIMediaInfo &csBase )
:CGSPObject()
,CIMediaInfo()
{
	bzero(&m_vChannels[0], sizeof(m_vChannels));
	for( UINT i =0; i<GSP_MAX_MEDIA_CHANNELS; i++ )
	{
		m_vChannels[i].iSubChannel = CIMediaInfo::INVALID_CHANNEL_ID;
	}
	m_iChannels = 0;
	Set(csBase);
}

CMediaInfo::~CMediaInfo(void)
{

}

void CMediaInfo::Set( const CIMediaInfo &csBase )
{
	Clear();
const StruMediaChannelInfo *p;
UINT iCnt = csBase.GetChannelNums();
	for( UINT i = 0; i<iCnt; i++ )
	{
		p = csBase.GetChannel(i);
		AddChannel(&p->stDescri, p->iSubChannel, NULL);
	}	  
}

INT CMediaInfo::AddChannel( const StruGSMediaDescri *pMedia ,
								 INT iChn, const char *szRtpSdpFmtp)
{
	GS_ASSERT_RET_VAL( iChn<GSP_MAX_MEDIA_CHANNELS, CIMediaInfo::INVALID_CHANNEL_ID);

	if( iChn<0 )
	{
		for( INT i = 0; i<GSP_MAX_MEDIA_CHANNELS; i++ )
		{
			if( m_vChannels[i].iSubChannel==INVALID_CHANNEL_ID )
			{
				iChn = i;
				break;
			}
		}
		GS_ASSERT_RET_VAL( iChn>-1, CIMediaInfo::INVALID_CHANNEL_ID);
	}

	if( m_vChannels[iChn].iSubChannel == INVALID_CHANNEL_ID )
	{
		m_iChannels++;
		GS_ASSERT(m_iChannels<=GSP_MAX_MEDIA_CHANNELS);
	}
	m_vChannels[iChn].iSubChannel=iChn;
	::memcpy(&m_vChannels[iChn].stDescri, pMedia, sizeof(StruGSMediaDescri));

    return iChn;
}





void CMediaInfo::Clear(INT iChn)
{
    if( iChn<0 )
    {
		for( UINT i =0; i<GSP_MAX_MEDIA_CHANNELS; i++ )
		{
			m_vChannels[i].iSubChannel = CIMediaInfo::INVALID_CHANNEL_ID;
		}
		m_iChannels = 0;
    }
    else 
    {
        GS_ASSERT_RET( iChn<GSP_MAX_MEDIA_CHANNELS );
		if( m_vChannels[iChn].iSubChannel != INVALID_CHANNEL_ID )
		{
			m_iChannels--;
			GS_ASSERT(m_iChannels<=GSP_MAX_MEDIA_CHANNELS);
		}
		m_vChannels[iChn].iSubChannel = CIMediaInfo::INVALID_CHANNEL_ID; 
    }
}

void CMediaInfo::Clear( EnumGSMediaType eType )
{
    if( eType==GS_MEDIA_TYPE_NONE)
    {
        Clear(-1);
    }
    else
    {
		for( UINT i =0; i<GSP_MAX_MEDIA_CHANNELS; i++ )
		{
			if( m_vChannels[i].stDescri.eMediaType == (INT)eType )
			{
				if( m_vChannels[i].iSubChannel != INVALID_CHANNEL_ID )
				{
					m_iChannels--;
					GS_ASSERT(m_iChannels<=GSP_MAX_MEDIA_CHANNELS);
				}
				m_vChannels[i].iSubChannel = CIMediaInfo::INVALID_CHANNEL_ID;
			}
		}
	}
}


CGSPString  CMediaInfo::Serial2String(void) const 
{
    CGSPString csStr;
     csStr.clear();
    const StruGSMediaDescri *p = NULL;
     for( INT i = 0; i<GSP_MAX_MEDIA_CHANNELS; i++ )
     {
			if( m_vChannels[i].iSubChannel == CIMediaInfo::INVALID_CHANNEL_ID )
			{
				continue;
			}

            p = &m_vChannels[i].stDescri;
             //开始一个Section
             csStr += "\n******************Begin Section**********************\n";

             //媒体类型

             const char  *czType = GetMediaName((EnumGSMediaType)p->eMediaType);
             GSStrUtil::AppendWithFormat(csStr, "Media Type: %d(%s).\n",
                p->eMediaType, czType );
 

             //通道号
             GSStrUtil::AppendWithFormat(csStr, "Sub Channel: %d.\n",m_vChannels[i].iSubChannel );
   
             
             switch( p->eMediaType )
             {
             case GS_MEDIA_TYPE_VIDEO :
                 {    //视频数据

                     //编码类型 
                     
                    GSStrUtil::AppendWithFormat(csStr, "CodeID: %d.\n",p->unDescri.struVideo.eCodeID );
         
                     //图像宽
                    GSStrUtil::AppendWithFormat(csStr, "Width: %d.\n",p->unDescri.struVideo.iWidth);


                     ///图像高
                    GSStrUtil::AppendWithFormat(csStr, "Height: %d.\n",p->unDescri.struVideo.iHeight);

                     //帧率
                    GSStrUtil::AppendWithFormat(csStr, "Frame Rate: %d.%d.\n",
                                p->unDescri.struVideo.iFrameRate,
                                p->unDescri.struVideo.iFrameRate2);
                 }
                 break;
             case GS_MEDIA_TYPE_AUDIO :
                 {
                     //音频流, 属性描述

                     //获取 编码类型
                     GSStrUtil::AppendWithFormat(csStr, "CodeID: %d.\n",p->unDescri.struAudio.eCodeID );

        
                     //获取 采用频率
                     GSStrUtil::AppendWithFormat(csStr, "Sampel: %d.\n",p->unDescri.struAudio.iSample );

      
                     //获取音频的采样位数
                     GSStrUtil::AppendWithFormat(csStr, "Bits: %d.\n",p->unDescri.struAudio.iBits );

                     //获取通道数
                     GSStrUtil::AppendWithFormat(csStr, "Channels: %d.\n",p->unDescri.struAudio.iChannels );

  
                 }
                 break;
             case GS_MEDIA_TYPE_PICTURE:
                 {
                     //图片数据,属性描述 

                     //获取 编码类型
                     GSStrUtil::AppendWithFormat(csStr, "CodeID: %d.\n",p->unDescri.struPicture.eCodeID );

                  }
              break;   

             case    GS_MEDIA_TYPE_OSD:
                 {
                     //OSD 数据,属性描述

                     //获取OSD叠加的坐标 X
                     GSStrUtil::AppendWithFormat(csStr, "PosX: %d.\n",p->unDescri.struOsd.iPosX );


                     ///获取OSD叠加的坐标 Y
                     GSStrUtil::AppendWithFormat(csStr, "PosY: %d.\n",p->unDescri.struOsd.iPosY );


                     //OSD叠加 数据类型
                     GSStrUtil::AppendWithFormat(csStr, "Data Type: %d.\n",p->unDescri.struOsd.iDataType );

     
                     //OSD叠加 叠加的透明度
                     GSStrUtil::AppendWithFormat(csStr, "Transparency: %d.\n",p->unDescri.struOsd.iTransparency );

                 }
                 break;
             case     GS_MEDIA_TYPE_BINARY :
                 {         
                     //二进制流, 属性描述 

                     GSStrUtil::AppendWithFormat(csStr, "Size: %lld.\n",p->unDescri.struBinary.iSize);

                 }
                 break;

             case       GS_MEDIA_TYPE_SYSHEADER :
                 {
                     //信息头
                     GSStrUtil::AppendWithFormat(csStr, "Size: %d.\n",p->unDescri.struSysHeader.iSize);

                 }
                 break;
             default :
                 {
                     GSStrUtil::AppendWithFormat(csStr, "Warnning: Unknown media type.\n");
                 }
                 break;

             }
             //Section结束
             csStr += "\n******************End   Section**********************\n";
     }
     return csStr;
}

 

 CMediaInfo &CMediaInfo::operator=( const CMediaInfo &csDest )
 {
     if( &csDest != (CIMediaInfo*)this )
     {
		 ::memcpy(&m_vChannels[0],&csDest.m_vChannels[0], sizeof(m_vChannels) );
		 m_iChannels = csDest.m_iChannels;
     }
     return *this;
 }



INT CMediaInfo::IsSupport( const  CIMediaInfo *pRequest ) const
{
    return IsSupport(this, pRequest);
}


INT CMediaInfo::IsSupport( const  CIMediaInfo *pSrc ,const  CIMediaInfo *pRequest )
{
INT iRequest = pRequest->GetChannelNums();
INT iSrc = pSrc->GetChannelNums();
INT iRet = 0;

	if( iRequest == 0 )
	{
		return iSrc;
	}

	if( iRequest == 0 )
	{
		return 0;
	}
	
  
    BOOL bOK;

    const CIMediaInfo::StruMediaChannelInfo *pRequestChn, *pSrcChn;


    CIMediaInfo *pRequestTemp = (CIMediaInfo *)pRequest;
    for( int i=0; i<iRequest; i++ )
    {
        pRequestChn = pRequest->GetChannel(i);
        GS_ASSERT_RET_VAL(pRequest, FALSE);
        bOK = FALSE;
        for( int j = 0; j<iSrc; j++ )
        {

            pSrcChn = pSrc->GetChannel(i);
            if( CMediaInfo::CmpMediaDescri(&(pRequestChn->stDescri), &(pSrcChn->stDescri)) )
            {
                iRet++;
                break;
            }
        }
    }
    return iRet;
}

BOOL CMediaInfo::CmpMediaDescri(const StruGSMediaDescri *pSrc, const StruGSMediaDescri *pRequest )
{
    if( pSrc==pRequest || pRequest->eMediaType == GS_MEDIA_TYPE_NONE )
    {
        return TRUE;
    }

    if( pSrc->eMediaType==pRequest->eMediaType )
    {

        if(pSrc-> eMediaType==GS_MEDIA_TYPE_VIDEO)
        {
            if( pSrc->unDescri.struVideo.eCodeID==pRequest->unDescri.struVideo.eCodeID )
            {
                if( pSrc->unDescri.struVideo.iWidth==0 ||
                    pRequest->unDescri.struVideo.iWidth==0 ||
                    (pSrc->unDescri.struVideo.iWidth==pRequest->unDescri.struVideo.iWidth &&
                    pSrc->unDescri.struVideo.iHeight==pRequest->unDescri.struVideo.iHeight))
                {
                    return TRUE;
                }

            }
        }
        else if( pSrc->eMediaType==GS_MEDIA_TYPE_AUDIO)
        {
            if( pSrc->unDescri.struAudio.eCodeID==pRequest->unDescri.struAudio.eCodeID )
            {
                return TRUE;
            }
        }
        else
        {
            return TRUE;
        }
    }
    return FALSE;
}

const CIMediaInfo::StruMediaChannelInfo *CMediaInfo::GetSubChannel( INT iSubChannel ) const
{
	if( iSubChannel>-1 && iSubChannel<GSP_MAX_MEDIA_CHANNELS 		
		&& m_vChannels[iSubChannel].iSubChannel != CIMediaInfo::INVALID_CHANNEL_ID )
	{
		return &m_vChannels[iSubChannel];
		
	}
	
	return NULL;
}

EnumGSMediaType CMediaInfo::GetMediaType( UINT iIndex) const
{
const StruMediaChannelInfo *p;
	p = GetChannel(iIndex);
	if( p )
	{
		return (EnumGSMediaType)p->stDescri.eMediaType;
	}
	return GS_MEDIA_TYPE_NONE;
}

const CIMediaInfo::StruMediaChannelInfo *CMediaInfo::GetChannel( UINT16 iIndex ) const
{
	GS_ASSERT_RET_VAL(iIndex<m_iChannels, NULL);
	for( UINT i = 0, iCnts = 0; i<GSP_MAX_MEDIA_CHANNELS; i++ )
	{
		if( m_vChannels[i].iSubChannel!=CIMediaInfo::INVALID_CHANNEL_ID )
		{
			
			if( iCnts==iIndex )
			{
				return &m_vChannels[i];					 
			}
			iCnts++;
		}
	}
	GS_ASSERT(0);
	return NULL;
}

UINT16 CMediaInfo::GetChannelNums(void) const
{
	return m_iChannels;
}

BOOL CMediaInfo::GetChannel(UINT16 iIndex, StruGSMediaDescri *&ppResult, 
				INT &iSize) const
{
const CIMediaInfo::StruMediaChannelInfo *p = GetChannel(iIndex);
	if( p )
	{
		ppResult = (StruGSMediaDescri *)&p->stDescri;
		iSize = sizeof(*p);
		return TRUE;
	}
	return FALSE;
}
namespace GSP
{


struct _ReStPktMap
{
	EnumStreamPackingType ePktType;
	UINT32 iMinCodeId;
	UINT32 iMaxCodeId;	
	EnumGSCodeID eDefaultCodeId;
};

static struct _ReStPktMap s_vMap[] =
{	
	//标准码流
	{ eSTREAM_PKG_Standard, (UINT32)GS_CODEID_ST_MP4, (UINT32)GS_CODEID_ST_MP4+0x200, GS_CODEID_ST_MP4}, 
	//标准 28181 PS 流
	{ eSTREAM_PKG_28181PS,(UINT32)GS_CODEID_PS, (UINT32)GS_CODEID_PS, GS_CODEID_PS}, 
	{ eSTREAM_PKG_GSPS,(UINT32)GS_CODEID_GS_PS, (UINT32)GS_CODEID_GS_PS, GS_CODEID_GS_PS}, 
	//gosun  461C
	{ eSTREAM_PKG_GS461C, (UINT32)GS_CODEID_GS_V462C, (UINT32)GS_CODEID_GS_A461C, GS_CODEID_GS_V462C}, 
	//gosun  2160I
	{ eSTREAM_PKG_GS2160I, (UINT32)GS_CODEID_GS_V2160I, (UINT32)GS_CODEID_GS_A2160I, GS_CODEID_GS_V2160I}, 
	//gosun  2160I-V
	{ eSTREAM_PKG_GS2160IV,(UINT32)GS_CODEID_GS_V2160IV, (UINT32)GS_CODEID_GS_A2160IV, GS_CODEID_GS_V2160IV},
	//黄河 
	{ eSTREAM_PKG_HuangHe,(UINT32)GS_COIDEID_HH_VDEFULT, (UINT32)GS_COIDEID_HH_VDEFULT+0xff, GS_COIDEID_HH_VDEFULT}, 
	//大华
	{ eSTREAM_PKG_DaHua, (UINT32)GS_CODEID_DH_VDEFAULT, (UINT32)GS_CODEID_DH_VDEFAULT+0xff, GS_CODEID_DH_COMPLEX},
	//恒忆
	{ eSTREAM_PKG_HengYi, (UINT32)GS_CODEID_HY_VDEFAULT, (UINT32)GS_CODEID_HY_VDEFAULT+0xff,GS_CODEID_HY_COMPLEX}, 
	//海康
	{ eSTREAM_PKG_HiKVS, (UINT32)GS_CODEID_HK_VDEFAULT, (UINT32)GS_CODEID_HK_VDEFAULT+0xff, GS_CODEID_HK_COMPLEX},
	//中本
	{ eSTREAM_PKG_ZBeng, (UINT32)GS_CODEID_ZBEN_VDEFULT, (UINT32)GS_CODEID_ZBEN_VDEFULT+0xff,GS_CODEID_ZBEN_VDEFULT},
	//高凯视
	{ eSTREAM_PKG_CLS, (UINT32)GS_CODEID_CALSYS_VDEFULT, (UINT32)GS_CODEID_CALSYS_VDEFULT+0xff,GS_CODEID_CALSYS_VDEFULT},
	//安联锐视
	{ eSTREAM_PKG_ALRS, (UINT32)GS_COIDEID_ALRS_VDEFULT, (UINT32)GS_COIDEID_ALRS_VDEFULT+0xff,GS_COIDEID_ALRS_VDEFULT},
	//讯美
	{ eSTREAM_PKG_XM,(UINT32)GS_CODEID_XM_VDEFULT, (UINT32)GS_CODEID_XM_VDEFULT+0xff,GS_CODEID_XM_VDEFULT},
	//海狮
	{ eSTREAM_PKG_Hi, (UINT32)GS_CODEID_HI_VDEFAULT, (UINT32)GS_CODEID_HI_VDEFAULT+0xff,GS_CODEID_HI_VDEFAULT},
	//高凯视GS76xx系列
	{ eSTREAM_PKG_CALSYSGS76XX, (UINT32)GS_CODEID_CALSYSGS76XX_VDEFULT, 
	     (UINT32)GS_CODEID_CALSYSGS76XX_VDEFULT+0xff,GS_CODEID_CALSYSGS76XX_VDEFULT},
		 //GSC3MVIDEO 封装
	{ eSTREAM_PKG_GSC3MVIDEO, (UINT32)GS_CODEID_GSC3MVIDEO, (UINT32)GS_CODEID_GSC3MVIDEO,GS_CODEID_GSC3MVIDEO},

	//高新兴新IPC
	{ eSTREAM_PKG_GSIPC, (UINT32)GS_CODEID_GS_VIPC, (UINT32)GS_CODEID_GS_AIPC,GS_CODEID_GS_VIPC},

};

} //end namespace GSP

EnumStreamPackingType CMediaInfo::GetStreamPkt4GsCodeId(EnumGSCodeID eCodeId )
{

	for( int i = 0; i<ARRARY_SIZE(s_vMap); i++ )
	{
		if( s_vMap[i].iMinCodeId<= (UINT32) eCodeId &&
			s_vMap[i].iMaxCodeId >=  (UINT32) eCodeId )
		{
			return s_vMap[i].ePktType;
		}
	}
	return eSTREAM_PKG_NONE;
}

EnumGSCodeID CMediaInfo::TestGsCodeId4StreamPkt(EnumStreamPackingType  ePkgType )
{
	for( int i = 0; i<ARRARY_SIZE(s_vMap); i++ )
	{
		if( s_vMap[i].ePktType == ePkgType )
		{
			return s_vMap[i].eDefaultCodeId;
		}
	}
	return GS_CODEID_NONE;	
}


namespace GSP
{


typedef struct _StruGSPSStreamTypeMap
{
	EnumGSCodeID eGsCodeId;
	UINT8 iGSPSStreamType;
}StruGSPSStreamTypeMap;

static StruGSPSStreamTypeMap s_vGSPSStreamTypeMap [] =
{
#define IMPx(eGSCodeId, iGSPSStreamType) {(EnumGSCodeID)eGSCodeId, (UINT8)iGSPSStreamType }

	IMPx(GS_CODEID_NONE, GSPS_CODETYPE_NONE), //放在首行

	//=== 在下面添加新定义
	IMPx(GS_CODEID_ST_H264, GSPS_CODETYPE_V_H264),
	IMPx(GS_CODEID_ST_MP4, GSPS_CODETYPE_V_MP4),
	IMPx(GS_CODEID_ST_SVAC, GSPS_CODETYPE_V_SVAC),
	IMPx(GS_CODEID_AUDIO_ST_G711U, GSPS_CODETYPE_A_G711),
	IMPx(GS_CODEID_AUDIO_ST_G711A, GSPS_CODETYPE_A_G711),
	IMPx(GS_CODEID_AUDIO_ST_G722, GSPS_CODETYPE_A_G722),
	IMPx(GS_CODEID_AUDIO_ST_G723, GSPS_CODETYPE_A_G723),
	IMPx(GS_CODEID_AUDIO_ST_G729, GSPS_CODETYPE_A_G729),
	IMPx(GS_CODEID_AUDIO_ST_SVAC, GSPS_CODETYPE_A_SVAC),


};

} //end namespace GSP

UINT8 CMediaInfo::GetGSPSStreamType4GsCodeId(EnumGSCodeID eCodeId )
{
	for( UINT i = 1; i<ARRARY_SIZE(s_vGSPSStreamTypeMap); i++ )
	{
		if( s_vGSPSStreamTypeMap[i].eGsCodeId == eCodeId )
		{
			return s_vGSPSStreamTypeMap[i].iGSPSStreamType;
		}
	}
	return s_vGSPSStreamTypeMap[0].iGSPSStreamType;
}

EnumGSCodeID CMediaInfo::GetGsCodeId4GSPSStreamType(INT  iType )
{
	for( UINT i = 1; i<ARRARY_SIZE(s_vGSPSStreamTypeMap); i++ )
	{
		if( (INT) s_vGSPSStreamTypeMap[i].iGSPSStreamType == iType )
		{
			return s_vGSPSStreamTypeMap[i].eGsCodeId;
		}
	}
	return s_vGSPSStreamTypeMap[0].eGsCodeId;
}
