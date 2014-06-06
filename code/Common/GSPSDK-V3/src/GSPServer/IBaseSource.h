/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : IBASESOURCE.H
Author :  zouyx
Version : 0.0.0.0
Date: 2012/3/19 10:22
Description: 数据源的基本接口类
********************************************
*/

#ifndef _GS_H_IBASESOURCE_H_
#define _GS_H_IBASESOURCE_H_

#include "GSPObject.h"
#include "ISource.h"
#include "MediaInfo.h"
#include "List.h"
#include "GSPMemory.h"
#include "ThreadPool.h"
#include "MainLoop.h"
#include <list>

namespace GSP
{
	class CRefSource;
	class CServer;

	class CIBaseSource : public CISource
	{
	public :
		virtual void SetTransMode( INT32 eMode ) = 0;	
		virtual EnumErrno Ctrl(const StruGSPCmdCtrl &stCtrl) = 0;
		virtual EnumErrno GetPlayStatus( StruPlayStatus *pStatus ) = 0;
		virtual CRefSource *RefSource(const CIMediaInfo *pMediaInfo ) = 0;
		virtual void UnrefSource(CRefSource *p) = 0;
		virtual  INT32 GetCtrlAbilities(void) = 0;
		virtual const CIMediaInfo &MediaInfo(void) = 0;
		

	};

	//流转换器
	class CIStreamConverter 
	{
	protected :
		BOOL m_bWaitKey;     //等待关键帧	
		BOOL m_bEnableSysHeader; //需要发送帧头
		BOOL m_bQuickFrame; //快速帧头
		BOOL m_bRequestFrame;
	public :
		CIStreamConverter(void)
		{
			m_bWaitKey = TRUE;
			m_bEnableSysHeader = TRUE;
			m_bQuickFrame = TRUE;
		}

		INLINE BOOL WaitSysHeader(void)
		{
			return (m_bEnableSysHeader);
		}

		INLINE void EnableWaitSysHeader( BOOL bEnable )
		{		
			m_bEnableSysHeader = bEnable;
		}

		INLINE void EnableWaitKeyFrame( BOOL bEnable )
		{		
			m_bWaitKey = bEnable;
		}

		INLINE void EnableQuickFrame( BOOL bEnable )
		{
			m_bQuickFrame = bEnable;
		}

		INLINE BOOL IsQuickFrame(void)
		{
			return m_bQuickFrame;
		}

		INLINE BOOL IsRequestStream(void)
		{
			return m_bRequestFrame;
		}

