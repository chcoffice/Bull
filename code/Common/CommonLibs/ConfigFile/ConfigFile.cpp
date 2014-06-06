/*************************************************
Copyright (C), 2010-2011, GOSUN 
File name : ConfigFile.cpp      
Author : laodx, jinfn
Version : Vx.xx        
DateTime : 2010/5/11 16:06
Description :     // 用于详细说明此程序文件完成的主要功能，与其他模块
// 或函数的接口，输出值、取值范围、含义及参数间的控
// 制、顺序、独立或依赖等关系
*************************************************/ 

#include "ConfigFile.h"

// 构造函数
CConfigFile::CConfigFile(void)
{
	m_vectSectionList.clear();
	m_szSplitToken = '#';
}

// 析构函数
CConfigFile::~CConfigFile(void)
{
	ClearAllProperty();
}

/*************************************************
  Function:	LoadFile     
  DateTime: 2010/5/11 16:15	
  Description: 装载配置文件
  Input:
		pszFileName:	配置文件的路径，不能为空
  Output:         	
  Return:
		TRUE:	成功
		FALSE:	失败
  Note:
*************************************************/
BOOL CConfigFile::LoadFile(char *pszFileName, char szToken /* = */ )
{
	m_szSplitToken = szToken;

	StruSectionInfo *pstSectionInfo = NULL;
	StruItemInfo stItemInfo;

	FILE *fpConfig = NULL;
	char szBuf[MAX_LEN_OF_LINE];
	string::size_type pos;
	string strComment;

	// 文件名为空则返回出错
	if (pszFileName == NULL)
		return FALSE;

	// 打开文件，失败则返回出错
#ifdef WINCE
	fpConfig = fopen(pszFileName, "r");
#elif _WIN32
	fopen_s(&fpConfig, pszFileName, "r");
#elif _LINUX
	fpConfig = fopen(pszFileName, "r");
#endif
	if (fpConfig == NULL)
		return FALSE;

	// 记录该配置文件的路径名
	m_strConfigFile = pszFileName;

	// 读取配置文件中的每一行
	while (fgets(szBuf, MAX_LEN_OF_LINE, fpConfig))
	{
		// 去掉前面的空格符
		string strLine = TrimLeft(szBuf);

		// 如果是空行则忽略，继续下一行
		if (strLine.empty() || strLine.at(0)=='\r' || strLine.at(0)=='\n')
			continue;

		// 是否为注释
		if (strLine.at(0) == COMMENT_CHAR)
		{
			// 若之前无注释，则保存除 szToken 外的所有内容；
			// 若之前已存在注释，则将本行的注释内容（包括szToken)附加在后面
			if (strComment.empty())
				strComment = strLine.substr(1);
			else
				strComment += strLine;

			continue;
		}

		// 是否为段
		if (strLine.at(0) == '[')
		{
			// 如果不存在对应的结束符号']'，则视为非法内容，忽略之，并删除之前的注释，然后继续下一行
			// 若存在']'，则为合法的段名称，创建新的段信息结构，填写段名称和注释，然后放入段链表中
			pos = strLine.find(']');
			if (pos != string::npos)
			{
				pstSectionInfo = new StruSectionInfo;

				// 填写配置段的名称（因为字符串中包含了'['，因此substr()的count参数为pos-1）
				pstSectionInfo->strName = Trim(strLine.substr(1, pos-1).c_str());

				// 检查右方是否有注释，有则进行处理
				strLine = Trim(strLine.substr(pos+1).c_str());
				if (!strLine.empty() && strLine.at(0) == szToken)
					pstSectionInfo->strCommentRight = Trim(strLine.substr(1).c_str());
				else
					pstSectionInfo->strCommentRight = "";

				// 如果上方存在注释，则进行处理
				pstSectionInfo->strCommentTop = TrimRight(strComment.c_str());	// 删除后面的分隔符和回车符

				// 放入配置段链表中
				m_vectSectionList.push_back(pstSectionInfo);
			}

			// 清楚上方的注释内容
			strComment.clear();

			continue;
		}

		// 排除以上可能，剩下的应该是配置项，格式为：
		// 1) KEY = VALUE
		// 2) STRING

		// 填写配置项信息结构的属性名、值、注释，并加入对应段结构的属性链表中

		// 查找'='，不存在则视为STRING类型的配置项
		pos = strLine.find('=');
		if (pos == string::npos)
		{
			//STRING类型的配置项，则将strKey置为空
			stItemInfo.strKey = "";
		}
		else
		{
			// 填写Key（substr的count参数为pos）
			stItemInfo.strKey = Trim(strLine.substr(0, pos).c_str());

			// 去掉Key部分的内容和等号，剩余的内容是：Value和注释
			strLine.erase(0, pos + 1);
		}

		// 检查右方是否有注释，并进行相应的处理：Value、右方注释
		pos = strLine.find(szToken);
		if (pos != string::npos)
		{
			// 填写Value：'szToken'左边的内容
			stItemInfo.strValue = Trim(strLine.substr(0, pos - 1).c_str());

			// 填写注释：'szToken'右边的内容
			stItemInfo.strCommentRight = Trim(strLine.substr(pos + 1).c_str());
		}
		else
		{
			stItemInfo.strValue = Trim(strLine.c_str());
			stItemInfo.strCommentRight = "";
		}

		// 填写上方的注释
		stItemInfo.strCommentTop = TrimRight(strComment.c_str());	// 删除后面的分隔符和回车符
		strComment.clear();

		// 找到配置项，但在它之前尚未定义任何配置段，则创建一个空的配置段
		if (pstSectionInfo == NULL)
		{
			pstSectionInfo = new StruSectionInfo;
			m_vectSectionList.push_back(pstSectionInfo);
		}

		// 将该配置项，放入当前的配置段的配置项列表中
		pstSectionInfo->vectItemList.push_back(stItemInfo);
	}

	fclose(fpConfig);

	return TRUE;
}

