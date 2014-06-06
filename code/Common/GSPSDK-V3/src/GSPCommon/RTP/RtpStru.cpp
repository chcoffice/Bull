#include "RtpStru.h"
#include "../OSThread.h"
#include <time.h>
#include "../StrFormater.h"
#include "../MediaInfo.h"

namespace GSP
{

namespace RTP
{
	typedef  struct _StruCmdDescri
	{
		INT eID; // RtpPlayload | 
		const char *czName;
		INT iV2;  // EnumGSCodeID
		INT iV3; // EnumGSMediaType
		EnumStreamPackingType ePkt; //封装类型
	}StruCmdDescri;



	static StruCmdDescri s_vRtpPtDs[] = 
	{
		{111, "X-UNKNOW", GS_CODEID_UNKNOW , GS_MEDIA_TYPE_VIDEO, eSTREAM_PKG_NONE },
		{eRTP_PT_PS,  "PS", GS_CODEID_PS, GS_MEDIA_TYPE_VIDEO, eSTREAM_PKG_28181PS},
		{eRTP_PT_GXX,  "X-GSMedia", GS_CODEID_PS_GXX, GS_MEDIA_TYPE_VIDEO, eSTREAM_PKG_NONE},
		
		//标准视频编码 
		{eRTP_PT_MP4,  "MPEG4", GS_CODEID_ST_MP4, GS_MEDIA_TYPE_VIDEO, eSTREAM_PKG_Standard },  //标准的 MP4 编码
		{eRTP_PT_MP4,"MP4V-ES", GS_CODEID_ST_MP4, GS_MEDIA_TYPE_PICTURE, eSTREAM_PKG_Standard },
		{eRTP_PT_MP4,"MP4V", GS_CODEID_ST_MP4},
		{eRTP_PT_H264, "H264",GS_CODEID_ST_H264, GS_MEDIA_TYPE_VIDEO, eSTREAM_PKG_Standard },  //标准的 H264 编码

		//标准图像编码
		{eRTP_PT_JPEG, "JPG", GS_CODEID_ST_JPEG, GS_MEDIA_TYPE_PICTURE , eSTREAM_PKG_Standard},
		{eRTP_PT_JPEG, "JPEG", GS_CODEID_ST_JPEG, GS_MEDIA_TYPE_PICTURE , eSTREAM_PKG_Standard }, //标准JPG
		{eRTP_PT_GXX, "BMP",  GS_CODEID_ST_BMP, GS_MEDIA_TYPE_PICTURE, eSTREAM_PKG_Standard },     //标准BMP
		{eRTP_PT_GXX, "YUV420P", GS_CODEID_ST_YUV420P, GS_MEDIA_TYPE_PICTURE , eSTREAM_PKG_Standard},  //标准YUV420P
	
	
		//=====高新新兴
		{eRTP_PT_GXX,"GS-V2160I", GS_CODEID_GS_V2160I, GS_MEDIA_TYPE_VIDEO, eSTREAM_PKG_GS461C },   // GOSUN 2160I 视频编码
		{eRTP_PT_GXX,"GS-A2160I", GS_CODEID_GS_A2160I, eSTREAM_PKG_GS461C },  // GOSUN 2160I 音频编码
		{eRTP_PT_GXX,"GS-V462C", GS_CODEID_GS_V462C , eSTREAM_PKG_GS461C},  //GOSUN BASS462C 视频编码
		{eRTP_PT_GXX, "GS-A462C",GS_CODEID_GS_A462C, eSTREAM_PKG_GS461C},  //GOSUN BASS462C 音频编码
		{eRTP_PT_GXX,"GS-V461C", GS_CODEID_GS_V461C  ,eSTREAM_PKG_GS461C},  //GOSUN BASS461C 视频编码
		{eRTP_PT_GXX,"GS-A461C", GS_CODEID_GS_A461C, eSTREAM_PKG_GS461C },  //GOSUN BASS461C 音频编码
		{eRTP_PT_GXX,"GS-V461A", GS_CODEID_GS_V461A , eSTREAM_PKG_GS461C },  //GOSUN BASS461A 视频编码
		{eRTP_PT_GXX,"GS-V2160IV", GS_CODEID_GS_V2160IV, eSTREAM_PKG_GS461C },// GOSUN 2160I_V 视频编码
		{eRTP_PT_GXX, "GS-V2160IV", GS_CODEID_GS_A2160IV, eSTREAM_PKG_GS461C }, // GOSUN 2160I_V 音频编码



		//===== 海狮 
		{eRTP_PT_GXX, "HI-VIDEO",GS_CODEID_HI_VDEFAULT, eSTREAM_PKG_GS461C, eSTREAM_PKG_Hi },   //海狮的视频通用编码
		{eRTP_PT_GXX, "HI-AUDIO",GS_CODEID_HI_ADEFAULT, GS_MEDIA_TYPE_AUDIO, eSTREAM_PKG_Hi },                         //海狮的音频通用编码
		{eRTP_PT_GXX, "HI-VH264",GS_CODEID_HI_VH264, GS_MEDIA_TYPE_VIDEO, eSTREAM_PKG_Hi },                     //海狮 H264
		{eRTP_PT_GXX, "HI-VMPEG4",GS_CODEID_HI_VMP4, GS_MEDIA_TYPE_VIDEO, eSTREAM_PKG_Hi },                     //海狮 H264
		{eRTP_PT_GXX, "HI",GS_CODEID_HI_COMPLEX, GS_MEDIA_TYPE_VIDEO, eSTREAM_PKG_Hi },                 //海狮复合通用编码


		//===== 海康
		{eRTP_PT_GXX, "HIK-VIDEO",GS_CODEID_HK_VDEFAULT, GS_MEDIA_TYPE_VIDEO, eSTREAM_PKG_HiKVS },  //海康的视频通用编码
		{eRTP_PT_GXX, "HIK-AUDIO",GS_CODEID_HK_ADEFAULT, GS_MEDIA_TYPE_AUDIO , eSTREAM_PKG_HiKVS},  //海康的音频通用编码
		{eRTP_PT_GXX, "HIK-VMPEG4",GS_CODEID_HK_VMP4, GS_MEDIA_TYPE_VIDEO , eSTREAM_PKG_HiKVS},     //海康 MP4
		{eRTP_PT_GXX, "HIK-VH264",GS_CODEID_HK_VH264, GS_MEDIA_TYPE_VIDEO , eSTREAM_PKG_HiKVS},    //海康 H264
		{eRTP_PT_HIK, "HIKVSION",GS_CODEID_HK_COMPLEX, GS_MEDIA_TYPE_VIDEO , eSTREAM_PKG_HiKVS},    //海康复合通用编码

		//===== 大华
		{eRTP_PT_GXX, "DH-VIDEO",GS_CODEID_DH_VDEFAULT, GS_MEDIA_TYPE_VIDEO , eSTREAM_PKG_DaHua},   //大华的视频通用编码
		{eRTP_PT_GXX, "DH-AUDIO",GS_CODEID_DH_ADEFAULT, GS_MEDIA_TYPE_AUDIO , eSTREAM_PKG_DaHua},   //大华的音频通用编码
		{eRTP_PT_GXX, "DH-VMPEG4",GS_CODEID_DH_VMP4, GS_MEDIA_TYPE_VIDEO, eSTREAM_PKG_DaHua },     //大华 MP4
		{eRTP_PT_GXX, "DH-VH264",GS_CODEID_DH_VH264, GS_MEDIA_TYPE_VIDEO , eSTREAM_PKG_DaHua},     //大华 H264
		{eRTP_PT_GXX, "DAHUA",GS_CODEID_DH_COMPLEX, GS_MEDIA_TYPE_VIDEO, eSTREAM_PKG_DaHua },    //大华复合通用编码


		//===== 恒益
		{eRTP_PT_GXX, "HY-VIDEO",GS_CODEID_HY_VDEFAULT , GS_MEDIA_TYPE_VIDEO, eSTREAM_PKG_HengYi}, //恒益的视频通用编码
		{eRTP_PT_GXX, "HY-AUDIO",GS_CODEID_HY_ADEFAULT, GS_MEDIA_TYPE_AUDIO, eSTREAM_PKG_HengYi }, //恒益的音频通用编码
		{eRTP_PT_GXX, "HY-VMPEG4",GS_CODEID_HY_VMP4, GS_MEDIA_TYPE_VIDEO , eSTREAM_PKG_HengYi}, //恒益 MP4
		{eRTP_PT_GXX, "HY-VH264",GS_CODEID_HY_VH264, GS_MEDIA_TYPE_VIDEO , eSTREAM_PKG_HengYi}, //恒益 H264
		{eRTP_PT_GXX, "HY-PSAV",GS_CODEID_HY_COMPLEX, GS_MEDIA_TYPE_VIDEO, eSTREAM_PKG_HengYi }, //恒益复合通用编码


		//=====手机
		{eRTP_PT_GXX, "WM-XVID",GS_CODEID_WM_XVID , GS_MEDIA_TYPE_VIDEO, eSTREAM_PKG_Standard },//手机客户端解码1,谭志添加测试

		// ===== 黄河
		{eRTP_PT_GXX, "HH-VIDEO" ,GS_COIDEID_HH_VDEFULT , GS_MEDIA_TYPE_VIDEO, eSTREAM_PKG_HuangHe  },    //黄河的视频通用编码

		// ===== 中本
		{eRTP_PT_GXX, "ZBEN-VIDEO", GS_CODEID_ZBEN_VDEFULT , GS_MEDIA_TYPE_VIDEO, eSTREAM_PKG_ZBeng }, //中本的视频编码
		{eRTP_PT_GXX, "ZBEN-AUDIO", GS_CODEID_ZBEN_ADEFULT, GS_MEDIA_TYPE_AUDIO , eSTREAM_PKG_ZBeng}, //中本的音频编码

		// ===== 高凯视
		{eRTP_PT_GXX, "CLS-VIDEO", GS_CODEID_CALSYS_VDEFULT , GS_MEDIA_TYPE_VIDEO, eSTREAM_PKG_CLS }, //高凯视的视频通用编码 
		{eRTP_PT_GXX, "CLS-AUDIO", GS_CODEID_CALSYS_ADEFULT, GS_MEDIA_TYPE_AUDIO, eSTREAM_PKG_CLS  }, //高凯视的音频通用编码

		// ==== 安联锐视
		{eRTP_PT_GXX, "ALRS-VIDEO", GS_COIDEID_ALRS_VDEFULT , GS_MEDIA_TYPE_VIDEO, eSTREAM_PKG_ALRS }, //安联锐视的视频通用编码

		// ====  讯美
		{eRTP_PT_GXX, "XM-VIDEO", GS_CODEID_XM_VDEFULT , GS_MEDIA_TYPE_VIDEO, eSTREAM_PKG_XM }, //讯美视频通用编码

		// ====  高凯视GS76xx系列
		{eRTP_PT_GXX, "GS76X-VIDEO", GS_CODEID_CALSYSGS76XX_VDEFULT  , GS_MEDIA_TYPE_VIDEO, eSTREAM_PKG_CLS  }, //高凯视GS76XX的视频通用编码 	

		// ====  C3M通用编码
		{eRTP_PT_C3MVIDEO, "GSC3M-VIDEO", GS_CODEID_GSC3MVIDEO, 
				GS_MEDIA_TYPE_VIDEO, eSTREAM_PKG_GSC3MVIDEO  }, //高凯视GS76XX的视频通用编码 	

		
	};


	

	
	EnumGSCodeID GetGsCodeId4RtpPtName(const CGSString &strRtpPtName)
	{
		for( int i = 0; i<ARRARY_SIZE(s_vRtpPtDs); i++ )
		{
			if( GSStrUtil::EqualsIgnoreCase(s_vRtpPtDs[i].czName,strRtpPtName) )
			{
				return (EnumGSCodeID)s_vRtpPtDs[i].iV2;
			}
		}
		return (EnumGSCodeID)s_vRtpPtDs[0].iV2;
	}

