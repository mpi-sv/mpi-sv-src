/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


/*----------------------------------------------------------------*
 *   Declaration of types and functions defined by this module    *
 *----------------------------------------------------------------*/
/* A server skeleton has no interface */


/*----------------------------------------------------------------*
 *   Declaration of types and functions used by this module       *
 *----------------------------------------------------------------*/
#include <config.h>

#if defined(__OSI)
  #include <osi.h>
#else
  #include <semaphore.h>
#endif

#include <xpn.h>
#include <addr.h>
#include <rpc.h>
#include <thr.h>
#include <com.h>
#include <pmp_sk.h>
#include <pmp.h>
#include <pmp_msg.h>

extern int    rpc_send  (const Addr_t dst, char     *buf,
                                           int       cnt,
                                           int       mode);
extern int    rpc_recv  (const Addr_t src, char     *buf,
                                           int       cnt,
                                           Status   *status,
                                           unsigned  timeout);

/*----------------------------------------------------------------*
 *   Definition of private constants                              *
 *----------------------------------------------------------------*/
#define PMP_SK_E_OK          0
#define PMP_SK_E_EXHAUST    (PMP_SK_E_OK          - 1)
#define PMP_SK_E_INTEGRITY  (PMP_SK_E_EXHAUST     - 1)
#define PMP_SK_E_TIMEOUT    (PMP_SK_E_INTEGRITY   - 1)
#define PMP_SK_E_INTERFACE  (PMP_SK_E_TIMEOUT     - 1)
#define PMP_SK_E_SYSTEM     (PMP_SK_E_INTERFACE   - 1)
#define PMP_SK_E_SIGNALED   (PMP_SK_E_SYSTEM      - 1)
#define PMP_SK_E_DEADPART   (PMP_SK_E_SIGNALED    - 1)


#ifdef __PMP_DEBUG
#define DBG_PRNT(pmsg)  fprintf pmsg
#else
#define DBG_PRNT(pmsg)
#endif

/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
static char *e_names[8] = {  /*  0 */ "PMP_SK_E_OK",
                             /*  1 */ "PMP_SK_E_EXHAUST",
                             /*  2 */ "PMP_SK_E_INTEGRITY",
                             /*  3 */ "PMP_SK_E_TIMEOUT",
                             /*  4 */ "PMP_SK_E_INTERFACE",
                             /*  5 */ "PMP_SK_E_SYSTEM",
                             /*  6 */ "PMP_SK_E_SIGNALED",
                             /*  7 */ "PMP_SK_E_DEADPART"
                           };


/* Thread descriptor of the PMP server */
Thr_t pmp_thr;

#if defined (HAVE_SEM_OPEN)
       sem_t     *pmp_sync;
#else
       sem_t      pmp_sync;
#endif


/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/

     /*----------------------------------------------------------------*\
    |    Pmp_Get                                                         |
    |                                                                    |
     \*----------------------------------------------------------------*/
static inline void Pmp_Get(Pmp_Msg_t rqst_msg, Pmp_Msg_t rply_msg) {

  int    gix;
  int    svc     = rqst_msg->Body.PortmapGet.Svc;
  char  *where   = "Pmp_Get (skltn)";


  DBG_PRNT((stdout, "\t\t\tPMP_GET\n"));

  rply_msg->Type = REPLY_PMP_GET;

  /* Get the gix */
  rply_msg->Body.ReplyPortmapGet.Reply  = PMP_get(svc, &gix, NULL);
  rply_msg->Body.ReplyPortmapGet.Gix    = gix;

  if (0 > rply_msg->Body.ReplyPortmapGet.Reply)                                goto exception;

  return;

exception:
  XPN_print(rply_msg->Body.ReplyPortmapGet.Reply);
  return;
}


     /*----------------------------------------------------------------*\
    |    Pmp_GetLoad                                                     |
    |                                                                    |
     \*----------------------------------------------------------------*/
static inline void Pmp_GetLoad(Pmp_Msg_t rqst_msg, Pmp_Msg_t rply_msg) {

  int    xpn;
  int    load;
  char  *where   = "Pmp_GetLoad (skltn)";


  DBG_PRNT((stdout, "\t\t\tPMP_GETLOAD\n"));

  rply_msg->Type = REPLY_PMP_GETLOAD;

  /* Get the load of this machine */
  if(0 > ( xpn
           = rply_msg->Body.ReplyPortmapGetLoad.Reply
           = PMP_getLoad(&load)))                                             goto exception;
  rply_msg->Body.ReplyPortmapGetLoad.CpuLoad   = load;
  return;

exception:
  XPN_print(xpn);
  return;
}



     /*\
    |   PMP_service_loop                                                 |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
void PMP_service_loop() {

  Status       status;
  Addr         addr;
  int          excpn;
  Pmp_Msg      rqst_msg;
  Pmp_Msg      rply_msg;
  int          policy;
  struct
  sched_param  schparam;
  char        *where         = "PMP_service_loop";

  pmp_thr = THR_self();
  
#if defined (HAVE_SEM_OPEN) 
  sem_post(pmp_sync);
#else
  sem_post(&pmp_sync);
#endif

  //if(pthread_getschedparam(pthread_self(), &policy, &schparam))                 {excpn = PMP_E_INTEGRITY;
   //                                                                              goto exception;}

  DBG_PRNT((stdout, "PMP_service_loop (GROUP %d, RANK %d) (AZQ thread %p):\n", getGroup(), getRank(), THR_self()));
  //DBG_PRNT((stdout, "PMP_service_loop: Policy %d. Priority %d\n", policy, schparam.sched_priority));
  //DBG_PRNT((stdout, "PMP_service_loop: running on machine %d\n", getCpuId()));

  while(1) {
    ADDR_setAny(addr);
    if (0 > (excpn = rpc_recv(&addr, (char *)&rqst_msg,
                            sizeof(Pmp_Msg),
                            &status, RPC_FOREVER))) {
      if(excpn == PMP_SK_E_DEADPART) continue;
      goto exception;
    }

    THR_setClient(&status.Src, status.SrcMchn);

    DBG_PRNT((stdout, "PMP_service_loop: After rpc_recv from [G %d  R %d] \n", status.Src.Group, status.Src.Rank));

    DBG_PRNT((stdout, "\n\t\t\t--m#%d----------------------------------------\n", getCpuId()));

    switch(rqst_msg.Type) {
      case PMP_GET:      Pmp_Get       (&rqst_msg, &rply_msg);         break;
	    case PMP_GETLOAD:  Pmp_GetLoad   (&rqst_msg, &rply_msg);         break;
 	    case PMP_SHUTDOWN:                                               goto exit;
     default:

        DBG_PRNT((stdout, "\t\t\tPMP_service: Bad Message\n"));

        rply_msg.Type                         = REPLY_PMP_OTHER;
        rply_msg.Body.ReplyPortmapOther.Reply = PMP_SK_E_INTERFACE; break;
    }

    DBG_PRNT((stdout, "\t\t\t----------------------------------------------\n"));

    if(0 > (excpn = rpc_send(&addr, (char *)&rply_msg, sizeof(Pmp_Msg),
                                   RPC_SVR)))                                   goto exception;
  }

exception:
  XPN_print(excpn);
  THR_exit(excpn);
  return;
  
exit:
  THR_exit(0);
  return;
}
