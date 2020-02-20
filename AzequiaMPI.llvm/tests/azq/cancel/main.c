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
extern int latency_azq  (int *param);


/*----------------------------------------------------------------*
 *   Operators to run                                             *
 *----------------------------------------------------------------*/
#define OPERATOR                  0xbf3af287

int oprTable[] = {
       /* Name                     Body Function            Stack size
          ----                     -------------            ---------- */
        OPERATOR,               (int)latency_azq,            (16 *1024)
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
#define NUM_OPERATOR     2

int createOperator(int *gix) {

  int        i;
  int        mchn     [NUM_OPERATOR];
  int        prio     [NUM_OPERATOR];
  void      *param    [NUM_OPERATOR];
  int        paramSize[NUM_OPERATOR];
  CommAttr   CommAttr;

  int        groupId  = GIX_NONE;
  int        minPrio  = sched_get_priority_min(SCHED_FIFO);


  for(i = 0; i < NUM_OPERATOR; i++) {
    prio     [i] = minPrio + i;
    param    [i] = (void *)NULL;
    paramSize[i] = 0;
    mchn     [i] = 0;
  }

  if(0 > GRP_create (&groupId, mchn, NUM_OPERATOR, getCpuId()))                 goto exception2;
  for(i = 0; i < NUM_OPERATOR; i++) {
    CommAttr.Flags = 0;
    //CommAttr.Flags = COMMATTR_FLAGS_SLM_ENABLED;

    if(0 > GRP_join(groupId, i, OPERATOR, prio[i],
                                         param[i], paramSize[i], &CommAttr))    goto exception;

  }
  if(0 > GRP_start(groupId))                                                    goto exception;
  *gix = groupId;
  //fprintf(stdout, "- Radiator Launched -\n");
  groupId++;
  return(0);

exception:
  fprintf(stdout, ">>> Exception raised in createOperator\n");
  GRP_kill(groupId);
  return(-1);

exception2:
  fprintf(stdout, ">>> Exception raised in createOperator\n");
  return(-1);
}



int AZQ_main(void) {

  int gix[128];
  int i, j, k;
  int retCode[NUM_OPERATOR];

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
  if (0 > createOperator(&gix[0]))                                           goto exception;
  if (0 > GRP_wait(gix[0], retCode))                                         goto exception;

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

