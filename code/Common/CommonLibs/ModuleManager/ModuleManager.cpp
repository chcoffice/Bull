#include "ModuleManager.h"

#ifdef _WIN32
#define LOAD_MODULE(dll)			LoadLibraryA(dll)
#define FREE_MODULE(handle)			FreeLibrary(handle)
#define GET_PROC(handle, proc_name)	GetProcAddress(handle, proc_name)
#else /* _LINUX */
#define LOAD_MODULE(dll)			dlopen(dll, RTLD_LAZY)
#define FREE_MODULE(handle)			dlclose(handle)
#define GET_PROC(handle, proc_name)	dlsym(handle, proc_name)
#endif

CModuleManager::CModuleManager()
{
	m_mapModuleList.clear();
}

CModuleManager::~CModuleManager()
{
	UnloadAll();
}

/************************************************************************************************************/
/*funtion:根据插件路径加载插件,获取插件控制结构,执行插件控制结构的Init过程									*/
/*parameter:strDllPath 要加载的插件的绝对路径																*/
/*          iModuleType 要加载的插件的类型																	*/
/*return: 成功则返回插件的控制结构指针,失败则返回NULL														*/
/*author:hwh																								*/
/*time:2010.5.17-08:44																						*/
/************************************************************************************************************/
IModuleBasePtr CModuleManager::Load(const std::string strDllPath, const INT32 iModuleType)
{
	//加载插件(动态链接库)
	HMODULE hModule = NULL;

	hModule = LOAD_MODULE(strDllPath.c_str());
	if (!hModule)
		return NULL;

	MapModuleList::iterator iter;

	//查找该插件是否在插件列表里面 
	for(iter = m_mapModuleList.begin(); iter != m_mapModuleList.end(); iter ++)
	{
		if(hModule == iter->second)
		{
			return iter->first;
		}
	}

	//获取插件入口函数
	FuncModuleEntrancePtr pfModuleEntrance = NULL;

#ifdef WINCE
	pfModuleEntrance = (FuncModuleEntrancePtr)GET_PROC(hModule, TEXT("MODULE_ENTRANCE_FUNC"));
#else
	pfModuleEntrance = (FuncModuleEntrancePtr)GET_PROC(hModule, MODULE_ENTRANCE_FUNC);
#endif

	if (! pfModuleEntrance)
	{
		FREE_MODULE(hModule);
		return NULL;
	}

	//获取插件信息控制结构
	IModuleBasePtr pModule = NULL;

	pModule = pfModuleEntrance();
	if (!pModule)
	{
		FREE_MODULE(hModule);
		return NULL;
	}

	if ((iModuleType != MODULE_TYPE_ANY) && (pModule->GetModuleInfo().iType != iModuleType))
	{
		FREE_MODULE(hModule);
		return NULL;
	}

	//根据插件名称卸载以前加载的同名插件
	UnLoad(pModule->GetModuleInfo().strName);

	//初始化插件内部的资源
	pModule->Init();

	//设置插件的路径 
	pModule->GetModuleInfo().strPath = strDllPath;
	
	//将插件放入插件信息控制结构列表里面 
	m_mapModuleList[pModule] = hModule;
	
	//返回插件控制结构指针 
	return pModule;
}

/************************************************************************************************************/
/*funtion:根据插件名称卸载插件																				*/
/*parameter:strModuleName 要卸载的插件的名称																*/
/*return: 成功则返TRUE,失败则返回FALSE																		*/
/*author:hwh																								*/
/*time:2010.5.17-09:24																						*/
/************************************************************************************************************/
BOOL CModuleManager::UnLoad(const std::string strModuleName)
{
	MapModuleList::iterator iter;
	IModuleBasePtr pModule = NULL;
	HMODULE hModule = NULL;

	//查找插件
	for(iter = m_mapModuleList.begin(); iter != m_mapModuleList.end(); iter ++)
	{
		pModule = iter->first;

		if(strModuleName == pModule->GetModuleInfo().strName)
		{
			//释放插件内部的资源
			pModule->UnInit();

			delete pModule;
			pModule = NULL;

			//卸载该插件 
			hModule = iter->second;

			FREE_MODULE(hModule);

			//从插件列表中移除该插件
			m_mapModuleList.erase(iter);

			return TRUE;
		}
	}

	return FALSE;
}

/************************************************************************************************************/
/*funtion:卸载全部插件																						*/
/*parameter:无																								*/
/*return: 成功则返TRUE,失败则返回FALSE																		*/
/*author:hwh																								*/
/*time:2010.5.17-09:34																						*/
/************************************************************************************************************/
BOOL CModuleManager::UnloadAll()
{
	MapModuleList::iterator iter;
	IModuleBasePtr pModule = NULL;
	HMODULE hModule = NULL;

	for(iter = m_mapModuleList.begin(); iter != m_mapModuleList.end(); iter ++)
	{
		pModule = iter->first;
		hModule = iter->second;

		//释放插件内部的资源
		pModule->UnInit();

		//delete pModule;
		//pModule = NULL;

		////卸载该插件 
		//FREE_MODULE(hModule);
	}

	for(iter = m_mapModuleList.begin(); iter != m_mapModuleList.end(); iter ++)
	{
		pModule = iter->first;
		hModule = iter->second;

		////释放插件内部的资源
		//pModule->UnInit();

		delete pModule;
		pModule = NULL;

		//卸载该插件 
		FREE_MODULE(hModule);
	}


	m_mapModuleList.clear();

	return TRUE;
}

/************************************************************************************************************/
/*funtion:根据插件名称获取插件信息控制结构																	*/
/*parameter:strDllName 要获取插件控制结构的插件的名称														*/
/*return: 成功则返回所要的插件控制结构的指针,失败则返回NULL													*/
/*author:hwh																								*/
/*time:2010.5.17-09:45																						*/
/************************************************************************************************************/
IModuleBasePtr CModuleManager::GetModule(const std::string strDllName)
{
	MapModuleList::iterator iter;
	IModuleBasePtr pModule = NULL;

	for(iter = m_mapModuleList.begin(); iter != m_mapModuleList.end(); iter ++)
	{
		pModule = iter->first;
		if(strDllName == pModule->GetModuleInfo().strName)
		{
			return pModule;
		}
	}

	return NULL;
}

/************************************************************************************************************/
/*funtion:获取插件控制结构map,并返回map中加载的插件的个数													*/
/*parameter:ModuleStruInfoMap 用于被填充的插件信息控制结构mao												*/
/*return: 成功则返回map中加载的插件的个数																	*/
/*author:hwh																								*/
/*time:2010.5.17-10:52																						*/
/************************************************************************************************************/
INT32  CModuleManager::GetModuleList(VectModuleList& vectModuleList)
{
	INT32 iRet = 0;
	MapModuleList::iterator iter;

	for(iter = m_mapModuleList.begin(); iter != m_mapModuleList.end(); iter ++)
	{
		vectModuleList.push_back(iter->first);
		
		iRet++;
	}

	return iRet;
}

