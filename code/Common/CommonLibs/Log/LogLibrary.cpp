#include "LogLibrary.h"

#include <time.h>
#include <math.h>
#include <assert.h>

#ifdef WINCE
#include <windows.h>
#include <stdarg.h>
#elif _WIN32
#include <direct.h>
#include <io.h>
#include <windows.h>
#elif _LINUX
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/vfs.h>    // or <sys/statfs.h>
#endif

#ifdef WINCE
#define ACCESS _access
#define MKDIR(a) _mkdir((a),NULL)
static int _mkdir(char* path,LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	WCHAR wszDirectory[MAX_PATH];
	int nResult = MultiByteToWideChar(
		CP_ACP,    
		MB_PRECOMPOSED,
		path,
		strlen(path)+1,
		wszDirectory,
		sizeof(wszDirectory)/sizeof(wszDirectory[0]));
	if(0 == nResult)
	{
		return nResult;
	}
	return CreateDirectory(wszDirectory,lpSecurityAttributes) ? 0 : -1 ;
}
#define vsnprintf _vsnprintf

static int _access(const char* czPathName,int flag)
{

	WCHAR wszDirectory[MAX_PATH];
	int nResult = MultiByteToWideChar(
		CP_ACP,    
		MB_PRECOMPOSED,
		czPathName,
		strlen(czPathName)+1,
		wszDirectory,
		sizeof(wszDirectory)/sizeof(wszDirectory[0]));
	if(0 == nResult)
	{
		return false;
	}
	return GetFileAttributes(wszDirectory)==FILE_ATTRIBUTE_DIRECTORY ? 0 : -1;
}
#elif _WIN32
#define ACCESS _access
#define MKDIR(a) _mkdir((a))
#elif _LINUX
#define ACCESS access
#define MKDIR(a) mkdir((a),0755)
#endif

#ifdef _LINUX
int _itoa(int val, char* buf, int _radix)
{
	// _radix not used, set radix to 10
	const unsigned int radix = 10;
	char* p;
	unsigned int a; //every digit
	int len;
	char* b; //start of the digit char
	char temp;
	unsigned int u;
	p = buf;
	if (val < 0)
	{
		*p++ = '-';
		val = 0 - val;
	}
	u = (unsigned int)val;
	b = p;
	do
	{
		a = u % radix;
		u /= radix;
		*p++ = a + '0';
	} while (u > 0);
	len = (int)(p - buf);
	*p-- = 0;
	//swap
	do
	{
		temp = *p;
		*p = *b;
		*b = temp;
		--p;
		++b;
	} while (b < p);
	return len;
}
#endif

#define MAX_LOGMSG_SIZE 2048

CLogLibrary::CLogLibrary(void)
{
	m_pfLog = NULL; 
	m_pCaller = NULL;
	m_pfnDealCB = NULL;
	m_uiLogSize = LOG_MAX_SIZE;//日志默认大小
	m_uiCurSize = 0;
	for(int i = 0;i<LOG_MEDIUM_NUM;i++)
	{
		m_stLogWay[i].uiMedium = (INT)(pow(2.0,i));
		m_stLogWay[i].bFlag = FALSE;
		m_stLogWay[i].uiLevel = LINFO;
	}
	m_uiWay = WREWIND;// 文件记录方式
 
	memset(m_strDir,0x0,LOG_DIR_NAME_NUM);//日志路径
	memset(m_strDocu,0x0,LOG_DOCUMENT_NAME_NUM);//当前日志所在文件夹
	memset(m_strFileName,0x0,LOG_FILE_NAME_NUM);//日志文件名
	memset(m_strFileEx,0x0,LOG_EX_NAME_NUM);	 //日志文件扩展名
	memset(m_strWholeName,0x0,1024);	 //日志文件扩展名
	m_pGSMutex = new CGSMutex;
	bDiskReady = TRUE;
}

