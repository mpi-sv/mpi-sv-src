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
#include <stdlib.h>
#include <unistd.h>


/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
#define  MAX_ITER     1
#define  PRINT_ITER   10000
#define  SIZE         8

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

  printf("HOLA: operator_main\n");

  myid = getRank();
  GRP_getSize(getGroup(), &numprocs);

  buf = (int *)malloc(SIZE * sizeof(int));

  fprintf(stdout, "Process %d of %d starting ... \n", myid, numprocs);

  if (myid == 0) {
    
    dst.Group = getGroup();
    dst.Rank  = 1;

    for (i = 0; i < MAX_ITER; i++) {

      //fprintf(stdout, "Filling buffer to send\n");
      for (j = 0; j < SIZE; j++)
        buf[j] = j + i;

      //usleep(1000);
      send(&dst, buf, SIZE * sizeof(int), 99);
      //fprintf(stdout, "-\n");fflush(stdout);

      if (!(i % PRINT_ITER)) fprintf(stdout, "Sended msg %d\n", i);
    }

  } else if (myid == 1) {

    src.Group = getGroup();
    src.Rank  = 0;

    for (i = 0; i < MAX_ITER; i++) {

      for (j = 0; j < SIZE; j++)
        buf[j] = 0;
      
      //fprintf(stdout, "Calling to receive (%d)... \n", i);
      recv(&src, buf, SIZE * sizeof(int), 99, &status);
      //fprintf(stdout, "+\n");fflush(stdout);

      for (j = 0; j < SIZE; j++) {
        if (buf[j] != j + i) {
          fprintf(stdout, "ERROR: incorrect data %d (expected %d)\n", buf[j], j + i);
          exit(1);
          break;
        }
      }

      if (!(i % PRINT_ITER)) fprintf(stdout, "Received msg %d\n", i);
      fprintf(stdout, "********************  Received msg %d\n", i);
    }

  }

  free(buf);

  fprintf(stdout, "I am the %d rank of %d processes finalizing ...\n", myid, numprocs);

  return(0);
}
