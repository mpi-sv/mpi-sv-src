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
 *   Number of operator to run                                    *
 *----------------------------------------------------------------*/
#define NUM_OPERATOR     2

/*----------------------------------------------------------------*
 *   Operator's main in another file                              *
 *----------------------------------------------------------------*/
extern int operator_main  (void *param);

/*----------------------------------------------------------------*
 *   Operators to run                                             *
 *----------------------------------------------------------------*/
#define OPERATOR                  0xbf3af287

REG_Entry oprTable[] = {
       /* Name               Body Function            Stack size
          ----               -------------            ---------- */
       { OPERATOR,           operator_main,           (32 * 1024) }
};

/*----------------------------------------------------------------*
 *   Creating an Azequia application                              *
 *----------------------------------------------------------------*/

int AZQ_main(int argc, char **argv) {

  int        retCode  [NUM_OPERATOR];
  int        i;
  int        mchn     [NUM_OPERATOR];
  int        prio     [NUM_OPERATOR];
  CommAttr   CommAttr;
  int        groupId  = GIX_NONE;
  int        minPrio  = sched_get_priority_min(SCHED_FIFO);



  /* Register operators */
  OPR_register(oprTable, 1);

  /* 2. Only one processor launches applications */
  if(getCpuId() != 0) {
    fprintf(stdout, "CPU %d. I do not lauch anything\n", getCpuId()); fflush(NULL);
    return 0;
  }

  /* 3. Launch applications */
  for(i = 0; i < NUM_OPERATOR; i++) {
    prio     [i] = minPrio;

    //if (i == 0) mchn[i] = 0;
    //else        mchn[i] = (i % 7) + 1;

    //mchn     [i] = i % 8;

    mchn     [i] = 0;
  }

  if(0 > GRP_create (&groupId, mchn, NUM_OPERATOR, getCpuId()))                goto exception2;
  //printf("Launching group ... %d \n", groupId);  fflush(NULL);
  for(i = 0; i < NUM_OPERATOR; i++) {
    //CommAttr.Flags = 0;
    CommAttr.Flags = COMMATTR_FLAGS_SLM_ENABLED;

    //if (i == 0) CommAttr.Flags = 0;
    //else CommAttr.Flags = COMMATTR_FLAGS_SLM_ENABLED;

    if(0 > GRP_join(groupId, i, OPERATOR, prio[i],
                                         NULL, 0, &CommAttr))                  goto exception;

  }
  if(0 > GRP_start(groupId))                                                   goto exception;

  if(0 > GRP_wait(groupId, retCode))                                           goto exception;

  GRP_shutdown();

  for (i = 0; i < NUM_OPERATOR; i++)
    if (retCode[i] != 0) {
      fprintf(stdout, "AZQ_main: finishing with errors: %d  %d\n", i, retCode[i]);
      return(retCode[i]);
    }

  //fprintf(stdout, "AZQ_main: End\n");

  return 0;

exception:
  fprintf(stdout, ">>> Exception raised in AZQ_main\n");
  GRP_kill(groupId);
  return(-1);

exception2:
  fprintf(stdout, ">>> Exception raised in AZQ_main creating group\n");
  return(-2);
}


/*----------------------------------------------------------------*
 *   Main                                                         *
 *----------------------------------------------------------------*/
int main(int argc, char **argv) {

  struct sched_param  sp;
  AZQ_Config azqcfg = AZQ_DEFAULT_CONFIG;
/*
  sp.sched_priority = sched_get_priority_max(SCHED_FIFO);
  if(0 > sched_setscheduler(getpid(), SCHED_FIFO, &sp)) {
    printf("%s::execTime:%s calling sched_setscheduler\n", __FILE__, strerror(errno));
    return(-1);
  }

  if(0 > mlockall(MCL_FUTURE | MCL_CURRENT)) {
    printf("%s::execTime:%s calling mlockall\n", __FILE__, strerror(errno));
    return(-1);
  }
*/

  azqcfg.OperatorNr = NUM_OPERATOR;
  azqcfg.GroupNr    = 1;

  /* Init Azequia */
  if(0 > AZQ_init((void *)&azqcfg, argc, argv)) {
    fprintf(stdout, "main: Cannot start AZEQUIA\n");
    return(-1);
  }

  /* Wait for AZQ_main termination */
  AZQ_wait();

#ifdef __DEBUG
  fprintf(stdout, "main: End.\n\n");
#endif

  return(0);
}

