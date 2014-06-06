#ifndef GSS_URI_DEF_H
#define GSS_URI_DEF_H




/*
******************************************
Copyright (C), 2010-2011, GOSUN
Filename : GSPURI.H
Author :  zouyx
Version : 0.1.0.0
Date: 2010/5/14 16:01
Description: GSP 中 uri 的分析

* scheme://host:port/path/filename?attrname&attrname1=attrvalue1

* scheme = 通信协议 (常用的http,ftp,maito 等)
* host = 主机 (域名或IP)
* port = 端口号(可选)
* path = 路径
* filename = 名称
* attrname 属性名
* attrname1 属性1名
* attrvalue1 属性1的值

* host:port 为NetInfo
* path/filename 为Key
********************************************
*/

#include <vector>
#include "GSPObject.h"
#include "IUri.h"

namespace GSP
{


    typedef std::vector<StruUriAttr>  CURIAttrList;

    class  CUri :
        public CGSPObject , public CIUri
    {
 
    protected :
         BOOL m_bValid;
        CGSPString m_strScheme;
        CGSPString m_strHost;
        UINT m_iPort;
        CGSPString m_strKey;
        CURIAttrList m_csAttrs;
        CGSPString m_strURI;


    public:
        CUri(void);
        CUri( const CUri &csDest);
        virtual ~CUri(void);

		virtual void Clear(void);

		virtual const char *GetScheme(void) const;
		virtual void  SetScheme(const char *szScheme);

		virtual const char *GetHost(void) const;
		virtual void  SetHost( const char *szHost );

		virtual UINT GetPort(void) const;
		virtual void SetPortArgs(UINT iPort);

		virtual const char *GetKey(void) const;
		virtual void SetKey(const char *szKey);


		virtual StruUriAttr *GetAttr(const char *szAttrName) const;

		virtual StruUriAttr *AttrBegin(void);

		virtual StruUriAttr *AttrNext(const  StruUriAttr *pAttr);


		virtual BOOL AddAttr( const char *szAttrName, const char *szAttrValue );
		virtual void ClearAttr( const char *szName); 


		virtual BOOL Analyse(const char *szURI );

		virtual const char *GetURI(void) const;

		virtual CIUri *Clone(void) const;

		CUri &operator=( const CUri &csUri );

	private :
		CURIAttrList::const_iterator FindAttr(const char *szAttrName ) const;
		StruUriAttr *AddAttrInner( const char *szAttrName, const char *szAttrValue );
    };


};

#endif
