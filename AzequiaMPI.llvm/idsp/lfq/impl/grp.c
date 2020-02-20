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
#include <config.h>
#include <pmi_interface.h>

#include <grp_sk.h>
#include <grp.h>


/*----------------------------------------------------------------*
 *   Declaration of types and functions used by this module       *
 *----------------------------------------------------------------*/

#if defined(__OSI)
  #include <osi.h>
#else
  #include <limits.h>
  #include <sched.h>
  #include <pthread.h>
  #include <sys/mman.h>

  #include <errno.h>
  #include <string.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <semaphore.h>
#endif

#include <azq.h>
#include <azq_types.h>
#include <util.h>
#include <opr.h>
#include <thr.h>
#include <rpc.h>
#include <thr_dptr.h>
#include <pmp.h>
#include <rpc_hddn.h>
#include <xpn.h>

extern void   panic           (char *where);
extern int    AZQ_main        (void);
extern void   GRP_service_loop(void *);
extern void   GRP_abandone    (int code);
extern int    GRP_done;
extern int    GRP_doneGix;
extern int    GRP_doneSize;
extern Addr   GRP_doneFatherAddr;
extern int    GRP_doneFatherMchn;
extern int    GRP_dont_reply;

extern void   COM_start(void);

/*----------------------------------------------------------------*
 *   Definition of private constants                              *
 *----------------------------------------------------------------*/
/* GRP Server and PMP server are system group always created, plus the
   AZQ initialization thread that becomes an Azequia group for invoking
   the interface functions needed
   Also, there are a system thread for all this servers. Make room in
   MchnThr structure for them (including the Creator thread)
 */
#define SYSTEM_GROUPS      3
#define SYSTEM_THREADS     SYSTEM_GROUPS
#define SYSTEM_MCHN_THR    (SYSTEM_THREADS * 2)

/*----------------------------------------------------------------*
 *   Definition of private types                                  *
 *----------------------------------------------------------------*/
/* This should be delared here and not in com.h 
   Performance made us to commit this crime against beauty and power of software architecture
   Stuttgart. July 20, 2010  

#define MchnThrSize ( \
            3 * sizeof(int) + \
                sizeof(Thr_t) \
               )
struct MchnThr {
  int    Mchn;
  int    Sigmask;
  int    ExitCode;
  Thr_t  Thr;
  char   Pad[((MchnThrSize) % CACHE_LINE_SIZE ? 64 - ((MchnThrSize) % CACHE_LINE_SIZE) : 0)];
};
typedef struct MchnThr MchnThr, *MchnThr_t;
 *
 */


/*----------------------------------------------------------------*
 *   Definition of private types                                  *
 *----------------------------------------------------------------*/
struct Group {
  int        Allocated;
  int        Gix;
  int        Size;
  int        LocalSize;
  MchnThr_t  MchnThr;
  int        LeaveCnt;
  int        Started;
  Addr       FatherAddr;
  int        FatherMchn;
  int        FatherWaiting;
};
typedef struct Group Group, *Group_t;


struct GRP_Table {
  int      AllocCnt;
  int      AllocSize;
  Group_t  Grp;
};
typedef struct GRP_Table GRP_Table;


/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
extern pthread_key_t  key;
extern Thr_t          grp_thr;

#if defined (HAVE_SEM_OPEN)
extern sem_t         *grp_sync;
#else
extern sem_t          grp_sync;
#endif

static GRP_Table      grpTable;

static char          *e_names[8] = {  // This order has to be consistent with grp.h
                                      /*  0 */ "GRP_E_OK",
                                      /*  1 */ "GRP_E_EXHAUST",
                                      /*  2 */ "GRP_E_INTEGRITY",
                                      /*  3 */ "GRP_E_TIMEOUT",
                                      /*  4 */ "GRP_E_INTERFACE",
                                      /*  5 */ "GRP_E_SYSTEM",
                                      /*  6 */ "GRP_E_SIGNALED",
                                      /*  7 */ "GRP_E_DEADPART"
                                   };

/*----------------------------------------------------------------*
 *   Declaration of private functions implemented by this module  *
 *----------------------------------------------------------------*/
static inline int  mchnThr_Alloc (MchnThr_t *block, int *mchn, int size, int creator);
static inline void mchnThr_Free  (MchnThr_t  block, int size);

static inline int  grpFind       (Group_t   *grp, int gix);
static inline int  grpAlloc      (Group_t   *grp, int gix, int *mchn, int size, int creator);
static inline void grpFree       (Group_t    grp);

              void newGix        (int       *gix);
static        int  setThr        (int        gix, int rank, Thr_t thr);

              int  getEnd        (Thr_t srcThr, Addr_t dst, int *mchn, void *thr);


