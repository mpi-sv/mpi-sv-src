#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

#define BUFSIZE     64

int main (int argc, char **argv) {

  float    *sbuf, *rbuf;
  int       rank, size;
  int       i;
  int       root = 0;
  double    t_start, t_end;

  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  sbuf = (float *) malloc (BUFSIZE * sizeof(float));
  rbuf = (float *) malloc (BUFSIZE * sizeof(float));

  for (i = 0; i < BUFSIZE; i++) {
    sbuf[i] = (float)rank;
    rbuf[i] = (float)999;
  }

  t_start = MPI_Wtime();
  for (i = 0; i < 100; i++)
    MPI_Reduce(sbuf, rbuf, BUFSIZE, MPI_FLOAT, MPI_SUM, root, MPI_COMM_WORLD);
  t_end = MPI_Wtime();

  if (rank == root) {
    printf("I am the process %d, the root\n", root);
    for (i = 0; i < BUFSIZE; i++)
      printf("%f\t", rbuf[i]);
    printf("Time = %.4f\n\n", t_end - t_start);
  }

  free(rbuf);
  free(sbuf);

  MPI_Finalize();
  return(0);
}