	EnumRTPPayloadType MakeRtpPtInfo4GsCodeId(const EnumGSCodeID eCodeId, CGSString &strRtpPtName)
	{
		for( int i = 0; i<ARRARY_SIZE(s_vRtpPtDs); i++ )
		{
			if( s_vRtpPtDs[i].iV2 == (int)eCodeId )
			{
				strRtpPtName = s_vRtpPtDs[i].czName;
				return (EnumRTPPayloadType)s_vRtpPtDs[i].eID;
			}
		}
		strRtpPtName = s_vRtpPtDs[0].czName;
		return (EnumRTPPayloadType)s_vRtpPtDs[0].eID;
	}



	static StruCmdDescri s_vTrsMdDs[] = 
	{
		{-1, "XXX/XXX", 0, 0},
		{eTRANSPORT_RTP_UDP, "RTP/AVP", 0, 0},
		{eTRANSPORT_RTP_TCP,  "RTP/AVP/TCP", 0, 0},
		{eTRANSPORT_RTP_UDP_MULTICAST,  "RTP/AVP;multicast", 0, 0},
		{eTRANSPORT_RAW_UDP, "RAW/RAW/UDP", 0, 0},

		{eTRANSPORT_RAW_UDP, "MP2T/H2221/UDP", 0, 0},
		{eTRANSPORT_RTP_UDP, "RTP/AVP;unicast", 0, 0},
		{eTRANSPORT_RTP_UDP, "RTP/AVP/UDP;unicast", 0, 0},
		{eTRANSPORT_RTP_UDP_MULTICAST,  "RTP/AVP/UDP;multicast", 0, 0},
		
	};


