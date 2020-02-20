#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

#define COUNT  (1024 * 8 * 64)

int main (int argc, char **argv) {

  int          myid, numprocs;
  int         *buf_1, *buf_2;
  int          i, k, err = 0;
  MPI_Status   st;
  MPI_Request  req;


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  if (numprocs != 2) {
    fprintf(stdout, "This program must be run with 2 processes\n");
    MPI_Finalize();
    return 0;
  }
  
  buf_1 = (int *) malloc (COUNT * sizeof(int));
  buf_2 = (int *) malloc (COUNT * sizeof(int));

  for (k = 0; k < 1000; k++) {

  if (myid == 0) {

    for (i = 0; i < COUNT; i++) {
      buf_1[i] = 1;
      buf_2[i] = -1;
    }

    MPI_Isend(buf_1, COUNT, MPI_INT, 1, 77, MPI_COMM_WORLD, &req);
    MPI_Recv(buf_2, COUNT, MPI_INT, 1, 77, MPI_COMM_WORLD, &st);
    MPI_Wait(&req, &st);

  } else /* (myid == 1) */ {

    for (i = 0; i < COUNT; i++) {
      buf_1[i] = 0;
      buf_2[i] = -1;
    }

    MPI_Isend(buf_1, COUNT, MPI_INT, 0, 77, MPI_COMM_WORLD, &req);
    MPI_Recv(buf_2, COUNT, MPI_INT, 0, 77, MPI_COMM_WORLD, &st);
    MPI_Wait(&req, &st);

  }

  fprintf(stdout, ". ");

  for (i = 0; i < COUNT; i++) {
    if (buf_2[i] != myid) err++;
  }
  if (err) break;

  }

  printf("\nProcess %d has errors: %d \n", myid, err);

  free(buf_1);
  free(buf_2);

  MPI_Finalize();
  return(0);
}