/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/

      /*_________________________________________________________________
     /                                                                   \
    |    mchnThr_Alloc                                                   |
    |                                                                    |
    |    Allocate and Store the machines of a group object that is       |
    |    being created                                                   |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o thr      (Output)                                             |
    |        Block of machines allocated                                 |
    |    o machine  (Input)                                              |
    |        Vector of "size" machines                                   |
    |    o size  (Input)                                                 |
    |        The size of the group plus one                              |
    |    o creator  (Input)                                              |
    |        The creator machine                                         |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
static inline int mchnThr_Alloc (MchnThr_t *block, int *machine, int size, int creator) {

  static
  char         *where    = "mchnThr_Alloc";
  int           excpn;
  Thr_t         thr;
  MchnThr_t     mchnthr;
  Placement_t   place;
  int           i, j = 0;
  int           mchn;
  int           localsz = 0;

  /* 1. Allocate memory for MchnThr tables */
  if (posix_memalign((void *)&mchnthr, 
                             CACHE_LINE_SIZE, (size + 1) * sizeof(MchnThr)))    { excpn = GRP_E_EXHAUST;
                                                                                  goto exception_0; }
  memset(mchnthr, 0, (size + 1) * sizeof(MchnThr));

  /* 2. Allocate space for Thrs in this machine */
  mchn = getCpuId();
  for (i = 0; i < size; i++) {
	if (mchn == machine[i]) localsz++;
  }
  if (posix_memalign((void *)&thr, CACHE_LINE_SIZE, localsz * sizeof(Thr)))     { excpn = GRP_E_EXHAUST;
                                                                                  goto exception_1; }
  memset(thr, 0, localsz * sizeof(Thr));

  if (posix_memalign((void *)&place, CACHE_LINE_SIZE, localsz * sizeof(Placement)))     
                                                                                { excpn = GRP_E_EXHAUST;
	                                                                              goto exception_2; }

  memset(place, 0, localsz * sizeof(Placement));
  
  for (i = 0; i < localsz; i++)
	THR_setLocalRank(&thr[i], i);

#ifdef __DEBUG
  fprintf(stdout, "Thread entries per group: (%d).  Local threads %d  %ld\n", size, localsz, 
		                               ((size + 1) * sizeof(MchnThr)) + (localsz * sizeof(Thr)));
#endif

  /* 3. Fill MchnThr contents */
  j = 0;
  for (i = 0; i < size; i++) {
	
	/* Every thread in the group knows the machine where running others */
    mchnthr[i].Mchn  = machine[i];
	mchnthr[i].Thr   = NULL;
	
	if (mchn == machine[i]) { /* Threads running in this machine */
	  
	  mchnthr[i].Thr       = &thr[j];
	  mchnthr[i].Placement = &place[j];
	  
	  /* Calculate bind thread parameters if neccessary */
	  PMII_setBindParams(mchnthr[i].Placement, i, j);
	  
	  THR_setPlacementInfo(mchnthr[i].Thr, mchnthr[i].Placement);
	  
	  j++;
	}
	
    mchnthr[i].Sigmask   = 0;
  }
  
  /* Creator thread (size + 1) */
  mchnthr[i].Mchn     = creator;
  mchnthr[i].Thr      = NULL;
  mchnthr[i].Sigmask  = 0;
  
  
  *block = mchnthr;
  
  return GRP_E_OK;
  
exception_2:
  free(thr);
exception_1:
  free(mchnthr);
exception_0:
  XPN_print(excpn);
  return excpn;
}



     /*----------------------------------------------------------------*\
    |    mchnThr_Free                                                    |
    |                                                                    |
    |                                                                    |
     \*----------------------------------------------------------------*/
static inline void mchnThr_Free (MchnThr_t block, int size) {

  int i;
  
  /* 1. Release memory from Thr table. */
  for (i = 0; i < size; i++) {
	if (block[i].Thr != NULL) {
	  AZQ_FREE(block[i].Thr);
	  AZQ_FREE(block[i].Placement);
	  break;
	}
  }
  if (i == size)                                                                panic("mchnThr_Free(1)");

  /* 2. Release memory from MchnThr table. */
  if (NULL == block)                                                            panic("mchnThr_Free(2)");
  AZQ_FREE(block);

  return;
}

     /*----------------------------------------------------------------*\
    |    grpAlloc                                                        |
    |                                                                    |
    |                                                                    |
     \*----------------------------------------------------------------*/
