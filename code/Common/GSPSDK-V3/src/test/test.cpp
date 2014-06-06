// test.cpp : 定义控制台应用程序的入口点。
//
#include <stdio.h>
#include <GSPObject.h>
#include <time.h>
#include <GSPMemory.h>
#include "cmdline.h"

#include <set>
#include <vector>
#include <list>
#include "Log.h"
#include "CircleQueue.h"
#include "IGSPClient.h"
#include "RTP/RtpNet.h"

using namespace GSP;
using namespace GSP::RTP;




static int OptionsTest(const char *czCmd,const char *args)
{
    MY_PRINTF( "On Options Test Function.");
    MY_PRINTF( "Args: %s.\n", args ? args : "null");
    char czBuffer[256];
    char *pParser = ArgsGetParser(args, czBuffer);
    int ret = 0;
    if( pParser )
    {
        char *czKey, *czValue;
        while( (ret=ParserFetch(&pParser, &czKey, &czValue ))==1 )
        {
            MY_PRINTF("Key(%s) Value(%s)\n", czKey, czValue ? czValue : "NULL");
        }
        if( ret )
        {
            MY_PRINTF( "Args is invalid.\n");
        }
    }
    return 0;
}



extern int TestBtree(const char *czCmd,const char *args);
extern int TestGSPString(const char *czCmd,const char *args);
extern int TestMemPool(const char *czCmd,const char *args);
extern int TestGSPServer(const char *czCmd,const char *args); 
extern int TestGSPClient(const char *czCmd,const char *args);
extern int TestThreadPool(const char *czCmd,const char *args);
extern int TestGSThreadPool(const char *czCmd,const char *args);
extern int TestJouEntry(const char *czCmd,const char *args);

static StruOptionLine _Options[]=
{
    {
        "-th",
            "--thread",
            "Test thread pool Function",
            TestThreadPool
    },
// 	{
// 		"-gsthp",
// 			"--gsthreadpool",
// 			"Test gosun thread pool Function",
// 			TestGSThreadPool
// 		},
    {
        "-cli",
            "--client",
            "Test gsp client Function",
            TestGSPClient
    },
    {
        "-srv",
            "--server",
            "Test gsp server Function",
            TestGSPServer
    },
//     {
//         "-mp",
//             "--mempool",
//             "Test gsp mempool Function",
//             TestMemPool
//     },

    {
        "-t",
            "--test",
            "Test Options Function",
            OptionsTest
    },

//         {
//             "-bt",
//                 "--btree",
//                 "Test btree Function",
//                 TestBtree
//         },
 		{
 			"-Jou",
 				"--Journal",
 				"Test Journal Function",
 				TestJouEntry
 			},
		
        {
            NULL,
                NULL,
                NULL,
                NULL,
            }
};

static CGSWRMutex *s_csWRMutex = NULL;
static INT *s_pITest = NULL, s_iTest = 0;

static void OnThreadTestEntry(CGSThread *gsThreadHandle, void *pUserParam )
{
INT  i = (INT) pUserParam;
INT  j;

#define MYPRINTF printf
//#define MYPRINTF(...) do{}while(0)
    while(1)
    {
        j = 1 + (int) (10.0 * (rand() / (RAND_MAX + 1.0)));
        MSLEEP(j);
        if( i<3 )
        {
 MYPRINTF("w-0-%d\n", i);
        if( !s_csWRMutex->LockWrite() )
        {
            assert(0);
        }
MYPRINTF("w-1-%d\n", i);
        assert(s_pITest!=NULL);
        *s_pITest = (*s_pITest)+1;
        s_pITest = NULL;
        j = 3 + (int) (100.0 * (rand() / (RAND_MAX + 1.0)));
        if( j )
        {
            MSLEEP(j);
        }
         s_pITest = &s_iTest;
 MYPRINTF("w-2-%d\n", i); 
        s_csWRMutex->UnlockWrite();
   
        } 
        else
        {
MYPRINTF("r-0-%d\n", i);
            if( !s_csWRMutex->LockReader() )
            {
                assert(0);
            }
MYPRINTF("r-1-%d\n", i); 
            assert(s_pITest!=NULL);
            *s_pITest = (*s_pITest)+1;
          //  s_pITest = NULL;
            j = 5 + (int) (10.0 * (rand() / (RAND_MAX + 1.0)));
            if( j )
            {
                MSLEEP(j);
            }
          //   s_pITest = &s_iTest;
MYPRINTF("r-2-%d\n", i);             
            s_csWRMutex->UnlockReader();
   
        }
    }
}


