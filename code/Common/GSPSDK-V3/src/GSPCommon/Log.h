/*
******************************************
Copyright (C), 2010-2011, GOSUN
Filename : LOG.H
Author :  zouyx
Version : 0.1.0.0
Date: 2010/8/20 15:09
Description: 日志封装类
********************************************
*/
#ifndef GSP_LOG_DEF_H
#define GSP_LOG_DEF_H


#include "GSPObject.h"
#include <time.h>

namespace GSP
{


	class CLog :
		public CRefObject
	{

	public:
		static CLog *s_pGlobalLog;
		ILogLibrary *m_pILog;


		typedef enum
		{
			LV_FATAL = LFATAL,
			LV_ERROR = LERROR,
			LV_WARN = LWARN,
			LV_DEBUG = LDEBUG,
			LV_INFO = LINFO,
			LV_NOTICE = LNOTICE,
			LV_ALL = (LV_FATAL|LV_ERROR|LV_WARN|LV_DEBUG|LV_INFO|LV_NOTICE)
		}EnumLogLevel;

		typedef enum
		{
			DIR_CONSOLE = 0x01,
			DIR_FILE =    0x02,
		} EnumLogDirection;

		UINT m_iLVDirConsole;
		UINT m_iLVDirFile;
		CGSString m_strLogPath;

		static CLog *GetGlobalLog(void)
		{
			return s_pGlobalLog;
		}

		virtual void SetLogPath(const char *czPathName);

		virtual void SetLogLevel( EnumLogDirection eDirect, EnumLogLevel eLevel);

		virtual void Message(EnumLogLevel eLevel,const char *czPrefix,const char  *czFormat,...);

		static CLog *Create(ILogLibrary *pILog=NULL)
		{
			return new CLog(pILog);
		}
	protected :
		CLog( ILogLibrary *pILog=NULL );
		virtual ~CLog(void);
	};

#define g_pLog (GSP::CLog::s_pGlobalLog)

#define ENABLE_LOG_STEP

#ifdef ENABLE_LOG_STEP

	class CFunctionDebug
	{
	public :
		CGSString m_strInfo;
		CGSString m_strFunc;
		CLog *m_pLog;

		CFunctionDebug(CLog *pLog, const CGSString &strFunc, const char  *czFormat,...);

