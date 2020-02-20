/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef __OSI
#include <osi.h>
#endif
#include <azq.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*----------------------------------------------------------------*
 *   For DSP compatibility (to eliminate)                         *
 *----------------------------------------------------------------*/
#include <net.h>
#include <limits.h>



/*----------------------------------------------------------------*
 *   Operator's main in another file                              *
 *----------------------------------------------------------------*/
extern int timed_operator  (int *param);


/*----------------------------------------------------------------*
 *   Operators to run                                             *
 *----------------------------------------------------------------*/
#define TIMED_OPERATOR                  0xbf3af287

int oprTable[] = {
       /* Name                     Body Function            Stack size
          ----                     -------------            ---------- */
        TIMED_OPERATOR,        (int)timed_operator,         (32 * 1024)
};


/*----------------------------------------------------------------*
 *   Creating an Azequia application                              *
 *----------------------------------------------------------------*/

      /*________________________________________________________________
     /                                                                  \
    |    createRadiator                                                  |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
#define SENDERS                1
#define RECEIVERS              1
#define PARALLEL_GRP_NR        1
//#define ITERATION_NR           0xefffffff
#define ITERATION_NR           1

//#define OPERATOR_MAX         (SENDERS + RECEIVERS + 1)
#define OPERATOR_MAX         (SENDERS + RECEIVERS)
#define DATA_MAX              128
int createTimedOperator (int *gix) {

  int        i;
  int        mchn     [OPERATOR_MAX];
  int        prio     [OPERATOR_MAX];
  void      *param    [OPERATOR_MAX];
  int        paramSize[OPERATOR_MAX];
  CommAttr   CommAttr;

  int        paramObj[3]         = {DATA_MAX, SENDERS, RECEIVERS};
  int        groupId             = GIX_NONE;
  int        minPrio             = sched_get_priority_min(SCHED_FIFO);

  /*fprintf(stdout, "\n\nCreateRadiator:\n");*/
  /*for(i = 0; i < RADIATOR_MAX; i++)
    prio     [i] = minPrio;*/

  for(i = 0; i < OPERATOR_MAX; i++)
    prio     [i] = minPrio + i;
  //prio[RADIATOR_MAX - 1] = minPrio + SENDERS/2;


  for(i = 0; i < OPERATOR_MAX; i++) {
    param    [i] = (void *)paramObj;
    paramSize[i] = 3 * sizeof(int);
  //mchn     [i] = getCpuId();
    mchn     [i] = 0;
  }

  if(0 > GRP_create (&groupId, mchn, OPERATOR_MAX, getCpuId()))                 goto exception2;
  for(i = 0; i < OPERATOR_MAX; i++) {
    CommAttr.Flags = 0;
    //CommAttr.Flags = COMMATTR_FLAGS_SLM_ENABLED;

    if(0 > GRP_join(groupId, i, TIMED_OPERATOR, prio[i],
                                         param[i], paramSize[i], &CommAttr))    goto exception;

  }
  if(0 > GRP_start(groupId))                                                    goto exception;
  *gix = groupId;
  //fprintf(stdout, "- Radiator Launched -\n");
  return(0);

exception:
  fprintf(stdout, ">>> Exception raised in createTimedOperator\n");
  GRP_kill(groupId);
  return(-1);

exception2:
  fprintf(stdout, ">>> Exception raised in createTimedOperator\n");
  return(-1);
}



int AZQ_main(void) {

  int gix[32];
  int i, j, k;
  int retCode[OPERATOR_MAX];

  fprintf(stdout, "AZQ_main:\n");
  
  OPR_register(&oprTable, 1);
  
  /* 2. Only one processor launches applications */
  /*if(getCpuId() != 0)
    return(0);*/
  if(getCpuId() != 0) {
    fprintf(stdout, "CPU %d. I do not lauch anything\n", getCpuId());
  //sleep(60*60);
    return(0);
  }


  /* 3. Launch applications */
  for(i = 0; i < ITERATION_NR ; i++) {
    //sleep(1);
    fprintf(stdout, "\n\n````````````````````````%d.Creando .........................%%%%%%%%%%%%%%%%%%..\n", i);
    for(k = 0; k < PARALLEL_GRP_NR; k++) {
      if (0 > createTimedOperator(&gix[k]))                                    goto exception;
      //fprintf(stdout, "Creado %d\n", k);
    }
    fprintf(stdout, "\n\n````````````````````````%d.Esperando .........................%%%%%%%%%%%%%%%%%%..\n", i);
    for(k = 0; k < PARALLEL_GRP_NR; k++) {
      if (0 > GRP_wait(gix[k], retCode))                                       goto exception;
      //sleep(1);
      for(j = 0; j < OPERATOR_MAX; j++) {
        fprintf(stdout, "AZQ_main: Ret[%d][%d] = %d\n", k, j, retCode[j]);
      }
    }
  }
  fprintf(stdout, "AZQ_main: End\n");
  return(0);

exception:
  fprintf(stdout, ">>> Exception raised in AZQ_main\n");
  return(-1);
}


/*----------------------------------------------------------------*
 *   Main                                                         *
 *----------------------------------------------------------------*/
int main(int argc, char **argv) {

  int azqparam = 0; // AZQ_rt desactivado

  /* Init Azequia */
  if(0 > AZQ_init((void *)&azqparam, argc, argv)) {
    fprintf(stdout, "main: Cannot start AZEQUIA\n");
    return(-1);
  }

  /* Wait for AZQ_main termination */
  AZQ_wait();
  return(0);
}

