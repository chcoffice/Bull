
#pragma  warning(disable:4996)//去掉的run-time library routines编译警告

#include <windows.h>
#include <Dbghelp.h>
#include  <io.h>
#include  <time.h>
#include <direct.h>
#include<string>

#include"mdump.h"
#define	R_A_S	200 * 1024 * 1024	//保留地址空间。


//保留虚拟内存管理，创建这个类是为了退出时能通过析构函数自动释放内存
class CReserveVirtualMemManage
{
public:
	CReserveVirtualMemManage():m_pAddrsSpace(NULL)
	{};

	void GetAddrsSpace(int dwSize)
	{
		ReleaseAddrsSpace();

		m_pAddrsSpace = VirtualAlloc(0, dwSize, MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	};

	void ReleaseAddrsSpace()
	{
		if(m_pAddrsSpace)
		{
			VirtualFree(m_pAddrsSpace, 0, MEM_RELEASE);
			m_pAddrsSpace = NULL;
		}
	}

	~CReserveVirtualMemManage(){ ReleaseAddrsSpace(); };

private:
	 void*  m_pAddrsSpace;

};


static CReserveVirtualMemManage g_ReserveVirtualMemManage;
static bool g_IsDumpWithFullMemory = true;


typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
										 CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
										 CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
										 CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
										 );



static void MsgError( const char* sDst )
{
	LPVOID lpMsgBuf;
	DWORD errNo = GetLastError();

	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errNo,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
		);

	char errMsg[1024] = {0};
	sprintf_s(errMsg,"%s (%s)",sDst,lpMsgBuf);

	::MessageBoxA( NULL, errMsg, "错误", MB_OK ); 
	LocalFree( lpMsgBuf );
}



//获取dump文件路径名
static bool GetDumpFilePath(std::string& strDumpPath)
{

	char szModPath[_MAX_PATH];

	if (!GetModuleFileNameA( NULL, szModPath, _MAX_PATH ))
		return false;


	std::string strModPath = szModPath;

	std::string strProgName = strModPath.substr(strModPath.rfind("\\")+1);

	strModPath =  strModPath.substr(0, strModPath.rfind("\\"));


	std::string sDumpPath = strModPath + "\\Dump";

	//如果不存在,创建dump目录
	if (_access(sDumpPath.c_str(),0) != 0)
	{
		if( _mkdir(sDumpPath.c_str())!=0)
			return false;
	}


	//文件名打上时间戳
	time_t   tval; 
	struct   tm   *now; 
	CHAR   buf[255] = {0};   
	tval   =   time(NULL);   
	now   =   localtime(&tval);
	sprintf_s(buf,   "\\%s(%4d%02d%02d %02d%02d%02d).dmp",   strProgName.c_str(),
		now->tm_year+1900,   now->tm_mon+1,   now->tm_mday,
		now->tm_hour,   now->tm_min,   now->tm_sec);  

	strDumpPath = sDumpPath + buf;

	return true;
}


//程序崩溃处理函数
static bool ApplicationCrashHandler(EXCEPTION_POINTERS *pException,bool IsDumpWithFullMemory)
{

	HMODULE hDll = NULL;
	char szModPath[_MAX_PATH];


	//获取程序所在路径
	std::string strModPath = "";

	//尝试加载指定版本 
	if (GetModuleFileNameA( NULL, szModPath, _MAX_PATH ))
	{

		strModPath = szModPath;
		strModPath =  strModPath.substr(0, strModPath.rfind("\\"));

		std::string strDbgHelpPath = strModPath + "\\DBGHELP.DLL";
		hDll = ::LoadLibraryA( strDbgHelpPath.c_str() );

	}

	if (hDll==NULL)
	{
		// 加载任意版本 
		hDll = ::LoadLibraryA( "DBGHELP.DLL" );
	}

	if(!hDll)//最终加载dll不成功
	{
		MsgError("程序由于遇到未知问题退出!加载DBGHELP.DLL失败!");
		return false;

	}


	if (hDll)
	{
		MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress( hDll, "MiniDumpWriteDump" );
		if (pDump)
		{


			std::string strDumpPath;
			if(!GetDumpFilePath(strDumpPath))
			{
				FreeLibrary(hDll);
				MsgError("程序由于遇到未知问题退出!获取文件名失败!");
				return false;

			}


			HANDLE hFile = ::CreateFileA(  strDumpPath.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL, NULL );

			

			if (hFile!=INVALID_HANDLE_VALUE) 
			{
				_MINIDUMP_EXCEPTION_INFORMATION ExInfo;
			
				ExInfo.ThreadId = ::GetCurrentThreadId();
				ExInfo.ExceptionPointers = pException;
				//ExInfo.ClientPointers = NULL;
				ExInfo.ClientPointers = true;


				PMINIDUMP_EXCEPTION_INFORMATION pExceptInfo = NULL;

				if(pException)
					pExceptInfo = &ExInfo;

				_MINIDUMP_TYPE eDumpType = MiniDumpWithFullMemory;
				if(IsDumpWithFullMemory)
					eDumpType = MiniDumpWithFullMemory;
				else
					eDumpType = MiniDumpNormal;



				// write the dump
				BOOL bOK = pDump( GetCurrentProcess(), GetCurrentProcessId(), hFile,  eDumpType, pExceptInfo, NULL, NULL );

				
				if (bOK)
				{

					::CloseHandle(hFile);
					::FreeLibrary(hDll);
					return true;
				}
				else
				{
					::CloseHandle(hFile);
					MsgError("程序由于遇到未知问题退出!保存dump文件失败!");
					::FreeLibrary(hDll);
					return false;
				}

 

			}
			else
			{
				MsgError("程序由于遇到未知问题退出!创建dump文件失败!");
				::FreeLibrary(hDll);
				return false;


			} 

		}
		::FreeLibrary(hDll);
	}

	return false;


}

