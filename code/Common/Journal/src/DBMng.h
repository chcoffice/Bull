/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : DBMNG.H
Author :  zouyx
Version : 0.0.0.0
Date: 2011/9/15 14:59
Description: 数据库范围接口操作对象
********************************************
*/

#ifndef _GS_H_DBMNG_H_
#define _GS_H_DBMNG_H_

#include <string>
#include "JouModule.h"
#include "IDBAccessModule.h"



using namespace DBAccessModule;

namespace JOU
{
	class CDBManager : public CJouModule
	{
	private:


        CGSString m_strServer;
        CGSString m_strDatabase;
        CGSString m_strUser;
        CGSString m_strPWD;
        EnumDatabaseType    m_eDbaseType;
        
   
		// 数据库连接对象池
		IConnectionPool* m_pConnPool; 

		IConnectionPool* m_pOutConnPool;  //外部线程池
	public:
		CDBManager(void);
		virtual ~CDBManager(void);

		virtual EnumJouErrno Init( CService *pServer );

		virtual EnumJouErrno Start(void* pData);
		virtual void Stop(void);


        virtual IConnection *GetConnection(void);

        INLINE CGSString ToTime( const char *strTm )
        {
             char temp[128];
             if( m_eDbaseType==ORACLE || m_eDbaseType==OCI )
             {
                
                 GS_SNPRINTF(temp,128, "TO_DATE('%s', 'HH24:Mi:SS')", strTm );               
             }
             else
             {
                 GS_SNPRINTF(temp,128, "'%s'", strTm);
             }
             return CGSString(temp);
        }

        INLINE CGSString ToDate( const char *strTm )
        {
            char temp[128];
            if( m_eDbaseType==ORACLE || m_eDbaseType==OCI )
            {
                
                GS_SNPRINTF(temp,128, "TO_DATE('%s', 'YYYY-MM-DD')", strTm );
               
            }
            else
            {
                sprintf( temp, "'%s'", strTm);
            }
             return CGSString( temp);
        }

        INLINE CGSString ToDateTime( const char *strTm )
        {
             char temp[128];
            if( m_eDbaseType==ORACLE || m_eDbaseType==OCI )
            {
               
                GS_SNPRINTF(temp,128, "TO_DATE('%s', 'YYYY-MM-DD HH24:Mi:SS')", strTm );
               
            }
            else
            {
                GS_SNPRINTF(temp,128, "'%s'", strTm);
            }
            return CGSString( temp);
        }

		INLINE EnumDatabaseType DBaseType(void) const
		{
			return m_eDbaseType;
		}

        INLINE CGSString DateTimeToChar( const char *strSectionName, const char *czAlterName )
        {
            char temp[128];
            if( m_eDbaseType==ORACLE || m_eDbaseType==OCI )
            {  
               GS_SNPRINTF(temp,128, "TO_CHAR(%s, 'YYYY-MM-DD HH24:Mi:SS') AS %s",strSectionName, czAlterName );
            }
            else
            {
               GS_SNPRINTF(temp,128, "%s AS %s", strSectionName, czAlterName);
            }
            return CGSString( temp);
        }
		
	};


} // end namespace JOU



#endif //end _GS_H_DBMNG_H_
