
#include "ISystemLayInterface.h"

#ifdef WINCE

#elif _WIN32
#include <io.h>
#include <direct.h>
#include <sys/stat.h>
#elif _LINUX
#include <sys/types.h>
#include <dirent.h>
#include<sys/stat.h>
#endif


/******************************************************************************
功能说明：目录枚举
******************************************************************************/

//构造函数
CGSDir::CGSDir()
{
	m_GSDir=NULL;

#ifdef WINCE
	ZeroMemory(m_wczDirPath,MAX_PATH);
#endif

}

//析构函数
CGSDir::~CGSDir()
{

}


/********************************************************************************************
  Function		: OpenDir    
  DateTime		: 2010/6/10 10:42	
  Description	: 打开目录
  Input			: const char *czDirPath：目录路径
  Output		: NULL
  Return		: 成功返回TRUE，失败返回FALSE
  Note			:				// 备注
********************************************************************************************/
BOOL CGSDir::OpenDir(const char *czDirPath)
{
	BOOL bOpenDirRet=TRUE;
#ifdef WINCE 

#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))

#ifdef UNICODE

	int     nResult;

	nResult = MultiByteToWideChar(
		CP_ACP,    
		MB_PRECOMPOSED,
		czDirPath,
		strlen(czDirPath)+1,
		m_wczDirPath,
		ARRAYSIZE(m_wczDirPath));
	if(0 == nResult)
	{
		return false;
	}
#else
	hr = StringCchCopy(m_wczDirPath, ARRAYSIZE(wszDirectory), czDirPath);
	if(FAILED(hr))
	{
		return false;
	}
#endif

	if(GetFileAttributes(m_wczDirPath)==FILE_ATTRIBUTE_DIRECTORY)
	{
		bOpenDirRet=TRUE;
	}
	else
		bOpenDirRet=FALSE;
	
#elif _WIN32

	//改变当前目录
	if(_chdir(czDirPath))
	{
		bOpenDirRet=FALSE;
	};

#elif _LINUX

	//打开指定目录
	m_GSDir =opendir(czDirPath); 
	if(m_GSDir==NULL)
	{
		bOpenDirRet=FALSE;
	}

#endif

	return bOpenDirRet;
}

/********************************************************************************************
  Function		: CloseDir    
  DateTime		: 2010/6/10 10:44	
  Description	: 关闭目录
  Input			: NULL
  Output		: NULL
  Return		: NULL
  Note			:				// 备注
********************************************************************************************/
void CGSDir::CloseDir()
{
#ifdef _WIN32


#elif _LINUX

	if (m_GSDir!=NULL)
	{
		closedir((DIR *)m_GSDir);
	}

#endif

}