static inline int grpAlloc(Group_t *grp, int gix, int *machine, int size, int creator) {

  int         i,
              excpn        = GRP_E_EXHAUST;
  MchnThr_t   block        = NULL;
  char        *where       = "grpAlloc";

  /* 1. Test if the group already exists */
  if(GRP_E_OK == grpFind(grp, gix))                                            {excpn = GRP_E_INTERFACE;
                                                                                goto exception;}
  /* 2. Create the group */
  for(i = 0; i < grpTable.AllocSize; i++) {
    if(!grpTable.Grp[i].Allocated) {  /* Free entry found !! */
      if(0 > (excpn = mchnThr_Alloc(&block, machine, size, creator)))          goto exception;
      memset(&grpTable.Grp[i], 0, sizeof(Group));
      grpTable.Grp[i].Gix       = gix;
      grpTable.Grp[i].Size      = size;
      grpTable.Grp[i].MchnThr   = block;
      grpTable.Grp[i].LocalSize = 0;	
      grpTable.Grp[i].LeaveCnt  = 0; 
      grpTable.Grp[i].Allocated = TRUE;
      grpTable.AllocCnt++;
      *grp = &grpTable.Grp[i];
	  
      return(GRP_E_OK);
    }
  }

exception:
  XPN_print(excpn);
  return(excpn);
}


     /*----------------------------------------------------------------*\
    |    grpFind                                                         |
    |                                                                    |
    |                                                                    |
     \*----------------------------------------------------------------*/
static inline int grpFind(Group_t *grp, int gix) {

  int i;

  for(i = 0; i < grpTable.AllocSize; i++) {
    if(grpTable.Grp[i].Allocated) {  /* Try just allocated entries */
      if(grpTable.Grp[i].Gix == gix) {
        *grp = (Group_t)(&grpTable.Grp[i]);
        return (GRP_E_OK);
      }
    }
  }

  return(-1);
}


     /*----------------------------------------------------------------*\
    |    grpFree                                                         |
    |                                                                    |
    |                                                                    |
     \*----------------------------------------------------------------*/
static void grpFree(Group_t grp) {

  if(!grp->Allocated)                                                           panic("grpFree");
  mchnThr_Free(grp->MchnThr, grp->Size);
  grp->Allocated = FALSE;
  if(0 > --grpTable.AllocCnt)                                                   panic("grpFree");
  return;
}


     /*----------------------------------------------------------------*\
    |    setThr                                                          |
    |                                                                    |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
static int setThr(int gix, int rank, Thr_t thr) {

  char       *where = "setThr";
  int         excpn;
  Group_t     grp;
  MchnThr_t   block;

  if(0 > grpFind(&grp, gix))                                                    {excpn = GRP_E_INTERFACE;
                                                                                goto exception;}
  block = grp->MchnThr;
  block[rank].Thr = thr;
  grp->LocalSize++;
  return(GRP_E_OK);

exception:
  XPN_print(excpn);
  return(excpn);
}


      /*________________________________________________________________
     /                                                                  \
    |    getThr                                                          |
    |                                                                    |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
static int getThr(int gix, int rank, Thr_t *thr) {

  char       *where = "getThr";
  int         excpn;
  Group_t     grp;
  MchnThr_t   block;

  if(0 > grpFind(&grp, gix))                                                    {excpn = GRP_E_INTERFACE;
                                                                                goto exception;}
  block = grp->MchnThr;
  *thr = block[rank].Thr;
  return(GRP_E_OK);

exception:
  XPN_print(excpn);
  return(excpn);
}


      /*_________________________________________________________________
     /                                                                   \
    |    GRP_localRank2globalRank                                         |
    |                                                                     |
    |                                                                     |
     \____________/  ____________________________________________________/
                 / _/
                /_/
               */