	const char *TransportModeI2Name(EnumTransportMode eType)
	{
		for( int i = 1; i<ARRARY_SIZE(s_vTrsMdDs); i++ )
		{
			if( s_vTrsMdDs[i].eID == (int)eType )
			{
				return s_vTrsMdDs[i].czName;
			}
		}
		return s_vTrsMdDs[0].czName;
	}

	EnumTransportMode TransportModeName2I( const char *czName)
	{
		for( int i = 0; i<ARRARY_SIZE(s_vTrsMdDs); i++ )
		{
			if( GSStrUtil::EqualsIgnoreCase(s_vTrsMdDs[i].czName,GSStrUtil::Trim(czName)) )
			{
				return (EnumTransportMode)s_vTrsMdDs[i].eID;
			}
		}
		return (EnumTransportMode)s_vRtpPtDs[0].eID;
	}


	static StruCmdDescri s_vMediaTypeDs[] = 
	{
		{GS_MEDIA_TYPE_NONE, "none", 0, 0},
		{GS_MEDIA_TYPE_VIDEO, "video", 0, 0},
		{GS_MEDIA_TYPE_AUDIO,  "audio", 0, 0},
		{GS_MEDIA_TYPE_PICTURE,  "picture", 0, 0},
		{GS_MEDIA_TYPE_OSD, "osd", 0, 0},

		{GS_MEDIA_TYPE_BINARY, "binary", 0, 0},
		{GS_MEDIA_TYPE_PROGRESS, "progress", 0, 0},
		{GS_MEDIA_TYPE_RECORD_INFO, "info", 0, 0},
		{GS_MEDIA_TYPE_SYSHEADER,  "sysheader", 0, 0},

	};

