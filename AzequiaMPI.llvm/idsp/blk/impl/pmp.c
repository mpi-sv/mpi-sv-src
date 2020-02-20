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
#include <pmp_sk.h>
#include <pmp.h>

/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/

#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdlib.h>
  #include <limits.h>
  #include <sched.h>
  #include <pthread.h>
  #include <string.h>
  #include <semaphore.h>
  #include <errno.h>
#endif

#include <azq_types.h>
#include <util.h>
#include <xpn.h>
#include <grp_hddn.h>
#include <rpc.h>
#include <com.h>
#include <pmp_cfg.h>

extern int PMP_service_loop();

/*----------------------------------------------------------------*
 *   Definition of private constants                              *
 *----------------------------------------------------------------*/
#define WINDOW_MAX 4

#ifdef __PMP_DEBUG
#define DBG_PRNT(pmsg)  fprintf pmsg
#else
#define DBG_PRNT(pmsg)
#endif

/*----------------------------------------------------------------*
 *   Definition of private types                                  *
 *----------------------------------------------------------------*/
/* Service Table: Keeps the services this machine is PAYING */
struct Cpl {
  int    Allocated;
  int    Svc;
  int    Gix;
};
typedef struct Cpl Cpl, *Cpl_t;

struct CplTable {
  pthread_mutex_t  Lock;
  int              AllocCnt;
  Cpl_t            Cpl;
};
typedef struct CplTable CplTable;

/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
static int        initialised = FALSE;
static CplTable   table;

#if defined (HAVE_SEM_OPEN)
extern sem_t      *pmp_sync;
#else
extern sem_t       pmp_sync;
#endif

static char *e_names[8] = {  /*  0 */ "PMP_E_OK",
                             /*  1 */ "PMP_E_EXHAUST",
                             /*  2 */ "PMP_E_INTEGRITY",
                             /*  3 */ "PMP_E_TIMEOUT",         /* This order has to be consistent */
                             /*  4 */ "PMP_E_INTERFACE",       /* with pmp.h                      */
                             /*  5 */ "PMP_E_SYSTEM",
                             /*  6 */ "PMP_E_SIGNALED",
                             /*  7 */ "PMP_E_DEADPART"
                           };


static int cpu_load_counter = 0,
           max_cpu_load_counter = 0,
           cpu_load = 0,
           v[WINDOW_MAX];


extern Thr_t pmp_thr;

/*----------------------------------------------------------------*
 *   Declaration of private functions implemented by this module  *
 *----------------------------------------------------------------*/
static inline int  cplAlloc (int svc, int  gix);
static inline void cplFree  (int svc);
static        int  cplGetGix(int svc, int *gix);


/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/

     /*\
    |    cplGetGix2                                                      |
    |                                                                    |
    |                                                                    |
     \*----------------------------------------------------------------*/
static int cplGetGix2(int svc, int *gix) {

  int i;

  for(i = 0; i < CPL_MAX; i++) {
    if(table.Cpl[i].Allocated) {  /* Try just allocated entries */
      if(table.Cpl[i].Svc == svc) {
        *gix = table.Cpl[i].Gix;
        return (PMP_E_OK);
      }
    }
  }
  *gix = 0; /* NONE */
  return(-1);
}

     /*\
    |    cplAlloc                                                        |
    |                                                                    |
    |                                                                    |
     \*----------------------------------------------------------------*/
static inline int  cplAlloc(int svc, int gix) {

  int  i, gix2;

  pthread_mutex_lock(&table.Lock);

  /* [1] Test if the couple already exists */
  if(PMP_E_OK == cplGetGix2(svc, &gix2)) {
    pthread_mutex_unlock(&table.Lock);
    return(PMP_E_OK);
  }

  /* [2] Create the couple */
  for(i = 0; i < CPL_MAX; i++) {
    if(!table.Cpl[i].Allocated) {  // Free entry found !!
	    table.Cpl[i].Svc = svc;
	    table.Cpl[i].Gix = gix;
      table.Cpl[i].Allocated = TRUE;
      table.AllocCnt++;
      pthread_mutex_unlock(&table.Lock);
      return(PMP_E_OK);
    }
  }
  pthread_mutex_unlock(&table.Lock);
  return(-1);
}


     /*\
    |    cplFree                                                         |
    |                                                                    |
    |                                                                    |
     \*----------------------------------------------------------------*/
static void cplFree(int svc) {

  int i;

  pthread_mutex_lock(&table.Lock);
  for(i = 0; i < CPL_MAX; i++) {
    if(table.Cpl[i].Allocated) {  /* Try just allocated entries */
      if(table.Cpl[i].Svc == svc) {
        table.Cpl[i].Allocated = FALSE;
        table.AllocCnt--;
        pthread_mutex_unlock(&table.Lock);
        return;
      }
    }
  }
  pthread_mutex_unlock(&table.Lock);
}


static int cplGetGix(int svc, int *gix) {

  int ret;

  pthread_mutex_lock(&table.Lock);
  ret = cplGetGix2(svc, gix);
  pthread_mutex_unlock(&table.Lock);
  return(ret);
}

     /*-----------------------------------------------------------------\
    |    load_init                                                       |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
static inline void load_init(void) {

  int j;

  for (j = 0; j < WINDOW_MAX; j++) {
    v[j] = 0;
  }

}


     /*\
    |    CPULOAD_idl                                                     |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
void CPULOAD_idl (void) {
  cpu_load_counter++;
}


     /*\
    |    CPULOAD_prd                                                     |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
void CPULOAD_prd (void) {

  static int index = 0;
  int j, average_counter = 0;

  /* CODE to determine the SUITABLE_NUMBER (Do not remove!!!)
    (the maximum value of max_cpu_load_counter in a test period) */
  if(cpu_load_counter > max_cpu_load_counter)
    max_cpu_load_counter = cpu_load_counter;
  else max_cpu_load_counter = 95 * max_cpu_load_counter / 100;

  if(index >= WINDOW_MAX)
    index = 0;
  v[index++] = cpu_load_counter;
  for (j = 0; j < WINDOW_MAX; j++) {
    average_counter += v[j];
  }
  average_counter = average_counter / WINDOW_MAX;
  cpu_load = 100 - ((100 * average_counter) / max_cpu_load_counter);
  cpu_load_counter = 0;
}


