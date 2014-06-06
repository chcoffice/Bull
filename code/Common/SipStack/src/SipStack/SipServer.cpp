#define WIN32_LEAN_AND_MEAN             

#include "SipServer.h"
#include "Util.h"
#include "eXosip2.h"

#if defined(_WIN32) || defined(WIN32) || defined(_WINCE)

#include <windows.h>
#include <winsock2.h>
#include <mswsock.h>
#include <process.h>


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

#endif




using namespace GSSIP;

GSAtomicInter CSipServer::s_iOSipRefs = 0;
int CSipServer::s_bOSipInit = 0;


#define GXX_TAG_MARK  "_ff34e34e_34e34e-"

#ifdef SIP_INSTANCE_ONE 

#define G_IS_VALID   (s_bOSipInit>0 && m_bStart)
#else

#define G_IS_VALID  (m_hEXosip!=NULL && m_bStart)

#endif

GSAtomicInter CClientInfo::m_iDlgSeq = 0;



//去除 ""
static CGSString TrimUpDot(const CGSString &strVal )
{

	CGSString strRet = GSStrUtil::Trim(strVal);
	if( *(strRet.begin()) ==  '\"' &&  *(strRet.end()-1) == '\"' )
	{
		strRet.erase(strRet.begin());
		strRet.erase(strRet.end()-1);
	}
	return strRet;
}

static BOOL Is28181Username( const CGSString &strVal )
{
//34020000001310000001
	int s = strVal.length();
	if( s == 20 ||s ==18 )
	{
		for( s-=1 ; s>-1;s-- )
		{
			if( '0' <= strVal[s] && '9' >=strVal[s]  )
			{
				continue;
			}
			else
			{
				return FALSE;
			}
		}
		return TRUE;
	}
	return FALSE;
}

static BOOL Is28181UsernameEqual(const CGSString &str28181Username, const CGSString &strDestUsername )
{
	if( Is28181Username(strDestUsername) )
	{
		//3401000000
		if( 0==memcmp( str28181Username.c_str(), strDestUsername.c_str(), 10) )
		{
			return TRUE;
		}
	}
	return FALSE;
}

/*
*********************************************************************
*
*@brief : 
*
*********************************************************************
*/
bool CClientConnecter::Parser( osip_message_t *pMsg, bool bRequest)
{
	Reset();
	GS_ASSERT_RET_VAL(pMsg, false);


	
	if( pMsg->from )
	{
		osip_uri_t *pUri = osip_from_get_url(pMsg->from);
		if( pUri )
		{
			char *pName = osip_uri_get_username(pUri);
			if( pName )
			{
				m_strFromUser = pName;
			}
		}
	}

	if( pMsg->to )
	{
		osip_uri_t *pUri = osip_to_get_url(pMsg->to);
		if( pUri )
		{
			char *pName = osip_uri_get_username(pUri);
			if( pName )
			{
				m_strToUser = pName;
			}
		}
	}


	m_bRequest = bRequest;
	if( bRequest )
	{
		//通过 contact 获取
#if 0
		char *czTemp;
		osip_contact_t *pContact = NULL;
		osip_message_get_contact(pXEvent->request, 0,
				&pContact );
		if( pContact )
		{
			osip_uri_t *pUrl = osip_contact_get_url(pContact);
			if( pUrl )
			{
				czTemp = osip_uri_get_host(pUrl);
				if( czTemp )
				{
					m_strRemoteIp = czTemp;
				}		
				czTemp = osip_uri_get_port(pUrl);
				if( czTemp )
				{
					m_iRemotePort = atoi(czTemp);
				}
				else
				{
					m_iRemotePort = 5060;
				}
				return true;
			}	
		}
#endif
		//同 VIA 获取
		osip_via_t *pVia = NULL;
		osip_message_get_via(pMsg, 0, &pVia);
		if( !pVia )
		{
			GS_ASSERT(0);
			return false;
		}
		char *czHost = via_get_host(pVia);
		GS_ASSERT_RET_VAL(czHost, false);
		m_strRemoteIp = czHost;
		czHost = via_get_port(pVia);
		GS_ASSERT_RET_VAL(czHost, false);
		m_iRemotePort = atoi(czHost);
		
		return true;
	}
	else
	{
		//回复是使用 Cnnid
		GS_ASSERT_RET_VAL(pMsg->from, false);
		osip_generic_param_t *pTag = NULL;
		osip_from_get_tag(pMsg->from, &pTag);			
		if( pTag && pTag->gvalue )
		{
			const char *p;
			p = strstr(pTag->gvalue, GXX_TAG_MARK );
			if( p )
			{				
				char Temp[60];			
				p += strlen(GXX_TAG_MARK);
				int i = 0;
				for( ; *p>='0' && *p<='9'; i++, p++  )
				{
					Temp[i] = *p;
				}
				Temp[i] =  '\0';

				GS_ASSERT(i>0);
				if( i )
				{
					m_iCnnId = GSStrUtil::ToNumber<UINT32>(Temp);
					return true;
				}
			}
		}
	}
	GS_ASSERT(0);
	return false;
}

/*
*********************************************************************
*
*@brief : 
*
*********************************************************************
*/

CGSString CClientInfo::MakeTag(const CGSString &strSipTag) //生成 Tag
{
	CGSString strTag = strSipTag;
	strTag += GXX_TAG_MARK;
	strTag += GSStrUtil::ToString(m_iCnnID);
	return strTag;
}

/*
*********************************************************************
*
*@brief : 
*
*********************************************************************
*/

CSipServer::CSipServer(void)
{
	bzero(&m_stListener, sizeof(m_stListener));
	m_hEXosip = NULL;
	m_bStart = FALSE;
	m_pUserContext = NULL;
	m_iExpiresRegister = 3600;
	m_strServerName.clear();
	m_tvLast= time(NULL);
#if defined(_DEBUG) || defined(DEBUG)
	osip_trace_enable_until_level(/*END_TRACE_LEVEL*/ OSIP_INFO2);
#else
	osip_trace_enable_until_level((osip_trace_level_t)0);
#endif
}

CSipServer::~CSipServer(void)
{
	m_bStart = FALSE;
}


CClientInfo *CSipServer::FindClientOfNet( const CClientConnecter &nner)
{
	

	
	if( nner.m_bRequest )
	{
		//使用IP:PORT 查找
		CSetClientOfNet::iterator csIt;
		for( csIt=m_mapClientsOfNet.begin(); csIt!=m_mapClientsOfNet.end(); ++csIt )
		{
				//客户端
			if( (*csIt)->m_iRemotePort==nner.m_iRemotePort &&
					((*csIt)->m_strRemoteIp == nner.m_strRemoteIp  )
					/*&& (*csIt)->m_strLocalSrvName==nner.m_strToSrvName*/ )
			{
				return (*csIt);
			}	
			
		}


		for( csIt=m_mapClientsOfNet.begin(); csIt!=m_mapClientsOfNet.end(); ++csIt )
		{
			//客户端
			if(!nner.m_strFromUser.empty() && (*csIt)->m_strRemoteSipSrvName==nner.m_strFromUser ) 
			{
				return (*csIt);
			}
		}

	}
	else 
	{
		if( nner.m_iCnnId > 0 )
		{
			//使用 CNN ID 查找
			return FindClientOfCnnId((INT32)nner.m_iCnnId  );
		}		
		GS_ASSERT(0); //不应该		
	}
	return NULL;
}

EnumSipErrorCode CSipServer::Init( const StruSipListener &stListener )
{
	CGSString strPath = GSGetApplicationPath();
	strPath += "sipdebug.level";
	FILE *fp = fopen(strPath.c_str(), "rb");
	if( fp )
	{
		char temp[32];
		int x = fread(temp, 1, 31, fp);
		if( x> 0 )
		{
			temp[x] = '\0';
			x = atoi(temp);
			osip_trace_enable_until_level((osip_trace_level_t)x);
		}	
		fclose(fp);
	}

	if( !LoadLocalIf() )
	{
		SIP_DEBUG_PRINT( "读取网卡配置失败.\n");
		return eSIP_RET_INIT_FAILURE;
	}
	m_stListener = stListener;
	int iRet = 0;
	m_bStart = TRUE;
#ifdef SIP_INSTANCE_ONE 
	if( AtomicInterInc(s_iOSipRefs) == 1 )
	{
		//初始化  osip 库
		iRet = eXosip_init(&m_hEXosip);
		if( iRet )
		{
			SIP_DEBUG_PRINT( "eXosip_init failure.\n");
			s_bOSipInit = -1;
			
		}
		else
		{
			s_bOSipInit = 1;
		}
	}
	else
	{
		while( !s_bOSipInit )
		{
			MSLEEP(10);
		}		
	}

	if( s_bOSipInit < 0 )
	{
		m_bStart = FALSE;
		if( AtomicInterDec(s_iOSipRefs) == 0  )
		{
			s_bOSipInit = 0;
		}
		return MAKE_OSIP_ERRNO(iRet);
	}
#else
	iRet = eXosip_init(&m_hEXosip);
	if( iRet )
	{

		GS_ASSERT(0);
		m_bStart = FALSE;
		eXosip_quit(m_hEXosip);
		m_hEXosip = NULL;
		return MAKE_OSIP_ERRNO(iRet);
	}
#endif
	return eSIP_RET_SUCCESS;
}


void CSipServer::SetGateway(const char *czGetewayIPV4 )
{
	if( czGetewayIPV4 && m_hEXosip )
	{
		eXosip_set_option( EXOSIP_OPT_SET_IPV4_FOR_GATEWAY, czGetewayIPV4, m_hEXosip);
	}
}

EnumSipErrorCode CSipServer::Start(EnumSipConnectType eCnnType,
						const char *czLocalIp, int iLocalPort)
{
	GS_ASSERT_RET_VAL(G_IS_VALID , eSIP_RET_E_STATUS);

	
	int iRet = IPPROTO_UDP;
	if( eCnnType == eSIP_CONNECT_TCP )
	{
		iRet = IPPROTO_TCP;
	}
	iRet = eXosip_listen_addr (iRet, czLocalIp ? czLocalIp : "0.0.0.0" , iLocalPort, AF_INET, 0, m_hEXosip);
	if ( iRet)
	{
		SIP_DEBUG_PRINT( "eXosip_listen_addr failure. %s:%d\n",
							czLocalIp ? czLocalIp : "0", iLocalPort);
		return MAKE_OSIP_ERRNO(iRet);
	}
	iLocalPort = eXosip_get_listen_port(m_hEXosip);
	m_eListenCnnType = eCnnType;
	if( czLocalIp )
	{
		m_strListenIP = czLocalIp;
		if( m_strServerName.empty() )
		{
			GSStrUtil::Format(m_strServerName, "%s:%d",czLocalIp, iLocalPort );
		}	
	}
	else
	{		
		m_strListenIP.clear();		
	}
	m_iListenPort = iLocalPort;
	m_csThread.Start(ThreadCallback, this);
	return eSIP_RET_SUCCESS;
}

void CSipServer::Stop(void)
{
	m_bStart = FALSE;
	m_csThread.Stop();
	MSLEEP(100);
	m_csThread.Join(2000);
	eXosip_quit(m_hEXosip);
	m_hEXosip = NULL;
}


void CSipServer::ThreadCallback(CGSThread *pThread, void *pParam )
{
	CSipServer *p = (CSipServer*)pParam;
	p->OnThreadEventEntry();
//	int ibreak;
}

