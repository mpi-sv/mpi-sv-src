/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#include <azq.h>

#ifdef __OSI
#include <osi.h>
#endif
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>


/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
//#define  SIZE         ((8192 * 2) * sizeof(int))
#define  SIZE         (20)
#define  NUM_MSGS     10000
#define  MOD          1

#define  MAX_RQST     10
/*----------------------------------------------------------------*
 *   Definition of private functions                              *
 *----------------------------------------------------------------*/
double abstime (void) {

  struct timespec t;

  clock_gettime(CLOCK_REALTIME, &t);

  return ((double) t.tv_sec + 1.0e-9 * (double) t.tv_nsec);
}


/*----------------------------------------------------------------*
 *   Definition of functions                                      *
 *----------------------------------------------------------------*/
int operator_main(int argc, char **argv) {

  int      myid;
  int      numprocs;
  Addr     src, dst;
  int      buf;
  double   t_start, t_end;
  Rqst     rqst;
  Rqst_t   all_rqst;
  Status   stats;
  Status   status;
  int      i, j;
  int      sbuf;


  myid = getRank();
  GRP_getSize(getGroup(), &numprocs);

  fprintf(stdout, "Process %d of %d starting ... (self: 0x%x)\n", myid, numprocs, (int)THR_self());fflush(NULL);

  usleep(10000);

  t_start = abstime();

  for (i = 0; i < NUM_MSGS; i++) {

    if (myid % 2) { /* Receptor */

      src.Group = getGroup();
      src.Rank  = myid - 1;

      fprintf(stdout, "--->> arecv %d \n", i);
      all_rqst = &rqst;
      arecv(&src, (char *)&buf, sizeof(int), 99, all_rqst);
      fprintf(stdout, "<<--- arecv: (msg %d)\n", i);
      wait(&all_rqst, &stats);
      fprintf(stdout, "<<--- wait_recv %d (buf: %d)\n", i, buf);
      if (buf != (i + 1000)) fprintf(stdout, "ERROR RECV: %d (buf: %d)\n", i, buf);
       
      fprintf(stdout, "--->> recv %d \n", i);
      recv(&src, (char *)&sbuf, sizeof(int), 100, &status);
      fprintf(stdout, "<<--- recv: (msg %d) %d  %d \n", i, sbuf, status.Tag);
      if (sbuf != i) fprintf(stdout, "ERROR RECV: %d (sbuf: %d)\n", i, sbuf);

      if (!(i % MOD)) {fprintf(stdout, "[%d] Recibido msg %d \n", getRank(), i); fflush(NULL); }
      if (!(i % 100)) usleep(1000);

    } else { /* Emisores */

      dst.Group = getGroup();
      dst.Rank  = myid + 1;

      buf = i + 1000;
      all_rqst = &rqst;
      fprintf(stdout, "--->> asend %d \n", i);
      asend(&dst, (char *)&buf, sizeof(int), 99, all_rqst);
      fprintf(stdout, "<<--- asend: (msg %d)\n", i);
      wait(&all_rqst, &stats);
      fprintf(stdout, "<<--- wait_send %d (buf: %d)\n", i, buf);

      if (!(i % MOD)) fprintf(stdout, "[%d] Enviado msg %d \n", getRank(), i);

      sbuf = i;
      fprintf(stdout, "--->> send %d \n", i);
      send(&dst, (char *)&sbuf, sizeof(int), 100); 
      fprintf(stdout, "<<--- send: (msg %d) %d  %d \n", i, sbuf, status.Tag);

    }
  }

  t_end = abstime();
  if (myid == 0) {fprintf(stdout, "TIME: %lf\n", t_end - t_start);}

  fprintf(stdout, "I am the %d rank of %d processes finalizing ...\n", myid, numprocs);
  fflush(stdout);

  return(0);
}
