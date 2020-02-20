/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


#ifndef _INET_H
#define _INET_H

/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#ifdef __OSI
#include <osi.h>
#else
#include <pthread.h>
#endif

/*----------------------------------------------------------------*
 *   Definition of public constants                               *
 *----------------------------------------------------------------*/
/* Error codes */
#define INET_E_OK             0
#define INET_E_EXHAUST       (INET_E_OK          - 1)
#define INET_E_INTEGRITY     (INET_E_EXHAUST     - 1)
#define INET_E_TIMEOUT       (INET_E_INTEGRITY   - 1)
#define INET_E_INTERFACE     (INET_E_TIMEOUT     - 1)
#define INET_E_SYSTEM        (INET_E_INTERFACE   - 1)
#define INET_E_OTHER         (INET_E_SYSTEM      - 1)

/* Max. number for the Protocol field in the message */
#define PROTOCOL_MAX         (8)

/* Max. nodes allowed in the network */
#define MAX_NODES            1024

/* NET messages modes */
#define INET_LAST_FRAGMENT   0x0001
#define INET_LOCAL           0x0002
#define INET_REMOTE          0x0004
#define INET_BROADCAST       0x0008
#define INET_ROUTE           0x0010
#define INET_TERMINATE       0x0020

/*----------------------------------------------------------------*
 *   Definition of public data types                              *
 *----------------------------------------------------------------*/
/* Machine number type */
typedef unsigned short  Mchn_t;

/* Protocol number type */
typedef unsigned short  Protocol_t;

/* IOV struct from upper levels */
struct INET_iovec {
  char   *Data;
  int     Size;
};
typedef struct INET_iovec INET_iovec, *INET_iovec_t;

/*----------------------------------------------------------------*
 *   Declaration of public interface                              *
 *----------------------------------------------------------------*/
int     INET_init       (void *param);
void    INET_finalize   (void);
int     INET_subscribe  (int (*upcall)(INET_iovec *iov, int last_frgmt, int *success), Protocol_t protocol);
int     INET_send       (INET_iovec *iov, Mchn_t dstMchn, Protocol_t protocol);
int     INET_broadcast  (INET_iovec *iov, Protocol_t protocol);
void    INET_recv       (char *packet);
int     INET_by         (Mchn_t dstMchn);
Mchn_t  INET_getCpuId   (void);
int     INET_getNodes   (void);


#endif
