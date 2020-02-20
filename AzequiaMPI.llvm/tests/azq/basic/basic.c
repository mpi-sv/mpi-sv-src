/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/

#ifdef __OSI
#include <osi.h>
#endif
#undef wait
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <timer.h>

#include <azq.h>

/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
#define  SIZE         (1025 * sizeof(int))
#define  NUM_MSGS     1000
#define  MOD          100

/*----------------------------------------------------------------*
 *   Definition of private functions                              *
 *----------------------------------------------------------------*/

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
  double   t_start, t_end, resol;

  myid = getRank();
  GRP_getSize(getGroup(), &numprocs);

  fprintf(stdout, "Process %d of %d starting ... \n", myid, numprocs);

  
  buf = (int *)malloc(SIZE * sizeof(int));

  resol = getResTime();
  fprintf(stdout, "Timer resolution: %f\n", resol);
  t_start = getAbsTime();

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

  t_end = getAbsTime();
  fprintf(stdout, "Time: %.6f\n", t_end - t_start);
  
  free(buf);

  fprintf(stdout, "I am the %d rank of %d processes finalizing ...\n", myid, numprocs);
  fflush(stdout);

  return(0);

exception:
  free(buf);
  fprintf(stdout, "I am the %d rank of %d processes finalizing with ERRORS\n", myid, numprocs);
  return(1);
}
