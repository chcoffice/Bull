#include "SIPBase.h"
using namespace GSSIP;

CSIPBase::CSIPBase(int id, int connect_type, ISIPListener* listener)
{

	_id = id;
	_connect_type = connect_type;
	_listener = listener;

	eXosip_init();
}

CSIPBase::~CSIPBase(void)
{
	eXosip_quit();
	close();
}



int CSIPBase::close(void)
{
	clearSentPduMap();
	return 0;
}


int CSIPBase::sendInvite(int conn, CSIPPacket* pdu)
{
	int res = 0;
	osip_message_t *invite = NULL;

	char  tmpTo[MAX_ADDRESS_NUM] = {0};
	char  tmpFrom[MAX_ADDRESS_NUM] = {0};
	sprintf(tmpFrom,"sip:%s@%s",pdu->_strFromCode,pdu->_strFromAdr);
	sprintf(tmpTo,"sip:%s@%s",pdu->_strToCode,pdu->_strToAdr);

	//fixme 后面两个参数的用处
	res = eXosip_call_build_initial_invite (&invite, tmpTo, tmpFrom, NULL, NULL);
	if (res != 0)
	{
		return SIP_BUILD_INVITE_FAILURE;
	}

	if ( pdu->_body != NULL )
	{
		osip_message_set_body (invite, pdu->_body, pdu->_bodyCapacity);
	}
	osip_message_set_content_type (invite, SIP_CONTENT_TYPE);

	//gaowei的代码还有部分代码我没有加
	eXosip_lock ();
	res = eXosip_call_send_initial_invite (invite);
	eXosip_unlock ();

	if ( res >= 0 )  // res == 0
	{
		//success
		res = 0;

		//由于exosip库对会话外命令的 cseq的问题（此种命令发送 cseq均为20）
		//所以这里不能再以cseq作为参数值了而要使用 call id
		//char* cseq = invite->cseq->number;
		char* cseq = invite->call_id->number;
		//uint64 sequence = AX_OS::atoul64(cseq);                       //modify lj uint32 atol(cseq
		addSentPdu(cseq,pdu);
	}
	else
	{
		res = SIP_SEND_INVITE_FAILURE;
	}

	return res;

}

int CSIPBase::sendInfo(int conn, CSIPPacket* pdu)
{

	CSIPRequest* pduReq = (CSIPRequest*)pdu;

	int res = -1;
	osip_message_t *info = NULL;

	res = eXosip_call_build_info (pduReq->_sipSessionInfo._dlgId, &info);
	if ( res != 0 )
	{
		return SIP_BUILD_INFO_FAILURE;
	}

	if ( pdu->_body != NULL )
	{
		osip_message_set_body (info, pdu->_body, pdu->_bodyCapacity);
	}
	osip_message_set_content_type (info, SIP_CONTENT_TYPE);

	eXosip_lock ();
	res = eXosip_call_send_request(pduReq->_sipSessionInfo._dlgId, info);
	eXosip_unlock ();

	if ( res >= 0 )
	{
		//success
		//char* cseq = info->cseq->number;
		char* cseq = info->call_id->number;
		//uint64 sequence = AX_OS::atoul64(cseq);         //modify lj uint32
		addSentPdu(cseq,pdu);                       
	}
	else
	{
		res = SIP_SEND_INFO_FAILURE;
	}

	return res;
}

int CSIPBase::sendNotify(int conn, CSIPPacket* pdu)
{
	CSIPRequest* pduReq = (CSIPRequest*)pdu;

	int res = -1;
	osip_message_t *notify = NULL;

	//fixme EXOSIP_SUBCRSTATE_ACTIVE这个值是可选的 注意搞清楚
	//res = eXosip_call_build_notify(pduReq->_sipSessionInfo._dlgId,EXOSIP_SUBCRSTATE_ACTIVE, &notify);delete lj 2010-05-25
	res = eXosip_insubscription_build_notify(pduReq->_sipSessionInfo._dlgId, EXOSIP_SUBCRSTATE_ACTIVE, 0, &notify);
	if ( res != 0 )
	{
		return SIP_BUILD_NOTIFY_FAILURE;
	}

	if ( pdu->_body != NULL )
	{
		osip_message_set_body (notify, pdu->_body, pdu->_bodyCapacity);
	}
	osip_message_set_content_type (notify, SIP_CONTENT_TYPE);

	eXosip_lock ();
	//res = eXosip_call_send_request(pduReq->_sipSessionInfo._dlgId, notify); delete lj 2010-05-25
	res = eXosip_insubscription_send_request(pduReq->_sipSessionInfo._dlgId, notify);
	eXosip_unlock ();

	if ( res >= 0 )
	{
		//success
		//char* cseq = notify->cseq->number;
		char* cseq = notify->call_id->number;
		//uint64 sequence = AX_OS::atoul64(cseq);               //modify lj uint32
		addSentPdu(cseq,pdu);               
	}
	else
	{
		res = SIP_SEND_INVITE_FAILURE;
	}

	return res;

}

