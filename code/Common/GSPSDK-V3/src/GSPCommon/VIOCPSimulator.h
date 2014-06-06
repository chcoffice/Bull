/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : VIOCPSIMULATOR.H
Author :  zouyx
Version : 0.0.0.0
Date: 2012/4/11 11:28
Description: 模拟IOCP 接口类
********************************************
*/

#ifndef _GS_H_VIOCPSIMULATOR_H_
#define _GS_H_VIOCPSIMULATOR_H_

#include "IAsyncIO.h"
#include "BTree.h"
#include "ThreadPool.h"



namespace GSP
{

#define SKEVT_PIPE_BUFF_SIZE 256

#define PIPE_FD_INDEX       0xFFFF
#define SK_INDEX_SEQ_BEGIN  1
#define GET_FD_INDEX_INDEX(x)  ((UINT16)((x)&0xFFFF))
#define GET_FD_INDEX_SEQ(x)   ((UINT16)((x)>>16)&0xFFFF)
#define INVALID_PRIVATE_DATA  0x0L


	

typedef   UINT32 EnumSocketEventMask;
#define eEVT_SOCKET_MASK_NONE  0
#define eEVT_SOCKET_MASK_READ  0x01
#define eEVT_SOCKET_MASK_WRITE  0x02
#define eEVT_SOCKET_MASK_WR  0x03

	
class CSkEvtClient;

class CVIOCPSimulator :
      public CIAsyncIO
{  
public :
	friend CSkEvtClient;
	typedef struct _StructSimEvents
	{
		SOCKET fd;
		EnumSocketEventMask iEvtMask;
		_StructSimEvents(void) : fd(INVALID_SOCKET), iEvtMask(0)
		{

		}
	}StructSimEvents;

	class CSimEventsContains
	{
	private :
		StructSimEvents *m_vEvts;
		UINT m_iMaxEvts;
		UINT m_iCurEvts;
	public :
		CSimEventsContains(void) : m_vEvts(NULL),m_iMaxEvts(0),m_iCurEvts(0)
		{

		}
		~CSimEventsContains(void)
		{
			if( m_vEvts )
			{
				CMemoryPool::Free(m_vEvts);
				m_vEvts = NULL;
			}
		}

		BOOL Init( UINT iSize )
		{
			m_vEvts = (StructSimEvents *) CMemoryPool::Malloc(sizeof(StructSimEvents)*iSize);
			if( m_vEvts )
			{
				m_iMaxEvts = iSize;
				return TRUE;
			}
			return FALSE;
		}

		INLINE void Reset(void)
		{
			m_iCurEvts = 0;
		}

		INLINE EnumErrno Add( StructSimEvents &vResult )
		{
			if( m_iCurEvts<m_iMaxEvts )
			{
				memcpy(&m_vEvts[m_iCurEvts], &vResult, sizeof( vResult) );
				m_iCurEvts++;
				return eERRNO_SUCCESS;
			}
			GS_ASSERT(0);
			return eERRNO_SYS_EFLOWOUT;
		}

		INLINE UINT Size(void) const
		{
			return m_iCurEvts;
		}

		INLINE StructSimEvents *Get(UINT iIndex)
		{
			GS_ASSERT(iIndex<m_iCurEvts);
			return &m_vEvts[iIndex];
		}

		CLASS_NO_COPY(CSimEventsContains)
	};


private :
	SOCKET m_hPipe[2];
	unsigned char m_bPipeBuffer[SKEVT_PIPE_BUFF_SIZE];
	BOOL m_bRunning;
	CGSPThreadPool m_csWatchTaskPool;	
	UINT m_iMaxChannels;	
	CSimEventsContains m_vEvtContains;
protected :
	typedef std::map<SOCKET, CISocket *> CMapOfSocket;
	CMapOfSocket m_mapChannels;
	CGSWRMutex m_csWRMutexChn;
public :       
	
	//以下为CIAsyncIO 接口
	virtual void Unregister( CISocket *pSocket );  
	virtual EnumErrno AsyncRcvFrom(CISocket *pSocket,BOOL bStart);
	virtual EnumErrno AsyncRcv(CISocket *pSocket,	BOOL bStart);    
	virtual EnumErrno AsyncAccept(CISocket *pSocket,BOOL bStart);   
	virtual EnumErrno AsyncSend(CISocket *pSocket, void *pKey, std::vector<CProPacket *> vProPacket );
	virtual EnumErrno AsyncSend(CISocket *pSocket, void *pKey,std::vector<CProFrame *> vProFrame);

protected : 
	CVIOCPSimulator(INT iID, UINT iMaxChannels);
	virtual ~CVIOCPSimulator(void);


	//以下为CIAsyncIO 接口
	virtual INT  GetMaxChannelContain(void);
	virtual INT  GetEmptyCounts(void);      
	virtual BOOL Init(void);
	virtual void Uninit(void);
	virtual BOOL Register(CISocket *pSocket, CNetError &csError);

	//以下为新增接口	

	//有本类保证线程安全
	virtual EnumErrno Watch( SOCKET fd , BOOL bWouldWakeup,
						void **ppPriData, FuncPtrFree *fnFreePri ) = 0;
	virtual void      UninitWatch(SOCKET fd, BOOL bWouldWakeup ) = 0;	

	//由子类保证线程安全
    virtual EnumErrno AddWatch( SOCKET fd, EnumSocketEventMask iEvtMask ,
													BOOL bWouldWakeup ) = 0;	
	virtual void	  CleanWatch( SOCKET fd, EnumSocketEventMask iEvtMask,
													BOOL bWouldWakeup  ) = 0;	

	virtual EnumErrno WaitEvent( CSimEventsContains &vResult  ) = 0;

	static void *GetChildClassPriData(CISocket *pSocket );


	BOOL Wakeup(void);

	SOCKET GetPipeReadSocket(void) const;
private :
	void OnWatchTaskEvent(CObjThreadPool *pThread, void *pArg );
	BOOL CreatePipe(void);

};




} //end namespace GSP

#endif //end _GS_H_VIOCPSIMULATOR_H_
