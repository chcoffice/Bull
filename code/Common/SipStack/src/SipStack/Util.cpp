#include "Util.h"
#include "osipparser2/osip_md5.h"


namespace GSSIP
{

struct _StruMITable
{
	EnumSipMethod eMethod;
	const char *czMethodName;
};


static struct _StruMITable _s_vMITable[] =
{
	{eSIP_METHOD_REGISTER, REGISTER_CODE },
	{eSIP_METHOD_INFO,		INFO_CODE },
	{eSIP_METHOD_NOTIFY,	NOTIFY_CODE },
	{eSIP_METHOD_SUBSCRIBE, SUBSCRIBE_CODE },
	{eSIP_METHOD_BYE,		BYE_CODE },
	{eSIP_METHOD_CANCEL,	CANCEL_CODE },
	{eSIP_METHOD_INFOEX,	 INFOEX_CODE },
	{eSIP_METHOD_MESSAGE,	MESSAGE_CODE },
	{eSIP_METHOD_RESPONSE,	RESPONSE_CODE },
	{eSIP_METHOD_INVITE,   INVITE_CODE},
	{eSIP_METHOD_ACK, ACK_CODE },


	{eSIP_METHOD_INVALID, "INVALID"}  //*****放在最后
};

EnumSipMethod CvtMethodStr2I( const char *czMethodName )
{
	int i = 0;
	for( ; i<_s_vMITable[i].eMethod != eSIP_METHOD_INVALID; i++ )
	{
		if( GSStrUtil::EqualsIgnoreCase(czMethodName,_s_vMITable[i].czMethodName ) )
		{
			return _s_vMITable[i].eMethod;
		}
	}
	return _s_vMITable[i].eMethod;
}

const char *CvtMethodI2Str( EnumSipMethod eMethod )
{
	int i = 0;
	for( ; i<_s_vMITable[i].eMethod != eSIP_METHOD_INVALID; i++ )
	{
		if( _s_vMITable[i].eMethod==eMethod  )
		{
			return _s_vMITable[i].czMethodName;
		}
	}
	return _s_vMITable[i].czMethodName;
}


struct _StruCttITable
{
	EnumSipContentType eType;
	const char *czTypeName;
};


static const struct _StruCttITable _s_vCttITable[] =
{
	{eSIP_CONTENT_SDP, SDP_CONTENT_TYPE	},
	{eSIP_CONTENT_MANSCDP_XML, MANSCDP_XML_CONTENT_TYPE},
	{eSIP_CONTENT_MANSRTSP, MANSRTSP_CONTENT_TYPE},
	{eSIP_CONTENT_MANSRTSP, RTSP_CONTENT_TYPE},
	{eSIP_CONTENT_INNER, "application/inner"	},
	{eSIP_CONTENT_INNER, "application/none"	},


	{eSIP_CONTENT_UNKNOWN, "appalication/unknow"}  //****放在最后
};

EnumSipContentType CvtContentTypeStr2I(const char *czContentType )
{
	int i = 0;
	for( ; _s_vCttITable[i].eType != eSIP_CONTENT_UNKNOWN; i++ )
	{
		if( GSStrUtil::EqualsIgnoreCase(czContentType,_s_vCttITable[i].czTypeName ) )
		{
			return _s_vCttITable[i].eType;
		}
	}
	return _s_vCttITable[i].eType;
}

const char * CvtContentTypeI2Str(EnumSipContentType ezContentType )
{
	int i = 0;
	for( ; _s_vCttITable[i].eType != eSIP_CONTENT_UNKNOWN; i++ )
	{
		if( _s_vCttITable[i].eType==ezContentType  )
		{
			return _s_vCttITable[i].czTypeName;
		}
	}
	return _s_vCttITable[i].czTypeName;
}




void DigestCalcHA1 (const char *pszAlg,
					const char *pszUserName,
					const char *pszRealm,
					const char *pszPassword,
					const char *pszNonce,
					const char *pszCNonce,
					OUT HashHexString strHexHA1)
{
	osip_MD5_CTX Md5Ctx;
	HashBin bHA1;

	osip_MD5Init (&Md5Ctx);
	osip_MD5Update (&Md5Ctx, (unsigned char *) pszUserName, strlen (pszUserName));
	osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
	osip_MD5Update (&Md5Ctx, (unsigned char *) pszRealm, strlen (pszRealm));
	osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
	osip_MD5Update (&Md5Ctx, (unsigned char *) pszPassword, strlen (pszPassword));
	osip_MD5Final ((unsigned char *) bHA1, &Md5Ctx);
	if ((pszAlg != NULL) && osip_strcasecmp (pszAlg, "md5-sess") == 0)
	{
		osip_MD5Init (&Md5Ctx);
		osip_MD5Update (&Md5Ctx, (unsigned char *) bHA1, HASH_LEN);
		osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
		osip_MD5Update (&Md5Ctx, (unsigned char *) pszNonce, strlen (pszNonce));
		osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
		osip_MD5Update (&Md5Ctx, (unsigned char *) pszCNonce, strlen (pszCNonce));
		osip_MD5Final ((unsigned char *) bHA1, &Md5Ctx);
	}
	HashCvtHexStr (bHA1, strHexHA1);
}

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
					/* request-digest or response-digest */ )
{
	osip_MD5_CTX Md5Ctx;
	HashBin HA2;
	HashBin RespHash;
	HashHexString HA2Hex;

	/* calculate H(A2) */
	osip_MD5Init (&Md5Ctx);
	osip_MD5Update (&Md5Ctx, (unsigned char *) pszMethod, strlen (pszMethod));
	osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
	osip_MD5Update (&Md5Ctx, (unsigned char *) pszDigestUri, strlen (pszDigestUri));

	if (pszQop == NULL)
	{
		goto auth_withoutqop;
	}
	else if (0 == strcmp (pszQop, "auth-int"))
	{
		goto auth_withauth_int;
	}
	else if (0 == strcmp (pszQop, "auth"))
	{
		goto auth_withauth;
	}

auth_withoutqop:
	osip_MD5Final ((unsigned char *) HA2, &Md5Ctx);
	HashCvtHexStr(HA2, HA2Hex);

	/* calculate response */
	osip_MD5Init (&Md5Ctx);
	osip_MD5Update (&Md5Ctx, (unsigned char *) strHexHA1, HASH_HEXSTR_LEN);
	osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
	osip_MD5Update (&Md5Ctx, (unsigned char *) pszNonce, strlen (pszNonce));
	osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);

	goto end;