static void OnTestCircleQueueWriteEntry(CGSThread *gsThreadHandle, void *pUserParam )
{
CCircleQueue<INT64*> *pQueue = (CCircleQueue<INT64*> *)pUserParam;
INT i = 1;
INT j;
INT iCnt = 20;
INT64 *pIVal;
	while(1)
	{
		pIVal = new INT64;
		GS_ASSERT(pIVal);
		*pIVal =  i;
		if(iCnt++>0 && eERRNO_SUCCESS == pQueue->Write(pIVal) )
		{
			i++;
		}
		else
		{
			delete pIVal;
			j = 1 + (int) (10.0 * (rand() / (RAND_MAX + 1.0)));
			MSLEEP(j);
			iCnt = 5+(int) (j  * (rand() / (RAND_MAX + 1.0)));
		}
	}	
}

static void OnTestCircleQueueReadEntry(CGSThread *gsThreadHandle, void *pUserParam )
{
	CCircleQueue<INT64*> *pQueue = (CCircleQueue<INT64*> *)pUserParam;
	INT64 *pI = 0;
	INT64 iOld = 0;
	INT j;
	INT iCnt = 10;
	while(1)
	{

		if( iCnt++>0 && eERRNO_SUCCESS == pQueue->Pop(&pI) )
		{
			if( (*pI-iOld) != 1)
			{
				GS_ASSERT(0);
			}
			iOld = *pI;
			delete pI;
			j = 1 + (int) (10.0 * (rand() / (RAND_MAX + 1.0)));
			MSLEEP(j);
		}
		else
		{			
			j = 1 + (int) (30.0 * (rand() / (RAND_MAX + 1.0)));
			MSLEEP(j);
			iCnt = 5+(int) (j  * (rand() / (RAND_MAX + 1.0)));
		}
	}	
}


#ifdef WINCE

int WINAPI WinMain(HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPTSTR    lpCmdLine,
				   int       nCmdShow)
#else

class CTestA
{
public : 
    char m_A[120];

    CTestA(void)
    {
        MY_PRINTF( "Create CTestA\n");
    }
    virtual ~CTestA(void)
    {
        MY_PRINTF( "Delete CTestA\n");
    }
};

namespace GSP
{


extern void GSPModuleInit(void);
extern void GSPModuleUnint(void);

} //end namespace GSP


