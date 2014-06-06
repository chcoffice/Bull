/*
******************************************
Copyright (C), 2010-2011, GOSUN
Filename : IGSPCLIENT.H
Author :  zouyx
Version : 0.1.0.0
Date: 2010/7/13 9:46
Description: 客户端接口类
********************************************
*/

#ifndef GSP_IGSPCLIENT_DEF_H
#define GSP_IGSPCLIENT_DEF_H

#include "GSPConfig.h"
#include "IMediaInfo.h"



namespace GSP
{
class CIClientSection;
class CIClientChannel;

/*
 *********************************************
 Function : GSPClientEventFunction
 DateTime : 2010/6/11 17:41
 Description :  客户端回调函数
 Input :  pcsClient 发生事件的会话对象 , 
 Input : pChannel  发生事件的连接通道对象， 对于Section事件 ， 该值为NULL， 具体参考EnumGSPClientEventType
 Input : eEvtType 事件类型 , 参考 EnumGSPClientEventType
 Input : pEventData，   事件参数， 参考 EnumGSPClientEventType
 Input :  iEvtDataLen  pEvtData 长度
  Input :  pUserData 用户数据
 Output :
 Return : 返回值， 参考 EnumGSPClientEventType
 Note :
 *********************************************
 */



 typedef enum
{
    GSP_EVT_CLI_FRAME = 1, //通道事件, pChannel有效， 收到一帧数据 ,        返回值无用，   参数为 CFrame *, 
    GSP_EVT_CLI_STATUS,    //通道事件, pChannel有效， 收到状态报告数据包 , 返回值无用，  参数为 StruPlayStatus *
    GSP_EVT_CLI_TRYFAIL,  //通道事件, pChannel有效， 重联失败，返回值无用 TRUE 将重试，FALSE 将停止重试， 参数为INT ， 表示重试的次数 
    GSP_EVT_CLI_ASSERT_CLOSE, //通道事件, pChannel有效， 客户端异被关闭, 参数为 char * error 错误描述
    GSP_EVT_CLI_CTRL_OK,  //通道事件, pChannel有效， 控制结果成功，  参数为  StruGSPCmdCtrl *
    GSP_EVT_CLI_CTRL_FAIL, //通道事件, pChannel有效， 控制结果失败，  参数为  StruGSPCmdCtrl *

    GSP_EVT_CLI_DESTROY, //通道事件, pChannel有效， 在所属Section 关闭时会发送
    GSP_EVT_CLI_COMPLETE, //通道事件, pChannel有效， 播放、下载结束
    GSP_EVT_CLI_RETREQUEST, //通道事件, pChannel有效，  点流失败, 参数为 BOOL ， TRUE 表示成功， FALSE 表示失败
    GSP_EVT_CLI_LOSTDATA, ////通道事件, pChannel有效, 丢帧事件， 参数为INT 表示丢帧的子通道

	GSP_EVT_CLI_SIP_SEND, ///通道请求， pChannel有效， 发送SIP 数据， 产生为 StruGSPSendSip *, 返回TRUE/FALSE
}EnumGSPClientEventType;

 
typedef struct _StruGSPSendSip
{	
	const void *pSendSipData; // const StruSipData *
	void *pRes; // StruSipData *
	int iTimeouts;
	void *pOutDlgKey; // StruSipData *
}StruGSPSendSip;


typedef INT (*GSPClientEventFunctionPtr)(CIClientSection *pcsClient,CIClientChannel *pChannel, 
                                            EnumGSPClientEventType eEvtType, 
                                            void *pEventData,  INT iEvtDataLen, void *pUserData );


class CIClientChannel
{ 
protected :
      virtual ~CIClientChannel(void)
	  {

	  }
public :
     typedef enum
     {
         ST_INIT,   //正常
         ST_READY,  //准备就绪
         ST_PLAYING, //播放中
         ST_PAUSE,   //暂停
         ST_ASSERT,  //异常
		 ST_WOPEN, //等待打开

     }EnumStatus;

	 typedef struct _StruChannelInfo
	 {
		INT64 iRcvFromNet; //接受的字节数
		INT64 iSendToNet;  //发送的字节数
		INT64 iLostFrames;  //丢失的帧数
		INT64 iLostNetFrames; //没有接受到对端而丢失的数据
	 }StruChannelInfo;

     typedef enum
     {
         ERRNO_SUCCESS = 0,
         ERRNO_EUNKNOWN = 1,
		 ERRNO_EURI,
		 ERRNO_ENPROTOCOL,
         ERRNO_EOFFLINE, //断线
		 ERRNO_ENCTRL,
		 ERRNO_ENFUNC,		 
     }EnumErrno;


