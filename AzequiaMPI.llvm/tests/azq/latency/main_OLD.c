#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef __OSI
#include <osi.h>
#endif
#include <azq.h>


/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>




/*----------------------------------------------------------------*
 *   For DSP compatibility (to eliminate)                         *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*
 *   Operator's main in another file                              *
 *----------------------------------------------------------------*/
extern int latency                 (int *param);


/*----------------------------------------------------------------*
 *   Operators to run                                             *
 *----------------------------------------------------------------*/
#define LATENCY_OPERATOR                  0x3e2a7d85

int oprTable[] = {
       /* Name                     Body Function            Stack size
          ----                     -------------            ---------- */
        LATENCY_OPERATOR,          (int)latency,            16*1024,
};


/*----------------------------------------------------------------*
 *   Creating an Azequia application                              *
 *----------------------------------------------------------------*/

      /*________________________________________________________________
     /                                                                  \
    |    createLatency                                                   |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
#define PARALLEL_GRP_NR        1
#define ITERATION_NR           1  //0xffffffff
#define LATENCY_MAX            4
int createLatency (int *gix)
{
  int      i;
  int      mchn[LATENCY_MAX];
  int      prio[LATENCY_MAX];
  int      groupId             = GIX_NONE;
  int      minPrio             = sched_get_priority_min(SCHED_FIFO);
  int      paramSize           = sizeof(int);
  int      dummy               = 0;
  CommAttr CommAttr;


  /*prio[0] = minPrio +1;
  prio[1] = minPrio;*/
  prio[0] = 3;
  prio[1] = 4;
  prio[2] = 4;
  prio[3] = 4;
  fprintf(stdout, "createLatency: (%d, %d). Prios %d and %d\n", getGroup(), getRank(), prio[0], prio[1]);
  for(i = 0; i < LATENCY_MAX; i++) {
    //mchn[i] = getCpuId();
    mchn[i] = 0;
  }

  if(0 > GRP_create (&groupId, mchn, LATENCY_MAX, getCpuId()))                  goto exception_0;
  for(i = 0; i < LATENCY_MAX; i++) {
    CommAttr.Flags = 0;
    //CommAttr.Flags = COMMATTR_FLAGS_SLM_ENABLED;
    if(0 > GRP_join(groupId, i, LATENCY_OPERATOR, prio[i],
                             (void *)(&dummy),
                             paramSize, &CommAttr))                             goto exception;
  }
  fprintf(stdout, "Starting LATENCY ... \n");
  if(0 > GRP_start(groupId))                                                    goto exception;
  *gix = groupId;
  fprintf(stdout, "- latency Launched -\n");
  return(0);

exception_0:
  fprintf(stdout, ">>> Exception raised in createLatency\n");
  return(-1);
exception:
  fprintf(stdout, ">>> Exception raised in createLatency\n");
  GRP_kill(groupId);
  return(-1);
}



int AZQ_main(void) 
{
  int gix[8];
  int i, j, k;
  int retCode[LATENCY_MAX];

  OPR_register(&oprTable, 1);

  for(j = 0; j < LATENCY_MAX; j++)
    retCode[j] = 55;

  /* 2. Only one processor launches applications */
  if(getCpuId() != 0) {
    fprintf(stdout, "CPU %d. I do not lauch anything\n", getCpuId());
    sleep(3600);
    return(0);
  }

  /* 3. Launch applications */
  for(i = 0; i < ITERATION_NR; i++) {
    //pthread_sleep(1);
    //fprintf(stdout, "\n\n````````````````````````%d..........................%%%%%%%%%%%%%%%%%%..\n", i);
    for(k = 0; k < PARALLEL_GRP_NR; k++) {
      if (0 > createLatency(&gix[k]))                                           goto exception;
    }
    for(k = 0; k < PARALLEL_GRP_NR; k++) {
      if (0 > GRP_wait(gix[k], retCode))                                            goto exception;
      fprintf(stdout, "\n");
      for(j = 0; j < LATENCY_MAX; j++) {
        fprintf(stdout, "AZQ_main: Ret[%d][%d] = %d\n", k, j, retCode[j]);
      }
    }
  }
  return(0);

exception:
  fprintf(stdout, ">>> Exception raised in AZQ_main\n");
  return(-1);
}


/*----------------------------------------------------------------*
 *   Main                                                         *
 *----------------------------------------------------------------*/
int main(int argc, char **argv) {
  /*struct in_addr ipAddr;
  AZQ_topology   azqTopology = {PROCESSOR_NR, Routed_Send, UNKNOWN_CPU_ID};
  AZQ_Param      azqParam    = {&azqTopology, oprTable};*/

  /* Init Routing table */
  /*fprintf(stdout, "main: Latency\n");
  if(!inet_aton("127.0.00.1", &ipAddr)) {
    fprintf(stdout, "Bad IP address\n");
    exit(1);
  }
  Routed_Send[0][0] = ipAddr.s_addr;*/

//fprintf(stdout, "IP en Network Byte Order = %d\n", Routed_Send[0][0]);
//fprintf(stdout, "Port = %d\n", Routed_Send[0][0]);

  /* Init Azequia */
  int azqparam = 0; // AZQ_rt desactivado
  fprintf(stdout, "main: Latency\n");
  if(0 > AZQ_init(&azqparam, argc, argv)) {
    fprintf(stdout, "main: Cannot start AZEQUIA\n");
    return(-1);
  }

  /* Wait for AZQ_main termination */
  AZQ_wait();
  return(0);
}