static void gs_timesub(const unsigned long * timep, struct tm * tmp)
{
#define SECSPERMIN	60
#define MINSPERHOUR	60
#define HOURSPERDAY	24
#define DAYSPERWEEK	7
#define DAYSPERNYEAR	365
#define DAYSPERLYEAR	366
#define SECSPERHOUR	(SECSPERMIN * MINSPERHOUR)
#define SECSPERDAY	((long) SECSPERHOUR * HOURSPERDAY)
#define MONSPERYEAR	12

#define TM_SUNDAY	0
#define TM_MONDAY	1
#define TM_TUESDAY	2
#define TM_WEDNESDAY	3
#define TM_THURSDAY	4
#define TM_FRIDAY	5
#define TM_SATURDAY	6

#define TM_JANUARY	0
#define TM_FEBRUARY	1
#define TM_MARCH	2
#define TM_APRIL	3
#define TM_MAY		4
#define TM_JUNE		5
#define TM_JULY		6
#define TM_AUGUST	7
#define TM_SEPTEMBER	8
#define TM_OCTOBER	9
#define TM_NOVEMBER	10
#define TM_DECEMBER	11

#define TM_YEAR_BASE	1900

#define EPOCH_YEAR	1970
#define EPOCH_WDAY	TM_THURSDAY
#define isleap(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

	static const int	mon_lengths[2][MONSPERYEAR] = {
		{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
		{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
	};

	static const int	year_lengths[2] = {
		DAYSPERNYEAR, DAYSPERLYEAR
	};

	
	 long			days;
	 long			rem;
	 int			y;
	 int			yleap;
	 const int *		ip;
	 long			corr;
	 int			hit;
	 int offset;

	 offset = 0;
	corr = 0;
	hit = 0;
	days = *timep / SECSPERDAY;
	rem = *timep % SECSPERDAY;

	rem += (offset - corr);
	while (rem < 0) {
		rem += SECSPERDAY;
		--days;
	}
	while (rem >= SECSPERDAY) {
		rem -= SECSPERDAY;
		++days;
	}
	tmp->tm_hour = (int) (rem / SECSPERHOUR);
	rem = rem % SECSPERHOUR;
	tmp->tm_min = (int) (rem / SECSPERMIN);
	/*
	** A positive leap second requires a special
	** representation.  This uses "... ??:59:60" et seq.
	*/
	tmp->tm_sec = (int) (rem % SECSPERMIN) + hit;
	tmp->tm_wday = (int) ((EPOCH_WDAY + days) % DAYSPERWEEK);
	if (tmp->tm_wday < 0)
		tmp->tm_wday += DAYSPERWEEK;
	y = EPOCH_YEAR;
#define LEAPS_THRU_END_OF(y)	((y) / 4 - (y) / 100 + (y) / 400)
	while (days < 0 || days >= (long) year_lengths[yleap = isleap(y)]) {
		register int	newy;

		newy = y + days / DAYSPERNYEAR;
		if (days < 0)
			--newy;
		days -= (newy - y) * DAYSPERNYEAR +
			LEAPS_THRU_END_OF(newy - 1) -
			LEAPS_THRU_END_OF(y - 1);
		y = newy;
	}
	tmp->tm_year = y - TM_YEAR_BASE;
	tmp->tm_yday = (int) days;
	ip = mon_lengths[yleap];
	for (tmp->tm_mon = 0; days >= (long) ip[tmp->tm_mon]; ++(tmp->tm_mon))
		days = days - (long) ip[tmp->tm_mon];
	tmp->tm_mday = (int) (days + 1);
	tmp->tm_isdst = 0;
}



//返回 当前时间到 1970-01-01 00:00:00 经过的秒数
unsigned long  
gs_mktime(const unsigned int year0, const unsigned int mon0,  
	   const unsigned int day, const unsigned int hour,  
	   const unsigned int min, const unsigned int sec)  
{  
	unsigned int mon = mon0, year = year0;  

	/* 1..12 -> 11,12,1..10 */  
	if (0 >= (int) (mon -= 2)) {  
		mon += 12;  /* Puts Feb last since it has leap day */  
		year -= 1;  
	}  

	return ((((unsigned long)  
		(year/4 - year/100 + year/400 + 367*mon/12 + day) +  
		year*365 - 719499  
		)*24 + hour /* now have hours */  
		)*60 + min /* now have minutes */  
		)*60 + sec; /* finally seconds */  
}  




int main(int argc, char* argv[])
#endif
{   
#ifdef WINCE
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);
	srand((int)sysTime.wMilliseconds);
#else
    srand((int)time(NULL));
#endif

	GSP::CIPkgConverter  *cnv = CreateGSPPkgConverter(GS_CODEID_HK_VDEFAULT, ePkgType_H264);

	struct tm stTm;
	bzero(&stTm, sizeof(stTm));
	unsigned long tv =  gs_mktime(2012, 7, 3, 12, 34, 12);
	gs_timesub(&tv, &stTm);

	
	stTm.tm_mon +=1;
	stTm.tm_year += 1900;



   s_csWRMutex = new CGSWRMutex();
     s_pITest = &s_iTest;
    CGSThread csThread1;
    CGSThread  csThread2;
    CGSThread csThread3;
    CGSThread csThread4;
     CGSThread csThread5;
     CGSThread csThread6;

// 	 CCircleQueue<INT64*> csQuenue1(50,NULL);
// 	 CCircleQueue<INT64*> csQuenue2(1150,NULL);
// 	 CCircleQueue<INT64*> csQuenue3(2150,NULL);
// 
// 
// 
//     csThread1.Start(OnTestCircleQueueReadEntry,(void*)&csQuenue1); 
//     csThread2.Start(OnTestCircleQueueWriteEntry,(void*)&csQuenue1);
// 
// 
//      csThread3.Start(OnTestCircleQueueReadEntry,(void*)&csQuenue2);
//      csThread4.Start(OnTestCircleQueueWriteEntry,(void*)&csQuenue2);
//    
//      csThread5.Start(OnTestCircleQueueReadEntry,(void*)&csQuenue3);
//       csThread6.Start(OnTestCircleQueueWriteEntry,(void*)&csQuenue3);

	 GSP::GSPModuleInit();

     OptionsEntery(_Options);


	  GSP::GSPModuleUnint();

	  int c = getchar();
    return 0;
}

