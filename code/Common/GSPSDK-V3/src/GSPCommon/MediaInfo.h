#ifndef GSS_MEDIAINFO_DEF_H
#define GSS_MEDIAINFO_DEF_H
/*
******************************************
Copyright (C), 2010-2011, GOSUN
Filename : MEDIAINFO.H
Author :  zouyx
Version : 0.1.0.0
Date: 2010/6/29 15:06
Description: 对媒体类型进行描述，格式化
********************************************
*/
#include <GSMediaDefs.h>

#include "GSPObject.h"
#include "IMediaInfo.h"



namespace GSP
{

	//流封装类型
	typedef enum
	{
		eSTREAM_PKG_NONE = 0,   //无或未知
		eSTREAM_PKG_Standard = 1, //标准码流
		eSTREAM_PKG_GSPS, //标准 GXX PS 流
		eSTREAM_PKG_28181PS, //标准 28181 PS 流
		eSTREAM_PKG_GS461C, //gosun  461C
		eSTREAM_PKG_GS2160I, //gosun  2160I
		eSTREAM_PKG_GS2160IV, //gosun  2160I-V
		eSTREAM_PKG_HuangHe,  //黄河 
		eSTREAM_PKG_DaHua, //大华
		eSTREAM_PKG_HengYi, //恒忆
		eSTREAM_PKG_HiKVS, //海康
		eSTREAM_PKG_ZBeng, //中本
		eSTREAM_PKG_CLS, //高凯视
		eSTREAM_PKG_ALRS, //安联锐视
		eSTREAM_PKG_XM, //讯美
		eSTREAM_PKG_Hi, //海狮
		eSTREAM_PKG_CALSYSGS76XX, //高凯视GS76xx系列	
		eSTREAM_PKG_GSC3MVIDEO, // 高兴新 带 StruGSFrameHeader 头
		eSTREAM_PKG_GSIPC, //高新兴视频编码
	}EnumStreamPackingType;


/*
*********************************************
ClassName :  CMediaInfo
DateTime : 2010/8/6 10:37
Description : 媒体信息类， 为管理和结构化媒体管理信息提供简便接口
Note :
*********************************************
*/
class  CMediaInfo :
    public CGSPObject,public CIMediaInfo
{
private :
	CIMediaInfo::StruMediaChannelInfo m_vChannels[GSP_MAX_MEDIA_CHANNELS];
	UINT m_iChannels;
public:
    CMediaInfo(void);
    CMediaInfo(const CIMediaInfo &csBase );
    virtual ~CMediaInfo(void);

	//接口====
	virtual const CIMediaInfo::StruMediaChannelInfo *GetSubChannel( INT iSubChannel ) const;
	virtual const CIMediaInfo::StruMediaChannelInfo *GetChannel( UINT16 iIndex ) const;
	virtual BOOL GetChannel(UINT16 iIndex, StruGSMediaDescri *&ppResult, INT &iSize) const;
	virtual UINT16 GetChannelNums(void) const;

	virtual EnumGSMediaType GetMediaType( UINT iIndex) const;

	//新增功能
     void Set( const CIMediaInfo &csBase );

     INT AddChannel(const StruGSMediaDescri *pMedia ,INT iChn, const char *szRtpSdpFmtp );




    /*
    *********************************************
    Function :  Clear
    DateTime : 2010/8/6 10:43
    Description : 清除媒体通道
    Input : iChn 指定的媒体通道， 如果为-1将清除所有通道
    Output :
    Return :
    Note :
    *********************************************
    */
     void Clear(INT iChn = -1);

     void Clear( EnumGSMediaType eType);

    /*
    *********************************************
    Function :  Serial2String
    DateTime : 2010/8/6 10:47
    Description :  输出媒体信息描述
    Input :
    Output :   csStr输出的信息
    Return :
    Note :
    *********************************************
    */
    CGSPString Serial2String(void) const; 


    CMediaInfo &operator=(const CMediaInfo &csDest ) ; 

	
   

    INT IsSupport( const  CIMediaInfo *pRequest ) const;

    static INT IsSupport( const  CIMediaInfo *pSrc ,const  CIMediaInfo *pRequest );

    static BOOL CmpMediaDescri(const StruGSMediaDescri *pSrc, const StruGSMediaDescri *pRequest );

	//由 GXX 编码类型 返回 流的封装类型
	static EnumStreamPackingType GetStreamPkt4GsCodeId(EnumGSCodeID eCodeId );

	//由流封装返回一个编码类型
	static EnumGSCodeID TestGsCodeId4StreamPkt(EnumStreamPackingType  ePkgType );


	static UINT8 GetGSPSStreamType4GsCodeId(EnumGSCodeID eCodeId );
	static EnumGSCodeID GetGsCodeId4GSPSStreamType(INT  iType );
};






};

#endif
