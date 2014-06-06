#include "JournalIf.h"
#include "Service.h"

using namespace JOU;


typedef struct _StruJouError
{
	EnumJouErrno eErrno;
	CGSString strError;
}StruJouError;


JOU_API const char * JouGetError( EnumJouErrno eErrno)
{
static StruJouError s_vError[] =
{
	{eJOU_E_UNKNOWN, _GSTX("未定义的错误类型") }, //放在最前面


	{eJOU_R_SUCCESS, _GSTX("成功") },
	{eJOU_E_UNKNOWN, _GSTX("未知错误") },
	{eJOU_E_NINIT, _GSTX("没有初始化") },
	{eJOU_E_NJOUTYPE, _GSTX("不存在的日志类型") },
	{eJOU_E_INVALID, _GSTX("非法参数") },
	{eJOU_E_NFUNCTION, _GSTX("不支持该功能") },
	{eJOU_E_BUSY, _GSTX("系统繁忙")},
	{eJOU_E_NMEM, _GSTX("系统资源匮乏")},

	


	{eJOU_E_DB_MODULE, _GSTX("初始化数据库模块失败") },
	{eJOU_E_DB_CONNPOOL, _GSTX("初始化数据库连接池失败") },
	{eJOU_E_DB_GETCONN, _GSTX("获取数据库连接失败") },
	{eJOU_E_DB_EXESQL, _GSTX("数据库执行SQL错误") },
	{eJOU_E_DB_ASSERT, _GSTX("数据库执行发送异常") },
};
	for( int i = 1; i<ARRARY_SIZE(s_vError); i++ )
	{
		if( s_vError[i].eErrno == eErrno )
		{
			return s_vError[i].strError.c_str();
		}
	}
	return s_vError[0].strError.c_str();;
}


static CService *s_pSrv = NULL;

#define s_gSrv (*s_pSrv)
static GSAtomicInter s_iInit = 0;


JOU_API EnumJouErrno  JouModuleInit( const char *czConfFilename,void* pConnectionPoolArgs)
{
EnumJouErrno eRet;
	
	if( !AtomicInterCompareExchange(s_iInit, 0, 1) )
	{
		return eJOU_R_SUCCESS;
	}
	CGSThreadPool::InitModule();
	s_pSrv = new CService();
	if( s_pSrv == NULL )
	{
		MY_ERROR("Service Init fail.\n");
		GS_ASSERT(0);
		AtomicInterSet(s_iInit, 0);
		return eJOU_E_NMEM;
	}
	eRet = s_gSrv.Init( czConfFilename, pConnectionPoolArgs);
	if( eRet )
	{
		MY_ERROR("Service Init fail.\n");
		GS_ASSERT(0);
		AtomicInterSet(s_iInit, 0);
	}


	return eRet;
}


JOU_API void  JouModuleUnint(void)
{
	if( !AtomicInterCompareExchange(s_iInit, 1, 0) )
	{
		return ;
	}
	if( s_pSrv )
	{
		s_gSrv.Uninit();
		delete s_pSrv;
	}
	CGSThreadPool::UninitModule();
}


JOU_API EnumJouErrno JouAdd( const StruJournalInfo *pLog)
{
	GS_ASSERT_RET_VAL( s_iInit, eJOU_E_NINIT);
	return s_gSrv.m_csWriter.JouAdd(pLog);
}


JOU_API EnumJouErrno  JouDelete(  EnumJournalType eType, INT64 iKeyID)
{
	GS_ASSERT_RET_VAL( s_iInit, eJOU_E_NINIT);
	return s_gSrv.m_csWriter.JouDelete(eType, iKeyID);
}

JOU_API EnumJouErrno  JouDeleteExt( const char czFuncName[JOU_FUNC_NAME_LEN],
								   const char czArgs[JOU_ARG_LEN] )
{
	GS_ASSERT_RET_VAL( s_iInit, eJOU_E_NINIT);
	return s_gSrv.m_csWriter.JouDeleteExt(czFuncName, czArgs);
}




JOU_API EnumJouErrno  JouUpdateOperation( const StruJouOperationUpdate *pData )
{
	GS_ASSERT_RET_VAL( s_iInit, eJOU_E_NINIT);
	return s_gSrv.m_csWriter.JouUpdateOperation(pData);
}



JOU_API EnumJouErrno  JouAsyncQuery( const StruQueryArgs *pArgs )
{
	GS_ASSERT_RET_VAL( s_iInit, eJOU_E_NINIT);
	return s_gSrv.m_csQuery.JouAsyncQuery(pArgs);
}


JOU_API EnumJouErrno  JouSetAsyncQueryCallback( JouFuncPtrAsyncQueryCallback fnCallback, 
											   void *pUserContent )
{
	GS_ASSERT_RET_VAL( s_iInit, eJOU_E_NINIT);
	return s_gSrv.m_csQuery.JouSetAsyncQueryCallback(fnCallback, pUserContent);
}
