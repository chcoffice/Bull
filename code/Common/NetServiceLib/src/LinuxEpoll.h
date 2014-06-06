#if !defined (LinuxEpoll_DEF_H)
#define LinuxEpoll_DEF_H

#if _LINUX


#include "NetServiceDataType.h"

namespace NetServiceLib
{



#define MAX_EPOLL_SIZE  100		//poll的最大范围

typedef struct _overlaped		//这个其实没有实际用 只是使得看起来和IOCP很相似而已
{
	int base;
}OVERLAPPED;

typedef struct _WSABUF
{
	void *buf;
	int len;
}WSABUF;

typedef OVERLAPPED *LPOVERLAPPED;

typedef struct 
{
	OVERLAPPED	Overlapped;
	WSABUF		DataBuf; 
	char		Buffer[DATA_BUFSIZE];
	INT			OptionType;					//操作类型
	sockaddr_in	struSockAddrFrom;	//UDP接收数据时的地址
	INT			iAddrFromLen;		//UDP接收数据时的地址的长度

} PER_IO_OPERATION_DATA, * LPPER_IO_OPERATION_DATA;

class CLinuxEpoll
{
public:
	CLinuxEpoll(void);
	virtual ~CLinuxEpoll(void);
	//创建epoll文件描述符
	INT32	CreateEpoll();
	//向epoll文件描述符注册事件，意思是，向epoll文件描述符增加sokcet、删除socket、修改sokcet
	INT32	EpollCtrl(INT32 Op, INT32 fd, struct epoll_event *event);
	//等待epoll文件描述符中某一个sokcet的事件发生 如EPOLLOUT、EPOLLIN
	INT32	EpollWait(struct epoll_event * events,int maxevents,int timeout);
	//获取cpu内核数目
	DWORD	GetNumberOfProcessors();
private:
	INT32		m_nPollHandle;			//epoll句柄
};

#endif //_linux

}

#endif