	 static const char *StatusString(EnumStatus eStatus)
	 {
		switch(eStatus )
		{
		case ST_INIT :
			{
				static const char *szResult = "Init";
				return szResult;
			}
		break;
		case ST_READY :
			{
				static const char *szResult = "Ready";
				return szResult;
			}
			break;
		case ST_PLAYING :
			{
				static const char *szResult = "Playing";
				return szResult;
			}
			break;
		case ST_PAUSE :
			{
				static const char *szResult = "Pause";
				return szResult;
			}
			break;
		case ST_ASSERT :
			{
				static const char *szResult = "Assert";
				return szResult;
			}
			break;
		case ST_WOPEN :
			{
				static const char *szResult = "WaitOpen";
				return szResult;
			}
			break;			
		}
		static const char *szNone = "Invalid";
		return szNone;
	 }

 
     /*
     *********************************************
     Function : GetSection
     DateTime : 2010/8/6 10:15
     Description :返回所属的Section
     Input :
     Output :
     Return :  返回所属Section对象的指针
     Note :
     *********************************************
     */
     virtual CIClientSection *GetSection(void) const =0;

     /*
     *********************************************
     Function :   Release
     DateTime : 2010/8/6 10:16
     Description : 释放通道对象
     Input :
     Output :
     Return :
     Note :
     *********************************************
     */
     virtual void Release(void) = 0;              
    

     /*
     *********************************************
     Function :GetErrno
     DateTime : 2010/8/6 10:16
     Description : 返回错误号
     Input :
     Output :
     Return :  返回错误号， 参考  CIClientChannel::EnumErrno定义
     Note :
     *********************************************
     */
     virtual CIClientChannel::EnumErrno GetErrno(void) const = 0;


     /*
     *********************************************
     Function :  SetURI/SetURIOfSip
     DateTime : 2010/6/12 8:20
     Description : 设置要连接的URI
     Input :  czURI 要设置的URI字串 
	 Input : czSdp  SDP 描述
     Output : TRUE/FALSE， 如果连接已经打开，设置将失败

     Return :
     Note :
     *********************************************
     */
     virtual BOOL SetURI( const char *czURI) = 0;
	 virtual BOOL SetURIOfSip( const char *czURI, const char *czSdp ) = 0;

     /*
     *********************************************
     Function : GetURI
     DateTime : 2010/6/12 8:21
     Description : 返回连接的URI
     Input :
     Output :
     Return :    返回字符串， 失败返回NULL
     Note :
     *********************************************
     */
     virtual const char *GetURI( void ) const = 0;

     /*
     *********************************************
     Function : AddRequestInfo
     DateTime : 2010/6/12 8:22
     Description : 添加请求流的属性
     Input : pInfo 属性
     Input : iLen 属性的长度 ，兼容需要    
     Output :
     Return : TRUE/FALSE
     Note :   连接打开后，添加属性不会有用, 
     *********************************************
     */
     virtual BOOL AddRequestInfo(const StruGSMediaDescri *pInfo, INT iLen)  = 0;

     /*
     *********************************************
     Function :  ClearRequestInfo
     DateTime : 2010/6/12 8:25
     Description :  清除请求流的属性
     Input :  eType 清除的类型， 如果为 GS_MEDIA_TYPE_NONE 清除全部
     Output :
     Return :
     Note :
     *********************************************
     */
     virtual void ClearRequestInfo( EnumGSMediaType eType = GS_MEDIA_TYPE_NONE  )  = 0;


     /*
     *********************************************
     Function :  Open
     DateTime : 2010/6/12 8:26
     Description : 连接
     Input : iTransType 请求流的传输类型， 参考《GSPStru.h》GSP传输类型定义
     Input : iTimeouts 异步方式和同步超时时间, < 1 表示异步操作， > 0 表示同步方式， 超时时间单位为 毫秒
     Output :
     Return :  TRUE/FALSE 
     Note :  要先设置URI， 如果成功对象将处于 READY
     *********************************************
     */
     virtual BOOL Open(INT iTransType, INT iTimeouts = 20000) = 0; 

     /*
     *********************************************
     Function :  Close
     DateTime : 2010/8/6 10:17
     Description :  关闭当前连接
     Input :
     Output :
     Return :
     Note :  关闭后可以从新打开
     *********************************************
     */
     virtual void Close(void) = 0;


     /*
     *********************************************
     Function :   GetStatus
     DateTime : 2010/8/6 10:18
     Description :  返回通道当前状态
     Input :
     Output :
     Return :  返回通道当前状态  参考 CIClientChannel::EnumStatus 定义 
     Note :
     *********************************************
     */
     virtual CIClientChannel::EnumStatus GetStatus(void) const= 0;   

