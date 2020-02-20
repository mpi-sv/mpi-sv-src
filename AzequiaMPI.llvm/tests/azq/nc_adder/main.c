#include <azq.h>


/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#include <stdio.h>


/*----------------------------------------------------------------*
 *   For DSP compatibility (to eliminate)                         *
 *----------------------------------------------------------------*/
#include <net.h>
#include <limits.h>

#define PROCESSOR_NR  1
int MACHINE_MAX = PROCESSOR_NR;
int DEVICE = NO_NET;
int Routed_Send[PROCESSOR_NR][PROCESSOR_NR] =
                /*    P0
                     ----   */
       /* P0 */  { {   SHM   }};


/*----------------------------------------------------------------*
 *   Operator's main in another file                              *
 *----------------------------------------------------------------*/
extern int adder_no_chn  (int *param);


/*----------------------------------------------------------------*
 *   Operators to run                                             *
 *----------------------------------------------------------------*/
#define ADDER_NO_CHN_OPERATOR                  0x3faa1d8c

int oprTable[] = {
       /* Name                     Body Function            Stack size
          ----                     -------------            ---------- */
        ADDER_NO_CHN_OPERATOR,     (int)adder_no_chn,       32*1024,
       /* The last one is like this */
         0,                       (int)NULL,              0,
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
#define ADDER_NO_CHN_MAX       3
#define DATA_MAX              64
int createAdder_No_Chn (int *gix)
{
  int        i;
  int        mchn[ADDER_NO_CHN_MAX];
  int        prio[ADDER_NO_CHN_MAX];
  void      *param[ADDER_NO_CHN_MAX];
  int        paramSize[ADDER_NO_CHN_MAX];
  CommAttr   CommAttr;
  int        dataDim             = DATA_MAX;
  int        groupId             = GIX_ERROR;
  int        minPrio             = sched_get_priority_min(SCHED_FIFO);
  int        dummy               = 0;

  //fprintf(stdout, "\n\nCreateAdder:\n");
  prio[0] = minPrio;
  prio[1] = minPrio;
  prio[2] = minPrio;
  param[0]     = param[1]     = param[2]     = (void *)&dataDim;
  paramSize[0] = paramSize[1] = paramSize[2] = sizeof(int);

  for(i = 0; i < ADDER_NO_CHN_MAX; i++) {
    mchn[i] = getCpuId();
  }

  if(0 > GRP_create (&groupId, mchn, ADDER_NO_CHN_MAX, getCpuId()))             goto exception;
  for(i = 0; i < ADDER_NO_CHN_MAX; i++) {
    //CommAttr.Flags = 0;
    CommAttr.Flags = COMMATTR_FLAGS_SLM_ENABLED;

    if(0 > GRP_join(groupId, i, ADDER_NO_CHN_OPERATOR, prio[i],
                                         param[i], paramSize[i], &CommAttr))    goto exception;

  }
  if(0 > GRP_start(groupId))                                                    goto exception;
  *gix = groupId;
  fprintf(stdout, "- Adder Launched -\n");
  return(0);

exception:
  fprintf(stdout, ">>> Exception raised in Adder\n");
  GRP_kill(groupId);
  return(-1);
}



int AZQ_main(void) {

  int gix[32];
  int i, j, k;
  int retCode[ADDER_NO_CHN_MAX];

  for(j = 0; j < ADDER_NO_CHN_MAX; j++)
    retCode[j] = 55;

  /* 1. Init libraries */
  //if(0 > CHN_init())                                                            goto exception;

  /* 2. Only one processor launches applications */
  if(getCpuId() != 0)
    return(0);

  /* 3. Launch applications */
  for(i = 0; /*i < 100*/; i++) {
    //pthread_sleep(1);
    //fprintf(stdout, "\n\n````````````````````````%d..........................%%%%%%%%%%%%%%%%%%..\n", i);
    for(k = 0; k < 16; k++) {
      if (0 > createAdder_No_Chn(&gix[k]))                                           goto exception;
    }
    for(k = 0; k < 16; k++) {
      if (0 > GRP_wait(gix[k], retCode))                                            goto exception;
      fprintf(stdout, "\n");
      for(j = 0; j < ADDER_NO_CHN_MAX; j++) {
        fprintf(stdout, "AZQ_main: Ret[%d][%d] = %d\n", k, j, retCode[j]);
      }
    }
  }
  return(0);

exception:
  fprintf(stdout, "\n>>> Exception raised in AZQ_main\n");
  return(-1);
}


/*----------------------------------------------------------------*
 *   Main                                                         *
 *----------------------------------------------------------------*/
int main(int argc, char **argv) {

  /* 1. Init Azequia */
  if(0 > AZQ_init(oprTable)) {
    fprintf(stdout, "main: Cannot start AZEQUIA\n");
    return -1;
  }
  AZQ_wait();
  return 0;
}
