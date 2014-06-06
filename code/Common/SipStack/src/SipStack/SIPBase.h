/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : SIPBASE.H
Author :  邹阳星
Version : 1.0.0.1
Date: 2012/8/16 16:46
Description: SIP协议命令处理，底层使用exosip库命令;	
			也应该建立一个存储队
*			列，在heart中不断取出数据进行内部发送
********************************************
*/

#ifndef _GS_H_SIPBASE_H_
#define _GS_H_SIPBASE_H_

namespace GSP
{


#include <eXosip2/eXosip.h>
#include <stdio.h>
#include <stdlib.h>
#include "SIPCommonDef.h"



#define ALLOW_HEAD		"INVITE, ACK, CANCEL, OPTIONS, BYE, REFER, NOTIFY, MESSAGE, SUBSCRIBE, INFO"
#define SUPPORTED_HEAD	"eventlist"


class ISIPListener : public CGSObject
{
public:
	virtual int OnClientConnectEvent(int iEngineId, int iClientId, const CGSString &strIP, int port) = 0;

	virtual int OnClientDisconnectEvent(int iEngineId, int iClientId) = 0;

	//给上层的回调皆用此接口 是网络请求的回调还是应答回调根据 packet类型决定
	virtual int OnSIPPacketEvent(int iEngineId, int iConn, CSIPPacket* pdu) = 0;
};

class CSIPBase  : public CGSObject
{

protected:
	/*!
	* \brief
	* 上层主动并发送网络成功的命令,并等待网络另一端的应答;
	*
	* \remarks
	* 键值为exosip中的命令的标志值callId（如果是会话内命令宜用Cseq）
	*/
	CSentPacketMap	m_mapSendQueue; //等待发送的数据队列
	CGSMutex m_csSendQueueMutex; // m_mapSendQueue 锁


	/*!
	* \brief
	* 上层回调指针
	*/
	ISIPListener* _listener;

	int			m_id;
	EnumConnectType		eConnectType;

public:

	CSIPBase(int iID, EnumConnectType eCnnType, ISIPListener* pListener);
	virtual ~CSIPBase(void);

	virtual EnumErrorCode Connect( const char* czHost, int iPort) = 0;
	virtual EnumErrorCode Connect(const char* czLocalIp, int iLocalPort, 
								  const char* czRemoteIp, int iRemotePort) = 0;
	virtual EnumErrorCode Start( const char* czIp, int port) = 0;

	//sand命令，直接发送;并根据发cseq存储packet;上层会根据请求的sequence建立他的索引
	//heartbeat中受到应答，根据cseq找到 请求的packet;生成应答packet应答APP
	virtual EnumErrorCode SendPacket(int iConn, CSIPPacket* pPacket) = 0;
	virtual EnumErrorCode Heartbeat( int iTimeout = 0);
	virtual EnumErrorCode Close(void);

protected:
	virtual EnumErrorCode CheckNetReqConnSession(const eXosip_event_t* pEXOEvt,
													OUT UINT32 &iConnId ) = 0;
	virtual EnumErrorCode CheckNetResConnSession(const eXosip_event_t* pEXOEvt,
													OUT UINT32 &iConnId ) = 0;

public:
	EnumErrorCode	RemoveSentPdu(const CGSString &strSequence);
	static EnumErrorCode ParseIpAndPortFromAddress( const char* czAddress,OUT CGSString &strIp,OUT int & port);

protected:
	/**
	*@brief 发送SIP命令，命令类型根据函数名区分
	*@param conn	无效id
	*@param	pdu		sip命令交互包
	*/
	int SendInvite(int iConn, CSIPPacket* pPacket);	
	int SendInfo(int iConn, CSIPPacket* pPacket);
	int SendNotify(int iConn, CSIPPacket* pPacket);
	int SendSubscribe(int iConn, CSIPPacket* pPacket);//
	int SendBye(int iConn, CSIPPacket* pPacket);//应该不需要再处理应答了
	int SendCancel(int iConn, CSIPPacket* pPacket);

	/**
	*@brief	发送默认的sip命令，不依赖于某个会话
	*@note	区别于上面的分类命令，这里的命令主要是发送会话外命令，即不基于Invite的Info和Notify命令.
	*@param conn	暂时无效
	*@param pdu		sip命令包
	*/
	int SendRequestEx(int iConn, CSIPPacket* pdu);

	/**
	*@brief	发送默认的sip命令的应答
	*@see	sendRequestEx
	*@note	区别于上面的分类命令，这里的命令主要是发送会话外命令，即不基于Invite的Info和Notify命令.
	*@param conn	暂时无效
	*@param pdu		sip命令包
	*/
	int SendRequestExRes(int conn, CSIPPacket* pdu);

	/**
	*@brief 发送应答
	*@param conn	无效id
	*@param	pdu		sip命令交互包
	*/
	int SendInviteRes(int conn, CSIPPacket* pdu);	
	int SendInfoRes(int conn, CSIPPacket* pdu);
	int SendNotifyRes(int conn, CSIPPacket* pdu);
	int SendSubscribeRes(int conn, CSIPPacket* pdu);

	//回调 从网络而来的 命令
	int BuildAndSendRes(eXosip_event_t* envent, osip_body_t* body, int bSuc = 1);	
	int BuildAndSendReq(eXosip_event_t* envent, osip_body_t* body, int sipType);

	int FetchAddreInfoFromReq(eXosip_event_t* envent,CSIPPacket* sipReq);
	int FetchAddreInfoFromRes(eXosip_event_t* envent,CSIPPacket* sipRes);

	int FetchAuthorization(eXosip_event_t* pEvent,CSIPPacket* sipReq);
	int BuildAndSendRegReq(eXosip_event_t* envent);
	int BuildAndSendRegRes(eXosip_event_t* pEvent);
	int FetchWWWAuthenticateFromRes(eXosip_event_t* envent,CSIPPacket* sipRes);

	int AddSentPdu(std::string sequence, CSIPPacket* packet );
	int DelSentPdu(std::string  sequence );
	CSIPPacket* FetchSentPdu(std::string sequence );
	CSIPPacket* GetSentPdu(std::string sequence );
	int ClearSentPduMap();

	void AddAllowHeadToSipMsg(osip_message *pSipMsg, const char *pContent = ALLOW_HEAD);
	void AddSupportedHeadToSipMsg(osip_message *pSipMsg, const char *pContent = SUPPORTED_HEAD);



};



} //end namespace GSP

#endif //end _GS_H_SIPBASE_H_
