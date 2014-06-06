/**************************************************************************************************
* Copyrights 2013  高新兴
*                  基础应用组
* All rights reserved.
*
* Filename：
*       OciDllDeal.h
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
#ifndef OCI_DLL_DEAL_H
#define OCI_DLL_DEAL_H


#include "ocilib.h"
//namespace	DBAccessModule
//{
typedef unsigned int OCI_API _D_OCI_ErrorGetType(OCI_Error *err);
typedef const mtext * OCI_API _D_OCI_ErrorGetString(OCI_Error *err);
typedef  OCI_Error * OCI_API _D_OCI_GetLastError(void);
typedef  OCI_Error * OCI_API _D_OCI_GetBatchError(OCI_Statement *stmt);
typedef  int OCI_API _D_OCI_ErrorGetOCICode(OCI_Error *err);


typedef  boolean OCI_API _D_OCI_StatementFree(OCI_Statement *stmt);
typedef  boolean OCI_API _D_OCI_ReleaseResultsets(OCI_Statement *stmt);
typedef  boolean OCI_API _D_OCI_ConnectionFree(OCI_Connection *con);
typedef  boolean OCI_API _D_OCI_Cleanup(void);


typedef boolean  OCI_API _D_OCI_Initialize(POCI_ERROR err_handler,const mtext *lib_path,unsigned int mode);
typedef  boolean OCI_API _D_OCI_ExecuteStmt(OCI_Statement *stmt,const mtext *sql);
typedef  OCI_Connection * OCI_API _D_OCI_ConnectionCreate(const mtext *db,const mtext *user,const mtext *pwd,unsigned int mode);
typedef  OCI_Resultset * OCI_API _D_OCI_GetResultset(OCI_Statement *stmt);
typedef  OCI_Statement * OCI_API _D_OCI_StatementCreate(OCI_Connection *con);
typedef  unsigned int OCI_API _D_OCI_LobRead(OCI_Lob *lob,void *buffer,unsigned int len);
typedef  boolean _D_OCI_PrepareFmt(OCI_Statement *stmt,const mtext   *sql,...);
typedef  boolean OCI_API _D_OCI_Prepare(OCI_Statement *stmt,const mtext   *sql);
typedef  boolean OCI_API _D_OCI_Execute(OCI_Statement *stmt);
typedef  boolean OCI_API _D_OCI_Commit(OCI_Connection *con);

typedef  boolean OCI_API _D_OCI_BindString(OCI_Statement *stmt,const mtext *name,dtext *data,unsigned int len);
typedef  boolean OCI_API _D_OCI_BindInt(OCI_Statement *stmt,const mtext *name,int *data);

typedef  boolean OCI_API _D_OCI_BindDate(OCI_Statement *stmt,const mtext   *name,OCI_Date *data);
typedef  boolean OCI_API _D_OCI_DateSetDateTime(OCI_Date *date,int year,int month,int day,int hour,int min,int sec);
typedef  OCI_Date * OCI_API _D_OCI_DateCreate(OCI_Connection *con);


typedef  boolean OCI_API _D_OCI_FetchNext(OCI_Resultset *rs);
typedef  boolean OCI_API _D_OCI_FetchLast(OCI_Resultset *rs);
typedef  boolean OCI_API _D_OCI_Rollback(OCI_Connection *con);

typedef  unsigned int OCI_API _D_OCI_GetColumnCount(OCI_Resultset *rs);
typedef  OCI_Column * OCI_API _D_OCI_GetColumn(OCI_Resultset *rs,unsigned int index);
typedef  const mtext * OCI_API _D_OCI_ColumnGetName(OCI_Column *col);
typedef  boolean OCI_API _D_OCI_IsNull2(OCI_Resultset *rs,const mtext   *name);
typedef  const dtext * OCI_API _D_OCI_GetString2(OCI_Resultset *rs,const mtext   *name);
typedef  unsigned int OCI_API _D_OCI_GetUnsignedInt2(OCI_Resultset *rs,const mtext   *name);
typedef  OCI_Lob * OCI_API _D_OCI_GetLob2(OCI_Resultset *rs,const mtext   *name);
typedef  big_uint OCI_API _D_OCI_LobGetLength(OCI_Lob *lob);
typedef  int OCI_API _D_OCI_GetInt(OCI_Resultset *rs,unsigned int   index);

class OciDllDeal
{
	public:

		OciDllDeal(void);

		~OciDllDeal(void);
	public:
		BOOL Load_Oci(LPCTSTR lpFileName32,LPCTSTR lpFileName64);
		BOOL UnLoad_Oci(void);

	public:
		BOOL IsWow64(void);

	public:

		_D_OCI_Initialize *pOCI_Initialize;
		_D_OCI_ConnectionCreate *pOCI_ConnectionCreate;
		_D_OCI_StatementCreate *pOCI_StatementCreate;
		_D_OCI_GetResultset *pOCI_GetResultset;

		_D_OCI_StatementFree *pOCI_StatementFree;
		_D_OCI_ReleaseResultsets *pOCI_ReleaseResultsets;
		_D_OCI_ConnectionFree *pOCI_ConnectionFree;
		_D_OCI_Cleanup *pOCI_Cleanup;

		_D_OCI_GetBatchError *pOCI_GetBatchError;
		_D_OCI_GetLastError *pOCI_GetLastError;
		_D_OCI_ErrorGetType *pOCI_ErrorGetType;
		_D_OCI_ErrorGetString *pOCI_ErrorGetString;
		_D_OCI_ErrorGetOCICode *pOCI_ErrorGetOCICode;


		_D_OCI_ExecuteStmt *pOCI_ExecuteStmt;
		_D_OCI_PrepareFmt *pOCI_PrepareFmt;
		_D_OCI_Prepare *pOCI_Prepare;
		_D_OCI_Execute *pOCI_Execute;
		_D_OCI_Commit *pOCI_Commit;

		_D_OCI_BindString *pOCI_BindString;
		_D_OCI_BindInt *pOCI_BindInt;
		_D_OCI_BindDate *pOCI_BindDate;
		_D_OCI_DateSetDateTime *pOCI_DateSetDateTime;
		_D_OCI_DateCreate *pOCI_DateCreate;

		_D_OCI_FetchNext *pOCI_FetchNext;
		_D_OCI_FetchLast *pOCI_FetchLast;
		_D_OCI_Rollback *pOCI_Rollback;



		_D_OCI_GetColumnCount *pOCI_GetColumnCount;
		_D_OCI_GetColumn *pOCI_GetColumn;
		_D_OCI_ColumnGetName *pOCI_ColumnGetName;
		_D_OCI_IsNull2 *pOCI_IsNull2;
		_D_OCI_GetString2 *pOCI_GetString2;
		_D_OCI_GetUnsignedInt2 *pOCI_GetUnsignedInt2;
		_D_OCI_GetLob2 *pOCI_GetLob2;
		_D_OCI_LobGetLength *pOCI_LobGetLength;
		_D_OCI_LobRead *pOCI_LobRead;
		_D_OCI_GetInt *pOCI_GetInt;

	public:
		HINSTANCE hDLLDrv;
};
//}
#endif