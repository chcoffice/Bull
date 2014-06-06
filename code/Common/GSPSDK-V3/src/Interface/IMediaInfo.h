#ifndef GSS_IMEDIAINFO_DEF_H
#define GSS_IMEDIAINFO_DEF_H
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
#include "GSPConfig.h"



namespace GSP
{

/*
*********************************************
ClassName :  CIMediaInfo
DateTime : 2010/8/6 10:37
Description : 媒体信息类， 为管理和结构化媒体管理信息提供简便接口
Note :
*********************************************
*/


class CIMediaInfo   
{   
public :
	static const INT INVALID_CHANNEL_ID = -1; //无效通道号
    typedef struct _StruMediaChannelInfo
    {
        INT iSubChannel; //通道号
        StruGSMediaDescri stDescri; //媒体描述
    }StruMediaChannelInfo;

	/*
	 *********************************************
	 Function : GetSubChannel
	 DateTime : 2012/4/24 10:36
	 Description :  根据通道号获取媒体信息
	 Input :  iSubChannel 通道号
	 Output : 
	 Return :  如果通道不存在返回NULL
	 Note :   
	 *********************************************
	 */
	virtual const CIMediaInfo::StruMediaChannelInfo *GetSubChannel( INT iSubChannel ) const = 0;

	/*
	*********************************************
	Function : GetChannel
	DateTime : 2012/4/24 10:36
	Description :  根据索引号获取媒体信息
	Input :  iIndex 索引号
	Output : 
	Return :  如果索引号对应通道不存在返回NULL
	Note :   
	*********************************************
	*/
    virtual const CIMediaInfo::StruMediaChannelInfo *GetChannel( UINT16 iIndex ) const = 0;

	/*
	*********************************************
	Function : GetChannel
	DateTime : 2012/4/24 10:36
	Description :  根据索引号获取媒体信息
	Input :  iIndex 索引号
	Output : ppResult 返回的媒体信息
	Return :  TRUE/FALSE
	Note :   
	*********************************************
	*/
	virtual BOOL GetChannel(UINT16 iIndex, StruGSMediaDescri *&ppResult, 
							INT &iSize) const = 0;

	/*
	 *********************************************
	 Function : GetChannelNums
	 DateTime : 2012/4/24 10:38
	 Description :  返回总的通道数
	 Input :  
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
    virtual UINT16 GetChannelNums(void) const = 0;


	/*
	*********************************************
	Function :  GetMediaType
	DateTime : 2010/8/6 10:41
	Description : 返回通道的媒体信息类型
	Input :
	Output :
	Return :   返回通道的媒体信息类型, 参考EnumGSMediaType定义 , 如果通道不存在返回 GS_MEDIA_TYPE_NONE
	Note :
	*********************************************
	*/
	virtual EnumGSMediaType GetMediaType( UINT iIndex) const= 0;

	virtual ~CIMediaInfo(void)
	{

	} 

};

} // GSP

#endif