/*************************************************
  Function:	SaveFile     
  DateTime: 2010/5/11 16:16	
  Description: 保存更改过的配置文件
  Input:
		pszFileName:	将配置保存到该文件，若该参数为空，则保存到LoadFile()时指定的文件中
  Output:
  Return:
		TRUE:	成功
		FALSE:	失败
  Note:				
*************************************************/
BOOL CConfigFile::SaveFile(char *pszFileName)
{
	StruSectionInfoPtr pSectionInfo;
	VectSectionList::iterator iter;

	FILE *fpConfig;
	char *fnSaveFile;

	fnSaveFile = (pszFileName != NULL) ? pszFileName : (char *)m_strConfigFile.c_str();
	if (fnSaveFile == NULL)
		return FALSE;

#ifdef WINCE
	fpConfig = fopen(pszFileName, "w");
#elif _WIN32
	fopen_s(&fpConfig, fnSaveFile, "w");
#elif _LINUX
	fpConfig = fopen(fnSaveFile, "w");
#endif
	if (fpConfig == NULL)
		return FALSE;

	// 遍历每个Section
	for (iter = m_vectSectionList.begin(); iter != m_vectSectionList.end(); iter++)
	{
		pSectionInfo = *iter;

		if (pSectionInfo == NULL)
			continue;

		//
		// 将Section写入文件中
		//

		// 写注释
		if (! pSectionInfo->strCommentTop.empty())
			fprintf(fpConfig, "%c%s\n", m_szSplitToken, pSectionInfo->strCommentTop.c_str());

		// 写段的名称
		if (! pSectionInfo->strName.empty())
		{
			// 写：[段名称]
			fprintf(fpConfig, "[%s]", pSectionInfo->strName.c_str());

			// 右方是否有注释？
			if (! pSectionInfo->strCommentRight.empty())
				fprintf(fpConfig, "\t%c %s\n", m_szSplitToken, pSectionInfo->strCommentRight.c_str());
			else
				fprintf(fpConfig, "\n");
		}

		// 写段内的所有配置项
		for (vector<StruItemInfo>::iterator iter2 = pSectionInfo->vectItemList.begin(); iter2 != pSectionInfo->vectItemList.end(); iter2++)
		{
			// 上方是否有注释？
			if (! iter2->strCommentTop.empty())
				fprintf(fpConfig, "%c%s\n", m_szSplitToken, iter2->strCommentTop.c_str());

			if (iter2->strKey.empty())
			{
				// 写：STRING
				fprintf(fpConfig, "%s", iter2->strValue.c_str());
			}
			else
			{
				// 写：Key = Value
				fprintf(fpConfig, "%s = %s", iter2->strKey.c_str(), iter2->strValue.c_str());
			}

			// 右方是否有注释？
			if (! iter2->strCommentRight.empty())
				fprintf(fpConfig, "\t%c %s\n", m_szSplitToken, iter2->strCommentRight.c_str());
			else
				fprintf(fpConfig, "\n");
		}

		// 在每个配置段的结束时，增加一个空行
		fprintf(fpConfig, "\n");
	}

	fclose(fpConfig);

	return TRUE;
}

