/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#include <azq.h>

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>


/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
//#define  SIZE         ((8192 * 1) * sizeof(int))
#define  SIZE         (20 * sizeof(int))
#define  NUM_MSGS     1000000
#define  MOD          1000

#define  MAX_ITER     10000

/*----------------------------------------------------------------*
 *   Definition of private functions                              *
 *----------------------------------------------------------------*/
double abstime (void) {

  struct timespec t;

  clock_gettime(CLOCK_REALTIME, &t);

  return ((double) t.tv_sec + 1.0e-9 * (double) t.tv_nsec);
}



void thr_fxn(void *arg) {

  int   rank;
  Addr  dst;
  int   buf;
  int   i;

  GRP_enroll2(arg);
  rank = getRank();
  fprintf(stdout, "[CHILD] Hello, I am the %d rank\n", rank);

  //THR_showctx(THR_self());

  for (i = 0; i < MAX_ITER; i++) {
    dst.Group = getGroup();
    dst.Rank  = rank - 1;
    buf       = rank + i; 
    fprintf(stdout, "[CHILD] Sending message %d from 0x%x\n", i, THR_self());
    send(&dst, (char *)&buf, sizeof(int), 97);
    //sleep(1);
  }
}


void thr_fxn2(void *arg) {

  int   rank;
  Addr  dst;
  int   buf;
  int   i;

  GRP_enroll2(arg);
  rank = getRank();
  fprintf(stdout, "[CHILD] Hello, I am the %d rank\n", rank);

  //THR_showctx(THR_self());

  for (i = 0; i < MAX_ITER; i++) {
    dst.Group = getGroup();
    dst.Rank  = rank - 1;
    buf       = rank + i;
    fprintf(stdout, "[CHILD] Sending message %d from 0x%x\n", i, THR_self());
    send(&dst, (char *)&buf, sizeof(int), 99);
    //sleep(1);
  }
}

/*----------------------------------------------------------------*
 *   Definition of functions                                      *
 *----------------------------------------------------------------*/
int operator_main(int argc, char **argv) {

  int        myid;
  int        numprocs;
  double     t_start, t_end;
  pthread_t  thr1, thr2;
  int        buf;
  Addr       src;
  Status     status;
  int        i;


  myid = getRank();
  GRP_getSize(getGroup(), &numprocs);

  fprintf(stdout, "Process %d of %d starting ... (self: 0x%x)\n", myid, numprocs, (int)THR_self());

  t_start = abstime();

  //THR_showctx(THR_self());

  if (myid % 2) {
    pthread_create(&thr1, NULL, (void *)thr_fxn, (void *)THR_self());
    pthread_create(&thr2, NULL, (void *)thr_fxn2, (void *)THR_self());
  }

  else {
    src.Group = getGroup();
    src.Rank  = myid + 1;
    for (i = 0; i < MAX_ITER; i++) {
      recv(&src, (char *)&buf, sizeof(int), 99, &status);
      fprintf(stdout, "[Rank %d] Received: %d\n", myid, buf);
      if (buf != (status.Src.Rank + i)) fprintf(stdout, "ERROR: rank %d received %d (expected %d)\n", myid, buf, status.Src.Rank + i);
      recv(&src, (char *)&buf, sizeof(int), 97, &status);
      fprintf(stdout, "[Rank %d] Received: %d\n", myid, buf);
      if (buf != (status.Src.Rank + i)) fprintf(stdout, "ERROR: rank %d received %d (expected %d)\n", myid, buf, status.Src.Rank + i);
    }
  }

  if(myid  % 2) {
    pthread_join(thr1, NULL);
    pthread_join(thr2, NULL); 
  }

  t_end = abstime();
  if (myid == 0) fprintf(stdout, "%lf\n" , t_end - t_start);

  fprintf(stdout, "I am the %d rank of %d processes finalizing ...\n", myid, numprocs);
  fflush(stdout);

  return(0);
}
