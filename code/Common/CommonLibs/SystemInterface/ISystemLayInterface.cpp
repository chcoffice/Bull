// ISystemLayInterface.cpp : 定义控制台应用程序的入口点。
//



#include "ISystemLayInterface.h"

#ifdef _WIN32
#include <direct.h>
#include <Mmsystem.h>
#include <windows.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <stdio.h>


#ifdef WINCE
#pragma once
#include <Windows.h>

#elif _WIN32
#include <intrin.h>
#else

static CGSMutex s_csAtomicInter;

#endif


// 对iVal 进行递增操作， 返回增加后的值
 long AtomicInterInc( GSAtomicInter &iVal )
{
#ifdef _WIN32
		return InterlockedIncrement( &iVal );
#else
		CGSAutoMutex locker( &s_csAtomicInter);
		iVal++;
		return iVal;
		
#endif

}
// 对iVal 进行递减操作， 返回递减后的值
 long AtomicInterDec( GSAtomicInter &iVal )
{

#ifdef _WIN32
	return InterlockedDecrement( &iVal );
#else
	CGSAutoMutex locker( &s_csAtomicInter);
	iVal--;
	return iVal;

#endif
}
//比较iVal 释放 以iOldVal 相等， 如果相等，把 iVal 设定为 iNewVal, 并返回TRUE， 否则返回FALSE
 BOOL AtomicInterCompareExchange(GSAtomicInter &iVal, 
										   const long iOldVal, const long iNewVal)
{

#ifdef _WIN32
	return  (InterlockedCompareExchange( &iVal,iNewVal, iOldVal) == iOldVal ? TRUE :FALSE );
#else
	CGSAutoMutex locker( &s_csAtomicInter);
	if( iVal == iOldVal )
	{
		iVal = iNewVal;
		return TRUE;
	}
	return FALSE;
#endif
}


 long AtomicInterSet(GSAtomicInter &iVal, const long iNewVal )
{
#ifdef _WIN32
	return InterlockedExchange( &iVal, iNewVal);
#else
	CGSAutoMutex locker( &s_csAtomicInter);
	long i = iVal;
	iVal = iNewVal;
	return i;
#endif
}


 long AtomicInterAnd(GSAtomicInter &iVal, const long iFlag )
{
#ifdef _WIN32
	return _InterlockedAnd( &iVal, iFlag);
#else
	CGSAutoMutex locker( &s_csAtomicInter);
	long i = iVal;
	iVal &= iFlag;
	return i;
#endif
}

 long AtomicInterOr(GSAtomicInter &iVal, const long iFlag )
{
#ifdef _WIN32
	return _InterlockedOr( &iVal, iFlag);
#else
	CGSAutoMutex locker( &s_csAtomicInter);
	long i = iVal;
	iVal |= iFlag;
	return i;
#endif
}



#ifdef _WIN32 
# define DIR_DELIMITERS   '\\'
#else
# define DIR_DELIMITERS   '/'
#endif


CGSString GSGetApplicationPath(void)
{
#define GS_MAX_PATH 300
	char czBuffer[GS_MAX_PATH];
	std::string strAppPath = "./";
	bzero(czBuffer, sizeof(czBuffer)); 
#ifdef _WIN32
	// 获取执行程序文件路径
	::GetModuleFileNameA(NULL,  czBuffer, GS_MAX_PATH-1);
	strAppPath = (char*) czBuffer;    
#else
	int n;
	n = readlink("/proc/self/exe" , czBuffer , GS_MAX_PATH-1);    
	if( n > 0 )
	{            
		strAppPath = czBuffer;        
	} 
	czBuffer[ GS_MAX_PATH-1] = '\0';
#endif
	CGSString strRet = strAppPath.substr(0,strAppPath.rfind(DIR_DELIMITERS) );
	GSPathParser(strRet);
	return strRet;	// 不存在返回当前路径

}

void GSPathParser( CGSString &strPath)
{
	std::string strTemp;
	char chrDelimiter = DIR_DELIMITERS;
	INT iCnts = 0;
	INT iLen = strPath.length();
	int i = 0;
	for( ; i<iLen; i++)
	{   
		if( strPath[i]=='\\' || strPath[i]=='/' )
		{
			if( iCnts==0 )
			{
				strTemp += chrDelimiter;
			}
			iCnts++;
		}
		else
		{
			iCnts = 0;
			strTemp += strPath[i];
		}
	}

	if( i && strPath[i-1]!=chrDelimiter )
	{
		strTemp += chrDelimiter;
	}
	strPath = strTemp;
}


static BOOL _CreateDir( const char *czPath )
{
#ifdef WINCE
	return (_mkdir( czPath,NULL ) == 0) ;
#elif _WIN32
	return (_mkdir( czPath ) == 0) ;
#else
	return (mkdir( czPath, 777 ) == 0) ;
#endif
}

BOOL GSTestAndCreateDir( const char *czPath)
{
std::string strTemp = czPath;
std::string::size_type iFound;

	int i = strTemp.length();
	if( i>0 && (strTemp[i-1] == '\\' || strTemp[i-1] == '/' ) )
	{
		strTemp = strTemp.substr( 0, i-1 );
	}
	if( strTemp.empty() )
	{
		return FALSE;
	}
	iFound = strTemp.find_last_of("/\\"); 
#ifdef _WIN32
	struct _stat stBuf;	
	if( iFound == std::string::npos )
	{
		//root
		strTemp += "\\";
		if( 0 == ::_stat(strTemp.c_str() , &stBuf) )
		{
			return stBuf.st_mode&_S_IFDIR ? TRUE : FALSE;
		}	
		return FALSE;
	}

	if( 0 == ::_stat(strTemp.c_str() , &stBuf) )
	{
		return stBuf.st_mode&_S_IFDIR ? TRUE : FALSE;
	}	
#else

	if( iFound == std::string::npos )
	{
		//root
		return FALSE;
	}

	struct stat stBuf;
	if( 0== stat(czPath, &stBuf) )
	{
		return S_ISDIR(stBuf.st_mode) ? TRUE : FALSE;
	}
#endif

	iFound = strTemp.find_last_of("/\\"); 
	if( iFound != std::string::npos )
	{
		std::string strParentDir = strTemp.substr( 0, iFound );	
		if( GSTestAndCreateDir( strParentDir.c_str() ) )
		{
			return _CreateDir(strTemp.c_str() );
		}
	}
	return FALSE;
}














