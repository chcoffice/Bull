/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : IOEPOLL.CPP
Author :  邹阳星
Version : 1.0.0.1
Date: 2013/4/1 15:23
Description: epoll 的异步IO 实现
********************************************
*/


#include "IAsyncIO.h"

#ifdef GSP_ASYNCIO_EPOLL

#include "VIOCPSimulator.h"
#include "Log.h"

#ifdef _LINUX

#   include <poll.h>
#   include <unistd.h>
#   include <netdb.h>
#   include <sys/socket.h>
#   include <arpa/inet.h>
#   include <unistd.h>
#   include <sys/time.h>
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <time.h>
#   include <fcntl.h>
#   include <poll.h>
#   include <sys/epoll.h>  




#else

 struct epoll_event
 {
	int events;
	union
	{
		void *ptr;
	}data;
 };

#define EPOLLERR 1
#define EPOLLET  1
#define EPOLLIN 1
#define EPOLLOUT 1

#define EPOLL_CTL_ADD 1
#define EPOLL_CTL_DEL 1
#define EPOLL_CTL_MOD 1
 void close(SOCKET fd);

 int epoll_create(...);
 int epoll_ctl(...);
int epoll_wait(...);

#endif // _LINUX

using namespace GSP;

namespace GSP
{
	class CEPollInfo
	{
	public :
		 struct epoll_event m_stEPEvt;
		 SOCKET m_sk;
		 CEPollInfo(void)
		 {
			 m_sk = INVALID_SOCKET;
			 bzero(&m_stEPEvt, sizeof(m_stEPEvt));
			 m_stEPEvt.events = EPOLLERR | EPOLLET;
			 m_stEPEvt.data.ptr = (void*) this;
		 }

	};


	class CIOEPoll :  public CVIOCPSimulator
	{
	private :
		 int m_hEpoll;	
		 #define WAIT_EPOLL_SIZE 512
#define EPOLL_MAX_CHANNELS 10000
		 struct epoll_event m_stEvents[WAIT_EPOLL_SIZE+1]; 

	public :
		CIOEPoll(INT iID );
		virtual ~CIOEPoll(void);

		virtual BOOL Init(void);
		virtual void Uninit(void);


		virtual EnumErrno Watch( SOCKET fd , BOOL bWouldWakeup,
			void **ppPriData, FuncPtrFree *fnFreePri  );
		virtual void  UinitWatch(SOCKET fd, BOOL bWouldWakeup ) ;	

		//由子类保证线程安全
		virtual EnumErrno AddWatch( SOCKET fd, EnumSocketEventMask iEvtMask, BOOL bWouldWakeup );	
		virtual void CleanWatch( SOCKET fd, EnumSocketEventMask iEvtMask, BOOL bWouldWakeup  );	


		virtual EnumErrno WaitEvent( CSimEventsContains &vResult );

	};



	
	static CEPollInfo *CreateEPInfo(void)
	{
		return new CEPollInfo();
	}

	static void FreeEPInfo( CEPollInfo *pInfo )
	{
		if( pInfo)
		{
			delete pInfo;
		}
	}
} //end namespace GSP

CIOEPoll::CIOEPoll(INT iID ) : CVIOCPSimulator(iID, EPOLL_MAX_CHANNELS)
{
	m_hEpoll = -1;
}

CIOEPoll::~CIOEPoll(void)
{
	if( m_hEpoll != -1 )
	{
		close(m_hEpoll);
		m_hEpoll = -1;
	}
}

 BOOL CIOEPoll::Init(void)
 {
	 if( !CVIOCPSimulator::Init() )
	 {
		return FALSE;
	 }
	 GS_ASSERT(m_hEpoll == -1 );
	 m_hEpoll = ::epoll_create(EPOLL_MAX_CHANNELS+1);

	 if( m_hEpoll==-1  )
	 {
		 CVIOCPSimulator::Uninit();

		 MY_LOG_FATAL(g_pLog, _GSTX("epoll_create 失败. errno:%d %s!\n"), 
						errno, strerror(errno) );
		 GSP_ASSERT(0);
		 return FALSE;

	 }
	 return TRUE;

 }

 void CIOEPoll::Uninit(void)
 {
	if( m_hEpoll != 0 )
	{
		Wakeup();
		close(m_hEpoll);
		m_hEpoll = NULL;
	}
	 CVIOCPSimulator::Uninit();
 }

EnumErrno CIOEPoll::Watch( SOCKET fd , BOOL bWouldWakeup,
						  void **ppPriData, FuncPtrFree *fnFreePri  )
{
	
	*ppPriData = NULL;
	*fnFreePri = NULL;
	CEPollInfo *pPri = CreateEPInfo();
	if( pPri == NULL )
	{
		GS_ASSERT(0);
		return eERRNO_SYS_ENMEM;
	}
	pPri->m_sk = fd;
	struct epoll_event epevt;
	memcpy(&epevt,&pPri->m_stEPEvt, sizeof(epevt));
	 int  r = ::epoll_ctl(m_hEpoll, EPOLL_CTL_ADD,fd, &epevt);
	 if( r )
	 {
		 MY_LOG_ERROR(g_pLog, _GSTX("epoll_ctl err add. %d strerror:%s.\n"), 
					errno, strerror(errno));
		 GSP_ASSERT(0);
		 FreeEPInfo(pPri);
		 return eERRNO_NET_EREG;
	 }
	 *ppPriData = pPri;
	 *fnFreePri = (FuncPtrFree)FreeEPInfo;
	 return eERRNO_SUCCESS;
}

