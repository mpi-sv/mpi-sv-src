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
#define  SIZE         ((8192 * 2) * sizeof(int))
#define  NUM_MSGS     100
#define  MOD          10

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
  int     *bufrecv;
  int      i, j;
  double   t_start, t_end;
  Status   status;
  int      total, total_r, result;


  myid = getRank();
  GRP_getSize(getGroup(), &numprocs);

  fprintf(stdout, "Process %d of %d starting ... (self: 0x%x)\n", myid, numprocs, (int)THR_self());

  t_start = abstime();

  buf     = (int *)malloc(SIZE * sizeof(int));
  bufrecv = (int *)malloc(SIZE * sizeof(int));

  if (!(myid % 2))  sleep(1);

  for (i = 0; i < NUM_MSGS; i++) {

    if (myid % 2) { /* IMPARES: Grupo de receptores */


      dst.Group = getGroup();
      src.Group = getGroup();
      src.Rank  = ADDR_RNK_ANY;

      recv(&src, (char *)bufrecv, SIZE * sizeof(int), 99, &status);

      total_r = 0;
      for (j = 0; j < status.Count / sizeof(int); j++) {
        total_r = total_r + bufrecv[j];
      }
      if ((!(i % MOD)) && (myid == 0)) fprintf(stdout, "[%d] Recibido de %d con total: %d\n", getRank(), status.Src.Rank, total_r);

      dst.Rank = status.Src.Rank;
      send (&dst, (char *)&total_r, sizeof(int), 57);

    } else { /* PARES: Grupo de emisores */

      dst.Group = getGroup();
      dst.Rank  = rand() % numprocs;
      if (!(dst.Rank % 2)) dst.Rank++;

      total = 0;
      for (j = 0; j < SIZE; j++) {
        buf[j] = rand() % 100;
        total = total + buf[j];
      }

      if ((!(i % MOD)) && (myid == 0)) fprintf(stdout, "[%d] Enviando a %d con total: %d\n", getRank(), dst.Rank, total);

      send(&dst, (char *)buf, SIZE * sizeof(int), 99);

      recv(&dst, (char *)&result, sizeof(int), TAG_ANY, &status);

      if (result == total) {
        if ((!(i % MOD)) && (myid == 0)) fprintf(stdout, "[%d] Recibido OK  %d\n", myid, i);
      } else {
        fprintf(stdout, "\n\n*************\n [%d] Recibido FAIL  %d  (res: %d)\n\n*************\n\n", myid, i, result);
        goto exception;
      }

    }
  }

  free(buf);
  free(bufrecv);

  t_end = abstime();
  if (myid == 0) fprintf(stdout, "TIME: %lf\n" , t_end - t_start);

  fprintf(stdout, "I am the %d rank of %d processes finalizing ...\n", myid, numprocs);
  fflush(stdout);

  return(0);

exception:
  free(buf);
  fprintf(stdout, "I am the %d rank of %d processes finalizing with ERRORS\n", myid, numprocs);
  return(1);
}
