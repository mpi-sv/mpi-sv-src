/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#ifdef __OSI
#include <osi.h>
#endif
#include <stdio.h>
#include <azq.h>
#include <malloc.h>


/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
#define  MAX_ITER     1000000
#define  PRINT_ITER   100000
#define  SIZE         32

char ALPHABET [25] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
                      'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
                      'u', 'v', 'x', 'y', 'z' };


/*----------------------------------------------------------------*
 *   Definition of functions                                      *
 *----------------------------------------------------------------*/
int latency_azq(int *param) {

  int      myid;
  int      numprocs;
  Addr     src, dst;
  char    *buf;
  Status   status;
  int      i, j;
  Rqst     rqst;

  myid     = getRank();
  GRP_getSize(getGroup(), &numprocs);

  srand();

  buf = (char *)malloc(SIZE);

  if (myid == 0) {

    dst.Group = getGroup();
    dst.Rank  = 1;

    for (i = 0; i < MAX_ITER; i++) {

      for (j = 0; j < SIZE; j++)
        buf[j] = ALPHABET[(i + j) % 25];

      if (rand() % 2) usleep(1);
      send(&dst, buf, SIZE, 99);
    }

  } else if (myid == 1) {

    src.Group = getGroup();
    src.Rank  = 0;

    for (i = 0; i < MAX_ITER; i++) {

      for (j = 0; j < SIZE; j++)
        buf[j] = 0;

      arecv(&src, buf, SIZE, 99, &rqst);
      cancel(&rqst);
      wait(&rqst, &status);
      if (status.Cancelled) {
        fprintf(stdout, "o");fflush(stdout);
        recv(&src, buf, SIZE, 99, &status);
      } else {
        fprintf(stdout, ".");fflush(stdout);
      }
      //fprintf(stdout, "Rqst cancelled %s\n", status.Cancelled ? "OK" : "FAIL");


      for (j = 0; j < SIZE; j++) {
        if (buf[j] != ALPHABET[(i + j) % 25]) {
          fprintf(stdout, "ERROR: incorrect data %c (expected %c)\n", buf[j], ALPHABET[(i + j) % 25]);
          break;
        }
      }

      if (!(i % PRINT_ITER)) fprintf(stdout, "Received msg %d\n", i);
    }

  }

  free(buf);

  fprintf(stdout, "I am the %d rank of %d processes finalizing ...\n", myid, numprocs);

  return(0);
}
