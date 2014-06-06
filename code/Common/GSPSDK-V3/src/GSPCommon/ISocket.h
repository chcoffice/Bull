#ifndef GSS_ISOCKET_DEF_H
#define GSS_ISOCKET_DEF_H

#include "OSSocket.h"
#include "ThreadPool.h"
#include "GSPMemory.h"



namespace GSP
{ 
	class CISocket;
	class CIAsyncIO;
 	class CWinIOCP;
	class CVIOCPSimulator;
	class CSkEvtClient;

	typedef struct _StruAsyncSendEvent
	{
		void *pKey;
		UINT64 iSends;
	}StruAsyncSendEvent;

	typedef enum 
	{
		eEVT_SOCKET_NONE = 0,
		eEVT_SOCKET_ARCV,       /*
								*异步读完成,  
								*参数 pParam = CGSPBuffer *
								*参数 pParamExt = NULL
								*返回 BOOL 类型, 返回FALSE 将停止接收， TRUE 继续接收
								*/

		eEVT_SOCKET_ASEND,      /*
								*异步发送完成
								*参数 pParam = const StruAsyncSendEvent *
								*参数 pParamExt = NULL
								*返回值无用
								*/

		eEVT_SOCKET_ARCVFROM,   /*
								*异步 RcvFrom 完成 ,  
								*参数 pParam = CGSPBuffer *,
								*参数  pParamExt = const StruLenAndSocketAddr *
								*返回 BOOL 类型, 返回FALSE 将停止接收， TRUE 继续接收
								*/
		eEVT_SOCKET_ERR,       /*
							   * 错误
							   * 参数 pParam = CNetError *
							   * 参数  pParamExt = NULL
							   * 返回值 不使用
							   */
								
		eEVT_SOCKET_ACCEPT,    /*
							   * 接收到连接
							   * 参数 pParam = CISocket *
							   * 参数 pParamExt = NULL
							   *返回 BOOL 类型, 如果返回FALSE 将停止接收， TRUE 继续接收
							   */
	   /*
	   注意： 每种事件是单线程返回，但事件间可以是多线程，eEVT_SOCKET_ACCEPT 可以多线程返回 
	   */
	}EnumSocketEvent;



	/*
	********************************************************************
	类注释
	类名    :    COSSocket
	作者    :    zouyx
	创建时间:    2011/11/2 9:11
	类描述  :		系统Socket 函数封装
	*********************************************************************
	*/





	/*
	*********************************************
	ClassName :  CISocket
	DateTime : 2011/7/8 9:46
	Description :
	Note :
	*********************************************
	*/

	typedef void *(CGSPObject::*FuncPtrSocketEvent)(CISocket *pSocket, 
		EnumSocketEvent iEvt,void *pParam, void *pParamExt );


	class CISocket : public COSSocket
	{

	
	//	CGSWRMutex m_csWRMutex;
	private  :         
		friend class CWinIOCP;
		friend class CVIOCPSimulator;
		friend class CSkEvtClient;

		CGSPObject *m_pEventObject;
		FuncPtrSocketEvent m_pEventFun;
		CIAsyncIO *m_pAIO;
		void *m_pUserData;
		void (*m_fnSaftFreeEvtPri)(void **);

		
	protected:
		CGSPThreadPool m_csTaskPool; 
		BOOL m_bNoShutdown;
		BOOL m_bRegistered;
		BOOL m_bRcvStart;
		BOOL m_bAcceptStart;

		INT64 m_iUdpLastSendTime;
		INT64 m_iUdpSendBytes;
	public :
		void *m_pEvtPri; //只给 事件线程使用		
	public :
		void SetListener(CGSPObject *pEvtFunOnwer, FuncPtrSocketEvent fnOnEvent);

		virtual EnumErrno AsyncRcvFrom( BOOL bStart ); //当前只有bStart = TRUE
		virtual EnumErrno AsyncRcv( BOOL bStart );     //当前只有bStart = TRUE
		virtual EnumErrno AsyncAccept( BOOL bStart );   
		virtual EnumErrno AsyncSend( void *pKey,  std::vector<CProPacket *> vProPacket );
		virtual EnumErrno AsyncSend( void *pKey,  std::vector<CProFrame *> vProFrame );
		virtual INT SendTo(const CProPacket *pPacket, const StruLenAndSocketAddr *pRemoteAddr );
		virtual INT SendTo(const CProFrame *pFrame, const StruLenAndSocketAddr *pRemoteAddr );