	EnumGSMediaType MediaTypeName2I( const CGSString &strName )

	{
		for( int i = 0; i<ARRARY_SIZE(s_vMediaTypeDs); i++ )
		{
			if( GSStrUtil::EqualsIgnoreCase(s_vMediaTypeDs[i].czName,GSStrUtil::Trim(strName) ) )
			{
				return (EnumGSMediaType)s_vMediaTypeDs[i].eID;
			}
		}
		return (EnumGSMediaType)s_vMediaTypeDs[0].eID;
	}

		const char *MediaTypeI2Name( EnumGSMediaType eType )
	{
		for( int i = 1; i<ARRARY_SIZE(s_vMediaTypeDs); i++ )
		{
			if( s_vMediaTypeDs[i].eID == (int)eType )
			{
				return s_vMediaTypeDs[i].czName;
			}
		}
		return s_vMediaTypeDs[0].czName;
	}

	


		



		//是否允许PS 封装
		BOOL GsCodeIDTestPT_PS(EnumGSCodeID eCodeId )
		{
			return TRUE;
			switch(eCodeId)
			{
			
			case GS_CODEID_HK_VDEFAULT :
			case GS_CODEID_HK_ADEFAULT :
			case GS_CODEID_HK_VMP4 :
			case GS_CODEID_HK_VH264 :
			case GS_CODEID_HK_COMPLEX :
				return FALSE;
				break;
			default :
			break;

			}
			return TRUE;
		}
		//是否允许H264 封装
		BOOL GsCodeIDTestPT_H264(EnumGSCodeID eCodeId )
		{
			
			switch(eCodeId)
			{
			case GS_CODEID_DH_ADEFAULT:
			case GS_CODEID_DH_VDEFAULT:
			case GS_CODEID_DH_VH264:
			case GS_CODEID_DH_COMPLEX:
			case GS_CODEID_PS :
			case GS_CODEID_ST_H264 :	
			case GS_CODEID_GS_V2160IV :
			case GS_CODEID_GS_V2160I :
				return TRUE;
				break;
			default :
				break;

			}
			return FALSE;
		}
		//是否允许MPEG4 封装
		BOOL GsCodeIDTestPT_MP4(EnumGSCodeID eCodeId )
		{
				
				switch(eCodeId)
				{
				case GS_CODEID_PS :
				case GS_CODEID_ST_MP4 :		
				case GS_CODEID_GS_V461C :
					return TRUE;
					break;
				default :
					break;
				}
				return FALSE;

		}




		UINT64 GetNTPTime(void)
		{
#define NTP_OFFSET 2208988800ULL
#define NTP_OFFSET_US (NTP_OFFSET * 1000000ULL)
			//  GSP_PRINTF("@@ %lld\n", OSMilliseconds() );
			return COSThread::Milliseconds()*1000+NTP_OFFSET_US;
		}





	
			







} //end namespace RTP

} //end namespace GSP
