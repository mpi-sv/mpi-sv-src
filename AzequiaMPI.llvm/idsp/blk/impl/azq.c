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
#include <azq.h>

/*----------------------------------------------------------------*
 *   Declaration of types and functions used by this module       *
 *----------------------------------------------------------------*/
#include <grp.h>

#if defined(__OSI)
  #include <osi.h>
#else
  #include <limits.h>
  #include <sched.h>
  #include <pthread.h>
  #include <stdio.h>
  #include <errno.h>
  #include <string.h>
/* wait definition come into conflict with wait macro in stdlib.h */
  #undef wait
  #include <stdlib.h>
#endif

#include <azq_types.h>
#include <opr.h>
#include <thr.h>
#include <rpc.h>
#include <thr_dptr.h>
#include <pmp.h>
#include <rpc_hddn.h>
#include <xpn.h>


extern void   panic        (char *where);
extern int    AZQ_clnt     (int argc, char **argv);

/*----------------------------------------------------------------*
 *   Definition of private constants                              *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*
 *   Definition of private types                                  *
 *----------------------------------------------------------------*/
/* Arguments to be passed to AZQ_clnt */
struct ThrArgs {
  REG_Entry  *OprTable;
  int         OprNr;
};

/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
extern pthread_key_t     key;

static pthread_t         azqThr;
static int               azqClntExitCode = AZQ_E_SYSTEM;
static struct ThrArgs    margs;

static char             *e_names[8] = {  /* This order has to be consistent with grp.h */
                                      /*  0 */ "AZQ_SUCCESS",
                                      /*  1 */ "AZQ_E_EXHAUST",
                                      /*  2 */ "AZQ_E_INTEGRITY",
                                      /*  3 */ "AZQ_E_TIMEOUT",
                                      /*  4 */ "AZQ_E_INTERFACE",
                                      /*  5 */ "AZQ_E_SYSTEM",
                                      /*  6 */ "AZQ_E_SIGNALED",
                                      /*  7 */ "AZQ_E_DEADPART"
                                      };

/*----------------------------------------------------------------*
 *   Declaration of private functions implemented by this module  *
 *----------------------------------------------------------------*/
static void  *azqClntBody  (void *args);

/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/

      /*_________________________________________________________________
     /                                                                   \
    |    azqMainBody                                                      |
    |                                                                     |
     \____________/  ____________________________________________________/
                 / _/
                /_/
               */
static void *azqClntBody(void *args) {
	//klee_disable_sync_chk(0);
  char           *where = "azqClntBody";
  struct ThrArgs *margs = (struct ThrArgs *)args;

#ifdef __DEBUG
  fprintf(stdout, "azqClntBody: Init\n");
#endif

  /* 1. main thread becomes an Azequia thread */
  if (0 > GRP_enroll())                                                         goto exception;

  /* 2. Register user applications */
  OPR_register(margs->OprTable, margs->OprNr);
  
  if (0 > (azqClntExitCode = AZQ_clnt(margs->OprTable->Argc, 
                                      margs->OprTable->Argv)))               goto exception;

#ifdef __DEBUG
  fprintf(stdout, "azqClntBody: End\n");
#endif

  //klee_enable_sync_chk(0);
  return (&azqClntExitCode);
  
exception:
  XPN_print(azqClntExitCode);

  //klee_enable_sync_chk(0);
  return(&azqClntExitCode);
}


/*----------------------------------------------------------------*
 *   Implementation of public interface                           *
 *----------------------------------------------------------------*/

      /*_________________________________________________________________
     /                                                                   \
    |    AZQ_init                                                         |
    |                                                                     |
     \____________/  ____________________________________________________/
                 / _/
                /_/
               */
int AZQ_init(REG_Entry *oprTable, int oprNr) {
  static int          initialised  = FALSE;
  void               *clntExitCode;
  int                 excpn        = AZQ_E_SYSTEM;
  char               *where        = "AZQ_init";

  //klee_disable_sync_chk(0);
#ifdef __DEBUG
  fprintf(stdout, "AZQ_init: \n");
#endif

  if(initialised)
    return(AZQ_SUCCESS);
  initialised = TRUE;

  /* Init Process Manager Interface with Azequia */
  //klee_disable_sync_chk();
  PMII_init();
    
  /* Init AZEQUIA and start daemonds */
  if (0 > (excpn = GRP_init()))                                                goto exception;

#ifdef VERBOSE_MODE
  time_t      curtime;
  struct tm  *loctime;
  int         i;

  if (getCpuId() == 0) {
    fprintf(stdout, "-------------------------------------------------------\n");
    fprintf(stdout, "\tWellcome to AZEQUIA %s  rev. %s\n", AZQ_VERSION, AZQ_REVISION);
  
    curtime = time (NULL);
    loctime = localtime (&curtime);
    fprintf(stdout, "\tDate: %s", asctime (loctime));
    fprintf(stdout, "\tProgram: %s\n", argv[0]);
    fprintf(stdout, "\tParameters: %d\n\t", argc - 1);
    for (i = 1; i < argc; i++) 
	  fprintf(stdout, "\t%s", argv[i]);
    fprintf(stdout, "\n");
  
    fprintf(stdout, "-------------------------------------------------------\n");
  }
#endif
  
  /* Start AZQ_main thread */
  /* Set up arguments for client thread */
  margs.OprTable = oprTable;
  margs.OprNr    = oprNr;
  if(pthread_create (&azqThr, NULL, azqClntBody, (void *)&margs))              goto exception;
  if(pthread_join   ( azqThr, (void **)&clntExitCode))                         goto exception;

  /* Wait for GRP server and free data */
  GRP_finalize();

  /* Interface to PMI */
  PMII_finalize();
  
#ifdef __DEBUG
  fprintf(stdout, "AZQ_init: End\n\n");
#endif
  //klee_enable_sync_chk(0);
  return(*((int *)clntExitCode));

exception:
  XPN_print(excpn);
  //klee_enable_sync_chk(0);
  return(excpn);
}
