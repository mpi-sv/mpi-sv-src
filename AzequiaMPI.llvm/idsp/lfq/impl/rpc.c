/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   implemented by this module                                   *
 *----------------------------------------------------------------*/
#include <rpc.h>


/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/

#if defined (__OSI)
  #include <osi.h>
#else
  #include <stdlib.h>
  #include <pthread.h>
  #include <errno.h>
  #include <string.h>
  #include <stdio.h>
#endif

#include <rqst.h>
#include <rqst_rpc.h>
#include <azq.h>
#include <azq_types.h>
#include <xpn.h>
#include <addr.h>
#include <pmp.h>
#include <mbx.h>
#include <mbx_rpc.h>
#include <util.h>
#include <inet.h>

extern  void panic           (char *where);

/*----------------------------------------------------------------*
 *   Definition of private constants                              *
 *----------------------------------------------------------------*/
#define self()             ((Thr_t)pthread_getspecific(key))

#define SVC_LOC_MAX         16
#define NONE_SATISFIED     (-1)

#ifdef __RPC_DEBUG
#define DBG_PRNT(pmsg) \
{ \
   fprintf pmsg \
   ; fflush(stdout); \
}
#else
#define DBG_PRNT(pmsg)
#endif

/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
extern pthread_key_t   key;

static int             initialised = FALSE;

static int           (*getEnd)(Thr_t srcThr, Addr_t dst, int *mchn, void *thr);

static char *e_names[8] = {  /* This order has to be consistent with rpc.h */
                             /*  0 */ "RPC_E_OK",
                             /*  1 */ "RPC_E_EXHAUST",
                             /*  2 */ "RPC_E_INTEGRITY",
                             /*  3 */ "RPC_E_TIMEOUT",
                             /*  4 */ "RPC_E_INTERFACE",
                             /*  5 */ "RPC_E_SYSTEM",
                             /*  6 */ "RPC_E_SIGNALED",
                             /*  7 */ "RPC_E_DEADPART"
                           };


/*----------------------------------------------------------------*
 *   Declaration of private functions implemented by this module  *
 *----------------------------------------------------------------*/
static inline int locateService(int svc, int *mchn, int *gix);

extern int  rpc_deal_send   (Rqst_t rqst, unsigned timeout);
extern int  rpc_deal_recv   (Rqst_t rqst);
extern int  rpc_deliver     (INET_iovec *iov, int last_frgmt, int *success);
extern int  rpc_timed_wait  (Rqst_t *rqst, Status *status, unsigned  timeout);



/*----------------------------------------------------------------*
 *   SvcCache package (Begin)                                     *
 *----------------------------------------------------------------*/
/*----------------------------------------------------------------*
 *   Definition of private types                                  *
 *----------------------------------------------------------------*/
/* Service Table: Keeps the services this machine is USING
   1. Each entry (locator) has a machine where the service runs
      and the address of the daemon in such machine
   2. Consider that a service may have more than one locator
      because different threads may choose different servers
      of the same service
 */
struct SvcDptr {
  int    Allocated;
  int    Svc;
  int    Mchn;
  int    Gix;
};
typedef struct SvcDptr SvcDptr, *SvcDptr_t;

struct SvcDptrTable {
  int              AllocCnt;
  SvcDptr_t         SvcDptr;
};
typedef struct SvcDptrTable SvcDptrTable;

/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
static SvcDptrTable     svcDptrTable;

/*----------------------------------------------------------------*
 *   Function interface                                           *
 *----------------------------------------------------------------*/
static        int svcDptrFind (int svc, int *mchn, int *gix);
static inline int svcDptrAlloc(int svc, int  mchn, int  gix);

     /*-----------------------------------------------------------------\
    |    svcDptrFind                                                      |
    |    Look up the locator of service "svc" running in machine "mchn"  |
    |                                                                    |
     \*----------------------------------------------------------------*/
