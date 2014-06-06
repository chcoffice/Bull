#include "OSThread.h"
#include <time.h>
#ifdef _WIN32
#include <direct.h>
#include <Mmsystem.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <stdio.h>

#include "GSPMemory.h"
#include "List.h"
#include "BTree.h"
#include "ISocket.h"
#include "ThreadPool.h"
#include "MainLoop.h"
#include "Log.h"
using namespace GSP;




/*
****************************************
brief : CSection 的实现
****************************************
*/

CSection::CSection(void)
{
#ifdef _WIN32
    ::InitializeCriticalSection(&m_stSection);
#else 
    if( ::pthread_mutexattr_init(&m_stAttr) )
    {
        GSP_ASSERT_EXIT(0, -1);
    }
    if( ::pthread_mutexattr_settype(&m_stAttr, PTHREAD_MUTEX_RECURSIVE) ) 
    {        
        //设为嵌套锁
        GSP_ASSERT_EXIT(0, -1);
    }
     if( ::pthread_mutex_init( &m_stSection,&m_stAttr) )
     {                          
         GSP_ASSERT_EXIT(0, -1);
     }
#endif
}

CSection::~CSection(void)
{
#ifdef _WIN32
    ::DeleteCriticalSection(&m_stSection);
#else
    ::pthread_mutex_destroy(&m_stSection);
    ::pthread_mutexattr_destroy(&m_stAttr);
#endif
}

BOOL CSection::Lock(void)
{
#ifdef _WIN32
	GS_ASSERT( 0==m_stSection.SpinCount );

    ::EnterCriticalSection(&m_stSection);   
#else
    if( pthread_mutex_lock(&m_stSection) )
    {
        GSP_ASSERT(0);
        return FALSE;
    }
#endif
    return TRUE;
}

BOOL CSection::Unlock(void)
{
#ifdef _WIN32
    ::LeaveCriticalSection(&m_stSection);   
#else
    if( pthread_mutex_unlock(&m_stSection) )
    {
          GSP_ASSERT(0);
        return FALSE;
    }
#endif
    return TRUE;
}


/*
****************************************
brief : OSThread 的实现
****************************************
*/



INT64 COSThread::Milliseconds(void)
{ 
#ifdef _WIN32 
    SYSTEMTIME stTm;
    struct tm stLcTm = {0};
    ::GetSystemTime(&stTm);
    stLcTm.tm_year = stTm.wYear-1900;  
    stLcTm.tm_mon = stTm.wMonth-1;  
    stLcTm.tm_mday = stTm.wDay;  
    stLcTm.tm_hour = stTm.wHour;  
    stLcTm.tm_min = stTm.wMinute;  
    stLcTm.tm_sec = stTm.wSecond;

    INT64 curTime;
    curTime = mktime(&stLcTm);
    curTime *= 1000;                // sec -> msec
    curTime += stTm.wMilliseconds;    // usec -> msec
    return curTime;
#else
    struct timeval t;
    int theErr = ::gettimeofday(&t, NULL);
    GSP_ASSERT(theErr == 0);

    INT64 curTime;
    curTime = t.tv_sec;
    curTime *= 1000;                // sec -> msec
    curTime += t.tv_usec / 1000;    // usec -> msec
    return curTime;
  //  return (curTime - sInitialMsec) + sMsecSince1970;
#endif
}

// BOOL COSThread::CTime( UINT32 iSecs, CGSSString &stFmt )
// {
//     time_t t = iSecs;
// #ifdef _WIN32
//     stFmt = ctime(&t );
// #else
//     CAtomic::Lock();
//     stFmt = ctime(&t );
//     CAtomic::Unlock();
// #endif
//     return TRUE;
// }

BOOL COSThread::GetCTimeOfGMT( CGSPString &stFmt)
{
	char szTemp[200];
#ifdef _WINCE
	SYSTEMTIME SystemTime;
	GetSystemTime(&SystemTime);
	WCHAR dateFormat[] = L"ddd, MMM dd yyyy";
	WCHAR timeFormat[] = L"HH:mm:ss GMT\r\n";
	WCHAR inBuf[200];
	DWORD locale = LOCALE_NEUTRAL;

	int ret = GetDateFormat(locale, 0, &SystemTime,
		(LPTSTR)dateFormat, (LPTSTR)inBuf, sizeof(inBuf) );
	inBuf[ret - 1] = ' ';
	ret = GetTimeFormat(locale, 0, &SystemTime,
		(LPTSTR)timeFormat,
		(LPTSTR)inBuf + ret, (sizeof inBuf) - ret);
	wcstombs(szTemp, inBuf, wcslen(inBuf));
#else
	time_t tt = time(NULL);
	strftime(szTemp, sizeof(szTemp), "%a, %b %d %Y %H:%M:%S GMT", gmtime(&tt));
#endif
	stFmt = szTemp;
	return TRUE;
}

