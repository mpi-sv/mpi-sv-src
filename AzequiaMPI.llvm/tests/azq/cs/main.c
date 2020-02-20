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
extern int cs                 (int *param);


/*----------------------------------------------------------------*
 *   Operators to run                                             *
 *----------------------------------------------------------------*/
#define CS_OPERATOR                  0xb2a0758f

int oprTable[] = {
       /* Name                     Body Function            Stack size
          ----                     -------------            ---------- */
        CS_OPERATOR,              (int)cs,                  16*1024,
       /* The last one is like this */
         0,                       (int)NULL,                0,
};


/*----------------------------------------------------------------*
 *   Creating an Azequia application                              *
 *----------------------------------------------------------------*/


      /*________________________________________________________________
     /                                                                  \
    |    createCs                                                   |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
#define CS_MAX       2
int createCs (int *gix)
{
  int   groupId             = GIX_ERROR;
  int   i;
  int   mchn[CS_MAX];
  int   prio[CS_MAX];
  int   minPrio             = sched_get_priority_min(SCHED_FIFO);
  int   paramSize           = sizeof(int);
  int   dummy               = 0;

  fprintf(stdout, "createCs:\n");
  prio[0] = minPrio + 1;
  prio[1] = minPrio + 1;

  for(i = 0; i < CS_MAX; i++) {
    mchn[i] = getCpuId();
  }

  if(0 > GRP_create (&groupId, mchn, CS_MAX, getCpuId()))                  goto exception;
  for(i = 0; i < CS_MAX; i++)
    if(0 > GRP_join(groupId, i, CS_OPERATOR, prio[i],
                             (void *)(&dummy),
                             paramSize, NULL))                                        goto exception;
  if(0 > GRP_start(groupId))                                                    goto exception;
  *gix = groupId;
  fprintf(stdout, "- CS Launched -\n");
  return(0);

exception:
  fprintf(stdout, ">>> Exception raised in CS\n");
  GRP_kill(groupId);
  return(-1);
}



int AZQ_main(void) {

  int gix[8];
  int i, j, k;
  int retCode[CS_MAX];

  for(j = 0; j < CS_MAX; j++)
    retCode[j] = 55;

  /* 1. Init libraries */
  //if(0 > CHN_init())                                                            goto exception;

  /* 2. Only one processor launches applications */
  if(getCpuId() != 0)
    return(0);

  /* 3. Launch applications */
  for(i = 0; i < 1; i++) {
    fprintf(stdout, "\n\n````````````````````````%d..........................%%%%%%%%%%%%%%%%%%..\n", i);
    for(k = 0; k < 1; k++) {
      if (0 > createCs(&gix[k]))                                           goto exception;
    }
    for(k = 0; k < 1; k++) {
      fprintf(stdout, "AZQ_main: Invoco GRP_wait...\n");
      if (0 > GRP_wait(gix[k], retCode))                                            goto exception;
      /*fprintf(stdout, "\n");
      for(j = 0; j < CS_MAX; j++) {
        fprintf(stdout, "AZQ_main: Ret[%d][%d] = %d\n", k, j, retCode[j]);
      }*/
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