//线程回调事件入口
void CSipServer::OnThreadEventEntry(void)
{
	eXosip_event_t *je;

	while( !m_csThread.TestExit() && G_IS_VALID )
	{
		je = eXosip_event_wait (0, 50, m_hEXosip);
		eXosip_lock(m_hEXosip);
		//eXosip_automatic_action ();
		eXosip_automatic_refresh (m_hEXosip);
		eXosip_unlock(m_hEXosip);
		if (je == NULL)
		{
			ClearDisActiveClient();
			continue;
		}
		//SIP_DEBUG_PRINT("On OSip Event*** : %d\n",je->type );
		switch(je->type)
		{
		case EXOSIP_REGISTRATION_NEW :
			{
				//新的注册
				//会话外（默认）命令，即eXosip_message_send_request 所发送的命令皆在此处处理
				EnumSipMethod eMethod = CvtMethodStr2I(je->request->sip_method);
				GS_ASSERT(eMethod != eSIP_METHOD_INVALID );

				if( eMethod == eSIP_METHOD_REGISTER )
				{
					//登陆
					HandleRequestWithRegister(je);
				}
				else
				{
					GS_ASSERT(0); //为什么不是 注册方法
					SendResponseUnauthorized(je, NULL);
				}
			}
		break;
		case EXOSIP_MESSAGE_NEW:
			{
				//会话外（默认）命令，即eXosip_message_send_request 所发送的命令皆在此处处理
				EnumSipMethod eMethod = CvtMethodStr2I(je->request->sip_method);
				GS_ASSERT(eMethod != eSIP_METHOD_INVALID );

				if( eMethod == eSIP_METHOD_REGISTER )
				{
					//登陆

					//GS_ASSERT(0); //为什么不是 注册方法
					HandleRequestWithRegister(je);
					//HandleRequestWithRegister(je);
				}
				else
				{
					HandleRequest(je,eMethod);
				}
			}
		break;
		case EXOSIP_CALL_INVITE:
		case EXOSIP_CALL_REINVITE:
			{				
				SIP_DEBUG_PRINT("***info got invite\n");
				EnumSipMethod eMethod = CvtMethodStr2I(je->request->sip_method);
				HandleRequest(je,eMethod );
			}
			break;
		case EXOSIP_CALL_MESSAGE_NEW:
			{
				//会话内的命令
				SIP_DEBUG_PRINT("receive ***CALL_MESSAGE_NEW cmd\n");
				EnumSipMethod eMethod = CvtMethodStr2I(je->request->sip_method);
				GS_ASSERT(eMethod != eSIP_METHOD_INVALID );
				HandleRequest(je,eMethod);

			}
			break;
		case EXOSIP_IN_SUBSCRIPTION_NEW:
		case EXOSIP_SUBSCRIPTION_UPDATE:
			{
				//SUBSCRIBE
				SIP_DEBUG_PRINT ("got a SUBSCRIPTION\n");
				EnumSipMethod eMethod = CvtMethodStr2I(je->request->sip_method);
				HandleRequest(je,eMethod );

			}
			break;
		case EXOSIP_SUBSCRIPTION_NOTIFY:
			{
				SIP_DEBUG_PRINT("get subcription notify.\n");				
				EnumSipMethod eMethod = CvtMethodStr2I(je->request->sip_method);
				HandleRequest(je,eMethod );
			}
			break;




			//////////////////////////////////////////////////////////////////////////
			//收到网路应答
		case EXOSIP_CALL_ACK:
			{

				SIP_DEBUG_PRINT("Ack call\n");
				EnumSipMethod eMethod = eSIP_METHOD_ACK; //CvtMethodStr2I(je->response->sip_method);
				HandleRequest(je,eMethod);
			}
			break;


		case EXOSIP_CALL_REDIRECTED:
		case EXOSIP_CALL_NOANSWER:
		case EXOSIP_CALL_RELEASED:
		case EXOSIP_CALL_CLOSED:
		case EXOSIP_CALL_CANCELLED:
			{
				//连接超时
				//TODO....
				SIP_DEBUG_PRINT("receive of timeouts.\n");
				//osip_message_get_body (je->request, 0, &body); 
				//由body 去组装SIPpacket
				//_listener->onResponse();//error
			}
			break;
		case EXOSIP_CALL_PROCEEDING:
			{
				//对端已经接受请求
				//INVITE the first two ack
				SIP_DEBUG_PRINT("processding receive \n");
			}
			break;
		case EXOSIP_CALL_RINGING:
			{
				//INVITE the third ack
				SIP_DEBUG_PRINT("EXOSIP_CALL_RINGING receive \n");
			}
			break;

		case EXOSIP_CALL_TIMEOUT:
		case EXOSIP_CALL_REQUESTFAILURE:
		case EXOSIP_CALL_SERVERFAILURE:
		case EXOSIP_CALL_GLOBALFAILURE:
			{	
				//应答错误 
					EnumSipMethod eMethod = eSIP_METHOD_RESPONSE; //CvtMethodStr2I(je->response->sip_method);
				HandleResponse(je,eMethod);
			}
			break;
		case EXOSIP_CALL_ANSWERED:
			{
				//INVITE last ack
				SIP_DEBUG_PRINT("answered\n");
					EnumSipMethod eMethod = eSIP_METHOD_RESPONSE; //CvtMethodStr2I(je->response->sip_method);
				HandleResponse(je,eMethod);

// 				osip_message_t *ack = NULL;
// 				eXosip_lock(m_hEXosip);
// 				if ( eXosip_call_build_ack( je->did, &ack ,m_hEXosip) == 0 )
// 				{
// 					int tmpRes = eXosip_call_send_ack( je->did, ack,m_hEXosip );
// 					int rest = tmpRes;
// 				}
// 				eXosip_unlock(m_hEXosip);
			}
			break;
		

		case EXOSIP_CALL_MESSAGE_ANSWERED:
			{
				//   INFO/ NOTIFY
				SIP_DEBUG_PRINT("INFO/ NOTIFY answered\n");
					EnumSipMethod eMethod = eSIP_METHOD_RESPONSE; //CvtMethodStr2I(je->response->sip_method);
				HandleResponse(je,eMethod);
			}
			break;

		case EXOSIP_SUBSCRIPTION_PROCEEDING:
			{
				//subscribe first ack(maybe)
				SIP_DEBUG_PRINT("subcribe proceeding\n");
			}
			break;

		case EXOSIP_SUBSCRIPTION_ANSWERED:
			{
				//subscribe ACK
				SIP_DEBUG_PRINT( "接收到：subscribe ACK");
					EnumSipMethod eMethod = eSIP_METHOD_RESPONSE; //CvtMethodStr2I(je->response->sip_method);
				HandleResponse(je,eMethod);
			}
			break;

		case EXOSIP_REGISTRATION_FAILURE:
			{
				//fixme error 
				//osip_message_get_body (je->request, 0, &body); 
				//buildAndSendRes(je,body,0);
				HandleResponseWithRegister(je);
			}			
			break;

		case EXOSIP_REGISTRATION_SUCCESS:
			{
				//osip_message_get_body (je->response, 0, &body); 
				//buildAndSendRes(je,body);
				HandleResponseWithRegister(je);
			}
			break;

		case EXOSIP_MESSAGE_ANSWERED:
			{
				SIP_DEBUG_PRINT( "接收到：EXOSIP_MESSAGE_ANSWERED");
				//outside of dialog Response success
					EnumSipMethod eMethod = eSIP_METHOD_RESPONSE; //CvtMethodStr2I(je->response->sip_method);
				HandleResponse(je,eMethod);
			}
			break;

		case EXOSIP_MESSAGE_REDIRECTED:
		case EXOSIP_MESSAGE_REQUESTFAILURE:
		case EXOSIP_MESSAGE_SERVERFAILURE:
		case EXOSIP_MESSAGE_GLOBALFAILURE:
			{
				//outside of dialog Response failure
				SIP_DEBUG_PRINT( "接收到：utside of dialog Response failure");
				EnumSipMethod eMethod = eSIP_METHOD_RESPONSE; //CvtMethodStr2I(je->response->sip_method);
				HandleResponse(je,eMethod);
			}
			break;
		default :
			{
				SIP_DEBUG_PRINT("****OXSip unkown event: %d\n", je->type);
				int iBreak = 0;
			}
		break;
		}
		eXosip_event_free(je,m_hEXosip);	
	}
	
	SIP_DEBUG_PRINT("****SipStack Thread Leav.\n");

}

void CSipServer::FreeSession( StruSessionPri *pSnPri)
{
	if( pSnPri )
	{
		SipSessionDisconnect(pSnPri->iCnnId);
		pSnPri->iCnnId = -1;
		delete pSnPri;
	}
}

EnumSipErrorCode CSipServer::Connect( EnumSipConnectType eCnnType, 
						 const char *czRemoteHost,int iRemotePort,
						 const char *czRemoteSipServerName, 
						 StruSessionPri **pSnPri )
{
	GS_ASSERT_RET_VAL(G_IS_VALID , eSIP_RET_E_STATUS);
	CGSString strRemoteIp = HostName2Ip(czRemoteHost); //远程IP
	if( strRemoteIp.empty() )
	{
		return eSIP_RET_E_INVALID;
	}
	CClientInfo *pInfo = new CClientInfo(this, INVALID_REG_ID);
	if( !pInfo )
	{
		GS_ASSERT(0);
		return eSIP_RET_E_NMEN;
	}
	pInfo->m_pSnPri = new StruSessionPri();
	if( !pInfo->m_pSnPri )
	{
		GS_ASSERT(0);
		pInfo->Unref();
		return eSIP_RET_E_NMEN;
	}
	pInfo->m_pSnPri->iCnnId = pInfo->m_iCnnID;
	pInfo->m_pSnPri->pSrv = this;
	pInfo->m_pSnPri->pUserData = NULL;

	pInfo->m_strRemoteIp = strRemoteIp;
	pInfo->m_iRemotePort = iRemotePort;
	pInfo->m_eCnnType = eCnnType;
	pInfo->m_bTypeClient = TRUE;
	pInfo->m_tvLastActive = DoGetTickCount();

	if( czRemoteSipServerName )
	{

		pInfo->m_strRemoteSipSrvName = czRemoteSipServerName;
	}


	if( !m_strListenIP.empty() )
	{
		pInfo->m_strLocalIp = m_strListenIP;
	}
	else
	{
		//猜测本地IP
		pInfo->m_strLocalIp = GuessLocalIp(czRemoteHost);
	}

	if( m_strServerName.empty() )
	{
		GSStrUtil::Format(pInfo->m_strLocalSrvName, "%s:%d",pInfo->m_strLocalIp.c_str(), m_iListenPort );
	}
	else
	{
		pInfo->m_strLocalSrvName = m_strServerName;
	}

	pInfo->m_iLocalPort = m_iListenPort;

	if( pSnPri )
	{
		*pSnPri = pInfo->m_pSnPri;
	}
	else
	{
		delete pInfo->m_pSnPri ;
		pInfo->m_pSnPri  = NULL;
	}

	m_wrMutex.LockWrite();
	m_mapClientsOfCnnId.insert(make_pair(pInfo->m_iCnnID, pInfo));
	m_wrMutex.UnlockWrite();
	return eSIP_RET_SUCCESS;

}



