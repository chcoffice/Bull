#if _LINUX

#include "LinuxEpoll.h"

using namespace NetServiceLib;

CLinuxEpoll::CLinuxEpoll(void)
{
	m_nPollHandle = -1;
}

CLinuxEpoll::~CLinuxEpoll(void)
{
}

INT32 CLinuxEpoll::CreateEpoll()
{
	m_nPollHandle = epoll_create( MAX_EPOLL_SIZE );

	if( m_nPollHandle == -1 ) 
	{
		assert(0);
		return ERROR_NET_CREATE_EPOLL_FAIL;
	}

	return ERROR_BASE_SUCCESS;
}
INT32 CLinuxEpoll::EpollCtrl(INT32 Op, INT32 fd, struct epoll_event *event)
{
	return epoll_ctl(m_nPollHandle,Op,fd, event);

}
INT32 CLinuxEpoll::EpollWait(struct epoll_event * events,int maxevents,int timeout)
{
	
	if ( m_nPollHandle <= 0 )
	{
		assert(0);
	}
	return epoll_wait( m_nPollHandle,events, maxevents, timeout );

}

DWORD CLinuxEpoll::GetNumberOfProcessors()
{
	return sysconf(_SC_NPROCESSORS_CONF);
}

#endif //_linux

