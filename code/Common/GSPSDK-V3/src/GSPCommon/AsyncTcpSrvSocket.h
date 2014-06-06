/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : ASYNCTCPSRVSOCKET.H
Author :  ×ÞÑôÐÇ
Version : 1.0.0.1
Date: 2013/7/24 14:22
Description: Òì²½TCP ·þÎñÆ÷
********************************************
*/

#ifndef _GS_H_ASYNCTCPSRVSOCKET_H_
#define _GS_H_ASYNCTCPSRVSOCKET_H_

#include "ISocket.h"


namespace GSP
{
class CAsyncTcpSrvSocket;

class CAsyncTcpSrvEventContext : public CRefObject
{
public :	
	static CAsyncTcpSrvEventContext *Create(void);
	virtual EnumErrno Init(void) = 0;
	virtual void Unint(void) = 0;
	virtual CAsyncTcpSrvSocket *Create(const char *czBindLocalIp, INT iListenPort ) = 0;
protected :
	CAsyncTcpSrvEventContext(void) : CRefObject()
	{

	}
	virtual ~CAsyncTcpSrvEventContext(void)
	{

	}
};


class CAsyncTcpSrvSocket : public CISocket
{
public :
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

	virtual EnumErrno AsyncAccept( BOOL bStart )
	{
		return  eERRNO_SUCCESS;
	}

	virtual void Disconnect(void)
	{

	}
protected :
	CAsyncTcpSrvSocket(void) : CISocket((EnumSocketType)(eSOCKET_SERVER|eSOCKET_TCP))
	{

	}
	virtual ~CAsyncTcpSrvSocket(void)
	{

	}
};


} //end namespace GSP


#endif //end _GS_H_ASYNCTCPSRVSOCKET_H_