static int svcDptrFind (int svc, int *mchn, int *gix) {

  int i;

  for(i = 0; i < SVC_LOC_MAX; i++) {
    if(svcDptrTable.SvcDptr[i].Allocated) {  // Try just allocated entries
      if(svcDptrTable.SvcDptr[i].Svc == svc) {
        if(*mchn == DFLT_MCHN) {
          *gix  = svcDptrTable.SvcDptr[i].Gix;
          *mchn = svcDptrTable.SvcDptr[i].Mchn;
          return (TRUE);
        }
        if(svcDptrTable.SvcDptr[i].Mchn == *mchn) {
          if(gix)
            *gix = svcDptrTable.SvcDptr[i].Gix;
          return (TRUE);
        }
      }
    }
  }
  return(FALSE);
}


     /*-----------------------------------------------------------------\
    |    svcDptrAlloc                                                     |
    |                                                                    |
    |                                                                    |
     \*----------------------------------------------------------------*/
static inline int svcDptrAlloc(int svc, int mchn, int gix) {

  int  i;

  /* [1] Test if the triple already exists */
  if(svcDptrFind(svc, &mchn, NULL))
    return(RPC_E_OK);

  /* [2] Create the triple */
  for(i = 0; i < SVC_LOC_MAX; i++) {
    if(!svcDptrTable.SvcDptr[i].Allocated) {  /* Free entry found !! */
	    svcDptrTable.SvcDptr[i].Svc       = svc;
	    svcDptrTable.SvcDptr[i].Mchn      = mchn;
	    svcDptrTable.SvcDptr[i].Gix       = gix;
      svcDptrTable.SvcDptr[i].Allocated = TRUE;
      svcDptrTable.AllocCnt++;
      return(RPC_E_OK);
    }
  }
  return(RPC_E_EXHAUST);
}

/*----------------------------------------------------------------*
 *   SvcCache package (End)                                       *
 *----------------------------------------------------------------*/


/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/

     /*-----------------------------------------------------------------\
    |    locateService                                                   |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o svc  (Input)                                                  |
    |        The identifier of the service                               |
    |    o mchn (Input-Output)                                           |
    |        On Input: The machine where the service runs                |
    |          . DFLT_MCHN or                                            |
    |          . A machine (0, 1, 2, 3, ...)                             |
    |        On Output: a machine (0, 1, 2, 3, ...)                      |
    |                  (Valid only if Input = DFLT_MCHN)                 |
    |    o gix  (Output)                                                 |
    |        The GROUP identifier of the task paying the service         |
    |                                                                    |
    |    RETURN:                                                         |
    |    0    : On success                                               |
    |    <  0 : On error                                                 |
    |                                                                    |
     \*----------------------------------------------------------------*/
static inline int locateService(int svc, int *mchn, int *gix) {

  int      excpn;

  DBG_PRNT((stdout, "locateService: [Port %d, Mchn %d] BEGIN\n", svc, *mchn));

  /* [1] Test if the location object of "svc" already exists */
  if(svcDptrFind(svc, mchn, gix)) {
    DBG_PRNT((stdout, "locateService: End\n"));
    return(RPC_E_OK);
  }

  /* [2] Look for a machine that pays the service */
  if(0 > (excpn = PMP_get(svc, gix, mchn)))                                     return(excpn);

  /* [3] Annotate the location object */
  if(0 > svcDptrAlloc(svc, *mchn, *gix))                                         return(-1);

  DBG_PRNT((stdout, "locateService: End\n"));

  return(RPC_E_OK);
}


     /*-----------------------------------------------------------------\
    |    getClient                                                       |
    |                                                                    |
    |                                                                    |
     \-----------------------------------------------------------------*/
