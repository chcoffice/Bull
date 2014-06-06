#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SipStack.h"


SipServiceHandle _g_uc = SIP_INVALID_HANDLE;
SipServiceHandle _g_ucs = SIP_INVALID_HANDLE;

static EnumSipErrorCode OnClientConnectEvent( SipServiceHandle hService,
										 SipSessionHandle hNewSession )
{
	printf("**** Rcv %s Connect\n", hService==_g_ucs ? "UCS" : "UC" );
	return eSIP_RET_SUCCESS;
}

//断开连接
static void OnClientDisconnectEvent(SipServiceHandle hService, SipSessionHandle hNewSession)
{
	printf("**** %s Disconnect\n", hService==_g_ucs ? "UCS" : "UC" );
}

//给上层的回调皆用此接口 是网络请求的回调还是应答回调根据 pData类型决定
static void  OnSIPPacketEvent(SipServiceHandle hService, 
						 SipSessionHandle hNewSession,
						 StruSipData *pData)
{
	printf("**** %s RcvPacket.\n", hService==_g_ucs ? "UCS" : "UC" );
	if( pData->iContentLength>0  )
	{
		printf("\r\n *****************\r\n" );
		printf("%s", pData->vContent );
		printf("\r\n *****************\r\n" );
	}

	if( pData->eMethod == eSIP_METHOD_REGISTER && pData->eDataType == eSIP_DATA_REQUEST   )
	{
		StruSipData stRes  = *pData;
		stRes.eContentType =eSIP_CONTENT_NONE;
		stRes.iContentLength = 0;
		stRes.eDataType = eSIP_DATA_RESPONSE;
		stRes.stResponseResult.bOk = 1;
		stRes.stResponseResult.iSipErrno = 200;
		stRes.stResponseResult.szMessage[0] = '\0';
		SipSession_SendMessage(hNewSession, &stRes, NULL, 0, NULL);

	}
}


static void UCEntry(void)
{
	printf( "Program run at UC....\r\n");
	StruSipListener stListen;
	SipSessionHandle hClient = SIP_INVALID_HANDLE;
	stListen.OnClientConnectEvent = OnClientConnectEvent;
	stListen.OnClientDisconnectEvent = OnClientDisconnectEvent;
	stListen.OnSIPPacketEvent = OnSIPPacketEvent;

	_g_uc = SipService_Create(&stListen);
	StruSipData res;
	if( _g_uc == SIP_INVALID_HANDLE )
	{
		printf("SipService_Create fail.\n");
	}
	else
	{


		if( SipService_Start(_g_uc, eSIP_CONNECT_UDP, NULL, 5060))
		{
			printf("Start service fail.\n");
		
		}
		else
		{
			SipService_Set_ServerName(_g_uc, "sip_srv_test" );
			hClient = SipService_Connect(_g_uc,eSIP_CONNECT_UDP,
				"192.168.33.59", 5060, NULL );
			if( SIP_INVALID_HANDLE==hClient )
			{
				printf("err : connect ucs fail.\n");
			}
			else
			{
				
				if( SipSession_Resigter(hClient, "34010000002000000001", "12345678", &res, 10 ) )
				{
					int i3;
				}
				int i2;

			}
			
		}
	}

	int x;

	do 
	{
		x = getchar();
	} while ( x!='q' && x!='Q');

	if( hClient != SIP_INVALID_HANDLE)
	{
		SipSession_Disconnect(hClient);		
		SipSession_Release(hClient);
	}
	if( _g_uc!=SIP_INVALID_HANDLE )
	{
		SipService_Stop(_g_uc);
		SipService_Release(_g_uc);
		_g_ucs = SIP_INVALID_HANDLE;
	}
}

static void UCSEntry(void)
{
	printf( "Program run at UCS....\r\n");
	StruSipListener stListen;	
	stListen.OnClientConnectEvent = OnClientConnectEvent;
	stListen.OnClientDisconnectEvent = OnClientDisconnectEvent;
	stListen.OnSIPPacketEvent = OnSIPPacketEvent;

	_g_ucs = SipService_Create(&stListen);

	if( _g_ucs == SIP_INVALID_HANDLE )
	{
		printf("UCS SipService_Create fail.\n");
	}
	else
	{


		if( SipService_Start(_g_ucs, eSIP_CONNECT_UDP, "192.168.15.142", 5060) )
		{
			printf("UCS SipService_Start fail.\n");	
		}
	}

	printf( "***Input 'q/Q' key to exist  ucs.\n");
	int x;
	do 
	{
		x = getchar();
	} while ( x!='q' && x!='Q');

	if( _g_ucs!=SIP_INVALID_HANDLE )
	{
		SipService_Stop(_g_ucs);
		SipService_Release(_g_ucs);
		_g_ucs = SIP_INVALID_HANDLE;
	}
}

int main( int argc, const char **argv)
{
	printf( "Select Program run mode: s/S is UCS  other is UC\r\n"  );
	int c = getchar();

	if(  c=='s' || c=='S' )
	{
			UCSEntry();
	}
	else 
	{
		UCEntry();
	}
	printf( "***Input 'q/Q' key to exist  ucs.\n");
	int x;
	do 
	{
		x = getchar();
	} while ( x!='q' && x!='Q');
	return 0;
}