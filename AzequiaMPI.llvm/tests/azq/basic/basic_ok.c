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
//#define  SIZE         ((8192 * 1) * sizeof(int))
#define  SIZE         (20 * sizeof(int))
#define  NUM_MSGS     1000000
#define  MOD          1000

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
  int      i, j;
  double   t_start, t_end;
  Status   status;
  int      total, total_r;
  Rqst     rqst;


  myid = getRank();
  GRP_getSize(getGroup(), &numprocs);

  //fprintf(stdout, "Process %d of %d starting ... (self: 0x%x)\n", myid, numprocs, (int)THR_self());

  buf = (int *)malloc(SIZE * sizeof(int));
 
  t_start = abstime();

  //if (!(myid % 2))  sleep(1);

  for (i = 0; i < NUM_MSGS; i++) {

    if (myid % 2) { /* IMPARES: Grupo de receptores */

      src.Group = getGroup();
      src.Rank  = myid - 1;

      recv(&src, (char *)buf, SIZE * sizeof(int), 99, &status);

      total_r = 0;
      for (j = 0; j < status.Count / sizeof(int); j++) {
        total_r = total_r + buf[j];
      }
      if ((!(i % MOD)) && (myid == 0)) {
        fprintf(stdout, "[%d] Recibido de %d con total: %d\n", getRank(), status.Src.Rank, total_r);
        fflush(NULL);
      }
      
      send(&src, (char *)&total_r, sizeof(int), 87);

    } else { /* PARES: Grupo de emisores */

      dst.Group = getGroup();
      dst.Rank  = myid + 1;

      total = 0;
      for (j = 0; j < SIZE; j++) {
        buf[j] = rand() % 100;
        total = total + buf[j];
      }

      if ((!(i % MOD)) && (myid == 0)) {
        fprintf(stdout, "[%d / %d] Enviando a %d con total: %d\n", getRank(), i, dst.Rank, total);
        fflush(NULL);
      }

      send(&dst, (char *)buf, SIZE * sizeof(int), 99);

      recv(&dst, (char *)&total_r, sizeof(int), TAG_ANY, &status);
      if (total != total_r) {
        fprintf(stdout, "ERROR: [%d] Recibido msg %d. Total: %d / %d\n", getRank(), i, total, total_r);
        fflush(NULL);
      }

    }
  }

  t_end = abstime();
  if (myid == 0) fprintf(stdout, "%lf\n" , t_end - t_start);

  free(buf);

  //fprintf(stdout, "I am the %d rank of %d processes finalizing ...\n", myid, numprocs);
  fflush(stdout);

  return(0);
}
