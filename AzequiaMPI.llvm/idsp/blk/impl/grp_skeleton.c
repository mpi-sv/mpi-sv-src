/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */

/*----------------------------------------------------------------*
 *   Declaration of types and functions used by this module       *
 *----------------------------------------------------------------*/
#include <config.h>

#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdlib.h>
  #include <semaphore.h>
#endif

#include <azq_types.h>
#include <xpn.h>
#include <rpc.h>
#include <com.h>
#include <thr.h>
#include <grp_sk.h>
#include <grp_hddn.h>
#include <grp.h>
#include <grp_msg.h>
#include <pmp.h>

extern void           panic(char *where);
extern pthread_key_t  key;
#define self()       ((Thr_t)pthread_getspecific(key))

/*----------------------------------------------------------------*
 *   Definition of private constants                              *
 *----------------------------------------------------------------*/
#define GRP_SK_E_OK          0
#define GRP_SK_E_EXHAUST    (GRP_SK_E_OK          - 1)
#define GRP_SK_E_INTEGRITY  (GRP_SK_E_EXHAUST     - 1)
#define GRP_SK_E_TIMEOUT    (GRP_SK_E_INTEGRITY   - 1)
#define GRP_SK_E_INTERFACE  (GRP_SK_E_TIMEOUT     - 1)
#define GRP_SK_E_SYSTEM     (GRP_SK_E_INTERFACE   - 1)
#define GRP_SK_E_SIGNALED   (GRP_SK_E_SYSTEM      - 1)
#define GRP_SK_E_DEADPART   (GRP_SK_E_SIGNALED    - 1)

/* Initial size of thread server buffer for receiving requests */
#define DFLT_GRP_BUFF_SZ       (512)

/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
int    GRP_doneGix;
int    GRP_doneSize;
Addr   GRP_doneFatherAddr;
int    GRP_doneFatherMchn;
int    GRP_done        = FALSE;
int    GRP_dont_reply  = FALSE;
static char *e_names[8] = {  /*  0 */ "GRP_SK_E_OK",
                             /*  1 */ "GRP_SK_E_EXHAUST",
                             /*  2 */ "GRP_SK_E_INTEGRITY",
                             /*  3 */ "GRP_SK_E_TIMEOUT",         // This order has to be consistent
                             /*  4 */ "GRP_SK_E_INTERFACE",       // with former definitions
                             /*  5 */ "GRP_SK_E_SYSTEM",
                             /*  6 */ "GRP_SK_E_SIGNALED",
                             /*  7 */ "GRP_SK_E_DEADPART"
                           };

/* Thread descriptor of the GRP server */
Thr_t  grp_thr;

#if defined (HAVE_SEM_OPEN)
       sem_t  *grp_sync;
#else
       sem_t   grp_sync;
#endif

/*----------------------------------------------------------------*
 *   Declaration of private functions implemented by this module  *
 *----------------------------------------------------------------*/
static inline void Grp_Create   (Grp_Msg_t rqst_msg, void *buff, int *cnt, Grp_Msg_t rply_msg);
static inline void Grp_Join     (Grp_Msg_t rqst_msg, void *buff, int *cnt, Grp_Msg_t rply_msg);
static inline void Grp_Start    (Grp_Msg_t rqst_msg, void *buff, int *cnt, Grp_Msg_t rply_msg);
static inline void Grp_Kill     (Grp_Msg_t rqst_msg, void *buff, int *cnt, Grp_Msg_t rply_msg);
static inline void Grp_Leave    (Grp_Msg_t rqst_msg, void *buff, int *cnt, Grp_Msg_t rply_msg);
static inline void Grp_Wait     (Grp_Msg_t rqst_msg, void *buff, int *cnt, Grp_Msg_t rply_msg);
static inline void Grp_Wait2    (Grp_Msg_t rqst_msg, void *buff, int *cnt, Grp_Msg_t rply_msg);
static inline void Grp_Destroy  (Grp_Msg_t rqst_msg, void *buff, int *cnt, Grp_Msg_t rply_msg);
static inline void Grp_Shutdown (Grp_Msg_t rqst_msg, void *buff, int *cnt, Grp_Msg_t rply_msg);

/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/

     /*----------------------------------------------------------------*\
    |    Grp_Create                                                      |
    |                                                                    |
     \*----------------------------------------------------------------*/