int CSIPBase::sendSubscribe(int conn, CSIPPacket* pdu)
{
	int res = -1;
	osip_message_t *subscribe = NULL;

	char  tmpTo[MAX_ADDRESS_NUM] = {0};
	char  tmpFrom[MAX_ADDRESS_NUM] = {0};
	sprintf(tmpFrom,"sip:%s@%s",pdu->_strFromCode,pdu->_strFromAdr);
	sprintf(tmpTo,"sip:%s@%s",pdu->_strToCode,pdu->_strToAdr);

	//fixme 后面几个参数都要弄清楚
	res = eXosip_subscribe_build_initial_request(&subscribe,tmpTo,tmpFrom, NULL,"presence", pdu->_subexpires);//modify lj 2010-05-21
	if ( res != 0 )
	{
		return SIP_BUILD_SUBSCRIBE_FAILURE;
	}

	if ( pdu->_body != NULL )
	{
		osip_message_set_body (subscribe, pdu->_body, pdu->_bodyCapacity);
	}
	osip_message_set_content_type (subscribe, SIP_CONTENT_TYPE);

	eXosip_lock ();
	res = eXosip_subscribe_send_initial_request(subscribe);
	eXosip_unlock ();

	if ( res >= 0 )
	{
		//success
		//char* cseq = subscribe->cseq->number;
		char* cseq = subscribe->call_id->number;
		//uint64 sequence = AX_OS::atoul64(cseq);                //modify lj uint32
		addSentPdu(cseq,pdu);                         
	}
	else
	{
		res = SIP_SEND_SUBSCRIBE_FAILURE;
	}

	return res;
}


int CSIPBase::sendBye(int conn, CSIPPacket* pdu)
{
	eXosip_lock();
	int ret = eXosip_call_terminate(pdu->_sipSessionInfo._callId,pdu->_sipSessionInfo._dlgId);
	eXosip_unlock();

	return ret;
}

int CSIPBase::sendCancel(int conn, CSIPPacket* pdu)
{
	return sendBye(conn,pdu);
}


int CSIPBase::sendInviteRes(int conn, CSIPPacket* pdu)
{
	int res = -1;
	osip_message_t *answer = NULL;
	CSIPResponse* pduResponse = (CSIPResponse*)pdu;

	eXosip_lock ();
	if ( pduResponse->_bSuccess != 1 )
	{
		res = eXosip_call_build_answer (pdu->_sipSessionInfo._transId, 400, &answer);
		if ( res == 0 )
		{
			if ( pduResponse->_body != NULL )
			{
				osip_message_set_body (answer, pdu->getBody(), pdu->getBodyCapacity());
			}
			osip_message_set_content_type (answer, SIP_CONTENT_TYPE);
			eXosip_call_send_answer (pdu->_sipSessionInfo._transId, 400, NULL);
		}
	}
	else
	{
		eXosip_call_send_answer (pdu->_sipSessionInfo._transId, 180, NULL);
		res = eXosip_call_build_answer (pdu->_sipSessionInfo._transId, 200, &answer);
		if (res != 0)
		{
			eXosip_call_send_answer (pdu->_sipSessionInfo._transId, 400, NULL);
		}
		else
		{
			if ( pduResponse->_body != NULL )
			{
				osip_message_set_body (answer, pdu->getBody(),pdu->getBodyCapacity());
			}
			osip_message_set_content_type (answer, SIP_CONTENT_TYPE);
			eXosip_call_send_answer (pdu->_sipSessionInfo._transId, 200, answer);
		}
	}
	eXosip_unlock ();
	res = 0;

	return res;
}


int CSIPBase::sendSubscribeRes(int conn, CSIPPacket* pdu)
{
	int res = -1;

	osip_message_t *answer = NULL;
	CSIPResponse* pduResponse = (CSIPResponse*)pdu;

	int statusCode = SIP_NET_CODE_OK;
	if ( pduResponse->_bSuccess == 0 )
	{
		statusCode = SIP_NET_CODE_ERROR;
	}

	eXosip_lock ();
	res = eXosip_insubscription_build_answer(pdu->_sipSessionInfo._transId, statusCode,&answer);
	if ( res == 0 )
	{
		if ( pdu->_body != NULL )
		{
			osip_message_set_body (answer, pdu->getBody(),pdu->getBodyCapacity());
		}
		osip_message_set_content_type (answer, SIP_CONTENT_TYPE);
		eXosip_insubscription_send_answer(pdu->_sipSessionInfo._transId, statusCode,answer);
	}
	eXosip_unlock ();

	res =0;
	return res;
}

