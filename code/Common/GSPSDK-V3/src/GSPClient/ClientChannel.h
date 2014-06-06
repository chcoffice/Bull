/*
******************************************
Copyright (C), 2010-2011, GOSUN
Filename : CLIENTCHANNEL.H
Author :  zouyx
Version : 0.1.0.0
Date: 2010/7/14 14:36
Description: 客户端通道的实现
********************************************
*/
#ifndef GSP_CLIENTCHANNEL_DEF_H
#define GSP_CLIENTCHANNEL_DEF_H
#include "IGSPClient.h"
#include "GSPObject.h"
#include "Uri.h"
#include "MediaInfo.h"
#include "MainLoop.h"
#include "CircleQueue.h"

namespace GSP
{

#define CLICHN_KEEPALIVE_TIMER_ID 1 //定时器 keepalive 
// #define CLICHN_ACTIVE_TIMER_ID  2
// #define CLICHN_CLOSE_TIMER_ID  3

class CClientSection;
class CIProChannel;
class CLog;
class CFrameCache;


class CClientChannel : public CGSPObject,  public CIClientChannel
{

public :
	//信号量定义
	typedef enum
	{
		eSIG_DISCONNECT, //网络端口
		eSIG_ASSERT, //异常
		eSIG_REMOTE_DISCONNECT ,//对端关闭
		eSIG_STREAM_FINISH, //播放结束
		eSIG_REMOTE_CLOSE, //对端关闭, 将会等待数据发送完成
		eSIG_REMOTE_ASSERT, //对端异常关闭
	}EnumSignal;
public:
	const UINT32 m_iAutoID;  //对象自增增长ID， 调试使用
	CClientSection *m_pSection; //所属的Section 管理者
	CIClientChannel::EnumErrno m_eErrno; //错误号
	CGSPString m_strURI; //请求打开的URI
	CUri m_csURI; //URI 分析器
	EnumProtocol m_eProtocol; //使用的协议
	
	CMediaInfo m_csMediaInfo; //通道的媒体信息描述
	CIClientChannel::EnumStatus m_eStatus; //通道状态
	INT32 m_iCtrlAbilities; //支持的控制命令
	StruPlayStatus m_stPlayStatus; //播放状态
	CLog *m_pLog; //日志对象
	INT32 m_eTranModel; //传输模式
	CIClientChannel::StruChannelInfo m_stChannelInfo; //通道信息
	long m_iDebugListTv; //丢帧消息计数
	INT m_VSeq[GSP_MAX_MEDIA_CHANNELS]; //个通道的Seq 计数
	EnumGSMediaType m_vMediaType[GSP_MAX_MEDIA_CHANNELS]; //个通道的媒体类型，方便快速读取



	CGSString m_strSdp;
	BOOL m_bFinish;
	

	BOOL m_bEnableRtp;
	BOOL m_bStep;
private :
	CGSMutex m_csMutex; //对象同步
    static GSAtomicInter s_iAutoIDSequence;   //自增长序列计数
	CIProChannel *m_pProChannel; //当前使用的协议通道对象
	void *m_pUserPrivateData; //用户数据
	BOOL m_bOpenning; //当前是否正在执行打开命令
	CGSPThreadPool m_csTaskPool; //数据处理线程

	INT m_iMaxKeepaliveStreamTimeouts; //最大超时时间 , 秒
	INT m_iKeepaliveStreamPlugs;  //keepalive 心跳计数

	CWatchTimer m_csKeepaliveStreamTimer; //断流检测


 	CGSMutex m_csCacheMutex; //缓冲安全锁
	CCircleQueue<CFrameCache*> m_csStreameQueue;  //流缓存
	CCircleQueue<StruPlayStatus*> m_csStatusQueue; //状态缓存

	INT m_iFlowCtrl; //当前进行流控

	BOOL m_bClosing; //正在关闭 

	BOOL m_bAcceptEvent; //是否发送消息

	INT m_iOpenTimeouts;
	CGSPThreadPool m_csAsyncOpenTaskPool; //异步打开
	CGSCondEx m_condOpen;
	GSP::EnumErrno m_eOpenErrno;

	INT m_iSpeed; //当前速度

	INT64 m_iWaitFrameSize; //缓冲的数据大小
	CGSMutex m_csMutexCacheSize; // m_iWaitFrameSize 锁

	CFrameCache *m_pSysHeaderCache;
public :
	
	/*
	 *********************************************
	 Function : CClientChannel
	 DateTime : 2012/4/24 10:06
	 Description :  
	 Input :  pSestion 所属的管理对象
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
   CClientChannel(CClientSection *pSestion);
    virtual ~CClientChannel(void);

	/*
	*********************************************************************
	*
	*@brief : 以下是接口实现
	*
	*********************************************************************
	*/
	
	virtual CIClientSection *GetSection(void) const;


	virtual void Release(void);              


	virtual CIClientChannel::EnumErrno GetErrno(void) const;


	virtual BOOL SetURI( const char *czURI);

	virtual BOOL SetURIOfSip( const char *czURI, const char *czSdp );


	virtual const char *GetURI( void ) const;