     /*
     *********************************************
     Function : GetMediaInfo 
     DateTime : 2010/8/6 10:19
     Description :返回数据连接数据源的媒体信息
     Input :
     Output :
     Return : 返回数据连接数据源的媒体信息的对象引用
     Note :   只有打开成的连接， 获取的信息才有意义
     *********************************************
     */
     virtual CIMediaInfo &GetMediaInfo(void) = 0;   

	/*
	 *********************************************
	 Function : GetSdp
	 Version : 1.0.0.1
	 Author : 邹阳星
	 DateTime : 2012/11/7 17:33
	 Description :  获取流的SDP 描述
	 Input :  
	 Output : 
	 Return : 视频返回NULL
	 Note :   
	 *********************************************
	 */
	 virtual const char *GetSdp(void) = 0;

     /*
     *********************************************
     Function : Ctrl
     DateTime : 2010/6/12 8:29
     Description : 播放控制
     Input :  struCtrl 参考 StruGSPCmdCtrl 定义
     Input : iTimeouts 异步方式和同步超时时间, < 1 表示异步操作， > 0 表示同步方式， 超时时间单位为 毫秒
     Output :
     Return :
     Note : 只有在 READY, PLAYING, PAUSE， 状态下有效
     *********************************************
     */
     virtual BOOL Ctrl(const StruGSPCmdCtrl &stCtrl, INT iTimeouts = 5000) = 0;


	  virtual BOOL CtrlOfManstrsp(const char *czMansrtsp, INT iTimeouts = 5000) = 0;

     /*
     *********************************************
     Function :  GetCtrlAbilities
     DateTime : 2010/8/6 10:20
     Description :  返回连接的数据源提供的控制能力
     Input :
     Output :
     Return :返回连接提供的控制能力, 参考 《GSPStru.h》 GSP 控制能力定义
     Note : 只有对有效的控制进行操作才有意义
     *********************************************
     */
     virtual UINT32 GetCtrlAbilities(void) const = 0;

     /*
     *********************************************
     Function : EnableAutoConnect
     DateTime : 2010/6/12 8:43
     Description : 设置是否自动重联
     Input :  bEnable 开停
     Output :
     Return :  TRUE/FALSE
     Note :  默认继承所属的Section的配置
     *********************************************
     */
     virtual BOOL EnableAutoConnect(BOOL bEnable = TRUE) = 0;

     /*
     *********************************************
     Function : SetReconnectInvalid
     DateTime : 2010/6/12 8:43
     Description : 设置重联间隔
     Input :  iSecs 重联将， 单位 秒
     Output :
     Return :  TRUE/FALSE
     Note :  默认继承所属的Section的配置
     *********************************************
     */
     virtual BOOL SetReconnectInterval(UINT iSecs) = 0;

     /*
     *********************************************
     Function : SetReconnectTryMax
     DateTime : 2010/6/12 8:43
     Description : 设置最大重联次数
     Input :  iCounts 重联次数， 超过次数后对象将被销毁
     Output :
     Return :  TRUE/FALSE
     Note :  默认继承所属的Section的配置
     *********************************************
     */
     virtual BOOL SetReconnectTryMax(UINT iCounts) = 0; 

     /*
     *********************************************
     Function :  GetDescri
     DateTime : 2010/6/12 8:46
     Description :  返回客户端得描述
     Input :
     Output :
     Return : 描述的字串
     Note :
     *********************************************
     */
     virtual const char *GetDescri(void) const= 0;

     /*
     *********************************************
     Function :  GetPlayStatus
     DateTime : 2010/8/6 10:22
     Description :  返回当前的播放状态
     Input :
     Output :
     Return : 返回当前的播放状态的对象指针， , 参考 《GSPStru.h》 StruPlayStatus定义
     Note :  当状态改变时， 会通过回调通知， 该功能只是查询最后一次的播放状态
     *********************************************
     */
     virtual const StruPlayStatus *GetPlayStatus(void) const = 0;

	 virtual const StruChannelInfo *GetInfo(void) const = 0;

     virtual void SetUserData(void *pData) = 0;

     virtual void *GetUserData(void) const = 0;


