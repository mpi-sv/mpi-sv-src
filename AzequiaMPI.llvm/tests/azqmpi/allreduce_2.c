#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "mpi.h"

#define TABLE_SIZE   4


int main (int argc, char **argv) {

  int        rank, size;
  int        i;
  int        in[TABLE_SIZE], out[TABLE_SIZE];
  int        errors = 0, toterrors;
  double     t_start, t_end;


  MPI_Init(NULL, NULL);

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  for (i = 0; i < TABLE_SIZE; i++)
    in[i] = i;

  MPI_Allreduce(in, out, TABLE_SIZE, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

  if (rank == 0) {
    for (i = 0; i < TABLE_SIZE; i++) fprintf(stdout,"%d  ", out[i]);
  }

  MPI_Finalize();

  return errors;
}
