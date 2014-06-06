/*
******************************************
Copyright (C), 2011-2012, GOSUN
Filename : EXOSIPDEFS.H
Author :  ×ÞÑôÐÇ
Version : 1.0.0.1
Date: 2012/9/26 14:23
Description: 
********************************************
*/

#ifndef _GS_H_EXOSIPDEFS_H_
#define _GS_H_EXOSIPDEFS_H_


#ifdef __cplusplus
extern "C"
{
#endif

#ifndef XOSIP_TYPEED
#define XOSIP_TYPEED
	typedef struct eXosip_t eXosip_t;
#endif

#ifdef __cplusplus
}
#endif

//#define SIP_INSTANCE_ONE

#ifdef SIP_INSTANCE_ONE

extern eXosip_t eXosip;
#define MPARAMS	 ,&eXosip
#define MPARAMS2  &eXosip
#else !defined(MARGS2)

#define eXosip (*_pXosip)
#define MPARAMS	 ,_pXosip
#define MPARAMS2  _pXosip
#endif

#define MARGS   ,eXosip_t *_pXosip
#define MARGS2  eXosip_t *_pXosip


#ifndef OSIP_MT
#define OSIP_MT
#endif

#endif //end _GS_H_EXOSIPDEFS_H_
