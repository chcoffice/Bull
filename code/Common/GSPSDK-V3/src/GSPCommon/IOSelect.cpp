/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : IOSELECT.CPP
Author :  邹阳星
Version : 1.0.0.1
Date: 2013/3/29 10:08
Description: 网异步模型 使用 SELECT
********************************************
*/

#include "IAsyncIO.h"
#include "Log.h"

#ifdef GSP_ASYNCIO_USED_SELECT

#include "VIOCPSimulator.h"


using namespace GSP;

namespace GSP
{


class CAIOSelect : public CVIOCPSimulator
{
private :
	fd_set m_stWaitReadSet; 
	fd_set m_stWaitWriteSet; 	
	int m_fdMax;	
public :
	CAIOSelect(INT iID);
	virtual ~CAIOSelect(void);
	
	
	virtual EnumErrno Watch( SOCKET fd , BOOL bWouldWakeup,
		void **ppPriData, FuncPtrFree *fnFreePri  );
	virtual void  UninitWatch(SOCKET fd, BOOL bWouldWakeup ) ;	

	//由子类保证线程安全
	virtual EnumErrno AddWatch( SOCKET fd, EnumSocketEventMask iEvtMask, BOOL bWouldWakeup );	
	virtual void CleanWatch( SOCKET fd, EnumSocketEventMask iEvtMask, BOOL bWouldWakeup  );	

	
	virtual EnumErrno WaitEvent( CSimEventsContains &vResult );


};

} //end namespace GSP


CAIOSelect::CAIOSelect(INT iID) : CVIOCPSimulator(iID, FD_SETSIZE-1)
{
	m_fdMax = -1;
	FD_ZERO(&m_stWaitReadSet);
	FD_ZERO(&m_stWaitWriteSet);
}

CAIOSelect::~CAIOSelect(void)
{

}


EnumErrno CAIOSelect::Watch( SOCKET fd  , BOOL bWouldWakeup,
							void **ppPriData, FuncPtrFree *fnFreePri)
 {	 
	 *ppPriData = NULL;
	 fnFreePri = NULL;
	 if( (INT) fd > m_fdMax )
	 {
		 m_fdMax = fd;
	 }
	return eERRNO_SUCCESS;
 }

 void  CAIOSelect::UninitWatch(SOCKET fd, BOOL bWouldWakeup)
 {
	
 INT bexist = 0;
	if( FD_ISSET(fd, &m_stWaitReadSet) )
	{
		FD_CLR(fd, &m_stWaitReadSet);
		bexist++;
	}
	if( FD_ISSET(fd, &m_stWaitWriteSet) )
	{
		FD_CLR(fd, &m_stWaitWriteSet);
		bexist++;
	}	
	if( fd=m_fdMax )
	{
		//从新查找最大值
		CMapOfSocket::reverse_iterator csIt;

		csIt = m_mapChannels.rbegin();
		if( csIt!= m_mapChannels.rend() )
		{
			++csIt;
			if( csIt!= m_mapChannels.rend() )
			{
				m_fdMax = csIt->first;
			}
			else
			{
				m_fdMax = GetPipeReadSocket();
			}
		}
		else
		{
			m_fdMax = GetPipeReadSocket();
		}		
	}

	if( bexist && bWouldWakeup) 
	{
		Wakeup();
	}
 }

 EnumErrno CAIOSelect::AddWatch( SOCKET fd, EnumSocketEventMask iEvtMask,BOOL bWouldWakeup )
 {
	 m_csWRMutexChn.LockWrite();
	 if( m_mapChannels.find(fd) == m_mapChannels.end() &&
		  fd!=GetPipeReadSocket()  )
	 {		
		m_csWRMutexChn.UnlockWrite();
		return eERRNO_NET_EREG;		
	 }
	 if( iEvtMask&eEVT_SOCKET_MASK_READ )
	 {
		 FD_SET(fd, &m_stWaitReadSet);
	 }
	 if( iEvtMask&eEVT_SOCKET_MASK_WRITE  )
	 {
		 FD_SET(fd, &m_stWaitWriteSet);
	 }
	 m_csWRMutexChn.UnlockWrite();
	 if( bWouldWakeup )
	 {
		Wakeup();
	 }
	 return eERRNO_SUCCESS;
 }


 void CAIOSelect::CleanWatch( SOCKET fd, EnumSocketEventMask iEvtMask ,BOOL bWouldWakeup )
 {

	 m_csWRMutexChn.LockWrite();	 
	 int bexist = 0;
	 if( iEvtMask&eEVT_SOCKET_MASK_READ )
	 {
		 FD_CLR(fd, &m_stWaitReadSet);
		 bexist++;
	 }
	 if( iEvtMask&eEVT_SOCKET_MASK_WRITE  )
	 {
		 FD_CLR(fd, &m_stWaitWriteSet);
		 bexist++;
	 }
	 m_csWRMutexChn.UnlockWrite();
	 if( bexist && bWouldWakeup )
	 {
		 Wakeup();
	 }
 }

 EnumErrno CAIOSelect::WaitEvent( CSimEventsContains &vResult )
 {
fd_set  rset, wset;
		
		if( m_fdMax == -1 )
		{
			//没有数据			
			return eERRNO_ENONE;
		}
// 		FD_ZERO(&rset);
// 		FD_ZERO(&wset);
		m_csWRMutexChn.LockReader();
		memcpy(&rset, &m_stWaitReadSet, sizeof(rset));
		memcpy(&wset, &m_stWaitWriteSet, sizeof(wset));
		m_csWRMutexChn.UnlockReader();

		INT iRet = select(m_fdMax, &rset, &wset, NULL, NULL );
		
		if( iRet< 1 )
		{
			int iErrno = COSSocket::GetSocketErrno();
			return eERRNO_SYS_ESTATUS;
		}	

		m_csWRMutexChn.LockReader();
		StructSimEvents stEvt;
		
		for( CMapOfSocket::iterator csIt=m_mapChannels.begin(); csIt!=m_mapChannels.end(); ++csIt )
		{
			stEvt.fd = csIt->first;
			stEvt.iEvtMask = eEVT_SOCKET_MASK_NONE;
			if( FD_ISSET(stEvt.fd, &rset) )
			{
				stEvt.iEvtMask |= eEVT_SOCKET_MASK_READ;
			
			}

			if( FD_ISSET(stEvt.fd, &wset) )
			{
				stEvt.iEvtMask |= eEVT_SOCKET_MASK_WRITE;				
			}

			if( stEvt.iEvtMask )
			{
				vResult.Add(stEvt);
			}
			
		}
		m_csWRMutexChn.UnlockReader();
		return eERRNO_SUCCESS;	
		
 }



 namespace GSP
 {

	 CIAsyncIO *CreateAsyncIO(INT iId )
	 {
		 if( iId == 0 )
		 {
			MY_DEBUG(_GSTX("GSP 网络库 使用 Select 模型...\n" ) );
		 }
		 return new CAIOSelect(iId);
	 }

 } //end namespace GSP


#endif // GSP_ASYNCIO_USED_SELECT