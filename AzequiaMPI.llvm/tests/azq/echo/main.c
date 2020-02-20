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
//#include <net.h>
//#include <limits.h>



/*----------------------------------------------------------------*
 *   Operator's main in another file                              *
 *----------------------------------------------------------------*/
extern int echo_server  (int *param);
extern int echo_client  (int *param);


/*----------------------------------------------------------------*
 *   Operators to run                                             *
 *----------------------------------------------------------------*/
#define ECHO_SERVER_OPERATOR                  0xaf2a92c7
#define ECHO_CLIENT_OPERATOR                  0x3f2c92d6

int oprTable[] = {
       /* Name                     Body Function            Stack size
          ----                     -------------            ---------- */
        ECHO_SERVER_OPERATOR,   (int)echo_server,          (32*1024),
        ECHO_CLIENT_OPERATOR,   (int)echo_client,          (32*1024),
       /* The last one is like this */
         0,                       (int)NULL,                0,
};


/*----------------------------------------------------------------*
 *   Creating an Azequia application                              *
 *----------------------------------------------------------------*/

      /*________________________________________________________________
     /                                                                  \
    |    echo_server                                                     |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
#define PARALLEL_GRP_NR        1
#define ITERATION_NR           1

#define ECHO_SERVER_MAX         1 //  (SENDERS + RECEIVERS + 1)
#define DATA_MAX              64
int createEchoServer(int *gix)
{
  int        i;
  int        mchn     [ECHO_SERVER_MAX];
  int        prio     [ECHO_SERVER_MAX];
  void      *param    [ECHO_SERVER_MAX];
  int        paramSize[ECHO_SERVER_MAX];
  CommAttr   CommAttr;

  int        paramObj[1]         = {DATA_MAX};
  int        groupId             = GIX_NONE;
  int        minPrio             = sched_get_priority_min(SCHED_FIFO);

  /*fprintf(stdout, "\n\ncreateEchoServer:\n");*/

  for(i = 0; i < ECHO_SERVER_MAX; i++)
    prio     [i] = minPrio + i;
  //prio[ECHO_SERVER_MAX - 1] = minPrio + SENDERS/2;


  for(i = 0; i < ECHO_SERVER_MAX; i++) {
    param    [i] = (void *)paramObj;
    paramSize[i] = 1 * sizeof(int);
  //mchn     [i] = getCpuId();
    mchn     [i] = i;
  }

  if(0 > GRP_create (&groupId, mchn, ECHO_SERVER_MAX, getCpuId()))                 goto exception2;
  for(i = 0; i < ECHO_SERVER_MAX; i++) {
    CommAttr.Flags = 0;
    //CommAttr.Flags = COMMATTR_FLAGS_SLM_ENABLED;

    if(0 > GRP_join(groupId, i, ECHO_SERVER_OPERATOR, prio[i],
                                         param[i], paramSize[i], &CommAttr))    goto exception;

  }
  if(0 > GRP_start(groupId))                                                    goto exception;
  *gix = groupId;
  //fprintf(stdout, "- Echo server Launched -\n");
  return(0);

exception:
  fprintf(stdout, ">>> Exception raised in createEchoServer\n");
  GRP_kill(groupId);
  return(-1);

exception2:
  fprintf(stdout, ">>> Exception raised in createEchoServer\n");
  return(-1);
}

#define ECHO_CLIENT_MAX         1
int createEchoClient(int *gix)
{
  int        i;
  int        mchn     [ECHO_CLIENT_MAX];
  int        prio     [ECHO_CLIENT_MAX];
  void      *param    [ECHO_CLIENT_MAX];
  int        paramSize[ECHO_CLIENT_MAX];
  CommAttr   CommAttr;

  int        paramObj[1]         = {DATA_MAX};
  int        groupId             = GIX_NONE;
  int        minPrio             = sched_get_priority_min(SCHED_FIFO);

  for(i = 0; i < ECHO_CLIENT_MAX; i++)
    prio     [i] = minPrio + i;

  for(i = 0; i < ECHO_CLIENT_MAX; i++) {
    param    [i] = (void *)paramObj;
    paramSize[i] = 1 * sizeof(int);
    mchn     [i] = getCpuId();
  //mchn     [i] = i;
  }

  if(0 > GRP_create (&groupId, mchn, ECHO_CLIENT_MAX, getCpuId()))              goto exception2;
  for(i = 0; i < ECHO_CLIENT_MAX; i++) {
    CommAttr.Flags = 0;
    //CommAttr.Flags = COMMATTR_FLAGS_SLM_ENABLED;

    if(0 > GRP_join(groupId, i, ECHO_CLIENT_OPERATOR, prio[i],
                                         param[i], paramSize[i], &CommAttr))    goto exception;

  }
  if(0 > GRP_start(groupId))                                                    goto exception;
  *gix = groupId;
  fprintf(stdout, "- Echo client Launched -\n");
  return(0);

exception:
  fprintf(stdout, ">>> Exception raised in createEchoClient\n");
  GRP_kill(groupId);
  return(-1);

exception2:
  fprintf(stdout, ">>> Exception raised in createEchoClient\n");
  return(-1);
}



int AZQ_main(void) 
{
  int gix[4];
  int i, j, k;
  int retCode[ECHO_SERVER_MAX];

  fprintf(stdout, "AZQ_main:\n");
  if(getCpuId() != 0) {
    fprintf(stdout, "CPU %d. I do not lauch anything\n", getCpuId());
    return(0);
  }

  /* 3. Launch applications */
  if (0 > createEchoServer(&gix[0]))                                            goto exception;
  if (0 > createEchoClient(&gix[1]))                                            goto exception;
  if (0 > GRP_wait(gix[1], retCode))                                            goto exception;
  GRP_kill(gix[0]);
  if (0 > GRP_wait(gix[0], retCode))                                            goto exception;


  fprintf(stdout, "AZQ_main: End\n");
  return(0);

exception:
  fprintf(stdout, ">>> Exception raised in AZQ_main\n");
  return(-1);
}


/*----------------------------------------------------------------*
 *   Main                                                         *
 *----------------------------------------------------------------*/
int main(int argc, char **argv) 
{
  fprintf(stdout, "Echo. Example of an User Service\n");
  /* Init Azequia */
  if(0 > AZQ_init((void *)&oprTable, argc, argv)) {
    fprintf(stdout, "main: Cannot start AZEQUIA\n");
    return(-1);
  }

  /* Wait for AZQ_main termination */
  AZQ_wait();
  return(0);
}