/*************************************************
  Function:	GetSectionNameList    
  DateTime:	2010/5/11 16:11	
  Description: 获取所有配置段的个数和名称链表
  Input:
  Output:
		vectNameList:	存放配置项的名称的链表
  Return:
		配置段的总数
  Note:
*************************************************/
INT32 CConfigFile::GetSectionNameList(vector<string>& vectNameList)
{
	StruSectionInfoPtr pSectionInfo;
	VectSectionList::iterator iter;

	int iSectionNum = 0;

	// 遍历每个配置段
	for (iter = m_vectSectionList.begin(); iter != m_vectSectionList.end(); iter++)
	{
		pSectionInfo = *iter;

		if (pSectionInfo == NULL)
			continue;

		// 将配置段的名称加入链表中
		vectNameList.push_back(pSectionInfo->strName);

		iSectionNum++;
	}

	return iSectionNum;
}

/*************************************************
  Function:	GetAllStringOfSection
  DateTime:	2010/8/18
  Description: 获取指定配置段下的所有配置串
  Input:
  Output:
        pszSection: 指定配置段的名称，不能为空
		vectString:	存放配置串的链表
  Return:
		配置串的总数
  Note:
*************************************************/
INT32 CConfigFile::GetAllStringOfSection(char *pszSection, vector<string>& vectString)
{
	StruSectionInfoPtr pSectionInfo;
	VectSectionList::iterator iter;
	INT32 iNum = 0;

	// pszSection不能为空
	if (pszSection == NULL)
		return FALSE;

	// 遍历每个Section
	for (iter = m_vectSectionList.begin(); iter != m_vectSectionList.end(); iter++)
	{
		pSectionInfo = *iter;

		if (pSectionInfo == NULL)
			continue;

		// 若指定了Section名，则判断Section是否为要找的Section，不是则忽略，继续查找下一个Section
		if ((pszSection != NULL) && (strcmp(pSectionInfo->strName.c_str(), pszSection) != 0))
			continue;

		// 遍历所有的配置项
		for (vector<StruItemInfo>::iterator iter2 = pSectionInfo->vectItemList.begin(); iter2 != pSectionInfo->vectItemList.end(); iter2++)
		{
			if (iter2->strKey.empty())
				vectString.push_back(iter2->strValue); //STRING类型的配置项
			else
				vectString.push_back(iter2->strKey+"="+iter2->strValue); //KEY=VALUE类型的配置项

			iNum++;
		}

		break;
	}

	return iNum;
}

