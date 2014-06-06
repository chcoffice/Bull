#ifndef _MODULE_MANAGER_H
#define _MODULE_MANAGER_H
 
#include"IModuleBase.h"

#if !defined(_WIN32) && !defined(_LINUX)
#error This program only support Windows, Linux...
#endif

#ifdef _LINUX
#include <dlfcn.h>

typedef void* HMODULE;
#endif

class CModuleManager;

typedef std::vector<IModuleBasePtr> VectModuleList;
typedef std::map<IModuleBasePtr, HMODULE> MapModuleList;

class CModuleManager
{
public:
	CModuleManager();
	virtual ~CModuleManager();

public:
	//根据插件路径加载插件
	IModuleBasePtr Load(const std::string strDllPath, const INT32 iModuleType);		

	//根据插件名称卸载插件
	BOOL UnLoad(const std::string strDllName);

	//卸载全部插件 
	BOOL UnloadAll();

	//根据插件名称获取插件控制结构指针
	IModuleBasePtr GetModule(const std::string strDllName);

	//获取插件结构信息列表
	INT32 GetModuleList(VectModuleList& vectModuleList);

private:

	MapModuleList m_mapModuleList;
};

#endif			//_MODULE_MANAGER_H
