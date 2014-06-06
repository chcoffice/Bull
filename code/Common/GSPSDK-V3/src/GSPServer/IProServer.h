/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : IPROSERVER.H
Author :  zouyx
Version : 0.0.0.0
Date: 2012/3/19 19:02
Description: 各个协议的服务接口
********************************************
*/

#ifndef _GS_H_IPROSERVER_H_
#define _GS_H_IPROSERVER_H_
#include "GSPObject.h"
#include "Uri.h"
#include "IServer.h"

namespace GSP
{
class CServer;
class CISrvSession;

class CIProServer :
	public CGSPObject
{
public :
	CServer *m_pServer;
protected :
	EnumProtocol m_eProtocol;
	CGSPString m_stClientIP;
	CGSPString m_strListenIP;
	INT m_iListenPort;
	
public:	
	CIProServer(EnumProtocol ePro)
		:CGSPObject()
		,m_eProtocol(ePro)
	{
		m_pServer = NULL;
		m_stClientIP = "";
		m_strListenIP = "0";
		m_iListenPort = -1;
	}
	virtual ~CIProServer(void)
	{

	}

	virtual EnumErrno Init( CServer *pServer ) = 0;
	virtual void Unint(void) = 0;

	virtual EnumErrno Start(void) = 0;

	virtual void Stop(void) = 0;


	INLINE EnumProtocol Protocol(void) const
	{
		return m_eProtocol;
	}
	
	const CGSPString &ListenIPInfo(void) const
	{
		return m_stClientIP;
	}

	const CGSPString &ListenIP(void) const
	{
		return m_strListenIP;
	}
	
	const INT ListenPort(void) const
	{
		return m_iListenPort;
	}
	


};

class CISrvSession : public CGSPObject
{
private :
	static GSAtomicInter s_iAutoIDSequence; 
protected :	
	const UINT32 m_iAutoID;
	CGSPString m_strClientIPInfo;
	CIProServer *m_pProServer;
	StruClientInfo m_stClientInfo;
public :
	 const CGSPString &ClientIPInfo(void) const
	 {
		 return m_strClientIPInfo;
	 }
	 
	 const StruClientInfo *ClientInfo(void) const
	 {
		 return &m_stClientInfo;
	 }

	
// 	 CIUri &RequestUri(void)
// 	 {
// 		 return m_csURI;
// 	 }

	 INLINE CIProServer *ProServer(void)
	 {
		 return m_pProServer;
	 }
	 virtual void Start(void) = 0;
	 virtual void DeleteBefore(void) = 0; //删除前调用
	 virtual ~CISrvSession(void)
	 {

	 }
protected :
	 CISrvSession(CIProServer *pProServer);
	
	 
};


} //end namespace GSP

#endif //end _GS_H_IPROSERVER_H_
