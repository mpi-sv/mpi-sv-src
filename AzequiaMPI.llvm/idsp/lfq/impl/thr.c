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
#include <config.h>
#include <pmi_interface.h>

#include <thr.h>
#include <rpc.h>
#include <assert.h>//added by Herman
/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#if defined (__OSI)
  #include <osi.h>
#else
  #include <pthread.h>
  #include <limits.h>
  #include <errno.h>
  #include <string.h>
  #include <stdio.h>
  #include <stdlib.h>
#endif


#include <azq_types.h>
#include <util.h>
#include <thr_dptr.h>
#include <xpn.h>
#include <addr.h>
#include <mbx.h>
#include <com.h>


/*----------------------------------------------------------------*
 *   Definition of private constants                              *
 *----------------------------------------------------------------*/
#define THR_LOCK(thr)   RPC_LOCK(thr)
#define THR_UNLOCK(thr) RPC_UNLOCK(thr)
#define THR_SIGNAL(thr) RPC_SIGNAL(thr)

/*----------------------------------------------------------------*
 *   Declaration of private data types (again)                    *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
       pthread_key_t    key;
//extern int              AZQ_cpuId;
static void           (*leaveUpcall)(int code);
static int              initialised = FALSE;

static const char      *e_names[8] = {
                             /*  0 */ "THR_E_OK",
                             /*  1 */ "THR_E_EXHAUST",
                             /*  2 */ "THR_E_INTEGRITY",
                             /*  3 */ "THR_E_TIMEOUT",         // This order has to be consistent
                             /*  4 */ "THR_E_INTERFACE",       // with thr.h
                             /*  5 */ "THR_E_SYSTEM",
                             /*  6 */ "THR_E_SIGNALED",
                             /*  7 */ "THR_E_DEADPART"
                           };

extern int rpq_match (void *elem, Addr_t addr, Tag_t tag, int type, int mode);

/*----------------------------------------------------------------*
 *   Declaration of private functions                             *
 *----------------------------------------------------------------*/
void   panic     (char *where);

#define self()       ((Thr_t)pthread_getspecific(key))

/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/

      /*________________________________________________________________
     /                                                                  \
    |    wrapper                                                         |
    |                                                                    |
    |    Universal body function for operators                           |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
static void *wrapper(void *thr) {

  int       i;
  int       offset;
  int       ret;

#ifdef __DEBUG
  fprintf(stdout, "wrapper:\n"); fflush(stdout);
#endif
  pthread_setspecific(key, (Thr_t)thr);

  /* Bind the thread to the logical processor corresponding to its rank.
   Not complaint if call failed, run without binding */
  PMII_bindSelf(((Thr_t)thr)->Placement);
  
  /* Invoke main() of an operator */
  ret = ((Thr_t)thr)->Body_Fxn(((Thr_t)thr)->Argc, ((Thr_t)thr)->Argv);

  THR_exit(ret);

  return(NULL);
}


/*----------------------------------------------------------------*
 *   Implementation of THR interface                              *
 *----------------------------------------------------------------*/

      /*________________________________________________________________
     /                                                                  \
    |    THR_init                                                        |
    |                                                                    |
    |    Initialise the THR module                                       |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int THR_init (void) {

  int      excpn      = THR_E_SYSTEM;
  char    *where      = "THR_init";


  if(initialised)
    return(THR_E_OK);

  if (pthread_key_create(&key, NULL))                                          goto exception;

#ifdef __DEBUG
  fprintf(stdout, "THR initialised\n");
#endif

  initialised = TRUE;
  return(THR_E_OK);

exception:
  XPN_print(excpn);
  return(excpn);
}


      /*________________________________________________________________
     /                                                                  \
    |    THR_finalize                                                    |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
void THR_finalize (void) {

  pthread_key_delete(key);
}


      /*________________________________________________________________
     /                                                                  \
    |    THR_create                                                      |
    |                                                                    |
    |    Creates a thread object in this machine                         |
    |                                                                    |
    |    Parameters:                                                     |
    |    o thr    (Output)                                               |
    |        Operator handle. Used to operate the new operator, such     |
    |        as start it, destroy it, etc.                               |
    |    o addr  (Input)                                                 |
    |        Address of the new operator                                 |
    |    o bodyFxn   (Input)                                             |
    |        body function of the operator                               |
    |    o stackSize (Input)                                             |
    |        Stack size of the operator                                  |
    |    o param     (Input)                                             |
    |        parameter of the operator                                   |
    |    o paramSize (Input)                                             |
    |        Size of parameter area                                      |
    |                                                                    |
    |    Return value:                                                   |
    |       0 on success                                                 |
    |      -1 on error                                                   |
    |                                                                    |
     \____________/  _/_________________________________________________/
                 / _/
                /_/
               */
