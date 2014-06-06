/*
******************************************
Copyright (C), 2010-2011, GOSUN
Filename : REFSOURCE.H
Author :  zouyx
Version : 0.1.0.0
Date: 2010/7/21 14:46
Description: CIRefSource 的实现
********************************************
*/
#ifndef GSS_REFSOURCE_DEF_H
#define GSS_REFSOURCE_DEF_H

#include "GSPObject.h"





namespace GSP
{
class CIBaseSource;
class CBaseSource;

class CRefSource :
    public  CRefObject
{
public :
	typedef enum
	{
		eEVT_STREAM_FRAME = 0, //流数据,参数 CFrameCache *
		eEVT_PLAY_STATUS, //播放状态数据, 参数 const StruPlayStatus *
		eEVT_SOURCE_RELEASE, //数据源被释放, 参数NULL
		eEVT_SOURCE_ASSERT, //数据源异常, 参数NULL
		eEVT_PLAY_END, //播放完成, 参数NULL
	}EnumRefSourceEvent;
	typedef EnumErrno (CGSPObject::*FuncPtrRefSourceEvent)(CRefSource *pRefSource,
									EnumRefSourceEvent eEvt, void *pParam);

protected :
	
    CBaseSource *m_pSrc;
    BOOL m_bWaitKey;     //等待关键帧
    BOOL m_bStart;      //对象是否已经启动
    BOOL m_bPlay;       //是否在播放状态
    UINT8 m_bEnableSysHeader; //需要发送帧头
	BOOL m_bQuickFrame; //快速帧头

    CGSPObject *m_pFnEvtOwner;
    FuncPtrRefSourceEvent m_fnEvtCallback;
	INT32 m_eTranModel;
	INT m_iFlowctrl;
private :
	void *m_pSourcePrivateData; //数据源私有数据
	UINT8 m_iDelayCounts;
public :

    

	INLINE void *GetSourcePrivate(void) const
	{
		return m_pSourcePrivateData;
	}

	INLINE void SetSourcePrivate( void *pData )
	{
		m_pSourcePrivateData = pData;
	}

	BOOL Start(void);
	void Stop(void);
	CIBaseSource *Source(void);
	void SetTransMode( INT32 eMode );	
	EnumErrno Ctrl(const StruGSPCmdCtrl &stCtrl);  
	void SetListener( CGSPObject *pFnEvtOwner, FuncPtrRefSourceEvent fnEvtCallback );
    INT32 GetCtrlAbilities(void);
  
    BOOL WaitSysHeader(void) ;

    INLINE void EnableWaitSysHeader( BOOL bEnable )
    {		
		if( bEnable )
		{
			m_bEnableSysHeader = 1;
		}
		else
		{
			m_bEnableSysHeader = 0;
		}
    }

    INLINE void EnableWaitKeyFrame( BOOL bEnable )
    {		
        m_bWaitKey = bEnable;
	}

	INLINE void EnableQuickFrame( BOOL bEnable )
	{
		m_bQuickFrame = bEnable;
	}

	INLINE BOOL IsQuickFrame(void)  const
	{
		return m_bQuickFrame;
	}

    INLINE BOOL IsRequestStream(void)  const
    {
        return m_bPlay && m_bStart;
    }
    virtual EnumErrno  OnSourceEvent( EnumRefSourceEvent eEvt, void *pParam);

	
	static CRefSource *Create(CBaseSource *pSource)
	{
		return new CRefSource(pSource);
	}

	void Release(void);
private :
	CRefSource(CBaseSource *pSource );
	virtual ~CRefSource(void);   

};




};

#endif