auth_withauth_int:

	osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
	osip_MD5Update (&Md5Ctx, (unsigned char *) strHexEntity, HASH_HEXSTR_LEN);

auth_withauth:
	osip_MD5Final ((unsigned char *) HA2, &Md5Ctx);
	HashCvtHexStr (HA2, HA2Hex);

	/* calculate response */
	osip_MD5Init (&Md5Ctx);
	osip_MD5Update (&Md5Ctx, (unsigned char *) strHexHA1, HASH_HEXSTR_LEN);
	osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
	osip_MD5Update (&Md5Ctx, (unsigned char *) pszNonce, strlen (pszNonce));
	osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
	if(Aka == 0){
		osip_MD5Update (&Md5Ctx, (unsigned char *) pszNonceCount, strlen (pszNonceCount));
		osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
		osip_MD5Update (&Md5Ctx, (unsigned char *) pszCNonce, strlen (pszCNonce));
		osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
		osip_MD5Update (&Md5Ctx, (unsigned char *) pszQop, strlen (pszQop));
		osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
	}
end:
	osip_MD5Update (&Md5Ctx, (unsigned char *) HA2Hex, HASH_HEXSTR_LEN);
	osip_MD5Final ((unsigned char *) RespHash, &Md5Ctx);
	HashCvtHexStr (RespHash, strHexResponse);
}

void HashCvtHexStr (const HashBin Bin, OUT HashHexString strHex)
{
	unsigned short i;
	unsigned char j;

	for (i = 0; i < HASH_LEN; i++)
	{
		j = (Bin[i] >> 4) & 0xf;
		if (j <= 9)
			strHex[i * 2] = (j + '0');
		else
			strHex[i * 2] = (j + 'a' - 10);
		j = Bin[i] & 0xf;
		if (j <= 9)
			strHex[i * 2 + 1] = (j + '0');
		else
			strHex[i * 2 + 1] = (j + 'a' - 10);
	};
	strHex[HASH_HEXSTR_LEN] = '\0';
}


} //end  namespace GSSIP
