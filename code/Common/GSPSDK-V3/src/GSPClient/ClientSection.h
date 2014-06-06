/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : CLIENTSECTION.H
Author :  zouyx
Version : 0.0.0.0
Date: 2012/3/26 14:58
Description: 实现客户端会话管理
********************************************
*/

#ifndef _GS_H_CLIENTSECTION_H_
#define _GS_H_CLIENTSECTION_H_
#include "GSPObject.h"
#include "IGSPClient.h"
#include "Log.h"
#include "List.h"
#include "SipStack.h"


namespace GSP
{
namespace SIP
{
	class CProSipClientService;
}
	
	
	
	/*
	********************************************************************
	类注释
	类名    :    CClientSection
	作者    :    zouyx
	创建时间:    2012/4/24 10:15
	类描述  :		客户端会话管理
	*********************************************************************
	*/
	
	class CClientSection : public CGSPObject , public CIClientSection
	{

	private :
		CConfigFile m_csConfig; //客户端配置
		void *m_pClientCallbackParam; //回调函数所属的对象
		GSPClientEventFunctionPtr m_fnClientCallback; //回调函数
		CGSWRMutex m_csChannelMutex; // m_csChannelList 的同步锁
		CList m_csChannelList; //通道缓冲 存储的是 CIClientChannel *

		CGSString m_strLogPath; //日志目录
		INT m_lvLogConsole; //日志屏幕输出
		INT m_lvLogFile;  //日志文件输出

		BOOL m_bUseRTP; //是否使用RTP 传输流
		INT m_iRtpUDPPort; //UDP 使用的端口


		CGSString m_strSipUdpBindIp;
		INT m_iSipUdpPort;

		CGSString m_strSipServerName;

		INT m_iRtpUdpPortBegin;
		INT m_iRtpUdpPortEnd;

		INT m_eGspStreamTransMode; // 流传输模式
	public :
		CLog *m_pLog; //日志对象
		BOOL m_bGspEnableRtp;
#ifdef ENABLE_SIP_MODULE
		SIP::CProSipClientService *m_pSipSrv;
#endif
	public:
		
		

		/*
		*********************************************************************
		*
		*@brief : 接口实现
		*
		*********************************************************************
		*/
		virtual BOOL EnableAutoConnect(BOOL bEnable = TRUE);


		virtual BOOL SetReconnectInterval(UINT iSecs);


		virtual BOOL SetReconnectTryMax(UINT iCounts);


		virtual INT FetchClientChannel( FuncPtrFetchClientChannelCallback fnCallback,
			void *pUserParam );


		virtual BOOL Release(void);

		virtual void InitLog( const char *czPathName, 
						INT lvConsole=GSP_LV_CONSLE, INT lvFile =GSP_LV_FILE );


		virtual BOOL Init( const char *szIniFilename );

		
		virtual CIClientChannel *CreateChannel(void);

		
		virtual void SetEventListener( GSPClientEventFunctionPtr fnCallback, 
										void *pUserParam);

		virtual BOOL OnTransRecvCmdData(const char *czProtocol,const void *pData, int iDataSize );

		virtual void SetGspStreamTransMode(int eGspStreamTransMode );

		/*
		*********************************************************************
		*
		*@brief : 增加的接口
		*
		*********************************************************************
		*/

		/*
		 *********************************************
		 Function : Config
		 DateTime : 2012/4/24 10:16
		 Description :  获取配置对象
		 Input :  
		 Output : 
		 Return : 
		 Note :   
		 *********************************************
		 */
		INLINE CConfigFile &Config(void)
		{
			return m_csConfig;
		}


		/*
		 *********************************************
		 Function : SendEvent
		 DateTime : 2012/4/24 10:16
		 Description :  向用户发送事件, 参考GSPClientEventFunctionPtr 说明
		 Input :  
		 Output : 
		 Return : 
		 Note :   
		 *********************************************
		 */
		INT SendEvent(CIClientChannel *pChannel, 
			EnumGSPClientEventType eEvtType, 
			void *pEventData,  INT iEvtDataLen)
		{
			if( m_fnClientCallback  )
			{
				return m_fnClientCallback(this, pChannel, eEvtType, pEventData, iEvtDataLen, m_pClientCallbackParam);
			}
			return 0;
		}
		CClientSection(void);



		/*
		 *********************************************
		 Function : OnClientChannelReleaseEvent
		 DateTime : 2012/4/24 10:17
		 Description :  处理客户端通道对象释放的事件
		 Input :  pChannel  释放的通道
		 Output : 
		 Return : 
		 Note :   
		 *********************************************
		 */
		void OnClientChannelReleaseEvent( CIClientChannel *pChannel);



		INT GetGspStreamTransMode(void) const
		{
			if( m_eGspStreamTransMode>-1)
			{
				return m_eGspStreamTransMode;
			}
			return GSP_STREAM_TRANS_MODE_MULTI_TCP;
		}
	protected :
		~CClientSection(void);

	private :
		//SIP 协议栈回调
		//连接
		static EnumSipErrorCode SipClientConnectCallback( SipServiceHandle hService,
			SipSessionHandle hNewSession );

		//断开
		static void SipClientDisconnectCallback(SipServiceHandle hService, SipSessionHandle hSession);
#ifdef ENABLE_SIP_MODULE
		//收包
		static void   SipSIPPacketCallback(SipServiceHandle hService, 
			SipSessionHandle hSession,
			StruSipData *pData);

		EnumSipErrorCode OnSipClientConnectEvent(SipSessionHandle hNewSession );
		void OnSipClientDisconnectEvent(SipSessionHandle hSession);
		void OnSipPacketEvent(SipSessionHandle hSession, StruSipData *pData);
#endif

	};

} //end namespace GSP

#endif //end _GS_H_CLIENTSECTION_H_
