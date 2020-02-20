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
extern int slowPi             (int *param);

/*----------------------------------------------------------------*
 *   Operators to run                                             *
 *----------------------------------------------------------------*/
#define SLOW_PI_OPERATOR             0xf1827df5

int oprTable[] = {
       /* Name                     Body Function            Stack size
          ----                     -------------            ---------- */
        SLOW_PI_OPERATOR,          (int)slowPi,             32*1024,

       /* The last one is like this */
         0,                       (int)NULL,              0,
};


/*----------------------------------------------------------------*
 *   Creating an Azequia application                              *
 *----------------------------------------------------------------*/


      /*________________________________________________________________
     /                                                                  \
    |    createPi                                                        |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
#define PI_MAX         16
#define PRECISION     (4096)
#define PULSOS        1
int createPi (int *gix)
{
  int   groupId = GIX_ERROR;
  int   i;
  int   mchn[PI_MAX];
  int   prio          = sched_get_priority_min(SCHED_FIFO);
  int   paramSize;
  int   precision;
  CommAttr CommAttr;

  fprintf(stdout, "createPi:\n");
  precision = PRECISION;
  paramSize = sizeof(int);

  for(i = 0; i < PI_MAX; i++) {
    mchn[i]      = getCpuId();
  }

  if(0 > GRP_create (&groupId, mchn, PI_MAX, getCpuId()))                       goto exception;
  for(i = 0; i < PI_MAX; i++) {
    CommAttr.Flags = COMMATTR_FLAGS_SLM_ENABLED;
    if(0 > GRP_join(groupId, i, SLOW_PI_OPERATOR, (i == 0 ? prio + 1 : prio),
                             (void *)&precision,
                             paramSize, &CommAttr))                             goto exception;
  }
  if(0 > GRP_start(groupId))                                                    goto exception;
  *gix = groupId;
  fprintf(stdout, "- createPi Launched -\n");
  return(0);

exception:
  fprintf(stdout, ">>> Exception raised in createPi\n");
  GRP_kill(groupId);
  return(-1);
}



int AZQ_main(void) {

  int gix;
  int j;
  int retCode[PI_MAX];


  /* 1. Only one processor launches applications */
  if(getCpuId() != 0)
    return(0);

  /* 2. Launch applications */
  if (0 > createPi(&gix))                                                      goto exception;
  if (0 > GRP_wait(gix, retCode))                                              goto exception;
  fprintf(stdout, "\n");
  for(j = 0; j < PI_MAX; j++) {
    if(retCode[j])
      fprintf(stdout, "AZQ_main: Ret[%d] = %d\n", j, retCode[j]);
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
