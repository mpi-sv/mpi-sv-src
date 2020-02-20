#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"


#define MAX_PROCS   32
#define BUFSIZE     512

int main (int argc, char **argv) {

  int         myid;
  int         numprocs;
  MPI_Request rqst;
  MPI_Status  status;
  int         id;
  int         count;
  int         i;
  int         root;
  int        *sbuf, *rbuf;
  double      t_start, t_end;


  MPI_Init (NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  root = 2;

  if (myid == root) {

    sbuf = (int *) malloc (numprocs * BUFSIZE * sizeof(int));

    for (i = 0; i < numprocs * BUFSIZE; i++)
       sbuf[i] = i;
  }

  rbuf = (int *) malloc (BUFSIZE * sizeof(int));
  for (i = 0; i < BUFSIZE; i++)
    rbuf[i] = -1;

  t_start = MPI_Wtime();

  for (i = 0; i < 1000; i++) {
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Scatter(sbuf, BUFSIZE, MPI_INT, rbuf, BUFSIZE, MPI_INT, root, MPI_COMM_WORLD);
  } 

  t_end = MPI_Wtime();

  if (myid == 0) {
    fprintf(stdout, "\nProccess: %d (time %.4f) \n", myid, t_end - t_start);
    for (i = 0; i < BUFSIZE; i++)
      fprintf(stdout, "%d\t", rbuf[i]);
  }

  MPI_Finalize();
  return(0);
}
