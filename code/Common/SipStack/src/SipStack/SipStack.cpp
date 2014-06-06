#include "SipStack.h"
#include "SipServer.h"
using namespace GSSIP;


#define PriSipSrv(_x) ((CSipServer *)_x)
#define CheckInvalidHandle( _x, _retcode )  if( _x == NULL ) { return _retcode; }

#define PriSipSession(_x) ((StruSessionPri*)_x)


int SipDialogKey_Cmp(const StruSipDialogKey *pSrc,const StruSipDialogKey *pDest )
{
	int iRet; 
	iRet = strncmp(pSrc->czDialogKey, pDest->czDialogKey, SIP_MAX_DIGID_STRING );
	if( iRet )
	{
		return iRet;
	}
	

	if( pSrc->iCnnId > pDest->iCnnId )
	{
		return 1;
	} 
	if( pSrc->iCnnId<pDest->iCnnId )
	{
		return -1;
	}
	if( pSrc->iDialogId > pDest->iDialogId )
	{
		return 1;
	}

	if( pSrc->iDialogId < pDest->iDialogId )
	{
		return -1;
	}

	if( pSrc->iCallId > pDest->iCallId )
	{
		return 1;
	}

	if( pSrc->iCallId < pDest->iCallId )
	{
		return -1;
	}
	return 0;
}


/*
*********************************************************************
*
*@brief :  SipService 的实现
*
*********************************************************************
*/


SipServiceHandle SipService_Create(const StruSipListener *pListener)
{
	CSipServer *pSrv = new CSipServer();
	if( pSrv )
	{
		if( pSrv->Init(*pListener) )
		{
			delete pSrv;
			pSrv = NULL;
		}
	}
	return  pSrv;
}

void SipService_Release(SipServiceHandle hSipService)
{
	if( PriSipSrv(hSipService) )
	{
		PriSipSrv(hSipService)->Stop();
		delete PriSipSrv(hSipService);
	}
}




void SipService_SetUserData(SipServiceHandle hSipService, void *pUserData )
{
	if( PriSipSrv(hSipService) )
	{
		PriSipSrv(hSipService)->SetUserData(pUserData);
	}
}

void *SipService_GetUserData(SipServiceHandle hSipService)
{
	CheckInvalidHandle(hSipService, NULL)	
	return 	PriSipSrv(hSipService)->GetUserData();
}

 EnumSipErrorCode SipService_Set_ServerName(SipServiceHandle hSipService, const char *czServerName)
 {
	 CheckInvalidHandle(hSipService, eSIP_RET_E_INVALID)	
		  	PriSipSrv(hSipService)->SetServerName(czServerName);
	 return eSIP_RET_SUCCESS;
 }

 EnumSipErrorCode SipService_Set_Gateway(SipServiceHandle hSipService, const char *czGatewayIPV4)
 {
	 CheckInvalidHandle(hSipService, eSIP_RET_E_INVALID)	
		 PriSipSrv(hSipService)->SetGateway(czGatewayIPV4);
	 return eSIP_RET_SUCCESS;
 }

EnumSipErrorCode SipService_Start(SipServiceHandle hSipService,
								  EnumSipConnectType eCnnType,
								  const char *czLocalIp, int iLocalPort)
{
	CheckInvalidHandle(hSipService, eSIP_RET_E_INVALID)
		return PriSipSrv(hSipService)->Start(eCnnType,czLocalIp,iLocalPort);

}

int SipService_Get_LocalPort(SipServiceHandle hSipService)
{
	CheckInvalidHandle(hSipService, -1)
		return PriSipSrv(hSipService)->GetLocalPort();
}

 void SipService_Stop(SipServiceHandle hSipService)
 {
	 if( PriSipSrv(hSipService) )
	 {
		 PriSipSrv(hSipService)->Stop();
	 }
 }

 SipSessionHandle    SipService_Connect( SipServiceHandle hSipService,
	 EnumSipConnectType eCnnType, 
	 const char *czRemoteHost,int iRemotePort, const char *czRemoteSipServerName )
 {
	 CheckInvalidHandle(hSipService, NULL)

	 StruSessionPri *pSession = NULL;

	 if( PriSipSrv(hSipService)->Connect(eCnnType,czRemoteHost,iRemotePort, czRemoteSipServerName, &pSession ) )
	 {		
		 return NULL;
	 }	
	 return (SipSessionHandle)pSession;
 }

 EnumSipErrorCode SipService_GetListenInfo(SipServiceHandle hSipService, StruSipConnnectInfo *pRes)
 {
	 //TODO
	 return eSIP_RET_E_NFUNC;
 }


 /*
 *********************************************************************
 *
 *@brief :  SipSession 的实现
 *
 *********************************************************************
 */
