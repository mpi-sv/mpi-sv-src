/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


/*----------------------------------------------------------------*
 *   Declaration of public functions implemented by this module   *
 *----------------------------------------------------------------*/
#include <grp.h>


/*----------------------------------------------------------------*
 *   Declaration of types and functions used by this module       *
 *----------------------------------------------------------------*/
#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
#endif

#include <azq_types.h>
#include <util.h>
#include <thr.h>
#include <com.h>
#include <rpc.h>
#include <grp_msg.h>
#include <xpn.h>
#include <pmp.h>


/* Package-wide functions implemented in grp.c */
extern int  getEnd(Thr_t srcThr, Addr_t dst, int *mchn, Thr_t *thr);
extern void newGix(int *gix);

extern pthread_key_t  key;
#define self()       ((Thr_t)pthread_getspecific(key))

/*----------------------------------------------------------------*
 *   Definition of private constants                              *
 *----------------------------------------------------------------*/


/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
static char *e_names[8] = {  /*  0 */ "GRP_E_OK",
                             /*  1 */ "GRP_E_EXHAUST",
                             /*  2 */ "GRP_E_INTEGRITY",
                             /*  3 */ "GRP_E_TIMEOUT",         // This order has to be consistent
                             /*  4 */ "GRP_E_INTERFACE",       // with grp.h
                             /*  5 */ "GRP_E_SYSTEM",
                             /*  6 */ "GRP_E_SIGNALED",
                             /*  7 */ "GRP_E_DEADPART"
                           };


/*----------------------------------------------------------------*
 *   Declaration of private functions implemented by this module  *
 *----------------------------------------------------------------*/
static int  GRP_leave   (int code);