		~CFunctionDebug(void)
		{
			if( m_pLog )
			{
				m_pLog->Message( CLog::LV_DEBUG, "FUNC", 
					"==%lld== **%s** LEAVE %s.\n", 
					(long long) DoGetTickCount(),
					m_strFunc.c_str(), 
					m_strInfo.c_str());
			}
			else
			{
				printf("DEBUG FUNC  ==%lld== **%s** LEAVE %s.\n", 
					(long long) DoGetTickCount(),
					m_strFunc.c_str(), 
					m_strInfo.c_str());
			}
		}


	};

#else

#define MY_LOG_STEP(... ) do{}while(0)

#endif


#if !defined(DEBUG) && !defined(_DEBUG)

#define MY_DEBUG_PRINTF(...)

#endif




#ifdef _WIN32

#if 0
	static INLINE  void MY_PRINTF(const char * lpFormat, ...)
	{
		char szBuf[1024];
		va_list marker;

		va_start( marker, lpFormat );
		int i = vsnprintf( szBuf,1024,lpFormat, marker );    
		va_end( marker );
		if( i< 1)
		{
			return;
		}
		szBuf[1023] = '\0';
		OutputDebugString((LPCTSTR) szBuf );
	}
#else

#define MY_PRINTF printf

#endif


#ifndef MY_DEBUG_PRINTF
#define MY_DEBUG_PRINTF MY_PRINTF
#endif

#define MY_MESSAGE(x, ...)  MY_DEBUG_PRINTF( "Message "__FILE__ ":%d  " x,__LINE__, __VA_ARGS__)
#define MY_WARNING(x, ...)  MY_DEBUG_PRINTF( "Warning "__FILE__ ":%d  " x,__LINE__, __VA_ARGS__)  
#define MY_DEBUG(x, ...)    MY_DEBUG_PRINTF( "Debug "__FILE__ ":%d  " x, __LINE__, __VA_ARGS__)  
#define MY_ERROR(x, ...)    MY_DEBUG_PRINTF( "Error "__FILE__ ":%d  " x, __LINE__, __VA_ARGS__)  


#define MY_LOG_FATAL( xLog, xFormat, ...  ) \
	(xLog)->Message(GSP::CLog::LV_FATAL, __FILE__, " (%s:%d) " xFormat,  __FUNCTION__,__LINE__, __VA_ARGS__)

#define MY_LOG_ERROR( xLog, xFormat, ...  ) \
	(xLog)->Message(GSP::CLog::LV_ERROR, __FILE__, " (%s:%d) " xFormat, __FUNCTION__, __LINE__, __VA_ARGS__)

#define MY_LOG_WARN( xLog, xFormat, ...  ) \
	(xLog)->Message(GSP::CLog::LV_WARN, __FILE__, " (%s:%d) " xFormat,  __FUNCTION__,__LINE__, __VA_ARGS__)

#define MY_LOG_DEBUG( xLog,  xFormat, ...  ) \
	(xLog)->Message(GSP::CLog::LV_DEBUG, __FILE__, " (%s:%d) " xFormat, __FUNCTION__, __LINE__, __VA_ARGS__)

#define MY_LOG_INFO( xLog, xFormat, ...  ) \
	(xLog)->Message(GSP::CLog::LV_INFO, __FILE__, " (%s:%d) " xFormat, __FUNCTION__, __LINE__, __VA_ARGS__)

#define MY_LOG_NOTICE( xLog, xFormat, ...  ) \
	(xLog)->Message(GSP::CLog::LV_NOTICE, __FILE__, " (%s:%d) " xFormat, __FUNCTION__, __LINE__, __VA_ARGS__)

#ifdef ENABLE_LOG_STEP
#define MY_LOG_STEP( xFormat, ...  ) \
	do{ if( GSP::CLog::s_pGlobalLog ) GSP::CLog::s_pGlobalLog->Message(GSP::CLog::LV_DEBUG, __FILE__, " (%s %d) -%lld- " xFormat, __FUNCTION__, __LINE__,(long long) DoGetTickCount(), __VA_ARGS__); } while(0)


#define MY_LOG_FUNC( xFormat, ...  ) \
	CFunctionDebug _FuncDebug(GSP::CLog::s_pGlobalLog,__FUNCTION__, xFormat,  __VA_ARGS__ );
#endif

#else



#define MY_PRINTF(args...) fprintf(stderr, args)

#ifndef MY_DEBUG_PRINTF
#define MY_DEBUG_PRINTF MY_PRINTF
#endif


#define MY_MESSAGE(x, ...)  MY_DEBUG_PRINTF( "Message %s:%d  " x,__FILE__ , __LINE__ , ##__VA_ARGS__)
#define MY_WARNING(x, ...)  MY_DEBUG_PRINTF( "Warning %s:%d  " x,__FILE__ , __LINE__ , ##__VA_ARGS__ 
#define MY_DEBUG(x, ...)  MY_DEBUG_PRINTF( "Debug %s:%d  " x, __FILE__ , __LINE__ , ##__VA_ARGS__) #define MY_ERROR(x, ...)  MY_DEBUG_PRINTF( "Error %s:%d  " x, __FILE__ , __LINE__ , ##__VA_ARGS__)      



#define MY_LOG_FATAL( xLog,  xFormat, ...  ) \
	(xLog)->Message(GSP::CLog::LV_FATAL, __FILE__, " (%s:%d) " xFormat, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define MY_LOG_ERROR( xLog,  xFormat, ...  ) \
	(xLog)->Message(GSP::CLog::LV_ERROR, __FILE__, " (%s:%d) " xFormat, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define MY_LOG_WARN( xLog,  xFormat, ...  ) \
	(xLog)->Message(GSP::CLog::LV_WARN, __FILE__, " (%s:%d) " xFormat, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define MY_LOG_DEBUG( xLog,  xFormat, ...  ) \
	(xLog)->Message(GSP::CLog::LV_DEBUG, __FILE__, " (%s:%d) " xFormat, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define MY_LOG_INFO( xLog, xFormat, ...  ) \
	(xLog)->Message(GSP::CLog::LV_INFO, __FILE__, " (%s:%d) " xFormat, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define MY_LOG_NOTICE( xLog, xFormat, ...  ) \
	(xLog)->Message(GSP::CLog::LV_NOTICE, __FILE__, " (%s:%d) " xFormat, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#ifdef ENABLE_LOG_STEP
#define MY_LOG_STEP( xFormat, ...  ) \
	do{ if( GSP::CLog::s_pGlobalLog ) GSP::CLog::s_pGlobalLog->Message(GSP::CLog::LV_DEBUG, __FILE__, " (%s %d) -%lld- " xFormat, __FUNCTION__, __LINE__,,(long long) DoGetTickCount(), ##__VA_ARGS__);} while(0)

#define MY_LOG_FUNC( xFormat, ...  ) \
	CFunctionDebug _FuncDebug(GSP::CLog::s_pGlobalLog, __FUNCTION__,xFormat, ##__VA_ARGS__ );
#endif


#endif





typedef struct _StruStreamInfo
{
    unsigned long m_iStartTv; //开始时间
    unsigned long m_iEndTv; //结束数据
    unsigned long long  m_iLostCnts; //总的丢帧数
    unsigned long long m_iTotals; //中帧数
    unsigned long m_iLastLog; //最后打印时间
    unsigned long m_iLastLostTv; //最后丢帧时间
    unsigned long long m_iNetLost; //网络丢帧数

    void Reset(void)
    {
        m_iStartTv = 0;
        m_iEndTv = 0;
        m_iLostCnts = 0;
        m_iLastLog = 0;
        m_iLastLostTv = 0;
        m_iTotals = 0;
        m_iNetLost = 0;
    }
    void Start(void)
    {
        Reset();
        m_iStartTv = (unsigned long)time(NULL);
    }
    void AddFrames(UINT iNum )
    {
        m_iTotals += iNum;
    }
    BOOL AddLost(UINT iNum )
    {
        m_iTotals += iNum;
        m_iLastLostTv = (unsigned long)time(NULL);
        m_iLostCnts += iNum;
        if( (UINT32)(m_iLastLostTv-m_iLastLog)> 30  )
        {
            m_iLastLog = m_iLastLostTv;
            return TRUE;
        }
        return FALSE;
    }
    void End(void)
    {
        m_iEndTv = (unsigned long)time(NULL);
    }
    void AddNetLost(UINT iNum)
    {
        m_iTotals += iNum;
        m_iNetLost += iNum;
    }
}StruStreamInfo;

#ifdef _WIN32

#define GSS_LOG_STREAM_INFO( xLog,  xInfo, xFormat, ...  ) \
    (xLog)->Message(GSS::CLog::LV_DEBUG, __FILE__, " (%s %d) " "TimeRange[%lu-%lu],Losts[%lu],LastLostTm[%llu],Totals[%llu],NetLost[%llu]. " xFormat, \
    __FUNCTION__, __LINE__, (xInfo)->m_iStartTv,(xInfo)->m_iEndTv,(xInfo)->m_iLostCnts,(xInfo)->m_iLastLostTv,(xInfo)->m_iTotals, (xInfo)->m_iNetLost, __VA_ARGS__)

#else

#define GSS_LOG_STREAM_INFO( xLog, xInfo, xFormat, ...  ) \
    (xLog)->Message(GSS::CLog::LV_DEBUG, __FILE__, " (%s %d) " "TimeRange[%lu-%lu],Losts[%lu],LastLostTm[%llu],Totals[%llu],NetLost[%llu]. " xFormat, \
    __FUNCTION__, __LINE__, (xInfo)->m_iStartTv,(xInfo)->m_iEndTv,(xInfo)->m_iLostCnts,(xInfo)->m_iLastLostTv,(xInfo)->m_iTotals, (xInfo)->m_iNetLost, ##__VA_ARGS__)

#endif

};


#endif
