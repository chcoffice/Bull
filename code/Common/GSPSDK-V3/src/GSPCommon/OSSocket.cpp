#include "OSSocket.h"
#include <vector>
#include <string>
#include "Log.h"
#include "IAsyncIO.h"
#include "GSPProDebug.h"



#ifdef _WIN32

#include <windows.h>
#include <winsock2.h>
#include <mswsock.h>
#include <process.h>

#include <io.h>
#include <direct.h>
#include <errno.h>

#ifndef _WINCE
#include <Iphlpapi.h>
#pragma   comment(lib,"Iphlpapi.lib")
#endif


#else

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <linux/tcp.h>


//#include <sys/ioctl.h>  
//#include <netinet/in.h>  
#include <net/if.h>  
// #include <net/if_arp.h>  
//#include <arpa/inet.h>  
//#include <errno.h> 

/*

#if __FreeBSD__
#include <ifaddrs.h>
#endif
#include <unistd.h>
#include <sys/utsname.h>

#if __solaris__
#include <sys/sockio.h>
#endif*/



#endif


using namespace GSP;



namespace GSP
{
	typedef struct _StruRouteNode
	{
		CGSString  strRouteIP;
		CGSString  strSubNetIP;
		CGSString  strSubnetMask;
	}StruRouteNode;

static GSAtomicInter s_iInitRefs = 0;
static std::vector<StruIPConfig> s_vIPConfig(0);
static std::vector<StruRouteNode> s_vManualRouteTable(0); //人工路由
};

CGSString CNetError::Error(void)
{
	CGSString stRet;
	GSStrUtil::Format(stRet, _GSTX("err:%s(%d), syserrno:%d, error:%s"),
		GetError(m_eErrno), (int)m_eErrno, m_iSysErrno, m_strError.c_str() );
	return stRet;
}


GSAtomicInter COSSocket::g_iSendTotalByts = 0;
GSAtomicInter COSSocket::g_iRcvTotalBytes = 0;
CGSMutex COSSocket::g_csCountsMutex;

COSSocket::COSSocket( EnumSocketType eSkType, SOCKET hSocket )
:CRefObject()
{
	m_eSockeType = eSkType;
	m_strLocalIP = "";
	m_strRemoteIP = "";
	m_iLocalPort = 0;
	m_iRemotePort = 0;      
	m_strBindLocalIp = "0.0.0.0";
	m_hSocket = hSocket;
	if( m_hSocket==INVALID_SOCKET)
	{
		m_bConnect = FALSE;
	}
	else
	{
		m_bConnect = TRUE;
	}

}

COSSocket::~COSSocket(void)
{
	CloseSocket();

	
}



/* 
*************************************************************************************
brief :    网络模块初始化的实现
*************************************************************************************
*/




void COSSocket::InitModule(void)
{
	if( AtomicInterInc(s_iInitRefs) > 1 )
	{
		return;
	}
#ifdef _WIN32
	WSADATA wsaData;
	INT iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != NO_ERROR)
	{
		GS_ASSERT(0);
		return ;
	}
#endif

	CNetAsyncIOGlobal::InitModule();

	//获取本地IP列表
#if defined(_WIN32) 

#ifdef _WINCE

	int tempSocket = ::WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, 0);
	if (tempSocket != INVALID_SOCKET)
	{          
		int tempSocket = ::socket(AF_INET, SOCK_DGRAM, 0);

#define  kMaxAddrBufferSize 2048
		char inBuffer[kMaxAddrBufferSize];
		char outBuffer[kMaxAddrBufferSize];
		DWORD theReturnedSize = 0;
		UINT iNumIPAddrs = 0;

		// Use the WSAIoctl function call to get a list of IP addresses
		int theErr = ::WSAIoctl(    tempSocket, SIO_GET_INTERFACE_LIST, 
			inBuffer, kMaxAddrBufferSize,
			outBuffer, kMaxAddrBufferSize,
			&theReturnedSize,
			NULL,
			NULL);
		::closesocket(tempSocket);
		if (theErr != 0)
		{			
			GS_ASSERT(0);

			return;
		}

		if( (theReturnedSize % sizeof(INTERFACE_INFO)) != 0 )
		{	
			GS_ASSERT(0);
			return ;
		}

		LPINTERFACE_INFO addrListP = (LPINTERFACE_INFO)&outBuffer[0];
		iNumIPAddrs = theReturnedSize / sizeof(INTERFACE_INFO);
		StruIPConfig stConfig;
		struct sockaddr_in* theAddr;
		for (UINT i = 0;
			i < iNumIPAddrs; i++)
		{
			stConfig.Reset();		
			stConfig.eType = eIP_TYPE_IPV4;
			theAddr = (struct sockaddr_in*)&addrListP[i].iiAddress;
			stConfig.strIPAddr = ::inet_ntoa(theAddr->sin_addr);
			theAddr =  (struct sockaddr_in*)&addrListP[i].iiNetmask;
			stConfig.strSubnetMask = ::inet_ntoa(theAddr->sin_addr);
			s_vIPConfig.push_back(stConfig);
		}

	}