		virtual void Release(void);
		virtual void Disconnect(void); 



		INLINE BOOL IsValid(void) const
		{
			return (m_bConnect && m_hSocket!=INVALID_SOCKET && m_bRegistered );
		}



		INLINE void SetUserData(void *pData)
		{
			m_pUserData = pData;
		}

		INLINE void *GetUserData(void)
		{
			return m_pUserData;
		}

		
	protected :

		CISocket( EnumSocketType eSkType, SOCKET hSocket = INVALID_SOCKET  );
		virtual ~CISocket(void);

		void SetTCPConnectOptions(void);


		virtual BOOL OnAIOEventRcv(CGSPBuffer *pBuffer );

		virtual void OnAIOEventSend(StruAsyncSendEvent &stEvt );

		virtual BOOL OnAIOEventRcvFrom(CGSPBuffer *pBuffer, 
									   const StruLenAndSocketAddr *pRemoteAddr );	

		virtual BOOL OnAIOEventAccept( CISocket *pSocket );

		virtual void OnAIOEventError(CNetError &csError );

		virtual BOOL BindAsyncIO( CNetError &csError );

		INLINE void *SendCallbackEvent(  EnumSocketEvent eEvt,void *pParam, void *ParamExt = NULL )
		{
			if( m_pEventFun )
			{
				return (m_pEventObject->*m_pEventFun)(this, eEvt, pParam, ParamExt);
			}
			return NULL;
		}
	private :
		void OnTaskPoolEvent(CObjThreadPool *pTkPool, void *pEvtPara );

	};


	/*
	*********************************************
	ClassName :   CTCPServerSocket
	DateTime : 2011/6/23 13:48
	Description :
	Note :
	*********************************************
	*/

	class CTCPServerSocket :
		public CISocket
	{

	public :
		static CTCPServerSocket *Create(void)
		{
			return new CTCPServerSocket();
		}



		virtual EnumErrno AsyncSend( CISocket *pPacket )
		{
			GS_ASSERT_RET_VAL( 0,eERRNO_SYS_EOPER );
		}
		virtual EnumErrno AsyncRcv(  BOOL bStart )
		{
			GS_ASSERT_RET_VAL( 0,eERRNO_SYS_EOPER );
		}        

		virtual EnumErrno AsyncRcvFrom(  BOOL bStart )
		{
			GS_ASSERT_RET_VAL( 0,eERRNO_SYS_EOPER );
		}

		virtual EnumErrno AsyncSend( void *pKey,  std::vector<CProPacket *> vProPacket  )
		{
				GS_ASSERT_RET_VAL( 0,eERRNO_SYS_EOPER );
		}

		virtual EnumErrno AsyncSend( void *pKey,  std::vector<CProFrame *> vProFrame  )
		{
				GS_ASSERT_RET_VAL( 0,eERRNO_SYS_EOPER );
		}


		virtual INT SendTo(const CProPacket *pPacket, const StruLenAndSocketAddr *pRemoteAddr )
		{
			GS_ASSERT_RET_VAL( 0,-1 );
		}

		virtual INT SendTo(const CProFrame *pFrame, const StruLenAndSocketAddr *pRemoteAddr )
		{
			GS_ASSERT_RET_VAL( 0,-1 );
		}	

		virtual BOOL BindAsyncIO( CNetError &csError )
		{
			return TRUE;
		}

		virtual EnumErrno AsyncAccept( BOOL bStart ); 

		virtual CNetError Open( INT iPort, const char *czLocalIP);  

		virtual void Disconnect(void);

	protected :
		CTCPServerSocket(void);      
		virtual ~CTCPServerSocket(void);

	private :
		static void ThreadEntry(CGSThread *gsThreadHandle, CTCPServerSocket *pObject);
		void WorkerEntry(void);	
		CGSThread m_csWorkerThread;
		BOOL m_bStarThread;   

		BOOL m_bEnableAccept;
	};
	

	

	/*
	*********************************************
	ClassName : CTCPClientSocket
	DateTime : 2011/6/23 13:49
	Description :
	Note :
	*********************************************
	*/

