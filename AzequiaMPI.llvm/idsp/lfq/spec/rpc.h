/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


#ifndef _RPC_H_
#define _RPC_H_

/*----------------------------------------------------------------*
 *   Declaration of types used by this module                     *
 *----------------------------------------------------------------*/
#include <addr.h>

/*----------------------------------------------------------------*
 *   Exported constants                                           *
 *----------------------------------------------------------------*/
#define NO_PORT           0

#define RPC_FULL          0x00000000
#define RPC_HALF          0x00000001
#define RPC_FOREVER       0xFFFFFFFF

/* Protocol argument */
#define RPC_PROTOCOL     ((Protocol_t)0x0001)

    /*----------------------------------------------------------------*
     *   Exported exceptions                                          *
     *----------------------------------------------------------------*/
#define RPC_E_OK          0
#define RPC_E_EXHAUST    (RPC_E_OK        - 1)
#define RPC_E_INTEGRITY  (RPC_E_EXHAUST   - 1)
#define RPC_E_TIMEOUT    (RPC_E_INTEGRITY - 1)
#define RPC_E_INTERFACE  (RPC_E_TIMEOUT   - 1)
#define RPC_E_SYSTEM     (RPC_E_INTERFACE - 1)
#define RPC_E_SIGNALED   (RPC_E_SYSTEM    - 1)
#define RPC_E_DEADPART   (RPC_E_SIGNALED  - 1)


#define RPC_LOCK(thr)     {                                                     \
                            if(pthread_mutex_lock(&((thr)->Lock)))              \
                              panic("RPC_LOCK");                                \
                          }

#define RPC_UNLOCK(thr)   {                                                     \
                            if(pthread_mutex_unlock(&((thr)->Lock)))            \
                              panic("RPC_UNLOCK");                              \
                          }

#define RPC_SIGNAL(thr)   {                                                     \
                            if(pthread_cond_signal(&((thr)->Ready)))            \
                              panic("RPC_SIGNAL");                              \
                          }


/*----------------------------------------------------------------*
 *   Exported types                                               *
 *----------------------------------------------------------------*/
struct Port {
  int Port;
  int Mchn;
};
typedef struct Port Port, *Port_t;

struct Hdr {
  Port Port;
  int  Mode;
  int  Size;
  int  Store[32 - (sizeof(Port)/sizeof(int))- 1];
};
typedef struct Hdr Hdr, *Hdr_t;


/*----------------------------------------------------------------*
 *   Exported function interface                                  *
 *----------------------------------------------------------------*/
extern int  RPC_init      (void);
extern void RPC_finalize  (void);
extern int  RPC_send      (Hdr_t hdr, void *buff,  int cnt);
extern int  RPC_recv      (Hdr_t hdr, void **buff,  int *cnt);
extern int  RPC_trans     (Hdr_t hdr, void *buff1, int cnt1, void *buff2, int cnt2);
extern int  RPC_register  (int port, int  gix);
extern void RPC_unregister(int port);
extern int  RPC_getServer (int port, int *gix);
extern void RPC_setLoc    (int (*f)(void *srcThr, Addr_t dstAddr, int *mchn, void *thr));

/*----------------------------------------------------------------*
 *   Declaration of common types used by the implementation       *
 *----------------------------------------------------------------*/
/* Bits of "mode" argument of rpc_send */
#define RPC_SVR    0x00000001    /* For RPC mode (invoked by a server)    */


#endif