INT COSThread::CurrentDeviceCPUNumber(void)
{
static GSAtomicInter s_iCpus = -1;
     if( s_iCpus != -1 )
     {
         return s_iCpus;
     }
#ifdef _WIN32
     typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
    SYSTEM_INFO si;
    // Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
    PGNSI pfnGNSI = (PGNSI) GetProcAddress(GetModuleHandle((LPCTSTR)"kernel32.dll"), "GetNativeSystemInfo");
    if(pfnGNSI)
    {
        pfnGNSI(&si);
    }
    else
    {
        GetSystemInfo(&si);
    }
    AtomicInterCompareExchange( s_iCpus,-1, si.dwNumberOfProcessors  );
#else
    FILE * pFile = fopen("/proc/cpuinfo", rb);
    if (!pFile)
        return 1;
    int iFileLen = KBYTES*512; //缓存区置的大些
    char * pBuff = NULL;
    int iRet = 0;
    int iCnts = 0;
    while( 1 )
    {
        
        pBuff = (char*)malloc(iFileLen);
        GS_ASSERT_EXIT(pBuff, -1);
        fseek(pFile, 0L, SEEK_SET);   
        pBuff[0] ='\0';
        iRet = fread(pBuff, 1 , iFileLen, pFile);//返回文件长度
        if( iRet < iFileLen )
        {
            break;
        }
        free(pBuff);
        iFileLen += KBYTES*512;
    } 
    char *p = pBuff;
    do
    {
        p = strstr(p, "processor");
        if( p )
        {
           iCnts++;
           p += 6;
        }        
    }while(p);
    free(pBuff);
    AtomicInterCompareExchange( s_iCpus,-1, si.dwNumberOfProcessors  );
#endif
    if( s_iCpus < 1 )
    {
        GS_ASSERT(0);
        AtomicInterSet(s_iCpus, 1);
    }
    return s_iCpus;
}


namespace GSP
{
    typedef struct _StrucInfoNameNode
    {
        INT iType;
        char *czName;
    }StrucInfoNameNode;
#define  INFO_NAME_NODE_VALUE( t, stv ) { (INT)(t), stv }




    static const char *_GetInfoName(const StrucInfoNameNode *pVNode, INT iVNodeNums,  INT iType, const char *pDefault )
    {
        for( INT i=0; i<iVNodeNums; i++ )
        {
            if( pVNode[i].iType==iType )
            {
                return pVNode[i].czName;
            }
        }
        return  pDefault;
    }

    static INT _GetInfoType(const StrucInfoNameNode *pVNode, INT iVNodeNums,  const char *czName, INT iDefault )
    {
        for( INT i=0; i<iVNodeNums; i++ )
        {
            if( strcasecmp(pVNode[i].czName, czName)==0 )
            {
                return pVNode[i].iType;
            }
        }
        return  iDefault;
    }

    static const StrucInfoNameNode s_sProList[] =
    {
        INFO_NAME_NODE_VALUE(ePRO_GSP,  _GSTX("gsp") ),
        INFO_NAME_NODE_VALUE(ePRO_RTSP, _GSTX("rtsp") ),
		INFO_NAME_NODE_VALUE(ePRO_SIP, _GSTX("sip") ),
        INFO_NAME_NODE_VALUE(ePRO_UNK,  _GSTX("unk") ),
    };

     const char *GetProtocolName( EnumProtocol ePro )
    {
        INT iCnts = ARRARY_SIZE(s_sProList);
        return _GetInfoName(s_sProList, iCnts-1,(INT)ePro,  s_sProList[iCnts-1].czName );
    }

