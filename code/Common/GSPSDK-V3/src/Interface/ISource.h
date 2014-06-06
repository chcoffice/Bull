#ifndef GSP_ISOURCE_DEF_H
#define GSP_ISOURCE_DEF_H  
#include "GSPConfig.h"
#include "IMediaInfo.h"
#include "IUri.h"


/*
******************************************
Copyright (C), 2010-2011, GOSUN
Filename : IGSPSOURCE.H
Author :  zouyx
Version : 0.1.0.0
Date: 2010/6/22 9:25
Description: 数据源接口定义
********************************************
*/

namespace GSP
{




class CISource
{   
public :
	typedef enum
	{
		eMODE_PUSH = 0, //推模式
		eMODE_PULL = 1, //拉模式
	}EnumMode;

    typedef enum
    {
        SRC_RET_SUCCESS = 0,
        SRC_RET_EUNKNOWN,  //未知错误
        SRC_RET_EINVALID,  //无效数据源
        SRC_RET_EFLOWOUT,  //写出缓冲溢出
        SRC_RET_EUNUSED,    //无人使用
    }EnumRetNO;

	typedef enum
	{
		eEVT_RELEASE = 0, //释放, 返回值不使用
		eEVT_REQUEST_FRAME,  //请求数据， 返回值 为 INT 类型
							 //  < 0 表示停止继续拉数据， > 0 表示 下次调用的间隔
							 // 0 表示 连续拉数据， 
							 // * 不要再回调中使用死循环发送数据,如果修养联系发送数据请返回0

	}EnumEvent;

typedef void *(*FuncPtrISourceEventCallback)(CISource *pSource, EnumEvent eEvt, void *pUserArgs);


	virtual CISource::EnumMode GetMode(void) const = 0;

	virtual void SetEventCallback(CISource::FuncPtrISourceEventCallback fnCallback, void *pUserArgs) = 0;

	virtual BOOL EnablePullEvent( BOOL bStart, INT iMSecs ) = 0;


	/*
	 *********************************************
	 Function : RefObject
	 DateTime : 2012/4/24 10:39
	 Description :  增加对象引用
	 Input :  
	 Output : 
	 Return : 返回数据源
	 Note :   
	 *********************************************
	 */
	virtual CISource *RefObject(void) = 0;

	/*
	 *********************************************
	 Function : UnrefObject
	 DateTime : 2012/4/24 10:39
	 Description :  减少对象应用
	 Input :  
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
	virtual void UnrefObject(void) = 0;

	/*
	 *********************************************
	 Function : GetSrcRefs
	 DateTime : 2012/4/24 10:40
	 Description :  返回数据源被引用的个数
	 Input :  
	 Output : 
	 Return : 
	 Note :   不等同以对象的引用数， 表示的是该数据源被多少客户端使用
	 *********************************************
	 */
    virtual UINT GetSrcRefs(void) = 0; 

	/*
	 *********************************************
	 Function : GetAutoID
	 DateTime : 2012/4/24 10:42
	 Description :  返回自增长的ID, 具有唯一性
	 Input :  
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
     virtual UINT32 GetAutoID(void) = 0;

	/*
	 *********************************************
	 Function : GetSourceID
	 DateTime : 2012/4/24 10:42
	 Description :  返回数据源的ID， 回收的ID 可以重复使用
	 Input :  
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
     virtual UINT16 GetSourceID(void) = 0;

	 /*
	  *********************************************
	  Function : SetMediaInfo
	  DateTime : 2012/4/24 10:43
	  Description :  添加设置媒体属性
	  Input :  iChn 通道号
	  Input :  pInfo 媒体属性
	  Output : 
	  Return : 
	  Note :   
	  *********************************************
	  */
     virtual void SetMediaInfo( UINT iChn,const StruGSMediaDescri *pInfo ) = 0;

	 /*
	  *********************************************
	  Function : SetCtrlAbilities
	  DateTime : 2012/4/24 10:44
	  Description :  设置可以提供控制的功能能力
	  Input :  iAbilities 能力描述, 参考 : <<GSPStru.h>>  GSP 控制能力定义 
	  Output : 
	  Return : 
	  Note :   
	  *********************************************
	  */
     virtual void SetCtrlAbilities(UINT32 iAbilities) = 0;

	 /*
	  *********************************************
	  Function : SourceEnableRef
	  DateTime : 2012/4/24 10:45
	  Description :  设置数据源释放同时可以被多人使用
	  Input :  bEnable 使能
	  Output : 
	  Return : 
	  Note :   
	  *********************************************
	  */
     virtual void SourceEnableRef(BOOL bEnable ) = 0;

