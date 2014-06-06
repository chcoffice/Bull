#include "ISystemLayInterface.h"



/******************************************************************************
功能说明：系统时间
******************************************************************************/


/********************************************************************************************
  Function		: DoGetTickCount    
  DateTime		: 2010/6/10 10:38	
  Description	: 获取从操作系统启动到现在所经过的毫秒数
  Input			: NULL
  Output		: NULL
  Return		: 返回从操作系统启动到现在所经过的毫秒数
  Note			:				// 备注
********************************************************************************************/
UINT64   DoGetTickCount()
{

	UINT64 iTickCount=0;
	
#ifdef _WIN32

	DWORD dwTickCount=0;
	dwTickCount=GetTickCount();
	iTickCount=(UINT64)dwTickCount;

#elif _LINUX

	struct timeval current;
	gettimeofday(&current, NULL);
	iTickCount =1000LL*current.tv_sec + current.tv_usec/1000;

#endif

	return iTickCount;

}


/********************************************************************************************
  Function		: GetLocalTime    
  DateTime		: 2010/6/10 10:40	
  Description	: 获取当地的当前系统日期和时间
  Input			: pLoaltime: 指向一个用户自定义包含日期和时间信息的类型为 StruSysTime 的变量，该变量用来保存函数获取的时间信息
  Output		: NULL
  Return		: NULL
  Note			:				// 备注
********************************************************************************************/
void    DoGetLocalTime(StruSysTimePtr pLoaltime)
{

	if(NULL == pLoaltime) return ;

#ifdef _WIN32
	GetLocalTime((LPSYSTEMTIME)pLoaltime);
#elif _LINUX

	time_t struTime;
	struct tm *pStruLocalTime;
	struct timeval tv;

	time(&struTime);
	pStruLocalTime=localtime(&struTime); 
	pLoaltime->wYear=1900+pStruLocalTime->tm_year;
	pLoaltime->wMonth=1+pStruLocalTime->tm_mon;
	pLoaltime->wDay=pStruLocalTime->tm_mday;
	pLoaltime->wDayOfWeek=1+pStruLocalTime->tm_wday;
	pLoaltime->wHour=pStruLocalTime->tm_hour;
	pLoaltime->wMinute=pStruLocalTime->tm_min;
	pLoaltime->wSecond=pStruLocalTime->tm_sec;

	//加上毫秒   
	gettimeofday(&tv, NULL);   
	tv.tv_usec /= 1000;  

	pLoaltime->wMilliseconds=(INT)tv.tv_usec;


#endif

}