static inline void Grp_Create(Grp_Msg_t rqst_msg, void *buff, int *cnt, Grp_Msg_t rply_msg) {

  int    excpn;
  int    trueGix,
         crtr,
         grpSize;
  char  *where       = "Grp_Create (skltn)";

#ifdef __GRP_DEBUG
  fprintf(stdout, "\t\tGRP_CREATE command %p \n", self());
#endif

  /* Take the parameters from the message */
  grpSize = rqst_msg->Body.GrpCreate.Size;
  trueGix = rqst_msg->Body.GrpCreate.Gix;
  crtr    = rqst_msg->Body.GrpCreate.Creator;

  if(0 > (excpn = rply_msg->Body.ReplyGrpCreate.Reply
                = GRP_create (&trueGix, (int *)buff, grpSize, crtr)))           goto exception;

  rply_msg->Type = REPLY_GRP_CREATE;
  *cnt = 0;

  return;

exception:
  XPN_print(excpn);
  return;
}


     /*----------------------------------------------------------------*\
    |    Grp_Join                                                        |
    |                                                                    |
     \*----------------------------------------------------------------*/
static inline void Grp_Join(Grp_Msg_t rqst_msg, void *buff, int *cnt, Grp_Msg_t rply_msg) {

  char    *where = "Grp_Join (skltn)";
  int      excpn;
  int      name, rank, gix, parSz;
  CommAttr commAttr;

#ifdef __GRP_DEBUG
  fprintf(stdout, "\t\tGRP_JOIN command\n");
#endif

  rply_msg->Type = REPLY_GRP_JOIN;
  *cnt = 0;

  /* Take the parameters from the message */
  name     = rqst_msg->Body.GrpJoin.Name;
  rank     = rqst_msg->Body.GrpJoin.Rank;
  gix      = rqst_msg->Body.GrpJoin.Gix;
  commAttr = rqst_msg->Body.GrpJoin.CommAttr;

  if(0 > (excpn = rply_msg->Body.ReplyGrpJoin.Reply
                = GRP_join(gix, rank, name, &commAttr)))           goto exception;
  return;

exception:
  XPN_print(excpn);
  return;
}


     /*----------------------------------------------------------------*\
    |    Grp_Kill                                                        |
    |                                                                    |
     \*----------------------------------------------------------------*/
static inline void Grp_Kill(Grp_Msg_t rqst_msg, void *buff, int *cnt, Grp_Msg_t rply_msg) {

  int      gix = rqst_msg->Body.GrpKill.Gix;
#ifdef __GRP_DEBUG
  Addr        clientAddr;
  int         clientMchn;
  THR_getClient(&clientAddr, &clientMchn);
  fprintf(stdout, "\t\tGRP_KILL command (from [%x, %d])\n", clientAddr.Group, clientAddr.Rank);
#endif
  GRP_kill(gix);
  rply_msg->Type = REPLY_GRP_KILL;
  *cnt = 0;
  return;
}




     /*----------------------------------------------------------------*\
    |    Grp_Leave                                                       |
    |                                                                    |
     \*----------------------------------------------------------------*/
extern  int GRP_SK_leave(int exitCode); /* To avoid compiler warning */
static inline void Grp_Leave(Grp_Msg_t rqst_msg, void *buff, int *cnt, Grp_Msg_t rply_msg) {

  int retCode = rqst_msg->Body.GrpLeave.ExitCode;
#ifdef __GRP_DEBUG
  Addr        clientAddr;
  int         clientMchn;
  THR_getClient(&clientAddr, &clientMchn);
  fprintf(stdout, "\t\tGRP_LEAVE command (from [%x, %d])\n", clientAddr.Group, clientAddr.Rank);
#endif
  GRP_leave(retCode);
  rply_msg->Type = REPLY_GRP_LEAVE;
  *cnt = 0;
  return;
}



     /*----------------------------------------------------------------*\
    |    Grp_Destroy                                                     |
    |                                                                    |
     \*----------------------------------------------------------------*/
