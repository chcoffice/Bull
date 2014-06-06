#include "ISystemLayInterface.h"
/**************************************************************************************************
  Copyright (C), 2010-2011, GOSUN 
  File name 	: CGSPROCESSINFO.CPP      
  Author 		: hf     
  Version 		: Vx.xx        
  DateTime 		: 2011/6/14 10:43
  Description 	: 获取进程信息
**************************************************************************************************/

CGSProcessInfo::CGSProcessInfo()
{
	m_processor_count = 0;
	
}
CGSProcessInfo::~CGSProcessInfo(void)
{

}


/**************************************************************************************************
  Function	:     
  DateTime	: 2011/6/14 14:20	
  Description	:// 函数功能、性能等的描述
  Input		:// 输入参数说明，包括每个参数的作
  Output	:// 对输出参数的说明。
  Return	:// 函数返回值的说明
  Note		:// 备注
**************************************************************************************************/

INT32	CGSProcessInfo::GSGetTotalCPUUsage(INT32 &iCPU)
{
	INT32 iRet = -1;
#ifdef _WIN32

	FILETIME preidleTime;
	FILETIME prekernelTime;
	FILETIME preuserTime;

	FILETIME idleTime;
	FILETIME kernelTime;
	FILETIME userTime;

	GetSystemTimes( &idleTime, &kernelTime, &userTime );

	preidleTime = idleTime;
	prekernelTime = kernelTime;
	preuserTime = userTime ;

	//休眠1s后再获取
	MSLEEP(1000);
	
	GetSystemTimes( &idleTime, &kernelTime, &userTime );

	INT64 idle = CompareFileTime( preidleTime,idleTime);
	INT64 kernel = CompareFileTime( prekernelTime, kernelTime);
	INT64 user = CompareFileTime(preuserTime, userTime);

	//CPU利用率
	 iCPU = (kernel +user - idle) *100/(kernel+user);

	 //CPU空闲率
	INT64 cpuidle = ( idle) *100/(kernel+user);

	iRet = 0;
#elif _LINUX
	DWORD total1;
	DWORD total2;

	DWORD user1;
	DWORD nice1;
	DWORD system1;
	DWORD idle1;
	DWORD iowait1;
	DWORD irq1;
	DWORD softirq1;

	DWORD user2;
	DWORD nice2;
	DWORD system2;
	DWORD idle2;
	DWORD iowait2;
	DWORD irq2;
	DWORD softirq2;

	char cpu[21];
	char text[201];

	FILE *fp;

	fp = fopen("/proc/stat", "r");
	
	fgets(text, 200, fp);
	if(strstr(text, "cpu"))
	{
		sscanf(text, "%s%lu%lu%lu%lu", cpu, &user1, &nice1, &system1, &idle1, &iowait1, &irq1, &softirq1);
	}
	
	fclose(fp);

	total1 = (user1+nice1+system1+idle1+iowait1+irq1+softirq1);

	MSLEEP(1000);        

	fp = fopen("/proc/stat", "r");
	
	fgets(text, 200, fp);
	if(strstr(text, "cpu"))
	{
		sscanf(text, "%s%lu%lu%lu%lu", cpu, &user2, &nice2, &system2, &idle2, &iowait2, &irq2, &softirq2);
	}
	
	fclose(fp);

	total2 = (user2+nice2+system2+idle2+iowait2+irq2+softirq2);

	if ((total2 - total1) == 0)
	{
		iCPU = 0;
	}
	else
	{
		iCPU = (INT32)((user2-user1+nice2 - nice1 +system2 - system1)*10000)/(total2-total1);
	}

	iRet = 0;
	
#endif	
	return	iRet;
}
/**************************************************************************************************
  Function	:     
  DateTime	: 2011/6/14 14:20	
  Description	:// 函数功能、性能等的描述
  Input		:// 输入参数说明，包括每个参数的作
  Output	:// 对输出参数的说明。
  Return	:// 函数返回值的说明
  Note		:// 备注
**************************************************************************************************/

INT32	CGSProcessInfo::GSGetTotalMemoryUsage(DWORD &dwMem)
{
	INT32 iRet = -1;
#ifdef _WIN32

	MEMORYSTATUSEX statex;

	statex.dwLength = sizeof (statex);

	GlobalMemoryStatusEx (&statex);

	dwMem = statex.dwMemoryLoad;

	iRet = 0;
#elif _LINUX
	FILE *fd;          
	int n;             
	char buff[256];   
	char name[20];    
	unsigned long total; 
	char name2[20];
	unsigned long free; 

	fd = fopen ("/proc/meminfo", "r"); 

	fgets (buff, sizeof(buff), fd); 
	fgets (buff, sizeof(buff), fd); 
	fgets (buff, sizeof(buff), fd); 
	fgets (buff, sizeof(buff), fd); 
	sscanf (buff, "%s %u %s", name, &total, name2); 

	fgets (buff, sizeof(buff), fd); //从fd文件中读取长度为buff的字符串再存到起始地址为buff这个空间里 
	sscanf (buff, "%s %u", name2, &free, name2); 

	fclose(fd);     //关闭文件fd

	dwMem = (total - free)*100/total;
#endif
	
	return	iRet;
}


#ifdef _WIN32
INT64 CGSProcessInfo::CompareFileTime ( FILETIME time1, FILETIME time2 )
{
	INT64 a = ((INT64)time1.dwHighDateTime) << 32 | time1.dwLowDateTime ;
	INT64 b = ((INT64)time2.dwHighDateTime) << 32 | time2.dwLowDateTime ;

	return   (b - a);
}
#endif

/**************************************************************************************************
  Function	:     
  DateTime	: 2011/6/14 15:58	
  Description	:// 函数功能、性能等的描述
  Input		:// 输入参数说明，包括每个参数的作
  Output	:// 对输出参数的说明。
  Return	:// 函数返回值的说明
  Note		:// 备注
**************************************************************************************************/

INT32	CGSProcessInfo::GSGetProcessorNumber()
{
	INT32 iNum = 0;
#ifdef _WIN32
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	iNum = (INT32)info.dwNumberOfProcessors;
#endif
	return	iNum;
}