static int GRP_localRank2globalRank(int gix, int lRank)
{
  int         lRankCnt = -1;
  int         gRank;
  MchnThr_t   block;
  Group_t     grp;
  int         excpn;
  char       *where    = "GRP_localRank2globalRank";

  if(0 > grpFind(&grp, gix))                                                    {excpn = GRP_E_INTERFACE;
                                                                                goto exception;}

  block = grp->MchnThr;
  for(gRank = 0; gRank < grp->Size; gRank++) {
    if(block[gRank].Thr) {
      if(block[gRank].Thr->LocalRank == lRank) {
        return gRank;
      }
    }
  }
  return 0;

exception:
  XPN_print(excpn);
  return(excpn);
}


      /*________________________________________________________________
     /                                                                  \
    |    newGix                                                          |
    |                                                                    |
    |    Inter-package function                                          |
    |    Build a new Gix                                                 |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
void newGix(int *gix) {

  do {
    *gix = random();
  } while(*gix == PMP_GIX || *gix == GIX_NONE);

  return;
}


      /*________________________________________________________________
     /                                                                  \
    |    getEnd                                                          |
    |    Upcall function                                                 |
    |    Retrieve the machine/thread where the operator run              |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o gix         (Input)                                           |
    |        Group id                                                    |
    |    o rank        (Input)                                           |
    |        Rank                                                        |
    |    o machine     (Output)                                          |
    |        The machine where the operator runs                         |
    |    o thr         (Output)                                          |
    |        The thread where the operator runs                          |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int getEnd(Thr_t srcThr, Addr_t dst, int *mchn, void *thr) {

  int        excpn;
  Group_t    grp;
  MchnThr_t  block;
  static
  char      *where = "getEnd";

  if(0 > grpFind((Group_t *)&grp, dst->Group))                                 {excpn = GRP_E_INTERFACE;
                                                                                goto exception;}

  block = grp->MchnThr;
  *mchn            = block[dst->Rank].Mchn;
  *((Thr_t *)thr)  = block[dst->Rank].Thr;
  
  return(GRP_E_OK);

exception:
  XPN_print(excpn);
  return(excpn);
}



      /*________________________________________________________________
     /                                                                  \
    |    GRP_joinFxn                                                     |
    |    Aument the group "gix" with a new operator                      |
    |    The operator is the one defined by "name".                      |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o gix      (Input)                                              |
    |        Group id to join                                            |
    |    o name     (Input)                                              |
    |        name of operator that joins the group                       |
    |    o rank     (Input)                                              |
    |        operator rank in the group                                  |
    |    o param    (Input)                                              |
    |        Each operator is provided with a parameter structure.       |
    |        Param is the address of this structure                      |
    |    o paramsize  (Input)                                            |
    |        Size of the "param" structure                               |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
static int GRP_joinFxn (int gix, int rank, void (*bodyFxn), int stackSize) {

  int         excpn;
  Addr        addr;
  char       *where       = "GRP_joinFxn";
  Group_t     grp;
  Thr_t       thr;

#ifdef __DEBUG
  fprintf(stdout, "GRP_joinFxn\n");
#endif

  if (0 > grpFind(&grp, gix))                                                   {excpn = GRP_E_INTERFACE;
                                                                                 goto exception; }
  /* Create the thread */
  addr.Group = gix;
  addr.Rank  = rank;
  thr = grp->MchnThr[rank].Thr;
  if(0 > (excpn = THR_create(&thr,
                             &addr,
                              bodyFxn,
                              stackSize,
                              NULL,
                              0, 
							  NULL)))                                           goto exception;

  /* Register the thread in the group */
  if(0 > (excpn = setThr(gix, rank, thr)))                                      goto exception2;
  return(GRP_E_OK);

exception2:
  THR_destroy(thr);
exception:
  XPN_print(excpn);
  return(excpn);
}



      /*_________________________________________________________________
     /                                                                   \
    |    GRP_server                                                      |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int GRP_server(int *gix, void (*bodyFxn), int port, int stackSize) {

  int     excpn;
  int     mchn      = getCpuId();
  Group_t grp; 
  Thr_t   thr;
  char   *where     = "GRP_server";

#ifdef __DEBUG
  fprintf(stdout, "GRP_server:\n");
#endif
  /* Create the GRP daemon */
  if(0 > (excpn = GRP_create ( gix, &mchn, 1, mchn)))                           goto exception;
  if(0 > (excpn = GRP_joinFxn(*gix, 0, bodyFxn, stackSize)))                    goto exception;
  /* Servers are not binded to a specific processor */
  if(GRP_E_OK != grpFind(&grp, *gix))                                           {excpn = GRP_E_INTERFACE;
	                                                                             goto exception;}
  PMII_setUnBind(&(grp->MchnThr->Placement[0]));
  
  
  if(0 > (excpn = RPC_register(port, *gix)))                                    goto exception;
  if(0 > (excpn = GRP_start  (*gix)))                                           goto exception;
#ifdef __DEBUG
  fprintf(stdout, "GRP_server: End\n");
#endif
  return(GRP_E_OK);

exception:
  XPN_print(excpn);
  return(excpn);
}


/*----------------------------------------------------------------*
 *   Implementation of GRP interface                              *
 *----------------------------------------------------------------*/

      /*_________________________________________________________________
     /                                                                   \
    |    GRP_init                                                        |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int GRP_init() {

  int          excpn          = GRP_E_SYSTEM;
  int          gix            = GIX_NONE;
  static int   initialised    = FALSE;
  char        *where          = "GRP_init";

#ifdef __DEBUG
  fprintf(stdout, "  GRP_init:\n");
#endif
  if(initialised)
    return(GRP_E_OK);

  /* Add the system groups */
  grpTable.AllocCnt  = 0;
  grpTable.AllocSize = USER_GROUPS + SYSTEM_GROUPS;

  /* Make room for the group table */
  if(NULL == (grpTable.Grp =
             (Group_t) AZQ_MALLOC (grpTable.AllocSize * sizeof(Group))))       goto exception_0;
  memset(grpTable.Grp, 0, grpTable.AllocSize * sizeof(Group));