void CSipServer::HandleRequestWithRegister( eXosip_event_t *pXEvent )
{
	//登记并注册
	osip_body_t *pBody = NULL;
	if ( pXEvent == NULL 
		|| pXEvent->request == NULL
		|| pXEvent->request->from == NULL
		|| pXEvent->request->from->url == NULL )
	{
		GS_ASSERT(0);
		return;
	}

	osip_header_t* expirse = NULL;
	int iExpirse = 0;
	if(osip_message_get_expires(pXEvent->request,0,&expirse) >= 0)
	{
		iExpirse = atoi(expirse->hvalue);
	}


	//暂时默认只有一个Authorization节点


	osip_authorization_t* pAuth = NULL;
	if( osip_message_get_authorization(pXEvent->request, 0, &pAuth ) || pAuth == NULL
		|| osip_authorization_get_response(pAuth) == NULL )
	{
		SendResponseUnauthorized(pXEvent, NULL);
		return;
	}


	
	
	long iCnnID = INVALID_CNN_ID;
	
	BOOL bNew = FALSE;
	CGSString strAuth;
	CGSString strUsername;

	char* tmpStr = NULL;
	osip_authorization_to_str(pAuth,&tmpStr);
	strAuth = tmpStr;
	free(tmpStr);

	tmpStr = osip_authorization_get_username(pAuth);
	if( tmpStr == NULL )
	{
		//没有用户名
		SendResponseUnauthorized(pXEvent, NULL);
		return;
	}

	strUsername = TrimUpDot(tmpStr);

	CClientConnecter cnner;
	if( pXEvent->request )
	{
		bool btest =  cnner.Parser( pXEvent->request, true);
		
		if( !btest  )
		{
			GS_ASSERT(0);
			SendSimpleResponse(pXEvent, SIP_RESPONSE_CODE_BAD_REQUEST, 
									"Bad Sip Header", NULL, NULL );
			return;
		}
	}


	m_wrMutex.LockWrite();


	CClientInfo *pInfo = FindClientOfNet(cnner);



	if( pInfo == NULL )
	{
		// 已经退出
		if( iExpirse < 1 )
		{
			//已经推出
			SendSimpleResponse(pXEvent, 200, "OK", NULL, pInfo );
			m_wrMutex.UnlockWrite();
			return;
		}		
		pInfo = new CClientInfo(this, pXEvent->rid);		
		iCnnID = pInfo->m_iCnnID;
		pInfo->m_strAuthorization = strAuth;
		pInfo->m_strUsername = strUsername;
		
		bNew = TRUE;
		pInfo->m_pSnPri = new StruSessionPri();
		pInfo->m_pSnPri->iCnnId = iCnnID;
		pInfo->m_pSnPri->pSrv = this;
		pInfo->m_pSnPri->pUserData = NULL;
		pInfo->m_iExpiresRegister=iExpirse;
		pInfo->m_strLocalSrvName = m_strServerName;
		pInfo->m_b28181Username = Is28181Username(strUsername);

		m_mapClientsOfNet.insert(pInfo);
		m_mapClientsOfCnnId.insert(make_pair(iCnnID, pInfo));		

	}
	else
	{
		pInfo->m_strAuthorization = strAuth;
		pInfo->m_strUsername = strUsername;
		pInfo->m_iExpiresRegister=iExpirse;
		pInfo->m_b28181Username = Is28181Username(strUsername);
		iCnnID = pInfo->m_iCnnID;
	}

	pInfo->m_tvRegisted = time(NULL);  //标志注册时间
	pInfo->m_tvLastActive = DoGetTickCount();
		

	if( osip_authorization_get_realm(pAuth) )
	{
		pInfo->m_strRealm = osip_authorization_get_realm(pAuth);
	}

	if( osip_authorization_get_nonce(pAuth) )
	{
		pInfo->m_strNonce = osip_authorization_get_nonce(pAuth);
	}



	osip_message_t *pRequest = pXEvent->request;
	char *pTemp = NULL;
	if( pRequest && pRequest->call_id && 0==osip_call_id_to_str(pRequest->call_id, &pTemp))
	{
		pInfo->m_strCallId = pTemp;
		osip_free(pTemp);
	}
	else
	{
		pInfo->m_strCallId = "unknown";
		GS_ASSERT(0);
	}



	if( pRequest && pRequest->from )
	{
		if( osip_from_get_displayname(pRequest->from) )
		{
			pInfo->m_strRemoteSipSrvName = osip_from_get_displayname(pRequest->from);
		}
		else
		{
			osip_uri_t *pUri = osip_from_get_url(pRequest->from);
			
			if( osip_uri_get_username(pUri) )
			{
				pInfo->m_strRemoteSipSrvName = osip_uri_get_username(pUri);
				pInfo->m_strRemoteSrvName  = pInfo->m_strRemoteSipSrvName;
			}
			else if( osip_uri_get_host(pUri) )
			{
				pInfo->m_strRemoteSipSrvName = osip_uri_get_host(pUri);
				if( osip_uri_get_port(pUri) )
				{
					pInfo->m_strRemoteSipSrvName += ":";
					pInfo->m_strRemoteSipSrvName += osip_uri_get_port(pUri);
				}
				
			}		
			else
			{
				GS_ASSERT(0);
				pInfo->m_strRemoteSipSrvName.clear();
			}
			pInfo->m_strRemoteSrvName  = pInfo->m_strRemoteSipSrvName;
			
		}		
	}



	int bRegisted = pInfo->m_bRegisted;
	pInfo->m_bRegisted = iExpirse>0;

	StruSessionPri *pSnPri = pInfo->m_pSnPri;


	//获取对端的IP 和 端口
	

	pInfo->m_strRemoteIp.clear();
	if( pInfo->m_strRemoteIp.empty() )
	{
		CClientConnecter cnner;
		cnner.Parser(pXEvent->request, true);
		pInfo->m_strRemoteIp = cnner.m_strRemoteIp;
		pInfo->m_iRemotePort = cnner.m_iRemotePort;
	}

	if( pInfo->m_strRemoteIp.empty() )
	{
		GS_ASSERT(0);
		SendSimpleResponse(pXEvent, 403, "Invalid Param remote ip", NULL, pInfo);
		m_wrMutex.UnlockWrite();
		if( bNew )
		{
			delete pSnPri;
			DeleteClient(iCnnID);
		}
		return;
	}

	if( pInfo->m_strRemoteSipSrvName.empty() )
	{
		//从新设定服务器名称
		GSStrUtil::Format(pInfo->m_strRemoteSipSrvName,
			"%s:%d",pInfo->m_strRemoteIp.c_str(), pInfo->m_iRemotePort );
		pInfo->m_strRemoteSipSrvName = pInfo->m_strRemoteSipSrvName;
	}
	

	if( !m_strListenIP.empty() )
	{
		pInfo->m_strLocalIp = m_strListenIP;
	}
	else
	{
		//猜测本地IP
		pInfo->m_strLocalIp = GuessLocalIp(pInfo->m_strRemoteIp);
	}

	pInfo->m_iLocalPort = m_iListenPort;


	
	pInfo->Ref();
	CClientInfo::CAutoUnref csAuto(pInfo);

	m_wrMutex.UnlockWrite();

	if( bNew )
	{
		//发送新建客户端连接
		if( m_stListener.OnClientConnectEvent==NULL 
			|| m_stListener.OnClientConnectEvent((SipServiceHandle)this, (SipSessionHandle)pSnPri) )
		{
			//拒绝连接
			
			SIP_DEBUG_PRINT("Register refuse of user.\n");
			delete pSnPri;			
			DeleteClient(iCnnID);
			SendSimpleResponse(pXEvent, eSIP_RESPONSE_BUSY, "Server Busy", NULL, pInfo);			
			return;
		}
	}
	else if( iExpirse < 1)
	{
		//退出
		if( bRegisted && m_stListener.OnClientDisconnectEvent ) 
		{
			pSnPri->iCnnId = -1;
			m_stListener.OnClientDisconnectEvent((SipServiceHandle)this, (SipSessionHandle)pSnPri );
		}
		
		DeleteClient(iCnnID);
		SendSimpleResponse(pXEvent, 200, "OK", NULL, pInfo);			
		return;
	}
	

	/*SendSimpleResponse(pXEvent, 200, "OK");*/

	StruSipData stSipData;
	bzero(&stSipData, sizeof(stSipData));

	stSipData.eContentType = eSIP_CONTENT_NONE;	
	stSipData.eMethod = eSIP_METHOD_REGISTER;

	InitSipData(iCnnID, pXEvent,eSIP_DATA_REQUEST, stSipData);

	if( strUsername.size() )
	{
		GS_SNPRINTF(stSipData.vContent,SIP_MAX_CONTENT_LEN,"%s:%s",
			SIP_STRKEY_USERNAME, strUsername.c_str());
		stSipData.iContentLength = strlen(stSipData.vContent);
	}
	GS_ASSERT( m_stListener.OnSIPPacketEvent );
	m_stListener.OnSIPPacketEvent((SipServiceHandle)this, (SipSessionHandle)pSnPri, &stSipData);

	

}

void CSipServer::SetDlgKeyMember(osip_message_t *pOSipMsg, StruSipDialogKey &stKey )
{
// 	stKey.iCallId = pXEvent->rid;
// 	stKey.iDialogId = pXEvent->did;
// 	stKey.iTransId = pXEvent->tid;
// 	stKey.iCSeq = pXEvent->cid;
	osip_contact_t* pContact = NULL;
	if(pOSipMsg )
	{		
		if( osip_message_get_contact(pOSipMsg,0, &pContact) == 0)
		{
			GS_SNPRINTF(stKey.szContactScheme, sizeof(stKey.szContactScheme),
				"%s", pContact->url->scheme );
			GS_SNPRINTF(stKey.szContactHost, sizeof(stKey.szContactHost),
				"%s", pContact->url->host );
			GS_SNPRINTF(stKey.szContackUsername, sizeof(stKey.szContackUsername),
				"%s", pContact->url->username );
			stKey.iContactPort = atoi(pContact->url->port);

			//osip_contact_free(pContact);
		}
		osip_header_t* expirse = NULL;
		if(osip_message_get_expires(pOSipMsg,0,&expirse) >= 0)
		{
			stKey.iExpirse = atoi(expirse->hvalue);
		}

		osip_call_id_t * callid = osip_message_get_call_id(pOSipMsg);
		if( callid )
		{
			char *czTemp = NULL;
			if( !osip_call_id_to_str(callid, &czTemp))
			{
				strncpy(stKey.czDialogKey, czTemp, SIP_MAX_DIGID_STRING);
				osip_free(czTemp);
			}
		}
	}
}


void CSipServer::InitSipData(const long iCnnId,const eXosip_event_t *pXEvent, 
							 const EnumSipDataType eDataType, 
							 StruSipData &stData )
{
	stData.eDataType = eDataType;
	stData.stDialog.iCnnId = iCnnId;
	stData.stDialog.iCallId = pXEvent->rid;
	stData.stDialog.iDialogId = pXEvent->did;
	stData.stDialog.iTransId = pXEvent->tid;
	stData.stDialog.iCSeq = pXEvent->cid;
	
	const osip_message_t *pMessage = pXEvent->request;

	char *czTemp;


	czTemp = osip_cseq_get_number(pMessage->cseq);
	if( czTemp )
	{
		stData.stDialog.iCSeq = GSStrUtil::ToNumber<UINT32>(czTemp);
	}

	if( pMessage->from )
	{
		czTemp = NULL;
		osip_from_to_str( pMessage->from, &czTemp);
		if( czTemp )
		{
			osip_generic_param_t *dest = NULL;
			strncpy(stData.stDialog.szFrom, czTemp, 255);
			osip_free(czTemp);
		}
		osip_uri_param_t *pTagParam = NULL;		
		if( !osip_from_get_tag(pMessage->from, &pTagParam)  && pTagParam->gvalue )
		{
			strncpy(stData.stDialog.czFromTag, pTagParam->gvalue, SIP_MAX_TAG_STRING);
		}
	}
	if( pMessage->to )
	{
		czTemp = NULL;
		osip_to_to_str( pMessage->to, &czTemp);
		if( czTemp )
		{
			strncpy(stData.stDialog.szTo, czTemp, 255);
			osip_free(czTemp);
		}
		osip_uri_param_t *pTagParam = NULL;		
		if( !osip_from_get_tag(pMessage->to, &pTagParam)  && pTagParam->gvalue )
		{
			strncpy(stData.stDialog.czToTag, pTagParam->gvalue, SIP_MAX_TAG_STRING);
		}
	}


	osip_call_id_t * callid = osip_message_get_call_id(pMessage);
	if( callid )
	{
		czTemp = NULL;
		if( !osip_call_id_to_str(callid, &czTemp))
		{
			strncpy(stData.stDialog.czDialogKey, czTemp, SIP_MAX_DIGID_STRING);
			osip_free(czTemp);
		}
	}


	if( eDataType == eSIP_DATA_RESPONSE )
	{
		pMessage = pXEvent->response;
		if( pMessage )
		{
			if( !callid )
			{
				callid = osip_message_get_call_id(pMessage);
				if( callid )
				{
					czTemp = NULL;
					if( !osip_call_id_to_str(callid, &czTemp))
					{
						strncpy(stData.stDialog.czDialogKey, czTemp, SIP_MAX_DIGID_STRING);
						osip_free(czTemp);
					}
				}
			}

			stData.stResponseResult.bOk = pMessage->status_code == 200;
			stData.stResponseResult.iSipErrno = pMessage->status_code;	

			if( pMessage->reason_phrase )
			{
				strncpy( stData.stResponseResult.szMessage, pMessage->reason_phrase, SIP_MAX_RESPONSE_ERROR_LEN-1);
			}
			else
			{
				stData.stResponseResult.szMessage[0] = '\0';
			}
			pMessage= pXEvent->response;
		}
		else
		{
			stData.stResponseResult.bOk = false;
			stData.stResponseResult.iSipErrno = 456;	
		}
	}

	

	

	if( pMessage )
	{
		osip_body_t *pBody = NULL;
		if( osip_message_get_body (pMessage, 0, &pBody) == 0 )
		{
			char *pDest =NULL;
			size_t iLen = 0;
			if(  0== osip_body_to_str(pBody, &pDest, &iLen) )
			{
				strncpy(stData.vContent, pDest, MIN(SIP_MAX_CONTENT_LEN,iLen) );
				stData.vContent[SIP_MAX_CONTENT_LEN-1] = '\0';
				stData.iContentLength = strlen(stData.vContent);
				osip_free(pDest);
			}
			else
			{
				GS_ASSERT(0);
			}
		}
		//添加 Subject
		osip_generic_param_t *pMsg = NULL;



		osip_generic_param_get_byname((osip_list_t *) &pMessage->headers,"Subject", &pMsg );

		if( pMsg )
		{
			czTemp = osip_generic_param_get_value(pMsg);
			if ( czTemp )
			{
				strncpy(stData.stDialog.szSubject, czTemp, SIP_MAX_SUBJECT_STRING);
			}
		}
	}





	
	if(pMessage )
	{		
		//注册的FROM和TO都UASIP地址,需特殊处理
		osip_contact_t* pContact = NULL;
		if( osip_message_get_contact(pMessage,0, &pContact) == 0)
	{
		GS_SNPRINTF(stData.stDialog.szContactScheme, sizeof(stData.stDialog.szContactScheme),
			"%s", pContact->url->scheme );
		GS_SNPRINTF(stData.stDialog.szContactHost, sizeof(stData.stDialog.szContactHost),
			"%s", pContact->url->host );
		GS_SNPRINTF(stData.stDialog.szContackUsername, sizeof(stData.stDialog.szContackUsername),
			"%s", pContact->url->username );
		stData.stDialog.iContactPort = atoi(pContact->url->port);

		//osip_contact_free(pContact);
		}
		osip_header_t* expirse = NULL;
		if(osip_message_get_expires(pMessage,0,&expirse) >= 0)
		{
			stData.stDialog.iExpirse = atoi(expirse->hvalue);
		}
	}
	
}