/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/
int sendToMachine(int gix, int rank, int *machine, int *send) {

  int           grpSize,
                j, maq,
                excpn,
                myMchn       = getCpuId();
  static char  *where        = "sendToMachine";

  if(0 > (excpn = GRP_getSize(gix, &grpSize)))                                  goto exception;
  if(rank < grpSize) {
    if(0 > (excpn = GRP_getMchn(gix, rank, machine)))                           goto exception;
    if(*machine == myMchn) {
      *send = FALSE;
      return(GRP_E_OK);
    }
    for(j = 0; j < rank; j++) {
      if(0 > (excpn = GRP_getMchn(gix, j, &maq)))                               goto exception;
      if(maq == *machine) {
        *send = FALSE;
        return(GRP_E_OK);
      }
    }
  }
  else
    *machine = myMchn;
  *send = TRUE;
  return(GRP_E_OK);

exception:
  XPN_print(excpn);
  return(excpn);
}


      /*________________________________________________________________
     /                                                                  \
    |    GRP_abandone                                                    |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
void GRP_abandone(int exitCode) {

  int gix = getGroup();
  int grp_gix;

  RPC_getServer(GRP_PORT, &grp_gix);
  if ((gix == PMP_GIX) || (gix == grp_gix)) return;

  if(exitCode >= 0) {
    GRP_leave(exitCode);
  }
  else {
    //GRP_kill(gix);
    GRP_leave(exitCode);
  }
}


     /*¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\
    |   GRP_leave                                                        |
    |                                                                    |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o exitCode  (Input)                                              |
    |        Exit code of the invoking thread                            |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
static int GRP_leave(int exitCode) {

  Hdr          hdr;
  int          creatorMchn,
               excpn,
               grpSize;
  Grp_Msg     *msg          = (Grp_Msg *)((int *)&hdr.Store);
  int          gix          = getGroup();
  char        *where        = "GRP_leave (clnt)";

  if(0 > (excpn = GRP_getSize(gix, &grpSize)))                                  goto exception;
  if(0 > (excpn = GRP_getMchn(gix, grpSize, &creatorMchn)))                     goto exception;
#ifdef __GRP_DEBUG
  fprintf(stdout, "\n\t^------------------------\n");
  fprintf(stdout, "\tGRP_leave (stub): Begin  (%p)([%x, %d])\n", THR_self(), gix, getRank());
#endif
  hdr.Port.Port          = GRP_PORT;
  hdr.Port.Mchn          = creatorMchn;
  hdr.Mode               = RPC_HALF;
  hdr.Size               = 0;
  msg->Type              = GRP_LEAVE;
  msg->Body.GrpLeave.ExitCode = exitCode;
  if(0 > (excpn = RPC_trans(&hdr, (void *) NULL, 0, (void *) NULL, 0)))         goto exception;
#ifdef __GRP_DEBUG
  fprintf(stdout, "\tGRP_leave (stub): End    (%p)([%x, %d]) pth %p: Adios \n", THR_self(), gix, getRank(), (void *)pthread_self());
  fprintf(stdout, "\tv-------------------------\n");
#endif
  return(GRP_E_OK);

exception:
  XPN_print(excpn);
  return(excpn);
}


/*----------------------------------------------------------------*
 *   Implementation of public functions                           *
 *----------------------------------------------------------------*/

     /*¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\
    |   GRP_create                                                       |
    |                                                                    |
    |   Create a group object in the target machines and in the          |
    |   invoking machine.                                                |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o gix      (Input)                                              |
    |        Random identificator of the group                           |
    |    o mchne    (Input)                                              |
    |        vector of machines ordered by rank                          |
    |        For instance, mchne[i] contains the machine where thread    |
    |        with rank i will run                                        |
    |    o size     (Input)                                              |
    |        Number of threads of the group. It is also the size of      |
    |        "machine"                                                   |
    |    o creat    (Input)                                              |
    |        Not used here                                               |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int GRP_create(int *gix, int *mchne, int size, int creat) {

  int         dstMchn;
  int         i, j, repetida,
              excpn;
  Hdr         hdr;
  Grp_Msg    *msg        = (Grp_Msg *)((int *)&hdr.Store);
  int         creator    = getCpuId();
  char       *where      = "GRP_create (clnt)";

  if(*gix == GIX_NONE)
    newGix(gix);

  /*      Build and send the GRP_CREATE message to the set of machines
   *      "mchne" in order to create in each one an instance of the group object.
   *      Send the message also to the local (creator) machine.
   */
  for(i = 0; i < size + 1; i++) {
    if(i < size) dstMchn = mchne[i];
    else         dstMchn = creator;
    repetida = FALSE;
    for(j = 0; j < i; j++) {
      if(mchne[j] == dstMchn) {
        repetida = TRUE;
        break;
      }
    }
    if(repetida)
      continue;

    hdr.Port.Port               = GRP_PORT;
    hdr.Port.Mchn               = dstMchn;
    hdr.Mode                    = RPC_FULL;
    hdr.Size                    = size * sizeof(int);
    msg->Type                   = GRP_CREATE;
    msg->Body.GrpCreate.Size    = size;
    msg->Body.GrpCreate.Creator = creator;
    msg->Body.GrpCreate.Gix     = *gix;
#ifdef __GRP_DEBUG
    fprintf(stdout, "\n\t^------------------------\n");
    fprintf(stdout, "\tGRP_create (stub): begin %p \n", self());
#endif
    if(0 > (excpn = RPC_trans(&hdr, (void *)mchne, size*sizeof(int), NULL, 0))) goto exception;
#ifdef __GRP_DEBUG
    fprintf(stdout, "\tGRP_create (stub): end   %p \n", self());
    fprintf(stdout, "\tv-------------------------\n");
#endif
    if(0 > (excpn = msg->Body.ReplyGrpCreate.Reply))                            goto exception;
  }
  return(GRP_E_OK);

exception:
  XPN_print(excpn);
  return(excpn);
}


     /*¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\
    |   GRP_join                                                         |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int GRP_join(int gix, int rank, int name, CommAttr *commAttr) {

  int        excpn;
  int        mchn;
  long       dummy;
  Hdr        hdr;
  int        offset;
  int        i;
  Addr       dst;
  Grp_Msg   *msg      = (Grp_Msg *)((int *)&hdr.Store);
  char      *where    = "GRP_join (clnt)";

#ifdef __GRP_DEBUG
  fprintf(stdout, "\n\t^------------------------\n");
  fprintf(stdout, "\tGRP_join (stub): begin %p \n", self());
#endif

  dst.Group = gix;
  dst.Rank  = rank;
  
  /* Get the machine where the operator is going to run */
  if(0 > (excpn = getEnd(self(), (Addr_t)&dst, (int *)&mchn, (void *)&dummy)))                
																			   goto exception;

  /*  Build and send the GRP_JOIN request to the target machine */
  hdr.Port.Port                = GRP_PORT;
  hdr.Port.Mchn                = mchn;
  hdr.Mode                     = RPC_FULL;
  hdr.Size                     = 0;
  msg->Type                    = GRP_JOIN;
  msg->Body.GrpJoin.Gix        = gix;
  msg->Body.GrpJoin.Rank       = rank;
  msg->Body.GrpJoin.Name       = name;

  if(commAttr)
    msg->Body.GrpJoin.CommAttr   = *commAttr;

  if(0 > (excpn = RPC_trans(&hdr, (void *) NULL, 0,
                                  (void *) NULL, 0)))                          goto exception;

  if(0 > (excpn = msg->Body.ReplyGrpJoin.Reply))                               goto exception;

