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
#define  MAX_ITER     1000000
#define  MOD          10000

/*----------------------------------------------------------------*
 *   Definition of private functions                              *
 *----------------------------------------------------------------*/
double abstime (void) {

  struct timespec t;

  clock_gettime(CLOCK_REALTIME, &t);

  return ((double) t.tv_sec + 1.0e-9 * (double) t.tv_nsec);
}



void thr_fxn(void *arg) {

  int     rank;
  Addr    src;
  int     buf;
  int     i;
  Status  status;

  GRP_enroll2(arg);
  rank = getRank();
  fprintf(stdout, "[CHILD] Hello, I am the %d rank. 0x%x of 0x%x\n", rank, THR_self(), (int)arg);

  //THR_showctx(THR_self());

  for (i = 0; i < MAX_ITER; i++) {
    src.Group = getGroup();
    src.Rank  = rank - 1;
    
    recv(&src, (char *)&buf, sizeof(int), 97, &status);
    //fprintf(stdout, "[CHILD %x] Received message %d from %d with value %d\n", THR_self(), i, status.Src.Rank, buf);
    if (i != buf) fprintf(stdout, "[CHILD %x] ERROR: Received message %d from %d with value %d\n", THR_self(), i, status.Src.Rank, buf);
    //sleep(1);
    if (!(i % MOD)) fprintf(stdout, "[CHILD %x] received %d messages\n", THR_self(), i);
  }

}


void thr_fxn2(void *arg) {

  int     rank;
  Addr    src;
  int     buf;
  int     i;
  Status  status;

  GRP_enroll2(arg);
  rank = getRank();
  fprintf(stdout, "[CHILD] Hello, I am the %d rank. 0x%x of 0x%x\n", rank, THR_self(), (int)arg);

  //THR_showctx(THR_self());

  for (i = 0; i < MAX_ITER; i++) {
    src.Group = getGroup();
    src.Rank  = rank - 1;

    recv(&src, (char *)&buf, sizeof(int), 99, &status);
    //fprintf(stdout, "[CHILD %x] Received message %d from %d with value %d\n", THR_self(), i, status.Src.Rank, buf);
    if (i != buf) fprintf(stdout, "[CHILD %x] ERROR: Received message %d from %d with value %d\n", THR_self(), i, status.Src.Rank, buf);
    //sleep(1);
    if (!(i % MOD)) fprintf(stdout, "[CHILD %x] received %d messages\n", THR_self(), i);
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
  Addr       dst;
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
    dst.Group = getGroup();
    dst.Rank  = myid + 1;
    for (i = 0; i < MAX_ITER; i++) {
      buf = i;
      send(&dst, (char *)&buf, sizeof(int), 99);
      fprintf(stdout, "[Rank %d] Sended: %d\n", myid, buf);
      send(&dst, (char *)&buf, sizeof(int), 97);
      fprintf(stdout, "[Rank %d] Sended: %d\n", myid, buf);
      //sched_yield();
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