	virtual BOOL AddRequestInfo(const StruGSMediaDescri *pInfo, INT iLen) ;


	virtual void ClearRequestInfo( EnumGSMediaType eType = GS_MEDIA_TYPE_NONE  ) ;


	


	virtual BOOL Open(INT iTransType, INT iTimeouts = 20000 ) ; 


	virtual void Close(void);



	virtual CIClientChannel::EnumStatus GetStatus(void) const;   


	virtual CIMediaInfo &GetMediaInfo(void);    

	virtual const char *GetSdp(void);


	virtual BOOL Ctrl(const StruGSPCmdCtrl &stCtrl, INT iTimeouts = 5000);

	virtual BOOL CtrlOfManstrsp(const char *czMansrtsp, INT iTimeouts = 5000);


	virtual UINT32 GetCtrlAbilities(void) const;


	virtual BOOL EnableAutoConnect(BOOL bEnable = TRUE);


	virtual BOOL SetReconnectInterval(UINT iSecs);


	virtual BOOL SetReconnectTryMax(UINT iCounts); 


	virtual const char *GetDescri(void) const;


	virtual const StruPlayStatus *GetPlayStatus(void) const;

	virtual const CIClientChannel::StruChannelInfo *GetInfo(void) const;

	virtual void SetUserData(void *pData) ;

	virtual void *GetUserData(void) const;

	 virtual UINT32 GetAutoID(void) const;

	/*
	*********************************************************************
	*
	*@brief : 以下为新增接口
	*
	*********************************************************************
	*/



	/*
	 *********************************************
	 Function : IsOpened
	 DateTime : 2012/4/24 10:06
	 Description :  判断是否已经打开
	 Input :  
	 Output : 
	 Return : TRUE/FALSE
	 Note :   
	 *********************************************
	 */
	INLINE BOOL IsOpened(void) const
	{
		return (m_eStatus==ST_READY || m_eStatus==ST_PLAYING || m_eStatus==ST_PAUSE);			
	}

	/*
	 *********************************************
	 Function : OnProChannelSignalEvent
	 DateTime : 2012/4/24 10:07
	 Description :  处理有协议通道发来的信号
	 Input :  eSignal 信号
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
	void OnProChannelSignalEvent( EnumSignal eSignal );

	/*
	 *********************************************
	 Function : HandleStream
	 DateTime : 2012/4/24 10:08
	 Description :  处理有协议通道发来的媒体数据流
	 Input :  pFrame 数据帧
	 Input :  bSafeThread 调帧保证多线程安全
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
	GSP::EnumErrno HandleStream( CFrameCache *pFrame, BOOL bSafeThread = TRUE );

	/*
	 *********************************************
	 Function : HandlePlayStatus
	 DateTime : 2012/4/24 10:08
	 Description :  处理有协议通道发来的播放状态
	 Input :  stPlayStatus 状态
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
	void HandlePlayStatus(const StruPlayStatus &stPlayStatus);


	/*
	 *********************************************
	 Function : IsInit
	 DateTime : 2012/4/24 10:09
	 Description :  对象是否初始化成功
	 Input :  
	 Output : 
	 Return : TRUE/FLASE
	 Note :   如果初始化不成功， 改对象不能使用
	 *********************************************
	 */
	BOOL IsInit(void); 

	/*
	 *********************************************
	 Function : SendEvent
	 DateTime : 2012/4/24 10:10
	 Description :  发送时间到上传, 参考 EnumGSPClientEventType 定义
	 Input :  eEvtType 事件类型
	 Input : pEventData 事件参数, 参考 EnumGSPClientEventType 定义
	 Input : iEvtDataLen pEventData 的长度
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */
	INLINE INT SendEvent( EnumGSPClientEventType eEvtType,
		void *pEventData = NULL ,  INT iEvtDataLen = 0);

	/*
	 *********************************************
	 Function : CountSeqStep
	 DateTime : 2012/4/24 10:12
	 Description :  计算 两个SEQ 的差值
	 Input :  iCurSeq 当前的Seq
	 Input : iLastSeq 上一个Seq
	 Output : 
	 Return : 返回差值
	 Note :   
	 *********************************************
	 */
	static INT CountSeqStep(INT iCurSeq, INT iLastSeq );


	
	/*
	 *********************************************
	 Function : RefreshMediaInfo
	 DateTime : 2012/4/24 10:13
	 Description :  更新m_vMediaType 数值
	 Input :  
	 Output : 
	 Return : 
	 Note :   
	 *********************************************
	 */

	void RefreshMediaInfo(void);

	//保持流的活动
	void KeepStreamAlive(void)
	{
		m_iKeepaliveStreamPlugs  = 0;
	}

private :
	//m_csTaskPool 回调
	void OnTaskPoolEvent( CObjThreadPool *pTkPool, void *pData );


	//定时器回调
	void OnTimerEvent( CWatchTimer *pTimer );

	
	//m_csAsyncOpenTaskPool 回调
	void OnAsyncOpenTaskPoolEvent( CObjThreadPool *pTkPool, void *pData );

};


};

#endif
