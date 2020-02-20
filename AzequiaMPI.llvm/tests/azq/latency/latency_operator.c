/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#ifdef __OSI
#include <osi.h>
#endif
#include <azq.h>

#include <stdio.h>
#include <malloc.h>
#include <time.h>

#include <routinginit.h>


/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
#define MSG_MAX   10000
#define MSG_SZ    128


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
int _latency(int *param)
{
  struct sched_param  schparam;
  int                 policy;
  int              excpn       = 0;
  int              i, j,
                   grpSize,
                   gix         = getGroup();
  int              myRank      = getRank();
  int             *buffer;
  int             *buffer2;
  Thr_t            me          = THR_self();
//static char     *where       = "Latency";
  int rank;

  //return(0);

  //fprintf(stdout, "\nLatency Operator: Rank %d\n", myRank);
  fprintf(stdout, "b **** \n");
  fprintf(stdout, "b **** latency(%x), (Gix %x, Rank %d) pth %x\n", (unsigned int)me, gix, myRank, (int)me->Self);
  if(pthread_getschedparam(pthread_self(), &policy, &schparam))                            panic("");
  fprintf(stdout, "latency: Policy %d. Priority %d\n", policy, schparam.sched_priority);

  GRP_getSize(gix, &grpSize);
  if(myRank == 0) {
    Addr             dst = {gix, 1};
    struct timespec delay = {0, 3000};
    //fprintf(stdout, "\n**************************************************************************************\n");
    //fprintf(stdout, "Latency_Source Operator(%x), (Gix %x, Rank %x)\n", (unsigned int)(THR_self()), gix, myRank);

    /* Get data memory */
    if(NULL == (buffer = (int *)malloc(MSG_SZ * sizeof(int))))                 {excpn = -5;
                                                                                goto exception_sender;}
    buffer[0] = 5;
    for(i = 0; i < MSG_MAX; i++) {
      //fprintf(stdout, "*\n*\n*\t\tLATENCY Envío %d...\n", i);
      if (!(i % 1000)) fprintf(stdout, "\tLATENCY Envío %d...\n", i);
      //*((int *)buffer) = i;

      //for (j = 0; j < MSG_SZ; j++)
      //  buffer[j] = i;
      buffer[0] = i;

      for (rank = 1; rank < grpSize; rank++) {
        dst.Rank = rank;
        buffer[1] = rank;
        if(0 > (excpn = send(&dst, (char *)buffer, MSG_SZ * sizeof(int), COM_FOREVER)))              goto exception_sender;
      }
      //recv (&dst, buffer, 4, COM_FOREVER, NULL);
      //usleep(20001);
      //nanosleep(&delay, NULL);
      //sleep(1);
      //send(&dst, buffer, 4, COM_FOREVER);
      //fprintf(stdout, "LATENCY enviado\n");
      //sched_yield();
    }
    free(buffer);
    return(0);
  }
  else {
    long long        now_1;
    long long        now_0;
    Addr             src = {gix, 0};

    //fprintf(stdout, "\n**************************************************************************************\n");
    fprintf(stdout, "Latency_Receiver Operator(%x), (Gix %x, Rank %x)\n", (unsigned int)(THR_self()), gix, myRank);

    /* Get data memory */
    if(NULL == (buffer2 = (char *)malloc(MSG_SZ * sizeof(int))))               {excpn = -5;
                                                                                goto exception_receiver;}
    now_0 = COM_now2();
    for(i = 0; i < MSG_MAX; i++) {
      //if(0 > (excpn = recv (&src, buffer2, 4, COM_FOREVER, NULL)))                        goto exception_receiver;
      //fprintf(stdout, "#\n#\n#\t\tLATENCY Recibo %d...\n", i);
      recv (&src, (char *)buffer2, MSG_SZ * sizeof(int), COM_FOREVER, NULL);
      //if (!(i % 10)) fprintf(stdout, "========================LATENCY Recibo %d...\n", i);
      if ((buffer2[0] != i) || (buffer2[1] != myRank))  fprintf(stdout, "****** ERROR.  Num %d  Rank %d\n", buffer2[0], buffer2[1]);
      //if (buffer2[0] != i)   goto exception_receiver;
      //if(0 > (excpn = send(&src, buffer2, 4, COM_FOREVER)))                                goto exception_sender;
      //fprintf(stdout, "========================LATENCY Recibido %d \n", *((int *)(buffer2)));
    }
    now_1 = COM_now2();
    //fprintf(stdout, "%d\n", (int)(buffer[0]));
    free(buffer2);
    fprintf(stdout, "\nmilliseconds %lld\n", (now_1 - now_0));
    fprintf(stdout, "%f us/msg \n", 1000*(((float)(now_1 - now_0)) / (float)MSG_MAX)  );
    return(0);
  }

exception_sender:
  fprintf(stdout, "::::::::::::::::: Latency Operator (Sender): Exception %d ::::::::::::::::\n", excpn);
  free(buffer);
  return(excpn);

exception_receiver:
  fprintf(stdout, "::::::::::::::::: Latency Operator (Receiver): Exception %d ::::::::::::::::\n", excpn);
  free(buffer2);
  return(excpn);
}


#define MIN_SIZE             0
#define MAX_SIZE    (1024 * 16)
#define MSG_INCR           128
#define MESSAGES        100000


int latency(int *param) {

  int     i, j;
  Addr    dst, src;
  int     myid;
  int    *window;
  Rqst   *rqst;
  Status  status;
  long long t0, t1;
  char   *bufsend, *bufrecv;



  myid = getRank();

  if (myid == 0) {
    fprintf(stdout, "Sender Operator (Rank %x, Gix %x)", getRank(), getGroup());

    if(NULL == (bufsend = (char *)malloc(MAX_SIZE * sizeof(char)))) goto exception;

    dst.Group = getGroup();
    dst.Rank  = 1;

    for (j = MIN_SIZE; j <= MAX_SIZE; j+=MSG_INCR) {
      init_send (&dst, (char *)bufsend, j, 99, &rqst);
      for (i = 0; i < MESSAGES; i++) {
	      //send(&dst, (char *)bufsend, j, 99);
	      //asend(&dst, (char *)bufsend, j, 99, &rqst);
	      start_send(rqst);
	      wait(&rqst, &status);
	      //if (!(i % 10000)) fprintf(stdout, "SENDER: %d messages of size %d\n", i, j);
      }
      free_request(&rqst);
  	}
	  
  } else { /* myid == 1 */
    fprintf(stdout, "Receiver Operator  (Rank %x, Gix %x)", getRank(), getGroup());

    if(NULL == (bufrecv = (char *)malloc(MAX_SIZE * sizeof(char)))) goto exception;

    src.Group = getGroup();
    src.Rank  = 0;

    for (j = MIN_SIZE; j <= MAX_SIZE; j += MSG_INCR) {
      t0 = COM_now2();
	    //init_recv(&src, (char *)bufrecv, j, 99, &rqst);
      for (i = 0; i < MESSAGES; i++) {
        recv(&src, (char *)bufrecv, j, 99, &status);
        //arecv(&src, (char *)bufrecv, j, 99, &rqst);
	      //start_recv(rqst);
	      //wait(&rqst, &status);
	      //if (!(i % 10000)) fprintf(stdout, "RECEIVER: %d messages of size %d\n", i, j);
	    }
	    //free_request(rqst);
      t1 = COM_now2();
	    fprintf(stdout, "%u\n", t1-t0);
    }
  }

  return;

exception:
  fprintf(stdout, "Sender exits with errors!!");
  return;
}