int CSIPBase::sendInfoRes(int conn, CSIPPacket* pdu)
{
	int res = 1;

	osip_message_t *answer = NULL;
	CSIPResponse* pduResponse = (CSIPResponse*)pdu;

	int statusCode = SIP_NET_CODE_OK;
	if ( pduResponse->_bSuccess == 0 )
	{
		statusCode = SIP_NET_CODE_ERROR;
	}

	eXosip_lock ();
	res = eXosip_call_build_answer(pdu->_sipSessionInfo._transId, statusCode,&answer);
	if ( res == 0 )
	{
		if ( pduResponse->_body != NULL )
		{
			osip_message_set_body (answer, pdu->getBody(), pdu->getBodyCapacity());
		}
		osip_message_set_content_type (answer, SIP_CONTENT_TYPE);
		eXosip_call_send_answer(pdu->_sipSessionInfo._transId, statusCode,answer);
	}
	eXosip_unlock ();

	res = 0;
	return res;
}

int CSIPBase::sendNotifyRes(int conn, CSIPPacket* pdu)
{
	int res = -1;

	osip_message_t *answer = NULL;
	CSIPResponse* pduResponse = (CSIPResponse*)pdu;

	int statusCode = SIP_NET_CODE_OK;
	if ( pduResponse->_bSuccess == 0 )
	{
		statusCode = SIP_NET_CODE_ERROR;
	}

	eXosip_lock ();
	res = eXosip_call_build_answer(pdu->_sipSessionInfo._transId, statusCode,&answer);
	if ( res == 0 )
	{
		if ( pdu->_body != NULL )
		{
			osip_message_set_body (answer, pdu->getBody(),pdu->getBodyCapacity());
		}
		osip_message_set_content_type (answer, SIP_CONTENT_TYPE);
		eXosip_call_send_answer(pdu->_sipSessionInfo._transId, statusCode,answer);
	}
	eXosip_unlock ();

	res = 0;
	return res;
}



int CSIPBase::sendRequestEx(int conn, CSIPPacket* pdu)
{
	CSIPRequest* pduReq = (CSIPRequest*)pdu;

	int res = -1;
	osip_message_t *message = NULL;

	//拼接方式雷同于Invite
	char  tmpTo[MAX_ADDRESS_NUM] = {0};
	char  tmpFrom[MAX_ADDRESS_NUM] = {0};
	sprintf(tmpFrom,"sip:%s@%s",pdu->_strFromCode,pdu->_strFromAdr);
	sprintf(tmpTo,"sip:%s@%s",pdu->_strToCode,pdu->_strToAdr);

	//build the default msg of sip,the msg type is according to the secord param
	switch(pdu->_method)
	{
	case SIP_NOTIFYEX:
		res = eXosip_message_build_request(&message, NOTIFY_CODE, tmpTo, tmpFrom, NULL);
		break;

	case SIP_INFOEX:
		res = eXosip_message_build_request(&message, INFO_CODE, tmpTo, tmpFrom, NULL);
		break;

	default:
		res = eXosip_message_build_request(&message, MESSAGE_CODE/*INFO_CODE*/, tmpTo, tmpFrom, NULL);
		break;
	}

	if ( res != 0 )
	{
		return SIP_BUILD_DEFAULT_MSG_FAILURE;
	}
	if ( pdu->_body != NULL )
	{
		osip_message_set_body(message, pdu->_body, pdu->_bodyCapacity);
	}
	osip_message_set_content_type(message, SIP_CONTENT_TYPE);

	eXosip_lock ();
	res = eXosip_message_send_request(message);
	eXosip_unlock ();

	if ( res >= 0 )
	{
		//success
		//char* cseq = message->cseq->number;
		char* cseq = message->call_id->number;
		//uint64 sequence = AX_OS::atoul64(cseq); //modify lj uint32
		addSentPdu(cseq,pdu);                             
	}
	else
		res = SIP_SEND_DEFAULT_MSG_FAILURE;

	return res;
}

int CSIPBase::sendRequestExRes(int conn, CSIPPacket* pdu)
{
	int res = 1;

	osip_message_t *answer = NULL;
	CSIPResponse* pduResponse = (CSIPResponse*)pdu;

	int statusCode = SIP_NET_CODE_OK;
	if ( pduResponse->_bSuccess == 0 )
	{
		statusCode = SIP_NET_CODE_ERROR;
	}

	eXosip_lock ();
	res = eXosip_message_build_answer(pdu->_sipSessionInfo._transId, statusCode,&answer);
	if ( res == 0 )
	{
		if ( pduResponse->_body != NULL )
		{
			osip_message_set_body(answer, pdu->getBody(), pdu->getBodyCapacity());
		}
		osip_message_set_content_type (answer, SIP_CONTENT_TYPE);
		res = eXosip_message_send_answer(pdu->_sipSessionInfo._transId, statusCode,answer);
	}
	eXosip_unlock ();

	res = 0;
	return res;
}

