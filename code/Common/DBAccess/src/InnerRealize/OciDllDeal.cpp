/**************************************************************************************************
* Copyrights 2013  高新兴
*                  基础应用组
* All rights reserved.
*
* Filename：
*       OciDllDeal.cpp
* Indentifier：
*
* Description：
*       DLL调用接口
* Author:
*       LiuHongPing
* Finished：
*
* History:
*       2013年05月，创建
*
**************************************************************************************************/

#include "OciDllDeal.h"
#include <tchar.h>


//using	namespace	DBAccessModule;

OciDllDeal::OciDllDeal(void)
{
		 pOCI_Initialize = NULL;
		 pOCI_StatementFree = NULL;
		 pOCI_ErrorGetType = NULL;
		 pOCI_ErrorGetString = NULL;

		 pOCI_GetBatchError = NULL;
		 pOCI_ReleaseResultsets = NULL;
		 pOCI_ExecuteStmt = NULL;
		 pOCI_GetLastError = NULL;
		 pOCI_ErrorGetOCICode = NULL;
		 pOCI_GetResultset = NULL;
		 pOCI_FetchNext = NULL;
		 pOCI_StatementCreate = NULL;
		 pOCI_FetchLast = NULL;
		 pOCI_GetColumnCount = NULL;
		 pOCI_GetColumn = NULL;
		 pOCI_ColumnGetName = NULL;

		 pOCI_IsNull2 = NULL;
		 pOCI_GetString2 = NULL;
		 pOCI_GetUnsignedInt2 = NULL;
		 pOCI_GetLob2 = NULL;
		 pOCI_LobGetLength = NULL;
		 pOCI_LobRead = NULL;
		 pOCI_PrepareFmt = NULL;
		 pOCI_Prepare = NULL;
		 pOCI_Execute = NULL;
		 pOCI_Commit = NULL;
		 pOCI_Cleanup = NULL;

		 pOCI_ConnectionCreate = NULL;
		 pOCI_ConnectionFree = NULL;
		 pOCI_Rollback = NULL;
		 pOCI_GetInt = NULL;

		 pOCI_BindString = NULL;
		 pOCI_BindInt = NULL;
		 pOCI_BindDate = NULL;
		 pOCI_DateSetDateTime = NULL;
		 pOCI_DateCreate = NULL;

		 hDLLDrv = NULL;
}

OciDllDeal::~OciDllDeal(void)
{
	if(hDLLDrv)
		FreeLibrary(hDLLDrv);
}
BOOL OciDllDeal::IsWow64(void) 
{ 
    typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL); 
    LPFN_ISWOW64PROCESS fnIsWow64Process; 
    BOOL bIsWow64 = FALSE; 
    fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress( GetModuleHandle(_T("kernel32")),"IsWow64Process"); 
    if (NULL != fnIsWow64Process) 
    { 
        fnIsWow64Process(GetCurrentProcess(),&bIsWow64);
    } 
    return bIsWow64; 
} 

