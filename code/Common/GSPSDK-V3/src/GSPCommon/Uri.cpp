#include "Uri.h" 

using namespace GSP;


namespace GSP
{

static INT _GetUriLength( const char *szStr )
{
	if( !szStr )
	{
		return -1;
	}
	INT i =0;
	while( *szStr != '\0' )
	{
		szStr++;
		i++;
		if( i>GSP_MAX_URI_LEN )
		{
			return -2;
		}
	}
	return i;
}

} //end namespace GSP


CUri::CUri(void)
:CGSPObject()
,CIUri()
,m_strScheme()
,m_strHost()
,m_iPort(0)
,m_strKey()
,m_csAttrs()
,m_strURI()
{
   
    Clear();
}

CUri::CUri(const CUri &csDest)
:CGSPObject()
,CIUri()
,m_strScheme()
,m_strHost()
,m_iPort(0)
,m_strKey()
,m_csAttrs()
,m_strURI()
{
    Clear();
    *this = csDest;
}

CUri::~CUri(void)
{
    Clear();
}

void CUri::Clear(void)
{

    m_strScheme.clear();
    m_strHost.clear();
    m_iPort=0;
    m_strKey.clear();
    m_csAttrs.clear();
    m_strURI.clear();
    m_bValid = FALSE;
}

const char *CUri::GetScheme(void) const
{
	return m_strScheme.c_str();
}

void  CUri::SetScheme(const char *szScheme)
{
	GS_ASSERT(szScheme);
	m_strScheme = szScheme;
}

const char *CUri::GetHost(void) const
{
	return m_strHost.c_str();
}

void  CUri::SetHost( const char *szHost )
{
	GS_ASSERT(szHost);
	m_strHost = szHost;
}

UINT CUri::GetPort(void) const
{
	return m_iPort;
}

void CUri::SetPortArgs(UINT iPort)
{
	m_iPort = iPort;

}

const char *CUri::GetKey(void) const
{
	return m_strKey.c_str();
}

void CUri::SetKey(const char *szKey)
{
	GS_ASSERT(szKey);
	m_strKey = szKey;
}


CURIAttrList::const_iterator CUri::FindAttr(const char *szAttrName) const
{
 CURIAttrList::const_iterator csIt;
    for( csIt = m_csAttrs.begin(); csIt!=m_csAttrs.end(); csIt++ )
    {
        if( 0== strncmp((*csIt).szName,szAttrName, MAX_URI_ATTRI_NAME_LEN) )
        {
            break;
        }
    }
    return csIt;
}

StruUriAttr *CUri::GetAttr(const char *szAttrName) const
{
CURIAttrList::const_iterator csIt;
    csIt = FindAttr( szAttrName );
    if( csIt != m_csAttrs.end() )
    {
        return const_cast<StruUriAttr*>(&(*csIt));
    }
    return NULL;
}

StruUriAttr *CUri::AttrBegin(void) 
{
	if( m_csAttrs.size() )
	{
		return &m_csAttrs[0];
	}
	return NULL;
}

StruUriAttr *CUri::AttrNext(const StruUriAttr *pAttr)
{
	GS_ASSERT_RET_VAL(NULL!=pAttr, NULL);
	for( UINT i = 0; i<m_csAttrs.size(); i++ )
	{
		if( pAttr == &m_csAttrs[i] )
		{
			i++;
			if( i<m_csAttrs.size() )
			{
				return &m_csAttrs[i];
			}
			break;
		}
	}	
	return NULL;
}

StruUriAttr *CUri::AddAttrInner( const char *szAttrName, const char *szAttrValue )
{
	GS_ASSERT_RET_VAL(szAttrValue, NULL);

	GS_ASSERT_RET_VAL(strlen(szAttrName)<MAX_URI_ATTRI_NAME_LEN, NULL);

	StruUriAttr stAttr;
	::strncpy(stAttr.szName, szAttrName, MAX_URI_ATTRI_NAME_LEN);
	if( szAttrValue )
	{
		GS_ASSERT_RET_VAL(strlen(szAttrValue)<MAX_URI_ATTRI_VALUE_LEN, NULL);
		::strncpy( stAttr.szValue, szAttrValue, MAX_URI_ATTRI_VALUE_LEN);	
	}
	else
	{
		stAttr.szValue[0] = '\0';
	}
	m_csAttrs.push_back( stAttr );
	return &m_csAttrs[m_csAttrs.size()-1];
}

BOOL CUri::AddAttr( const char *szAttrName, const char *szAttrValue )
{
	if( AddAttrInner(szAttrName,szAttrValue) )
	{
		return TRUE;
	}
	return FALSE;
}


void CUri::ClearAttr( const char *szName)
{  
    CURIAttrList::const_iterator csIt;
	do 
	{
		csIt = FindAttr( szName );
		if( csIt == m_csAttrs.end() )
		{
			break;
		}
		else 
		{
			m_csAttrs.erase( csIt );
		}
	}while(1);
}