int CSIPBase::fetchAddreInfoFromReq(eXosip_event_t* envent,CSIPPacket* sipReq)
{
	if ( sipReq == NULL
		|| envent == NULL 
		|| envent->request == NULL
		|| envent->request->to == NULL 
		|| envent->request->from == NULL
		|| envent->request->from->url == NULL
		|| envent->request->to->url == NULL )
	{
		return -1;
	}

	//from
	if ( envent->request->from->url->username != NULL )
	{
		strncpy(sipReq->_strFromCode,envent->request->from->url->username,sizeof(sipReq->_strFromCode)-1);
	}
	if ( envent->request->from->url->host != NULL )
	{
		strncpy(sipReq->_strFromAdr,envent->request->from->url->host,sizeof(sipReq->_strFromAdr)-1);
		if ( envent->request->from->url->port != NULL )
		{
			sprintf_s(sipReq->_strFromAdr,sizeof(sipReq->_strFromAdr)-1,"%s:%s",sipReq->_strFromAdr,envent->request->from->url->port);
		}
	}

	//to
	if ( envent->request->to->url->username != NULL )
	{
		strncpy(sipReq->_strToCode,envent->request->to->url->username,sizeof(sipReq->_strToCode)-1);
	}
	if ( envent->request->to->url->host != NULL )
	{
		strncpy(sipReq->_strToAdr,envent->request->to->url->host,sizeof(sipReq->_strToAdr)-1);
		if ( envent->request->to->url->port != NULL )
		{
			sprintf_s(sipReq->_strToAdr,sizeof(sipReq->_strToAdr)-1,"%s:%s",sipReq->_strToAdr,envent->request->to->url->port);
		}
	}

	//------add lj 2010---------------------------------------
	osip_header_t* expirse = NULL;
	if(osip_message_get_expires(envent->request,0,&expirse) >= 0)
	{
		sipReq->_subexpires = atoi(expirse->hvalue);
	}
	//--------------------------------------------------


	return 0;

}

int CSIPBase::fetchAddreInfoFromRes(eXosip_event_t* envent,CSIPPacket* sipRes)
{
	if ( sipRes == NULL
		|| envent == NULL 
		|| envent->response == NULL
		|| envent->response->to == NULL 
		|| envent->response->from == NULL
		|| envent->response->from->url == NULL
		|| envent->response->to->url == NULL )
	{
		return -1;
	}

	//from
	if ( envent->response->from->url->username != NULL )
	{
		strncpy(sipRes->_strFromCode,envent->response->from->url->username,sizeof(sipRes->_strFromCode)-1);
	}
	if ( envent->response->from->url->host != NULL )
	{
		strncpy(sipRes->_strFromAdr,envent->response->from->url->host,sizeof(sipRes->_strFromAdr)-1);
		if ( envent->response->from->url->port != NULL )
		{
			sprintf_s(sipRes->_strFromAdr,sizeof(sipRes->_strFromAdr)-1,"%s:%s",sipRes->_strFromAdr,envent->response->from->url->port);
		}
	}

	//to
	if ( envent->response->to->url->username != NULL )
	{
		strncpy(sipRes->_strToCode,envent->response->to->url->username,sizeof(sipRes->_strToCode)-1);
	}
	if ( envent->response->to->url->host != NULL )
	{
		strncpy(sipRes->_strToAdr,envent->response->to->url->host,sizeof(sipRes->_strToAdr)-1);
		if ( envent->response->to->url->port != NULL )
		{
			sprintf_s(sipRes->_strToAdr,sizeof(sipRes->_strToAdr)-1,"%s:%s",sipRes->_strToAdr,envent->response->to->url->port);
		}
	}

	return 0;
}