void  CIOEPoll::UninitWatch(SOCKET fd, BOOL bWouldWakeup )
{
	CMapOfSocket::iterator  csIt = m_mapChannels.find(fd);
	if( csIt == m_mapChannels.end() )
	{	
		return ;
	}
	CEPollInfo *pPri = (CEPollInfo *)GetChildClassPriData(csIt->second );
	if(!pPri)
	{
		return;
	}
	pPri->m_sk = INVALID_SOCKET;
	struct epoll_event epevt;
	bzero( &epevt, sizeof(epevt));
	epevt.events = 0;	
	int r = epoll_ctl(m_hEpoll, EPOLL_CTL_DEL,fd, &epevt);                   
	if( r  )
	{		
		MY_LOG_ERROR(g_pLog, _GSTX("epoll_ctl del. %d strerror:%s\n"), errno, strerror(errno) );
		GSP_ASSERT(0);
		return;
	}

	if( bWouldWakeup )
	{
		Wakeup();
	}

}

EnumErrno CIOEPoll::AddWatch( SOCKET fd, EnumSocketEventMask iEvtMask, BOOL bWouldWakeup )
{
	CGSAutoWriterMutex wlock(&m_csWRMutexChn);
	
	CMapOfSocket::iterator  csIt = m_mapChannels.find(fd);
	if( csIt == m_mapChannels.end() )
	{	
		return eERRNO_NET_EREG;
	}	
	CEPollInfo *pPri = (CEPollInfo *)GetChildClassPriData(csIt->second );
	
	if(!pPri)
	{
		GS_ASSERT(0);
		return eERRNO_NET_EREG;
	}    

	if( iEvtMask&eEVT_SOCKET_MASK_READ )
	{
		pPri->m_stEPEvt.events |= EPOLLET|EPOLLIN;
	}
	if( iEvtMask&eEVT_SOCKET_MASK_WRITE  )
	{
		pPri->m_stEPEvt.events |= EPOLLET|EPOLLOUT;
	}

	struct epoll_event epevt;
	memcpy(&epevt,&pPri->m_stEPEvt, sizeof(epevt));
	int  r = ::epoll_ctl(m_hEpoll, EPOLL_CTL_MOD,csIt->first, &epevt);
	if( r )
	{
		MY_LOG_ERROR(g_pLog, _GSTX("epoll_ctl err mod. %d strerror:%s.\n"), 
			errno, strerror(errno));
		GSP_ASSERT(0);		
		return eERRNO_NET_EREG;
	}
	
	if( bWouldWakeup )
	{
		Wakeup();
	}
	return eERRNO_SUCCESS;
}


void CIOEPoll::CleanWatch( SOCKET fd, EnumSocketEventMask iEvtMask, BOOL bWouldWakeup  )
{
	CGSAutoWriterMutex wlock(&m_csWRMutexChn);

	CMapOfSocket::iterator  csIt = m_mapChannels.find(fd);
	if( csIt == m_mapChannels.end() )
	{	
		return;
	}	
	CEPollInfo *pPri = (CEPollInfo *)GetChildClassPriData(csIt->second );

	if(!pPri)
	{
		GS_ASSERT(0);
		return;
	}    

	if( iEvtMask&eEVT_SOCKET_MASK_READ )
	{
		pPri->m_stEPEvt.events &= ~EPOLLIN;
	}
	if( iEvtMask&eEVT_SOCKET_MASK_WRITE  )
	{
		pPri->m_stEPEvt.events &= ~EPOLLOUT;
	}

	struct epoll_event epevt;
	memcpy(&epevt,&pPri->m_stEPEvt, sizeof(epevt));
	int  r = ::epoll_ctl(m_hEpoll, EPOLL_CTL_MOD,csIt->first, &epevt);
	if( r )
	{
		MY_LOG_ERROR(g_pLog, _GSTX("epoll_ctl err mod. %d strerror:%s.\n"), 
			errno, strerror(errno));
		GSP_ASSERT(0);		
		return ;
	}

	if( bWouldWakeup )
	{
		Wakeup();
	}
	return ;
}


EnumErrno CIOEPoll::WaitEvent( CSimEventsContains &vResult )
{
	
	struct epoll_event *events = &m_stEvents[0];
	int iRet;
	int i;
	iRet = ::epoll_wait(m_hEpoll, events, WAIT_EPOLL_SIZE, 1 );
	CEPollInfo *pPri;
	StructSimEvents stEvt;

	if( iRet<1 )
	{   
		if(iRet<0 )
		{
			MY_LOG_ERROR(g_pLog, _GSTX("epoll_wait err. %d strerror:%s\n"), 
				errno, strerror(errno) );
			return eERRNO_SYS_ESTATUS;
		}
		return eERRNO_ENONE;
	}
	for( i = 0; i<iRet; i++ )
	{
		pPri = (CEPollInfo*)events[i].data.ptr;

		stEvt.fd = pPri->m_sk;
		stEvt.iEvtMask = eEVT_SOCKET_MASK_NONE;
		if(  events[i].events&EPOLLIN )
		{
			stEvt.iEvtMask |= eEVT_SOCKET_MASK_READ;

		}

		if( events[i].events&EPOLLOUT )
		{
			stEvt.iEvtMask |= eEVT_SOCKET_MASK_WRITE;				
		}

		if( stEvt.iEvtMask )
		{
			vResult.Add(stEvt);
		}
	}
	return eERRNO_SUCCESS;
}


namespace GSP
{

	CIAsyncIO *CreateAsyncIO(INT iId )
	{
		if( iId == 0 )
		{
			MY_DEBUG( _GSTX("GSP 网络库 使用 EPoll 模型...\n" ) );
		}

		return new CIOEPoll(iId);
	}

} //end namespace GSP


#endif // GSP_ASYNCIO_EPOLL