/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : OSNET.H
Author :  邹阳星
Version : 1.0.0.1
Date: 2012/7/27 9:43
Description: 网络库的操作系统相关项
********************************************
*/

#ifndef _GS_H_OSSOCKET_H_
#define _GS_H_OSSOCKET_H_

#include "GSPObject.h"


#ifdef _WIN32

#include <windows.h>
#include <ws2tcpip.h>
#include <winsock2.h>

#else

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define SOCKET int
#define closesocket  close
#define INVALID_SOCKET  	-1

#define WSAEINTR EINTR
#define WSAEWOULDBLOCK EAGAIN

#endif




namespace GSP
{ 


	typedef enum
	{
		eIP_TYPE_NONE = 0,
		eIP_TYPE_IPV4 = 1,
		eIP_TYPE_IPV6 = 2,
	}EnumIPType;
	
	typedef struct _StruLenAndSocketAddr 
	{
		socklen_t len;
		union {
			struct sockaddr sa;
			struct sockaddr_in sin;
		} sn;
		_StruLenAndSocketAddr(void)
		{
			Reset();
		}

		INLINE socklen_t Size(void)
		{
			return sizeof(sn);
		}
		INLINE void Reset(void)
		{
			len = sizeof(sn);
			bzero(&sn, sizeof(sn) );
		}

		INLINE unsigned long IntegerAddress(void)
		{
			return *(unsigned long*)&sn.sin.sin_addr;
		}
	} StruLenAndSocketAddr;


	typedef struct _StruIPConfig
	{
		EnumIPType eType;
		CGSString strIPAddr;
		CGSString strSubnetMask;
		CGSString strGateway;
		CGSString strDNS;
		_StruIPConfig(void)
		{
			Reset();
		}
		void Reset(void)
		{
			strIPAddr.clear();
			strDNS.clear();
			strSubnetMask.clear();
			strGateway.clear();
		}
	}StruIPConfig;


	class CNetError
	{
	public :
		EnumErrno m_eErrno;    //GSS 错误号
		INT      m_iSysErrno; //系统错误号
		CGSString m_strError;  //错误消息
		CNetError(EnumErrno eErrno = eERRNO_EUNKNOWN, INT iSysErrno = -1, const char *czError = NULL)
		{
			m_eErrno = eErrno;
			m_iSysErrno = iSysErrno;
			if( czError )
			{
				m_strError = czError;
			}
			else
			{
				m_strError = "Unknonw";
			}
		}
		CNetError(const CNetError &csDest )
		{
			*this = csDest;
		}

		CNetError &operator=(const CNetError &csDest )
		{
			if( this!=&csDest )
			{
				m_eErrno = csDest.m_eErrno;
				m_iSysErrno = csDest.m_iSysErrno;
				m_strError = csDest.m_strError;
			}
			return *this;
		}

		CGSString Error(void);
	};


	typedef enum
	{
		eSOCKET_TCP = 0x0001,
		eSOCKET_UDP = 0x0002,

		eSOCKET_CLIENT  = 0x0100,
		eSOCKET_SERVER  = 0x0200,
		eSOCKET_CONNECT = 0x0400,

		eSOCKET_TCP_SERVER = (eSOCKET_TCP|eSOCKET_SERVER),
		eSOCKET_UDP_SERVER = (eSOCKET_UDP|eSOCKET_SERVER),

		eSOCKET_TCP_CLIENT = (eSOCKET_TCP|eSOCKET_CLIENT),        
		eSOCKET_UDP_CLIENT = (eSOCKET_UDP|eSOCKET_CLIENT),

		eSOCKET_TCP_CONNECT = (eSOCKET_TCP|eSOCKET_CONNECT),
		eSOCKET_UDP_CONNECT = (eSOCKET_UDP|eSOCKET_SERVER),

	}EnumSocketType;

#define IS_TCP_SOCKET(x) (((x)&eSOCKET_TCP)==eSOCKET_TCP)
#define IS_UDP_SOCKET(x) (((x)&eSOCKET_UDP)==eSOCKET_UDP) 
#define IS_SERVER_SOCKET(x)  (((x)&eSOCKET_SERVER)==eSOCKET_SERVER)
#define IS_CLIENT_SOCKET(x)  (((x)&eSOCKET_CLIENT)==eSOCKET_CLIENT)
#define IS_CONNECT_SOCKET(x)  (((x)&eSOCKET_CONNECT)==eSOCKET_CONNECT)




	/*
	********************************************************************
	类注释
	类名    :    COSSocket
	作者    :    zouyx
	创建时间:    2011/11/2 9:11
	类描述  :		系统Socket 函数封装
	*********************************************************************
	*/


