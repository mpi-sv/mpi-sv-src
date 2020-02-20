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
#define PARALLEL_GRP_NR         5
#define ITERATION_NR           50

#define GROUP_ID              100

#define ENABLE_SLM              1

/*----------------------------------------------------------------*
 *   Global variables                                             *
 *----------------------------------------------------------------*/
int  radiator_max = 0;
int  data_max     = 0;

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
int createRadiatorGroup (int argc, char *argv[], int *grpId) {

  int       i;
  int       num_nodes;
  int      *mchn;
  int      *prio;
  CommAttr  CommAttr;
  int       minPrio     = sched_get_priority_min(SCHED_FIFO);
  static 
  int       groupId     = GROUP_ID;

  mchn = (int *) malloc (radiator_max * sizeof(int));
  prio = (int *) malloc (radiator_max * sizeof(int));

  if (0 == (num_nodes = INET_getNodes() - 1)) num_nodes = 1;

  for(i = 0; i < radiator_max; i++) {
    prio  [i] = minPrio;
    //mchn  [i] = 0;
    
    //if (i == (radiator_max - 1))  mchn  [i] = 0;
    //else                          mchn  [i] = 1;

    /*
    if (i == (radiator_max - 1)) mchn[i] = 0;
    else                         mchn[i] = (i  % num_nodes) + 1;

    if (mchn[i] >= num_nodes) mchn[i] -= 1;
    */

    mchn  [i] = i % INET_getNodes();
  }

  if(0 > GRP_create (&groupId, mchn, radiator_max, getCpuId()))                goto exception2;

  for(i = 0; i < radiator_max; i++) {

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

  //int   retCode[PARALLEL_GRP_NR];
  int  *retCode;
  int   i, j;
  //int   gix[PARALLEL_GRP_NR];
  int  *gix;
  int   err;
  
  //OPR_register(oprTable, 1);

  retCode = (int *) malloc (radiator_max * sizeof(int));
  gix     = (int *) malloc (PARALLEL_GRP_NR * sizeof(int));

  for(i = 0; i < ITERATION_NR; i++) {

    fprintf(stdout, "\n\n ==============================   %d. Creando   ============================== \n", i);
    fflush(stdout);

    for(j = 0; j < PARALLEL_GRP_NR; j++) {
      if (0 > createRadiatorGroup(argc, argv, &gix[j]))                        goto exception;
    }

#ifdef __DEBUG
    fprintf(stdout, "\n\n ==============================   %d. Esperando   ==============================\n", i);
#endif

    for(j = 0; j < PARALLEL_GRP_NR; j++) {
      if (0 > (err = GRP_wait(gix[j], retCode)))                               goto exception2;

#ifdef __DEBUG
      fprintf(stdout, "AZQ_main: (%d) Ret[%d][%d] = %d\n", err, j, 0, retCode[0]);
#endif

    }

    //sleep(1);
  }

  //GRP_shutdown();

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

  if (argc != 4) {
    fprintf(stderr, "\nUse:  radiator  message_size  num_senders  num_receivers\n\n");
    exit(1);
  }
  radiator_max = atoi(argv[2]) + atoi(argv[3]) + 1;
  data_max     = atoi(argv[1]);

#ifdef __DEBUG
  fprintf(stdout, "main: Radiator application with %d nodes\n", radiator_max);
#endif

  azqcfg.OperatorNr = radiator_max;
  azqcfg.GroupNr    = PARALLEL_GRP_NR;

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

