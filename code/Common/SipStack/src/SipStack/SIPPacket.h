/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : SIPPACKET.H
Author :  邹阳星
Version : 1.0.0.1
Date: 2012/8/16 15:28
Description: 
********************************************
*/

#ifndef _GS_H_SIPPACKET_H_
#define _GS_H_SIPPACKET_H_
#include "SIPCommonDef.h"


namespace GSSIP
{

	
	/*
	********************************************************************
	类注释
	类名    :    CRefObject
	作者    :    邹阳星
	版本    :	 1.0.0.1
	创建时间:    2012/8/16 16:26
	类描述  :   可以引用的类对象 
	*********************************************************************
	*/

	class CRefObject : public CGSObject
	{
	private :
		GSAtomicInter m_iRefs; //引用数
	public :
		virtual void Ref(void);
		virtual void Unref(void);

		//返回引用数
		INLINE long GetRefCounts(void) const 
		{
			return m_iRefs;
		}
	protected :
		CRefObject(void) :
			  CGSObject(),
				  m_iRefs(1)
			  {

			  }

			  virtual ~CRefObject(void)
			  {

			  }
	};	



/*
********************************************************************
类注释
类名    :    CPacket
作者    :    邹阳星
版本    :	 1.0.0.1
创建时间:    2012/8/16 16:26
类描述  :    SIP 数据包基本类
*********************************************************************
*/

class CPacket : public CRefObject
{
public :
	// body 的类型
	typedef enum
	{
		eBODY_STRING = 0, //文本方式
		eBODY_BINARY, //二进制数
	}EnumBodyType;

	//命令方向
	typedef enum 
	{
		eDIR_REQUEST = 0, //请求
		eDIR_RESPONSE = 1, //回复
	}EnumDirect;
protected :

	EnumBodyType m_eBodyType;
	CGSString m_strBody;   //内部分配空间 APP发送命令时填写
	BYTE *m_bBody;
	int  m_iBodyLength;   //_body长度	   APP发送命令时填写
	int m_iBodyBufSize; //

	CGSString	m_strFromCode;//from中@之前的地址编码 APP发送请求时填写
	CGSString	m_strFromAdr;	//from中@之后的地址     APP发送请求时填写
	CGSString	m_strToCode;	//to中@之前的地址编码   APP发送请求时填写
	CGSString	m_strToAdr;	//to中@之后的地址       APP发送请求时填写

	int		m_iCmdId;						//协议序号 APP发送请求时填写 收到网络应答时方便处理 
										//因为有些命令的应答是没有包体的供解析

	EnumSipMethod	m_eMethod;		//sip命令类型	SIP层收到网络命令以及APP层主动发送时填写
	int     m_iSubexpires;                //告警预定的时间


	StruSipSessionInfo	m_stSipSessionInfo;	//sip库收到网络请求时填写；
											//为网络请求的session信息 用于发送应答

	int		m_iSequence;					//APP发送的命令顺序号 
											//由其发送请求时填写 用于处理网络应答


	EnumDirect	m_eDirect;					//方向

public:
	CPacket( EnumDirect eDirect, EnumBodyType eBodyType );
	virtual ~CPacket(void);

	INLINE EnumDirect	GetDirect(void) const
	{
		return m_eDirect;
	}

	INLINE int		GetSequence(void) const 
	{
		return m_iSequence;
	}

	INLINE void	SetSequence(int iSequence)
	{
		m_iSequence = iSequence;
	}

	const CGSString &GetStrBody(void) const
	{
		return m_strBody;
	}

	INLINE int GetBodyLength(void) const
	{
		if( m_eBodyType )
		{
			return m_iBodyLength;
		}
		return m_strBody.size();
	}

	const BYTE*	GetBinaryBody(void) const
	{
		return m_bBody;
	}

	BOOL	SetBody( const CGSString &strBody );

	BOOL   SetBody( BYTE *bBuf, int iSize);



	
};


/*
********************************************************************
类注释
类名    :    CRequest
作者    :    邹阳星
版本    :	 1.0.0.1
创建时间:    2012/8/16 16:26
类描述  :		Request 类型数据包 基本类
*********************************************************************
*/

class CRequest : public CPacket
{
public:
	CRequest( EnumBodyType eBodyType )
		:CPacket(CPacket::eDIR_REQUEST, eBodyType)
	{

	}
	virtual ~CRequest(void)
	{

	}
};

/*
********************************************************************
类注释
类名    :    CRequest
作者    :    邹阳星
版本    :	 1.0.0.1
创建时间:    2012/8/16 16:26
类描述  :		Response 类型数据包 基本类
*********************************************************************
*/
class CResponse : public CPacket
{
public:
	EnumSipResponseCode	 m_eResonseCode;
	int  m_iOSipEvtCode;			//osip event中的信息 
	CGSString m_strMessage;

public:
	CResponse( EnumBodyType eBodyType ):
	  CPacket(CPacket::eDIR_RESPONSE, eBodyType)
	{
		m_eResonseCode = eSIP_RESPONSE_OK;
		m_iOSipEvtCode = 1;
		m_strMessage = "Success";
	}
	virtual ~CResponse(void);
};



/*
********************************************************************
类注释
类名    :    CRegRequest
作者    :    邹阳星
版本    :	 1.0.0.1
创建时间:    2012/8/31 10:12
类描述  :		注册请求
*********************************************************************
*/

class CRegRequest:public CRequest
{
public:
	CGSString	m_strAllow;
	CGSString	m_strSupported;
	CGSString	m_strUserAgent;
	CGSString	m_strAuthorization;
	CGSString    m_strContact;

	CGSString	m_strSipUserName;
	CGSString    m_strSipPassword;
	CGSString    m_strWholeContact;
	int		m_iExpirse;

public:
	CRegRequest(void)
		:CRequest(CPacket::eBODY_STRING)
	{
		m_iExpirse = 0;
	}
	virtual ~CRegRequest(void)
	{

	}


};




/*
********************************************************************
类注释
类名    :    CRegResponse
作者    :    邹阳星
版本    :	 1.0.0.1
创建时间:    2012/8/31 10:13
类描述  :		注册请求回复
*********************************************************************
*/

class CRegResponse:public CResponse
{
public:
	CGSString	m_strWWWAuthenticate;
	int     m_iSipRid;
	CGSString    m_strWholeContact;
	int		m_iExpirse;
public:
	CRegResponse(void)
		:CResponse(CPacket::eBODY_STRING)
	{
		m_iSipRid = 0;
		m_iExpirse = 0;
	}

	virtual ~CRegResponse(void)
	{

	}


};


} //end namespace GSSIP

#endif //end _GS_H_SIPPACKET_H_