/*----------------------------------------------------------------*
 *   Implementation of public interface                           *
 *----------------------------------------------------------------*/

      /*________________________________________________________________
     /                                                                  \
    |    PMP_init                                                        |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int PMP_init() {

  int      excpn     = PMP_E_SYSTEM,
           gix       = PMP_GIX;
  char    *where     = "PMP_init";

  if(initialised)
    return(PMP_E_OK);

  DBG_PRNT((stdout, "PMP_init:\n"));

  /* Make room for the couple table */
  table.AllocCnt = 0;
  if(NULL == (table.Cpl = (Cpl_t) AZQ_MALLOC(CPL_MAX * sizeof(Cpl))))              {excpn = PMP_E_EXHAUST;
                                                                                goto exception_0;}
  memset(table.Cpl, 0, (CPL_MAX) * sizeof(Cpl));

  DBG_PRNT((stdout, "CPL_MAX: (%d)  %ld\n", CPL_MAX, CPL_MAX * sizeof(Cpl)));

  /* Global mutex of the packet */
  if(pthread_mutex_init(&table.Lock, NULL))                                    {excpn = PMP_E_SYSTEM;
                                                                                goto exception_1;}
  /* Synchronize with start of GRP daemon */
#if defined (HAVE_SEM_OPEN)
  sem_unlink("/azqsempmp");
  pmp_sync = sem_open("/azqsempmp", O_CREAT | O_EXCL, 0, 0);
  if (pmp_sync == (sem_t *)SEM_FAILED)                                         goto exception_2;  
#else 
  if (0 > sem_init(&pmp_sync, 0, 0))                                           goto exception_2;
#endif

  load_init();

  /* Create the PMP daemon */
  if(0 > (excpn = GRP_server(&gix, PMP_service_loop,
                                   NO_PORT,
                                   PTHREAD_STACK_MIN)))                        goto exception_3;

  /* forces pmp server to execute */
#if defined (HAVE_SEM_OPEN)
  do {
    if (0 > sem_wait((sem_t *)pmp_sync)) {
      if(errno == EINTR)
        continue;
      else
        goto exception_2;
    }
    break;
  } while(1);  
  if (0 > sem_close((sem_t *)pmp_sync))                                        goto exception_2;
  if (0 > sem_unlink("/azqsempmp"))                                            goto exception_2;  
 
#else  
  do {
    if (0 > sem_wait(&pmp_sync)) {
      if(errno == EINTR)
        continue;
      else
        goto exception_2;
    }
    break;
  } while(1);
  if (0 > sem_destroy(&pmp_sync))                                              goto exception_2;
#endif
  
  initialised = TRUE;

  DBG_PRNT((stdout, "PMP_init: PMP daemon launched. \n"));

  return(PMP_E_OK);

exception_3:
#if defined (HAVE_SEM_OPEN)
  sem_close((sem_t *)pmp_sync);
  sem_unlink("/azqsempmp");
#else   
  sem_destroy(&pmp_sync);
#endif  
exception_2:
  pthread_mutex_destroy(&table.Lock);
exception_1:
  AZQ_FREE(table.Cpl);
exception_0:
  XPN_print(excpn);
  return(excpn);
}


      /*________________________________________________________________
     /                                                                  \
    |    PMP_finalize                                                    |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
void PMP_finalize(void) {

  THR_wait(pmp_thr, NULL);

  if (table.Cpl)
    AZQ_FREE(table.Cpl);

  pthread_mutex_destroy(&table.Lock);

  return;
}

      /*________________________________________________________________
     /                                                                  \
    |    PMP_get                                                         |
    |                                                                    |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int PMP_get (int svc, int *gix, int *mchn) {

  char     *where   = "PMP_get";
  int       excpn;

  if(0 > cplGetGix(svc, gix))                                                  {excpn = PMP_E_INTERFACE;
                                                                                goto exception;}
  return(PMP_E_OK);

exception:
  DBG_PRNT((stdout, "PMP_get: Service %d not registered\n", svc));
  XPN_print(excpn);
  return(excpn);
}


      /*________________________________________________________________
     /                                                                  \
    |    PMP_register                                                    |
    |                                                                    |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int PMP_register (int svc, int gix) {

  char     *where   = "PMP_register";
  int       excpn;

  DBG_PRNT((stdout, "PMP_register: Registering SERVICE %d as GROUP %x\n", svc, gix));

  if(0 > cplAlloc(svc, gix))                                                    {excpn = PMP_E_EXHAUST;
                                                                                goto exception;}
  DBG_PRNT((stdout, "PMP_register: End\n"));

  return(PMP_E_OK);

exception:
  XPN_print(excpn);
  return(excpn);
}


      /*________________________________________________________________
     /                                                                  \
    |    PMP_unregister                                                  |
    |                                                                    |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
void PMP_unregister (int svc) {

  cplFree(svc);
  return;
}


      /*________________________________________________________________
     /                                                                  \
    |    PMP_getLoad                                                     |
    |                                                                    |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int PMP_getLoad (int *load) {

  *load = cpu_load;

  return(PMP_E_OK);
}
