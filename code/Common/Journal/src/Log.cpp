#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <stdarg.h>
#endif

#include "Log.h"

using namespace JOU;



static const char *s_czFatal = "FATAL";
static const char *s_czError = "ERROR";
static const char *s_czWarn = "WARN";
static const char *s_czDebug = "DEBUG";
static const char *s_czInfo = "INFO";
static const char *s_czUnknow = "UNKNOWN";



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
:CJouObj()
,m_pILog(pILog)
,m_iLVDirConsole(LV_ALL)
,m_iLVDirFile( (LV_FATAL|LV_ERROR|LV_WARN|LV_DEBUG)) 
{
      if( s_pGlobalLog==NULL)
      {
          s_pGlobalLog = this;
      }
      m_strLogPath = "JouLog";
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
        p->SetLogDir(  m_strLogPath.c_str() ,"gsp"  );
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
	if( !( eLevel&m_iLVDirConsole ) && !( eLevel&m_iLVDirFile ) )
	{
		return;
	}
CGSString strTemp;
BOOL bRet;
#ifdef _WIN32
const char *czTempPrefix = czPrefix;
	czTempPrefix = strstr(czPrefix, "\\src\\" );
	if( czTempPrefix )
	{
		czPrefix = czTempPrefix+1;
	}
#endif
    va_list ap;
    va_start(ap, czFormat);
    bRet = GSStrUtil::VFormat( strTemp, czFormat, ap);
    va_end(ap);
    GS_ASSERT_RET(bRet);
    if( m_pILog )
    {
        m_pILog->Log((UINT)eLevel,(char*)  czPrefix,(char*) strTemp.c_str() );
    }
    else
    {
       if( eLevel&m_iLVDirConsole )
       {
           MY_PRINTF("%s %s %s", GetLevelName(eLevel), czPrefix,  strTemp.c_str() );
       }
    }
}


#ifdef ENABLE_LOG_STEP
CFunctionDebug::CFunctionDebug(CLog *pLog, const CGSString &strFunc, const char  *czFormat,...)
{
	m_strInfo.clear();
	va_list ap;
	va_start(ap, czFormat);
	GSStrUtil::VFormat( m_strInfo, czFormat, ap);
	va_end(ap);
	if( m_strInfo.empty() )
	{
		m_strInfo = "";
	}
	m_pLog = pLog;	
	m_strFunc = strFunc;
	if( m_pLog )
	{
		m_pLog->Message( CLog::LV_DEBUG, "FUNC", 
			">>>%lld<<< **%s** Enter %s.\n",
			(long long) DoGetTickCount(), 
			m_strFunc.c_str(), 
			m_strInfo.c_str());
	}
	else
	{
		MY_PRINTF("DEBUG FUNC >>>%lld<<< **%s** Enter %s.\n",
			(long long) DoGetTickCount(), 
			m_strFunc.c_str(), 
			m_strInfo.c_str());
	}
}

#endif //ENABLE_LOG_STEP