int THR_create(Thr_t *thr, Addr_t addr,  int (*bodyFxn)(int, char **), int stackSize,
                                         char **argv, int argc, CommAttr_t comAttr) {

  Thr_t     newthr   = *thr;
  int       excpn    = THR_E_SYSTEM,
            j;
  char     *where    = "THR_create";

#ifdef __DEBUG
  fprintf(stdout, "THR_create:\n"); 
#endif

  /* [1] Create the thread object */
  newthr->State           = ALLOCATED;
  newthr->Address         = *addr;
  newthr->Body_Fxn        = bodyFxn;
  newthr->Stack_Size      = stackSize;

  DLQ_create(&newthr->RecvPendReg,  NULL);
  DLQ_create(&newthr->AsendPendReg, NULL);
  MBX_create(&newthr->MailBox,      NULL);

  /* [2] Parameters argc/argv come packed in a buffer of type MainArgs */
  newthr->Argc = argc;
  newthr->Argv = argv;
  
  if(comAttr) {
    ;
  }
  memset(&newthr->SyncRqst, 0, sizeof(Rqst));
  newthr->SyncRqst.Hdr.Src      =  *(addr);
  newthr->SyncRqst.Hdr.SrcMchn  =  AZQ_cpuId;
  newthr->SyncRqst.Hdr.Mode     =  0;
  newthr->SyncRqst.Owner        = (void *)(newthr);
  newthr->SyncRqst.Type        |= RQST_SYNC;
  
//newthr->SpinCounter           = 0;

  /* [3] Initialize the object */
  if (pthread_mutex_init(&newthr->Lock, NULL))                                 goto exception;
  if (pthread_cond_init (&newthr->Ready, NULL))                                goto exception;
  if (pthread_attr_init (&newthr->Attr))                                       goto exception;
#if defined (__OSI)
  if (pthread_attr_setschedpolicy(&newthr->Attr, SCHED_FIFO))                  goto exception;
#endif

#ifdef __DEBUG
  fprintf(stdout, "THR_create: End.\n");
#endif
  return(THR_E_OK);

exception:
  XPN_print(excpn);
  return(excpn);
}


      /*________________________________________________________________
     /                                                                  \
    |    THR_self                                                        |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
Thr_t THR_self(void) {
  return(self());
}


      /*________________________________________________________________
     /                                                                  \
    |    THR_exit                                                        |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
void THR_exit(int code) {

  Thr_t   me       = self();

#ifdef __DEBUG
  fprintf(stdout, "THR_exit(%p):\n", me);
#endif

  THR_LOCK(me);
  me->State    |= ZOMBIE;
  me->ExitCode  = code;
  THR_UNLOCK(me);

  leaveUpcall(code); /* currently GRP_abandone */

#ifdef __DEBUG
  fprintf(stdout, "THR_exit(%p): End\n", me);
#endif
  pthread_exit(NULL);
}

      /*________________________________________________________________
     /                                                                  \
    |    THR_wait                                                        |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
void THR_wait (Thr_t thr, int *status) {

#ifdef __DEBUG
  fprintf(stdout, "THR_wait: for %p\n", thr);
#endif

  pthread_join(thr->Self, NULL);
  if(status)
    *status = thr->ExitCode;

  if (pthread_attr_destroy(&thr->Attr))                                         panic("THR_wait(1)");
  if (pthread_cond_destroy(&thr->Ready))                                        panic("THR_wait(2)");
  if (pthread_mutex_destroy(&thr->Lock))                                        panic("THR_wait(3)");


#ifdef __DEBUG
  fprintf(stdout, "THR_wait: for %p. End\n", thr);
#endif
  return;
}

      /*_________________________________________________________________
     /                                                                   \
    |    THR_start                                                        |
    |                                                                     |
    |    start starts the execution of a thread                           |
    |                                                                     |
     \____________/  ____________________________________________________/
                 / _/
                /_/
               */