	class COSSocket : protected CRefObject
	{
	public :
		static CGSMutex g_csCountsMutex;
		static GSAtomicInter g_iSendTotalByts;
		static GSAtomicInter g_iRcvTotalBytes;
	public :
		static INLINE void  CountAddSendBytes( long iSize )
		{
// 			g_csCountsMutex.Lock();
// 			g_iSendTotalByts+=iSize;
// 			g_csCountsMutex.Unlock();

		}
		static INLINE void CountAddRcvBytes( long iSize )
		{
// 			g_csCountsMutex.Lock();
// 			g_iRcvTotalBytes+=iSize;
// 			g_csCountsMutex.Unlock();

		}
		INLINE EnumSocketType SocketType(void) const
		{
			return m_eSockeType;
		}

		INLINE SOCKET GetOSSocket(void) const
		{
			return m_hSocket;
		}

		INLINE CGSString GetDescri(void) const
		{
			CGSString strRet;
			GSStrUtil::Format( strRet, "%s:%d => %s:%d", 
				m_strLocalIP.c_str(), m_iLocalPort,
				m_strRemoteIP.c_str(), m_iRemotePort );
			return strRet;
		}

		//返回本地绑定的IP， 如果没有绑定， 返回 "0.0.0.0"
		INLINE const CGSString &BindLocalIp(void) const
		{
			return m_strBindLocalIp;
		}


		INLINE const char *LocalIP(void) const 
		{

			return m_strLocalIP.c_str();
		}

		INLINE INT LocalPort(void)  const
		{
			return m_iLocalPort;
		}

		INLINE const char *RemoteIP(void) const
		{
			return m_strRemoteIP.c_str();
		}

		INLINE INT RemotePort(void) const
		{
			return m_iRemotePort;
		}

		INLINE BOOL IsInvalidSocket(void) const
		{
			return (m_hSocket==INVALID_SOCKET);
		}

		INLINE BOOL IsConnect(void) const
		{
			return m_bConnect;
		}

		static void InitModule(void);

		static void UninitModule(void);

		static INT GetIPConfigCounts(void);

		static const StruIPConfig *GetIPConfig( INT iIndex );

		static const char *GetTypeName( EnumSocketType eType);

		static BOOL  Host2Addr( const char *host, int port, StruLenAndSocketAddr &stAddr );

		static INT AddrToString(const  StruLenAndSocketAddr &stAddr, CGSPString &csHost   );

		static CGSString GuessLocalIp( const CGSString &strRemoteIp, const CGSString &strLocalIp);

		static BOOL TestConnectOnSubnet( const CGSString &strIp1, const CGSString &strIp2,
			const CGSString &strSubnetMark, EnumIPType eIpType=eIP_TYPE_IPV4 );

		static INLINE INT16 Int16H2N(INT16 iValue )
		{
			return htons(iValue);
		}
		static INLINE INT16 Int16N2H(INT16 iValue )
		{
			return ntohs(iValue);
		}
		static INLINE INT32 Int32H2N(INT32 iValue )
		{
			return htonl(iValue);
		}

		static INLINE INT32 Int32N2H(INT32 iValue )
		{
			return ntohl(iValue);
		}

		static INLINE  int GetSocketErrno(void)
		{
#ifdef _WIN32
			return ::WSAGetLastError();
#else       
			return errno;
#endif
		}
		//加载人工路由表
		static void LoadManualRouteTable(const CGSString &strIniFilename );

	protected :

		COSSocket( EnumSocketType eSkType, SOCKET hSocket = INVALID_SOCKET );
		virtual ~COSSocket(void);
		

		void RefreshLocalSockName(void);

		CNetError Socket(void);

		CNetError Bind(UINT16 iPort, const char *czIP);

		CNetError NonBlocking( BOOL bEnable ); 

		CNetError SetRcvBuffer( UINT32 iSize); 

		CNetError SetSendBuffer( UINT32 iSize);

		CNetError ReuseAddr(void);

		CNetError SetKeepalive( UINT32 iSeconds);

		CNetError SetListenCounts(INT iCnts = 10 );

		CNetError SetSendLowWait( UINT32 iSize);

		CNetError NoDelay(BOOL bEnable );

		CNetError SetLinger(INT iSeconds );

		CNetError OSConnect(UINT16 iSrvPort, const char *czSrvHostName,  
			UINT16 iLocalPort = 0, const char *czLocalIP = NULL );

		void Shutdown(void);

		void CloseSocket(void);

		CNetError  Accept( SOCKET *pHSocket, StruLenAndSocketAddr &stRemoteAddr );


		INT OSSendTo( const struct iovec *vIO, INT iVecs , const StruLenAndSocketAddr *pRemoteAddr);

	protected :
		EnumSocketType m_eSockeType;
		CGSString m_strLocalIP;
		CGSString m_strRemoteIP;
		INT m_iLocalPort;
		INT m_iRemotePort;         
		SOCKET m_hSocket;
		BOOL m_bConnect;
		CGSString m_strBindLocalIp;
	};

	} //end GSP

#endif //end _GS_H_OSNET_H_