/*======================================================================
*
* 名称：CSIPBase::buildAndSendRes
* 功能：接收到网络上的应答命令；则要先去对主动发送的send命令的MAP进行查
*		找删除,其索引为send时 SIP协议中的Cseq值；并将网络应答信息封成
*		packet应答到APP层
* 参数：[int bSuc] 	--- 命令是否成功 0为失败，1为成功（缺省）
*
=======================================================================*/ 
int CSIPBase::buildAndSendRes(eXosip_event_t* envent, osip_body_t* body, int bSuc/* = 1*/)
{
	if ( envent == NULL )
	{
		return -1;
	}

	//会话外的发送后 cseq值改用callId 2010-02-08
	//char* cseq = envent->request->cseq->number;
	char* cseq = envent->request->call_id->number;
	//uint64 sequence = AX_OS::atoul64(cseq);              //modify lj 2010-04-21
	//uint64 sequence = AX_OS::atoul64(cseq);

	CSIPPacket* packet = fetchSentPdu(cseq);
	if ( packet == NULL )
	{
		//不应该删除;除非是上层检测请求已超时,主动剔除
		//Log_Printer(6000, "packet == NULL, sequence == %d, cseq = %d", sequence, cseq);
		return -1;
	}

	uint32 connId = 0;
	if ( checkNetResConnSession( envent, connId ) < 0 )
	{
		return -2;
	}

	CSIPResponse* response = NULL;
	if ( body != NULL )
	{
		response = new CSIPResponse((int)body->length);
		response->setBody(body->body);
	}
	else
	{
		response = new CSIPResponse(0);
	}

	response->_sipSessionInfo._callId = envent->cid;
	response->_sipSessionInfo._dlgId = envent->did;
	response->_sipSessionInfo._transId = envent->tid;

	response->_method = packet->_method;
	response->_cmdId = packet->_cmdId;				//收到网络请求时没有用
	response->_bSuccess = bSuc;
	response->setSequence(packet->getSequence());	//对网络上受到的应答进行转换，转换为vplib与sip库之间的sequence
	fetchAddreInfoFromRes(envent,response);

	if ( envent->response != NULL )
	{
		//fixme 这个response怎么会是空的 难道是调试的缘故
		response->_code = envent->response->status_code;
		strncpy(response->_message, envent->response->reason_phrase, sizeof(response->_message)-1);
	}

	response->addRef();
	_listener->onSIPPacket(0,connId,response);
	response->release();

	return 0;
}

/*======================================================================
*
* 名称：CSIPBase::buildAndSendReq
* 功能：接收到网络上的请求命令；则要先去对该命令进行存储,记录下一些信息
*		这些信息组装成struct(可以进行扩充)存在MAP中,键值为本层主动生成递增
*		序列。并将网络请求信息封成packet发送到APP层，等其使用sendPacket发送
*		response命令时，进行检查取出有用的信息，并对map进行清理
* 参数：[int sipType] 	--- sipType指定命令是哪种SIP命令，INVITE INFO... ... 
*
=======================================================================*/ 
int CSIPBase::buildAndSendReq(eXosip_event_t* envent, osip_body_t* body, int sipType)
{
	if ( envent == NULL )
	{
		return -1;
	}

	uint32 connId = 0;
	if ( checkNetReqConnSession( envent, connId ) < 0 )
	{
		//对于应该存在的注册命令如何处理 
		return -2;
	}

	CSIPRequest* request = NULL;
	if ( body != NULL )
	{
		request = new CSIPRequest((int)body->length);
		request->setBody(body->body);
	}
	else
	{
		request = new CSIPRequest(0);
	}

	request->_sipSessionInfo._callId = envent->cid;
	request->_sipSessionInfo._dlgId = envent->did;
	request->_sipSessionInfo._transId = envent->tid;

	//uint32 sequence = getSequence();
	//request->setSequence(sequence);
	request->_method = sipType;
	fetchAddreInfoFromReq(envent,request);

	request->addRef();
	_listener->onSIPPacket(0,connId,request);
	request->release();

	return 0;
}

int CSIPBase::addSentPdu( std::string sequence, CSIPPacket* packet )
{
	_mtxSentPdu.acquire();
	_mapSentPdu[sequence] = packet;
	_mtxSentPdu.release();

	return 0;
}

int	CSIPBase::removeSentPdu(std::string sequence)
{
	return delSentPdu(sequence);
}

int CSIPBase::delSentPdu( std::string sequence )
{
	int res = -1;

	_mtxSentPdu.acquire();
	SentPduMap::iterator it = _mapSentPdu.find(sequence);
	if ( it != _mapSentPdu.end() )
	{
		//最后应答回去由new的地方进行释放
		_mapSentPdu.erase(it);
		res = 0;
	}
	_mtxSentPdu.release();

	return res;
}

CSIPPacket* CSIPBase::getSentPdu( std::string sequence )
{
	CSIPPacket* packet = NULL;

	_mtxSentPdu.acquire();
	SentPduMap::iterator it = _mapSentPdu.find(sequence);
	if ( it != _mapSentPdu.end() )
	{
		packet = it->second;
	}
	_mtxSentPdu.release();

	return packet;

}