int THR_start(Thr_t thr) {

  int    excpn   = THR_E_SYSTEM;
  char  *where   = "THR_start";
  //assert(thr!=NULL);
  //printf("THR_start:%x\n",&thr->Self);
  THR_LOCK(thr);

  thr->State |= STARTED;

  //if(pthread_attr_setdetachstate(&thr->Attr, PTHREAD_CREATE_JOINABLE))         goto exception;
  //if(pthread_attr_setstacksize(&thr->Attr, thr->Stack_Size))                   goto exception;
  if(pthread_create(&thr->Self, &thr->Attr, wrapper, (void *)thr))             goto exception;
  THR_UNLOCK(thr);
  return(THR_E_OK);

exception:
  thr->State &= ~STARTED;
  THR_UNLOCK(thr);
  XPN_print(excpn);
  return(excpn);
}


      /*________________________________________________________________
     /                                                                  \
    |    THR_kill                                                        |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
void THR_kill (Thr_t thr, int sig) {

#ifdef __DEBUG
  fprintf(stdout, "THR_kill(%p):\n", self());
#endif
  if(!(thr->State & ALLOCATED))                                                 panic("THR_kill: Not allocated!");
  if(thr->State & STARTED) {
    THR_LOCK(thr);
    if(thr->State & ZOMBIE) {
      THR_UNLOCK(thr);
#ifdef __DEBUG
      fprintf(stdout, "THR_kill(%p): %p is ZOMBI. End\n", self(), thr);
#endif
      return;
    }
    if(thr->State & SIGMASK) {
#ifdef __DEBUG
      fprintf(stdout, "THR_kill(%p): %p masked->NOT signaled \n", self(), thr);
#endif
      thr->State |= SIGPEND;
    }
    else {
      thr->State |= SIGNALED;
      THR_SIGNAL(thr);

#ifdef __DEBUG
      fprintf(stdout, "THR_kill(%p): %p signaled\n", self(), thr);
#endif
    }
    THR_UNLOCK(thr);
  }
  else {
    if (pthread_attr_destroy(&thr->Attr))                                       panic("THR_kill(1)");
    if (pthread_cond_destroy(&thr->Ready))                                      panic("THR_kill(2)");
    if (pthread_mutex_destroy(&thr->Lock))                                      panic("THR_kill(3)");
  }

#ifdef __DEBUG
   fprintf(stdout, "THR_kill(%p): End\n", self());
#endif
  return;
}


      /*________________________________________________________________
     /                                                                  \
    |    THR_destroy                                                     |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
void THR_destroy (Thr_t thr) {

  if (pthread_attr_destroy(&thr->Attr))                                         panic("THR_destroy(1)");
  if (pthread_cond_destroy(&thr->Ready))                                        panic("THR_destroy(2)");
  if (pthread_mutex_destroy(&thr->Lock))                                        panic("THR_destroy(3)");
  return;
}


      /*________________________________________________________________
     /                                                                  \
    |    THR_setmask                                                     |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
void THR_setmask(Thr_t thr, int mode) {

#ifdef __DEBUG
  if(mode == MASK_ON)
    fprintf(stdout, "\tTHR_setmask(%p): Masking %p\n", self(), thr);
  else
    fprintf(stdout, "\tTHR_setmask(%p): Unmasking %p\n", self(), thr);
#endif
  if(thr) {
    if(mode == MASK_ON) {
      thr->State |= SIGMASK;
    }
    else {
      thr->State &= ~SIGMASK;
      if(thr->State & SIGPEND) {
        thr->State &= ~SIGPEND;
        thr->State |= SIGNALED;
        THR_SIGNAL(thr);
      }
      else ;
    }
  }

  return;
}


      /*________________________________________________________________
     /                                                                  \
    |    THR_getClient                                                   |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
void THR_getClient(Addr *addr, int *mchn) {

  Thr_t thr = self();

  if(thr) {
    *addr = thr->ClientAddr;
    *mchn = thr->ClientMchn;
  }
  return;
}


      /*________________________________________________________________
     /                                                                  \
    |    THR_getSelf                                                     |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
pthread_t THR_getSelf(Thr_t thr) {
  return thr->Self;
}


      /*________________________________________________________________
     /                                                                  \
    |    THR_setClient                                                   |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
void THR_setClient(Addr *addr, int mchn) {

  Thr_t thr = self();

  if(thr) {
    thr->ClientAddr = *addr;
    thr->ClientMchn = mchn;
  }
  return;
}


      /*________________________________________________________________
     /                                                                  \
    |    THR_installLeave                                                |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
void THR_installLeave(void (*upcall)(int code)) {

  leaveUpcall = upcall;
  return;
}


      /*________________________________________________________________
     /                                                                  \
    |    THR_setGrpInfo                                                  |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
void THR_setGrpInfo(Thr_t thr, void *grpInfo) {
  thr->GrpInfo = grpInfo;
}


      /*________________________________________________________________
     /                                                                  \
    |    THR_setPlacementInfo                                            |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
void THR_setPlacementInfo(Thr_t thr, Placement_t place) {
  thr->Placement = place;
}
