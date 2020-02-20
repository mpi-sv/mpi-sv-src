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
#define  MOD          100

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
  int     *buf;
  double   t_start, t_end;
  Rqst     rqst [MAX_RQST];
  Rqst_t   all_rqst[MAX_RQST];
  Status   stats[MAX_RQST];
  int      i, j;


  myid = getRank();
  GRP_getSize(getGroup(), &numprocs);

  fprintf(stdout, "Process %d of %d starting ... (self: 0x%x)\n", myid, numprocs, (int)THR_self());fflush(NULL);

  usleep(10000);

  t_start = abstime();

  buf = (int *)malloc(SIZE * sizeof(int) * MAX_RQST);

  for (i = 0; i < NUM_MSGS; i++) {

    if (myid % 2) { /* Receptor */

      src.Group = getGroup();
      src.Rank  = myid - 1;

      for (j = 0; j < MAX_RQST; j++) {
        all_rqst[j] = &rqst[j];
        //fprintf(stdout, "ARECV %d ...\n", j);
        arecv(&src, (char *)buf + (j * SIZE * sizeof(int)), SIZE * sizeof(int), 99 - j, all_rqst[j]);
      }

      //fprintf(stdout, "WAITALL ...\n");
      waitall(all_rqst, MAX_RQST, stats);

      for (j = 0; j < MAX_RQST; j++) {
        if ((int *)((char *)buf + (j * SIZE * sizeof(int)))[0] != stats[j].Tag) {
          printf("ERROR\n");
          exit(1);
        }
      }

      if (!(i % MOD)) fprintf(stdout, "[%d] Recibido msg %d \n", getRank(), i);
      if (!(i % 100)) usleep(1000);
      //fflush(NULL);

    } else { /* Emisores */

      dst.Group = getGroup();
      dst.Rank  = myid + 1;

      //usleep(1000);
      //fprintf(stdout, "ASEND %d ...\n", 0);
      buf[0] = 94;
      send(&dst, (char *)buf, SIZE * sizeof(int), 94);
      //fprintf(stdout, "ASEND %d ...\n", 1);
      buf[0] = 95;
      send(&dst, (char *)buf, SIZE * sizeof(int), 95);
      buf[0] = 93;
      send(&dst, (char *)buf, SIZE * sizeof(int), 93);
      buf[0] = 96;
      send(&dst, (char *)buf, SIZE * sizeof(int), 96);
      buf[0] = 92;
      send(&dst, (char *)buf, SIZE * sizeof(int), 92);
      buf[0] = 97;
      send(&dst, (char *)buf, SIZE * sizeof(int), 97);
      buf[0] = 91;
      send(&dst, (char *)buf, SIZE * sizeof(int), 91);
      buf[0] = 98;
      send(&dst, (char *)buf, SIZE * sizeof(int), 98);
      buf[0] = 90;
      send(&dst, (char *)buf, SIZE * sizeof(int), 90);
      buf[0] = 99;
      send(&dst, (char *)buf, SIZE * sizeof(int), 99);

      if (!(i % MOD)) fprintf(stdout, "[%d] Enviado msg %d \n", getRank(), i);
      sched_yield();

    }
  }

  free(buf);

  t_end = abstime();
  if (myid == 0) { fprintf(stdout, "TIME: %lf\n" , t_end - t_start);}

  fprintf(stdout, "I am the %d rank of %d processes finalizing ...\n", myid, numprocs);
  fflush(stdout);

  return(0);
}
