/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : RTSPSERVER.H
Author :  zouyx
Version : 0.0.0.0
Date: 2012/4/18 10:11
Description: RTSP 服务器类
********************************************
*/

#ifndef _GS_H_RTSPSERVER_H_
#define _GS_H_RTSPSERVER_H_


#include "../IProServer.h"
#include "ISocket.h"
#include "ThreadPool.h"

namespace GSP
{
	namespace RTSP
	{
		class CRtspSrvSession;

		class CRtspServer :
			public CIProServer
		{
		public :
			typedef struct _StruRtspServerConfig
			{
				INT iKeepaliveTimeouts;
				INT m_iRtpUdpPortBegin; // rtp udp 端口范围的开始端口， -1 无效
				INT m_iRtpUdpPortEnd;  // rtp udp 端口范围的结束端口， -1 无效

				CGSPString m_strRtspTcpBindIP; //TCP 监听绑定的IP
				INT  m_iRtspTcpPort;  //TCP 监听端口
			}StruRtspServerConfig;

			StruRtspServerConfig m_stConfig;
		private :
			CTCPServerSocket *m_pListenSocket;	
			CGSPThreadPool m_csTaskDestorySession; //异步释放Session 线程
		public:
			CRtspServer(void);
			virtual ~CRtspServer(void);

			/*
			*********************************************************************
			*
			*@brief : 接口实现
			*
			*********************************************************************
			*/
			virtual EnumErrno Init( CServer *pServer );
			virtual void Unint(void);

			virtual EnumErrno Start(void);

			virtual void Stop(void);	


			/*
			*********************************************************************
			*
			*@brief : 增加接口
			*
			*********************************************************************
			*/
			void AsyncDestroySession( CRtspSrvSession *pSession );

		private :
			void *OnTcpListenSocketEvent(CISocket *pSocket, EnumSocketEvent iEvt,void *pParam, void *pParamExt);


			void OnAsyncDelSessionTaskPoolEvent( CObjThreadPool *pTkPool, void *pData );
		};
	} //end namespace RTSP
} //end namespace GSP



#endif //end _GS_H_RTSPSERVER_H_
