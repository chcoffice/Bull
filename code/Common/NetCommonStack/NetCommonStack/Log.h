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
#ifndef CMM_LOG_DEF_H
#define CMM_LOG_DEF_H

#include "GSCommon.h"
#include <time.h>
#include <string>
#include <assert.h>
#include <time.h>

namespace CMM
{

    class CLog
    {

    protected :
        ILogLibrary *m_pILog;
    public:
        static CLog *s_pGlobalLog;
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
        std::string m_strLogPath;
        CLog( ILogLibrary *pILog=NULL );
        virtual ~CLog(void);

        static CLog *GetGlobalLog(void)
        {
            return s_pGlobalLog;
        }

        void SetInitFile(const char *czFilename );

        void SetLogPath(const char *czPathName);

        void SetLogLevel( EnumLogDirection eDirect, EnumLogLevel eLevel);

        void Message(EnumLogLevel eLevel,const char *czPrefix,const char  *czFormat,...);
    };


};  //end namespace


#define CMM_ASSERT(x) assert(x)


//#define ENABLE_LOG_STEP

#ifndef ENABLE_LOG_STEP
#define CMM_LOG_STEP(...) do{}while(0)
#endif

#define g_pLog (CMM::CLog::GetGlobalLog())

#ifdef _WIN32

#define LOG_FATAL( xLog, xPrefix, xFormat, ...  ) \
    (xLog)->Message(CMM::CLog::LV_FATAL, xPrefix, " (%s %d) " xFormat, __FILE__, __LINE__, __VA_ARGS__)

#define LOG_ERROR( xLog, xPrefix, xFormat, ...  ) \
    (xLog)->Message(CMM::CLog::LV_ERROR, xPrefix, " (%s %d) " xFormat, __FILE__, __LINE__, __VA_ARGS__)

#define LOG_WARN( xLog, xPrefix, xFormat, ...  ) \
    (xLog)->Message(CMM::CLog::LV_WARN, xPrefix, " (%s %d) " xFormat, __FILE__, __LINE__, __VA_ARGS__)

#define LOG_DEBUG( xLog, xPrefix, xFormat, ...  ) \
    (xLog)->Message(CMM::CLog::LV_DEBUG, xPrefix, " (%s %d) " xFormat, __FILE__, __LINE__, __VA_ARGS__)

#define LOG_INFO( xLog, xPrefix, xFormat, ...  ) \
    (xLog)->Message(CMM::CLog::LV_INFO, xPrefix, " (%s %d) " xFormat, __FILE__, __LINE__, __VA_ARGS__)


#define LOG2_FATAL( xFormat, ...  ) \
    g_pLog->Message(CMM::CLog::LV_FATAL, __FUNCTION__, xFormat, __VA_ARGS__)

#define LOG2_ERROR(  xFormat, ...  ) \
    g_pLog->Message(CMM::CLog::LV_ERROR, __FUNCTION__," Line(%d) " xFormat, __LINE__, __VA_ARGS__)

#define LOG2_WARN(  xFormat, ...  ) \
    g_pLog->Message(CMM::CLog::LV_WARN, __FUNCTION__, " Line(%d) " xFormat, __LINE__, __VA_ARGS__)

#define LOG2_DEBUG(  xFormat, ...  ) \
    g_pLog->Message(CMM::CLog::LV_DEBUG, __FUNCTION__, " Line(%d) " xFormat,__LINE__, __VA_ARGS__)

#define LOG2_INFO(  xFormat, ...  ) \
    g_pLog->Message(CMM::CLog::LV_INFO, __FUNCTION__,  " Line(%d) " xFormat,__LINE__, __VA_ARGS__)


#ifdef ENABLE_LOG_STEP
#define LOG_STEP( xPrefix,xFormat, ...  ) \
    do{ if( g_pLog ) g_pLog->Message(CMM::CLog::LV_DEBUG, xPrefix, " (%s %d) " xFormat, __FUNCTION__, __LINE__, __VA_ARGS__); } while(0)
#endif

#else


#define LOG_FATAL( xLog, xPrefix, xFormat, ...  ) \
    (xLog)->Message(CMM::CLog::LV_FATAL, xPrefix, " (%s %d) " xFormat, __FILE__, __LINE__, ##__VA_ARGS__)

#define LOG_ERROR( xLog, xPrefix, xFormat, ...  ) \
    (xLog)->Message(CMM::CLog::LV_ERROR, xPrefix, " (%s %d) " xFormat, __FILE__, __LINE__, ##__VA_ARGS__)

#define LOG_WARN( xLog, xPrefix, xFormat, ...  ) \
    (xLog)->Message(CMM::CLog::LV_WARN, xPrefix, " (%s %d) " xFormat, __FILE__, __LINE__, ##__VA_ARGS__)

#define LOG_DEBUG( xLog, xPrefix, xFormat, ...  ) \
    (xLog)->Message(CMM::CLog::LV_DEBUG, xPrefix, " (%s %d) " xFormat, __FILE__, __LINE__, ##__VA_ARGS__)

#define LOG2_INFO(   xFormat, ...  ) \
    g_pLog->Message(CMM::CLog::LV_INFO, __FUNCTION__, " Line(%d) " xFormat,  __LINE__, ##__VA_ARGS__)
#define LOG2_FATAL(   xFormat, ...  ) \
    g_pLog->Message(CMM::CLog::LV_FATAL, __FUNCTION__, " Line(%d) " xFormat,  __LINE__, ##__VA_ARGS__)

#define LOG2_ERROR(   xFormat, ...  ) \
    g_pLog->Message(CMM::CLog::LV_ERROR, __FUNCTION__, " Line(%d) " xFormat,  __LINE__, ##__VA_ARGS__)

#define LOG2_WARN(   xFormat, ...  ) \
    g_pLog->Message(CMM::CLog::LV_WARN, __FUNCTION__, " Line(%d) " xFormat,  __LINE__, ##__VA_ARGS__)

#define LOG2_DEBUG(   xFormat, ...  ) \
    g_pLog->Message(CMM::CLog::LV_DEBUG, __FUNCTION__, " Line(%d) " xFormat,  __LINE__, ##__VA_ARGS__)

#define LOG2_INFO(   xFormat, ...  ) \
    g_pLog->Message(CMM::CLog::LV_INFO, __FUNCTION__, " Line(%d) " xFormat,  __LINE__, ##__VA_ARGS__)


#ifdef ENABLE_LOG_STEP
#define LOG_STEP( xPrefix,xFormat, ...  ) \
    do{ if( g_pLog) g_pLog->Message(CMM::CLog::LV_DEBUG, xPrefix, " (%s %d) " xFormat, __FUNCTION__, __LINE__, ##__VA_ARGS__);} while(0)
#endif





#endif




#endif
