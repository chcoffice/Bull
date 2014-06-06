#ifndef GSS_SERVER_DEF_H
#define GSS_SERVER_DEF_H

#include "IServer.h"
#include "GSPObject.h"
#include "Uri.h"
#include "IMediaInfo.h"
#include "List.h"



namespace GSP
{
#define MAX_SOURCE_MANAGER 3000

	class CRefSource;
	class CIProServer;
	class CLog;
	class CBaseSource;

	typedef enum
	{
		eEVT_PROSRV_ACCEPT, //接受到新连接, pEvtArg 为 CISrvSession *
		eEVT_PROSRV_SESSION_RELEASE, // 会话被释放， pEvtArg 为 CISrvSession *
	}EnumProServerEvent;

	typedef enum
	{
		eEVT_SRC_REF,   // pEvtParam == NULL
		eEVT_SRC_UNREF, // pEvtParam == NULL
		eEVT_SRC_RELEASE, // pEvtParam == NULL
		eEVT_SRC_CTRL, //控制事件 pEvtParam ==   StruGSPCmdCtrl *
		eEVT_SRC_FRAME, //帧数据 pEvtParam == CFrame *
		eEVT_SRC_REQST, //获取状态， pEvtParam == NULL
	}EnumSourceEvent;


	class CISecurity : public CGSPObject
	{
	public :
		virtual CGSPString &GetSecurityKey( const char *szSourceKey ) = 0;
		virtual EnumErrno  SecurityCheck( const char *szSecurityKey, const char *szSourceKey ) = 0;
	};

	
class CServer :
    public CGSPObject, public CIServer
{
    
private:
	const static int s_iMaxServerManageSource = 3000;
    CLog *m_pLog;
    CGSMutex m_csMutex;
	
	CGSWRMutex m_csSrcWRMutex; //数据源锁
	std::vector<CBaseSource *> m_vSource; //能管理的数据源
	typedef std::map<CGSString, CBaseSource *> CSourceMap; //有效的数据源
	CSourceMap m_mSource;
	CConfigFile m_csConfig;
	typedef std::map<EnumProtocol, CIProServer *> CProServerModuleMap;
	CProServerModuleMap m_mIServer;
	
	void *m_pEventFnParam;       //事件回调参数
	GSPServerEventFunctionPtr m_fnOnEvent; //事件回调

	CISecurity *m_pSecurityCenter;
	INT m_bDestroying;

	CList m_csSessionList;   //连接的Session 列表
	CGSWRMutex m_csSessionMutex; // m_csSessionList 锁



public :
    CServer(void);
    virtual ~CServer(void);

	void *operator new(size_t iSize)
	{
		return ::malloc(iSize);
	}

	void operator delete(void *pBuffer)
	{
		::free(pBuffer);
	}

	/*
	*********************************************
	Function :  InitLog 
	DateTime : 2010/6/11 16:19
	Description : 设置日志数据接口
	Input :   czPathName 设置日志接口的路径
	Output :  
	Return :  
	Note :
	*********************************************
	*/
	virtual void InitLog( const char *czPathName );


	virtual CIUri *CreateEmptyUriObject(void);

	/*
	*********************************************
	Function : Init
	DateTime : 2010/6/11 16:22
	Description :  启动服务
	Input :
	Output :
	Return : TRUE/FALSE
	Note :
	*********************************************
	*/
	virtual BOOL Init( const char *csConfigFilename,const char *csLogPathName);

	/*
	*********************************************
	Function : Stop
	DateTime : 2010/6/11 16:22
	Description :  停止服务
	Input :
	Output :
	Return : TRUE/FALSE
	Note :
	*********************************************
	*/
	virtual BOOL Stop(void);


	/*
	*********************************************
	Function :   FindSource
	DateTime : 2010/6/11 16:27
	Description :  用键值来查找数据源
	Input : czKey 要查找的数据源键值
	Output :
	Return :  不存在返回NULL
	Note :
	*********************************************
	*/
	virtual CISource *FindSource(const char *csKey);

	virtual CISource *FindSource(UINT16 iSourceIndex );


	/*
	*********************************************
	Function :   AddPushSource
	DateTime : 2010/6/11 16:27
	Description :  添加推模式数据源
	Input : szKey 数据源键值
	Output :
	Return : 返回建立的推模式数据源， 失败返回NULL
	Note :
	*********************************************
	*/
	virtual CISource *AddSource( const char *szKey );

	virtual CISource *AddPullSource( const char *czKey );




	virtual CISource::EnumRetNO WriteData( UINT16 iSourceIndex,  
		const void *pData, INT iLen, UINT iChn, BOOL bKey); 


	virtual CISource::EnumRetNO WriteSysHeader( UINT16 iSourceIndex, 
		const void *pData, INT iLen, UINT iChn) ;

	virtual CISource::EnumRetNO WriteDataV( UINT16 iSourceIndex, 
		const StruIOV *pIOV, INT iVNums, UINT iChn, BOOL bKey); 


	virtual CISource::EnumRetNO WriteSysHeaderV( UINT16 iSourceIndex, 
		const StruIOV *pIOV, INT iVNums, UINT iChn);


	/*
	*********************************************
	Function : SetEventCallback
	DateTime : 2010/6/11 16:32
	Description :  设置服务器器事件回调
	Input :   fnOnEvent 回调函数
	Input : pParam  回调的用户参数
	Output :
	Return : 
	Note :
	*********************************************
	*/
	virtual void SetEventCallback( GSPServerEventFunctionPtr fnOnEvent, void *pParam);



	virtual void InitURI( CIUri &csRet, const char *czKey, const char *czPro = "gsp", const char *szRemoteIP = NULL);

	/*
	*********************************************
	Function : QueryStatus
	DateTime : 2012/3/13 15:38
	Description :  状态查询
	Input :  eQueryStatus 指定查询的选项
	Output : ppResult 返回的数据结果
	Output : pResultSize 结果的大小
	Return : 
	Note :   结果需要调用 FreeQueryStatusResult 释放
	*********************************************
	*/
	virtual BOOL QueryStatus(const EnumGSPServerStatus eQueryStatus, 
		char **ppResult,INT *pResultSize  );

	/*
	*********************************************
	Function : FreeQueryStatusResult
	DateTime : 2012/3/13 15:44
	Description :  释放查询的结果集
	Input :  
	Output : 
	Return : 
	Note :   
	*********************************************
	*/
	virtual void FreeQueryStatusResult( char *pResult);



	virtual BOOL GetOptions(CGSPServerOptions &Opts) const
	{
		return TRUE;
	}

	virtual BOOL SetOptions(const CGSPServerOptions &csOptions)
	{
		return TRUE;
	}


	/*
	*********************************************************************
	*
	*@brief : 为内部接口
	*
	*********************************************************************
	*/
	EnumErrno RequestSource(const CUri &csUri,const CIMediaInfo &csRquestMedia , INT eTransModel,
							CRefSource **ppResult);

	void  OnProServerEvent(CIProServer *pServer, EnumProServerEvent eEvt, void *pEvtArg);

	void  *OnSourceEvent(CBaseSource *pSource, EnumSourceEvent eEvt, void *pEvtArg);	

	INLINE CConfigFile &GetConfig(void)
	{
		return m_csConfig;
	}

private :
	BOOL InitProModule(void);
	void *ProcessServerEvent(  CISource *pSource,EnumGSPServerEvent eEvent, 
		void *pEventPararms, INT iLen);

	CGSString GetRouteIP(const char * czRemoteIP, CIProServer *pServer );


	void RemoveSource( CISource *pSource);


	BOOL RequestOutSource(void);

	

	CISource *AddSource( const char *szKey, CISource::EnumMode eMode );

};


};

#endif