BOOL CUri::Analyse(const char *szURI)
{
    Clear();
    GS_ASSERT_RET_VAL(_GetUriLength(szURI)>0, FALSE);

    UINT iPosHost;

    UINT iPosAttr;
    UINT iPosPort;
    UINT iPosKey;
    UINT iTemp;
	CGSPString strURI(szURI);
    CGSPString strTemp;


    //拷贝 Scheme 
     iPosHost = strURI.find("://");
    if (iPosHost == CGSPString::npos ) 
    {
       return FALSE;
    }
    m_strScheme = strURI.substr(0,iPosHost);  
    iTemp = iPosHost+3;

    iPosPort = strURI.find( ':', iTemp);
    if( iPosPort != CGSPString::npos )
    {
         iTemp = iPosPort+1;
    }


    iPosKey = strURI.find( '/', iTemp);
    if( iPosKey != CGSPString::npos )
    {
        iTemp = iPosKey+1;
    }


    iPosAttr = strURI.find('?', iTemp );


    if( iPosAttr!=CGSPString::npos )
    {
        //带有参数项
        strTemp = strURI.substr(iPosAttr+1);
        std::vector<CGSPString> vAttrs;
        GSStrUtil::Split(vAttrs, strTemp, "&" );
        //格式化Key=Value

        for( UINT i = 0; i<vAttrs.size(); i++ )
        {
 
            iTemp = vAttrs[i].find('=');
			
			if(iTemp !=  CGSPString::npos)
			{
				AddAttrInner(vAttrs[i].substr(0, iTemp).c_str(), 
							vAttrs[i].substr(iTemp+1).c_str());
			}
			else
			{
				AddAttrInner(vAttrs[i].substr(0, iTemp).c_str(), NULL);          
			}
            
        }
    }

    //拷贝IP



    iTemp =  (iPosPort == CGSPString::npos ? iPosKey : iPosPort);
    if( iTemp== CGSPString::npos )
    {
        iTemp = iPosAttr;
    }
    if( iTemp!= CGSPString::npos )
    {
        iTemp = iTemp-iPosHost-3;
    } 
    m_strHost = strURI.substr( iPosHost+3, iTemp );
    if( m_strHost.empty() )
    {
        //没有Host 是错误的....
        Clear();
        return FALSE;
    }

 
    if( iPosPort !=  CGSPString::npos )
    {
        //带有端口号
        iTemp = (iPosKey == CGSPString::npos ? iPosAttr : iPosKey ); 
        if( iTemp!= CGSPString::npos )
        {
            iTemp = iTemp - iPosPort-1;
        }
        strTemp = strURI.substr(iPosPort+1, iTemp  );
        m_iPort = GSStrUtil::ToNumber<UINT>(strTemp);
    }    

   
    if( iPosKey !=  CGSPString::npos) 
    {
        //带有Key参数
         iTemp =  iPosAttr;
         if( iTemp!= CGSPString::npos )
         {
             iTemp = iTemp - iPosKey-1;
         } 
         m_strKey = strURI.substr( iPosKey+1, iTemp );
    }    

  
    m_strURI = strURI;
    m_bValid = TRUE;
    return TRUE;
}

const char *CUri::GetURI(void) const
{
    if( m_strScheme.empty() )
    {
        return NULL;
    }
    if( !m_strURI.empty() )
    {
        return m_strURI.c_str();
    }
    
	CGSPString *str = const_cast<CGSPString*>(&m_strURI);

    (*str) = m_strScheme;
    (*str) += "://";
    (*str) += m_strHost;
    if( m_iPort != 0 )
    {
        char szTemp[16];
        GS_SNPRINTF(szTemp,16,  ":%d", m_iPort);
        (*str) += szTemp;
    }
    if( !(*str).empty() )
    {
        (*str) += "/";
        (*str) += m_strKey;
    }
    CURIAttrList::iterator csIt; 
    BOOL bFirst = TRUE;
    CURIAttrList *pList = const_cast<CURIAttrList*>(&m_csAttrs);
    for( csIt = pList->begin(); csIt != pList->end(); csIt++ )
    {    
        if( bFirst)
        {
            (*str) += "?";
            bFirst = FALSE;
        }
        else
        {
            (*str) += "&";
        }
        (*str) += (*csIt).szName;
        if( (*csIt).szValue[0]!='\0'  )
        {
            (*str) += "=";
            (*str) += (*csIt).szValue;
        }
    }
    return (*str).c_str();
}

 CIUri *CUri::Clone(void) const
{
	return new CUri(*this);
}

CUri &CUri::operator=(const CUri &csDest)
{
	if( this!=&csDest )
	{

		Clear();
		m_strScheme = csDest.m_strScheme;
		m_strHost = csDest.m_strHost;
		m_iPort = csDest.m_iPort;
		m_strKey = csDest.m_strKey;
		m_strURI = csDest.m_strURI;
		m_csAttrs = csDest.m_csAttrs;
	}   
	return *this;
}