CLogLibrary::~CLogLibrary(void)
{


	if (m_pfLog != NULL)
	{
		fclose(m_pfLog);
		m_pfLog = NULL;
	}

	if (m_pGSMutex != NULL)
	{
		delete m_pGSMutex;
		m_pGSMutex = NULL;
	}

}
BOOL CLogLibrary::CountDiskFreeSpace(const char *pDir,UINT64 &uiTotalFreeSize)// 获取磁盘可用剩余空间
{
	
	BOOL bRetCode = FALSE;
#ifdef WINCE
	ULARGE_INTEGER TotalAvailable;

	WCHAR wszDirectory[MAX_PATH];
	int nResult = MultiByteToWideChar(
		CP_ACP,    
		MB_PRECOMPOSED,
		pDir,
		strlen(pDir)+1,
		wszDirectory,
		sizeof(wszDirectory)/sizeof(wszDirectory[0]));
	if(0 == nResult)
	{
		return nResult;
	}
	
	bRetCode = ::GetDiskFreeSpaceEx(wszDirectory, &TotalAvailable, NULL, NULL);	
	uiTotalFreeSize = TotalAvailable.QuadPart;
#elif _WIN32
	ULARGE_INTEGER TotalAvailable;
	string strDir(pDir);
	bRetCode = ::GetDiskFreeSpaceExA(strDir.c_str(), &TotalAvailable, NULL, NULL);	
	uiTotalFreeSize = TotalAvailable.QuadPart;
#else
	struct statfs stStatfs;
	bRetCode = ((statfs("./", &stStatfs) == 0) ? TRUE : FALSE);    // statfs64()
	uiTotalFreeSize = stStatfs.f_bavail;
#endif

	return bRetCode;

}
INT CLogLibrary::CreatLogDir(char *pszDir)
{

	INT32 i = 0;
	INT32 iRet;
	INT32 iLen = strlen(pszDir);
	//在末尾加/
	if (pszDir[iLen - 1] != '\\' && pszDir[iLen - 1] != '/')
	{
		pszDir[iLen] = '/';
		pszDir[iLen + 1] = '\0';
	}

	// 创建目录
	for (i = 0;i < iLen;i ++)
	{
		if (pszDir[i] == '\\' || pszDir[i] == '/')
		{	
			if (i == 0)//防止linux下/home/a/b创建失败
			{
				continue;
			}
			pszDir[i] = '\0';

			//如果不存在,创建
			iRet = ACCESS(pszDir,0);
			if (iRet != 0)
			{
				iRet = MKDIR(pszDir);
				if (iRet != 0)
				{
					return -1;
				}	
			}
			//支持linux,将所有\换成/
			pszDir[i] = '/';
		}
	}

	return 0;
}

INT CLogLibrary::GenerateLogDir(const char *pszDir)
{
	//生成路径信息
	strcpy(m_strDir,pszDir);//拷贝路径字符串

	if (m_strDir[strlen(m_strDir) -1] != '/' && m_strDir[strlen(m_strDir) -1] != '\\')//目录后加'/'
	{
		strcat(m_strDir,"/");
	}
	
	// 创建路径
	if (CreatLogDir(m_strDir) != 0)
	{
		return ERROR_LOGLIBRARY_MKDIR_FAIl;
	}
	
    //生成日志文件夹
#ifdef WINCE
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);
	sprintf(m_strWholeName,"log%d-%d-%d",sysTime.wYear,sysTime.wMonth,sysTime.wDay);
#else //如果不是wince系统
	time_t tt;
	struct tm *ttime; 
	
	
	time(&tt);
	ttime = localtime(&tt);

	strftime(m_strWholeName,LOG_DOCUMENT_NAME_NUM,"log%Y-%m-%d",ttime); 
#endif	
	if (strcmp(m_strWholeName,m_strDocu) != 0)//文件夹不存在,创建该文件夹
	{
		strcpy(m_strDocu,m_strWholeName);//创建文件夹
		strcpy(m_strWholeName,m_strDir);//拷贝路径
		strcat(m_strWholeName,m_strDocu);//拷贝日志文件
		strcat(m_strWholeName,"/");
		if (ACCESS(m_strWholeName,0) == -1)//判断此文件是否存在,如果不存在,创建文件夹
		{
			if (MKDIR(m_strWholeName) != 0)
			{
				return ERROR_LOGLIBRARY_MKDIR_FAIl;
			}	
		}
		
	}
	
	//生成文件名
	
	strcpy(m_strFileName,m_strDocu);//加时间
	strcat(m_strFileName,"_");//加下划线
#ifdef WINCE
	sprintf(m_strWholeName,"log%d-%d-%d",sysTime.wHour,sysTime.wMinute,sysTime.wSecond);
#else
	strftime(m_strWholeName,LOG_DOCUMENT_NAME_NUM,"%H-%M-%S",ttime);
