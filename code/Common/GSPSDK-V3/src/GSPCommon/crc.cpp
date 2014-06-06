#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crc.h"

#define CRCPOLY_LE 0xedb88320
#define CRCPOLY_BE 0x04c11db7

#define BE_TABLE_SIZE (1<<4)

static  unsigned int crc32table_be[BE_TABLE_SIZE]=
{
0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 
0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005, 
0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61, 
0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd
};
/*
static void crc32init_be(void)
{
	unsigned i, j;
	 unsigned int crc = 0x80000000;

	crc32table_be[0] = 0;

	for (i = 1; i < BE_TABLE_SIZE; i <<= 1) {
		crc = (crc << 1) ^ ((crc & 0x80000000) ? CRCPOLY_BE : 0);
		for (j = 0; j < i; j++)
			crc32table_be[i + j] = crc ^ crc32table_be[j];
	}
}
*/

 unsigned int  mk_crc32( void *data, int len,unsigned int crc )
{  
unsigned char *p = (unsigned char *)data;
	while (len--) {
		crc ^= *p++ << 24;
		crc = (crc << 4) ^ crc32table_be[crc >> 28];
		crc = (crc << 4) ^ crc32table_be[crc >> 28];
	}
	return crc;
}

 GS_API  unsigned int   mk_crc16( void *data, int len, unsigned int crc)
 {        
     unsigned char *p = (unsigned char *)data;
     while (len--) {
         crc ^= *p++ << 24;
         crc = (crc << 4) ^ crc32table_be[crc >> 28];
         crc = (crc << 4) ^ crc32table_be[crc >> 28];
     }
     return crc;
 }


 