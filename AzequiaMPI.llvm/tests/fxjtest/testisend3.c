#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

#define COUNT  8

int main (int argc, char **argv) {

  int         myid, numprocs;
  int        *buf;
  int         i;
  MPI_Status  status;
  MPI_Request request;
  int         src, dst;


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);


  buf = (int *) malloc (COUNT * sizeof(int));

  for (i = 0; i < COUNT; i++)
    buf[i] = myid;

  if (myid == 0) {

    dst = 1;
    MPI_Isend(buf, COUNT, MPI_INT, dst, 77, MPI_COMM_WORLD, &request);
    MPI_Wait(&request, &status);

  } else if (myid == 2) {

    src = 1;
    MPI_Irecv(buf, COUNT, MPI_INT, src, 99, MPI_COMM_WORLD, &request);
    MPI_Wait(&request, &status);

  } else if (myid == 1) {

    dst = 2; src = 0;
    MPI_Sendrecv_replace(buf, COUNT, MPI_INT, dst, 99, src, 77, MPI_COMM_WORLD, &status);

  } else {
    printf("\nProcess %d has nothing to do ", myid);
    MPI_Finalize();
    return(0);
  }


  printf("\nProcess %d -> ", myid);
  for (i = 0; i < COUNT; i++)
    printf("%d\t", buf[i]);

  free(buf);

  MPI_Finalize();
  return(0);
}

