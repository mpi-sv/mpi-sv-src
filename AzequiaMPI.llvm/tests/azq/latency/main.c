#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
#endif

#include <unistd.h>

#include <azq.h>


#define LATENCY_OPERATOR                  0x3e2a7d85
extern int latency (int *param);

int oprTable[] = {
       /* Name                     Body Function            Stack size
          ----                     -------------            ---------- */
        LATENCY_OPERATOR,          (int)latency,            64*1024
};

#define OPERATOR_MAX 16


int createApp (int *gix) {

  int      i;
  int      mchn[OPERATOR_MAX];
  int      prio[OPERATOR_MAX];
  int      groupId             = GIX_NONE;
  int      minPrio             = sched_get_priority_min(SCHED_FIFO);
  int      dummy               = 0;
  CommAttr CommAttr;
  void    *param    [OPERATOR_MAX];
  int      paramSize[OPERATOR_MAX];


  for(i = 0; i < OPERATOR_MAX; i++) {
    mchn     [i] = 0;
  }

  if(0 > GRP_create (&groupId, mchn, OPERATOR_MAX, getCpuId()))                goto exception_0;
  //fprintf(stdout, "User group created  0x%x\n", groupId);
  for(i = 0; i < OPERATOR_MAX; i++) {
    CommAttr.Flags = 0;
    //CommAttr.Flags = COMMATTR_FLAGS_SLM_ENABLED;
    //fprintf(stdout, "Thread number %d joining group 0x%x\n", i, groupId);
    if(0 > GRP_join(groupId, i, LATENCY_OPERATOR, prio[i],
                             NULL,
                             0, &CommAttr))                                    goto exception;
  }
  //fprintf(stdout, "Starting group 0x%x\n", groupId);
  if(0 > GRP_start(groupId))                                                   goto exception;
  *gix = groupId;
  return(0);

exception_0:
  fprintf(stdout, ">>> Exception raised in createLatency\n");
  return(-1);
exception:
  fprintf(stdout, ">>> Exception raised in createLatency\n");
  GRP_kill(groupId);
  return(-1);
}


int AZQ_main(void) {

  int gix;
  int stat;

  OPR_register(&oprTable, 1);

  /* 2. Only one processor launch applications */
  if(getCpuId() != 0)
    return 0;

  /* 3. Launch applications */
  if (0 > createApp(&gix))
    fprintf(stdout, "ERROR: Application CANNOT be created. Gix: 0x%x\n", gix);


  fprintf(stdout, "AZQ_main. Application finished.\n");

  return 0;

exception:
  fprintf(stdout, ">>> Exception raised in AZQ_main\n");
  return(-1);
}


int main(int argc, char **argv) {

  int realtime = 0;

  /* Init Azequia */
  if(0 > AZQ_init(&realtime, argc, argv)) {
    fprintf(stdout, "main: Cannot start AZEQUIA\n");
    return(-1);
  }

  /* Wait for AZQ_main termination */
  AZQ_wait();
  fprintf(stdout, "Bye, cruel world (%d)\n", getCpuId());

  return EXIT_SUCCESS;
}
