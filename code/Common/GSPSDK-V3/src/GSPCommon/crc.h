#ifndef GSS_CRC_DEF_H
#define GSS_CRC_DEF_H


#include "GSPConfig.h"
 
GS_API  unsigned int   mk_crc32( void *data, int len, unsigned int crc = 0xe3404289 );

GS_API  unsigned int   mk_crc16( void *data, int len, unsigned int crc = 0xe3404289 );

#endif
