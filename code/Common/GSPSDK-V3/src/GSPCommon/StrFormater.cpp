#include "StrFormater.h"
#include <time.h>

using namespace  GSP;
CStrFormater::CStrFormater(void)
{
}

CStrFormater::~CStrFormater(void)
{
}


INT64 CStrFormater::ParserTimeString(const CGSString &strValue )
{
	// yyyymmddhhmmss
	GS_ASSERT_RET_VAL(strValue.length()==14, -1);
	char szTemp[16];	
	strncpy(szTemp, strValue.c_str(), 15);
	struct tm stTm;
	bzero(&stTm, sizeof(stTm));
	char *p;

	p = &szTemp[14];
	*p = '\0';
	p -= 2;
	stTm.tm_sec = atoi(p); // ss

	*p = '\0';
	p -= 2;
	stTm.tm_min = atoi(p); //mm

	*p = '\0';
	p -= 2;
	stTm.tm_hour = atoi(p); //hh

	*p = '\0';
	p -= 2;
	stTm.tm_mday = atoi(p); //dd

	*p = '\0';
	p -= 2;
	stTm.tm_mon = atoi(p)-1; //mm

	*p = '\0';
	p -= 4;
	stTm.tm_year = atoi(p)-1900; //yyyy

	return (INT64) mktime(&stTm);

}


void CStrFormater::Skip(const char **pp, const char *czEnd)
{
	const char *p;
	p = *pp;
	while (*p != '\0' && !strchr(czEnd, *p)  )
	{
		p++;
	}
	*pp = p;
}

void CStrFormater::SkipSpaces(const char **pp)
{
	const char *p;
	p = *pp;
	while (*p != '\0' && strchr(RTP_SPACE_CHARS, *p)  )
	{
		p++;
	}
	*pp = p;
}


void CStrFormater::GetWordUntilChars(CGSString &strWord, const char *sep, const char **pp)
{
	const char *p; 
	strWord.clear();
	p = *pp;
	CStrFormater::SkipSpaces(&p); 

	while (*p != '\0' && !strchr(sep, *p)  ) 
	{
		strWord += *p;
		p++;
	}  
	*pp = p;
}

BOOL CStrFormater::StrIsStart(const char *str, const char *pfx, const char **ptr)
{
	while ( *pfx!='\0' && *str!='\0' && toupper((unsigned)*pfx) == toupper((unsigned)*str)) 
	{
		pfx++;
		str++;
	}
	if (*pfx=='\0' && ptr)
	{
		*ptr = str;
	}
	return (*pfx=='\0');
}


//把进制数转为字符串

void CStrFormater::BinaryToString(const BYTE *pBind, int iLen, CGSString &strResult )
{
	char czHexA[17] = "0123456789abcdef";
	strResult.clear();
	std::ostringstream oss;
	for (int i = 0; i < iLen; i++) {
		oss <<  czHexA[(pBind[i] & 0xF0) >> 4];
		oss <<  czHexA[pBind[i] & 0x0F];
	}	
	strResult = oss.str();
}

//iBufLen 出入参数
BOOL CStrFormater::StringToBinary( const CGSString &strString, BYTE *pBuf, int &iBufLen )
{
	const char *p = (const char * ) strString.c_str();
	int iStrI = strString.length();
	if( iBufLen*2 < iStrI )
	{
		GS_ASSERT(0);
		iBufLen = 0 ;
		return FALSE;
	}
	iBufLen = 0;
	BYTE iValue = 0;
	for( int i = 0; i<iStrI; )
	{
		if( p[i]>= '0' && p[i]<='9' )
		{
			iValue = p[i]-'0';
		} 
		else if( p[i]>= 'a' && p[i]<='f' )
		{
			iValue = p[i]-'a'+10;
		}
		else if( p[i]>= 'A' && p[i]<='F' )
		{
			iValue = p[i]-'A'+10;
		}
		else
		{	
			GS_ASSERT(0);
			iBufLen = 0 ;
			return FALSE;			
		}
		i++;
		iValue = iValue<<4;
		if( p[i]>= '0' && p[i]<='9' )
		{
			iValue |= p[i]-'0';
		} 
		else if( p[i]>= 'a' && p[i]<='f' )
		{
			iValue |= p[i]-'a'+10;
		}
		else if( p[i]>= 'A' && p[i]<='F' )
		{
			iValue |= p[i]-'A'+10;
		}
		else
		{	
			GS_ASSERT(0);
			iBufLen = 0 ;
			return FALSE;			
		}
		i++;
		*pBuf = iValue;
		pBuf++;
		iBufLen++;
	}
	return TRUE;
}