	 virtual UINT32 GetAutoID(void) const = 0;

};

/*
 *********************************************
 Function : FuncPtrFetchClientChannelCallback
 DateTime : 2012/3/28 11:21
 Description :  枚举包含的成员通道回调函数
 Input :  
 Output : pChannel 成员通道
 Output : pUserParam 透传的用户参数
 Return : 返回FALSE 中断， TRUE 继续下一个
 Note :   
 *********************************************
 */
typedef BOOL (*FuncPtrFetchClientChannelCallback)(const CIClientChannel*pChannel,
														void *pUserParam );


class  CIClientSection 
{    
protected :
    virtual ~CIClientSection(void)
	{

	}
public:
	
  
    /*
    *********************************************
    Function : EnableAutoConnect
    DateTime : 2010/6/12 8:43
    Description : 设置客户端是否自动重联
    Input :  bEnable TRUE允许/FALSE禁止
    Output :
    Return :  TRUE/FALSE
    Note : 
    *********************************************
    */    
    virtual BOOL EnableAutoConnect(BOOL bEnable = TRUE) = 0;


    /*
    *********************************************
    Function : SetReconnectInterval
    DateTime : 2010/6/12 8:43
    Description : 设置重联重试的时间间隔
    Input :  iSecs 重联重试的时间间隔， 单位秒
    Output :
    Return :  TRUE/FALSE
    Note :  
    *********************************************
    */
    virtual BOOL SetReconnectInterval(UINT iSecs) = 0;

    /*
    *********************************************
    Function : SetReconnectTryMax
    DateTime : 2010/6/12 8:43
    Description : 设置客户端每次断线后最大重联次数
    Input :  iCounts 重联次数， 超过次数后将不会自动重联
    Output :
    Return :  TRUE/FALSE
    Note : 
    *********************************************
    */
    virtual BOOL SetReconnectTryMax(UINT iCounts) = 0;

    /*
    *********************************************
    Function :  FetchClientChannel
    DateTime : 2010/6/12 8:49
    Description : 枚举客户端列表
    Input :  fnCallback 回调函数， 参考 FuncPtrFetchClientChannelCallback 声明 
    Output : pUserParam 用户参数， 由回调返回
    Return :  -1 出错， 0 成功， 1 表示被回调中断
    Note :
    *********************************************
    */
	virtual INT FetchClientChannel( FuncPtrFetchClientChannelCallback fnCallback,
								void *pUserParam )=0;

     /*
     *********************************************
     Function :  Release
     DateTime : 2010/8/6 10:24
     Description :  释放会话管理对象
     Input :
     Output :
     Return :
     Note :
     *********************************************
     */
    virtual BOOL Release(void) = 0;

    /*
    *********************************************
    Function :  InitLog 
    DateTime : 2010/6/11 16:19
    Description : 设置日志数据接口
    Input :   czPathName 日志的存储路径
    Output :  
    Return :  
    Note :
    *********************************************
    */

	#define GSP_LV_FATAL  0x00000002
	#define GSP_LV_ERROR  0x00000010
	#define GSP_LV_WARN   0x00000020
	#define GSP_LV_DEBUG  0x00000100
	#define GSP_LV_INFO   0x00000080
	#define GSP_LV_NOTICE 0x00000040
#if defined(_DEBUG) || defined(DEBUG)
	#define	GSP_LV_CONSLE (GSP_LV_NOTICE|GSP_LV_FATAL|GSP_LV_ERROR|GSP_LV_WARN|GSP_LV_DEBUG)
	#define	GSP_LV_FILE (GSP_LV_NOTICE|GSP_LV_FATAL|GSP_LV_ERROR|GSP_LV_WARN|GSP_LV_DEBUG)
#else
	#define	GSP_LV_CONSLE GSP_LV_NOTICE //(GSP_LV_NOTICE|GSP_LV_FATAL|GSP_LV_ERROR)
	#define	GSP_LV_FILE (GSP_LV_NOTICE|GSP_LV_FATAL|GSP_LV_ERROR)
#endif

    virtual void InitLog( const char *czPathName, INT lvConsole=GSP_LV_CONSLE, INT lvFile =GSP_LV_FILE ) = 0;

    /*
    *********************************************
    Function : Init
    DateTime : 2010/6/11 16:22
    Description :  启动当前会话
    Input : szIniFilename 配置文件， 等于NULL 将使用默认值
    Output :
    Return : TRUE/FALSE
    Note :
    *********************************************
    */
    virtual BOOL Init( const char *szIniFilename  = NULL) = 0;

    /*
    *********************************************
    Function : CreateChannel
    DateTime : 2010/8/6 10:26
    Description : 新建立连接通道
    Input :
    Output :
    Return :  新建立连接通道的对象指针， 失败返回NULL
    Note :  用户不使用该对象是要Releas
    *********************************************
    */
    virtual CIClientChannel *CreateChannel(void) = 0;