     EnumProtocol GetProtocolType( const char *strName)
    {
        INT iCnts = ARRARY_SIZE(s_sProList);
        return (EnumProtocol)_GetInfoType(s_sProList, iCnts-1,strName, s_sProList[iCnts-1].iType );
    }

//      const char *GetPlayCommandName( EnumPlayCommand eCommand )
//     {
//         static const StrucInfoNameNode s_sPlayCmdList[] =
//         {
//             INFO_NAME_NODE_VALUE(ePLAY_CMD_PLAY, _GSTX("播放") ),
//             INFO_NAME_NODE_VALUE(ePLAY_CMD_STOP, _GSTX("停止")  ), 
//             INFO_NAME_NODE_VALUE(ePLAY_CMD_PAUSE, _GSTX("暂停") ),  
//             INFO_NAME_NODE_VALUE(ePLAY_CMD_FAST, _GSTX("快进")  ),  
//             INFO_NAME_NODE_VALUE(ePLAY_CMD_BFAST, _GSTX("快退")  ),  
//             INFO_NAME_NODE_VALUE(ePLAY_CMD_STEP, _GSTX("单帧前")  ),  
//             INFO_NAME_NODE_VALUE(ePLAY_CMD_BSTEP, _GSTX("单帧后")  ),  
//             INFO_NAME_NODE_VALUE(ePLAY_CMD_SLOW, _GSTX("慢进")  ), 
//             INFO_NAME_NODE_VALUE(ePLAY_CMD_BSLOW, _GSTX("慢退 ")  ),  
//             INFO_NAME_NODE_VALUE(ePLAY_CMD_FILTER, _GSTX("过滤")  ),  
//             INFO_NAME_NODE_VALUE(ePLAY_CMD_SETPERCENT, _GSTX("拖动:百分比")  ),        
//             INFO_NAME_NODE_VALUE(ePLAY_CMD_SETPOINT, _GSTX("拖动:绝对值  ")  ), 
//             INFO_NAME_NODE_VALUE(ePLAY_CMD_SETTIME, _GSTX("拖动:时间点")  ),
//             INFO_NAME_NODE_VALUE(0, _GSTX("未知") ),
//         };
// 
//         INT iCnts = ARRARY_SIZE(s_sPlayCmdList);
//         return _GetInfoName(s_sPlayCmdList, iCnts-1,(INT)eCommand, s_sPlayCmdList[iCnts-1].czName );
//     };

//      const char *GetModuleStatusName(  EnumModuleStatus eStatus )
//     {
//         static const StrucInfoNameNode s_stStatusList[] =
//         {
//             INFO_NAME_NODE_VALUE(eSTATUS_MOD_UINIT, _GSTX("未初始化") ), 
//             INFO_NAME_NODE_VALUE(eSTATUS_MOD_RUNNING, _GSTX("运行中") ),  
//             INFO_NAME_NODE_VALUE(eSTATUS_MOD_STOP, _GSTX("停止")  ), 
//             INFO_NAME_NODE_VALUE(eSTATUS_MOD_ERR, _GSTX("错误")  ),  
//             INFO_NAME_NODE_VALUE(-1, _GSTX("未知") ),
//         };
//         INT iCnts = ARRARY_SIZE(s_stStatusList);
//         return _GetInfoName(s_stStatusList, iCnts-1,(INT)eStatus, s_stStatusList[iCnts-1].czName );
// 
//     }

    const char *GetMediaName( EnumGSMediaType eType)
    {
        static const StrucInfoNameNode s_stMTypeList[] =  
        {   
            INFO_NAME_NODE_VALUE(GS_MEDIA_TYPE_NONE, _GSTX("None")), 
            INFO_NAME_NODE_VALUE(GS_MEDIA_TYPE_VIDEO,_GSTX("Video")), 
            INFO_NAME_NODE_VALUE(GS_MEDIA_TYPE_AUDIO,_GSTX("Audio")), 
            INFO_NAME_NODE_VALUE(GS_MEDIA_TYPE_PICTURE,_GSTX("Picture")), 
            INFO_NAME_NODE_VALUE(GS_MEDIA_TYPE_OSD,_GSTX("OSD") ), 
            INFO_NAME_NODE_VALUE(GS_MEDIA_TYPE_BINARY,_GSTX("Binary") ),
            INFO_NAME_NODE_VALUE(GS_MEDIA_TYPE_SYSHEADER,_GSTX("SysHeader") ),
            INFO_NAME_NODE_VALUE(-1,_GSTX("Unknown") ),
        };
        INT iCnts = ARRARY_SIZE(s_stMTypeList);
        return _GetInfoName(s_stMTypeList, iCnts-1,(INT)eType, s_stMTypeList[iCnts-1].czName );

    }


