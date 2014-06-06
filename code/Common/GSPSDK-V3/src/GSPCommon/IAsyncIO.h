/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : IASYNCIO.H
Author :  zouyx
Version : 0.0.0.0
Date: 2012/3/15 14:35
Description: 
********************************************
*/

#ifndef _GS_H_IASYNCIO_H_
#define _GS_H_IASYNCIO_H_




#include "GSPObject.h"
#include "GSPMemory.h"
#include "OSSocket.h"
#include <vector>

 

namespace GSP
{
	class CISocket;
	class CIAsyncIO;



//#define GSP_ASYNCIO_USED_SELECT

#ifndef GSP_ASYNCIO_USED_SELECT
#ifdef _WIN32

//#define GSP_ASYNCIO_USED_WSASELECT
#define GSP_ASYNCIO_USED_IOCP

#elif defined(_LINUX)

#define GSP_ASYNCIO_EPOLL 

#endif
#endif


	class CNetAsyncIOGlobal 
	{
	private : 	
		static CNetAsyncIOGlobal s_csGlobal;
		std::vector<CIAsyncIO*> m_vAIO;
		CGSMutex m_csMutex;
		UINT m_iRefs;
		INT m_iIDSeq;
	public :
		static CNetAsyncIOGlobal &Intance(void);
		static void InitModule(void);
		static void UninitModule(void);
		CIAsyncIO *Register(CISocket *pSocket, CNetError &csError ); 
	private :
		void Init(void);
		void Uninit(void);
		CNetAsyncIOGlobal(void);
		~CNetAsyncIOGlobal(void);
		
	};



    class CIAsyncIO :
        public CRefObject
    {
	private : 
		friend class CNetAsyncIOGlobal;
		INT m_iId;
    public :       
        virtual void Unregister( CISocket *pSocket ) = 0;  
		virtual EnumErrno AsyncRcvFrom(CISocket *pSocket,BOOL bStart) = 0;
		virtual EnumErrno AsyncRcv(CISocket *pSocket,	BOOL bStart) = 0;    
		virtual EnumErrno AsyncAccept(CISocket *pSocket,BOOL bStart) = 0;   
		virtual EnumErrno AsyncSend(CISocket *pSocket, void *pKey, std::vector<CProPacket *> vProPacket ) = 0;
		virtual EnumErrno AsyncSend(CISocket *pSocket, void *pKey,std::vector<CProFrame *> vProFrame) = 0;
	protected : 

		virtual INT  GetMaxChannelContain(void) = 0;
		virtual INT  GetEmptyCounts(void) = 0;      
        virtual BOOL Init(void) = 0;
        virtual void Uninit(void) = 0;
		virtual BOOL Register(CISocket *pSocket, CNetError &csError) = 0;

	
		CIAsyncIO(INT iId) : CRefObject(), m_iId(iId)
		{
		}
        virtual ~CIAsyncIO(void)
		{

		}
    };

//#define MAX_SOCKET_ASYNC_THREADS 1

	CIAsyncIO *CreateAsyncIO(INT iId );
};





#endif //end _GS_H_IASYNCIO_H_