static inline void getClient(Thr_t me, Addr *addr, int *mchn) {

  *addr = me->ClientAddr;
  *mchn = me->ClientMchn;

  return;
}


      /*________________________________________________________________
     /                                                                  \
    |    rpc_send                                                        |
    |                                                                    |
    |    Send the data in  "buff"                                        |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o dstAddr  (Input)                                              |
    |        The desired destination address                             |
    |    o buff   (Input)                                                |
    |        User buffer of outgoing data                                |
    |    o cnt    (Input)                                                |
    |        Input:  Size of "buffer"                                    |
    |    o mode     (Input)                                              |
    |        Bitmap with COM_RPC_SVR on if I am a RPC server sending     |
    |        a reply. Off if I am a RPC client sending a request         |
    |        (See com.h)                                                 |
    |                                                                    |
    |    RETURN:                                                         |
    |    >= 0 : Number of bytes received in buffer                       |
    |    <  0 : On error                                                 |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int rpc_send(const Addr_t dst, char *buff, int cnt, int mode) {

  Thr_t     dstThr;
  int       dstMchn;
  int       dummy;
  Rqst_t    rqst;
  Thr_t     me        = self();
  int       excpn;
  static
  char     *where     = "rpc_send";


//  fflush(stdout); usleep(100000);
  
  /* 1. Init */
  if(mode & RPC_SVR) {
    /* I am a RPC server sending a reply */
    getClient(me, dst, &dstMchn);
  } else {
    /* I am a RPC client sending a request */
    dstMchn = dst->Rank;
    dst->Rank = 0;
  }
  DBG_PRNT((stdout, "rpc_send(%p): From [G: %x, R: %d, M: %d] To [G: %x, R: %d, M: %d\n", 
                              me, getGroup(), getRank(), getCpuId(), dst->Group, dst->Rank, dstMchn));

  if (!INET_by(dstMchn)) {
    if(0 > getEnd(me, dst, &dummy, &dstThr))                                    {excpn = RPC_E_INTERFACE;
								                         goto exception;}
  }
	  
  rqst = &(me->SyncRqst);
  RQST_initRpcSend(rqst, 
                  dst,
                  &me->Address,
                  0,
                  buff,
                  cnt,
                  dstMchn,
                  me,
                  dstThr);

  /* 2. Start */
  if (0 > (excpn = rpc_deal_send(rqst, COM_FOREVER)))                               goto exception;

  /* 3. Wait */
  if (0 > (excpn = rpc_timed_wait(&rqst, AZQ_STATUS_IGNORE, COM_FOREVER)))          goto exception;

  DBG_PRNT((stdout, "rpc_send(%p): End\n", me));

  return RPC_E_OK;

exception:
  XPN_print(excpn);
  return(azq_err[-excpn]);
}


      /*________________________________________________________________
     /                                                                  \
    |    rpc_recv                                                        |
    |                                                                    |
    |    Receive the data in  "buff"                                     |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o srcAddr  (Input)                                              |
    |        The desired source address                                  |
    |    o buff   (Input)                                                |
    |        User buffer of incoming data                                |
    |    o cnt    (Input)                                                |
    |        Input:  Size of "buffer"                                    |
    |    o status (Output)                                               |
    |                                                                    |
    |    RETURN:                                                         |
    |    >= 0 : Number of bytes received in buffer                       |
    |    <  0 : On error                                                 |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int rpc_recv(const Addr_t src, char *buff, int cnt, Status *status, unsigned timeout) {

  Thr_t      me         = self();
  Rqst      *rqst;
  int        excpn;
  static
  char      *where      = "rpc_recv";

  
  /*** QUITARLO ***/
  timeout=RPC_FOREVER;
  
  
  DBG_PRNT((stdout, "rpc_recv(%p): From [%x, %d]. Timeout %x\n", self(), src->Group, src->Rank, timeout));

  /* [1]. Init */
  rqst = &(me->SyncRqst);
  RQST_initRpcRecv(rqst, 
                  src,
                  &me->Address,
                  0,
                  buff,
                  cnt,
                  DFLT_MCHN,
                  me,
                  NULL);

  /* 2. Start */
  if (0 > (excpn = rpc_deal_recv(rqst)))                                           goto exception;

  /* 3. Wait */
  if (0 > (excpn = rpc_timed_wait(&rqst, status, timeout)))                        goto exception;

  DBG_PRNT((stdout, "rpc_recv(%p): End\n", me));

  return RPC_E_OK;

exception:
  XPN_print(excpn);
  return(azq_err[-excpn]);
}


