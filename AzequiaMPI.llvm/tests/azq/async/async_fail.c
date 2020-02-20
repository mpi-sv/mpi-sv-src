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
  int     *buf;
  double   t_start, t_end;
  Rqst     rqst [MAX_RQST];
  Rqst_t   all_rqst[MAX_RQST];
  Status   stats[MAX_RQST];
  Status   status;
  int      i, j;
  int      sbuf;


  myid = getRank();
  GRP_getSize(getGroup(), &numprocs);

  fprintf(stdout, "Process %d of %d starting ... (self: 0x%x)\n", myid, numprocs, (int)THR_self());fflush(NULL);

  usleep(10000);

  t_start = abstime();

  buf = (int *)malloc(SIZE * sizeof(int) * (MAX_RQST + 1));

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
       
      /*
      recv(&src, (char *)buf + (10 * SIZE * sizeof(int)), SIZE * sizeof(int), 100, &status);
      if (((int *)((char *)buf + (10 * SIZE * sizeof(int))))[0] != status.Tag) {
          fprintf(stdout, "ERROR (SYNCHRONOUS): (msg %d) %d  %d \n", i, ((int *)((char *)buf + (10 * SIZE * sizeof(int))))[0], status.Tag);
          fflush(NULL);
        }
      */
      fprintf(stdout, "rrrr %d \n", i);
      recv(&src, (char *)&sbuf, sizeof(int), 100, &status);
      fprintf(stdout, "RECV - SYNCHRONOUS: (msg %d) %d  %d \n", i, sbuf, status.Tag);

      //for (j = 0; j < MAX_RQST; j++)
      //  recv(&src, (char *)buf + (j * SIZE * sizeof(int)), SIZE * sizeof(int), 99 - j, &stats[j]);

      for (j = 0; j < MAX_RQST; j++) {
        if (((int *)((char *)buf + (j * SIZE * sizeof(int))))[0] != stats[j].Tag) {
          fprintf(stdout, "ERROR: (msg %d) %d  %d \n", i, ((int *)((char *)buf + (j * SIZE * sizeof(int))))[0], stats[j].Tag);
          fflush(NULL);
          //exit(1);
        }
      }

      if (!(i % MOD)) {fprintf(stdout, "[%d] Recibido msg %d \n", getRank(), i); fflush(NULL); }
      if (!(i % 100)) usleep(1000);
      //fflush(NULL);

    } else { /* Emisores */

      dst.Group = getGroup();
      dst.Rank  = myid + 1;

      //usleep(10000);
      //fprintf(stdout, "ASEND %d ...\n", 0);
      all_rqst[0] = &rqst[0];
      //buf[0] = 94;
      ((int *)((char *)buf + (0 * SIZE * sizeof(int))))[0] = 94;
      asend(&dst, ((char *)buf + (0 * SIZE * sizeof(int))), SIZE * sizeof(int), 94, all_rqst[0]);
      //fprintf(stdout, "ASEND %d ...\n", 1);

      all_rqst[1] = &rqst[1];
      //buf[0] = 95;
      ((int *)((char *)buf + (1 * SIZE * sizeof(int))))[0] = 95;
      asend(&dst, ((char *)buf + (1 * SIZE * sizeof(int))), SIZE * sizeof(int), 95, all_rqst[1]);

      all_rqst[2] = &rqst[2];
      //buf[0] = 93;
      ((int *)((char *)buf + (2 * SIZE * sizeof(int))))[0] = 93;
      asend(&dst, ((char *)buf + (2 * SIZE * sizeof(int))), SIZE * sizeof(int), 93, all_rqst[2]);

      all_rqst[3] = &rqst[3];
      //buf[0] = 96;
      ((int *)((char *)buf + (3 * SIZE * sizeof(int))))[0] = 96;
      asend(&dst, ((char *)buf + (3 * SIZE * sizeof(int))), SIZE * sizeof(int), 96, all_rqst[3]);

      all_rqst[4] = &rqst[4];
      //buf[0] = 92;
      ((int *)((char *)buf + (4 * SIZE * sizeof(int))))[0] = 92;
      asend(&dst, ((char *)buf + (4 * SIZE * sizeof(int))), SIZE * sizeof(int), 92, all_rqst[4]);

      all_rqst[5] = &rqst[5];
      //buf[0] = 97;
      ((int *)((char *)buf + (5 * SIZE * sizeof(int))))[0] = 97;
      asend(&dst, ((char *)buf + (5 * SIZE * sizeof(int))), SIZE * sizeof(int), 97, all_rqst[5]);

      all_rqst[6] = &rqst[6];
      //buf[0] = 91;
      ((int *)((char *)buf + (6 * SIZE * sizeof(int))))[0] = 91;
      asend(&dst, ((char *)buf + (6 * SIZE * sizeof(int))), SIZE * sizeof(int), 91, all_rqst[6]);
      //send(&dst, ((char *)buf + (6 * SIZE * sizeof(int))), SIZE * sizeof(int), 91);

      all_rqst[7] = &rqst[7];
      //buf[0] = 98;
      ((int *)((char *)buf + (7 * SIZE * sizeof(int))))[0] = 98;
      asend(&dst, ((char *)buf + (7 * SIZE * sizeof(int))), SIZE * sizeof(int), 98, all_rqst[7]);

      all_rqst[8] = &rqst[8];
      //buf[0] = 90;
      ((int *)((char *)buf + (8 * SIZE * sizeof(int))))[0] = 90;
      asend(&dst, ((char *)buf + (8 * SIZE * sizeof(int))), SIZE * sizeof(int), 90, all_rqst[8]);

      all_rqst[9] = &rqst[9];
      //buf[0] = 99;
      ((int *)((char *)buf + (9 * SIZE * sizeof(int))))[0] = 99;
      asend(&dst, ((char *)buf + (9 * SIZE * sizeof(int))), SIZE * sizeof(int), 99, all_rqst[9]);

      waitall(all_rqst, MAX_RQST, stats);

      if (!(i % MOD)) fprintf(stdout, "[%d] Enviado msg %d \n", getRank(), i);

      // SYNCHRONOUS
      //((int *)((char *)buf + (10 * SIZE * sizeof(int))))[0] = 100;
      //send(&dst, ((char *)buf + (10 * SIZE * sizeof(int))), SIZE * sizeof(int), 100);
      sbuf = i;
      fprintf(stdout, "ssss %d \n", i);
      send(&dst, (char *)&sbuf, sizeof(int), 100); 
      fprintf(stdout, "SEND - SYNCHRONOUS: (msg %d) %d  %d \n", i, sbuf, status.Tag);

      for (j = 0; j < SIZE * MAX_RQST; j++)  buf[j] = 0;

      //sched_yield();

    }
  }

  free(buf);

  t_end = abstime();
  if (myid == 0) {fprintf(stdout, "TIME: %lf\n", t_end - t_start);}

  fprintf(stdout, "I am the %d rank of %d processes finalizing ...\n", myid, numprocs);
  fflush(stdout);

  return(0);
}