    /*
    *********************************************
    Function : SetEventListener
    DateTime : 2010/8/6 10:28
    Description : 设置事件回调
    Input :  fnCallback 事件回调函数， 参考   GSPClientEventFunctionPtr定义
    Input : pUserParam 透传到回调的用户参数
    Output :
    Return :
    Note : pUserParam只是做指针赋值， 管理有用户进行
    *********************************************
    */
    virtual void SetEventListener( GSPClientEventFunctionPtr fnCallback, void *pUserParam) = 0;


	/*
	 *********************************************
	 Function : OnTransRecvData
	 Version : 1.0.0.1
	 Author : 邹阳星
	 DateTime : 2013/1/15 9:13
	 Description :  再转发模式下，接受到得数据
	 Input :  czProtocol 接受的协议类型
	 Input : pData 接受的数据
	 Input : iDataSize pData 的长度
	 Output : 
	 Return : TRUE/FALSE
	 Note :   
	 *********************************************
	 */
	virtual BOOL OnTransRecvCmdData(const char *czProtocol,const void *pData, int iDataSize ) = 0;


	/*
	*********************************************
	Function : SetGspStreamTransMode
	Version : 1.0.0.1
	Author : 邹阳星
	DateTime : 2013/7/25 17:20
	Description :  设置在使用GSP协议， 视频流的传输方式
	Input :  eGspStreamTransMode 传输方式 参考 GSP 协议流传输方式
	GSP_STREAM_TRANS_MODE_MULTI_TCP (0)   /在GSP信令上复合传输流
	GSP_STREAM_TRANS_MODE_RTP_UDP (1)		RTP_UDP 传输
	GSP_STREAM_TRANS_MODE_TCP (2)			使用独立TCP 传输

	Output : 
	Return : 
	Note : 默认使用 配置文件设定 如果没配置文件 默认为 GSP_STREAM_TRANS_MODE_TCP  
	*********************************************
	*/
	virtual void SetGspStreamTransMode(int eGspStreamTransMode ) = 0;
};




/*
*********************************************************************
*
*@brief : 转封装器
*
*********************************************************************
*/

class CIPkgConverter
{
public :
	typedef struct _StruPkg
	{
		void *pData;
		long iDataSize;		
		EnumGSMediaType eMediaType;
		BOOL isKey;
		unsigned long iTimestamp; // (从1970年1月1日0时0分0秒到此时的秒数) 
	}StruPkg;
	/*
	 *********************************************
	 Function : InputData
	 Version : 1.0.0.1
	 Author : 邹阳星
	 DateTime : 2014/4/21 9:07
	 Description :  输入媒体流
	 Input : pData 媒体流数据, 不包含 StruGSFrameHeader头
	 Input : iSize 媒体流数据长度
	 Output : 
	 Return : TRUE/FALSE
	 Note :   
	 *********************************************
	 */
	virtual BOOL InputData(const void *pData, long iSize ) = 0;

	//返回当前数据, 返回NULL 表示读取完成, 返回的媒体数据， StruGSFrameHeader
	virtual const StruPkg *GetData(void) = 0;
	
	//下一数据
	virtual void  NextData(void) = 0;


	/*
	例子
		
		InputData(....);
		StruPkg *p;
		while( (p=GetData()))
		{
			DoSomething(p);
			NextData();
		}
		
	*/

	virtual ~CIPkgConverter(void)
	{

	}
protected :
	CIPkgConverter(void)
	{

	}

private :
	CIPkgConverter(CIPkgConverter &csDest );
	CIPkgConverter &operator=(CIPkgConverter &csDest );

};

typedef  enum
{
	ePkgType_PS = 1,
	ePkgType_H264 = 2,
	ePkgType_MP4 = 3
} EnumPkgType;

GS_API GSP::CIPkgConverter *CreateGSPPkgConverter( EnumGSCodeID eInputDataCodeID,
												  EnumPkgType ePkgType  );


/*
*********************************************
Function : CreateGSPClientSectionInterface
DateTime : 2010/6/12 8:54
Description :  新建客户端管理接口对象
Input :
Output :
Return :  失败返回NULL
Note :
*********************************************
*/

GS_API GSP::CIClientSection *CreateGSPClientSectionInterface(void);

/*
*********************************************
Function : GspSetMaxCacheMemorySize
Version : 1.0.0.1
Author : 邹阳星
DateTime : 2013/5/14 11:20
Description :  设置缓冲区大小
Input :  iSizeMByte 设定值， 单位 兆 
Output : 
Return : 
Note :   在 CreateGSPServerInterface 和 CreateGSPClientSectionInterface 调用前设定有效
*********************************************
*/
GSP_API  void GspSetMaxCacheMemorySize( UINT iSizeMByte );

}; //end GSP





#endif