static inline void Grp_Destroy(Grp_Msg_t rqst_msg, void *buff, int *cnt, Grp_Msg_t rply_msg) {

  int      gix      = rqst_msg->Body.GrpDestroy.Gix;

#ifdef __GRP_DEBUG
  Addr        clientAddr;
  int         clientMchn;
  THR_getClient(&clientAddr, &clientMchn);
  fprintf(stdout, "\t\tGRP_DESTROY command (from [%x, %d])\n", clientAddr.Group, clientAddr.Rank);
#endif

  GRP_destroy(gix);
  *cnt = 0;
  return;
}


     /*----------------------------------------------------------------*\
    |    Grp_Shutdown                                                    |
    |                                                                    |
     \*----------------------------------------------------------------*/
static inline void Grp_Shutdown(Grp_Msg_t rqst_msg, void *buff, int *cnt, Grp_Msg_t rply_msg) {

#ifdef __GRP_DEBUG
  Addr        clientAddr;
  int         clientMchn;
  THR_getClient(&clientAddr, &clientMchn);
  fprintf(stdout, "\t\tGRP_SHUTDOWN command (from [%x, %d])\n", clientAddr.Group, clientAddr.Rank);
#endif

  GRP_shutdown();
  *cnt = 0;

  return;
}


     /*----------------------------------------------------------------*\
    |    Grp_Wait                                                        |
    |                                                                    |
     \*----------------------------------------------------------------*/
static inline void Grp_Wait(Grp_Msg_t rqst_msg, void *buff, int *cnt, Grp_Msg_t rply_msg) {

  int      excpn;
  int      gix      = rqst_msg->Body.GrpWait.Gix;
  char    *where    = "Grp_Wait (skltn)";

#ifdef __GRP_DEBUG
  Addr        clientAddr;
  int         clientMchn;
  THR_getClient(&clientAddr, &clientMchn);
  fprintf(stdout, "\t\tGRP_WAIT command (from [%x, %d])\n", clientAddr.Group, clientAddr.Rank);
#endif

  rply_msg->Type = REPLY_GRP_WAIT;
  if(0 > (excpn = rply_msg->Body.ReplyGrpWait.Reply
                = GRP_wait(gix, (int *)buff)))                                  goto exception;
  return;

exception:
  XPN_print(excpn);
  return;
}


     /*----------------------------------------------------------------*\
    |    Grp_Wait2                                                       |
    |                                                                    |
     \*----------------------------------------------------------------*/
static inline void Grp_Wait2(Grp_Msg_t rqst_msg, void *buff, int *cnt, Grp_Msg_t rply_msg) {

  int      excpn;
  int      gix      = rqst_msg->Body.GrpWait.Gix;
  char    *where    = "Grp_Wait2 (skltn)";

#ifdef __GRP_DEBUG
  Addr        clientAddr;
  int         clientMchn;
  THR_getClient(&clientAddr, &clientMchn);
  fprintf(stdout, "\t\tGRP_WAIT2 command (from [%x, %d])\n", clientAddr.Group, clientAddr.Rank);
#endif

  rply_msg->Type = REPLY_GRP_WAIT;
  *cnt = sizeof(int) * rqst_msg->Body.GrpWait.GrpSize;
  if(0 > (excpn = rply_msg->Body.ReplyGrpWait.Reply
                = GRP_wait2(gix, (int *)buff)))                                 goto exception;

  return;

exception:
  XPN_print(excpn);
  return;
}


     /*----------------------------------------------------------------*\
    |*   Grp_Start                                                      *|
    |*                                                                  *|
     \*----------------------------------------------------------------*/
static inline void Grp_Start(Grp_Msg_t rqst_msg, void *buff, int *cnt, Grp_Msg_t rply_msg) {

  int  reply;
  int  gix    = rqst_msg->Body.GrpStart.Gix;

#ifdef __GRP_DEBUG
  fprintf(stdout, "\t\tGRP_START command\n");
#endif
  reply = GRP_start(gix);
  rply_msg->Type                     = REPLY_GRP_START;
  rply_msg->Body.ReplyGrpStart.Reply = reply;
  *cnt = 0;
  return;
}


/*----------------------------------------------------------------*
 *                                                                *
 *----------------------------------------------------------------*/