#ifdef __GRP_DEBUG
  fprintf(stdout, "\tGRP_join (stub): end   %p \n", self());
  fprintf(stdout, "\tv-------------------------\n");
#endif

  return(GRP_E_OK);

exception:
  XPN_print(excpn);
#ifdef __GRP_DEBUG
  fprintf(stdout, "\tGRP_join (stub): end   %p \n", self());
  fprintf(stdout, "\tv-------------------------\n");
#endif
  return(excpn);
}


     /*¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\
    |   GRP_start                                                        |
    |                                                                    |
    |    Start a group. The method is to                                 |
    |    build and send the GRP_START request to all the machines        |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o gix      (Input)                                              |
    |        Identificator of the group                                  |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int GRP_start(int gix) {
  Hdr          hdr;
  int          dstMchn,
               excpn,
               i, send,
               grpSize;
  Grp_Msg     *msg          = (Grp_Msg *)((int *)&hdr.Store);
  static char *where        = "GRP_start (clnt)";


  if(0 > (excpn = GRP_getSize(gix, &grpSize)))                                  goto exception;
  for(i = 0; i < grpSize + 1; i++) {
    if(0 > (excpn = sendToMachine(gix, i, &dstMchn, &send)))                    goto exception;
    if(send == FALSE) continue;

#ifdef __GRP_DEBUG
    fprintf(stdout, "\n\t^------------------------\n");
    fprintf(stdout, "\tGRP_start (stub): begin %p \n", self());
#endif
    hdr.Port.Port          = GRP_PORT;
    hdr.Port.Mchn          = dstMchn;
    hdr.Mode               = RPC_FULL;
    hdr.Size               = 0;
    msg->Type              = GRP_START;
    msg->Body.GrpStart.Gix = gix;

    if(0 > (excpn = RPC_trans(&hdr, (void *) NULL, 0, (void *) NULL, 0)))       goto exception;
    if(0 > (excpn = msg->Body.ReplyGrpStart.Reply))                             goto exception;
#ifdef __GRP_DEBUG
    fprintf(stdout, "\tGRP_start (stub):end %p \n", self());
    fprintf(stdout, "\tv-------------------------\n");
#endif
  }
  return(GRP_E_OK);

exception:
  XPN_print(excpn);
  return(excpn);
}


     /*¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\
    |   GRP_kill                                                         |
    |                                                                    |
    |    Kill a group. The method is to                                  |
    |    build and send the GRP_KILL request to all the machines         |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o gix      (Input)                                              |
    |        Identificator of the group                                  |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int GRP_kill(int gix) {

  Hdr          hdr;
  int          dstMchn,
               excpn,
               i, send,
               grpSize;
  Grp_Msg     *msg          = (Grp_Msg *)((int *)&hdr.Store);
  static char *where        = "GRP_kill (clnt)";

  if(0 > (excpn = GRP_getSize(gix, &grpSize)))                                  goto exception;
  for(i = 0; i < grpSize + 1; i++) {
    if(0 > (excpn = sendToMachine(gix, i, &dstMchn, &send)))                    goto exception;
    if(send == FALSE) continue;

#ifdef __GRP_DEBUG
    fprintf(stdout, "\n\t-----------------------\n");
    fprintf(stdout, "\tGRP_kill (stub): begin %p \n", self());
#endif
    hdr.Port.Port          = GRP_PORT;
    hdr.Port.Mchn          = dstMchn;
    hdr.Mode               = RPC_FULL;
    hdr.Size               = 0;
    msg->Type              = GRP_KILL;
    msg->Body.GrpKill.Gix  = gix;

    if(0 > (excpn = RPC_trans(&hdr, (void *) NULL, 0,
                                    (void *) NULL, 0)))                         goto exception;
    if(0 > (excpn = msg->Body.ReplyGrpKill.Reply))                              goto exception;
#ifdef __GRP_DEBUG
    fprintf(stdout, "\tGRP_kill (stub):end %p \n", self());
    fprintf(stdout, "\t-----------------------\n");
#endif
  }

  return(GRP_E_OK);

exception:
  XPN_print(excpn);
  return(excpn);
}


     /*¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\
    |   GRP_wait                                                         |
    |                                                                    |
    |    Wait for a group to finish. The method is to                    |
    |    build and send the GRP_WAIT request to all the machines         |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o gix      (Input)                                              |
    |        Identificator of the group                                  |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int GRP_wait(int gix, int *status) {

  Hdr          hdr;
  int          dstMchn,
               excpn,
               send,
               grpSize;
  int          i;
  Grp_Msg     *msg          = (Grp_Msg *)((int *)&hdr.Store);
  static char *where        = "GRP_wait (clnt)";

  if(0 > (excpn = GRP_getSize(gix, &grpSize)))                                  goto exception;
#ifdef __GRP_DEBUG
  fprintf(stdout, "\n\t^---------------------- %d --\n", getCpuId());
  fprintf(stdout, "\tGRP_wait(%p) (stub): Begin  \n", self());
#endif

  hdr.Port.Port             = GRP_PORT;
  hdr.Port.Mchn             = getCpuId();
  hdr.Mode                  = RPC_FULL;
  hdr.Size                  = grpSize * sizeof(int);
  msg->Type                 = GRP_WAIT;
  msg->Body.GrpWait.Gix     = gix;
  msg->Body.GrpWait.GrpSize = grpSize;

  if(0 > (excpn = RPC_trans(&hdr, (void *) status, grpSize * sizeof(int),
                                  (void *) status, grpSize * sizeof(int))))     goto exception;
  if(0 > (excpn = msg->Body.ReplyGrpWait.Reply))                                goto exception;

#ifdef __GRP_DEBUG
    fprintf(stdout, "\tGRP_wait(%p) (stub): End  \n", self());
    fprintf(stdout, "\tv------------------------ %d --\n", getCpuId());
#endif

  /* Send destroy to all threads. This service mean join with zombies threads and destroy group */
  for(i = 0; i < grpSize + 1; i++) {
    if(0 > (excpn = sendToMachine(gix, i, &dstMchn, &send)))                    goto exception;
    if(send == FALSE) continue;

#ifdef __GRP_DEBUG
    fprintf(stdout, "\n\t^---------------------- %d --\n", i);
    fprintf(stdout, "\tGRP_destroy(%p) (stub) (to machine %d): Begin \n", self(), dstMchn);
#endif

    hdr.Port.Port              = GRP_PORT;
    hdr.Port.Mchn              = dstMchn;
    hdr.Mode                   = RPC_HALF;
    hdr.Size                   = 0;
    msg->Type                  = GRP_DESTROY;
    msg->Body.GrpDestroy.Gix   = gix;
    if(0 > (excpn = RPC_trans(&hdr, (void *) NULL, 0, (void *) NULL, 0)))       goto exception;

#ifdef __GRP_DEBUG
    fprintf(stdout, "\tGRP_destroy(%p) (stub): End  \n", self());
    fprintf(stdout, "\tv------------------------ %d --\n", i);
#endif
  }

  return(GRP_E_OK);