#ifdef __DEBUG
  fprintf(stdout, "Group entries: (%d)  %ld\n", USER_GROUPS, USER_GROUPS * sizeof(Group));
#endif

  if(0 > (excpn = THR_init()))                                                 goto exception_2;
  THR_installLeave(GRP_abandone);

  if(0 > (excpn = COM_init()))                                                 goto exception_2;
  COM_setLoc((int (*)(void *srcThr, Addr_t dstAddr, int *mchn, void *thr))getEnd);

  if(0 > (excpn = RPC_init()))                                                 goto exception_2;
  RPC_setLoc((int (*)(void *srcThr, Addr_t dstAddr, int *mchn, void *thr))getEnd);

  /* Synchronize with start of GRP daemon */
#if defined (HAVE_SEM_OPEN)
  sem_unlink("/azqsemgrp");
  grp_sync = sem_open("/azqsemgrp", O_CREAT | O_EXCL, 0, 0);
  if (grp_sync == (sem_t *)SEM_FAILED)                                         goto exception_2;  
#else 
  if (0 > sem_init(&grp_sync, 0, 0))                                           goto exception_2;
#endif

  /* Create the GRP daemon */
  if(0 > (excpn = GRP_server(&gix, (void *)GRP_service_loop,
                                   GRP_PORT,
                                   PTHREAD_STACK_MIN)))                        goto exception_3;

  /* forces grp server to execute */
#if defined (HAVE_SEM_OPEN)
  do {
    if (0 > sem_wait(grp_sync)) {
      if(errno == EINTR)
        continue;
      else
        goto exception_2;
    }
    break;
  } while(1);  
  if (0 > sem_close(grp_sync))                                                 goto exception_2;
  if (0 > sem_unlink("/azqsemgrp"))                                            goto exception_2;  
 
#else  
  do {
    if (0 > sem_wait(&grp_sync)) {
      if(errno == EINTR)
        continue;
      else
        goto exception_2;
    }
    break;
  } while(1);
  if (0 > sem_destroy(&grp_sync))                                              goto exception_2;
#endif
  
  /* allow inet to start receiving messages */
  COM_start();

#ifdef __DEBUG
  fprintf(stdout, "  GRP daemon launched\n");
#endif

  initialised = TRUE;

#ifdef __DEBUG
  fprintf(stdout, "GRP_init: (gix: 0x%x) End\n", gix);
#endif
  return(GRP_E_OK);

exception_3:
#if defined (HAVE_SEM_OPEN)
  sem_close(grp_sync);
  sem_unlink("/azqsemgrp");  
#else  
  sem_destroy(&grp_sync);
#endif  
exception_2:
  AZQ_FREE(grpTable.Grp);