		virtual EnumErrno Init(const CIMediaInfo *pInputInfo,
								const CIMediaInfo *pOutInfo) = 0;
		virtual const CIMediaInfo *GetInputMediaInfo(void) const = 0;
		virtual const CIMediaInfo *GetOutputMediaInfo(void) const = 0;
		virtual EnumErrno InputStreamFrame( CFrameCache *pFrame ) = 0;
		virtual void AddRefSourceListener( CRefSource *pSource ) = 0;
		virtual void RemoveRefSourceListener( CRefSource *pSource ) = 0;
	
		
	};


class CBaseSource :
	public CGSPObject, public CIBaseSource
{
private :
	static GSAtomicInter s_iAutoIDSequence; //自增长ID 序列
public :
	CServer *m_pServer;
protected :		
	const UINT32 m_iAutoID;
	CMediaInfo m_csMediaInfo;
	
	typedef std::list<CRefSource*> CListListener;
	CListListener m_csListener;  //监听者  ,对象为 CRefSource *

	UINT16 m_iSourceID;
	UINT32 m_iAbiliteMasrk;
	BOOL m_bEnableRef;
	INT m_eTransModel;
	CList m_csStreamCache; //数据缓冲区, 对象为 CSliceFrame *
	
	CGSPString m_strKey;
	CGSWRMutex m_csWRMutex;
	typedef std::vector<CFrameCache*> CFrameCacheRow;
	typedef std::vector<CFrameCacheRow> CFrameTable;
	CFrameTable m_csSysFrameCache; //系统头缓冲	
	BOOL m_bValid; //有效
	StruPlayStatus m_stPlayStatus;

	CGSPThreadPool m_csStreamTask; //异步处理流

	BOOL m_bRelease; //释放已经释放


	CISource::EnumMode m_eMode;

	CISource::FuncPtrISourceEventCallback m_fnCallback;
	void *m_pUserArgs;
private :
	void *m_pUserData;
	GSAtomicInter m_iObjectRefs;
	GSAtomicInter m_iTestVWrite;
	CWatchTimer *m_pPullTimer;
	BOOL m_bReleased;
	CGSMutex m_mutexRelease;
public :

	virtual CISource::EnumMode GetMode(void) const;

	virtual void SetEventCallback(CISource::FuncPtrISourceEventCallback fnCallback, 
		void *pUserArgs);

	virtual BOOL EnablePullEvent( BOOL bStart, INT iMSecs );


	virtual CISource * RefObject(void);

	virtual void UnrefObject(void);

	virtual UINT GetSrcRefs(void); 

	virtual UINT32 GetAutoID(void);

	virtual UINT16 GetSourceID(void);

	virtual void SetMediaInfo( UINT iChn,const StruGSMediaDescri *pInfo );

	virtual void SetCtrlAbilities(UINT32 iAbilities);

	virtual void SourceEnableRef(BOOL bEnable );

	virtual void SourceEnableValid( BOOL bEnable);

	virtual CIUri *MakeURI( const char *czPro = "gsp",const char *pRemoteIP=NULL );

	virtual void SetPlayStatus( const StruPlayStatus *pStatus );

	virtual void ReplayEnd(void);

	virtual CISource::EnumRetNO WriteData( const void *pData, INT iLen, UINT iChn, BOOL bKey); 


	virtual CISource::EnumRetNO WriteSysHeader( const void *pData, INT iLen, UINT iChn);


	virtual CISource::EnumRetNO WriteDataV( const StruIOV *pIOV, INT iVNums, UINT iChn, BOOL bKey); 

	virtual CISource::EnumRetNO WriteSysHeaderV( const StruIOV *pIOV, INT iVNums, UINT iChn);


	/*
	*********************************************
	Function :SetUserData
	DateTime : 2010/8/6 8:56
	Description :设置用户数据
	Input : pData 用户的数据
	Output :
	Return :
	Note : 只是做指针的存储,如果需要释放操作， 由用户自行处理
	*********************************************
	*/
	virtual void SetUserData(void *pData);

	/*
	*********************************************
	Function : GetUserData
	DateTime : 2010/8/6 8:57
	Description : 返回用户数据
	Input :
	Output : 
	Return :  返回  SetUserData 设定的值 
	Note : 如果没有调用过 SetUserData， 默认值为NULL
	*********************************************
	*/
	virtual void *GetUserData(void );


	virtual void Release(void);

	virtual const char *GetKey(void);


	virtual void SetTransMode( INT32 eMode );	

	virtual EnumErrno Ctrl(const StruGSPCmdCtrl &stCtrl) ;

	virtual EnumErrno GetPlayStatus( StruPlayStatus *pStatus );

	virtual CRefSource *RefSource(const CIMediaInfo *pMediaInfo );

	virtual void UnrefSource(CRefSource *p);

	virtual  INT32 GetCtrlAbilities(void);



	INT GetRefCounts(void) const
	{
		return m_iObjectRefs;
	}

	
	void SetKeyInfo( const char *szKey, UINT16 iSourceID);

	virtual const CIMediaInfo &MediaInfo(void)
	{
		return m_csMediaInfo;
	}

// 	INLINE INT32 GetTransModel(void)
// 	{
// 		return m_eTransModel;
// 	}

	static CBaseSource *Create( CServer *pServer, CISource::EnumMode  eMode = CISource::eMODE_PUSH)
	{
		return new CBaseSource(pServer,eMode);
	}

// 	void *operator new(size_t iSize)
// 	{
// 		return ::malloc(iSize);
// 	}
// 	void operator delete(void *pBuffer)
// 	{
// 		::free(pBuffer);
// 	}
private :
	CBaseSource(CServer *pServer, CISource::EnumMode  eMode = CISource::eMODE_PUSH);
	virtual ~CBaseSource(void);

	 CISource::EnumRetNO  NotifyRefSource( CFrameCache *pFrame);
	 CISource::EnumRetNO  NotifyRefSource( INT eRefSourceEvent, void *pParam);

	 //处理流人物线程回调
	 void OnStreamTaskPoolEvent( CObjThreadPool *pTkPool, void *pData );

	 void OnPullTimerEvent(  CWatchTimer *pTimer  );

};

} //end namespace GSP

#endif //end _GS_H_IBASESOURCE_H_
