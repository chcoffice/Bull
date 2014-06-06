#ifndef GSS_OSTHREAD_DEF_H
#define GSS_OSTHREAD_DEF_H

#ifndef _WIN32
#include <pthread.h>
#endif

#include "GSPObject.h"



namespace GSP
{

    class CSection
    {
    public :
        CSection(void);
        ~CSection(void);

        BOOL Lock(void);

        BOOL Unlock(void);

    private :

#ifdef _WIN32
        CRITICAL_SECTION m_stSection;
#else
        pthread_mutex_t m_stSection;
        pthread_mutexattr_t m_stAttr;
#endif
    };

    class CAutoSection
    {
    private :
        CSection *m_pLocker;
    public :
        CAutoSection(CSection *pLocker)
        {
            m_pLocker = pLocker;
            pLocker->Lock();
        } 
        ~CAutoSection(void)
        {
			if( m_pLocker )
			{
				m_pLocker->Unlock();
			}
        }

		void Breakup(void)
		{
			if( m_pLocker )
			{
				m_pLocker->Unlock();
				m_pLocker = NULL;
			}
			
		}
    };

	class CMyAutoMutex
	{
	private :
		CGSMutex *m_pMutex;
	public :
		CMyAutoMutex(CGSMutex *pMutex)
			: m_pMutex(pMutex)
		{
			m_pMutex->Lock();
		}

		~CMyAutoMutex(void)
		{
			if( m_pMutex )
			{
				m_pMutex->Unlock();
			}
		}

		void Breakup(void)
		{
			if( m_pMutex )
			{
				m_pMutex->Unlock();
				m_pMutex = NULL;
			}
			
		}

	};


    class COSThread : public CGSPObject
    {

    public :

#ifdef _WIN32
    #define THREAD_ID DWORD
    #define INVALID_THREAD_ID  ((DWORD)-1)
#else
    #define  INVALID_THREAD_ID  ((pthread_t)-1)
    #define THREAD_ID pthread_t
#endif

        static INLINE THREAD_ID CurrentThreadID(void)
        {
#ifdef _WIN32
            return GetCurrentThreadId() ;
#else
            return pthread_self();
#endif
        }

        static INT CurrentDeviceCPUNumber(void);

      //  static void PathParser( CGSSString &strPath);

     //   static CGSSString GetApplicationPath(void);

      //  static  BOOL TestAndCreateDir( const char *czPath);

          //返回GMT 的偏差 单位 小时
        static  INT32 GetGMTOffset(void);

          //返回自1970的毫秒数 UTC
        static  INT64 Milliseconds(void);

      //  static  BOOL CTime( UINT32 iSecs, CGSSString &stFmt );

        static  BOOL GetCTimeOfGMT( CGSPString &stFmt);
        
    };

	class CGSWRMutexAutoWrite
	{
	private :
		CGSWRMutex *m_pMutex;
		int *m_pCurID;
	public :
		CGSWRMutexAutoWrite( CGSWRMutex *pMutex, int *pCurID = NULL)
			:m_pMutex(pMutex)

		{

			GS_ASSERT(m_pMutex!=NULL);
			m_pMutex->LockWrite();
			m_pCurID = pCurID;
		}


		~CGSWRMutexAutoWrite(void)
		{
			m_pMutex->UnlockWrite();
			if( m_pCurID )
			{
				*m_pCurID=0;
			}
		}
	};

	class CGSWRMutexAutoRead
	{
	private :
		CGSWRMutex *m_pMutex;

	public :

		CGSWRMutexAutoRead( CGSWRMutex *pMutex)
			:m_pMutex(pMutex)

		{

			GS_ASSERT(m_pMutex!=NULL);
			m_pMutex->LockReader();

		}

		~CGSWRMutexAutoRead(void)
		{
			m_pMutex->UnlockReader();

		}
	};

	extern void GSPModuleInit(void);
	extern void GSPModuleUnint(void);
};

#endif