exception_0:
  XPN_print(excpn);
  return(excpn);
}



      /*_________________________________________________________________
     /                                                                   \
    |    GRP_finalize                                                    |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
void GRP_finalize (void) {

  THR_wait(grp_thr, NULL);

  RPC_finalize();
  COM_finalize();
  THR_finalize();

  if (grpTable.Grp)
    AZQ_FREE(grpTable.Grp);
}


      /*________________________________________________________________
     /                                                                  \
    |    GRP_create                                                      |
    |    Create a group object in this machine                           |
    |    When creating a new group, a group object is created in a       |
    |    machine M because:                                              |
    |      a) The thread that creates the group runs in M and needs      |
    |         to operate later on the group, maybe to kill it            |
    |      b) One of the threads of the group will run in M              |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o gix      (Input-Output)                                       |
    |        Group id                                                    |
    |    o machine  (Input)                                              |
    |        Vector of [size] components.                                |
    |        They are the target machines ordered by rank.               |
    |    o size     (Input)                                              |
    |        Number of threads in the group                              |
    |    o creator     (Input)                                           |
    |        Creator machine                                             |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int GRP_create(int *gix, int *machine, int size, int creator) {

  int       excpn;
  Group_t   grp;
  char     *where   = "GRP_create";

  if(*gix == GIX_NONE)
    newGix(gix);
  if(0 > (excpn = grpAlloc(&grp, *gix, machine, size, creator)))                   goto exception;
  THR_getClient(&grp->FatherAddr, &grp->FatherMchn);

  return(GRP_E_OK);

exception:
  XPN_print(excpn);
  return(excpn);
}


      /*________________________________________________________________
     /                                                                  \
    |   GRP_getSize                                                      |
    |   Get the size of the group                                        |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o gix         (Input)                                           |
    |        Group id                                                    |
    |    o grpSize     (Output)                                          |
    |        Number of operators that compose the group                  |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int GRP_getSize(int gix, int *grpSize) {

  int      excpn;
  Group_t  grp;
  char    *where = "GRP_getSize";

  if(0 > grpFind(&grp, gix))                                                    {excpn = GRP_E_INTERFACE;
                                                                                goto exception;}
  *grpSize = grp->Size;

  return(GRP_E_OK);

exception:
  XPN_print(excpn);
  return(excpn);
}



      /*________________________________________________________________
     /                                                                  \
    |   GRP_getLocalSize                                                 |
    |   Get the number of ranks running in this node                     |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o gix         (Input)                                           |
    |        Group id                                                    |
    |    o locSize     (Output)                                          |
    |        Number of ranks of the group running in this machine        |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int GRP_getLocalSize(int gix, int *locSize) {
  
  static
  char    *where = "GRP_getLocalSize";
  int      excpn;
  Group_t  grp;
  
  
  if(0 > grpFind(&grp, gix))                                                    {excpn = GRP_E_INTERFACE;
	                                                                             goto exception;}
  *locSize = grp->LocalSize;
  
  return(GRP_E_OK);
  
exception:
  XPN_print(excpn);
  return(excpn);
}



      /*________________________________________________________________
     /                                                                  \
    |   GRP_getMchn                                                      |
    |   Retrieve the machine where the operator run                      |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o gix         (Input)                                           |
    |        Group id                                                    |
    |    o rank        (Input)                                           |
    |        Rank                                                        |
    |    o machine     (Output)                                          |
    |        The machine where the operator runs                         |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int GRP_getMchn(int gix, int rank, int *mchn) {

  static
  char       *where    = "GRP_getMchn";
  int         excpn;
  Thr_t       me       = THR_self();
  MchnThr_t   block;
  Group_t     grp;

  
  if (me->Address.Group != gix) { 
	
	if(0 > grpFind((Group_t *)&grp, gix))                                      {excpn = GRP_E_INTERFACE;
	                                                                            goto exception;}
	block = grp->MchnThr;
  
  } else {
  
    block = (MchnThr_t)me->GrpInfo;
	
  } 
  
  *mchn  = block[rank].Mchn;
  
  
  return(GRP_E_OK);

exception:
  XPN_print(excpn);
  return(excpn);
}


      /*_________________________________________________________________
     /                                                                   \
    |    GRP_join                                                        |
    |    Aument the group "gix" with a new operator                      |
    |    The operator is the one defined by "name".                      |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o gix      (Input)                                              |
    |        Group id to join                                            |
    |    o name     (Input)                                              |
    |        name of operator that joins the group                       |
    |    o rank     (Input)                                              |
    |        operator rank in the group                                  |
    |    o param    (Input)                                              |
    |        Each operator is provided with a parameter structure.       |
    |        Param is the address of this structure                      |
    |    o paramsize  (Input)                                            |
    |        Size of the "param" structure                               |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int GRP_join (int gix, int rank, int name, CommAttr *commAttr) {

  int         excpn;
  Addr        addr;
  void       *bodyFxn;
  int         stackSize;
  char       *where       = "GRP_join";
  Thr_t       thr;
  Group_t     grp;
  int         argc;
  char      **argv;
  
  if (0 > grpFind(&grp, gix))                                                   { excpn = GRP_E_INTERFACE;
                                                                                  goto exception; }
  /* Create the thread */
  addr.Group = gix;
  addr.Rank  = rank;
  thr = grp->MchnThr[rank].Thr;
  if(0 > (excpn = OPR_getBody     (&bodyFxn,  name)))                           goto exception;
  if(0 > (excpn = OPR_getStackSize(&stackSize, name)))                          goto exception;
  if(0 > (excpn = OPR_getParams   (&argc, &argv, name)))                        goto exception;

  if(0 > (excpn = THR_create(&thr,
                             &addr,
                              bodyFxn,
                              stackSize,
                              argv,
                              argc, commAttr)))                                 goto exception;

  /* Set group info in thread. For performance */
  THR_setGrpInfo(thr, (void *)grp->MchnThr);

  /* Register the thread in the group */
  if(0 > (excpn = setThr(gix, rank, thr)))                                      goto exception2;
  return(GRP_E_OK);

exception2:
  THR_destroy(thr);
exception:
  XPN_print(excpn);
  return(excpn);
}


      /*________________________________________________________________
     /                                                                  \
    |   GRP_kill                                                         |
    |   Kill a group                                                     |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int GRP_kill(int gix) {

  Addr         clientAddr;
  int          clientMchn;
  MchnThr_t    block;
  Group_t      grp;
  int          i;
  int          excpn;
  char        *where       = "GRP_kill";

  if((gix == GIX_NONE) || (0 > grpFind(&grp, gix)))                            {excpn = GRP_E_INTERFACE;
                                                                                goto exception;}
  THR_getClient(&clientAddr, &clientMchn);
  block = grp->MchnThr;
  for (i = 0; i < grp->Size; i++) {
    /* Kill just the local operators */
	if ((block[i].Thr != NULL) && !block[i].Sigmask) {
      if (clientAddr.Group == gix) {
        if (i != clientAddr.Rank)
          THR_kill(block[i].Thr, 0);
      }
      else
        THR_kill(block[i].Thr, 0);
      block[i].Sigmask = TRUE;
    }
  }
  return(GRP_E_OK);

