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
#define  MAX_ITER     10000
#define  MOD          1000

/*----------------------------------------------------------------*
 *   Definition of private functions                              *
 *----------------------------------------------------------------*/
double abstime (void) {

  struct timespec t;

  clock_gettime(CLOCK_REALTIME, &t);

  return ((double) t.tv_sec + 1.0e-9 * (double) t.tv_nsec);
}



int thr_recv_1() {

  int     rank;
  Addr    src;
  int     buf;
  int     i;
  Status  status;
  Thr_t    thr       = NULL;
  
  //THR_create2(&thr, arg);
  //GRP_enroll2(arg);
  rank = getRank();
  fprintf(stdout, "[CHILD] Hello, I am the %d rank. 0x%x \n", rank, THR_self());

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

  return 0;
}


int thr_recv_2() {

  int     rank;
  Addr    src;
  int     buf;
  int     i;
  Status  status;
  Thr_t    thr       = NULL;
  
  //THR_create2(&thr, arg);
  //GRP_enroll2(arg);
  rank = getRank();
  fprintf(stdout, "[CHILD] Hello, I am the %d rank. 0x%x \n", rank, THR_self());

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

  return 0;
}


int p_recv() {

  int     rank;
  Addr    src;
  int     buf;
  int     i;
  Status  status;

  rank = getRank();
  fprintf(stdout, "[NODE] not - child. Hello, I am the %d rank. (0x%x) \n", rank, THR_self());

  for (i = 0; i < MAX_ITER; i++) {
    src.Group = getGroup();
    src.Rank  = rank - 1;
    
    recv(&src, (char *)&buf, sizeof(int), 95, &status);
    //fprintf(stdout, "[CHILD %x] Received message %d from %d with value %d\n", THR_self(), i, status.Src.Rank, buf);
    if (i != buf) fprintf(stdout, "[CHILD %x] ERROR: Received message %d from %d with value %d\n", THR_self(), i, status.Src.Rank, buf);
    //sleep(1);
    if (!(i % MOD)) fprintf(stdout, "[CHILD %x] received %d messages\n", THR_self(), i);
  }

  return 0;
}


int thr_send_1() {

  int   rank;
  Addr  dst;
  int   buf;
  int   i;
  Thr_t    thr       = NULL;
  
  //THR_create2(&thr, arg);
  //GRP_enroll2(arg);
  rank = getRank();
  fprintf(stdout, "[CHILD] Hello, I am the %d rank\n", rank);

  //THR_showctx(THR_self());

  for (i = 0; i < MAX_ITER; i++) {
    dst.Group = getGroup();
    dst.Rank  = rank + 1;
    buf       = i;
    if (!(i % MOD)) fprintf(stdout, "[CHILD] Sending message %d from 0x%x\n", i, THR_self());
    send(&dst, (char *)&buf, sizeof(int), 99);
    //sleep(1);
  }

  return 0;
}

int thr_send_2() {

  int   rank;
  Addr  dst;
  int   buf;
  int   i;
  Thr_t    thr       = NULL;
  
  //THR_create2(&thr, arg);
  //GRP_enroll2(arg);
  rank = getRank();
  fprintf(stdout, "[CHILD] Hello, I am the %d rank\n", rank);

  //THR_showctx(THR_self());

  for (i = 0; i < MAX_ITER; i++) {
    dst.Group = getGroup();
    dst.Rank  = rank + 1;
    buf       = i;
    if (!(i % MOD)) fprintf(stdout, "[CHILD] Sending message %d from 0x%x\n", i, THR_self());
    send(&dst, (char *)&buf, sizeof(int), 97);
    //sleep(1);
  }

  return 0;
}

int p_send() {

  int   rank;
  Addr  dst;
  int   buf;
  int   i;

  rank = getRank();
  fprintf(stdout, "[NODE] not - CHILD Hello, I am the %d rank\n", rank);

  for (i = 0; i < MAX_ITER; i++) {
    dst.Group = getGroup();
    dst.Rank  = rank + 1;
    buf       = i;
    if (!(i % MOD)) fprintf(stdout, "[CHILD] Sending message %d from 0x%x\n", i, THR_self());
    send(&dst, (char *)&buf, sizeof(int), 95);
    //sleep(1);
  }

  return 0;
}

/*----------------------------------------------------------------*
 *   Definition of functions                                      *
 *----------------------------------------------------------------*/
int operator_main(int argc, char **argv) {

  int        myid;
  int        numprocs;
  double     t_start, t_end;
  Thr_t      thr1, thr2;
  int        buf;
  Addr       dst;
  Status     status;
  int        i;
  int        st1, st2;


  myid = getRank();
  GRP_getSize(getGroup(), &numprocs);

  fprintf(stdout, "Process %d of %d starting ... (self: 0x%x)\n", myid, numprocs, (int)THR_self());

  t_start = abstime();

  if (myid % 2) {
    THR_spawn(&thr1, (void *)thr_recv_1);
    THR_spawn(&thr2, (void *)thr_recv_2);
  } else {
    THR_spawn(&thr1, (void *)thr_send_1);
    THR_spawn(&thr2, (void *)thr_send_2);
  }

  if (myid % 2) {
    p_recv();
  } else {
    p_send();
  }   

  if(myid  % 2) {
    THR_wait(thr1, &st1);
    THR_wait(thr2, &st2); 
  } else {
    THR_wait(thr1, &st1);
    THR_wait(thr2, &st2); 
  }

  t_end = abstime();
  if (myid == 0) fprintf(stdout, "%lf\n" , t_end - t_start);

  fprintf(stdout, "I am the %d rank of %d processes finalizing ...\n", myid, numprocs);
  fflush(stdout);

  return(0);
}