	const char *GetError( EnumErrno eErrno )
	{
		static const StrucInfoNameNode s_stErrnoList[] =  
		{
			INFO_NAME_NODE_VALUE(eERRNO_SUCCESS,  _GSTX("成功") ),
			INFO_NAME_NODE_VALUE(eERRNO_EUNKNOWN,  _GSTX("未知错误") ),
			INFO_NAME_NODE_VALUE(eERRNO_EFATAL,  _GSTX("未知的不可修复的错误") ),
			INFO_NAME_NODE_VALUE(eERRNO_ENONE,  _GSTX("无错误") ),


			INFO_NAME_NODE_VALUE(eERRNO_SYS_EINVALID,  _GSTX("无效参数") ),
			INFO_NAME_NODE_VALUE(eERRNO_SYS_EFLOWOUT,  _GSTX("缓冲溢出") ),
			INFO_NAME_NODE_VALUE(eERRNO_SYS_ENMEM,  _GSTX("没有内存") ),
			INFO_NAME_NODE_VALUE(eERRNO_SYS_EOPER,  _GSTX("不支持的操作") ),
			INFO_NAME_NODE_VALUE(eERRNO_SYS_ESTATUS,  _GSTX("错误状态下的操作") ),
			INFO_NAME_NODE_VALUE(eERRNO_SYS_ENCTRL,  _GSTX("不支持的控制操作") ),
			INFO_NAME_NODE_VALUE(eERRNO_SYS_EPERMIT,  _GSTX("没有权限") ),
			INFO_NAME_NODE_VALUE(eERRNO_SYS_ETIMEOUT,  _GSTX("超时") ),
			INFO_NAME_NODE_VALUE(eERRNO_SYS_EKPL,  _GSTX("keepalive 超时") ),
			INFO_NAME_NODE_VALUE(eERRNO_SYS_EBUSY,  _GSTX("资源缺乏") ),
			INFO_NAME_NODE_VALUE(eERRNO_SYS_ETKP,  _GSTX("任务操作失败") ),
			INFO_NAME_NODE_VALUE(eERRNO_SYS_EFUNC,  _GSTX("该功能没有实现") ),
			INFO_NAME_NODE_VALUE(eERRNO_SYS_EPRO,  _GSTX("不支持的协议") ),
			INFO_NAME_NODE_VALUE(eERRNO_SYS_EPERMIT,  _GSTX("没有权限") ),
			INFO_NAME_NODE_VALUE(eERRNO_SYS_ENEXIST,  _GSTX("对象已经不存在") ),
			


			INFO_NAME_NODE_VALUE(eERRNO_NET_EUNK,  _GSTX("网络错误: 未知的网络操作错误") ),
			INFO_NAME_NODE_VALUE(eERRNO_NET_EDISCNN,  _GSTX("网络错误: 对端网络关闭") ),
			INFO_NAME_NODE_VALUE(eERRNO_NET_ECLOSED,  _GSTX("网络错误: Socket 已经关闭") ),
			INFO_NAME_NODE_VALUE(eERRNO_NET_EREG,  _GSTX("网络错误: 注册失败") ),
			INFO_NAME_NODE_VALUE(eERRNO_NET_EWEVT,  _GSTX("网络错误: 请求写事件失败") ),
			INFO_NAME_NODE_VALUE(eERRNO_NET_EREVT,  _GSTX("网络错误: 请求写事件失败") ),
			INFO_NAME_NODE_VALUE(eERRNO_NET_EEVT,  _GSTX("网络错误: 请求事件失败") ),
			INFO_NAME_NODE_VALUE(eERRNO_NET_EBIN,  _GSTX("网络错误: 绑定端口失败") ),
			INFO_NAME_NODE_VALUE(eERRNO_NET_EOPT,  _GSTX("网络错误: 属性操作失败") ),
			INFO_NAME_NODE_VALUE(eERRNO_NET_ESEND,  _GSTX("网络错误: 写错误") ),
			INFO_NAME_NODE_VALUE(eERRNO_NET_ERCV,  _GSTX("网络错误: 读错误") ),
			INFO_NAME_NODE_VALUE(eERRNO_NET_ESOCKET,  _GSTX("网络错误: 建立SOCKET 失败") ),


			INFO_NAME_NODE_VALUE(eERRNO_SRC_ENXIST,  _GSTX("数据源不存在") ),
			INFO_NAME_NODE_VALUE(eERRNO_SRC_EUNUSED,  _GSTX("数据源无人使用") ),
			INFO_NAME_NODE_VALUE(eERRNO_SRC_ECLOSE,  _GSTX("数据源被关闭") ),
			INFO_NAME_NODE_VALUE(eERRNO_SRC_EASSERT,  _GSTX("数据源异常") ),
			INFO_NAME_NODE_VALUE(eERRNO_SRC_ECODER,  _GSTX("数据源不支持编码类型") ),
			


			INFO_NAME_NODE_VALUE(eERRNO_CLI_ECLOSE,  _GSTX("客户端错误 对端请求关闭") ),
			INFO_NAME_NODE_VALUE(eERRNO_CLI_EASSERT,  _GSTX("客户端错误 对端异常关闭") ),
			INFO_NAME_NODE_VALUE(eERRNO_CLI_ENSRC,  _GSTX("客户端错误 请求不存在数据源") ),


			INFO_NAME_NODE_VALUE(eERRNO_SRV_REFUSE,  _GSTX("上层拒绝连接") ),


			INFO_NAME_NODE_VALUE(eERRNO_EVT_CONTINUE_NEXT,  _GSTX("Continue Next") ),


			INFO_NAME_NODE_VALUE(-1,    _GSTX("无效错误号") ),
		};
		INT iCnts = ARRARY_SIZE(s_stErrnoList);
		return _GetInfoName(s_stErrnoList, iCnts-1,(INT)eErrno, s_stErrnoList[iCnts-1].czName );
	}


// 	GS_API const char *GetClientStatusText( EnumClientStatus eStatus)
// 	{
// 		static const StrucInfoNameNode s_stErrnoList[] =  
// 		{
// 			INFO_NAME_NODE_VALUE(eST_CLI_INIT,  _GSTX("初始化") ),
// 			INFO_NAME_NODE_VALUE(eST_CLI_CONNECT,  _GSTX("等待打开") ),
// 			INFO_NAME_NODE_VALUE(eST_CLI_CONNECT,  _GSTX("连接") ),
// 			INFO_NAME_NODE_VALUE(eST_CLI_READY,  _GSTX("准备") ),			
// 			INFO_NAME_NODE_VALUE(eST_CLI_CONNECTING,  _GSTX("连接中") ),
// 			INFO_NAME_NODE_VALUE(eST_CLI_WAIT_REQUEST,  _GSTX("等待URI请求") ),
// 			INFO_NAME_NODE_VALUE(eST_CLI_REQUEST,  _GSTX("已请求URI") ),
// 			INFO_NAME_NODE_VALUE(eST_CLI_REQUESTING,  _GSTX("正在请求URI") ),	
// 			INFO_NAME_NODE_VALUE(eST_CLI_WAIT_START,  _GSTX("等待开始") ),
// 			INFO_NAME_NODE_VALUE(eST_CLI_PLAYING,  _GSTX("播放中") ),
// 			INFO_NAME_NODE_VALUE(eST_CLI_STOP,  _GSTX("停止") ),
// 			INFO_NAME_NODE_VALUE(eST_CLI_ASSERT,  _GSTX("异常") ),
// 			INFO_NAME_NODE_VALUE(-1,    _GSTX("未知状态") ),
// 		};
// 		INT iCnts = ARRARY_SIZE(s_stErrnoList);
// 		return _GetInfoName(s_stErrnoList, iCnts-1,(INT)eStatus, s_stErrnoList[iCnts-1].czName );
// 	}


}