exception:
  XPN_print(excpn);
  return(excpn);
}


      /*________________________________________________________________
     /                                                                  \
    |   GRP_leave                                                        |
    |                                                                    |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int GRP_leave(int retCode) {

  MchnThr_t    block;
  Addr         clientAddr;
  int          clientMchn;
  Group_t      grp;
  int          excpn;
  char        *where       = "GRP_leave";

  THR_getClient(&clientAddr, &clientMchn);

  if(0 > grpFind(&grp, clientAddr.Group))                                       {excpn = GRP_E_INTERFACE;
                                                                                goto exception;}
  block                           = grp->MchnThr;
  block[clientAddr.Rank].ExitCode = retCode;
  grp->LeaveCnt += 1;
  if((grp->LeaveCnt > grp->Size))                                               panic("GRP_leave");
  if((grp->LeaveCnt == grp->Size) && (grp->FatherWaiting)) {
    GRP_done           = TRUE;
    GRP_doneGix        = clientAddr.Group;
    GRP_doneSize       = grp->Size;
    GRP_doneFatherAddr = grp->FatherAddr;
    GRP_doneFatherMchn = grp->FatherMchn;
  }

  return(GRP_E_OK);

exception:
  XPN_print(excpn);
  return(excpn);
}


      /*________________________________________________________________
     /                                                                  \
    |   GRP_wait                                                         |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o gix      (Input)                                              |
    |        Group id to wait                                            |
    |    o status   (Output)                                             |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int GRP_wait(int gix, int *status) {

  Group_t       grp;
  int           excpn       = GRP_E_INTERFACE;
  char         *where       = "GRP_wait";

  if(gix == GIX_NONE)                                                          goto exception;
  if(0 > grpFind(&grp, gix))                                                   goto exception;

  if(grp->FatherWaiting)                                                       goto exception;
  grp->FatherWaiting = TRUE;
  GRP_dont_reply     = TRUE;
  if(grp->LeaveCnt == grp->Size) {
    GRP_done           = TRUE;
    GRP_doneGix        = gix;
    GRP_doneSize       = grp->Size;
    GRP_doneFatherAddr = grp->FatherAddr;
    GRP_doneFatherMchn = grp->FatherMchn;
  }

  return(GRP_E_OK);

exception:
  XPN_print(excpn);
  return(excpn);
}


      /*________________________________________________________________
     /                                                                  \
    |   GRP_wait2                                                        |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o gix      (Input)                                              |
    |        Group id to wait                                            |
    |    o status   (Output)                                             |
    |        Vector of return values                                     |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int GRP_wait2(int gix, int *status) {

  int           i;
  MchnThr_t     block;
  Group_t       grp      = GIX_NONE; /* To avoid compiler warning */

#ifdef __DEBUG
  fprintf(stdout, "GRP_wait2:\n");
#endif
  if(0 > grpFind(&grp, gix))                                                    panic("GRP_wait2");
  /* Complete the vector of return values with the operators running in this machine */
  block = grp->MchnThr;
  for(i = 0; i < grp->Size; i++) {
    status[i] = block[i].ExitCode;
  }
#ifdef __DEBUG
  fprintf(stdout, "GRP_wait2: End\n");
#endif

  return(GRP_E_OK);
}


      /*________________________________________________________________
     /                                                                  \
    |   GRP_destroy                                                      |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o gix      (Input)                                              |
    |        Group id to wait                                            |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int GRP_destroy(int gix) {

  int           i;
  MchnThr_t     block;
  Group_t       grp      = GIX_NONE; /* To avoid compiler warning */

#ifdef __DEBUG
  fprintf(stdout, "GRP_destroy:\n");
#endif
  if(0 > grpFind(&grp, gix))                                                    panic("GRP_destroy");
  /* The operators of this machine are now zombies. Wait for them  */
  block = grp->MchnThr;
  for (i = 0; i < grp->Size; i++) {
	if (block[i].Thr != NULL)
      THR_wait(block[i].Thr, NULL);
  }
  grpFree(grp);
#ifdef __DEBUG
  fprintf(stdout, "GRP_destroy: End\n");