/*----------------------------------------------------------------*
 *   Implementation of public interface                           *
 *----------------------------------------------------------------*/

      /*________________________________________________________________
     /                                                                  \
    |    RPC_init                                                        |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int RPC_init(void) {

  int     excpn;
  char   *where     = "RPC_init";

  DBG_PRNT((stdout, "\nRPC_init: \n"));

  if(initialised)
    return(RPC_E_OK);

  /* Make room for the triple table */
  svcDptrTable.AllocCnt = 0;
  if(NULL == (svcDptrTable.SvcDptr =
                 (SvcDptr_t)AZQ_MALLOC((SVC_LOC_MAX)*sizeof(SvcDptr))))       {excpn = RPC_E_SYSTEM;
                                                                               goto exception;}
  memset(svcDptrTable.SvcDptr, 0, (SVC_LOC_MAX)*sizeof(SvcDptr));

  DBG_PRNT((stdout, "SVC_LOC_MAX: (%d)  %ld\n", SVC_LOC_MAX, SVC_LOC_MAX * sizeof(SvcDptr)));

  if(0 > (excpn = INET_subscribe(rpc_deliver, RPC_PROTOCOL)))                      goto exception;

  DBG_PRNT((stdout, "RPC_init: Subscribed %p for protocol %d. End\n", rpc_deliver, RPC_PROTOCOL));

  if (0 > (excpn = PMP_init()))                                                goto exception;
  
  DBG_PRNT((stdout, "RPC_init: End\n"));

  initialised = TRUE;
  return(RPC_E_OK);

exception:
  XPN_print(excpn);
  return(excpn);
}


      /*________________________________________________________________
     /                                                                  \
    |    RPC_finalize                                                    |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
void RPC_finalize(void) {

  PMP_finalize();

  if (svcDptrTable.SvcDptr)
    AZQ_FREE(svcDptrTable.SvcDptr);
}


      /*________________________________________________________________
     /                                                                  \
    |    RPC_send                                                        |
    |    Asynchronous send for RPC communication                         |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o buffer   (Input)                                              |
    |        User buffer to send                                         |
    |    o count    (Input)                                              |
    |        Size of "buffer"                                            |
    |    o dst      (Input)                                              |
    |        Destination entity:                                         |
    |        a) The client operator to reply                             |
    |        b) A [machine, service] pair                                |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int RPC_send(Hdr_t hdr, void *buff, int cnt) {

  int      excpn;
  Addr     addr;
  char    *where    = "RPC_send";

  DBG_PRNT((stdout, "RPC_send(%p): BEGIN\n", self()));

  if(0 > (excpn = rpc_send(&addr, (char *)hdr, sizeof(Hdr), RPC_SVR)))          goto exception;
  if(0 > (excpn = rpc_send(&addr, buff, cnt, RPC_SVR)))                         goto exception;

  DBG_PRNT((stdout, "RPC_send(%p): END\n", self()));

  return(excpn);

exception:
  XPN_print(excpn);
  return(excpn);
}


      /*________________________________________________________________
     /                                                                  \
    |    RPC_recv                                                        |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o hdr   (Input)                                                 |
    |        Only takes the service port. Overwritten by incomming       |
    |        header, that contains the service message which,in turn,    |
    |        contains the method identificator and small parameters      |
    |        of the method                                               |
    |    o buffer    (Input)                                             |
    |        Filled with an eventual  big parameter of the method        |
    |    o count    (Input)                                              |
    |        Size of "buffer"                                            |
    |                                                                    |
    |    RETURN value:                                                   |
    |      Number of bytes stored in "buff", on success                  |
    |      < 0, on error                                                 |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int RPC_recv(Hdr_t hdr, void **buff, int *cnt) {

  int       ret;
  Status    status;
  Addr      srcAddr;
  char     *where      = "RPC_recv";

  DBG_PRNT((stdout, "RPC_recv(%p): BEGIN\n", self()));

  ADDR_setAny(srcAddr);
  if(0 > (ret = rpc_recv(&srcAddr, (char *)hdr,
                                   sizeof(Hdr), &status, RPC_FOREVER)))         goto exception;
  if(status.Count != sizeof(Hdr)) {
       printf("RPC_recv: Bad Status.Count %d. I shoul be %d. Source [G %x, R %x]\n", 
               status.Count, sizeof(Hdr), status.Src.Group, status.Src.Rank);
	ret = RPC_E_INTEGRITY;
	goto exception;
  }
  srcAddr = status.Src;

  /* Dynamically adjusts the user buffer to hold the incoming data */
  if (hdr->Size > *cnt) {    
    if (NULL == (*buff = realloc(*buff, hdr->Size)))                    goto exception;
    *cnt = hdr->Size;
  }

  if(0 > (ret = rpc_recv(&srcAddr, *buff, *cnt, &status, RPC_FOREVER)))   goto exception;
  THR_setClient(&status.Src, status.SrcMchn);

  DBG_PRNT((stdout, "RPC_recv(%p): END\n", self()));

  return(ret);