EnumSipErrorCode CSipServer::SipService_GetListenInfo(StruSipConnnectInfo *pRes)
{
	GS_ASSERT_RET_VAL(G_IS_VALID , eSIP_RET_E_STATUS);
	bzero(pRes, sizeof(*pRes));
	pRes->bOnline = true;
	strncpy(pRes->szLocalIp, m_strListenIP.c_str(), SIP_MAX_IP_LEN-1);
	pRes->iLocalPort = m_iListenPort;
	pRes->eConnectType = m_eListenCnnType;
	return eSIP_RET_SUCCESS;
}









/*
*********************************************************************
*
*@brief : 一下为Session 操作部分
*
*********************************************************************
*/



void CSipServer::ClearDisActiveClient(void)
{
	//清除非活动的客户端

	time_t  tv = time(NULL);
	if( tv<m_tvLast )
	{
		m_tvLast = tv;
		return;
	}
	if( (tv-m_tvLast) < 30 )
	{
		return;
	}
	m_tvLast = tv;

	CMapClientOfCnnId::iterator csIt;
	UINT64 tvCur = DoGetTickCount();
	CClientInfo *p;
	m_wrMutex.LockReader();
	for( csIt = m_mapClientsOfCnnId.begin(); csIt!=m_mapClientsOfCnnId.end();  )
	{
		p = csIt->second;

		if( p->m_bTypeClient )
		{
			//客户端不检测
			++csIt;
			continue;
		}
		if( tvCur < p->m_tvLastActive )
		{
			p->m_tvLastActive = tvCur;
			++csIt;
			continue;
		}

		long iCnnID = p->m_iCnnID;
		if( (tvCur-p->m_tvLastActive) > (3600L*1000) )
		{
			//端口						
			if(p->m_pSnPri->iCnnId>-1 && p->m_bRegisted && m_stListener.OnClientDisconnectEvent ) 
			{			
				p->m_pSnPri->iCnnId = -1;
				m_stListener.OnClientDisconnectEvent((SipServiceHandle)this, (SipSessionHandle)p->m_pSnPri );
			}
			m_wrMutex.UnlockReader();
			DeleteClient(iCnnID);
			m_wrMutex.LockReader();
			csIt = m_mapClientsOfCnnId.begin();
		}
		else
		{
			++csIt;
		}
	}
	m_wrMutex.UnlockReader();
}


void CSipServer::RequestResetFromTag( osip_message_t *pOSipMsg, CClientInfo *pClient)
{

	//非对话
	osip_uri_param_t *pTagParam = NULL;
	char *czTag = NULL;
	if( !osip_from_get_tag(pOSipMsg->from, &pTagParam) )
	{
		czTag = pTagParam->gvalue;
		CGSString strNewTag = pClient->MakeTag(czTag);		
		pTagParam->gvalue = osip_strdup(strNewTag.c_str());
		osip_free(czTag);
	}
}

// void  CSipServer::ResponseResetTag( osip_message_t *pOSipMsg, CClientInfo *pClient)
// {
// // 	osip_uri_param_t *pTagParam = NULL;
// // 	char *czTag = NULL;
// // 	if( !osip_from_get_tag(pOSipMsg->from, &pTagParam) )
// // 	{
// // 		czTag = pTagParam->gvalue;
// // 		CGSString strNewTag = pClient->MakeTag(czTag);		
// // 		pTagParam->gvalue = osip_strdup(strNewTag.c_str());
// // 		osip_free(czTag);
// // 	}
// }


//注册
EnumSipErrorCode CSipServer::SipSessionResigter(long iCnnId,
									const char *czUserName, 
									const char *czPassword, 
									StruSipData *pRes, int iTimeout,
									int iExpires )
{
	
	if( iCnnId<0 )
	{
		return eSIP_RET_E_SNNEXIST;
	}

	m_wrMutex.LockWrite();
	CClientInfo *pClient = FindClientOfCnnId(iCnnId);
	if( !pClient )
	{
		m_wrMutex.UnlockWrite();
		return eSIP_RET_E_SNNEXIST;
	}
	if( iExpires<0 )
	{
		iExpires = m_iExpiresRegister;
	}




	if( czUserName )
	{
		pClient->m_strUsername = czUserName;
		pClient->m_b28181Username = Is28181Username(pClient->m_strUsername);
	}
	else
	{
		czUserName = pClient->m_strUsername.c_str();
	}

	if(czPassword)
	{
		pClient->m_strPasswrd = czPassword;
	}
	else
	{
		czPassword = pClient->m_strPasswrd.c_str();
	}

	pClient->m_iExpiresRegister = iExpires;
	


	CGSString strFromUser;
	CGSString strProxy;
	CGSString strContact;
	GSStrUtil::Format(strFromUser, "sip:%s@%s", 
		                  czUserName, pClient->m_strLocalSrvName.c_str() );
	GSStrUtil::Format(strProxy, "sip:%s:%d",
		pClient->m_strRemoteIp.c_str(), pClient->m_iRemotePort );
 	GSStrUtil::Format(strContact, "<sip:%s@%s:%d>",
 				czUserName,pClient->m_strLocalIp.c_str(), pClient->m_iLocalPort );
// 	GSStrUtil::Format(strContact, "<sip:%s@%s:%d>",
//  		czUserName,pClient->m_strRemoteHost.c_str(),pClient->m_iRemotePort );

	//eXosip_clear_authentication_info(m_hEXosip); //???
// 	printf("eXosip_add_authentication_info username=%s,password=%s\n",czUserName,czPassword);
// 	if(eXosip_add_authentication_info(czUserName,czUserName,czPassword,pClient->m_strHa1.c_str(),
// 		pClient->m_strRealm.c_str(), m_hEXosip) != 0)
// 	{
// 		return eSIP_RET_ADD_AUTHENTICATION_FAILURE;
// 	}
	osip_message_t *pMsg = NULL;
	eXosip_lock(m_hEXosip);
	int regid = eXosip_register_build_initial_register (strFromUser.c_str(), 
		strProxy.c_str(), strContact.c_str(), iExpires, &pMsg, m_hEXosip);
	eXosip_unlock(m_hEXosip);
	if(regid < 1 )
	{
		eXosip_unlock(m_hEXosip);
		m_wrMutex.UnlockWrite();
		/*eXosip_call_set_reference(iCnnId, m_hEXosip);*/
		return eSIP_RET_BUILD_REGISTER_FAILURE;
	}
	
	CGSString strAuth;
	strAuth = "Capability algorithm=A:RSA;H:MD5;S:DES";

	osip_message_set_authorization(pMsg, strAuth.c_str());


	pClient->m_iRegId = regid; //保留， 通信时使用


	
	if( !pClient->m_strRemoteSipSrvName.empty()  )
	{

		osip_to_t *pNewTo  = NULL;
		osip_to_init(&pNewTo);
		if( pNewTo )
		{
			
			CGSString strNewTo;
			GSStrUtil::Format(strNewTo, "<sip:%s@%s:%d>", 
				pClient->m_strRemoteSipSrvName.c_str(),		  		    
				pClient->m_strRemoteIp.c_str(),pClient->m_iRemotePort );
			if( osip_to_parse(pNewTo,strNewTo.c_str() ) )
			{
				osip_to_free(pNewTo);
			}
			else
			{
				osip_to_free(pMsg->to);
				pMsg->to = pNewTo;
			}
		}
	}

	// 设置 Tag	
	RequestResetFromTag(pMsg, pClient);

	m_mapClientsOfNet.insert(pClient);
	
	

	char *pTemp;
	if( pMsg->call_id && 0==osip_call_id_to_str(pMsg->call_id, &pTemp))
	{
		pClient->m_strCallId = pTemp;
		osip_free(pTemp);
	}
	else
	{
		pClient->m_strCallId = "unknown";
		GS_ASSERT(0);
	}

	

	
	CResponseWaiter *pCond = NULL;
	int iRet;
	if( pRes )
	{
		//需要等待返回		
		pCond = PrepareWaitResponse(pMsg, eSIP_METHOD_REGISTER );
		if( pCond==NULL )
		{
			GS_ASSERT(0);
			m_wrMutex.UnlockWrite();
			osip_message_free(pMsg);

			return eSIP_RET_OSIP_E_OPER;
		}
	}
	eXosip_lock(m_hEXosip);
	iRet = eXosip_register_send_register( regid, pMsg, m_hEXosip ); //无论成功失败 pMsg 将会被函数释放
	eXosip_unlock(m_hEXosip);
	if ( iRet )
	{
		SIP_DEBUG_PRINT( "%s.Regist: %s:%d\n",
			pClient->m_strUsername.c_str(), pClient->m_strLocalIp.c_str(),pClient->m_iRemotePort);
		CancelWaitResponse(pCond);
		m_wrMutex.UnlockWrite();
		return MAKE_OSIP_ERRNO(iRet);
	}
	m_wrMutex.UnlockWrite();
	
	EnumSipErrorCode eRet= eSIP_RET_SUCCESS;
	if( pCond )
	{
		eRet = WaitResponse(pCond, pRes, iTimeout);	
	}
	return eRet;
}

EnumSipErrorCode CSipServer::SipSessionDisconnect(long iCnnId)
{
	if( iCnnId<0 )
	{
		return eSIP_RET_E_SNNEXIST;
	}
	//断开也就是发送 注销消息
	Unresigter( iCnnId );
	return eSIP_RET_SUCCESS;
}

