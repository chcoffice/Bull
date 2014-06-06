/*
  eXosip - This is the eXtended osip library.
  Copyright (C) 2002,2003,2004,2005,2006,2007  Aymeric MOIZARD  - jack@atosc.org
  
  eXosip is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  eXosip is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __EXTRANSPORT_H__
#define __EXTRANSPORT_H__

#ifdef ENABLE_MPATROL
#include <mpatrol.h>
#endif
#include <eXosip2/eXosipDefs.h>
#include "eXosip2.h"

#ifdef HAVE_OPENSSL_SSL_H
/* to access version number of ssl from any file */
#include <openssl/opensslv.h>
#endif

#if defined (HAVE_SYS_SELECT_H)
#  include <sys/select.h>
#endif

struct eXtl_protocol {
	int enabled;

	int proto_port;
	char proto_name[10];
	char proto_ifs[20];
	int proto_num;
	int proto_family;
	int proto_secure;
	int proto_reliable;


	


	int (*tl_init) (MARGS2);
	int (*tl_free) (MARGS2);
	int (*tl_open) (MARGS2);
	int (*tl_set_fdset) ( fd_set * osip_fdset, fd_set * osip_wrset, int *fd_max MARGS);
	int (*tl_read_message) ( fd_set * osip_fdset, fd_set * osip_wrset MARGS);
	int (*tl_send_message) ( osip_transaction_t * tr, osip_message_t * sip,
							char *host, int port, int out_socket MARGS);
	int (*tl_keepalive) ( MARGS2 );
	int (*tl_set_socket) (int socket MARGS);
	int (*tl_masquerade_contact) (const char *ip, int port MARGS);
	int (*tl_get_masquerade_contact) (char *ip, int ip_size, char *port,
									  int port_size MARGS);
#ifndef SIP_INSTANCE_ONE
	int socket;
	struct sockaddr_storage addr;
	char firewall_ip[64];
	char firewall_port[10];
	void *pPri;
#endif
};


void eXosip_xtl_init_default_udp(struct eXtl_protocol *eXtl);
void eXosip_xtl_uninit_udp(struct eXtl_protocol *eXtl);

void eXosip_xtl_init_default_tcp(struct eXtl_protocol *eXtl);
void eXosip_xtl_uninit_tcp(struct eXtl_protocol *eXtl);
#ifndef DISABLE_TLS
void eXosip_xtl_init_default_tls(struct eXtl_protocol *eXtl);
void eXosip_xtl_uninit_tls(struct eXtl_protocol *eXtl);

void eXosip_xtl_init_default_dtls(struct eXtl_protocol *eXtl);
void eXosip_xtl_uninit_dtls(struct eXtl_protocol *eXtl);
#endif

#ifdef SIP_INSTANCE_ONE

extern struct eXtl_protocol eXtl_udp;
extern struct eXtl_protocol eXtl_tcp;
#ifndef DISABLE_TLS
extern struct eXtl_protocol eXtl_tls;
extern struct eXtl_protocol eXtl_dtls;
#endif

#else
#define eXtl_udp eXosip.xtl_udp
#define eXtl_tcp eXosip.xtl_tcp
#define eXtl_tls eXosip.xtl_tls
#define eXtl_dtls eXosip.xtl_dtls


#endif

#if defined (WIN32) || defined (_WIN32_WCE)
#define eXFD_SET(A, B)   FD_SET((unsigned int) A, B)
#else
#define eXFD_SET(A, B)   FD_SET(A, B)
#endif

#endif
