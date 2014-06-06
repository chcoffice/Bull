#include "GSCommon.h"
#include "INetService.h"
#include "NetCommonStack.h"
#include "NetServer.h"
#include "NetClient.h"
#include "Log.h"

#ifdef WINCE
	#ifdef _DEBUG
	#pragma comment(lib, "NetServiceLibD.lib")
	#pragma comment(lib, "CommonLibsD.lib")
	#else
	#pragma comment(lib, "../../libM/NetServiceLib.lib")
	#pragma comment(lib, "../../libM/CommonLibs.lib")
	#endif
#else 

	#ifdef _DEBUG
	#pragma comment(lib, "../../lib/NetService/NetServiceLibD.lib")
	#pragma comment(lib, "../../lib/CommonLibs/CommonLibsD.lib")
	#else
	#pragma comment(lib, "../../lib/NetService/NetServiceLib.lib")
	#pragma comment(lib, "../../lib/CommonLibs/CommonLibs.lib")
	#endif
#endif


static CMM::CLog *s_pLog;


INT32 InitNetCommuStack()
{
    s_pLog = new CMM::CLog();
    s_pLog->SetLogPath("CmmLog");
    s_pLog->SetInitFile("Cmm.ini");

	return 0;
}
 

INetServer* CreateNetServer()
{
	CNetServer* pSvr;

	pSvr = new CNetServer;
	return pSvr;
}
INetClient* CreateNetClient()
{
	CNetClient* pCli;
	pCli = new CNetClient;
	return pCli;
}