void CSipServer::SipSessionRelease(long iCnnId)
{
	if( iCnnId<0 )
	{
		return ;
	}

	Unresigter(iCnnId);

	m_wrMutex.LockWrite();
	CClientInfo *pClient = FindClientOfCnnId(iCnnId);
	if( pClient )
	{
		m_mapClientsOfNet.erase(pClient);		
		pClient->m_bRegisted = 0;
		pClient->m_iCnnID = -1;
	}
	m_mapClientsOfCnnId.erase(iCnnId);
	m_wrMutex.UnlockWrite();
	if( pClient )
	{		
		pClient->Unref();
	}
}

	EnumSipErrorCode CSipServer::SipSessionReconnect(long iCnnId)
	{
		if( iCnnId<0 )
		{
			return eSIP_RET_E_SNNEXIST;
		}
		m_wrMutex.LockWrite();
		CClientInfo *pClient = FindClientOfCnnId(iCnnId);
		if( pClient )
		{
			pClient->m_bRegisted = 1;
			m_wrMutex.UnlockWrite();
			return eSIP_RET_SUCCESS;
		}	
		m_wrMutex.UnlockWrite();
		return eSIP_RET_E_SNNEXIST;
	}


	const char *CSipServer::SipSessionAuthorizationUsername(long iCnnId)
	{
		if( iCnnId<0 )
		{
			return NULL;
		}
		
		m_wrMutex.LockReader();
		CClientInfo *pClient = FindClientOfCnnId(iCnnId);
		if( pClient )
		{
			const char *p = pClient->m_strUsername.c_str();
			m_wrMutex.UnlockReader();
			return p;
		}	
		m_wrMutex.UnlockReader();
		return NULL;
	}

	EnumSipErrorCode CSipServer::SipSessionCmpAuthorize(long iCnnId, 
		const CGSString &strUsername, 
		const CGSString &strPasswrd)
	{

		if( iCnnId<0 )
		{
			return eSIP_RET_E_SNNEXIST;
		}

		m_wrMutex.LockReader();
		CClientInfo *pClient = FindClientOfCnnId(iCnnId);
		if( !pClient )
		{
			m_wrMutex.UnlockReader();
			return eSIP_RET_FAILURE;
		}
		pClient->Ref();
		CClientInfo::CAutoUnref csAuto(pClient);	
		m_wrMutex.UnlockReader();




		osip_www_authenticate_t *oswwwAuth = NULL;
		osip_authorization_t *pH = NULL;
		osip_authorization_t *pNewAuth = NULL;

		int res;


		if((res = osip_www_authenticate_init(&oswwwAuth)) )
		{
			GS_ASSERT(0);
			return MAKE_OSIP_ERRNO(res);
		}			
		if( (res=osip_authorization_init( &pH )) )
		{
			GS_ASSERT(0);
			osip_www_authenticate_free(oswwwAuth);
			return MAKE_OSIP_ERRNO(res);
		}

		if( (res=osip_authorization_parse(pH, 
			pClient->m_strAuthorization.c_str()) ) )
		{
			GS_ASSERT(0);
			osip_authorization_free(pH);
			osip_www_authenticate_free(oswwwAuth);
			return MAKE_OSIP_ERRNO(res);
		}

		if( osip_authorization_get_algorithm(pH) )
		{

			osip_www_authenticate_set_algorithm(oswwwAuth, 
				osip_strdup(TrimUpDot(osip_authorization_get_algorithm(pH)).c_str() ) );

		}
		else
		{
			osip_www_authenticate_set_auth_type(oswwwAuth, osip_strdup( "MD5") );
		}
		if( osip_authorization_get_auth_type(pH) )
		{
			osip_www_authenticate_set_auth_type(oswwwAuth,
				osip_strdup(TrimUpDot( osip_authorization_get_auth_type(pH)).c_str() ) );
		}
		else
		{
			osip_www_authenticate_set_auth_type(oswwwAuth, osip_strdup( "Digest") );
		}
		if( osip_authorization_get_realm(pH) )
		{
			osip_www_authenticate_set_realm(oswwwAuth, 
				osip_strdup(TrimUpDot(osip_authorization_get_realm(pH)).c_str() ) );
		}
		if( osip_authorization_get_nonce(pH) )
		{

			osip_www_authenticate_set_nonce(oswwwAuth, 
				osip_strdup(TrimUpDot(osip_authorization_get_nonce(pH)).c_str() ) );
		}
		if( osip_authorization_get_opaque(pH) )
		{
			osip_www_authenticate_set_opaque(oswwwAuth, 				
				osip_strdup(TrimUpDot(osip_authorization_get_opaque(pH)).c_str() ));
		}

		CGSString strUri;
		CGSString strCNonce;
		CGSString strNonceCount;

		if( osip_authorization_get_uri(pH) )
		{
			strUri = TrimUpDot(osip_authorization_get_uri(pH));				
		}

		if( osip_authorization_get_cnonce(pH) )
		{
			strCNonce = TrimUpDot(osip_authorization_get_cnonce(pH));
		}

		if( osip_authorization_get_nonce_count(pH) )
		{
			strNonceCount = TrimUpDot(osip_authorization_get_nonce_count(pH));
		}


		res = __eXosip_create_authorization_header(oswwwAuth,
			strUri.empty() ? NULL : strUri.c_str(), 
			strUsername.c_str(), 
			strPasswrd.c_str(),
			NULL, &pNewAuth, "REGISTER",
			strCNonce.empty() ? "0a4f113b" : strCNonce.c_str() , 
			strNonceCount.empty() ? 1 : GSStrUtil::ToNumber<INT>(strNonceCount), m_hEXosip);


		CGSString strResponse1="2";
		CGSString strResponse2="1";

		if( osip_authorization_get_response(pH) )
		{
			strResponse1 = osip_authorization_get_response(pH);
		}
		osip_authorization_free(pH);
		osip_www_authenticate_free(oswwwAuth);
		if( res )
		{
			GS_ASSERT(0);				
			return MAKE_OSIP_ERRNO(res);
		}
		if( osip_authorization_get_response(pNewAuth) )
		{
			strResponse2 = osip_authorization_get_response(pNewAuth);
		}
		osip_authorization_free(pNewAuth);
		if( strResponse1 == strResponse2 )
		{
			return eSIP_RET_SUCCESS;
		}
		return eSIP_RET_FAILURE;
	}

	EnumSipErrorCode CSipServer::GetConnectInfo(long iCnnId, 
		StruSipConnnectInfo *pRes)
	{
		if( iCnnId<0 )
		{
			return eSIP_RET_E_SNNEXIST;
		}

		CGSAutoReaderMutex rlocker( &m_wrMutex);
		CClientInfo *pClient = FindClientOfCnnId(iCnnId);
		if( !pClient )
		{
			return eSIP_RET_E_SNNEXIST;
		}
		bzero(pRes, sizeof(*pRes));
		pRes->bOnline = pClient->m_bRegisted;
		pRes->eConnectType = eSIP_CONNECT_UDP;
		pRes->iLocalPort = pClient->m_iLocalPort;
		pRes->iRemotePort = pClient->m_iRemotePort;
		if( pClient->m_strLocalIp.length() )
		{
			GS_SNPRINTF(pRes->szLocalIp,SIP_MAX_IP_LEN, pClient->m_strLocalIp.c_str() );
		}
		if( pClient->m_strRemoteIp.length() )
		{
			GS_SNPRINTF(pRes->szRemoteIp,SIP_MAX_IP_LEN, pClient->m_strRemoteIp.c_str() );
		}
		return eSIP_RET_SUCCESS;
	}

	EnumSipErrorCode CSipServer::SipSessionSetOptions(long iCnnId, INT iOptionName, const char *pValue, int iValueSize)
	{
		switch( iOptionName )
		{
		case SIP_GLOBAL_DEBUG_LEVEL :
			{
				int * iValue = (int *)pValue;
				if( iValue )
				{
					osip_trace_enable_until_level((osip_trace_level_t)*iValue);
				}
				return eSIP_RET_SUCCESS;
			}
		break;
		default:
		break;
		}
		if( iCnnId<0 )
		{
			return eSIP_RET_E_SNNEXIST;
		}
		

		CGSAutoReaderMutex rlocker( &m_wrMutex);
		CClientInfo *pClient = FindClientOfCnnId(iCnnId);
		if( !pClient )
		{
			return eSIP_RET_E_SNNEXIST;
		}
	

		switch(iOptionName)
		{
		case SIP_SESSION_O_UNREGISTER :
			{
				int * iValue = (int *)pValue;
				if( iValue )
				{
					pClient->m_bOOffUnresiter = *iValue==0 ? 1 : 0;
				}
			}
			break;
		default:
			{
				return eSIP_RET_E_NFUNC;
			}
		}
		return eSIP_RET_SUCCESS;
	}

void CSipServer::SetOSipMessageContent(const StruSipData *pSipData, osip_message_t *pOSipMsg)
{
	if( (pSipData->iContentLength >0 || pSipData->eContentType) && pSipData->eContentType != eSIP_CONTENT_DATE
		&& pSipData->eContentType != eSIP_CONTENT_INNER )
	{
		if( pSipData->iContentLength >0 )
		{
			osip_message_set_body (pOSipMsg, 
			pSipData->vContent, pSipData->iContentLength);			
		}
		osip_message_set_content_type (pOSipMsg,
			CvtContentTypeI2Str(pSipData->eContentType));
	}
	else
	{
		osip_message_set_content_type (pOSipMsg, SIP_CONTENT_TYPE);
	}



	if( pSipData->stDialog.szContactHost[0]!='\0' )
	{
		osip_contact_t *pOld = NULL;
		osip_message_get_contact(pOSipMsg, 0, &pOld);
		if( pOld )
		{
			osip_list_remove(&pOSipMsg->contacts, 0);
			osip_contact_free(pOld);
		}
		osip_message_set_contact(pOSipMsg,pSipData->stDialog.szContactHost );
	}

	if( pSipData->stDialog.szSubject[0] != '\0' )
	{
		osip_message_set_header(pOSipMsg, "Subject",  pSipData->stDialog.szSubject);
	}
}

EnumSipErrorCode  CSipServer::SipSessionSendMessage(long iCnnId,	
									   const StruSipData *pSendData, 
									   StruSipData *pRes,int iTimeouts, 
									   StruSipDialogKey *pOutDlgKey)
{
	if( iCnnId<0 )
	{
		return eSIP_RET_E_SNNEXIST;
	}
	m_wrMutex.LockReader();
	CClientInfo *pClient = FindClientOfCnnId(iCnnId);
	if( !pClient )
	{
		m_wrMutex.UnlockReader();
		return eSIP_RET_E_SNNEXIST;		
	}
	pClient->m_tvLastActive = DoGetTickCount();
	pClient->Ref();
	CClientInfo::CAutoUnref csAuto(pClient);
	m_wrMutex.UnlockReader();
	int res = 1;

	if( !pClient->m_bRegisted )
	{
		if( !(pSendData->eDataType == eSIP_DATA_RESPONSE || pSendData->eMethod == eSIP_METHOD_REGISTER) )
		{
			return eSIP_RET_E_STATUS;
		}
	}

	if( pSendData->eDataType==eSIP_DATA_RESPONSE )
	{
		//回复包
		
		osip_message_t *answer = NULL;
		int iSipCodeStatus = SIP_RESPONSE_CODE_SUCCESS;
		if( pSendData->stResponseResult.bOk )
		{
			if( pSendData->eMethod == eSIP_METHOD_INVITE )
			{
// 				eXosip_lock (m_hEXosip);
// 				eXosip_call_send_answer (pSendData->stDialog.iTransId, 
// 							180, NULL, m_hEXosip);
// 				eXosip_unlock (m_hEXosip);
				((StruSipData*)pSendData)->stDialog.szContactHost[0]='\0';

			}
		}
		else
		{
			iSipCodeStatus = SIP_RESPONSE_CODE_FAILURE;
		}

		eXosip_lock (m_hEXosip);
		res = eXosip_call_build_answer(pSendData->stDialog.iTransId, iSipCodeStatus,
			&answer, m_hEXosip);
		if( res )
		{
			eXosip_call_send_answer (pSendData->stDialog.iTransId, 400, NULL, m_hEXosip);
			eXosip_unlock (m_hEXosip);
			return MAKE_OSIP_ERRNO(res);
		}
		
		if( pSendData->eMethod == eSIP_METHOD_REGISTER  )
		{
			//注册回复， 添加 日期
			StruSysTime stTm;
			bzero(&stTm, sizeof(stTm));
			DoGetLocalTime(&stTm);
			char czData[60];
			GS_SNPRINTF(czData, 60, "%04d-%02d-%02dT%02d:%02d:%02d.%d",
					stTm.wYear, stTm.wMonth, stTm.wDay, stTm.wHour, stTm.wMinute, stTm.wSecond,
					 (int)((100.0L*stTm.wMilliseconds)/1000.0) );
			osip_message_set_date(answer, czData);
			((StruSipData*)pSendData)->stDialog.szContactHost[0]='\0';
		}
		SetOSipMessageContent(pSendData, answer);
		res = eXosip_call_send_answer(pSendData->stDialog.iTransId, 
			iSipCodeStatus,answer, m_hEXosip);		
		eXosip_unlock(m_hEXosip);

		if( res==OSIP_WRONG_STATE )
		{
			res = 0;
		}

		if( res )
		{
			return MAKE_OSIP_ERRNO(res);
		}
	}
	else
	{
		eXosip_lock (m_hEXosip);
		//Request
		EnumSipErrorCode eCode = eSIP_RET_FAILURE;
		osip_message_t *pRequest = NULL;
		switch( pSendData->eMethod )
		{
		case eSIP_METHOD_CANCEL :
 		case eSIP_METHOD_BYE :
			{
				res = eXosip_call_terminate(pSendData->stDialog.iCallId, 
					pSendData->stDialog.iDialogId, m_hEXosip);
				eXosip_unlock (m_hEXosip);
				if( res )
				{
					return MAKE_OSIP_ERRNO(res);
				}

				if(pRes )
				{
					bzero(pRes, sizeof(*pRes));
					memcpy(&pRes->stDialog, &pSendData->stDialog,  sizeof(*pOutDlgKey)  );
					pRes->eMethod = eSIP_METHOD_BYE;
					pRes->eDataType = eSIP_DATA_RESPONSE;
					pRes->stResponseResult.bOk = TRUE;
					pRes->stResponseResult.iSipErrno = 200;
					strncpy( pRes->stResponseResult.szMessage, "OK", 3);

				}
				if( pOutDlgKey )
				{
					memcpy(pOutDlgKey, &pSendData->stDialog, sizeof(*pOutDlgKey) );
				}				
				return eSIP_RET_SUCCESS;
			}
 		break;
		case eSIP_METHOD_INVITE :
			eCode = BuildInvite(pClient,pSendData, &pRequest);
		break;
		case eSIP_METHOD_ACK :
			eCode = BuildAck(pClient,pSendData, &pRequest);
		break;
		case eSIP_METHOD_SUBSCRIBE :
			eCode = BuildSubscribe(pClient,pSendData, &pRequest);
		break;

		case eSIP_METHOD_INFO :	
			{
				eCode = BuildInfo(pClient,pSendData, &pRequest);
			}
		break;
		case eSIP_METHOD_NOTIFY :			
		default :
			eCode = BuildNormal(pClient,pSendData,  pSendData->eMethod , &pRequest);
		break;
		}

		if( eCode )
		{
			eXosip_unlock (m_hEXosip);

			return eCode;
		}
		// 设置 Tag
		if(  pSendData->stDialog.iDialogId < 1 && pSendData->stDialog.iTransId < 1)
		{
			//非对话
			RequestResetFromTag(pRequest, pClient);
		}

		//发送消息
		eCode = SendSyncRequest(pClient,pSendData->stDialog, 
			pSendData->eMethod, pRequest, 
				pRes, iTimeouts, pOutDlgKey );
		return eCode;

	}

	return eSIP_RET_SUCCESS;
}