	/*
	 *********************************************
	 Function : SourceEnableValid
	 DateTime : 2012/4/24 10:46
	 Description :  设置数据源是否有效
	 Input :  bEnable 有效性
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
     virtual void SourceEnableValid( BOOL bEnable) = 0;


	 /*
	  *********************************************
	  Function : MakeURI
	  DateTime : 2012/4/24 10:47
	  Description :  生成数据源的URI
	  Input :  czPro 使用的协议
	  Output : 
	  Return : 失败返回NULL， 返回的对象需要调delete 释放
	  Note :   
	  *********************************************
	  */
     virtual CIUri *MakeURI( const char *czPro = "gsp", const char *pRemoteIP=NULL ) = 0;


	 /*
	  *********************************************
	  Function : SetPlayStatus
	  DateTime : 2012/4/24 10:48
	  Description :  设置播放状态
	  Input :  pStatus 播放状态
	  Output : 
	  Return : 
	  Note :   
	  *********************************************
	  */
     virtual void SetPlayStatus( const StruPlayStatus *pStatus )=0;

	 /*
	  *********************************************
	  Function : ReplayEnd
	  DateTime : 2012/4/24 10:48
	  Description :  回放或下载方式时设置文件结束
	  Input :  
	  Output : 
	  Return : 
	  Note :   
	  *********************************************
	  */
     virtual void ReplayEnd(void) = 0;


	/*
	 *********************************************
	 Function : WriteData
	 DateTime : 2012/4/24 10:50
	 Description :  发送数据
	 Input :  pData 数据
	 Input : iLen  pData数据长度
	 Input : iChn 数据所属通道
	 Input : bKey  是否是关键数据, 该标准用以流控, 建议 I帧， 音频，系统头 设为TRUE
	 Output : 
	 Return : EnumRetNO 参考定义 
	 Note :   
	 *********************************************
	 */
     virtual CISource::EnumRetNO WriteData( const void *pData, INT iLen, UINT iChn, BOOL bKey) = 0; 

	/*
	 *********************************************
	 Function : WriteSysHeader
	 DateTime : 2012/4/24 10:52
	 Description :  写系统头
	 Input :  pData 数据
	 Input : iLen  pData数据长度
	 Input : iChn 数据所属通道
	 Output : 
	 Return : EnumRetNO 参考定义 
	 Note :   
	 *********************************************
	 */
     virtual CISource::EnumRetNO WriteSysHeader( const void *pData, INT iLen, UINT iChn) = 0;

	/*
	 *********************************************
	 Function : WriteDataV
	 DateTime : 2012/4/24 10:53
	 Description :  发送数据， 功能对应于 WriteData
	 Input :  pIOV 发送的数据数组
	 Input :  iVNums 指明 pIOV 的个数
	 Input : iChn 数据所属通道
	 Input : bKey  是否是关键数据, 该标准用以流控, 建议 I帧， 音频，系统头 设为TRUE
	 Output : 
	 Return : 
	 Note :   一次WriteDataV 调用多个 StruIOV将 合并为一帧数据发送
	 *********************************************
	 */
     virtual CISource::EnumRetNO WriteDataV( const StruIOV *pIOV, INT iVNums, UINT iChn, BOOL bKey) = 0; 
	 /*
	 *********************************************
	 Function : WriteDataV
	 DateTime : 2012/4/24 10:53
	 Description :  写系统头， 功能对应于 WriteSysHeader
	 Input :  pIOV 发送的数据数组
	 Input :  iVNums 指明 pIOV 的个数
	 Input : iChn 数据所属通道	
	 Output : 
	 Return : 
	 Note :   一次WriteDataV 调用多个 StruIOV将 合并为一个系统头发送
	 *********************************************
	 */
     virtual CISource::EnumRetNO WriteSysHeaderV( const StruIOV *pIOV, INT iVNums, UINT iChn) = 0;


     /*
     *********************************************
     Function :SetUserData
     DateTime : 2010/8/6 8:56
     Description :设置用户数据
     Input : pData 用户的数据
     Output :
     Return :
     Note : 只是做指针的存储,如果需要释放操作， 由用户自行处理
     *********************************************
     */
     virtual void SetUserData(void *pData) = 0;

     /*
     *********************************************
     Function : GetUserData
     DateTime : 2010/8/6 8:57
     Description : 返回用户数据
     Input :
     Output : 
     Return :  返回  SetUserData 设定的值 
     Note : 如果没有调用过 SetUserData， 默认值为NULL
     *********************************************
     */
     virtual void *GetUserData(void ) = 0;

	 /*
	  *********************************************
	  Function : Release
	  DateTime : 2012/4/24 10:56
	  Description :  释放对象
	  Input :  
	  Output : 
	  Return : 
	  Note :   
	  *********************************************
	  */
     virtual void Release(void) = 0;


	 /*
	  *********************************************
	  Function : GetKey
	  DateTime : 2012/4/24 10:57
	  Description :  返回构建数据源是传递的数据源KEY
				指 CIServer::AddSource 指定的key 值
	  Input :  
	  Output : 
	  Return : 
	  Note :   
	  *********************************************
	  */
      virtual const char *GetKey(void) = 0;

protected :
	virtual ~CISource()
	{
	}
     
    
};


};


#endif
