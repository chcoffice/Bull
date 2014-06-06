
#include "ISystemLayInterface.h"


/******************************************************************************
功能说明：磁盘枚举
******************************************************************************/

/********************************************************************************************
  Function		: GSGetSysDisk    
  DateTime		: 2010/6/10 10:41	
  Description	: 枚举磁盘
  Input			: std::vector<std::string> &vectDiskList，磁盘列表
  Output		: NULL
  Return		: 磁盘个数
  Note			:				// 备注
********************************************************************************************/
INT GSGetSysDisk(std::vector<std::string> &vectDiskList)
{

	INT iDiskNum=0;
	std::string strDiskName;
	vectDiskList.clear();
#ifdef WINCE
	return iDiskNum;
#elif _WIN32

	DWORD dwDriveStrlen=0;
	//通过GetLogicalDriveStrings()函数获取所有驱动器字符串信息长度。
	dwDriveStrlen=GetLogicalDriveStrings(0,NULL);

	//用获取的长度在堆区创建一个字符串数组
	// char *szDriveName=new char[dwDriveStrlen];
	//将申请的长度改为乘2,避免GetLogicalDriveStrings后delete[] szDriveName出错
	char *szDriveName=new char[dwDriveStrlen * 2];
	memset(szDriveName, 0, dwDriveStrlen * 2);
	char *pDriveName=NULL;

	//通过GetLogicalDriveStrings将字符串信息复制到堆区数组中,其中保存了所有驱动器的信息。
	if(GetLogicalDriveStrings(dwDriveStrlen * 2,(LPTSTR)szDriveName))
	{
		pDriveName=szDriveName;
		while(*pDriveName!=NULL)
		{
			//INT DType;
			////判断磁盘类型
			//DType = GetDriveType((LPCWSTR)pDriveName);
			//if(DType == DRIVE_FIXED)
			//{
			//	iDiskNum++;
			//	//将驱动器信息存入列表
			//	strDiskName=pDriveName;
			//	pDriveName+=8;
			//	vectDiskList.push_back(strDiskName);
			//}	
			//将驱动器信息存入列表
			iDiskNum++;
			strDiskName=pDriveName;
			pDriveName+=8;
			vectDiskList.push_back(strDiskName);
		}
	}
	else 
	{
		iDiskNum = 0;
	}

	//删除创建的字符串数组
	if(NULL != szDriveName)
	{
		delete[] szDriveName;
		szDriveName = NULL;
	}

#elif _LINUX

	FILE *fp;
	INT i=0;
	char czDiskName[20];
	char czDiskInfoLine[255]; 
	char *p;
	//打开读取磁盘信息
	if((fp=fopen("/proc/partitions","r"))==NULL) 
	{
		exit(0);
	}
	//读取文件的头两行
	fgets(czDiskInfoLine,255,fp);
	fgets(czDiskInfoLine,255,fp);
	//从第三行开始循环读取文件
	while (fgets(czDiskInfoLine,255,fp)!=NULL)
	{
		iDiskNum++;	
		//调用strtok分割字符串czDiskInfoLine
		p=strtok(czDiskInfoLine," ");
		i=0;
		do   
		{  
			i++;
			p = strtok(NULL, " "); //NULL即为上面返回的指针   
			if (i==3)
			{
				//将取得的磁盘名称格式化
				snprintf(czDiskName, 20, "%s", p);
				//将磁盘信息存入列表
				vectDiskList.push_back(czDiskName);
			}
		}while(p); 
	}
	//关闭文件
	fclose(fp);

#endif

	//返回磁盘个数
	return iDiskNum;

}