static LPTOP_LEVEL_EXCEPTION_FILTER WINAPI SetUnhandledExceptionFilter2( __in LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter )
{
	//printf("enter SetUnhandledExceptionFilter....\n");
	return 0;
}

static bool DisableSetUnhandledExceptionFilter()
{

	HMODULE hKernel32 = LoadLibraryA("kernel32.dll");
	if (hKernel32 == NULL) 
	{
		MsgError("加载kernel32.dll相关函数地址失败!");
		return false;
	}

	void *pOrgEntry = GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");
	if(pOrgEntry == NULL) 
		return false;

	DWORD dwOldProtect = 0;
	SIZE_T jmpSize = 5;

	BOOL bProt = VirtualProtect(pOrgEntry, jmpSize, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	if (!bProt)
	{
		MsgError("调用VirtualProtect失败!");

		return false;
	}

	BYTE newJump[20];
	void *pNewFunc = &SetUnhandledExceptionFilter2;

	SIZE_T dwOrgEntryAddr = (SIZE_T) pOrgEntry;
	dwOrgEntryAddr += jmpSize; // add 5 for 5 op-codes for jmp rel32
	SIZE_T dwNewEntryAddr = (SIZE_T) pNewFunc;
	SIZE_T dwRelativeAddr = dwNewEntryAddr - dwOrgEntryAddr;
	// JMP rel32: Jump near, relative, displacement relative to next instruction.
	newJump[0] = 0xE9;  // JMP rel32
	memcpy(&newJump[1], &dwRelativeAddr, sizeof(pNewFunc));

	SIZE_T bytesWritten;
	BOOL bRet = WriteProcessMemory(GetCurrentProcess(), pOrgEntry, newJump, jmpSize, &bytesWritten);
	if (!bRet)
	{
		MsgError("调用WriteProcessMemory失败!");
		return false;
	}

	DWORD dwBuf;
	VirtualProtect(pOrgEntry, jmpSize, dwOldProtect, &dwBuf);
	return true;

}


static long DoMiniDump( void* pExceptionInfo,bool IsDumpWithFullMemory )
{
	
	g_ReserveVirtualMemManage.ReleaseAddrsSpace();


	if(pExceptionInfo)
	{
		ApplicationCrashHandler( (EXCEPTION_POINTERS*)pExceptionInfo,IsDumpWithFullMemory ); //此处无需再申请地址空间，程序将会退出。
	}
	else
	{
		ApplicationCrashHandler( NULL ,IsDumpWithFullMemory);
		g_ReserveVirtualMemManage.GetAddrsSpace(R_A_S); 
	}

	return EXCEPTION_EXECUTE_HANDLER;

}


static void purecall_handler(void)
{
	//printf("enter purecall_handler handle\n");
	DoMiniDump( NULL ,g_IsDumpWithFullMemory);
} 



static void invalidparameter_handle(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved)
{
	//printf("enter invalidparameter_handle handle\n");
	DoMiniDump( (void*)expression ,g_IsDumpWithFullMemory);
}


//手动产生dump文件
bool getDump(bool IsDumpWithFullMemory )
{

	DoMiniDump( NULL ,IsDumpWithFullMemory); 
	return true;


}


static LONG WINAPI my_exception_handle(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
	
	if(ExceptionInfo)
	{
		return DoMiniDump( (void*)ExceptionInfo ,g_IsDumpWithFullMemory);
	}
	else
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}
}



//程序崩溃注册函数
//IsPreventOtherExceptionHandling如果设置成true会屏蔽其他地方对SetUnhandledExceptionFilter的调用
void MiniDump(bool IsDumpWithFullMemory,
			  bool IsPreventOtherExceptionHandling)
{

	::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
	::SetUnhandledExceptionFilter( (LPTOP_LEVEL_EXCEPTION_FILTER)my_exception_handle );
	::_set_purecall_handler(purecall_handler);
	::_set_invalid_parameter_handler(invalidparameter_handle);

	g_IsDumpWithFullMemory = IsDumpWithFullMemory;

	//申请保留虚拟地址空间
	g_ReserveVirtualMemManage.GetAddrsSpace(R_A_S);

	if(IsPreventOtherExceptionHandling)
		DisableSetUnhandledExceptionFilter();

	return;
}

