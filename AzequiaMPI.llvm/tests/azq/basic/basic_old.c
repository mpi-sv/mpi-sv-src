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
#define  SIZE         (1025 * sizeof(int))
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
  Status   status;
  int      i, j;
  double   t_start, t_end;

  myid = getRank();
  GRP_getSize(getGroup(), &numprocs);

  fprintf(stdout, "Process %d of %d starting ... \n", myid, numprocs);

  t_start = abstime();

  buf = (int *)malloc(SIZE * sizeof(int));


  if (myid == 0) {

    dst.Group = getGroup();
    dst.Rank  = 1;

    for (i = 0; i < NUM_MSGS; i++) {

      for (j = 0; j < SIZE; j++)
        buf[j] = i;

      send(&dst, (char *)buf, SIZE * sizeof(int), 99);

      if (!(i % MOD)) fprintf(stdout, "[%d] Sended msg %d\n", myid, i);


      src.Group = getGroup();
      src.Rank  = (myid == 0) ? (numprocs - 1) : myid - 1;

      recv(&src, (char *)buf, SIZE * sizeof(int), 99, &status);

      for (j = 0; j < SIZE; j++)
        if (buf[j] != i) {
          printf("[%d] ERROR:  process received msg %d con valor %d\n", myid, i, buf[0]);
          goto exception;
        }

      if (!(i % MOD)) fprintf(stdout, "[%d] Received msg %d con valor %d\n", myid, i, buf[0]);
      fflush(stdout);

    }

  } else {

    dst.Group = getGroup();
    dst.Rank  = (myid + 1) % numprocs;

    for (i = 0; i < NUM_MSGS; i++) {

      src.Group = getGroup();
      src.Rank  = myid - 1;

      recv(&src, (char *)buf, SIZE * sizeof(int), 99, &status);

      for (j = 0; j < SIZE; j++)
        if (buf[j] != i) {
          printf("[%d] ERROR:  process received msg %d con valor %d\n", myid, i, buf[0]);
          goto exception;
        }

      if (!(i % MOD)) fprintf(stdout, "[%d] Received msg %d con valor %d\n", myid, i, buf[0]);


      for (j = 0; j < SIZE; j++)
        buf[j] = i;

      send(&dst, (char *)buf, SIZE * sizeof(int), 99);

      if (!(i % MOD)) fprintf(stdout, "[%d] Sended msg %d\n", myid, i);
      fflush(stdout);

    }

  }


  free(buf);

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