CSIPPacket* CSIPBase::fetchSentPdu( std::string sequence )          //modify lj 2010-04-21
{
	CSIPPacket* packet = NULL;

	_mtxSentPdu.acquire();
	SentPduMap::iterator it = _mapSentPdu.find(sequence);
	if ( it != _mapSentPdu.end() )
	{
		packet = it->second;
		_mapSentPdu.erase(it);
	}
	_mtxSentPdu.release();

	return packet;
}

int CSIPBase::clearSentPduMap()
{
	_mtxSentPdu.acquire();
	_mapSentPdu.clear();
	_mtxSentPdu.release();

	return 0;
}

void CSIPBase::AddAllowHeadToSipMsg(osip_message *pSipMsg, const char *pContent)
{
	if(pSipMsg)
		osip_message_set_allow(pSipMsg, pContent);
}

void CSIPBase::AddSupportedHeadToSipMsg(osip_message *pSipMsg, const char *pContent)
{
	if(pSipMsg)
		osip_message_set_supported(pSipMsg, pContent);
}

int CSIPBase::heartbeat(int timeout/* = 0*/)
{
	//侦听消息
	Sleep(105);
	eXosip_event_t *je;
	je = eXosip_event_wait( 0, 1);

	//协议栈带有此语句,具体作用未知
	eXosip_lock ();
	//eXosip_default_action (je);
	eXosip_automatic_refresh ();
	eXosip_unlock ();

	if ( je == NULL )
		return 0;

	osip_body_t *body = NULL;
	switch(je->type)
	{
		//////////////////////////////////////////////////////////////////////////
		//收到网络请求

	case EXOSIP_MESSAGE_NEW:
		{
			//会话外（默认）命令，即eXosip_message_send_request 所发送的命令皆在此处处理

			int nMethod = SIP_REGISTER;
			char* sip_method = je->request->sip_method;
			if ( strcmp( sip_method,INFO_CODE) == 0 )
			{
				nMethod = SIP_INFOEX;
			}
			else if (strcmp( sip_method,NOTIFY_CODE) == 0 )
			{
				nMethod = SIP_NOTIFYEX;
			}
			else if ( strcmp( sip_method,MESSAGE_CODE) == 0 )
			{
				nMethod = SIP_MESSAGE;
			}

			if(nMethod == SIP_REGISTER)
			{
				buildAndSendRegReq(je);
			}
			else
			{
				//fixme what's mean about the secord param
				osip_message_get_body (je->request, 0, &body); 
				buildAndSendReq(je,body,nMethod);
			}		
		}
		break;

	case EXOSIP_CALL_INVITE:
	case EXOSIP_CALL_REINVITE:
		{
			printf ("got a invite\n");
			osip_message_get_body (je->request, 0, &body); 
			buildAndSendReq(je,body,SIP_INVITE);
		}
		break;

	case EXOSIP_CALL_MESSAGE_NEW:
		{
			//会话内的命令
			printf("receive ***CALL_MESSAGE_NEW cmd\n");
			int nMethod = 0;
			char* sip_method = je->request->sip_method;

			if (  strcmp( sip_method,BYE_CODE) == 0 )
			{
				nMethod = SIP_BYE;
			}
			if (  strcmp( sip_method,CANCEL_CODE) == 0 )
			{
				nMethod = SIP_CANCEL;
			}
			else if ( strcmp( sip_method,INFO_CODE) == 0 )
			{
				nMethod = SIP_INFO;
			}
			else if ( strcmp( sip_method,NOTIFY_CODE) == 0  )
			{
				nMethod = SIP_NOTIFY;
			}

			osip_message_get_body (je->request, 0, &body); 
			buildAndSendReq(je,body,nMethod);

		}
		break;

	case EXOSIP_IN_SUBSCRIPTION_NEW:
	case EXOSIP_SUBSCRIPTION_UPDATE://add lj 2010-05-24
		{
			//SUBSCRIBE
			printf ("got a SUBSCRIPTION\n");
			osip_message_get_body (je->request, 0, &body); 
			buildAndSendReq(je,body,SIP_SUBSCRIBE);
		}
		break;
	case EXOSIP_SUBSCRIPTION_NOTIFY:
		{
			printf("get subcription notify");
			osip_message_get_body (je->request, 0, &body); 
			buildAndSendReq(je,body, SIP_NOTIFY);
		}
		break;


		//////////////////////////////////////////////////////////////////////////
		//收到网路应答

	case EXOSIP_CALL_REDIRECTED:
	case EXOSIP_CALL_NOANSWER:
	case EXOSIP_CALL_RELEASED:
	case EXOSIP_CALL_CLOSED:
	case EXOSIP_CALL_CANCELLED:
		{
			//osip_message_get_body (je->request, 0, &body); 
			//由body 去组装SIPpacket
			//_listener->onResponse();//error
		}
		break;
	case EXOSIP_CALL_PROCEEDING:
		{
			//INVITE the first two ack
			printf("processding receive \n");
		}
		break;
	case EXOSIP_CALL_RINGING:
		{
			//INVITE the third ack
			printf("EXOSIP_CALL_RINGING receive \n");
		}
		break;
	case EXOSIP_CALL_TIMEOUT:
	case EXOSIP_CALL_REQUESTFAILURE:
	case EXOSIP_CALL_SERVERFAILURE:
	case EXOSIP_CALL_GLOBALFAILURE:
		{	
			//error
			//_listener->onResponse();
			osip_message_get_body (je->response, 0, &body); 
			buildAndSendRes(je,body,0);
		}
		break;
	case EXOSIP_CALL_ANSWERED:
		{
			//INVITE last ack
			printf("answered\n");
			osip_message_get_body (je->response, 0, &body); 
			buildAndSendRes(je,body);

			osip_message_t *ack = NULL;
			eXosip_lock();
			if ( eXosip_call_build_ack( je->did, &ack ) == 0 )
			{
				int tmpRes = eXosip_call_send_ack( je->did, ack );
				int rest = tmpRes;
			}
			eXosip_unlock();
		}
		break;
	case EXOSIP_CALL_ACK:
		printf("call ack\n");
		//_listener->onResponse();//success
		break;

	case EXOSIP_CALL_MESSAGE_ANSWERED:
		{
			//   INFO/ NOTIFY
			printf("INFO/ NOTIFY answered\n");
			//Log_Printer(6000, "接收到：INFO/ NOTIFY answered");
			osip_message_get_body (je->response, 0, &body); 
			buildAndSendRes(je,body);
		}
		break;

	case EXOSIP_SUBSCRIPTION_PROCEEDING:
		{
			//subscribe first ack(maybe)
			printf("subcribe proceeding\n");
		}
		break;

	case EXOSIP_SUBSCRIPTION_ANSWERED:
		{
			//subscribe ACK
			//Log_Printer(6000, "接收到：subscribe ACK");
			osip_message_get_body (je->response, 0, &body); 
			buildAndSendRes(je,body);
		}
		break;

	case EXOSIP_REGISTRATION_FAILURE:
		{
			//fixme error 
			//osip_message_get_body (je->request, 0, &body); 
			//buildAndSendRes(je,body,0);
			buildAndSendRegRes(je);
		}			
		break;

	case EXOSIP_REGISTRATION_SUCCESS:
		{
			//osip_message_get_body (je->response, 0, &body); 
			//buildAndSendRes(je,body);
			buildAndSendRegRes(je);
		}
		break;

	case EXOSIP_MESSAGE_ANSWERED:
		{
			//Log_Printer(6000, "接收到：EXOSIP_MESSAGE_ANSWERED");
			//outside of dialog Response success
			osip_message_get_body (je->response, 0, &body); 
			buildAndSendRes(je,body);
		}
		break;

	case EXOSIP_MESSAGE_REDIRECTED:
	case EXOSIP_MESSAGE_REQUESTFAILURE:
	case EXOSIP_MESSAGE_SERVERFAILURE:
	case EXOSIP_MESSAGE_GLOBALFAILURE:
		{
			//outside of dialog Response failure
			osip_message_get_body (je->request, 0, &body); 
			buildAndSendRes(je,body,0);
		}
		break;

	default:
		break;
	}

	eXosip_event_free(je);

	return 0;
}


