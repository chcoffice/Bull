/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : UTIL.H
Author :  邹阳星
Version : 1.0.0.1
Date: 2012/9/25 15:00
Description: 工具类
********************************************
*/

#ifndef _GS_H_UTIL_H_
#define _GS_H_UTIL_H_

#include "SIPCommonDef.h"
#include "SipStack.h"


namespace GSSIP
{


EnumSipMethod CvtMethodStr2I( const char *czMethodName );
const char *CvtMethodI2Str( EnumSipMethod eMethod );

EnumSipContentType CvtContentTypeStr2I(const char *czContentType );
const char * CvtContentTypeI2Str(EnumSipContentType ezContentType );

#define HASH_HEXSTR_LEN 32
#define HASH_LEN 16

typedef  char HashHexString[HASH_HEXSTR_LEN+1];
typedef  unsigned char HashBin[HASH_LEN];



//按 rfc2617.txt 计算认证信息
void DigestCalcHA1 (const char *pszAlg,      /*algirthm*/
					const char *pszUserName,
					const char *pszRealm,
					const char *pszPassword,
					const char *pszNonce,
					const char *pszCNonce,
					OUT HashHexString strHexHA1);
void
DigestCalcResponse (IN const HashHexString strHexHA1,     /* H(A1) */
					IN const char *pszNonce,    /* nonce from server */
					IN const char *pszNonceCount,       /* 8 hex digits */
					IN const char *pszCNonce,   /* client nonce */
					IN const char *pszQop,      /* qop-value: "", "auth", "auth-int" */
					IN int Aka,		/* Calculating AKAv1-MD5 response */
					IN const char *pszMethod,   /* method from the request */
					IN const char *pszDigestUri,        /* requested URL */
					IN const HashHexString strHexEntity, /* H(entity body) if qop="auth-int" */
					OUT HashHexString strHexResponse
					/* request-digest or response-digest */ );

void HashCvtHexStr (const HashBin Bin, OUT HashHexString strHex );


} //end namespace GSSIP


#endif //end _GS_H_UTIL_H_