EnumSipErrorCode   CSipServer::BuildNormal(CClientInfo *pClient,
							  const StruSipData *pSendData,EnumSipMethod eMethod,
							  osip_message_t **ppOSipMsg)
{
	int res = 1;
	osip_message_t *pMsg = NULL;
	if( pSendData->stDialog.iDialogId > 0 )
	{
		res = eXosip_call_build_request(pSendData->stDialog.iDialogId, 
		CvtMethodI2Str(pSendData->eMethod ), &pMsg, m_hEXosip);
		if (!res)
		{
			SetOSipMessageContent(pSendData, pMsg);
			*ppOSipMsg = pMsg;
			return eSIP_RET_SUCCESS;

		}
	}
	
	CGSString strTo;
	CGSString strFrom;
	CGSString strRoute;


	if( pSendData->stDialog.szTo[0] !='\0' )
	{
		strTo = pSendData->stDialog.szTo;
	}
	else
	{

		GSStrUtil::Format(strTo, "sip:%s@%s:%d",
			pClient->m_strRemoteSipSrvName.c_str(),
			pClient->m_strRemoteIp.c_str(), 
			pClient->m_iRemotePort);
	}

	if( pSendData->stDialog.szFrom[0] !='\0' )
	{
		strFrom = pSendData->stDialog.szFrom;
	}
	else
	{
		if( pClient->m_bTypeClient )
		{
			GSStrUtil::Format(strFrom, "sip:%s@%s:%d", 
				pClient->m_strUsername.c_str(),
				pClient->m_strLocalIp.c_str(), pClient->m_iLocalPort);
		}
		else
		{
			GSStrUtil::Format(strFrom, "sip:%s@%s:%d", 
				pClient->m_strLocalSrvName.c_str(),
				pClient->m_strLocalIp.c_str(), pClient->m_iLocalPort);
		}
	}

	GSStrUtil::Format(strRoute, "<sip:%s@%s:%d;lr>",
		pClient->m_strRemoteSipSrvName.c_str(),
		pClient->m_strRemoteIp.c_str(), 
		pClient->m_iRemotePort);

	if( /*eMethod == eSIP_METHOD_MESSAGE && */pSendData->stDialog.szContactHost[0]=='\0' )
	{
			
		GS_SNPRINTF((char*) pSendData->stDialog.szContactHost, 127,  "<sip:%s@%s:%d>",
			pClient->m_strUsername.c_str(),
			pClient->m_strRemoteIp.c_str(), pClient->m_iRemotePort );
	}

	if( pSendData->stDialog.szFrom[0] =='\0' )
	{

		res = eXosip_message_build_request(&pMsg, CvtMethodI2Str(pSendData->eMethod), 
			strTo.c_str(), strFrom.c_str(), strRoute.c_str(), m_hEXosip);
	}
// 	else
// 		{
// 			res = eXosip_message_build_request(&pMsg, CvtMethodI2Str(pSendData->eMethod), 
// 				strFrom.c_str(), strTo.c_str(), strRoute.c_str(), m_hEXosip);
// 		}

	if( res )
	{
		return MAKE_OSIP_ERRNO(res);
	}
	if( pSendData->stDialog.iCSeq > 0 )
	{
		CGSString strCSeq =  GSStrUtil::ToString( pSendData->stDialog.iCSeq);
		if( pMsg->cseq )
		{
			if( pMsg->cseq->number )
			{
				osip_free(pMsg->cseq->number);
			}
			osip_cseq_set_number( pMsg->cseq, osip_strdup(strCSeq.c_str()) );
		}
	}
	SetOSipMessageContent(pSendData, pMsg);
	*ppOSipMsg = pMsg;
	return eSIP_RET_SUCCESS;
}

EnumSipErrorCode  CSipServer::BuildSubscribe(CClientInfo *pClient,
								 const StruSipData *pSendData, 
								 osip_message_t **ppOSipMsg)
{
	int res = 0;
	osip_message_t *pMsg = NULL;
	CGSString strTo;
	CGSString strFrom;
	GSStrUtil::Format(strTo, "sip:%s@%s:%d",
		pClient->m_strRemoteSipSrvName.c_str(),
		pClient->m_strRemoteIp.c_str(), 
		pClient->m_iRemotePort);

	GSStrUtil::Format(strFrom, "sip:%s@%s:%d", 
		pClient->m_strUsername.c_str(),
		pClient->m_strLocalIp.c_str(), pClient->m_iLocalPort);


	res = eXosip_subscribe_build_initial_request(&pMsg,strTo.c_str(),strFrom.c_str(),
		NULL,"presence", pSendData->stDialog.iExpirse, m_hEXosip);
	if (res)
	{
		return eSIP_RET_BUILD_SUBSCRIBE_FAILURE;
	}
	SetOSipMessageContent(pSendData, pMsg);
	*ppOSipMsg = pMsg;
	return eSIP_RET_SUCCESS;
}

EnumSipErrorCode CSipServer::BuildAck(CClientInfo *pClient,const StruSipData *pSendData, 
						  osip_message_t **ppOSipMsg)
{
	int res = 0;
	osip_message_t *pMsg = NULL;

	res = eXosip_call_build_ack (pSendData->stDialog.iDialogId,
					&pMsg, m_hEXosip);
	if (res)
	{
		return eSIP_RET_BUILD_ACK_FAILURE;
	}
	SetOSipMessageContent(pSendData, pMsg);
	*ppOSipMsg = pMsg;
	return eSIP_RET_SUCCESS;
}

EnumSipErrorCode CSipServer::BuildInfo(CClientInfo *pClient,const StruSipData *pSendData, 
						   osip_message_t **ppOSipMsg)
{
	int res = 0;
	osip_message_t *pMsg = NULL;

	res = eXosip_call_build_info(pSendData->stDialog.iDialogId,
		&pMsg, m_hEXosip);
	if (res)
	{
		return eSIP_RET_BUILD_ACK_FAILURE;
	}
	SetOSipMessageContent(pSendData, pMsg);
	*ppOSipMsg = pMsg;
	return eSIP_RET_SUCCESS;
}

EnumSipErrorCode  CSipServer::BuildInvite(CClientInfo *pClient,
							  const StruSipData *pSendData, 
							  osip_message_t **ppOSipMsg)
{
	int res = 0;
	osip_message_t *invite = NULL;

	CGSString strTo;
	CGSString strFrom;
	CGSString strRoute;

	if( pSendData->stDialog.szTo[0] !='\0' )
	{
		strTo = pSendData->stDialog.szTo;
	}
	else
	{

		GSStrUtil::Format(strTo, "sip:%s@%s:%d",
			pClient->m_strRemoteSipSrvName.c_str(),
			pClient->m_strRemoteIp.c_str(), 
			pClient->m_iRemotePort);
	}

	if( pSendData->stDialog.szFrom[0] !='\0' )
	{
		strFrom = pSendData->stDialog.szFrom;
	}
	else
	{
		if( pClient->m_bTypeClient )
		{
			GSStrUtil::Format(strFrom, "sip:%s@%s:%d", 
				pClient->m_strUsername.c_str(),
				pClient->m_strLocalIp.c_str(), pClient->m_iLocalPort);
		}
		else
		{
			GSStrUtil::Format(strFrom, "sip:%s@%s:%d", 
				pClient->m_strLocalSrvName.c_str(),
				pClient->m_strLocalIp.c_str(), pClient->m_iLocalPort);
		}
	}

	GSStrUtil::Format(strRoute, "<sip:%s@%s:%d;lr>",
		pClient->m_strRemoteSipSrvName.c_str(),
		pClient->m_strRemoteIp.c_str(), 
		pClient->m_iRemotePort);

	 res = eXosip_call_build_initial_invite (&invite, 
		strTo.c_str(), strFrom.c_str(), strRoute.c_str(),
		NULL, 
		m_hEXosip );

	if (res)
	{
		return eSIP_RET_BUILD_INVITE_FAILURE;
	}
	SetOSipMessageContent(pSendData, invite);
	*ppOSipMsg = invite;
	return eSIP_RET_SUCCESS;
}



EnumSipErrorCode CSipServer::SendSyncRequest(CClientInfo *pClient,
									const StruSipDialogKey &stRemoteKey,
								   EnumSipMethod eMethod, osip_message_t *pMsg,
								 StruSipData *pRes,int iTimeouts, 
								 StruSipDialogKey *pOutDlgKey )
{
StruSipDialogKey stKey;

		if( pOutDlgKey == NULL )
		{
			pOutDlgKey = &stKey;
		}
		bzero(pOutDlgKey, sizeof(stKey) );

		CResponseWaiter *pCond = NULL;
		if( pRes )
		{
			//需要等待返回		
			pCond = PrepareWaitResponse(pMsg, eMethod );
			if( pCond==NULL )
			{
				eXosip_unlock (m_hEXosip);				
				osip_message_free(pMsg);				
				return eSIP_RET_OSIP_E_OPER;
			}
		}
		int res;
		

		SetDlgKeyMember(pMsg, *pOutDlgKey);
		pClient->SetDlgKeyMember(*pOutDlgKey);

		EnumSipErrorCode eCode = eSIP_RET_SUCCESS;
		switch( eMethod )
		{
		case eSIP_METHOD_INVITE :
			{


				pOutDlgKey->iCallId = eXosip_call_send_initial_invite(pMsg, m_hEXosip);
				//返回的是CallID
				if(pOutDlgKey->iCallId<0 )
				{
					eCode = MAKE_OSIP_ERRNO(pOutDlgKey->iCallId);
				}
				else
				{
					//获取dialog ID
					eXosip_call_t *pJc;
					res =  eXosip_call_find(pOutDlgKey->iCallId ,&pJc, m_hEXosip);
					if( res )
					{

						eCode =  MAKE_OSIP_ERRNO(res);
					}
					else if( pJc->c_dialogs )
					{
						pOutDlgKey->iDialogId = pJc->c_dialogs->d_id;
					}
				}
			}
			break;
		case eSIP_METHOD_ACK :
			{

				res = eXosip_call_send_ack(stRemoteKey.iDialogId, pMsg, m_hEXosip);
				if( res )
				{

					eCode =  MAKE_OSIP_ERRNO(res);
				}
				else
				{

					pOutDlgKey->iDialogId = stRemoteKey.iDialogId;
					pOutDlgKey->iCallId = stRemoteKey.iCallId;
				}
			}
		break;
		case eSIP_METHOD_MESSAGE :
			{
				res = eXosip_message_send_request(pMsg, m_hEXosip);
				if( res )
				{

					eCode =  MAKE_OSIP_ERRNO(res);
				}
				else
				{

					pOutDlgKey->iDialogId = stRemoteKey.iDialogId;
					pOutDlgKey->iCallId = stRemoteKey.iCallId;
				}

			}
		break;
		case eSIP_METHOD_INFO :
// 		case eSIP_METHOD_OPTIONS :
// 		case eSIP_METHOD_REFER :
// 		case eSIP_MEDHOD_UPDATE :
		case eSIP_METHOD_NOTIFY  :
		case eSIP_METHOD_SUBSCRIBE :
		case eSIP_METHOD_CANCEL :
		
		default :
			{
				//INVITE, OPTIONS, INFO, REFER, UPDATE
				res = eXosip_call_send_request(stRemoteKey.iDialogId, pMsg, m_hEXosip);
				if( res )
				{

					eCode =  MAKE_OSIP_ERRNO(res);
				}
				else
				{

					pOutDlgKey->iDialogId = stRemoteKey.iDialogId;
					pOutDlgKey->iCallId = stRemoteKey.iCallId;
				}

			}			
			break;
		}

		eXosip_unlock (m_hEXosip);	
		if( eCode )
		{
			//失败
			if( pCond )
			{
				CancelWaitResponse(pCond);

			}
		} 
		else if( pCond )
		{
			eCode = WaitResponse(pCond, pRes, iTimeouts);	

		}
		return eCode;

}



void CSipServer::Unresigter( int iCnnId )
{
	m_wrMutex.LockReader();
	CClientInfo *pClient = FindClientOfCnnId(iCnnId);
	if( !pClient || pClient->m_iExpiresRegister==0 )
	{
		m_wrMutex.UnlockReader();
		return;
	}
	if( pClient->m_bTypeClient && !pClient->m_bOOffUnresiter )
	{
		if( pClient->m_bRegisted )
		{
			CGSString strUsername = pClient->m_strUsername;
			CGSString strPasswrd = pClient->m_strPasswrd;
			m_wrMutex.UnlockReader();
			StruSipData stRes;
			bzero(&stRes, sizeof(stRes));
			SipSessionResigter(iCnnId, strUsername.c_str(),strPasswrd.c_str(), &stRes, 1000, 0 );
		}
	}
	else
	{		
		m_wrMutex.UnlockReader();
	}
	return;
}

EnumSipContentType CSipServer::GetOSipMessageContentType(const osip_message_t *pOSipMsg, 
																CGSString *pStrContentTypeName)
{
	EnumSipContentType eContentType = eSIP_CONTENT_NONE;	
	if(pOSipMsg && pOSipMsg->content_type)
	{
		char *szContentType = NULL;
		if( 0 == osip_content_type_to_str(pOSipMsg->content_type, &szContentType) )
		{
			if( pStrContentTypeName )
			{
				*pStrContentTypeName = szContentType;
			}
			eContentType = CvtContentTypeStr2I(szContentType);
			osip_free(szContentType);
		}		
	}
	return eContentType;
}