int CSIPBase::parseIpAndPortFromAddress(const char* szAddress,char* ip,int& port)
{
	if ( szAddress == NULL 
		|| szAddress[0] == NULL 
		|| ip == NULL )
	{
		return -1;
	}

	const char* index = strstr(szAddress,":");
	if ( index == NULL )
		return -1;

	strncpy(ip,szAddress,index-szAddress);
	port = atoi(index+1);
	return 0;
}

int CSIPBase::fetchAuthorization(eXosip_event_t* pEvent,CSIPPacket* sipReq)
{
	if(pEvent->request->authorizations.nb_elt == 0)
	{//无Authorization头
		return -1;
	}

	//暂时默认只有一个Authorization节点
	CSIPRegRequest* pRegReq = (CSIPRegRequest*)sipReq;
	osip_authorization_t* pAuth = (osip_authorization_t*)pEvent->request->authorizations.node->element;
	if(pAuth == NULL)
	{
		return -1;
	}

	char* tmpStr = NULL;
	osip_authorization_to_str(pAuth,&tmpStr);
	strncpy(pRegReq->_authorization,tmpStr,sizeof(pRegReq->_authorization));
	delete[] tmpStr;

	return 0;
}

int CSIPBase::buildAndSendRegReq(eXosip_event_t* envent)
{
	if ( envent == NULL )
	{
		return -1;
	}

	uint32 connId = 0;
	if(checkNetReqConnSession(envent,connId) < 0)
	{
		return -2;
	}

	CSIPRegRequest* request = new CSIPRegRequest;

	request->_sipSessionInfo._callId = envent->cid;
	request->_sipSessionInfo._dlgId = envent->did;
	request->_sipSessionInfo._transId = envent->tid;

	request->_method = SIP_REGISTER;
	fetchAddreInfoFromReq(envent,request);
	fetchAuthorization(envent,request);	

	//注册的FROM和TO都UASIP地址,需特殊处理
	osip_contact_t* pContact = NULL;
	if(osip_message_get_contact(envent->request,0,&pContact) == 0)
	{
		sprintf(request->_contact,"%s:%s",pContact->url->host,pContact->url->port);
		//<sip:下级平台地址编码@下级平台IP地址?rinstance=444cf68a4faf46f2>
		_snprintf(request->_wholeContact, 255, "<%s:%s@%s>", pContact->url->scheme, pContact->url->username, 
			pContact->url->host);
		//osip_contact_free(pContact);
	}

	osip_header_t* expirse = NULL;
	if(osip_message_get_expires(envent->request,0,&expirse) >= 0)
	{
		request->_expirse = atoi(expirse->hvalue);
	}

	request->addRef();
	_listener->onSIPPacket(0,connId,request);
	request->release();

	return 0;
}