exception:
  XPN_print(excpn);
  return(excpn);
}


     /*¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\
    |   GRP_shutdown                                                     |
    |                                                                    |
    |    Shutdown the GRP server                                         |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o none                                                          |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int GRP_shutdown(void) {

  Hdr          hdr;
  int          node       = getCpuId();
  int          i;
  int          excpn;
  Grp_Msg     *msg        = (Grp_Msg *)((int *)&hdr.Store);
  static char *where      = "GRP_shutdown (clnt)";


  for (i = 0; i < INET_getNodes(); i++) {

    node = (node + 1) % INET_getNodes();

#ifdef __GRP_DEBUG
    fprintf(stdout, "\n\t^---------------------- %d --\n", node);
    fprintf(stdout, "\tGRP_shutdown(%p) (stub): Begin  \n", self());
#endif

    hdr.Port.Port                 = GRP_PORT;
    hdr.Port.Mchn                 = node;
    hdr.Mode                      = RPC_HALF;
    hdr.Size                      = 0;
    msg->Type                     = GRP_SHUTDOWN;
    msg->Body.GrpShutdown.Empty   = 0;

    if(0 > (excpn = RPC_trans(&hdr, (void *) NULL, 0,
                                    (void *) NULL, 0)))                        goto exception;

#ifdef __GRP_DEBUG
    fprintf(stdout, "\tGRP_shutdown(%p) (stub): End  \n", self());
    fprintf(stdout, "\tv------------------------ %d --\n", node);
#endif

  }

  return(GRP_E_OK);

exception:
  XPN_print(excpn);
  return(excpn);
}
