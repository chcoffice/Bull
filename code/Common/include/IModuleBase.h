#ifndef _IMODULE_BASE_H
#define _IMODULE_BASE_H

#include "GSType.h"

#if !defined(_WIN32) && !defined(_LINUX)
#error This program only support Windows, Linux...
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _WIN32
#define MODULE_API extern "C" __declspec(dllexport)
#else /* _LINUX */
#define MODULE_API extern "C"
#endif

//插件信息
typedef struct _Module_Info
{
	INT32 iType;			//插件类型
	INT32 iVersion;			//插件板本
	std::string strName;	//插件名称
	std::string strPath;	//插件路径

	BOOL bUsing;			//记录该插件的使用状态
	
	//初始化插件信息
	_Module_Info() : iType(0), iVersion(0), strName(""), strPath(""), bUsing(FALSE)
	{
	}
} StruModuleInfo, *StruModuleInfoPtr;

class IModuleBase
{
public:

	//缺省构造函数
	IModuleBase(void) {};

	//缺省析构函数
	virtual ~IModuleBase(void) {};

	//获取模块信息
	virtual StruModuleInfo& GetModuleInfo()
	{
		return m_ModuleInfo;
	};

	//初始化插件内部的资源
	virtual void Init(void) {};			

	//释放插件内部的资源
	virtual void UnInit(void) {};

protected:

	StruModuleInfo m_ModuleInfo;
};

typedef IModuleBase* IModuleBasePtr;

typedef IModuleBasePtr (*FuncModuleEntrancePtr)();	//插件入口函数的类型定义

//定义插件输出函数名
#define MODULE_ENTRANCE_FUNC "ModuleEntrance"

//定义插件的入口函数
#define DECLARE_MODULE_ENTRANCE_FUNC() \
MODULE_API IModuleBasePtr ModuleEntrance()

//实现插件的入口函数
#define IMPLEMENT_MODULE_ENTRANCE_FUNC(module_class_name, type, ver, name) \
MODULE_API IModuleBasePtr ModuleEntrance(void) \
{ \
	module_class_name * pcsModule; \
 \
    pcsModule = new module_class_name(); \
 \
	pcsModule->GetModuleInfo().iType = type; \
	pcsModule->GetModuleInfo().iVersion = ver; \
	pcsModule->GetModuleInfo().strName = name; \
	pcsModule->GetModuleInfo().strPath = ""; \
	pcsModule->GetModuleInfo().bUsing = FALSE; \
 \
	return pcsModule; \
}

typedef enum
{
	MODULE_TYPE_SDK = 1,			//SDK插件
	MODULE_TYPE_STREAMING_PROTOCOL,	//流转发协议插件

	MODULE_TYPE_TEST_ADD = 1001,	//测试用的ADD插件
	MODULE_TYPE_TEST_SUB,			//测试用的SUB插件

	MODULE_TYPE_ANY = -1,			//任意类型
} EnumModuleType;

#endif		//_IMODULE_BASE_H