#else  
	//2003 

	PIP_ADAPTER_INFO pAdapterInfo;
	pAdapterInfo = (IP_ADAPTER_INFO *) ::malloc( sizeof(IP_ADAPTER_INFO) );
	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);

	// Make an initial call to GetAdaptersInfo to get
	// the necessary size into the ulOutBufLen variable
	if ( GetAdaptersInfo( pAdapterInfo, &ulOutBufLen) != ERROR_SUCCESS) 
	{
		::free (pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *) ::malloc (ulOutBufLen);
	}

	if ( NO_ERROR == GetAdaptersInfo( pAdapterInfo, &ulOutBufLen) ) 
	{
		PIP_ADAPTER_INFO pAdapter  =  pAdapterInfo;
		StruIPConfig stConfig;
		while (pAdapter) 
		{
			stConfig.Reset();
			stConfig.eType = eIP_TYPE_IPV4;
			stConfig.strIPAddr = pAdapter->IpAddressList.IpAddress.String;
			stConfig.strSubnetMask = pAdapter->IpAddressList.IpMask.String;
			stConfig.strGateway = pAdapter->GatewayList.IpAddress.String;
			pAdapter = pAdapter->Next;
			if( stConfig.strIPAddr.length()>1 && stConfig.strIPAddr!=CGSString("0.0.0.0") )
			{
			  s_vIPConfig.push_back(stConfig);
			}
		}
	}
	::free(pAdapterInfo);
#endif // !_WINCE

#else // !_WIN32


#define  iMaxIFR  500


	struct ifconf ifc;
	bzero(&ifc,sizeof(ifc));
	struct ifreq ifrbuf[iMaxIFR];
	struct ifreq *ifr;

	int tempSocket = ::socket(AF_INET, SOCK_DGRAM, 0);
	if (tempSocket == -1)
	{          
		MY_DEBUG("socket(AF_INET, SOCK_DGRAM, 0) fail.\n");
		GS_ASSERT(0);
		return;
	}

	ifc.ifc_len = sizeof(ifrbuf);
	ifc.ifc_buf = (char*)&ifrbuf[0];


	int err = ::ioctl(tempSocket, SIOCGIFCONF, (char*)&ifc);

	if (err == -1)
	{        
		MY_DEBUG("ioctl(tempSocket, SIOCGIFCONF, (char*)&ifc) fail.\n");
		GS_ASSERT(0);
		return ;
	}
	::close(tempSocket);
	tempSocket = -1;

	int iCnts = ifc.ifc_len/sizeof(struct ifreq);
	if( iCnts>iMaxIFR )
	{
		MY_DEBUG("if flowout : %d.\n", iCnts);
		iCnts = iMaxIFR;

	}

	ifr = ifc.ifc_req;

	UINT addrCount = 0;
	StruIPConfig stConfig;
	struct sockaddr_in* addrPtr;
	for( int i = 0; i<iCnts;  i++, ifr++ )
	{

		if (ifr->ifr_addr.sa_family == AF_INET)
		{
			stConfig.Reset();
			stConfig.eType = eIP_TYPE_IPV4;

			addrPtr = (struct sockaddr_in*)&ifr->ifr_addr;  
			stConfig.strIPAddr = ::inet_ntoa(addrPtr->sin_addr);

			addrPtr = (struct sockaddr_in*)&ifr->ifr_netmask;  
			stConfig.strSubnetMask = ::inet_ntoa(addrPtr->sin_addr);

			s_vIPConfig.push_back();
		}
	}

#endif

	//
	// If LocalHost is the first element in the array, switch it to be the second.
	// The first element is supposed to be the "default" interface on the machine,
	// which should really always be en0.

	if ((s_vIPConfig.size() > 1) && s_vIPConfig[0].strIPAddr == "127.0.0.1" )
	{
		StruIPConfig csTemp;
		csTemp = s_vIPConfig[0];
		s_vIPConfig[0] = s_vIPConfig[1];
		s_vIPConfig[1] = csTemp;
	}

	
}

void COSSocket::UninitModule(void)
{	
	if( AtomicInterDec(s_iInitRefs) > 0 )
	{
		return;
	}
#ifdef _WIN32 
	::WSACleanup();
#endif
	CNetAsyncIOGlobal::UninitModule();
}


INT COSSocket::GetIPConfigCounts(void) 
{

	return s_vIPConfig.size();
}

const StruIPConfig *COSSocket::GetIPConfig( INT iIndex ) 
{

	if( iIndex>-1 && iIndex<(INT)s_vIPConfig.size() )
	{
		return &s_vIPConfig[iIndex];
	}
	return NULL;
}

