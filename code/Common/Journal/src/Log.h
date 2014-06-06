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
#ifndef JOU_LOG_DEF_H
#define JOU_LOG_DEF_H


#include "JouObj.h"
#include <time.h>

namespace JOU
{


class CLog :
    public CJouObj
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
       LV_ALL = (LV_FATAL|LV_ERROR|LV_WARN|LV_DEBUG|LV_INFO)
    }EnumLogLevel;

    typedef enum
    {
       DIR_CONSOLE = 0x01,
       DIR_FILE =    0x02,
    } EnumLogDirection;

    UINT m_iLVDirConsole;
    UINT m_iLVDirFile;
    CGSString m_strLogPath;

	CLog( ILogLibrary *pILog=NULL );
	virtual ~CLog(void);

 

    static CLog *GetGlobalLog(void)
    {
        return s_pGlobalLog;
    }
     
    virtual void SetLogPath(const char *czPathName);

    virtual void SetLogLevel( EnumLogDirection eDirect, EnumLogLevel eLevel);

    virtual void Message(EnumLogLevel eLevel,const char *czPrefix,const char  *czFormat,...);
};

#define g_pLog (JOU::CLog::s_pGlobalLog)

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
	char szBuf[512];
	va_list marker;

	va_start( marker, lpFormat );
	int i = vsnprintf( szBuf,512,lpFormat, marker );    
	va_end( marker );
	if( i< 1)
	{
		return;
	}
	szBuf[511] = '\0';
	OutputDebugString((LPCTSTR) szBuf );
}
#else

#define MY_PRINTF printf

#endif


#ifndef DEBUG_PRINTF
#define DEBUG_PRINTF MY_PRINTF
#endif

#define MY_MESSAGE(x, ...)  DEBUG_PRINTF( "Message "__FILE__ ":%d  " x,__LINE__, __VA_ARGS__)
#define MY_WARNING(x, ...)  DEBUG_PRINTF( "Warning "__FILE__ ":%d  " x,__LINE__, __VA_ARGS__)  
#define MY_DEBUG(x, ...)    DEBUG_PRINTF( "Debug "__FILE__ ":%d  " x, __LINE__, __VA_ARGS__)  
#define MY_ERROR(x, ...)    DEBUG_PRINTF( "Error "__FILE__ ":%d  " x, __LINE__, __VA_ARGS__)  


#define MY_LOG_FATAL( xLog, xFormat, ...  ) \
	(xLog)->Message(JOU::CLog::LV_FATAL, __FILE__, " (%s:%d) " xFormat,  __FUNCTION__,__LINE__, __VA_ARGS__)

#define MY_LOG_ERROR( xLog, xFormat, ...  ) \
    (xLog)->Message(JOU::CLog::LV_ERROR, __FILE__, " (%s:%d) " xFormat, __FUNCTION__, __LINE__, __VA_ARGS__)

#define MY_LOG_WARN( xLog, xFormat, ...  ) \
    (xLog)->Message(JOU::CLog::LV_WARN, __FILE__, " (%s:%d) " xFormat,  __FUNCTION__,__LINE__, __VA_ARGS__)

#define MY_LOG_DEBUG( xLog,  xFormat, ...  ) \
    (xLog)->Message(JOU::CLog::LV_DEBUG, __FILE__, " (%s:%d) " xFormat, __FUNCTION__, __LINE__, __VA_ARGS__)

#define MY_LOG_INFO( xLog, xFormat, ...  ) \
    (xLog)->Message(GSS::CLog::LV_INFO, __FILE__, " (%s:%d) " xFormat, __FUNCTION__, __LINE__, __VA_ARGS__)

#ifdef ENABLE_LOG_STEP
#define MY_LOG_STEP( xFormat, ...  ) \
	do{ if( JOU::CLog::s_pGlobalLog ) JOU::CLog::s_pGlobalLog->Message(JOU::CLog::LV_DEBUG, __FILE__, " (%s %d) -%lld- " xFormat, __FUNCTION__, __LINE__,(long long) DoGetTickCount(), __VA_ARGS__); } while(0)


#define MY_LOG_FUNC( xFormat, ...  ) \
	CFunctionDebug _FuncDebug(JOU::CLog::s_pGlobalLog,__FUNCTION__, xFormat,  __VA_ARGS__ );
#endif

#else



#define MY_PRINTF(args...) fprintf(stderr, args)

#ifndef DEBUG_PRINTF
#define DEBUG_PRINTF MY_PRINTF
#endif


#define MY_MESSAGE(x, ...)  DEBUG_PRINTF( "Message %s:%d  " x,__FILE__ , __LINE__ , ##__VA_ARGS__)
#define MY_WARNING(x, ...)  DEBUG_PRINTF( "Warning %s:%d  " x,__FILE__ , __LINE__ , ##__VA_ARGS__ 
#define MY_DEBUG(x, ...)  DEBUG_PRINTF( "Debug %s:%d  " x, __FILE__ , __LINE__ , ##__VA_ARGS__) #define MY_ERROR(x, ...)  DEBUG_PRINTF( "Error %s:%d  " x, __FILE__ , __LINE__ , ##__VA_ARGS__)      



#define MY_LOG_FATAL( xLog,  xFormat, ...  ) \
    (xLog)->Message(JOU::CLog::LV_FATAL, __FILE__, " (%s:%d) " xFormat, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define MY_LOG_ERROR( xLog,  xFormat, ...  ) \
    (xLog)->Message(JOU::CLog::LV_ERROR, __FILE__, " (%s:%d) " xFormat, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define MY_LOG_WARN( xLog,  xFormat, ...  ) \
    (xLog)->Message(JOU::CLog::LV_WARN, __FILE__, " (%s:%d) " xFormat, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define MY_LOG_DEBUG( xLog,  xFormat, ...  ) \
    (xLog)->Message(JOU::CLog::LV_DEBUG, __FILE__, " (%s:%d) " xFormat, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define MY_LOG_INFO( xLog, xFormat, ...  ) \
    (xLog)->Message(JOU::CLog::LV_INFO, __FILE__, " (%s:%d) " xFormat, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#ifdef ENABLE_LOG_STEP
#define MY_LOG_STEP( xFormat, ...  ) \
    do{ if( JOU::CLog::s_pGlobalLog ) JOU::CLog::s_pGlobalLog->Message(JOU::CLog::LV_DEBUG, __FILE__, " (%s %d) -%lld- " xFormat, __FUNCTION__, __LINE__,,(long long) DoGetTickCount(), ##__VA_ARGS__);} while(0)

#define MY_LOG_FUNC( xFormat, ...  ) \
	CFunctionDebug _FuncDebug(JOU::CLog::s_pGlobalLog, __FUNCTION__,xFormat, ##__VA_ARGS__ );
#endif


#endif

};


#endif
