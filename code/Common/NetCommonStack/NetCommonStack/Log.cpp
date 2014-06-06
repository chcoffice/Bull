#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <stdarg.h>
#endif

#include "Log.h"
using namespace CMM;



static const char *s_czFatal = "FATAL";
static const char *s_czError = "ERROR";
static const char *s_czWarn = "WARN";
static const char *s_czDebug = "DEBUG";
static const char *s_czInfo = "INFO";
static const char *s_czUnknow = "UNKNOWN";


static BOOL StringVFormat(std::string &strOString,const char *czFormat, va_list ap )
{
#define MAX_STRING_LENGHT (1024L<<10)
    char sBuffer[256];
    va_list apStart;
    char *pBuffer;
    int n, size = 256; 


    strOString.clear();

    apStart = ap;       
    pBuffer = sBuffer;

    while (pBuffer) {           
        ap = apStart;
        n = vsnprintf (pBuffer, size, czFormat, ap);          
        if (n > -1 && n < size )
        {
            //成功格式化
            //pBuffer[n] = '\0';
            strOString =  pBuffer;
            if( pBuffer!=sBuffer )
            {          
                ::free(pBuffer);
            }
            return TRUE;
        }
        if( pBuffer!=sBuffer )
        {          
            ::free(pBuffer);
        }
        size *= 2;
        if( size> MAX_STRING_LENGHT )
        {
            return FALSE;
        }       
        pBuffer = (char*) malloc(size);       
    }
    if(pBuffer && pBuffer!=sBuffer )
    {
        ::free(pBuffer);
    }
    return FALSE;
}





static INLINE const char *GetLevelName( CLog::EnumLogLevel eLevel )
{
    switch(eLevel )
    {
    case CLog::LV_ERROR :
        return s_czError;
        break;
    case CLog::LV_DEBUG :
        return s_czDebug;
        break;
    case CLog::LV_INFO :
        return s_czInfo;
        break;
    case CLog::LV_WARN :
        return s_czWarn;
        break;
    case CLog::LV_FATAL:
        return s_czFatal;
        break;
    default :
        break;
    }
    return s_czUnknow;
}

CLog *CLog::s_pGlobalLog = NULL;

CLog::CLog(ILogLibrary *pILog)
:m_pILog(pILog)
,m_iLVDirConsole(LV_ALL)
,m_iLVDirFile( (LV_FATAL|LV_ERROR|LV_WARN|LV_DEBUG)) 
{
      if( s_pGlobalLog==NULL)
      {
          s_pGlobalLog = this;
      }
      m_strLogPath = "CMMLog";
}

CLog::~CLog(void)
{

    if( s_pGlobalLog==this)
    {
        s_pGlobalLog = NULL;
    }

    if( m_pILog )
    {
        ClearLogInstance(m_pILog);
    }
    m_pILog = NULL;   
}

void CLog::SetLogPath(const char *czPathName)
{
    if(m_pILog )
    {
        return;
    }
ILogLibrary *p;
    p = GetLogInstance();
    if( p )
    {
        if( czPathName )
        {
            m_strLogPath = czPathName;
        }
#ifdef _WIN32
        _mkdir( m_strLogPath.c_str() );
#else
        mkdir( m_strLogPath.c_str(), 777 );
#endif
        p->SetLogDir(  m_strLogPath.c_str() ,"Cmm"  );
        p->SetLogLevel(MCONSOLE, m_iLVDirConsole);
        p->SetLogLevel(MFILE, m_iLVDirFile);
        m_pILog = p;
    }
    s_pGlobalLog = this;
}


void CLog::SetLogLevel(EnumLogDirection eDirect, EnumLogLevel eLevel)
{
   if( m_pILog )
   {
       m_pILog->SetLogLevel((UINT)eDirect, (UINT)eLevel);
   }
   if( eDirect&DIR_CONSOLE )
   {
      m_iLVDirConsole = (UINT)eLevel;
   }
   if( eDirect&DIR_FILE )
   {
        m_iLVDirFile = (UINT)eLevel;
   }

}


void CLog::Message(EnumLogLevel eLevel,const char *czPrefix,const char  *czFormat,...)
{
std::string strTemp;
BOOL bRet;
    va_list ap;
    va_start(ap, czFormat);
    bRet = StringVFormat( strTemp, czFormat, ap);
    va_end(ap);
    if( !bRet)     
    {
        assert(0);
        return;
    }
    if( m_pILog )
    {
        m_pILog->Log((UINT)eLevel,(char*)  czPrefix,(char*) strTemp.c_str() );
    }
    else
    {
       if( eLevel&m_iLVDirConsole )
       {
           printf("%s %s %s", GetLevelName(eLevel), czPrefix,  strTemp.c_str() );
       }
    }
}

void CLog::SetInitFile( const char *czFilename )
{
    CConfigFile stConfigInfo;	
    stConfigInfo.LoadFile((char*)czFilename );

    //日志配置
    UINT32 iLevel;
    INT iRet;



	INT iDebug = 0;
#if defined(_DEBUG) || defined(DEBUG)
    iDebug = 1;
#endif

	iLevel = 0;

    iRet = stConfigInfo.GetProperty("DebugConsole", "FATAL",1);
    if(iRet )
    {
        iLevel |= LFATAL;
    }
    iRet = stConfigInfo.GetProperty("DebugConsole", "ERROR",1);
    if(iRet )
    {
        iLevel |= LERROR;
    }
    iRet = stConfigInfo.GetProperty("DebugConsole", "WARN",0|iDebug);
    if(iRet )
    {
        iLevel |= LWARN;
    }
    iRet = stConfigInfo.GetProperty("DebugConsole", "DEBUG",0);
    if(iRet )
    {
        iLevel |= LDEBUG;
    }
    iRet = stConfigInfo.GetProperty("DebugConsole", "INFO",0);
    if(iRet )
    {
        iLevel |= LINFO;
    }
    
    SetLogLevel(DIR_CONSOLE,(EnumLogLevel) iLevel);


    iLevel = 0 ;
    iRet = stConfigInfo.GetProperty("DebugFile", "FATAL",1);
    if(iRet )
    {
        iLevel |= LFATAL;
    }

    iRet = stConfigInfo.GetProperty("DebugFile", "ERROR",1);
    if(iRet )
    {
        iLevel |= LERROR;
    }

    iRet = stConfigInfo.GetProperty("DebugFile", "WARN",0|iDebug);
    if(iRet )
    {
        iLevel |= LWARN;
    }

    iRet = stConfigInfo.GetProperty("DebugFile", "DEBUG",0|iDebug);
    if(iRet )
    {
        iLevel |= LDEBUG;
    }

    iRet = stConfigInfo.GetProperty("DebugFile", "INFO",0);
    if(iRet )
    {
        iLevel |= LINFO;
    } 
    SetLogLevel(DIR_FILE,(EnumLogLevel) iLevel);
}