void SipSession_SetUserData(SipSessionHandle hSipSession, void *pUserData )
{
	if(  PriSipSession(hSipSession) )
	{
		PriSipSession(hSipSession)->pUserData = pUserData;
	}
}

 void *SipSession_GetUserData(SipSessionHandle hSipSession)
 {
	 CheckInvalidHandle(hSipSession, NULL)
	return  PriSipSession(hSipSession)->pUserData; 
 }


 EnumSipErrorCode SipSession_Resigter(SipSessionHandle hSipSession,
	 const char *czUserName, 
	 const char *czPassword,
	 StruSipData *pRes, int iTimeout )
 {
	 CheckInvalidHandle(hSipSession, eSIP_RET_E_INVALID)
	 StruSessionPri *pPri = PriSipSession(hSipSession);
	 return  pPri->pSrv->SipSessionResigter(pPri->iCnnId, czUserName, czPassword, pRes, iTimeout); 
 }

 EnumSipErrorCode SipSession_Unresigter(SipSessionHandle hSipSession,
	 StruSipData *pRes, int iTimeout )
 {
	 CheckInvalidHandle(hSipSession, eSIP_RET_E_INVALID)
		 StruSessionPri *pPri = PriSipSession(hSipSession);
	 return  pPri->pSrv->SipSessionResigter(pPri->iCnnId,NULL, NULL, pRes, iTimeout, 0);
 }

 EnumSipErrorCode SipSession_Disconnect(SipSessionHandle hSipSession)
 {
	 CheckInvalidHandle(hSipSession, eSIP_RET_E_INVALID)
	 StruSessionPri *pPri = PriSipSession(hSipSession);
	 return  pPri->pSrv->SipSessionDisconnect(pPri->iCnnId); 
 }

 void SipSession_Release(SipSessionHandle hSipSession)
 {
	 StruSessionPri *pPri = PriSipSession(hSipSession);
	 if( pPri )
	 {
		 pPri->pSrv->SipSessionRelease(pPri->iCnnId);
		 pPri->iCnnId = 0;
		 CSipServer *pSrv = pPri->pSrv;
		 pSrv->FreeSession(pPri);
	 }
 }

 EnumSipErrorCode SipSession_SendMessage(SipSessionHandle hSipSession,			   
	 const StruSipData *pSendData, StruSipData *pRes,
	 int iTimeouts,  StruSipDialogKey *pOutDlgKey)
 {
	 CheckInvalidHandle(hSipSession, eSIP_RET_E_INVALID)
	 StruSessionPri *pPri = PriSipSession(hSipSession);
	 return pPri->pSrv->SipSessionSendMessage(pPri->iCnnId,pSendData, pRes, iTimeouts, pOutDlgKey );
 }

 EnumSipErrorCode SipSession_GetConnectInfo(SipSessionHandle hSipSession, 
	 StruSipConnnectInfo *pRes)
 {
	 CheckInvalidHandle(hSipSession, eSIP_RET_E_INVALID)
		 StruSessionPri *pPri = PriSipSession(hSipSession);
	 return pPri->pSrv->GetConnectInfo(pPri->iCnnId,pRes);
 }

 SipServiceHandle SipSession_GetService(SipSessionHandle hSipSession)
 {
	StruSessionPri *pPri = PriSipSession(hSipSession);
	if( pPri )
	{
		return (pPri->pSrv);
	}
	GS_ASSERT(0);
	return NULL;
 }

 EnumSipErrorCode SipSession_Reconnect(SipSessionHandle hSipSession)
 {
	 CheckInvalidHandle(hSipSession, eSIP_RET_E_INVALID)
		 StruSessionPri *pPri = PriSipSession(hSipSession);
	 return pPri->pSrv->SipSessionReconnect(pPri->iCnnId);
 }

 const char *SipSession_Authorization_Username(SipSessionHandle hSipSession)
 {
	 CheckInvalidHandle(hSipSession, NULL)
		 StruSessionPri *pPri = PriSipSession(hSipSession);
	 return pPri->pSrv->SipSessionAuthorizationUsername(pPri->iCnnId);
 }

 EnumSipErrorCode SipSession_Authorize(SipSessionHandle hSipSession, 
	 const char *czUsername, const char *czPasswrd )
 {
	 CheckInvalidHandle(hSipSession, eSIP_RET_E_INVALID)
	 StruSessionPri *pPri = PriSipSession(hSipSession);
	 return pPri->pSrv->SipSessionCmpAuthorize(pPri->iCnnId, czUsername, czPasswrd );
 }


 EnumSipErrorCode SipSession_SetOptions(SipSessionHandle hSipSession, 
	 int iOptionName, const void *pValue, int iValueSize )
 {
	 CheckInvalidHandle(hSipSession, eSIP_RET_E_INVALID)
		 StruSessionPri *pPri = PriSipSession(hSipSession);
	 return pPri->pSrv->SipSessionSetOptions(pPri->iCnnId, iOptionName,(const char*) pValue,iValueSize );
 }