/*************************************************
  Function:	GetProperty
  DateTime: 2010/5/11 16:19	
  Description:  获取某个配置项的值
  Input:
		pszSection:	配置段名称，为空则表示在所有配置段中进行查找
		pszKey:		配置项的键
		pszDefaultValue:若配置项不存在，则返回该默认值
  Output:       
  Return:
		返回配置项的值，若配置项不存在，则返回pszDefaultValue的值        
  Note:				
*************************************************/
string CConfigFile::GetProperty(char *pszSection,char *pszKey, char *pszDefaultValue)
{
	StruItemInfoPtr pItemInfo = FindItem(pszSection, pszKey);
	if (pItemInfo == NULL)
		return (pszDefaultValue != NULL) ? pszDefaultValue : "";	// 未找到配置项
	else
	{		
		string str = Trim(pItemInfo->strValue.c_str());
		// 如果字符串为空则忽略，返回默认配置值
		if (str.empty() || str.at(0)=='\r' || str.at(0)=='\n')
		{
			return (pszDefaultValue != NULL) ? pszDefaultValue : "";	// 未找到配置项
		}
	}
	return pItemInfo->strValue;	// 找到配置项
}

INT32 CConfigFile::GetProperty(char *pszSection,char *pszKey, INT32 iDefaultValue)
{
	StruItemInfoPtr pItemInfo = FindItem(pszSection, pszKey);
	return (pItemInfo == NULL) ? iDefaultValue : atoi(pItemInfo->strValue.c_str());
}

/*************************************************
  Function:	SetProperty
  DateTime: 2010/5/11 16:23	
  Description: 修改某个配置项的值
  Input:
		pszSection:	配置段的名称，为空则表示在所有配置段中进行查找
		pszKey:		配置项的键，不能为空
		pszValue:	配置项的值，不能为空
		pszComment:	配置项的注释
  strKey：配置项名称；
  strDefaultValue:默认值           
  Output:        
  Return:	设置是否成功，失败返回FALSE，成功返回TRUE         	
  Note:				
*************************************************/
BOOL CConfigFile::SetProperty(char *pszSection, char *pszKey, char *pszValue, char *pszComment)
{
	StruItemInfoPtr pItemInfo;

	// 配置项的键、值都不能为空
	if (pszKey == NULL || pszValue == NULL)
		return FALSE;

	// 检查配置项是否存在，不存在则返回出错
	pItemInfo = FindItem(pszSection, pszKey);
	if (pItemInfo == NULL)
		return FALSE;

	// 修改配置项的值、注释

	pItemInfo->strValue = pszValue;

	if (pszComment != NULL)
		pItemInfo->strCommentRight = pszComment;

	return TRUE;
}

BOOL CConfigFile::SetProperty(char *pszSection, char *pszKey, INT32 iValue, char *pszComment)
{
	ostringstream oss;
	oss << iValue;
	return SetProperty(pszSection, pszKey, (char *)oss.str().c_str(), pszComment);
}

/*************************************************
  Function:	AddProperty
  DateTime: 2010/5/11 16:23	
  Description:	增加一个配置项
  Input:
		pszSection:	配置段名称；若指定的配置段不存在，则创建新的配置段；若该参数为空，则将配置项添加到最后一个配置段中；
		pszKey:		配置项的键
		pszValue:	配置项的值
		pszComment:	配置项的注释
  Output:        
  Return:
		FALSE:		失败
		TRUE:		成功         	
  Note:				
*************************************************/
BOOL CConfigFile::AddProperty(char *pszSection,char *pszKey, char *pszValue, char *pszComment)
{
	StruItemInfoPtr pItemInfo = NULL;
	StruSectionInfoPtr pSectionInfo = NULL;
	VectSectionList::iterator iter;

	// 配置项的键、值都不能为空
	if (pszKey == NULL || pszValue == NULL)
		return FALSE;
	
	// 检查配置项是否存在，存在则返回出错
	pItemInfo = FindItem(pszSection, pszKey);
	if (pItemInfo != NULL)
		return FALSE;

	if (pszSection == NULL)
	{
		// pszSection为空，则将配置项添加到最后一个配置段中

		if (! m_vectSectionList.empty())
			pSectionInfo = m_vectSectionList.back();
	}
	else
	{
		// 指定了pszSection，则查找该Section是否存在

		for (iter = m_vectSectionList.begin(); iter != m_vectSectionList.end(); iter++)
		{
			pSectionInfo = *iter;

			// 找到Section，退出
			if (strcmp(pSectionInfo->strName.c_str(), pszSection) == 0)
				break;

			pSectionInfo = NULL;
		}
	}

	// pSectionInfo为空，表示配置段不存在，需要增加新的配置段
	if (pSectionInfo == NULL)
	{
		pSectionInfo = new StruSectionInfo;
		pSectionInfo->strName = (pszSection ? pszSection : "");
		m_vectSectionList.push_back(pSectionInfo);
	}

	// 填写配置项的信息，并添加到相应配置段的配置项链表中
	pItemInfo = new StruItemInfo;
	pItemInfo->strKey = pszKey;
	pItemInfo->strValue = pszValue;
	pItemInfo->strCommentRight = (pszComment != NULL) ? pszComment : "";
	pSectionInfo->vectItemList.push_back(*pItemInfo);
	delete pItemInfo;

	// 添加成功
	return TRUE;
}