void GRP_service_loop() {
  bool old_chk_flag=  klee_disable_sync_chk(0);
  Addr         clientAddr;
  int          clientMchn;
  char        *buff;
  int          size = DFLT_GRP_BUFF_SZ;
  Hdr          hdr;
  int          policy;
  struct
  sched_param  schparam;
  int          ret          = 0;
  Grp_Msg     *rqst_msg     = (Grp_Msg *)((int *)&hdr.Store);
  Grp_Msg     *rply_msg     = (Grp_Msg *)((int *)&hdr.Store);
  char        *where        = "GRP_service_loop";

  grp_thr = THR_self();
  
#if defined (HAVE_SEM_OPEN) 
  sem_post(grp_sync);
#else
  sem_post(&grp_sync);
#endif

#ifdef __GRP_DEBUG
  fprintf(stdout, "GRP_service_loop (GROUP %x, RANK %d) (AZQ thread %p):\n", getGroup(), getRank(), grp_thr);
#endif
  
  //if(pthread_getschedparam(pthread_self(), &policy, &schparam))                 panic("");

  if (NULL == (buff = malloc(size)))                                    goto exception;

  while(1) {
    if(GRP_done) { /* Previous service, either GRP_LEAVE or GRP_WAIT,
                    * left work to be done by Grp_Wait2 */
      GRP_done = FALSE;
      hdr.Mode = RPC_FULL;
      rqst_msg->Type                 = GRP_WAIT2;
      rqst_msg->Body.GrpWait.Gix     = GRP_doneGix;
      rqst_msg->Body.GrpWait.GrpSize = GRP_doneSize;
      THR_setClient(&GRP_doneFatherAddr, GRP_doneFatherMchn);
    }
    else {
      if(0 > (ret = RPC_recv(&hdr, (void **) &buff, &size))) {
        if(ret == RPC_E_DEADPART) {
          fprintf(stdout, "GRP_service_loop %p: DEADPART \n", THR_self());
          continue;
        }
        else                                                                    goto exception;
      }
    }
#ifdef __GRP_DEBUG
    fprintf(stdout, "\n\t\t^--m#%d---------------------------------------\n", getCpuId());
#endif
    GRP_dont_reply = ((hdr.Mode & RPC_HALF) ? TRUE : FALSE);
    switch(rqst_msg->Type) {
      case GRP_CREATE:   Grp_Create   (rqst_msg, buff, &ret, rply_msg);  break;
      case GRP_JOIN:     Grp_Join     (rqst_msg, buff, &ret, rply_msg);  break;
      case GRP_START:    Grp_Start    (rqst_msg, buff, &ret, rply_msg);  break;
      case GRP_KILL:     Grp_Kill     (rqst_msg, buff, &ret, rply_msg);  break;
      case GRP_LEAVE:    Grp_Leave    (rqst_msg, buff, &ret, rply_msg);  break;
      case GRP_WAIT:     Grp_Wait     (rqst_msg, buff, &ret, rply_msg);  break;
      case GRP_WAIT2:    Grp_Wait2    (rqst_msg, buff, &ret, rply_msg);  break;
      case GRP_DESTROY:  Grp_Destroy  (rqst_msg, buff, &ret, rply_msg);  break;
      case GRP_SHUTDOWN: Grp_Shutdown (rqst_msg, buff, &ret, rply_msg);  goto exit;
      default:
        rply_msg->Type = REPLY_GRP_OTHER;
        rply_msg->Body.ReplyGrpOther.Reply = GRP_SK_E_INTERFACE;    break;
    }
#ifdef __GRP_DEBUG
    fprintf(stdout, "\t\tv---------------------------------------------\n");
#endif

    if (GRP_dont_reply)                 { continue; }
    THR_getClient(&clientAddr, &clientMchn);
    if(0 > (ret = RPC_send(&hdr, (void *)buff, ret))) {
      switch(ret) {
        case RPC_E_DEADPART:
          THR_getClient(&clientAddr, &clientMchn);
          fprintf(stdout, "\t\tRPC_E_DEADPART rank %d ----------------------------------------------\n", clientAddr.Rank);
          continue;
          break;
        default:
          goto exception;
      }
    }
  }

exception:
  XPN_print(ret);
  RPC_unregister(GRP_PORT);
  panic(where);

exit:
#ifdef __GRP_DEBUG
  fprintf(stdout, "GRP_server shutdown in machine %d \n", getCpuId());
#endif
  if (buff) free(buff);
  RPC_unregister(GRP_PORT);
  PMP_shutdown();
  if(old_chk_flag) klee_enable_sync_chk(0);
  THR_exit(GRP_E_OK);
}