BOOL  COSSocket::Host2Addr( const char *host, int port, StruLenAndSocketAddr &stAddr )
{
	void *our_s_addr;	// Pointer to sin_addr or sin6_addr


	struct hostent *hp=NULL;
	GS_ASSERT_RET_VAL( host, FALSE );

	stAddr.Reset();
	our_s_addr = (void *) &stAddr.sn.sin.sin_addr;	

#ifdef _WIN32	
	if ( inet_addr(host)==INADDR_NONE ) {
#else
	if( 0==inet_aton(host,(struct in_addr*) our_s_addr) ) {
#endif

#ifdef HAVE_GETHOSTBYNAME2
		hp=(struct hostent*)gethostbyname2( host, AF_INET );
#else
		hp=(struct hostent*)gethostbyname( host );
#endif
		GS_ASSERT_RET_VAL(hp, FALSE);
		memcpy( our_s_addr, (void*)hp->h_addr_list[0], hp->h_length );
	}   else  {
#ifdef _WIN32	
		unsigned long lip = inet_addr(host);
		memcpy( our_s_addr, (void*)&lip, sizeof(lip) );
#endif

	}
	stAddr.sn.sin.sin_family=AF_INET;
	if( port>0 )
	{
		stAddr.sn.sin.sin_port=htons(port);
	}    
	return TRUE;
}

INT COSSocket::AddrToString(const  StruLenAndSocketAddr &stAddr, CGSPString &csHost   )
{
#ifdef _WIN32
	csHost =  inet_ntoa( stAddr.sn.sin.sin_addr);  //inet_ntoa  windows下线程安全， linux 不安全
#else
	char czIP[64];
	int len = 64;
	if(inet_ntop(AF_INET, &stAddr.sn.sin.sin_addr, czIP,len  ) )
	{
		csHost = czIP;
	}
	else
	{
		csHost = "err"
	}
#endif
	return  ntohs(stAddr.sn.sin.sin_port); 

}

BOOL COSSocket::TestConnectOnSubnet( const CGSString &strIp1, const CGSString &strIp2,
									const CGSString &strSubnetMark, EnumIPType eIpType )
{
	GS_ASSERT(eIpType==eIP_TYPE_IPV4 );


	StruLenAndSocketAddr addrIp1,addrIp2, addrSubnet;
	if( Host2Addr(strIp1.c_str() , 0, addrIp1) && 
		Host2Addr(strIp2.c_str(), 0, addrIp2) &&
		Host2Addr(strSubnetMark.c_str(), 0, addrSubnet) )
	{
		unsigned long iIp1, iIp2, iSubnet;
		iIp1 =  *(unsigned long *)&addrIp1.sn.sin.sin_addr;
		iIp2 =  *(unsigned long *)&addrIp2.sn.sin.sin_addr;
		iSubnet =  *(unsigned long *)&addrSubnet.sn.sin.sin_addr;

		if( (iIp1&iSubnet)==(iIp2&iSubnet) )
		{
			return TRUE;
		}
	}
	else
	{
		GS_ASSERT(0);
	}
	return FALSE;
}


CGSString COSSocket::GuessLocalIp( const CGSString &strRemoteIp, const CGSString &strLocalIp )
{
	CGSString str127 =  CGSString("127.0.0.1");
	INT iCounts = COSSocket::GetIPConfigCounts();
	const StruIPConfig *pCfg;
	BOOL bLocalInvalid = (strLocalIp.length()<1 || strLocalIp==CGSString("0") || strLocalIp==CGSString("0.0.0.0"));
	


	if( str127 == strRemoteIp )
	{
		return str127;
	}

	if( !bLocalInvalid )
	{
		return strLocalIp;
	}

	if( strRemoteIp.empty() )
	{
		
		//返回第一个非127.0.0.1 的IP	
		for( int i = 0; i<iCounts; i++ )
		{

			pCfg = COSSocket::GetIPConfig(i);			
			if( pCfg->strIPAddr.length()>1 && pCfg->strIPAddr!=str127 )
			{
				return pCfg->strIPAddr;
			}
		}
		return str127;
	}
	
	//优先使用人工路由表
	for( UINT i = 0; i<s_vManualRouteTable.size(); i++ )
	{
		if( COSSocket::TestConnectOnSubnet(strRemoteIp,
			s_vManualRouteTable[i].strSubNetIP , s_vManualRouteTable[i].strSubnetMask ))
		{
			return s_vManualRouteTable[i].strRouteIP;
		}
	}




	//进行猜测		
	for( int i = 0; i<iCounts; i++ )
	{
		pCfg = COSSocket::GetIPConfig(i);			
		if( pCfg->strIPAddr.length()>1 && 
			pCfg->strSubnetMask.length() >1 &&
			COSSocket::TestConnectOnSubnet(strRemoteIp,pCfg->strIPAddr, pCfg->strSubnetMask ))
		{
			return pCfg->strIPAddr;
		}
	}

	//使用 Connect 猜测


	StruLenAndSocketAddr stAddr;

	int on = 1;

	COSSocket::Host2Addr(strRemoteIp.c_str(), 31111, stAddr);
	
	SOCKET sk = ::socket(AF_INET, SOCK_DGRAM, 0);
	if( sk != INVALID_SOCKET )
	{
		if (0== setsockopt(sk, SOL_SOCKET, SO_BROADCAST,(char*) &on, sizeof(on))) 
		{
			
			if ( 0 == ::connect(sk, &stAddr.sn.sa, stAddr.len ) ) 
			{
				
				stAddr.Reset();
				if ( 0 == getsockname(sk, &stAddr.sn.sa, &stAddr.len )) 
				{					
					CGSString strResult;
					COSSocket::AddrToString(stAddr, strResult);
					if( !strResult.empty() )
					{
						closesocket(sk);
						return strResult;
					}
				}
			}
		}
		closesocket(sk);
		GS_ASSERT(0);
	}
	GS_ASSERT(0);

	//返回第一个非127.0.0.1 的IP	
	for( int i = 0; i<iCounts; i++ )
	{

		pCfg = COSSocket::GetIPConfig(i);			
		if( pCfg->strIPAddr.length()>1 && pCfg->strIPAddr!=str127 )
		{
			return pCfg->strIPAddr;
		}
	}
	GS_ASSERT(0);
	return str127;
}



INT COSSocket::OSSendTo(const struct iovec *vIO, INT iVecs , 
						const StruLenAndSocketAddr *pRemoteAddr)
{

	if( m_hSocket == INVALID_SOCKET )
	{
		return -(INT)eERRNO_NET_ECLOSED; 
	}
	GS_ASSERT(pRemoteAddr);

	INT iRet;
	INT iErrno;
#ifdef _LINUX
	struct msghdr stMsg;
	bzero( &stMsg, sizeof(stMsg) );
	stMsg.msg_name = &pRemoteAddr->sn.sa;
	stMsg.msg_namelen = pRemoteAddr->len;
	stMsg.msg_iov = vIO;
	stMsg.msg_iovlen = iVecs;
#else
	DWORD theBytesSent;
#endif    
	do {
#ifdef _LINUX
		iRet = ::sendmsg(m_hSocket, &stMsg, 0); 
#else
		iRet = ::WSASendTo(m_hSocket, (LPWSABUF)vIO, iVecs, &theBytesSent, 0,
			&pRemoteAddr->sn.sa,pRemoteAddr->len, 
			NULL, NULL);
		if( iRet )
		{
			iRet = -1;
		}
		else
		{
			iRet = theBytesSent;
		}
#endif
	} while(g_bModuleRunning && (iRet == -1) && ( (iErrno =GetSocketErrno()) == WSAEINTR));

	if( iRet < 0 )
	{
		if (iErrno == WSAEWOULDBLOCK)
		{
			iRet = 0;
		}
		else
		{
			MY_LOG_DEBUG(g_pLog, _GSTX("SysSk[%d] SendTo fail. errno: %d\n"), 
				(int)m_hSocket, iErrno);
			iRet = -(INT)(eERRNO_NET_EUNK);

		} 		
	}   
	else
	{
		CountAddSendBytes(iRet);
	}
	return iRet;

}

void COSSocket::Shutdown(void)
{
	m_bConnect = FALSE;
	if( m_hSocket != INVALID_SOCKET)
	{		
		MY_LOG_DEBUG(g_pLog, _GSTX("SysSk[%d] ShowDown.\n"), (int) m_hSocket); 
		int iHow = 2;
#ifdef _WIN32
		iHow = SD_BOTH;
#else
		iHow = SHUT_RDWR;
#endif
		shutdown(m_hSocket, iHow );
	} 
}

void COSSocket::CloseSocket(void)
{
	m_bConnect = FALSE;
	if( m_hSocket != INVALID_SOCKET )
	{

		if( IS_TCP_SOCKET(m_eSockeType))
		{
			//避免进入TIME_WAIT

			BOOL  bDontLinger = FALSE; 
			setsockopt(m_hSocket,SOL_SOCKET,SO_DONTLINGER,(const char*)&bDontLinger,sizeof(BOOL));

			linger stLinger;
			bzero(&stLinger,sizeof(stLinger ) );	
			stLinger.l_onoff = 1;  // (在closesocket()调用,但是还有数据没发送完毕的时候容许逗留)
			stLinger.l_linger = 0; // (容许逗留的时间为0秒)				
			::setsockopt(m_hSocket,SOL_SOCKET,SO_LINGER, 
				(const char*)&stLinger,sizeof(stLinger));

		}
		MY_LOG_DEBUG(g_pLog, _GSTX("SysSk[%d] Close.\n"), (int) m_hSocket);
		closesocket(m_hSocket);
		m_hSocket = INVALID_SOCKET;
	}    

}

const char *COSSocket::GetTypeName( EnumSocketType eType)
{
	struct _StruSkNameInfo
	{
		EnumSocketType eType;
		char *czName;
	};

	static struct _StruSkNameInfo s_vSkNameInfo[] =
	{
		{eSOCKET_TCP, _GSTX("tcp")    },
		{eSOCKET_UDP, _GSTX("udp")     },
		{eSOCKET_CLIENT, _GSTX("client")     },
		{eSOCKET_SERVER, _GSTX("server")     },
		{eSOCKET_CONNECT, _GSTX("connect")     },  

		{eSOCKET_TCP_SERVER, _GSTX("tcp_server")     },
		{eSOCKET_UDP_SERVER, _GSTX("udp_server")     },

		{eSOCKET_TCP_CLIENT, _GSTX("tcp_client")     },
		{eSOCKET_UDP_CLIENT, _GSTX("udp_server")     },

		{eSOCKET_TCP_CONNECT, _GSTX("tcp_connect")     },
		{eSOCKET_UDP_CONNECT, _GSTX("udp_connect")     },
		{ (EnumSocketType)0, _GSTX("Unknown")   },
	};
	int i = 0;
	for(  i = 0; i<ARRARY_SIZE(s_vSkNameInfo); i++  )
	{
		if( s_vSkNameInfo[i].eType==eType )
		{
			break;
		}

	}
	return   s_vSkNameInfo[i].czName;
}


void COSSocket::RefreshLocalSockName(void)
{
	StruLenAndSocketAddr stAddr;
	::getsockname( m_hSocket,&stAddr.sn.sa , &stAddr.len); 
	m_iLocalPort = AddrToString(stAddr, m_strLocalIP);
}

CNetError COSSocket::Socket(void)
{
	CNetError csError(eERRNO_SUCCESS, 0);	

	if( m_hSocket!=INVALID_SOCKET)
	{
		csError.m_eErrno = eERRNO_SYS_ESTATUS;
		csError.m_strError = _GSTX("Socket had exist.");
		GS_ASSERT(0);
		return csError;
	}


	if( IS_TCP_SOCKET(m_eSockeType) )
	{
#ifdef _WIN32

# ifdef GSP_ASYNCIO_USED_IOCP
		m_hSocket = ::WSASocket(   AF_INET, SOCK_STREAM , IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED );
#else
		m_hSocket = ::socket(   AF_INET, SOCK_STREAM, IPPROTO_TCP );
#endif

#else  //linux
		m_hSocket = ::socket(   AF_INET, SOCK_STREAM, 0 );
#endif
	}
	else  if( IS_UDP_SOCKET(m_eSockeType) )
	{
#ifdef _WIN32
#   ifdef GSP_ASYNCIO_USED_IOCP
		m_hSocket = ::WSASocket(   AF_INET, SOCK_DGRAM , IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED );

		DWORD dwBytesReturned = 0;
		BOOL bNewBehavior = FALSE;
		DWORD status;

		// disable  new behavior using
		// IOCTL: SIO_UDP_CONNRESET
		if( m_hSocket!=INVALID_SOCKET) 
		{
			status = WSAIoctl(m_hSocket, SIO_UDP_CONNRESET,
				&bNewBehavior, sizeof(bNewBehavior),
				NULL, 0, &dwBytesReturned,
				NULL, NULL);
		}
#else
		m_hSocket = ::socket(   AF_INET, SOCK_DGRAM, IPPROTO_UDP );
#endif

#else   //linux
		m_hSocket = ::socket(   AF_INET, SOCK_DGRAM, 0 );
#endif

	}
	else
	{

		csError.m_eErrno = eERRNO_SYS_EINVALID;
		csError.m_strError = _GSTX("Socket type not exist.");
		GS_ASSERT(0);
		return csError;
	}
	if( m_hSocket==INVALID_SOCKET )
	{
		csError.m_iSysErrno = GetSocketErrno(); 
		csError.m_eErrno = eERRNO_NET_ESOCKET;
		csError.m_strError = _GSTX("Create sys socket fail.");       
		MY_LOG_ERROR( g_pLog, _GSTX("Create %s Sys Socket fail errno: %d\n"),
			GetTypeName(m_eSockeType),  csError.m_iSysErrno );
		return csError;
	} 
	RefreshLocalSockName(); 	
	m_bConnect = TRUE;
	MY_LOG_DEBUG(g_pLog,_GSTX("SysSk[%d] Create On %s.\n"), (int)m_hSocket, GetTypeName(m_eSockeType));
	return csError;

}

CNetError COSSocket::Bind(UINT16 iPort, const char *czIP)
{
	CNetError csError(eERRNO_SUCCESS, 0);

	if( m_hSocket== INVALID_SOCKET  )
	{
		csError.m_eErrno = eERRNO_NET_ECLOSED;
		csError.m_strError = _GSTX("Socket is invalid.");
		GS_ASSERT(0);
		return csError;
	}
	StruLenAndSocketAddr struAddr;

	struAddr.sn.sin.sin_family = AF_INET;				//如果是IP地址 一直就这个AF_INET
	if ( czIP==NULL || czIP[0] == '0' || czIP[0] == '\0')
	{
		struAddr.sn.sin.sin_addr.s_addr = htonl(INADDR_ANY);

	}
	else
	{
		struAddr.sn.sin.sin_addr.s_addr = inet_addr(czIP);//htonl(INADDR_ANY);//INADDR_ANY表示任意分配给本机的IP都可以使用该套接字 有些机器有多个网卡IP
	}

	if(iPort )
	{
		struAddr.sn.sin.sin_port = htons(iPort);	
	}
	else
	{
		struAddr.sn.sin.sin_port = 0;
	}  

	int iRet = ::bind( m_hSocket, &struAddr.sn.sa, struAddr.len);
	if( iRet )
	{
		csError.m_iSysErrno = GetSocketErrno(); 
		csError.m_eErrno = eERRNO_NET_EOPT;
		csError.m_strError = _GSTX("Bind sys socket fail.");       
		MY_LOG_ERROR(g_pLog, _GSTX("SysSk[%d] Bind %s:%d fail. Errno:%d.\n"),
			(int)m_hSocket, czIP ? "0.0.0.0" : czIP, iPort, csError.m_iSysErrno  );
		return csError;
	}  
	else
	{
		RefreshLocalSockName();
	}
	if( czIP )
	{
		m_strBindLocalIp = czIP;
	}
	else
	{
		m_strBindLocalIp = "0.0.0.0";
	}
	return csError;
}



CNetError COSSocket::NonBlocking( BOOL bEnable )
{
	CNetError csError(eERRNO_SUCCESS, 0);

	if( m_hSocket== INVALID_SOCKET  )
	{
		csError.m_eErrno = eERRNO_NET_ECLOSED;
		csError.m_strError = _GSTX("Socket is invalid.");
		GS_ASSERT(0);
		return csError;
	}
#ifdef _WIN32
	u_long iOne = 0;
	if( bEnable )
	{
		iOne = 1;
	}
	int iErr = ::ioctlsocket(m_hSocket, FIONBIO, &iOne);
#else
	int iFlag = ::fcntl(m_hSocket, F_GETFL, 0);
	if( bEnable )
	{
		iFlag |= O_NONBLOCK;
	}
	else
	{
		iFlag &= ~O_NONBLOCK;
	}
	int iErr = ::fcntl(m_hSocket, F_SETFL, iFlag | O_NONBLOCK);
#endif
	if( iErr )
	{
		csError.m_iSysErrno = GetSocketErrno(); 
		csError.m_eErrno = eERRNO_NET_EOPT;
		csError.m_strError = _GSTX("NonBlocking sys socket fail."); 
		MY_LOG_ERROR(g_pLog, _GSTX("SysSk[%d] NonBlocking(%d) fail. errno:%d.\n"),
			(int)m_hSocket,bEnable, csError.m_iSysErrno);

	}
	return csError;

}

CNetError COSSocket::SetRcvBuffer( UINT32 iSize)
{
	CNetError csError(eERRNO_SUCCESS, 0);

	if( m_hSocket== INVALID_SOCKET  )
	{
		csError.m_eErrno = eERRNO_NET_ECLOSED;
		csError.m_strError = _GSTX("Socket is invalid.");
		GS_ASSERT(0);
		return csError;
	}

	int iBufSize = iSize;
	int iErr = ::setsockopt(m_hSocket, SOL_SOCKET, SO_RCVBUF, (char*)&iBufSize, sizeof(int));
	if( iErr )
	{
		csError.m_iSysErrno = GetSocketErrno(); 
		csError.m_eErrno = eERRNO_NET_EOPT;
		csError.m_strError = _GSTX("SetRcvBuffer sys socket fail."); 
		MY_LOG_ERROR(g_pLog, _GSTX("SysSk[%d] SetRcvBuffer(%d) fail. errno:%d.\n"),
			(int)m_hSocket, (int) iSize, csError.m_iSysErrno);
	}
	return csError;
} 

CNetError COSSocket::SetSendBuffer( UINT32 iSize)
{
	CNetError csError(eERRNO_SUCCESS, 0);

	if( m_hSocket== INVALID_SOCKET  )
	{
		csError.m_eErrno = eERRNO_NET_ECLOSED;
		csError.m_strError = _GSTX("Socket is invalid.");
		GS_ASSERT(0);
		return csError;
	}

	int iBufSize = iSize;
	int iErr = ::setsockopt(m_hSocket, SOL_SOCKET, SO_SNDBUF, (char*)&iBufSize, sizeof(int));
	if( iErr )
	{
		csError.m_iSysErrno = GetSocketErrno(); 
		csError.m_eErrno = eERRNO_NET_EOPT;
		csError.m_strError = _GSTX("SetRcvBuffer sys socket fail."); 
		MY_LOG_ERROR(g_pLog, _GSTX("SysSk[%d] SetSendBuffer(%d) fail. errno:%d.\n"),
			(int)m_hSocket, (int) iSize, csError.m_iSysErrno);
	}
	return csError;
}

CNetError COSSocket::NoDelay(BOOL bEnable )
{
	CNetError csError(eERRNO_SUCCESS, 0);

	if( m_hSocket== INVALID_SOCKET  )
	{
		csError.m_eErrno = eERRNO_NET_ECLOSED;
		csError.m_strError = _GSTX("Socket is invalid.");
		GS_ASSERT(0);
		return csError;
	}
	if( (m_eSockeType&eSOCKET_TCP)!=eSOCKET_TCP )
	{
		csError.m_eErrno = eERRNO_NET_ESOCKET;
		csError.m_strError = _GSTX("Socket is't tcp.");
		GS_ASSERT(0);
		return csError;
	}

	int iOne = bEnable ? 1 : 0;
	int iErr = ::setsockopt(m_hSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&iOne, sizeof(int));
	if( iErr )
	{
		csError.m_iSysErrno = GetSocketErrno(); 
		csError.m_eErrno = eERRNO_NET_EOPT;
		csError.m_strError = _GSTX("NoDelay sys socket fail."); 
		MY_LOG_ERROR(g_pLog, _GSTX("SysSk[%d] NoDelay(%d) fail. errno:%d.\n"),
			(int)m_hSocket,iOne,  csError.m_iSysErrno);
	}
	return csError;
}

CNetError COSSocket::ReuseAddr(void)
{
	CNetError csError(eERRNO_SUCCESS, 0);

	if( m_hSocket== INVALID_SOCKET  )
	{
		csError.m_eErrno = eERRNO_NET_ECLOSED;
		csError.m_strError = _GSTX("Socket is invalid.");
		GS_ASSERT(0);
		return csError;
	}
	int iOne = 1;
	int iErr = ::setsockopt(m_hSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&iOne, sizeof(int));
	if( iErr )
	{
		csError.m_iSysErrno = GetSocketErrno(); 
		csError.m_eErrno = eERRNO_NET_EOPT;
		csError.m_strError = _GSTX("ReuseAddr sys socket fail."); 
		MY_LOG_ERROR(g_pLog, _GSTX("SysSk[%d] ReuseAddr fail. errno:%d.\n"),
			(int)m_hSocket,  csError.m_iSysErrno);
	}
	return csError;
}

CNetError COSSocket::SetLinger(INT iSeconds )
{
	CNetError csError(eERRNO_SUCCESS, 0);

	if( m_hSocket== INVALID_SOCKET  )
	{
		csError.m_eErrno = eERRNO_NET_ECLOSED;
		csError.m_strError = _GSTX("Socket is invalid.");
		GS_ASSERT(0);
		return csError;
	}
	linger stLinger;
	bzero(&stLinger,sizeof(stLinger ) );
	int bOpen  = 0;
	if( iSeconds )
	{
		stLinger.l_onoff = 1;  // (在closesocket()调用,但是还有数据没发送完毕的时候容许逗留)
		stLinger.l_linger = iSeconds; // (容许逗留的时间为0秒)		
	}	
	else
	{
		stLinger.l_onoff = 1;
	}
	int	 iErr;
	 iErr = ::setsockopt(m_hSocket,SOL_SOCKET,SO_LINGER, 
		(const char*)&stLinger,sizeof(stLinger));

	if( iErr )
	{
		csError.m_iSysErrno = GetSocketErrno(); 
		csError.m_eErrno = eERRNO_NET_EOPT;
		csError.m_strError = _GSTX("SetLinger sys socket fail."); 
		MY_LOG_ERROR(g_pLog, _GSTX("SysSk[%d] SetLinger(%d) fail. errno:%d.\n"),
			(int)m_hSocket,iSeconds,  csError.m_iSysErrno);
	}
	return csError;
}

CNetError COSSocket::SetKeepalive( UINT32 iSeconds)
{
	CNetError csError(eERRNO_SUCCESS, 0);

	if( m_hSocket== INVALID_SOCKET  )
	{
		csError.m_eErrno = eERRNO_NET_ECLOSED;
		csError.m_strError = _GSTX("Socket is invalid.");
		GS_ASSERT(0);
		return csError;
	}
	int iOne = iSeconds;
	int iErr = ::setsockopt(m_hSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&iOne, sizeof(int));
	if( iErr )
	{
		csError.m_iSysErrno = GetSocketErrno(); 
		csError.m_eErrno = eERRNO_NET_EOPT;
		csError.m_strError = _GSTX("SetKeepalive sys socket fail."); 
		MY_LOG_ERROR(g_pLog, _GSTX("SysSk[%d] SetKeepalive fail. errno:%d.\n"),
			(int)m_hSocket,  csError.m_iSysErrno);
	}
	return csError;
}

CNetError COSSocket::SetListenCounts(INT iCnts )
{
	GS_ASSERT(iCnts>0);
	CNetError csError(eERRNO_SUCCESS, 0);

	if( m_hSocket== INVALID_SOCKET  )
	{
		csError.m_eErrno = eERRNO_NET_ECLOSED;
		csError.m_strError = _GSTX("Socket is invalid.");
		GS_ASSERT(0);
		return csError;
	}

	int iErr = ::listen(m_hSocket, iCnts);
	if( iErr )
	{
		csError.m_iSysErrno = GetSocketErrno(); 
		csError.m_eErrno = eERRNO_NET_EOPT;
		csError.m_strError = _GSTX("listen sys socket fail."); 
		MY_LOG_ERROR(g_pLog, _GSTX("SysSk[%d] SetListenCounts(%d) fail. errno:%d.\n"),
			(int)m_hSocket,iCnts,  csError.m_iSysErrno);
	}
	return csError;
}

CNetError COSSocket::SetSendLowWait( UINT32 iSize)
{
	CNetError csError(eERRNO_SUCCESS, 0);

	if( m_hSocket== INVALID_SOCKET  )
	{
		csError.m_eErrno = eERRNO_NET_ECLOSED;
		csError.m_strError = _GSTX("Socket is invalid.");
		GS_ASSERT(0);
		return csError;
	}
#ifndef _WIN32
	int iBufSize = iSize;
	int iErr = ::setsockopt(m_hSocket, SOL_SOCKET,SO_SNDLOWAT, (char*)&iBufSize, sizeof(int));
	if( iErr )
	{
		csError.m_iSysErrno = GetSocketErrno(); 
		csError.m_eErrno = eERRNO_NET_EOPT;
		csError.m_strError = _GSTX("SendLowWait sys socket fail."); 
		MY_LOG_ERROR(g_pLog, _GSTX("SysSk[%d] SetSendLowWait(%d) fail. errno:%d.\n"),
			(int)m_hSocket,iSize,  csError.m_iSysErrno);
	}
#endif
	return csError;

}


CNetError  COSSocket::Accept( SOCKET *pHSocket, StruLenAndSocketAddr &stRemoteAddr )
{
	CNetError csError(eERRNO_SUCCESS, 0);

	if( m_hSocket== INVALID_SOCKET  )
	{
		csError.m_eErrno = eERRNO_NET_ECLOSED;
		csError.m_strError = _GSTX("Socket is invalid.");
		GS_ASSERT(0);
		return csError;
	}
	if( (m_eSockeType&eSOCKET_TCP_SERVER) != eSOCKET_TCP_SERVER )
	{
		csError.m_eErrno = eERRNO_SYS_EOPER;
		csError.m_strError = _GSTX("Socket is't tcp server.");
		GS_ASSERT(0);
		return csError;
	}

	stRemoteAddr.Reset();
	SOCKET sk;
	sk = ::accept(m_hSocket, &stRemoteAddr.sn.sa , &stRemoteAddr.len);
	if( sk == INVALID_SOCKET )
	{
		csError.m_iSysErrno = GetSocketErrno(); 
		csError.m_eErrno = eERRNO_NET_EUNK;
		csError.m_strError = _GSTX("accept sys socket fail."); 
		MY_DEBUG("accept fail. errno:%d.\n",  csError.m_iSysErrno  );
	}
	else
	{
		*pHSocket = sk;
	}
	MY_LOG_DEBUG(g_pLog, _GSTX("SysSk[%d] Acccept SysSk[%d].\n"),
		(int)m_hSocket, (int) sk);
	return csError;
}

CNetError COSSocket::OSConnect( UINT16 iSrvPort, const char *czSrvHostName,  
							   UINT16 iLocalPort, const char *czLocalIP) 
{
	CNetError csError(eERRNO_SUCCESS);
	StruLenAndSocketAddr stAddr; 
	int timeout = 5;

	if( m_hSocket==INVALID_SOCKET )
	{
		csError.m_eErrno = eERRNO_NET_ESOCKET;
		csError.m_strError = _GSTX("无效Socket");
		return csError;
	}



	if( IS_UDP_SOCKET(m_eSockeType) )
	{
		timeout = 0;
	}

	if( !Host2Addr(czSrvHostName, iSrvPort, stAddr) )
	{
		csError.m_eErrno = eERRNO_SYS_EINVALID;
		csError.m_strError = _GSTX("无效对端地址");
		MY_LOG_ERROR(g_pLog,  "Host %s:%d to addr fail.\n",
			czSrvHostName, iSrvPort);
		return csError;		
	}

	m_iRemotePort = AddrToString(stAddr, m_strRemoteIP );
	GS_ASSERT(m_iRemotePort==iSrvPort);

	if( iLocalPort>0 || czLocalIP )
	{
		csError = Bind(iLocalPort, czLocalIP );
		if( csError.m_eErrno  )
		{
			MY_LOG_ERROR(g_pLog, _GSTX("SysSk[%d,%s] Connect fail bind %s:%d fail.\n"),
				(int)m_hSocket,GetTypeName(m_eSockeType), 
				czLocalIP ? czLocalIP : "0", iLocalPort);	
			csError.m_eErrno = eERRNO_NET_EBIN;
			return csError;
		}
	}

	if( timeout>0 )
	{
		NonBlocking( TRUE );
	}

	int   iErr = ::connect( m_hSocket, &stAddr.sn.sa , stAddr.len );
	if( iErr )
	{

		int iErrno = 0;
#ifdef _WIN32 
		iErrno  = WSAGetLastError();
		if( iErrno != WSAEINPROGRESS && iErrno != WSAEWOULDBLOCK ) 
		{             
#else
		iErrno = errno;
		if( iErrno!=EINPROGRESS || timeout<1 ) 
		{                
#endif
			csError.m_eErrno = eERRNO_SYS_EBUSY;
			csError.m_strError = _GSTX("connect 失败");
			csError.m_iSysErrno = iErrno;
			MY_LOG_ERROR(g_pLog,  "SysSk[%d,%s]  Connect  %s:%d fail errno:%d.\n",
				(int)m_hSocket,GetTypeName(m_eSockeType),
				czSrvHostName, iSrvPort,iErrno  );			
			return csError;
		}
	}

	if( timeout>0 ) {
#ifdef _WIN32
		fd_set set;
		struct timeval tv;
		tv.tv_sec = timeout;
		tv.tv_usec = 500000;		
		FD_ZERO( &set );
		FD_SET( m_hSocket, &set );       
		iErr = select( m_hSocket+1, NULL, &set, NULL, &tv);   
		if( iErr > 0  && FD_ISSET(m_hSocket, &set ) ) {
			iErr = 0;
		}else {
			iErr = iErr ? -1 : -2 ;
		}	  
#else 
		struct pollfd pollset;
		bzero( &pollset, sizeof(pollset));
		pollset.fd = m_hSocket;
		pollset.events = POLLOUT;
		iErr = ::poll( &pollset, 1, 1000L*timeout );
		if( iErr>0 && pollset.revents&POLLOUT ) {
			iErr = 0;
		}else{
			iErr = iErr ? -1 : -2 ;
		}     
#endif
		if( iErr ) {

			csError.m_eErrno = eERRNO_SYS_ETIMEOUT;
			csError.m_strError = _GSTX("Select 失败");
			csError.m_iSysErrno = 0;
			MY_LOG_ERROR(g_pLog,  "SysSk[%d,%s]  Connect  %s:%d timeout.\n",
				(int)m_hSocket,GetTypeName(m_eSockeType), czSrvHostName, iSrvPort  );			
			return csError;
		}
	}

	if( timeout>0 ) {
		NonBlocking(FALSE);
	}
	// Check if there were any error
	socklen_t err_len = sizeof(int);
	int oerr = -1;
	oerr = -1;
	iErr =  ::getsockopt(m_hSocket,SOL_SOCKET,SO_ERROR, (char *) &oerr,  &err_len);
	if( iErr < 0) {
#ifdef _WIN32
		iErr = WSAGetLastError();
#else
		iErr = errno;
#endif
		csError.m_eErrno = eERRNO_NET_EOPT;
		csError.m_strError = _GSTX("getsockopt 失败");
		csError.m_iSysErrno = iErr;
		MY_LOG_ERROR(g_pLog,  "SysSk[%d,%s]  Connect %s:%d getsockopt fail. errno:%d\n",
			(int)m_hSocket,GetTypeName(m_eSockeType), czSrvHostName, iSrvPort, iErr  );			
		return csError;
	}
	if( oerr ) 
	{
		csError.m_eErrno = eERRNO_NET_EDISCNN;
		csError.m_strError = _GSTX("SO_ERROR 侦测失败");
		csError.m_iSysErrno = oerr;
		MY_LOG_ERROR(g_pLog,  "SysSk[%d, %s]  Connect  %s:%d SO_ERROR err %d fai\n",
			(int)m_hSocket,GetTypeName(m_eSockeType), czSrvHostName, iSrvPort, oerr  );		
	}
	else
	{
		RefreshLocalSockName();
	}
	return csError;
}

void COSSocket::LoadManualRouteTable(const CGSString &strIniFilename )
{
	CConfigFile csCfg;

	StruLenAndSocketAddr stAddr;
	StruRouteNode stNode;

	if( strIniFilename.empty() )
	{
		GS_ASSERT(0);
		return;
	}


	if( !csCfg.LoadFile( (char*) strIniFilename.c_str()) )
	{
		MY_LOG_FATAL(g_pLog, _GSTX("GSP 协议栈加载路由配置文件 '%s' 失败.\n"),
			strIniFilename.c_str() );
	//	GS_ASSERT(0);
		return;
	}
	std::string strIP;
	//获取多个路由 RouteTable1..x
	char szSessionTemp[128];
	for( INT i = 1; ;i++)
	{
		GS_SNPRINTF(szSessionTemp, 100, "RouteTable_%d", i);
		stNode.strRouteIP   = csCfg.GetProperty(szSessionTemp, "RouteIP", "" );
		

		if( stNode.strRouteIP.length() < 1  || 
			!COSSocket::Host2Addr(stNode.strRouteIP.c_str(), 0, stAddr) )
		{
			break;
		}

		stNode.strSubNetIP   = csCfg.GetProperty(szSessionTemp, "SubNetIP", "" );
		stNode.strSubnetMask  = csCfg.GetProperty(szSessionTemp, "SubNetMark", "" );

		if( stNode.strRouteIP.length()>2 && 
			COSSocket::Host2Addr(stNode.strRouteIP.c_str(), 0, stAddr) && 
			stNode.strSubnetMask.length()>2 &&
			COSSocket::Host2Addr(stNode.strSubnetMask.c_str(), 0, stAddr ) )
		{
			s_vManualRouteTable.push_back(stNode);		
		}
	}
}