BOOL CConfigFile::AddProperty(char *pszSection,char *pszKey, INT32 iValue, char *pszComment)
{
	ostringstream oss;
	oss << iValue;
	return AddProperty(pszSection, pszKey, (char *)oss.str().c_str(), pszComment);
}

/*************************************************
  Function:	DeleteProperty  
  DateTime: 2010/5/11 16:27	
  Description: 删除指定的配置项
  Input:
		pszSection:	配置段的名称，为空则表示在所有配置段中进行查找
		pszKey:		配置项的名称，为空则表示删除整个配置段
  strKey：配置项名称；         	
  Output:         
  Return:删除某个配置项是否成功，成功返回TRUE，失败返回FALSE         	
  Note:				
*************************************************/
BOOL CConfigFile::DeleteProperty(char *pszSection, char *pszKey)
{
	StruSectionInfoPtr pSectionInfo;
	VectSectionList::iterator iter;

	// 至少有一个不为空
	//
	// pszSection不空，pszKey不空：	在pszSection中查找pszKey，并删除
	// pszSection空，pszKey不空：	在所有Section中查找pszKey，并删除
	// pszSection不空，pszKey空：	删除整个pszSection
	if ((pszSection == NULL) && (pszKey == NULL))
		return FALSE;

	// 遍历每个Section
	for (iter = m_vectSectionList.begin(); iter != m_vectSectionList.end(); iter++)
	{
		pSectionInfo = *iter;

		if (pSectionInfo == NULL)
			continue;

		// 若指定了Section名，则判断Section是否为要找的Section，不是则忽略，继续查找下一个Section
		if ((pszSection != NULL) && (strcmp(pSectionInfo->strName.c_str(), pszSection) != 0))
			continue;

		// pszKey为空，表示删除整个section
		if (pszKey == NULL)
		{
			pSectionInfo->vectItemList.clear();
			m_vectSectionList.erase(iter);
			delete pSectionInfo;
			return TRUE;
		}

		// 查找同名的配置项，找到则将其删除
		for (vector<StruItemInfo>::iterator iter2 = pSectionInfo->vectItemList.begin(); iter2 != pSectionInfo->vectItemList.end(); iter2++)
		{
			if (strcmp(iter2->strKey.c_str(), pszKey) == 0)
			{
				pSectionInfo->vectItemList.erase(iter2);

				// 若配置项链表为空，则将其对应的配置段也删除
				if (pSectionInfo->vectItemList.empty())
					m_vectSectionList.erase(iter);

				return TRUE;
			}
		}
	}

	// 未找到配置项，返回出错
	return FALSE;
}