/*
*********************************************************************
*
*@brief : 模块初始化
*
*********************************************************************
*/
namespace GSP
{


typedef void (*FuncPtrModuleInit)(void);

typedef struct _StruInitFunc
{
	FuncPtrModuleInit fnInit;
	FuncPtrModuleInit fnUninit;
}StruInitFunc;

static GSAtomicInter _s_iInitRefs = 0;

static StruInitFunc s_InitFunc[] =
{              
	{ CMemoryPool::InitModule, CMemoryPool::UninitModule},
	{ CGSPThreadPool::InitModule, CGSPThreadPool::UninitModule}, 
	{ CMainLoop::InitModule, CMainLoop::UninitModule},
	{ CISocket::InitModule, CISocket::UninitModule},
	
};


void GSPModuleInit(void)
{
	if( AtomicInterInc(_s_iInitRefs ) > 1 )
	{
		return;
	}
	CGSPObject::g_bModuleRunning = TRUE;
	for( int i =0; i<ARRARY_SIZE(s_InitFunc); i++ )
	{
		if( s_InitFunc[i].fnInit )
		{
			s_InitFunc[i].fnInit();
		}
	}
}

void GSPModuleUnint(void)
{
	if(_s_iInitRefs>0 && AtomicInterDec(_s_iInitRefs ) == 0 )
	{
		CGSPObject::g_bModuleRunning = FALSE;
		
		for( int i=ARRARY_SIZE(s_InitFunc); i>0; i-- )
		{
			if( i==1 )
			{
				if( g_pLog )
				{
					g_pLog->UnrefObject();
					g_pLog = NULL;
				}
			}
			if( s_InitFunc[i-1].fnUninit )
			{
				s_InitFunc[i-1].fnUninit();
			}
		}

	}

}

} //end namespace GSP