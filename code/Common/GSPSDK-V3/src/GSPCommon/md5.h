#ifndef GSS_MD5_DEF_H
#define GSS_MD5_DEF_H


#include "GSPConfig.h"




typedef UINT16 UINT2;
typedef UINT32 UINT4;



/* MD5 context. */
typedef struct {
  UINT4 state[4];                                   /* state (ABCD) */
  UINT4 count[2];                               /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];                         /* input buffer */
} MD5_CTX;


#define MD5_LEN 16

//#ifdef __cplusplus
//extern "C" {
//#endif
        void MD5Init(MD5_CTX *);
        void MD5Update(MD5_CTX *, unsigned char *, unsigned int);
        void MD5Final (unsigned char digest[MD5_LEN], MD5_CTX *); 
        void MD5Sum(unsigned char *pData, int len, unsigned char md5res[MD5_LEN] );
       INLINE CGSString MD5toString(unsigned char md5res[MD5_LEN])
       {
		   char szTemp[MD5_LEN*2+2];
		   char *p = szTemp;
           for( int i = 0; i<MD5_LEN; i++ )
           {
                GS_SNPRINTF(p, 3, "%02x", md5res[i] );
				p += 2;
           }
		   *p = '\0';
		   return CGSString(szTemp);
       }
//#ifdef __cplusplus
//}
//#endif


#endif