/*************************************************
  Function:	ClearAllProperty
  DateTime: 2010/5/11 16:28	
  Description;	删除所有的配置项
  Input:
  Output:         
  Return:
  Note:				
*************************************************/
void CConfigFile::ClearAllProperty(void)
{
	StruSectionInfoPtr pSectionInfo;
	VectSectionList::iterator iter;

	// 遍历每个配置段
	for (iter = m_vectSectionList.begin(); iter != m_vectSectionList.end(); iter++)
	{
		pSectionInfo = *iter;

		if (pSectionInfo == NULL)
			continue;

		// 清空配置段中的配置项链表
		pSectionInfo->vectItemList.clear();

		// 清除配置段对应的结构
		delete pSectionInfo;
	}

	// 清空配置段链表
	m_vectSectionList.clear();
}

/*************************************************
  以下为私有函数
*************************************************/

/*************************************************
  Function:	FindItem     
  DateTime: 2010/6/04 10:28	
  Description: 根据段名和配置名查找配置项
  Input:
		pszSection:	配置段的名称，为空则表示在所有配置段中进行查找
		pszKey:		配置项的名称，不能为空
  Output:
  Return:
		NULL:		未找到对应的配置项
		非NULL:		对应配置项的信息结构指针
  Note:				
*************************************************/
StruItemInfoPtr CConfigFile::FindItem(char *pszSection, char *pszKey)
{
	StruSectionInfoPtr pSectionInfo;
	VectSectionList::iterator iter;

	// 配置名不能为空，否则返回出错
	if (pszKey == NULL)
		return NULL;

	// 遍历每个配置段
	for (iter = m_vectSectionList.begin(); iter != m_vectSectionList.end(); iter++)
	{
		pSectionInfo = *iter;

		if (pSectionInfo == NULL)
			continue;

		// 若指定了Section名，则判断Section是否为要找的Section，不是则忽略，继续查找下一个Section
		if ((pszSection != NULL) && (strcmp(pSectionInfo->strName.c_str(), pszSection) != 0))
			continue;

		// 在配置段中遍历每个配置项，找到同名的配置项则结束函数，返回对应的配置项结构
		for (vector<StruItemInfo>::iterator iter2 = pSectionInfo->vectItemList.begin(); iter2 != pSectionInfo->vectItemList.end(); iter2++)
		{
			if (strcmp(iter2->strKey.c_str(), pszKey) == 0)
			{				
				return &(*iter2);
			}
			
		}
	}

	// 未找到同名的配置项，返回出错
	return NULL;
}

/*************************************************
  Function:	Trim     
  DateTime: 2010/6/04 10:28	
  Description: 去掉字符串前后的分隔符，包括：空格、制表符、回车、换行。
  Input:
		pszLine:	待处理的字符串
  Output:
  Return:
		去掉分隔符后的string
  Note:				
*************************************************/
string CConfigFile::Trim(const char *pszLine)
{
	string::size_type pos;
    string str = pszLine;

	pos = str.find_first_not_of(" \t\n\r");
	if (pos != string::npos)
		str.erase(0, pos);

	// 删除前面的空格、制表符、回车、换行
	str.erase(0,str.find_first_not_of(" "));

	pos = str.find_last_not_of(" \t\n\r");
	if (pos != string::npos)
		str.erase(pos + 1);
	//删除后面的空格、制表符、回车、换行
	str.erase(str.find_last_not_of(" ") + 1,str.size()-1);

    return str;
}

string CConfigFile::TrimLeft(const char *pszLine)
{
	string::size_type pos;
    string str = pszLine;

	pos = str.find_first_not_of(" \t\n\r");
	if (pos != string::npos)
		str.erase(0, pos);
	// 删除前面的空格
	str.erase(0,str.find_first_not_of(" "));
    return str;
}

string CConfigFile::TrimRight(const char *pszLine)
{
	string::size_type pos;
    string str = pszLine;

	pos = str.find_last_not_of(" \t\n\r");
	if (pos != string::npos)
		str.erase(pos + 1);
	//删除后面的空格
	str.erase(str.find_last_not_of(" ") + 1,str.size()-1);

    return str;
}
