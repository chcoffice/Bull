/*************************************************
Copyright (C), 2010-2011, GOSUN 
File name : CConfigFile.H      
Author : laodx, jinfn      
Version : V1.00       
DateTime : 2010/5/11 16:06
Description :配置文件类用于完成对配置文件的装载，读取，修改，删除，保存的操作
*************************************************/

#ifndef _CONFIG_FILE_H_
#define _CONFIG_FILE_H_

#include "GSCommon.h"
#include "GSType.h"

#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

#define COMMENT_CHAR '#'
#define MAX_LEN_OF_LINE 256

typedef struct
{
	string strKey;
	string strValue;

	string strCommentTop;
	string strCommentRight;
} StruItemInfo, *StruItemInfoPtr;

typedef struct
{
	string strName;

	string strCommentTop;
	string strCommentRight;

	vector<StruItemInfo> vectItemList;
} StruSectionInfo, *StruSectionInfoPtr;

typedef vector<StruSectionInfoPtr> VectSectionList;

class CConfigFile
{
public:
	CConfigFile(void);
	~CConfigFile(void);

public:
	//载入配置文件
	BOOL LoadFile(char *pszFileName, char szToken = '#');
  
	//保存更改过的配置文件
	BOOL SaveFile(char *pszFileName);

	//获得所有配置段的个数
	INT32 GetSectionNameList(vector<string>& vectNameList);

	//获得指定Section下的所有配置串
	INT32 GetAllStringOfSection(char *pszSection, vector<string>& vectString);

	//获取某个配置项的值
	string GetProperty(char *pszSection, char *pszKey, char *pszDefaultValue);
	INT32 GetProperty(char *pszSection, char *pszKey, INT32 iDefaultValue);

	//设置某个配置项的值
	BOOL SetProperty(char *pszSection, char *pszKey, char *pszValue, char *pszComment);
	BOOL SetProperty(char *pszSection, char *pszKey, INT32 iValue, char *pszComment);

	//增加某个配置项的值
	BOOL AddProperty(char *pszSection, char *pszKey, char *pszValue, char *pszComment);
	BOOL AddProperty(char *pszSection, char *pszKey, INT32 iValue, char *pszComment);

	//删除某个配置项
	BOOL DeleteProperty(char *pszSection, char *pszKey);

	//删除所有配置项
	void ClearAllProperty(void);

private:
	string Trim(const char *pszLine);
	string TrimLeft(const char *pszLine);
	string TrimRight(const char *pszLine);
	StruItemInfoPtr FindItem(char* pszSection, char* pszKey);	

private:
	string m_strConfigFile;
	VectSectionList m_vectSectionList;
	char m_szSplitToken;
};
#endif // _CONFIG_FILE_H_