BOOL OciDllDeal::Load_Oci(LPCTSTR lpFileName32,LPCTSTR lpFileName64)
{
	if( (lpFileName32 == NULL || lpFileName32 == _T("")) && (lpFileName64 == NULL || lpFileName64 == _T("")) )
		return FALSE;

	BOOL btitle;
	btitle = IsWow64();
	if (btitle)
	{
		hDLLDrv=LoadLibrary(lpFileName64);
		if(!hDLLDrv)
			return FALSE;
	}
	else
	{
		hDLLDrv=LoadLibrary(lpFileName32);
		if(!hDLLDrv)
			return FALSE;
	}
	if(!hDLLDrv)
		return FALSE;

		pOCI_Initialize=(_D_OCI_Initialize *)GetProcAddress(hDLLDrv,"_OCI_Initialize@12");
		pOCI_ConnectionCreate = (_D_OCI_ConnectionCreate *)GetProcAddress(hDLLDrv,"_OCI_ConnectionCreate@16");
		pOCI_StatementCreate = (_D_OCI_StatementCreate *)GetProcAddress(hDLLDrv,"_OCI_StatementCreate@4");
		pOCI_GetResultset = (_D_OCI_GetResultset *)GetProcAddress(hDLLDrv,"_OCI_GetResultset@4");

		if((pOCI_Initialize == NULL) || (pOCI_ConnectionCreate == NULL) || (pOCI_StatementCreate == NULL) || (pOCI_GetResultset == NULL))
		{
			FreeLibrary(hDLLDrv);
			hDLLDrv = NULL;
			return FALSE;
		}

		pOCI_StatementFree = (_D_OCI_StatementFree *)GetProcAddress(hDLLDrv,"_OCI_StatementFree@4");
		pOCI_ReleaseResultsets = (_D_OCI_ReleaseResultsets *)GetProcAddress(hDLLDrv,"_OCI_ReleaseResultsets@4");
		pOCI_ConnectionFree	 = (_D_OCI_ConnectionFree *)GetProcAddress(hDLLDrv,"_OCI_ConnectionFree@4");
		pOCI_Cleanup = (_D_OCI_Cleanup *)GetProcAddress(hDLLDrv,"_OCI_Cleanup@0");

		if((pOCI_StatementFree == NULL) || (pOCI_ReleaseResultsets == NULL) || (pOCI_ConnectionFree == NULL) || (pOCI_Cleanup == NULL))
		{
			FreeLibrary(hDLLDrv);
			hDLLDrv = NULL;
			return FALSE;
		}

		pOCI_GetBatchError = (_D_OCI_GetBatchError *)GetProcAddress(hDLLDrv,"_OCI_GetBatchError@4");
		pOCI_GetLastError = (_D_OCI_GetLastError *)GetProcAddress(hDLLDrv,"_OCI_GetLastError@0");
		pOCI_ErrorGetType = (_D_OCI_ErrorGetType *)GetProcAddress(hDLLDrv,"_OCI_ErrorGetType@4");
		pOCI_ErrorGetString	= (_D_OCI_ErrorGetString *)GetProcAddress(hDLLDrv,"_OCI_ErrorGetString@4");
		pOCI_ErrorGetOCICode = (_D_OCI_ErrorGetOCICode *)GetProcAddress(hDLLDrv,"_OCI_ErrorGetOCICode@4");

		if((pOCI_GetBatchError == NULL) || (pOCI_GetLastError == NULL) || (pOCI_ErrorGetType == NULL) || (pOCI_ErrorGetString == NULL)|| (pOCI_ErrorGetOCICode == NULL))
		{
			FreeLibrary(hDLLDrv);
			hDLLDrv = NULL;
			return FALSE;
		}

		pOCI_ExecuteStmt = (_D_OCI_ExecuteStmt *)GetProcAddress(hDLLDrv,"_OCI_ExecuteStmt@8");
		pOCI_PrepareFmt	= (_D_OCI_PrepareFmt *)GetProcAddress(hDLLDrv,"OCI_PrepareFmt");
		pOCI_Prepare	= (_D_OCI_Prepare *)GetProcAddress(hDLLDrv,"_OCI_Prepare@8");
		pOCI_Execute = (_D_OCI_Execute *)GetProcAddress(hDLLDrv,"_OCI_Execute@4");
		pOCI_Commit	= (_D_OCI_Commit *)GetProcAddress(hDLLDrv,"_OCI_Commit@4");

		if((pOCI_ExecuteStmt == NULL) || (pOCI_PrepareFmt == NULL) || (pOCI_Execute == NULL) || (pOCI_Commit == NULL) || (pOCI_Prepare == NULL))
		{
			FreeLibrary(hDLLDrv);
			hDLLDrv = NULL;
			return FALSE;
		}


		pOCI_FetchNext	= (_D_OCI_FetchNext *)GetProcAddress(hDLLDrv,"_OCI_FetchNext@4");
		pOCI_FetchLast	= (_D_OCI_FetchLast *)GetProcAddress(hDLLDrv,"_OCI_FetchLast@4");
		pOCI_Rollback	= (_D_OCI_Rollback *)GetProcAddress(hDLLDrv,"_OCI_Rollback@4");

		if((pOCI_FetchNext == NULL) || (pOCI_FetchLast == NULL) || (pOCI_Rollback == NULL))
		{
			FreeLibrary(hDLLDrv);
			hDLLDrv = NULL;
			return FALSE;
		}


		pOCI_GetColumnCount	= (_D_OCI_GetColumnCount *)GetProcAddress(hDLLDrv,"_OCI_GetColumnCount@4");
		pOCI_GetColumn	= (_D_OCI_GetColumn *)GetProcAddress(hDLLDrv,"_OCI_GetColumnCount@4");
		pOCI_ColumnGetName	= (_D_OCI_ColumnGetName *)GetProcAddress(hDLLDrv,"_OCI_ColumnGetName@4");
		pOCI_IsNull2 = (_D_OCI_IsNull2 *)GetProcAddress(hDLLDrv,"_OCI_IsNull2@8");

		if((pOCI_GetColumnCount == NULL) || (pOCI_GetColumn == NULL) || (pOCI_IsNull2 == NULL) || (pOCI_ColumnGetName == NULL))
		{
			FreeLibrary(hDLLDrv);
			hDLLDrv = NULL;
			return FALSE;
		}

		pOCI_GetString2	= (_D_OCI_GetString2 *)GetProcAddress(hDLLDrv,"_OCI_GetString2@8");
		pOCI_GetUnsignedInt2	= (_D_OCI_GetUnsignedInt2 *)GetProcAddress(hDLLDrv,"_OCI_GetUnsignedInt2@8");
		pOCI_GetLob2	= (_D_OCI_GetLob2 *)GetProcAddress(hDLLDrv,"_OCI_GetLob2@8");
		pOCI_LobGetLength	= (_D_OCI_LobGetLength *)GetProcAddress(hDLLDrv,"_OCI_LobGetLength@4");

		if((pOCI_GetString2 == NULL) || (pOCI_GetUnsignedInt2 == NULL) || (pOCI_GetLob2 == NULL) || (pOCI_LobGetLength == NULL))
		{
			FreeLibrary(hDLLDrv);
			hDLLDrv = NULL;
			return FALSE;
		}

		pOCI_LobRead	= (_D_OCI_LobRead *)GetProcAddress(hDLLDrv,"_OCI_LobRead2@16");
		pOCI_GetInt	= (_D_OCI_GetInt *)GetProcAddress(hDLLDrv,"_OCI_GetInterval@8");
		pOCI_BindString	= (_D_OCI_BindString *)GetProcAddress(hDLLDrv,"_OCI_BindString@16");
		pOCI_BindInt	= (_D_OCI_BindInt *)GetProcAddress(hDLLDrv,"_OCI_BindInt@12");

		if((pOCI_LobRead == NULL) || (pOCI_GetInt == NULL) || (pOCI_BindString == NULL) || (pOCI_BindInt == NULL))
		{
			FreeLibrary(hDLLDrv);
			hDLLDrv = NULL;
			return FALSE;
		}

		pOCI_BindDate	= (_D_OCI_BindDate *)GetProcAddress(hDLLDrv,"_OCI_BindDate@12");
		pOCI_DateSetDateTime	= (_D_OCI_DateSetDateTime *)GetProcAddress(hDLLDrv,"_OCI_DateSetDateTime@28");
		pOCI_DateCreate	= (_D_OCI_DateCreate *)GetProcAddress(hDLLDrv,"_OCI_DateCreate@4");

		if((pOCI_BindDate == NULL) || (pOCI_DateSetDateTime == NULL) || (pOCI_DateCreate == NULL))
		{
			FreeLibrary(hDLLDrv);
			hDLLDrv = NULL;
			return FALSE;
		}

	return TRUE;

}

BOOL OciDllDeal::UnLoad_Oci(void)
{
	if(hDLLDrv)
	{
		FreeLibrary(hDLLDrv);
		hDLLDrv = NULL;
		return TRUE;
	}
	return TRUE;

}