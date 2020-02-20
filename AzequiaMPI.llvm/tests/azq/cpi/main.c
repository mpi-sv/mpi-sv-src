/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#ifdef __OSI
#include <osi.h>
#endif
#include <azq.h>
#include <azq_types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/types.h>


/*----------------------------------------------------------------*
 *   Constants definition                                         *
 *----------------------------------------------------------------*/
#define PARALLEL_GRP_NR         1
#define ITERATION_NR            2 

#define GROUP_ID              100

#define ENABLE_SLM              1

/*----------------------------------------------------------------*
 *   Global variables                                             *
 *----------------------------------------------------------------*/
int  cpi_max   = 0;
int  precision = 0;

/*----------------------------------------------------------------*
 *   Node's main in another file                                  *
 *----------------------------------------------------------------*/
extern int node_main  (int argc, char *argv[]);

/*----------------------------------------------------------------*
 *   Nodes to run                                                 *
 *----------------------------------------------------------------*/
#define OPERATOR             0xa3d8f9c1

REG_Entry oprTable[] = {
       /* Name               Body Function            Stack size
          ----               -------------            ---------- */
       {  OPERATOR,           node_main,              (32 * 1024)  }
};

/*----------------------------------------------------------------*
 *   Launching a group                                            *
 *----------------------------------------------------------------*/
int createCpiGroup (int argc, char *argv[], int *grpId) {

  int       i;
  int       num_nodes;
  int      *mchn;
  int      *prio;
  CommAttr  CommAttr;
  int       minPrio     = sched_get_priority_min(SCHED_FIFO);
  static 
  int       groupId     = GROUP_ID;

  mchn = (int *) malloc (cpi_max * sizeof(int));
  prio = (int *) malloc (cpi_max * sizeof(int));

  if (0 == (num_nodes = INET_getNodes() - 1)) num_nodes = 1;

  for(i = 0; i < cpi_max; i++) {
    prio  [i] = minPrio;
    mchn  [i] = i % INET_getNodes();
  }

  if(0 > GRP_create (&groupId, mchn, cpi_max, getCpuId()))                     goto exception2;

  for(i = 0; i < cpi_max; i++) {

#if (ENABLE_SLM == 1)
    CommAttr.Flags = COMMATTR_FLAGS_SLM_ENABLED;
#else
    CommAttr.Flags = 0;
#endif

    if(0 > GRP_join(groupId, i, OPERATOR, prio[i], argv, argc, &CommAttr))     goto exception;

  }
  if(0 > GRP_start(groupId))                                                   goto exception;

  *grpId = groupId;
  groupId++;

  free (mchn);
  free (prio);

  return 0;

exception:
  fprintf(stderr, ">>> Exception raised in AZQ_main\n");
  GRP_kill(groupId);
  return(-1);

exception2:
  fprintf(stderr, ">>> Exception raised in AZQ_main creating group\n");
  return(-2);
}


/*----------------------------------------------------------------*
 *   Creating an Azequia application                              *
 *----------------------------------------------------------------*/


int AZQ_clnt(int argc, char **argv) {

  int  *retCode;
  int   i, j;
  int  *gix;
  int   err;
  
  retCode = (int *) malloc (cpi_max * sizeof(int));
  gix     = (int *) malloc (PARALLEL_GRP_NR * sizeof(int));

  for(i = 0; i < ITERATION_NR; i++) {

    fprintf(stdout, "\n\n ==============================   %d. Creando   ============================== \n", i);
    fflush(stdout);

    for(j = 0; j < PARALLEL_GRP_NR; j++) {
      if (0 > createCpiGroup(argc, argv, &gix[j]))                             goto exception;
    }

#ifdef __DEBUG
    fprintf(stdout, "\n\n ==============================   %d. Esperando   ==============================\n", i);
#endif

    for(j = 0; j < PARALLEL_GRP_NR; j++) {
      if (0 > (err = GRP_wait(gix[j], retCode)))                               goto exception2;

#ifdef __DEBUG
      if (retCode[0]) fprintf(stdout, "AZQ_main: (%d) Ret[%d][%d] = %d\n", err, j, 0, retCode[0]);
#endif

    }

  }

  free (gix);
  free(retCode);

  return 0;

exception2:
  fprintf(stderr, ">>> Exception raised in AZQ_main waiting for a group\n");
exception:
  fprintf(stderr, ">>> Exception raised in AZQ_main creating a group\n");
  return(-2);
}


/*----------------------------------------------------------------*
 *   Main                                                         *
 *----------------------------------------------------------------*/
int main(int argc, char **argv) {

  AZQ_Config azqcfg = AZQ_DEFAULT_CONFIG;

#ifdef __SCHED_FIFO
  struct sched_param  sp;

  sp.sched_priority = sched_get_priority_max(SCHED_FIFO);
  if(0 > sched_setscheduler(getpid(), SCHED_FIFO, &sp)) {
    fprintf(stderr, "%s::execTime:%s calling sched_setscheduler\n", __FILE__, strerror(errno));
    return(-1);
  }

#ifdef __LOCKALL
  if(0 > mlockall(MCL_FUTURE | MCL_CURRENT)) {
    fprintf(stderr, "%s::execTime:%s calling mlockall\n", __FILE__, strerror(errno));
    return(-1);
  }
#endif

#endif

  if (argc != 3) {
    fprintf(stderr, "\nUse:  cpi  precision  num_ranks\n\n");
    exit(1);
  }
  cpi_max   = atoi(argv[2]);
  precision = atoi(argv[1]);

  azqcfg.OperatorNr = cpi_max;
  azqcfg.GroupNr    = PARALLEL_GRP_NR;

  /* 1. Init Azequia */
  if(0 > AZQ_init((void *)&azqcfg, oprTable, 1, argc, argv)) {
    fprintf(stderr, "main: Cannot start AZEQUIA\n");
    return(-1);
  }

  return(0);
}