//读取目录列表
/********************************************************************************************
  Function		: ReadDir    
  DateTime		: 2010/6/10 10:44	
  Description	: 读取目录
  Input			: std::vector<StruGSFileInfo> &vectFileList：目录信息列表
  Output		: NULL
  Return		: NULL
  Note			:				// 备注
********************************************************************************************/
void CGSDir::ReadDir(std::vector<StruGSFileInfo> &vectFileList)
{

	StruGSFileInfo DirInfo;
	vectFileList.clear();
#ifndef WINCE
	struct stat	  FileBuf;
#endif
#ifdef WINCE

	HANDLE hDirectory=NULL;
	WIN32_FIND_DATAW wfd={0};
	hDirectory=FindFirstFile(m_wczDirPath,&wfd);

	if (hDirectory==INVALID_HANDLE_VALUE)
	{
		return;
	}
	do 
	{

		char tmp[MAX_PATH];
		WideCharToMultiByte(CP_ACP,0,wfd.cFileName,256,tmp,256,NULL,NULL);
		DirInfo.strFileName=tmp;

		if((wfd.dwFileAttributes) & FILE_ATTRIBUTE_DIRECTORY)
		{
			DirInfo.iFileType=GS_FILE_DIRECTORY;
		}
		//判断文件是否为普通文件
		else if ((wfd.dwFileAttributes) & FILE_ATTRIBUTE_NORMAL)
		{
			DirInfo.iFileType=GS_FILE_COMMON;
		}
		//判断文件是否为管道文件
// 		else if ((wfd.dwFileAttributes) & _S_IFIFO)
// 		{
// 			DirInfo.iFileType=GS_FILE_FIFO;
// 		}
		//文件为其他类型
		else 
		{
			DirInfo.iFileType=GS_FILE_OTHER;
		}	

		vectFileList.push_back(DirInfo);

	} while (FindNextFile(hDirectory,&wfd));

#elif _WIN32
	
	long handle; 
	struct _finddata_t filestruct;

	//得到当前目录的第一个子目录
	handle=_findfirst("*",&filestruct);
	
	//判断第一个目录获取是否成功，失败则返回
	if(handle==-1)
		{
			return;
		}
	
	//获取第一个目录的属性，存到FileBuf中
	if (stat(filestruct.name,&FileBuf)==0)
	{
		DirInfo.strFileName=filestruct.name;
		
		//判断文件是否为目录文件
		if((FileBuf.st_mode) & _S_IFDIR)
		{
			DirInfo.iFileType=GS_FILE_DIRECTORY;
		}
		//判断文件是否为普通文件
		else if ((FileBuf.st_mode) & _S_IFREG)
		{
			DirInfo.iFileType=GS_FILE_COMMON;
		}
		//判断文件是否为管道文件
		else if ((FileBuf.st_mode) & _S_IFIFO)
		{
			DirInfo.iFileType=GS_FILE_FIFO;
		}
		//文件为其他类型
		else 
		{
			DirInfo.iFileType=GS_FILE_OTHER;
		}	
		//将文件信息存入列表中
		vectFileList.push_back(DirInfo);

	}

	//循环读取目录下文件
	 while(!(_findnext(handle,&filestruct))) 
	 	{
			//获取文件属性
			if (stat(filestruct.name,&FileBuf)==0)
			{
				DirInfo.strFileName=filestruct.name;
				
				//判断文件是否为目录文件
				if((FileBuf.st_mode) & _S_IFDIR)
				{
					DirInfo.iFileType=GS_FILE_DIRECTORY;
				}
				//判断文件是否为普通文件
				else if ((FileBuf.st_mode) & _S_IFREG)
				{
					DirInfo.iFileType=GS_FILE_COMMON;
				}
				//判断文件是否为管道文件
				else if ((FileBuf.st_mode) & _S_IFIFO)
				{
					DirInfo.iFileType=GS_FILE_FIFO;
				}
				//文件为其他类型
				else 
				{
					DirInfo.iFileType=GS_FILE_OTHER;
				}	
				//将文件信息存入列表中
				vectFileList.push_back(DirInfo);
			}
		 }

	 //关闭目录
    	_findclose(handle); 
		

#elif _LINUX

	struct dirent *dirStru;

	//判断文件句柄是否为空
	if (m_GSDir!=NULL)
	{
		//循环读取文件列表
		while((dirStru=readdir((DIR *)m_GSDir))!=NULL)
		{
			//读取文件失败，返回
			if(errno != 0)
			{
				return;
			}
			//获取文件属性
			if (stat(dirStru->d_name,&FileBuf)==0)
			{
				DirInfo.strFileName=dirStru->d_name;

				//判断文件是否是目录文件
				if(S_ISDIR(FileBuf.st_mode))
				{
					DirInfo.iFileType=GS_FILE_DIRECTORY;
				}
				//判断文件是否是普通文件
				else if (S_ISREG(FileBuf.st_mode))
				{
					DirInfo.iFileType=GS_FILE_COMMON;
				}
				//判断文件是否为管道文件
				else if (S_ISFIFO(FileBuf.st_mode))
				{
					DirInfo.iFileType=GS_FILE_FIFO;
				}
				//文件为其他文件
				else 
				{
					DirInfo.iFileType=GS_FILE_OTHER;
				}
				//将文件信息存入列表
				vectFileList.push_back(DirInfo);
			}
			
		}
		
	}
	
#endif

}