void CSipServer::HandleRequest(eXosip_event_t *pXEvent,  EnumSipMethod eMethod )
{
int iCnnId;
	
	CClientConnecter cnner;
	iCnnId = cnner.Parser(pXEvent->request, true);
	if(!iCnnId)
	{
		GS_ASSERT(0);
		SendSimpleResponse(pXEvent, SIP_RESPONSE_CODE_BAD_REQUEST, 
			"Bad Sip Header", NULL , NULL);
		return;
	}



	m_wrMutex.LockReader();
	CClientInfo *pClient = FindClientOfNet(cnner);
	StruSessionPri *pSnPri = NULL;	
	if( pClient )
	{
		iCnnId = pClient->m_iCnnID;
		pClient->m_tvLastActive = DoGetTickCount();
		
	}	
	else
	{
		m_wrMutex.UnlockReader();
		SIP_DEBUG_PRINT( "Error Sip Request client not register.\n");
		SendSimpleResponse(pXEvent, SIP_RESPONSE_CODE_FORBIDDEN, "Unauthorized", NULL, pClient); //没有注册
		return;
	}
	pClient->Ref();
	CClientInfo::CAutoUnref csAuto(pClient);
	m_wrMutex.UnlockReader();
	
	pSnPri = pClient->m_pSnPri;
	CGSString strContentType;
	EnumSipContentType eContentType = GetOSipMessageContentType(pXEvent->request, &strContentType);
	
	switch( eContentType )
	{
	case eSIP_CONTENT_SDP :
		{
			//点流
			SIP_DEBUG_PRINT( "sdp request....\n");
		}	
	break;

	case eSIP_CONTENT_MANSRTSP :
	case eSIP_CONTENT_MANSCDP_XML :
		{
			//控制命令
		}

	break;
	default :
		{
			if( eMethod != eSIP_METHOD_BYE )
			{

			SIP_DEBUG_PRINT( "Error Sip Request unknown content type: %s\n",
				strContentType.empty() ? "null" : strContentType.c_str() );
			SendSimpleResponse(pXEvent, SIP_RESPONSE_CODE_NOTALLOWED,
								"Method Not Allowed",NULL,  pClient ); //没有注册
			GS_ASSERT(0);
			
			return;
			}
		}
	break;
	}

	

	CGSString strTo;
	if( /*eSIP_CONTENT_MANSCDP_XML == eContentType*/
		eMethod == eSIP_METHOD_MESSAGE   )
	{
		//控制命令 先回复 200 OK
		
		SendSimpleResponse(pXEvent, 200, "OK", &strTo, pClient ); //没有注册
	}

	StruSipData stSipData;
	bzero(&stSipData, sizeof(stSipData));

	stSipData.eContentType = eContentType;	
	stSipData.eMethod = eMethod;

	InitSipData(iCnnId, pXEvent,eSIP_DATA_REQUEST, stSipData);	
	if( /*stSipData.stDialog.szTo[0]=='\0' && */ strTo.length() > 1 )
	{
		strncpy( stSipData.stDialog.szTo,  strTo.c_str(), 255 );
	}
	GS_ASSERT( m_stListener.OnSIPPacketEvent );
	
	m_stListener.OnSIPPacketEvent((SipServiceHandle)this, (SipSessionHandle)pSnPri, &stSipData);
}



void CSipServer::HandleResponseWithRegister( eXosip_event_t *pXEvent )
{
	int iCnnId;
	INT iExpiresRegister = m_iExpiresRegister;

	CClientConnecter cnner;
	iCnnId = cnner.Parser(pXEvent->response ? pXEvent->response : pXEvent->request, false);
	if(!iCnnId)
	{

		GS_ASSERT(0);
		SendSimpleResponse(pXEvent, SIP_RESPONSE_CODE_BAD_REQUEST, 
			"Bad Sip Header", NULL, NULL );
		return;
	}

	m_wrMutex.LockWrite();
	CClientInfo *pClient = FindClientOfNet(cnner);
	if( pClient )
	{
		iCnnId = pClient->m_iCnnID;
		iExpiresRegister = pClient->m_iExpiresRegister;
		pClient->m_tvRegisted= time(NULL);
		pClient->m_tvLastActive = DoGetTickCount();

	}	
	else
	{
		SIP_DEBUG_PRINT( "Warnning Sip Response client not exist.\n");
		m_wrMutex.UnlockWrite();
		return;
	}
	osip_message_t *pResponse = pXEvent->response;

	if( pResponse == NULL )
	{
		m_wrMutex.UnlockWrite();
		return;
	}


	char *pTemp = NULL;

	pClient->m_bRegisted = pResponse->status_code == 200 && pClient->m_iExpiresRegister>0 ;	
	if( pResponse->status_code == SIP_RESPONSE_CODE_UNAUTHORIZED )
	{
		pClient->m_bRegisted = FALSE;
		osip_www_authenticate* pAuth = NULL;
		//osip_authorization_t
		osip_message_get_www_authenticate(pResponse,0, &pAuth);
		if( pAuth != NULL)
		{
			if( osip_www_authenticate_get_realm(pAuth) )
			{
				pClient->m_strRealm = osip_www_authenticate_get_realm(pAuth);
			}

			if( osip_www_authenticate_get_nonce(pAuth) )
			{
				pClient->m_strNonce = osip_www_authenticate_get_nonce(pAuth);
			}
		}
		else
		{
			GS_ASSERT(0);
		}


		eXosip_lock(m_hEXosip);
		int iRet;
		osip_message_t *pRegMsg = NULL;
		CGSString strHa1 = "1234567890abcdef1234567890abcdef";
		eXosip_clear_authentication_info(m_hEXosip); 	
		eXosip_add_authentication_info(pClient->m_strUsername.c_str(),
						pClient->m_strUsername.c_str(),pClient->m_strPasswrd.c_str(),
						/*strHa1.c_str()*/ NULL, pClient->m_strRealm.c_str(),
						m_hEXosip);
		iRet = eXosip_register_build_register(pXEvent->rid,iExpiresRegister, &pRegMsg, m_hEXosip);
		if( iRet == OSIP_SUCCESS )
		{	
			iRet = eXosip_register_send_register( pXEvent->rid, pRegMsg, m_hEXosip ); 
			if( iRet == OSIP_SUCCESS )
			{
				eXosip_unlock(m_hEXosip);
				m_wrMutex.UnlockWrite();
				return;
			}
		}
		eXosip_unlock(m_hEXosip);	
		//GS_ASSERT(0);
		SIP_DEBUG_PRINT("error : Reg failure of Response 401\n");
		StruSipData stESipData;
		bzero(&stESipData, sizeof(stESipData));
		stESipData.eContentType = eSIP_CONTENT_NONE;	
		stESipData.eMethod = eSIP_METHOD_REGISTER;
		InitSipData(iCnnId, pXEvent,eSIP_DATA_RESPONSE, stESipData);
		stESipData.stResponseResult.bOk= FALSE;
		stESipData.stResponseResult.iSipErrno = SIP_RESPONSE_CODE_UNAUTHORIZED;
		WakeupWaitResponse(stESipData.eMethod, pXEvent, &stESipData );
		return;
	}
	

	if( pResponse && pResponse->call_id && 0==osip_call_id_to_str(pResponse->call_id, &pTemp))
	{
		pClient->m_strCallId = pTemp;
		osip_free(pTemp);
	}
	
	if( pClient->m_strRemoteSipSrvName.empty() )
	{
		if( pResponse && pResponse->from &&
			osip_from_get_displayname(pResponse->from) )
		{
			pClient->m_strRemoteSipSrvName = osip_from_get_displayname(pResponse->from);
		}
		else
		{
			osip_uri_t *pUri = osip_from_get_url(pResponse->from);
			if( osip_uri_get_username(pUri) )
			{
				pClient->m_strRemoteSipSrvName = osip_uri_get_username(pUri);
			}
			else if( osip_uri_get_host(pUri) )
			{
				pClient->m_strRemoteSipSrvName = osip_uri_get_host(pUri);
			}
			else
			{
				GS_ASSERT(0);
				pClient->m_strRemoteSipSrvName = "Unknow";
			}
		}
	}
	


	m_wrMutex.UnlockWrite();

	


	StruSipData stSipData;
	bzero(&stSipData, sizeof(stSipData));

	stSipData.eContentType = eSIP_CONTENT_NONE;	
	stSipData.eMethod = eSIP_METHOD_REGISTER;

	InitSipData(iCnnId, pXEvent,eSIP_DATA_RESPONSE, stSipData);	

	////Date: 2012-12-13T15:35:48.518
	//获取登陆时间
	osip_header_t *pParam = NULL;
	if(pXEvent->response && osip_message_header_get_byname(pXEvent->response, "Date", 0, &pParam) )
	{
		if( pParam && pParam->hvalue)
		{
				
			strncpy((char*)stSipData.vContent, pParam->hvalue, SIP_MAX_CONTENT_LEN );
			pTemp = strchr((char*)stSipData.vContent, 'T' );
			if( pTemp )
			{
				stSipData.eContentType = eSIP_CONTENT_DATE;
				*pTemp = ' ';
			}
			pTemp = strchr((char*)stSipData.vContent, '.' );
			if( pTemp )
			{
				*pTemp = '\0';
			}
			stSipData.iContentLength = strlen((char*)stSipData.vContent);		
		}
	}

	WakeupWaitResponse(stSipData.eMethod, pXEvent, &stSipData );

	
}


void CSipServer::SendSimpleResponse(eXosip_event_t *pXEvent, int iSipErrno, 
						const char *czMessage ,CGSString *pTo, CClientInfo *pClient)
{
	int res = -1;

	osip_message_t *answer = NULL;


	eXosip_lock (m_hEXosip);
	res = eXosip_call_build_answer(pXEvent->tid , iSipErrno,&answer, m_hEXosip);
	if ( res == 0 )
	{
// 		if( pClient )
// 		{
// 			ResponseResetTag(answer, pClient);
// 		}
		if( pTo )
		{
			if( answer->to )
			{
				char *czTemp = NULL;
				osip_to_to_str( answer->to, &czTemp);
				if( czTemp )
				{
					*pTo = czTemp;
					osip_free(czTemp);
				}
			}
		}

		eXosip_call_send_answer(pXEvent->tid, iSipErrno,answer, m_hEXosip);
	}
	eXosip_unlock (m_hEXosip);

	res = 0;

}

void CSipServer::SendResponseUnauthorized(eXosip_event_t *pXEvent, CClientConnecter *pCnner)
{
	int res = -1;

	osip_message_t *answer = NULL;

	CGSString strRealm;
	CGSString strNonce;
	
	m_wrMutex.LockReader();
	
	CClientInfo *pClient = NULL;

	if( pCnner )
	{
		pClient = FindClientOfNet(*pCnner);

	}

	if( pClient )
	{
		strNonce = pClient->m_strNonce;
		strRealm = pClient->m_strRealm;
	}
	else
	{
		strNonce = GetNewNonce();
		strRealm = GetNewRealm();
	}
	m_wrMutex.UnlockReader();
	

	osip_www_authenticate_t *oswwwAuth = NULL;
	CGSString strWWWAuth;
	osip_www_authenticate_init(&oswwwAuth);
	if( oswwwAuth == NULL )
	{
		GS_ASSERT(0);
		return;
	}
	osip_www_authenticate_set_auth_type(oswwwAuth, osip_strdup( "Digest") );
	osip_www_authenticate_set_realm(oswwwAuth, osip_strdup(strRealm.c_str()) );
	osip_www_authenticate_set_nonce(oswwwAuth, osip_strdup(strNonce.c_str()) );

	char *czTemp=NULL;

	osip_www_authenticate_to_str(oswwwAuth, &czTemp);
	
	osip_www_authenticate_free(oswwwAuth);
	if( NULL== czTemp )
	{
		GS_ASSERT(0);
		return;		
	}
	strWWWAuth = czTemp;
	osip_free(czTemp);

	
	eXosip_lock (m_hEXosip);
	res = eXosip_call_build_answer(pXEvent->tid , SIP_RESPONSE_CODE_UNAUTHORIZED,&answer, m_hEXosip);
	if ( res == 0 )
	{
// 		if( pClient)
// 		{
// 			ResponseResetTag(answer, pClient);
// 		}
		osip_message_set_www_authenticate(answer, strWWWAuth.c_str() );
		eXosip_call_send_answer(pXEvent->tid, SIP_RESPONSE_CODE_UNAUTHORIZED,answer, m_hEXosip);
	}
	else
	{
		GS_ASSERT(0);
	}
	eXosip_unlock (m_hEXosip);

	res = 0;
}


