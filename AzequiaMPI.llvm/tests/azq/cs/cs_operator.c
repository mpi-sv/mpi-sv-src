/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#include <stdio.h>
#include <azq.h>
#include <malloc.h>
#include <time.h>

/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
#define MSG_MAX   50000

/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/
unsigned long long COM_now2(void)
{
  struct timespec      now;
  unsigned long long   milliseconds;
  if(clock_gettime(CLOCK_REALTIME, &now))                                       goto exception;

  milliseconds = ((unsigned long long)now.tv_sec) * 1000 +
                 ((unsigned long long)now.tv_nsec) / 1000000;
//fprintf(stdout, "NOW: %ld milliseconds\n", milliseconds);
  return(milliseconds);

exception:
  fprintf(stdout, "COM_now2: error\n");
  return(-1);
}


     /*----------------------------------------------------------*\
    |    latency                                                   |
    |                                                              |
    |                                                              |
     \*----------------------------------------------------------*/
int cs(int *param)
{
  int              excpn       = 0, src, dst;
  int              i, size,
                   grpSize,
                   gix         = getGroup();
  int              myRank      = getRank();
  char            *buffer;
  char            *buffer2;
//static char     *where       = "ContextSwitch";

  fprintf(stdout, "\nContextSwitch Operator: Rank %d\n", myRank);
  GRP_getSize(gix, &grpSize);
  //sleep(1);
  //return(0);
  if(myRank == 0) {
    fprintf(stdout, "\n**************************************************************************************\n");
    fprintf(stdout, "ContextSwitch Sender Operator(%x), (Gix %x, Rank %x)\n", (unsigned int)(THR_self()), gix, myRank);
    //for(size = 1; size < 2; size++) {
      /* Get data memory */
      /*if(NULL == (buffer = (char *)malloc(size)))                               {excpn = -5;
                                                                                goto exception;}*/
      //buffer[0] = 5;
      for(i = 0; i < MSG_MAX; i++) {
        //fprintf(stdout, "\n\t\tContextSwitch Envío %d...\n", i+1);
        //*((int *)buffer) = i;
        //if(0 > (excpn = GC_send(buffer, size, 1, 0, 0)))                        goto exception;
        //if(0 > (excpn = GC_send(buffer, 4, 1, 0, 0)))                        goto exception;
        sched_yield();
        memcpy(&dst, &src, 4);
        //fprintf(stdout, "\t\t%d\n", i);
      }
      //free(buffer);
    //}
  }
  else {
    long long        now_1;
    long long        now_0;
    fprintf(stdout, "\n**************************************************************************************\n");
    fprintf(stdout, "ContextSwitch Receiver Operator(%x), (Gix %x, Rank %x)\n", (unsigned int)(THR_self()), gix, myRank);
    now_0 = COM_now2();
    for(i = 0; i < MSG_MAX; i++) {
      //fprintf(stdout, "%d \n", i);
      sched_yield();
    }
    fprintf(stdout, "Invoco COM_now2...\n");
    now_1 = COM_now2();
    fprintf(stdout, "\nmilliseconds %lld\n", (now_1 - now_0));
    //exit(0);
  }
  return(0);

exception:
  fprintf(stdout, "::::::::::::::::: ContextSwitch Operator: Exception %d ::::::::::::::::\n", excpn);
  return(excpn);
}