	class CTCPClientSocket :
		public CISocket
	{


	private :
		CGSString m_strSrvHostName;
	protected :
		CTCPClientSocket(void);
		virtual ~CTCPClientSocket(void);
	public :
		static CTCPClientSocket *Create(void)
		{
			return new CTCPClientSocket();
		}

		INLINE const char *GetServerHostName(void) const 
		{
			return m_strSrvHostName.c_str();
		}

		virtual EnumErrno Connect( UINT16 iSrvPort, const char *czSrvHostName,  UINT16 iLocalPort = 0, const char *czLocalIP = NULL );

		CGSPString GetHostName(void) const;
		INT GetSrvPort(void) const;
	};

	
	
	/*
	*********************************************
	ClassName : CTCPCnnSocket
	DateTime : 2011/6/23 13:49
	Description :
	Note :
	*********************************************
	*/

	class CTCPCnnSocket  : public CISocket
	{
	private :
		CGSPString m_strServerDescri;
	protected :
		CTCPCnnSocket( CISocket *pServer,SOCKET hSocket);
		virtual ~CTCPCnnSocket(void);
	public :
		static  CTCPCnnSocket *Create( CISocket *pServer, SOCKET hSocket, 
					const StruLenAndSocketAddr &stRmote );
		const CGSPString &ServerDescri(void) const
		{
			return m_strServerDescri;
		}
	};



	/*
	*********************************************
	ClassName : CUDPSocket
	DateTime : 2011/6/23 13:49
	Description :
	Note :
	*********************************************
	*/

	class CUDPSocket :
		public CISocket
	{

	protected :
		CUDPSocket(void);
		virtual ~CUDPSocket(void);

		typedef struct _StruRemoteAddr
		{
			StruLenAndSocketAddr stAddr;
			CGSString strHostName;
			int iPort;

			_StruRemoteAddr(void) : stAddr(), strHostName(),iPort(-1)
			{
				stAddr.len = 0;
			}

			bool operator==(const _StruRemoteAddr &stDest ) const
			{
				if( this!=&stDest )
				{
					return (strHostName==stDest.strHostName  && iPort==stDest.iPort );
				}
				return false;
			}
		} StruRemoteAddr;

		StruRemoteAddr m_stRemoteAddr;	
		BOOL m_bMulticast;
	public :
		virtual EnumErrno AsyncSend( void *pKey,  std::vector<CProPacket *> vProPacket  );
		virtual EnumErrno AsyncSend( void *pKey,  std::vector<CProFrame *> vProFrame  );
	

		static CUDPSocket *Create(BOOL bSend, BOOL bRcv);

		//0  不改变默认大小
		void ResetBuffer( UINT iSendBufSize, UINT iRcvBufSize);

		INLINE  const StruLenAndSocketAddr &GetInitRemoteAddr(void)
		{
			return m_stRemoteAddr.stAddr;
		}


		INLINE BOOL HadRemoteAddr(void) const
		{
			return (m_stRemoteAddr.stAddr.len>0 || m_bMulticast);
		}


		EnumErrno InitSocket(  UINT16 iLocalPort = 0, const char *czLocalIP = NULL, 
								BOOL bMulticast = FALSE  );

		EnumErrno InitSocket(  UINT16 iMinLocalPort, UINT16 iMaxLocalPort,
								const char *czLocalIP  = NULL, BOOL bMulticast = FALSE   );

		EnumErrno SetRemoteAddress(  const CGSPString &strRemoteHost, int iPort);

		void ClearRemoteAddress( void );

		EnumErrno JoinMulticast(const CGSPString &strRemoteHost );

		EnumErrno LeaveMulticast(const CGSPString &strRemoteHost );

		EnumErrno    SetTtl(UINT16 timeToLive);

		EnumErrno    SetMulticastInterface(const CGSPString &strLocalIp );


		EnumErrno SendTo(const CProPacket *pPacket);

		EnumErrno SendTo(const CProFrame *pFrame);


		EnumErrno SendTo( const struct iovec *vIO, INT iVecs);
		

	private :
		EnumErrno OpenSocket(BOOL bSend, BOOL bRcv);

	};



} //end GSP

#endif