#endif
	strcat(m_strFileName,m_strWholeName);//加时间戳

	if (strlen(m_strFileEx) != 0)//加扩展
	{
		strcat(m_strFileName,"_");
		strcat(m_strFileName,m_strFileEx);
	}

	strcat(m_strFileName,".txt");
	
	//生成全路径
	strcpy(m_strWholeName,m_strDir);
	strcat(m_strWholeName,m_strDocu);
	strcat(m_strWholeName,"/");
	strcat(m_strWholeName,m_strFileName);

	if (m_pfLog != NULL)
	{
		fclose(m_pfLog);
		m_pfLog = NULL;
	}

	m_pfLog = fopen(m_strWholeName,"w+");
	assert(m_pfLog);
	if (m_pfLog == NULL)
	{
		return ERROR_LOGLIBRARY_FILE_NOT_OPEN;
	}

	//	日志当前大小为0
	m_uiCurSize = 0;

	return ERROR_LOGLIBRARY_OPER_SUCCESS;
}

INT CLogLibrary:: WriteToFile(const char *pszMsg)
{

	INT32 iRet = 0;
	// 如果文件句柄无效，则创建新文件
	if (m_pfLog == NULL)
	{
		iRet = GenerateLogDir(m_strDir);
		if (iRet != ERROR_LOGLIBRARY_OPER_SUCCESS)
		{
			return iRet;
		}
	}
	 
	// 如果磁盘空间不够，就进行检测，如果检测到有空间，就继续写日志
	if (!bDiskReady)
	{
		UINT64 uiFreeSize = 0;
		BOOL bFlag = FALSE;
		bFlag = CountDiskFreeSpace(m_strDir,uiFreeSize); // 计算磁盘剩余空间
		if ((!bFlag)||(uiFreeSize < m_uiLogSize - m_uiCurSize))
		{
			assert(bFlag);// 检测失败
			bDiskReady = FALSE;
			return ERROR_LOGLIBRARY_DISK_ERROR;
		}

		bDiskReady = TRUE;
	}
		
	//如果文件大小超过设置大小，则按照指定方式新建文件或者回卷（文件从头写）
	INT32 iMsgLen = strlen(pszMsg);
	if (m_uiCurSize + iMsgLen > m_uiLogSize)
	{
		if (m_pfLog != NULL)
		{
			fclose(m_pfLog);
			m_pfLog = NULL;
		}
		
		// 当前文件大小清0
		m_uiCurSize = 0;

		switch(m_uiWay)
		{
		case WREWIND://回卷
			{
				m_pfLog = fopen(m_strWholeName,"w+");
				if (m_pfLog == NULL)
				{
					return ERROR_LOGLIBRARY_FILE_NOT_OPEN;
				}
				break;
			}
		case WCREATE:// 新建
		default:
			{
				iRet = GenerateLogDir(m_strDir);
				if (iRet !=ERROR_LOGLIBRARY_OPER_SUCCESS)
				{	
					m_pfLog = NULL;
					return iRet;
				}
			}
		}

	}
	// 写文件
	INT32 iCount = 0;	
	iCount = fwrite(pszMsg,1,iMsgLen,m_pfLog);

  //assert(iCount == (size_t)iMsgLen);

	// 如果fwrite失败，则放弃本次写入，将磁盘标志置为FALSE
	if (iCount != (size_t)iMsgLen)
	{
		bDiskReady = FALSE;
		return ERROR_LOGLIBRARY_DISK_ERROR;
	}

	// 如果fflush失败，则放弃本次写入，将磁盘标志置为FALSE
	iRet = fflush(m_pfLog);
  //assert(iRet == 0);
	if (iRet != 0)
	{
		bDiskReady = FALSE;
		return ERROR_LOGLIBRARY_DISK_ERROR;
	}

	m_uiCurSize += iCount;
	return 0;

}

INT  CLogLibrary::SetLogDir(const char *pszDir,const char *pszExtName)
{
    INT iRet;
	m_pGSMutex->Lock();
	if (pszExtName != NULL)
	{
		if (strlen(pszExtName) > LOG_EX_NAME_NUM - 1)
		{
			m_pGSMutex->Unlock();
			return ERROR_LOGLIBRARY_STRING_ILLEGAL;
		}
		strcpy(m_strFileEx,pszExtName);
	}
	iRet = GenerateLogDir(pszDir);
	m_pGSMutex->Unlock();
	
	return iRet;
}

INT  CLogLibrary::SetLogSize(UINT uiWay,UINT uiSize)
{
	if(uiSize < 0)//文件大小不对
	{
		return ERROR_LOGLIBRARY_FILESIZE_ILLEGAL;
	}

	if (uiWay != WREWIND && uiWay != WCREATE)//策略不对
	{
		return ERROR_LOGLIBRARY_FILEWAY_ILLEGAL;
	}
	m_pGSMutex->Lock();
	if (uiSize == 0)
	{
		m_uiLogSize = LOG_MAX_SIZE;
	}
	else
	{
		m_uiLogSize = uiSize * 1024 *1024;
	}

	m_uiWay = uiWay;
	m_pGSMutex->Unlock();
	
	return ERROR_LOGLIBRARY_OPER_SUCCESS;
}

