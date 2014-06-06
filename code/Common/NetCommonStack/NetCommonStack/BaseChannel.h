#ifndef BASE_CHANNEL_DEF_H
#define BASE_CHANNEL_DEF_H

class CBaseChannel
{
public:
	CBaseChannel();
	virtual ~CBaseChannel();


public:
	char* BaseGetRemoteIPAddrString();
	UINT32 BaseGetRemoteIPAddr();
	char* BaseGetLocalIPAddrString();
	UINT32 BaseGetLocalIPAddr();
	UINT16 BaseGetRemotePort();
	UINT16 BaseGetLocalPort();

	void SetRemoteIPAddrString(char* pszIPAddr);
	void SetRemoteIPAddr(UINT32 uiIPAddr);
	void SetLocalIPAddrString(char* pszIPAddr);
	void SetLocalIPAddr(UINT32 uiIPAddr);
	void SetRemotePort(UINT16 uiPort);
	void SetLocalPort(UINT16 uiPort);
	//逻辑的远端ip地址，不一定是真正建立socket连接的地址，由用户自由选择设置该地址。
	//比如，用于BS登陆服务器时，代表BS客户端的地址，而不是tomcat的地址。
	void SetLogicRemoteIPAddrString(const char* pszIPAddr);
	//逻辑的远端ip地址，不一定是真正建立socket连接的地址，由用户自由选择设置该地址。
	//比如，用于BS登陆服务器时，代表BS客户端的地址，而不是tomcat的地址。
	char* GetLogicRemoteIPAddrString();

protected:
	char m_szRemoteIPAddr[20];
	char m_szBindingIPAddr[20];
	UINT32 m_uiRemoteIPAddr;
	UINT32 m_uiLocalIPAddr;
	UINT16 m_uiRemotePort;
	UINT16 m_uiLocalPort;
	//逻辑的远端ip地址，不一定是真正建立socket连接的地址，由用户自由选择设置该地址。
	//比如，用于BS登陆服务器时，代表BS客户端的地址，而不是tomcat的地址。
	char	m_szLogicRemoteIPAddr[16];
};


#endif