exception:
  XPN_print(ret);
  return(ret);
}


      /*________________________________________________________________
     /                                                                  \
    |    RPC_trans                                                       |
    |                                                                    |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int RPC_trans(Hdr_t hdr, void *buff1, int cnt1, void *buff2, int cnt2) {

  int        excpn;
  Addr       addr;
  Status     status;
  Thr_t      me         = THR_self();
  Port_t     port       = &hdr->Port;
  char      *where      = "RPC_trans";
  int        mode       = 0;

  DBG_PRNT((stdout, "RPC_trans(%p): BEGIN\n", self()));

  THR_setmask(me, MASK_ON);
  if(0 > (excpn = locateService(port->Port, &port->Mchn, &addr.Group)))         goto exception;

  DBG_PRNT((stdout, "\tRPC_trans: Port %x served by [G %x, R %d] in Mchn: %d\n", port->Port, addr.Group, addr.Rank, port->Mchn));

  addr.Rank  = port->Mchn;
  if(0 > (excpn = rpc_send(&addr, (char *)hdr, sizeof(Hdr), mode)))             goto exception;
  addr.Rank  = port->Mchn;
  if(0 > (excpn = rpc_send(&addr, buff1, cnt1, mode)))                          goto exception;
  if(hdr->Mode & RPC_HALF) {
    goto retorno;
  }
  addr.Rank = 0;
  if(0 > (excpn = rpc_recv(&addr, (char *)hdr,
                            sizeof(Hdr), &status, RPC_FOREVER)))                goto exception;
  if(0 > (excpn = rpc_recv(&addr, (char *)buff2, cnt2, &status, RPC_FOREVER)))  goto exception;

retorno:
  THR_setmask(me, MASK_OFF);

  DBG_PRNT((stdout, "\tRPC_trans(%p): END\n", THR_self()));

  return(excpn);

exception:
  THR_setmask(me, MASK_OFF);
  XPN_print(excpn);
  return(excpn);
}


      /*________________________________________________________________
     /                                                                  \
    |    RPC_register                                                    |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int RPC_register(int port, int gix) {

  int       excpn;
  char     *where     = "RPC_register";

  if((port == NO_PORT))
    return(RPC_E_OK);

  DBG_PRNT((stdout, "RPC_register: \n"));

  if(0 > (excpn = svcDptrAlloc(port, getCpuId(), gix)))                        goto exception;
  if(0 > (excpn = PMP_register(port, gix)))                                    goto exception;

  DBG_PRNT((stdout, "RPC_register: End\n"));

  return(RPC_E_OK);

exception:
  XPN_print(excpn);
  DBG_PRNT((stdout, "RPC_register: End\n"));
  return(excpn);
}


      /*________________________________________________________________
     /                                                                  \
    |    RPC_unregister                                                  |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
void RPC_unregister(int port) {
  PMP_unregister(port);
  return;
}


      /*________________________________________________________________
     /                                                                  \
    |    RPC_setLoc                                                      |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
void RPC_setLoc(int (*f)(void *srcThr, Addr_t dstAddr, int *mchn, void *thr)) {
  getEnd = (int (*)(Thr_t srcThr, Addr_t dst, int *mchn, void *thr))f;
  return;
}


      /*________________________________________________________________
     /                                                                  \
    |    RPC_getServer                                                   |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int RPC_getServer(int port, int *gix) {

  int     excpn;
  int     myMchn    = getCpuId();
  char   *where     = "RPC_getServer";

  DBG_PRNT((stdout, "RPC_getServer(%p): BEGIN\n", self()));

  if(0 > (excpn = locateService(port, &myMchn, gix)))                           goto exception;
  DBG_PRNT((stdout, "RPC_getServer(%p): END\n", self()));
  return(RPC_E_OK);

exception:
  XPN_print(excpn);
  DBG_PRNT((stdout, "RPC_getServer(%p): END\n", self()));
  return(excpn);
}