int CSIPBase::buildAndSendRegRes(eXosip_event_t* envent)
{
	if(envent == NULL)
	{
		return -1;
	}

	if ( envent == NULL )
	{
		return -1;
	}

	char* cseq = envent->request->call_id->number;

	CSIPPacket* packet = fetchSentPdu(cseq);
	if ( packet == NULL && envent->type == EXOSIP_REGISTRATION_SUCCESS)
	{
		//不应该删除;除非是上层检测请求已超时,主动剔除
		return -1;
	}

	int iStatusCode = -1;
	if(envent->response != NULL)                                             //add lj 2010-04-21 当uas没有开启的时候，response = NULL 会引起异常
	{
		iStatusCode = envent->response->status_code;
		if(iStatusCode != SIP_OK)
		{//需要继续保存发送的注册信令
			addSentPdu(cseq,packet);
		}
		uint32 connId = 0;
		if ( checkNetResConnSession( envent, connId ) < 0 )
		{
			return -2;
		}

		CSIPRegResponse* response = new CSIPRegResponse();

		response->_sipSessionInfo._callId = envent->cid;
		response->_sipSessionInfo._dlgId = envent->did;
		response->_sipSessionInfo._transId = envent->tid;
		response->_sipRid = envent->rid;

		response->_method = SIP_REGISTER;
		response->_bSuccess = 0;
		//response->setSequence(packet->getSequence());	//对网络上受到的应答进行转换，转换为vplib与sip库之间的sequence
		fetchAddreInfoFromRes(envent,response);

		response->_bSuccess = 0;
		if(iStatusCode == SIP_OK)
		{
			response->_bSuccess = 1;
		}
		else if(iStatusCode == SIP_UNAUTHORIZED)
		{
			//response->_bSuccess = 0;
			fetchWWWAuthenticateFromRes(envent,response);
		}
		response->_code = iStatusCode;

		if ( envent->response != NULL )
		{
			//fixme 这个response怎么会是空的 难道是调试的缘故
			//response->_code = envent->response->status_code;
			strncpy(response->_message, envent->response->reason_phrase, sizeof(response->_message)-1);
		}

		osip_header_t* expirse = NULL;
		if(osip_message_get_expires(envent->response,0,&expirse) >= 0)
		{
			response->_expirse = atoi(expirse->hvalue);
		}
		response->addRef();
		_listener->onSIPPacket(0,connId,response);
		response->release();
	}
	else
	{
		printf("sip register response = null error, may be the uas is close!");
	}
	return 0;
}

int CSIPBase::fetchWWWAuthenticateFromRes(eXosip_event_t* pEvent,CSIPPacket* sipReq)
{
	CSIPRegResponse* pRegRes = (CSIPRegResponse*)sipReq;
	osip_www_authenticate_t * auth;
	char *strAuth=NULL;
	char m_strAuth[256] = {0};
	if(osip_message_get_www_authenticate(pEvent->response,0,&auth))
	{
		return -1;
	}
	if(osip_www_authenticate_to_str(auth,&strAuth))
	{
		return -1;
	}

	strncpy(pRegRes->_www_authenticate,strAuth,sizeof(pRegRes->_www_authenticate));;//保存认证字符串
	delete []strAuth;

	return 0;
}