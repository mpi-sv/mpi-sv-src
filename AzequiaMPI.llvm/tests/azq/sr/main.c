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
 *   MACROS to eliminate                                          *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*
 *   Constants definition                                         *
 *----------------------------------------------------------------*/
#define PARALLEL_GRP_NR         1
#define ITERATION_NR           1

#define GROUP_ID              100

#define ENABLE_SLM              0

/*----------------------------------------------------------------*
 *   Global variables                                             *
 *----------------------------------------------------------------*/
int  nodes_max  = 0;
int  data_max   = 0;

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
int createGroup (int argc, char *argv[], int *grpId) {

  int       i;
  int      *mchn;
  int      *prio;
  CommAttr  CommAttr;
  int       minPrio     = sched_get_priority_min(SCHED_FIFO);
  static 
  int       groupId     = GROUP_ID;

  mchn = (int *) malloc (nodes_max * sizeof(int));
  prio = (int *) malloc (nodes_max * sizeof(int));

  for(i = 0; i < nodes_max; i++) {
    prio [i] = minPrio;
    /*if (i == 0)
      mchn   [i] = 0;
    else 
      mchn   [i] = 1; 
    */
    mchn     [i] = rand() % INET_getNodes();
    //mchn  [i] = 0;
  }

  if(0 > GRP_create (&groupId, mchn, nodes_max, getCpuId()))                   goto exception2;

  for(i = 0; i < nodes_max; i++) {

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
  
  retCode = (int *) malloc (nodes_max * sizeof(int));
  gix     = (int *) malloc (PARALLEL_GRP_NR * sizeof(int));

  for(i = 0; i < ITERATION_NR; i++) {

#ifdef __DEBUG
    fprintf(stdout, "\n\n -------------------------   %d.Creando    -------------------------\n", i);
    fflush(stdout);
#endif

    for(j = 0; j < PARALLEL_GRP_NR; j++) {
      if (0 > createGroup(argc, argv, &gix[j]))                                goto exception;
    }

#ifdef __DEBUG
    fprintf(stdout, "\n\n -------------------------    %d.Esperando    -------------------------\n", i);
#endif

    for(j = 0; j < PARALLEL_GRP_NR; j++) {
      if (0 > (err = GRP_wait(gix[j], retCode)))                               goto exception2;

#ifdef __DEBUG
      fprintf(stdout, "AZQ_main: (%d) Ret[%d][%d] = %d\n", err, j, 0, retCode[0]);
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
    fprintf(stderr, "\nUse:  sr message_size num_nodes\n\n");
    exit(1);
  }
  nodes_max = atoi(argv[2]);
  data_max  = atoi(argv[1]);

  if (nodes_max % 2) {
    fprintf(stderr, "\nnum_nodes  must be multiple of 2\n\n");
    exit(1);
  }

#ifdef __DEBUG
  fprintf(stdout, "main: Send/Recv application with %d nodes\n", nodes_max);
#endif

  azqcfg.OperatorNr = nodes_max;
  azqcfg.GroupNr    = PARALLEL_GRP_NR;

  srand(432908);

  /* 1. Init Azequia */
  if(0 > AZQ_init((void *)&azqcfg, oprTable, 1, argc, argv)) {
    fprintf(stderr, "main: Cannot start AZEQUIA\n");
    return(-1);
  }

#ifdef __DEBUG
  fprintf(stdout, "main: End.\n\n");
#endif

  return(0);
}