#endif

  return(GRP_E_OK);
}


      /*________________________________________________________________
     /                                                                  \
    |   GRP_shutdown                                                     |
    |      Shutdown the server. Free resources                           |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o none                                                          |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int GRP_shutdown(void) {

#ifdef __DEBUG
  fprintf(stdout, "GRP_shutdown:\n");
#endif

#ifdef __DEBUG
  fprintf(stdout, "GRP_shutdown: End\n");
#endif

  return(GRP_E_OK);
}


      /*________________________________________________________________
     /                                                                  \
    |   GRP_start                                                        |
    |   Start the group                                                  |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int GRP_start (int gix) {

  char          *where       = "GRP_start";
  int            excpn;
  int            i = 0, j;
  MchnThr_t      block;
  Group_t        grp;

  if(0 > grpFind(&grp, gix))                                                    {excpn = GRP_E_INTERFACE;
                                                                                 goto exception;}
  if(grp->Started) {
    fprintf(stdout, "\t\tGRP_start: Already started\n");
    return(GRP_E_OK);
  }
  block = grp->MchnThr;

#ifdef USE_FASTBOXES
  /* Initialize fastBoxes
   * This is the place to initialize the fastBoxes because only at this time the group description "grp->MchnThr" is complete 
   */
  for (i = 0; i < grp->Size; i++) {
    if (block[i].Thr != NULL) {
      block[i].Thr->LocalGroupSize = grp->LocalSize;
      for(j = 0; j < grp->LocalSize; j++) { 
        int senderGlobalRank; 
        if(0 > (senderGlobalRank = GRP_localRank2globalRank(gix, j)))           {excpn = GRP_E_INTEGRITY;
                                                                                 goto exception;}
        (block[i].Thr->FastBox)[j].SenderGlobalRank = senderGlobalRank;
        (block[i].Thr->FastBox)[j].SenderLocalRank  = block[senderGlobalRank].Thr->LocalRank;
        (block[i].Thr->FastBox)[j].Turn             = TURN_SEND;
      } 
    }
  }
#endif

  /* Start the execution of the threads */
  for (i = 0; i < grp->Size; i++) {
    if (block[i].Thr != NULL) {
      if(0 > (excpn = THR_start(block[i].Thr)))                                 goto exception;
    }
  }
  grp->Started = TRUE;
  return(GRP_E_OK);

exception:
  for(j = 0; j < i; j++)
    THR_kill(block[j].Thr, 0);
  XPN_print(excpn);
  return(excpn);
}



      /*_________________________________________________________________
     /                                                                   \
    |    GRP_enroll                                                       |
    |                                                                     |
    |    Makes the invoking system thread to become the Azequia thread    |
    |    of a single-thread group                                         |
    |                                                                     |
     \____________/  ____________________________________________________/
                 / _/
                /_/
               */
int GRP_enroll () {

  Addr     addr;
  int      excpn,
           mchn;
  Group_t  grp;
  Thr_t    thr;
  int      gix       = GIX_NONE;
  char    *where     = "GRP_enroll";

#ifdef __DEBUG
  fprintf(stdout, "GRP_enroll:\n");
#endif
  /* [1] Create the single-threaded group object */
  mchn = getCpuId();
  if(0 > (excpn = GRP_create(&gix, &mchn, 1, mchn)))                            goto exception;

  if (0 > grpFind(&grp, gix))                                                   { excpn = GRP_E_INTERFACE;
                                                                                  goto exception; }
  /* [2] Create the environment object of the invoking thread */
  addr.Group = gix;
  addr.Rank  = 0;
  thr = grp->MchnThr[addr.Rank].Thr;
  
  if(0 > (excpn = THR_create(&thr, &addr, NULL, 0, NULL, 0, NULL)))             goto exception;

  /* [3] Register the environment in the group */
  if(0 > (excpn = setThr(gix, 0, thr)))                                         goto exception;

  /* [4] Set the environment as variable the invoking thread */
  if(pthread_setspecific(key, thr))                                             {excpn = GRP_E_SYSTEM;
                                                                                goto exception;}
#ifdef __DEBUG
  fprintf(stdout, "GRP_enroll: End\n");
#endif
  return(GRP_E_OK);

exception:
  THR_destroy(thr);
  GRP_kill(gix);
  XPN_print(excpn);
  return(excpn);
}


/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/



int getInfoLayout(int gix, int orig, int dest, int *mchn_orig, int *mchn_dest, int *socket_orig, int *socket_dest) {
  
  int        excpn;
  Group_t    grp;
  MchnThr_t  block;
  static
  char      *where = "getInfo";
  
  if(0 > grpFind((Group_t *)&grp, gix))                                 {excpn = GRP_E_INTERFACE;
	goto exception;}
  block = grp->MchnThr;
  *mchn_orig            = block[orig].Mchn;
  *mchn_dest            = block[dest].Mchn;
  
  *socket_orig = block[orig].Placement->Socket;
  *socket_dest = block[dest].Placement->Socket;
  
  return(GRP_E_OK);
  
exception:
  XPN_print(excpn);
  return(excpn);  
}


















