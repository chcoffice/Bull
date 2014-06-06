

#include <windows.h>
#include <iostream>
#include "WinService.h"
#include"CSimpleLog.h"

static SERVICE_STATUS g_ServiceStatus;  
static SERVICE_STATUS_HANDLE g_hStatus;
static char   g_ServiceName[512];
static StartServiceCallback g_pStartServiceCB = NULL;
static StopServiceCallback  g_pStopServiceCB = NULL;
static HANDLE g_hSvcStopEvent = NULL;  //服务停止句柄
static CSimpleLog g_SimpleLog;


static void ReportSvcStatus( DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
	static DWORD dwCheckPoint = 1;
	g_ServiceStatus.dwCurrentState = dwCurrentState;
	g_ServiceStatus.dwWin32ExitCode = dwWin32ExitCode;
	g_ServiceStatus.dwWaitHint = dwWaitHint;
	if (dwCurrentState == SERVICE_START_PENDING)
	{
		g_ServiceStatus.dwControlsAccepted = 0;
	}
	else 
	{
		g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	}

	if ( (dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED) )
	{
		g_ServiceStatus.dwCheckPoint = 0;
	}
	else 
	{
		g_ServiceStatus.dwCheckPoint = dwCheckPoint++;
	}

	SetServiceStatus( g_hStatus, &g_ServiceStatus );
}



void WINAPI ControlHandler(DWORD request) 
{ 
	switch(request) 
	{ 
	case SERVICE_CONTROL_STOP: 
	case SERVICE_CONTROL_SHUTDOWN: 
		g_SimpleLog.Add("Monitoring stopped.");
		ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);  
	    SetEvent(g_hSvcStopEvent);  
		return; 

	default:
		break;
	} 

	// Report current status
	 ReportSvcStatus(g_ServiceStatus.dwCurrentState, NO_ERROR, 0);  

	return; 
}

void WINAPI ServiceMain(int argc, char** argv)
{


	g_ServiceStatus.dwServiceType             = SERVICE_WIN32;
	g_ServiceStatus.dwCurrentState            = SERVICE_START_PENDING;
	g_ServiceStatus.dwControlsAccepted        = SERVICE_ACCEPT_SHUTDOWN|SERVICE_ACCEPT_STOP;
	g_ServiceStatus.dwWin32ExitCode           = 0;
	g_ServiceStatus.dwServiceSpecificExitCode = 0;
	g_ServiceStatus.dwCheckPoint              = 0;
	g_ServiceStatus.dwWaitHint                = 0;

	g_hStatus = ::RegisterServiceCtrlHandlerA(g_ServiceName, ControlHandler);
	if ( g_hStatus == 0 )
	{
		g_SimpleLog.Add("RegisterServiceCtrlHandler fail,last error:%d",GetLastError());
		return;
	}

	if(!g_pStartServiceCB())//启动程序
	{
		g_SimpleLog.Add("启动服务 %s 失败! 请查看相关服务日志定位问题.",g_ServiceName);
		ReportSvcStatus( SERVICE_STOPPED, ERROR_SERVICE_SPECIFIC_ERROR, 0 );
		return;
	}


	g_SimpleLog.Add("启动服务 %s 成功!",g_ServiceName);

	ReportSvcStatus( SERVICE_START_PENDING, NO_ERROR, 3000 );

	g_hSvcStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if ( g_hSvcStopEvent == NULL)
	{
		g_SimpleLog.Add("create event fail,last error:%d",GetLastError());
		ReportSvcStatus( SERVICE_STOPPED, NO_ERROR, 0 );
		return;
	}

	ReportSvcStatus( SERVICE_RUNNING, NO_ERROR, 0 );

	
	while(1)  
	{  
		// Check whether to stop the service.   
		WaitForSingleObject(g_hSvcStopEvent, INFINITE);  
		g_SimpleLog.Add("准备退出服务 %s ...",g_ServiceName);
		g_pStopServiceCB();
		g_SimpleLog.Add("退出服务 %s 成功!",g_ServiceName);
		ReportSvcStatus( SERVICE_STOPPED, NO_ERROR, 0 );  

		return;  
	}  



}




bool RunAsService(int argc, char* argv[],StartServiceCallback pStartServiceCB,StopServiceCallback pStopServiceCB)
{

	g_SimpleLog.SetLogPath("ServiceLog");


	if(argc < 2)  //以服务启动的话必须带两个启动参数（第一个固定是service,第二个是希望运行的服务名，如:DeviceAccessService.exe service das）
		return false;
	else
	{

		bool isRunAsService = false;
		for(int i=1;i<argc;i++)
			if(strcmp(argv[i] ,"service") == 0)
			{
				if((i+1)<=argc)
				{
					memset(g_ServiceName,0,sizeof(g_ServiceName));
					strcpy_s(g_ServiceName,argv[i+1]);
					isRunAsService = true;
					break;

				}

			}


			if(!isRunAsService)
				return false;


			g_pStartServiceCB = pStartServiceCB;
			g_pStopServiceCB = pStopServiceCB;
		
			SERVICE_TABLE_ENTRYA ServiceTable[2];
			ServiceTable[0].lpServiceName = (LPSTR)g_ServiceName;
			ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTIONA)ServiceMain;

			ServiceTable[1].lpServiceName = NULL;
			ServiceTable[1].lpServiceProc = NULL;

		
			// 启动服务的控制分派机线程
			if (!StartServiceCtrlDispatcherA(ServiceTable))
			{
				g_SimpleLog.Add("StartServiceCtrlDispatcher fail,last error:%d",GetLastError());

			}

			return true;







	}


}