INT CLogLibrary::SetLogLevel(UINT uiMedium,UINT uiLevel)//可同时支持设置多中介质，并设置多个等级,如果某对应等级设置0，则取消该输出方式
{  
	//媒质不合法
	if (uiMedium > pow(2.0,LOG_MEDIUM_NUM )-1|| uiMedium <= 0x0)
	{
		return ERROR_LOGLIBRARY_LOGLEVEL_ILLEGAL;
	}

	//等级不合法
	if (uiLevel > pow(2.0,LOG_LEVEL_NUM )-1)
	{
		return ERROR_LOGLIBRARY_LOGMEDIUM_ILLEGAL;
	}

	m_pGSMutex->Lock();

	//设置对应输出媒质及日子等级
	for(int i = 0;i < LOG_MEDIUM_NUM;i++)
	{
		if (((uiMedium>>i) & 0x0001) == 1)
		{
			if (uiLevel == 0)
			{
				m_stLogWay[i].bFlag = FALSE;
			}
			else
			{
				m_stLogWay[i].bFlag = TRUE;
			}
			m_stLogWay[i].uiLevel = uiLevel;

		}
	}
	m_pGSMutex->Unlock();
	
	return ERROR_LOGLIBRARY_OPER_SUCCESS;
}

void CLogLibrary::SetLogCB(void *pCaller,void ( *pfnDealCB)(void *pCaller,const char * pszLevel,const char *pszPrefix,const char *pszMsg))
{
	m_pGSMutex->Lock();
	m_pCaller = pCaller;
	m_pfnDealCB = pfnDealCB;
	m_pGSMutex->Unlock();
}
string CLogLibrary::GetTimeStr()
{
	stringstream	streamTime;
	StruSysTime stTime;
	DoGetLocalTime(&stTime);

	//年
	streamTime<<stTime.wYear<<"-";
	//月
	if (stTime.wMonth < 10)
	{
		streamTime<<"0";
	}
	streamTime<<stTime.wMonth<<"-";

	//日
	if (stTime.wDay < 10)
	{
		streamTime<<"0";
	}
	streamTime<<stTime.wDay<<"-";
	
	//时
	if (stTime.wHour < 10)
	{
		streamTime<<"0";
	}
	streamTime<<stTime.wHour<<"-";

	//分
	if (stTime.wMinute < 10)
	{
		streamTime<<"0";
	}
	streamTime<<stTime.wMinute<<"-";

	//秒
	if (stTime.wSecond < 10)
	{
		streamTime<<"0";
	}
	streamTime<<stTime.wSecond<<"-";

	//毫秒
	if (stTime.wMilliseconds < 10)
	{
		streamTime<<"00";
	}
	else if (stTime.wMilliseconds < 100)
	{
		streamTime<<"0";
	}
	streamTime<<stTime.wMilliseconds<<"  ";

	string str = streamTime.str();
	
	return str;
}
/*************************************************
Function:   Log
DateTime:   2010-5-21 11:46:07   
Description: 写日志
Input:    uiLevel		日志文件的输出等级，其值及级别关系 LEMERG>LFATAL>LALERT>LCRIT>LERROR>LWARN
>LNOTICE>LINFO>LDEBUG>LNOTSET
pszPrefix	日志前缀，为NULL时，为默认前缀no，其长度不应超过64字节
pszFormat	日志格式串，与printf中格式串用法相同
...			格式串中对应值
Output:   无
Return:   ERROR_LOGLIBRARY_OPER_SUCCESS		操作成功
ERROR_LOGLIBRARY_LOGLEVEL_ILLEGAL	日志等级非法
Note:      
*************************************************/
INT  CLogLibrary::Log(UINT uiLevel,char *pszPrefix,char  *pszFormat,...)
{
	//日志格式：时间+等级+前缀+内容（支持变长）
	
	
	/*char	strLog[MAX_LOGMSG_SIZE] = {0x0};
	char	strContent[MAX_LOGMSG_SIZE] = {0x0};*/
	char	strLevel[16] = {0x0};
	char	strPrefix[64] = {0x0};
	int iRet = -1;

	/*对等级分析，看是否是合法等级*/
	int i = 0;
	for (i=0;i<LOG_LEVEL_NUM;i++)
	{
		if ((uiLevel>>i) == 1)
		{
			break;
		}
	}
	if (i >= LOG_LEVEL_NUM)
	{
		return ERROR_LOGLIBRARY_LOGLEVEL_ILLEGAL;
	}

	/*等级串*/
	switch(uiLevel)
	{
	case LEMERG:
		strcpy(strLevel,"LEMERG");
		break;
	case LFATAL:
		strcpy(strLevel,"LFATAL");
		break;
	case LALERT:
		strcpy(strLevel,"LALERT");
		break;
	case LCRIT:
		strcpy(strLevel,"LCRIT");
		break;
	case LERROR:
		strcpy(strLevel,"LERROR");
		break;
	case LWARN:
		strcpy(strLevel,"LWARN");
		break;
	case LNOTICE:
		strcpy(strLevel,"LNOTICE");
		break;
	case LINFO:
		strcpy(strLevel,"LINFO");
		break;
	case LDEBUG:
		strcpy(strLevel,"LDEBUG");
		break;
	case LNOTSET:
		strcpy(strLevel,"LNOTSET");
		break;
	default:
		return ERROR_LOGLIBRARY_LOGLEVEL_ILLEGAL;
	}
	
	/*前缀串*/
	if (pszPrefix == NULL || strlen(pszPrefix) > 63)//无前缀
	{
		strcpy(strPrefix,"no");
	}

	else
	{
		strcpy(strPrefix,pszPrefix);
	}
	

	char			szlogContent[MAX_LOGMSG_SIZE] = {0x0};
	/*信息串（处理变长串）*/
	va_list  ap;
	va_start(ap, pszFormat);	
	iRet = vsnprintf(szlogContent,MAX_LOGMSG_SIZE - 1,pszFormat,ap);
	va_end(ap);

	// vsnprintf可能会执行错误，比如字符串超过strContent长度
	if (iRet == -1)
	{
		return ERROR_LOGLIBRARY_MSGSIZE_ILLEGAL;
	}
	m_pGSMutex->Lock();

	if (m_pfnDealCB != NULL)
	{
		m_pfnDealCB(m_pCaller,strLevel,strPrefix,szlogContent);
	}

	/*时间串*/
	string strTime = GetTimeStr();
	string	strContent = string(szlogContent);
	stringstream	streamlog;
	streamlog <<strTime;
	streamlog <<string(strLevel)<<"  ";
	streamlog <<string(strPrefix)<<"  ";
	streamlog <<strContent;

	//判断是否有换行符
	streamlog<<"\n";
	
	string strlog = streamlog.str();

	// 检测日志文件是否过长(过长是否不打印)
	if (strlog.size() > MAX_LOGMSG_SIZE - 2)
	{
		m_pGSMutex->Unlock();
		return ERROR_LOGLIBRARY_STRING_ILLEGAL;
	}

	// 按设定输出方式输出日志
	for (i=0;i<LOG_MEDIUM_NUM;i++)
	{
		if (m_stLogWay[i].bFlag == TRUE )//该输出方式是否有效
		{
			if ((m_stLogWay[i].uiLevel & uiLevel) != 0)//是否设定等级
			{
				if (m_stLogWay[i].uiMedium == MCONSOLE)//输出到控制台
				{
					printf("%s",strlog.c_str());
				}

				if (m_stLogWay[i].uiMedium == MFILE)//输出到文件
				{
					char chTemp[LOG_DOCUMENT_NAME_NUM] = {0};
#ifdef WINCE
					SYSTEMTIME sysTime;
					GetSystemTime(&sysTime);
					sprintf(chTemp,"log%d-%d-%d",sysTime.wYear,sysTime.wMonth,sysTime.wDay);
#else
					time_t tt;
					struct tm *ttime; 
					time(&tt);
					ttime = localtime(&tt);
					strftime(chTemp,LOG_DOCUMENT_NAME_NUM,"log%Y-%m-%d",ttime); 
#endif

					if (strcmp(chTemp,m_strDocu) != 0)//新的一天,创建新的目录
					{
						if (GenerateLogDir(m_strDir) !=ERROR_LOGLIBRARY_OPER_SUCCESS)
						{
							m_pGSMutex->Unlock();
							return -1;
						}
					}
					WriteToFile(strlog.c_str());
				}
			}
		}
	}
	m_pGSMutex->Unlock();
	
	return ERROR_LOGLIBRARY_OPER_SUCCESS;
}

ILogLibrary *GetLogInstance()
{
	ILogLibrary *pLogLibrary = new CLogLibrary;

	if (pLogLibrary != NULL)
	{
		return pLogLibrary;
	}
	
	return NULL;
}
void ClearLogInstance(ILogLibrary *pLogLibrary)
{
	if (pLogLibrary != NULL)
	{
		delete pLogLibrary;
		pLogLibrary = NULL;
	}
}