void CSipServer::HandleResponse(eXosip_event_t *pXEvent,  EnumSipMethod eMethod )
{
	int iCnnId;
	CClientConnecter cnner;
	iCnnId = cnner.Parser(pXEvent->response ? pXEvent->response : pXEvent->request, false);
	if(!iCnnId)
	{

		GS_ASSERT(0);
		SendSimpleResponse(pXEvent, SIP_RESPONSE_CODE_BAD_REQUEST, 
			"Bad Sip Header", NULL, NULL );
		return;
	}

	m_wrMutex.LockReader();
	CClientInfo *pClient = FindClientOfNet(cnner);
	if( pClient )
	{
		pClient->m_tvLastActive = DoGetTickCount();
		iCnnId = pClient->m_iCnnID;
	}
	else
	{
		m_wrMutex.UnlockReader();
		SIP_DEBUG_PRINT( "Error Sip Response client not register or released.\n");				
		return;
	}	
	pClient->Ref();
	CClientInfo::CAutoUnref csAuto(pClient);

	m_wrMutex.UnlockReader();
	StruSessionPri *pSnPri = pClient->m_pSnPri;

	CGSString strContentType;
	EnumSipContentType eContentType = GetOSipMessageContentType(pXEvent->request, &strContentType);
	if( eMethod == eSIP_METHOD_RESPONSE )
	{
		eMethod = CvtMethodStr2I(pXEvent->request->sip_method);
	}
	switch( eContentType )
	{
	case eSIP_CONTENT_NONE :
	case eSIP_CONTENT_SDP :
	case eSIP_CONTENT_MANSCDP_XML :
	case eSIP_CONTENT_MANSRTSP :
	break;
	default :
		{
			SIP_DEBUG_PRINT( "Error Sip Response unknown content type: %s\n",
				strContentType.empty() ? "null" : strContentType.c_str() );			
	//		GS_ASSERT(0);
			return;
		}
		break;
	}

	//唤醒等待的线程

	StruSipData stSipData;
	bzero(&stSipData, sizeof(stSipData));

	stSipData.eContentType = eContentType;	
	stSipData.eMethod = eMethod;

	InitSipData(iCnnId, pXEvent,eSIP_DATA_RESPONSE, stSipData);	

	if( !WakeupWaitResponse(eMethod, pXEvent, &stSipData )  && m_stListener.OnSIPPacketEvent )
	{
		m_stListener.OnSIPPacketEvent((SipServiceHandle)this, (SipSessionHandle)pSnPri, &stSipData);
	}
}

void CSipServer::DeleteClient(long iCnnId)
{
	SipSessionRelease(iCnnId);
}

CSipServer::CResponseWaiter *CSipServer::PrepareWaitResponse(const osip_message_t *posipMsg,
						EnumSipMethod eMethod )
{
	//准备等待返回
	char *szTemp = NULL;
	CGSString strCallId;
	UINT32 iCSeq=0;
	if( osip_call_id_to_str(posipMsg->call_id, &szTemp ) )
	{
		GS_ASSERT(0);		
		return NULL;
	}
	strCallId = szTemp;
	osip_free(szTemp);

	szTemp = osip_cseq_get_number(posipMsg->cseq);
	if( !szTemp )
	{
		GS_ASSERT(0);
		return NULL;
	}
	iCSeq = GSStrUtil::ToNumber<UINT32>(szTemp);


	osip_uri_param_t *pTagParam = NULL;
	char *czTag = NULL;
	if( !osip_from_get_tag(posipMsg->from, &pTagParam) )
	{
		czTag = pTagParam->gvalue;
	}

	CSipServer::CResponseWaiter *p = CResponseWaiter::Create(eMethod, strCallId, iCSeq, czTag);
	if( p )
	{
		m_setWaiter.insert(p);
	}
	return p;
}

void CSipServer::CancelWaitResponse(CResponseWaiter *pWaiter)
{
	m_setWaiter.erase(pWaiter);
	pWaiter->Unref();
}

EnumSipErrorCode CSipServer::WaitResponse(CResponseWaiter *pWaiter, StruSipData *pResult, int iTimeouts )
{
EnumSipErrorCode eRet =	pWaiter->WaitTimeout(pResult, iTimeouts);
	m_wrMutex.LockWrite();
	m_setWaiter.erase(pWaiter);
	m_wrMutex.UnlockWrite();
	pWaiter->Unref();
	return eRet;
}

BOOL CSipServer::WakeupWaitResponse(EnumSipMethod eMethod, const eXosip_event_t *pXEvent, StruSipData *pRes)
{
	const osip_message_t *posipMsg = pXEvent->response;
	if( !posipMsg )
	{
		posipMsg = pXEvent->request;
		return FALSE;
	}
	char *szTemp = NULL;
	CGSString strCallId;
	UINT32 iCSeq=0;
	if( osip_call_id_to_str(posipMsg->call_id, &szTemp ) )
	{
		GS_ASSERT(0);		
		return NULL;
	}
	strCallId = szTemp;
	osip_free(szTemp);

	szTemp = osip_cseq_get_number(posipMsg->cseq);
	if( !szTemp )
	{
		GS_ASSERT(0);
		return NULL;
	}
	iCSeq = GSStrUtil::ToNumber<UINT32>(szTemp);


	osip_uri_param_t *pTagParam = NULL;
	char *czTag = NULL;
	if( !osip_from_get_tag(posipMsg->from, &pTagParam) )
	{
		czTag = pTagParam->gvalue;
	}

	m_wrMutex.LockWrite();
	std::set<CResponseWaiter *>::iterator csIt;
	CResponseWaiter *pWaiter = NULL;
	for( csIt = m_setWaiter.begin(); csIt!=m_setWaiter.end(); ++csIt )
	{
		if( (*csIt)->IsEqual(eMethod,strCallId, iCSeq, czTag ) )
		{
			pWaiter = *csIt;	
			pWaiter->Ref();			
			m_setWaiter.erase(csIt);
			break;
		}
	}
	m_wrMutex.UnlockWrite();
	if( pWaiter )
	{
		pWaiter->Wakeup(pRes);
		pWaiter->Unref();
		return TRUE;
	}
	return FALSE;
	

}


CGSString CSipServer::GetNewRealm(void)
{
	CGSString strRet;
	GSStrUtil::Format(strRet, "\"%u\"", osip_build_random_number() );
	return strRet;
}

CGSString CSipServer::GetNewNonce(void)
{
	CGSString strRet;
	GSStrUtil::Format(strRet, "\"%02x\"", osip_build_random_number() );
	return strRet;
}

CGSString CSipServer::GuessLocalIp( const CGSString &strRemoteIp )
{
	if( strRemoteIp.empty() )
	{
		if( m_vNetIf.size() )
		{
			return m_vNetIf[0].strIp;
		}
		GS_ASSERT(0);
		return CGSString("127.0.0.1");
	}

	
#if defined(WIN32) || defined(_WIN32) || defined(WINCE)
	SOCKET sock_rt;
	int len;
#else
	#define closesocket  close
	int sock_rt;
	unsigned int len;
#endif

	int on = 1;
	struct sockaddr_in iface_out;

	struct sockaddr_in remote;

	memset(&remote, 0, sizeof(struct sockaddr_in));

	remote.sin_family = AF_INET;
	remote.sin_addr.s_addr = inet_addr(strRemoteIp.c_str() );
	remote.sin_port = htons(31111);

	memset(&iface_out, 0, sizeof(iface_out));
	sock_rt = socket(AF_INET, SOCK_DGRAM, 0);

	if (setsockopt(sock_rt, SOL_SOCKET, SO_BROADCAST,(char*) &on, sizeof(on)) == -1) 
	{
		GS_ASSERT(0);
		::closesocket(sock_rt);
		if( m_vNetIf.size() )
		{
			return m_vNetIf[0].strIp;
		}
		GS_ASSERT(0);
		return CGSString("127.0.0.1");
	}

	if (connect(sock_rt, (struct sockaddr *) &remote, sizeof(struct sockaddr_in)) == -1) 
	{
			GS_ASSERT(0);			
			::closesocket(sock_rt);
			if( m_vNetIf.size() )
			{
				return m_vNetIf[0].strIp;
			}
			GS_ASSERT(0);
			return CGSString("127.0.0.1");
	}

	len = sizeof(iface_out);
	if (getsockname(sock_rt, (struct sockaddr *) &iface_out, &len) == -1) 
	{
		GS_ASSERT(0);		
		::closesocket(sock_rt);
		return CGSString("127.0.0.1");
	}

	::closesocket(sock_rt);
	if (iface_out.sin_addr.s_addr == 0) 
	{	/* what is this case?? */
		GS_ASSERT(0);		
		if( m_vNetIf.size() )
		{
			return m_vNetIf[0].strIp;
		}
		GS_ASSERT(0);
		return CGSString("127.0.0.1");
	}
	return CGSString( inet_ntoa(iface_out.sin_addr) ) ;
}

BOOL CSipServer::LoadLocalIf(void)
{
	

#ifdef _WIN32
	WSADATA wsaData;
	INT iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != NO_ERROR)
	{
		GS_ASSERT(0);
		return FALSE;
	}
#endif

	StruNetIf stIf;

	//获取本地IP列表

#if defined(_WIN32) || defined(WIN32) || defined(_WINCE)

#ifdef _WINCE

	SOCKET tempSocket = ::WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, 0);
	if (tempSocket != INVALID_SOCKET)
	{          
		//tempSocket = ::socket(AF_INET, SOCK_DGRAM, 0);

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

		struct sockaddr_in* theAddr;
		for (UINT i = 0;
			i < iNumIPAddrs; i++)
		{						
			theAddr = (struct sockaddr_in*)&addrListP[i].iiAddress;
			stIf.strIp = ::inet_ntoa(theAddr->sin_addr);
			theAddr =  (struct sockaddr_in*)&addrListP[i].iiNetmask;
			stIf.strNetmask = ::inet_ntoa(theAddr->sin_addr);
			m_vNetIf.push_back(stIf);
		}

	}
#else  
	//2003 

	PIP_ADAPTER_INFO pAdapterInfo;
	pAdapterInfo = (IP_ADAPTER_INFO *) ::malloc( sizeof(IP_ADAPTER_INFO) );
	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);

	// Make an initial call to GetAdaptersInfo to get
	// the necessary size into the ulOutBufLen variable
	if ( ::GetAdaptersInfo( pAdapterInfo, &ulOutBufLen) != ERROR_SUCCESS) 
	{
		::free (pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *) ::malloc (ulOutBufLen);
	}

	if ( NO_ERROR == GetAdaptersInfo( pAdapterInfo, &ulOutBufLen) ) 
	{
		PIP_ADAPTER_INFO pAdapter  =  pAdapterInfo;
		
		while (pAdapter) 
		{
					
			stIf.strIp = pAdapter->IpAddressList.IpAddress.String;
			stIf.strNetmask = pAdapter->IpAddressList.IpMask.String;			
			pAdapter = pAdapter->Next;
			m_vNetIf.push_back(stIf);
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
	struct sockaddr_in* addrPtr;
	for( int i = 0; i<iCnts;  i++, ifr++ )
	{

		if (ifr->ifr_addr.sa_family == AF_INET)
		{
			

			addrPtr = (struct sockaddr_in*)&ifr->ifr_addr;  
			stIf.strIp = ::inet_ntoa(addrPtr->sin_addr);

			addrPtr = (struct sockaddr_in*)&ifr->ifr_netmask;  
			stIf.strNetmask = ::inet_ntoa(addrPtr->sin_addr);

			m_vNetIf.push_back(stIf);
		}
	}

#endif

	//
	// If LocalHost is the first element in the array, switch it to be the second.
	// The first element is supposed to be the "default" interface on the machine,
	// which should really always be en0.

	if ((m_vNetIf.size() > 1) && m_vNetIf[0].strIp == "127.0.0.1" )
	{
		
		stIf = m_vNetIf[0];
		m_vNetIf[0] = m_vNetIf[1];
		m_vNetIf[1] = stIf;
	}
	return TRUE;

}

CGSString CSipServer::HostName2Ip(const CGSString &strHost)
{
	CGSString strRet;
	const char *czHost = strHost.c_str();
	void *our_s_addr;	// Pointer to sin_addr or sin6_addr

	struct sockaddr_in sin;

	struct hostent *hp=NULL;
	GS_ASSERT_RET_VAL( czHost, strRet );

	bzero(&sin, sizeof(sin));
	our_s_addr = (void *) & sin.sin_addr;	

#ifdef _WIN32	
	if ( inet_addr(czHost)==INADDR_NONE ) {
#else
	if( 0==inet_aton(czHost,(struct in_addr*) our_s_addr) ) {
#endif

#ifdef HAVE_GETHOSTBYNAME2
		hp=(struct hostent*)gethostbyname2( czHost, AF_INET );
#else
		hp=(struct hostent*)gethostbyname( czHost );
#endif
		GS_ASSERT_RET_VAL(hp, strRet);
		memcpy( our_s_addr, (void*)hp->h_addr_list[0], hp->h_length );

#ifdef _WIN32
		strRet =  inet_ntoa( sin.sin_addr);  //inet_ntoa  windows下线程安全， linux 不安全
#else
		char czIP[64];
		int len = 64;
		if(inet_ntop(AF_INET, &sin.sin_addr, czIP,len  ) )
		{
			strRet = czIP;
		}
#endif
	}   
	else  
	{
		strRet = strHost;
	}
	return strRet;
}