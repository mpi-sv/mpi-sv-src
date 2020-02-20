#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "mpi.h"

//#define TABLE_SIZE   517
#define TABLE_SIZE 7

int randlc() {
 
  int myid;
  int val;

  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
  if (myid == 0) {
    val = rand() % 200;
  }
  MPI_Bcast(&val, 1, MPI_INT, 0, MPI_COMM_WORLD);

  return val;
} 


int main (int argc, char **argv) {

  int        rank, size;
  int        i;
  int        in[TABLE_SIZE], out[TABLE_SIZE];
  int        errors = 0, toterrors;
  double     t_start, t_end;


  MPI_Init(NULL, NULL);

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (rank == 0) srand(120);
  for (i = 0; i < TABLE_SIZE; i++)
    in[i] = randlc();

  if (rank == 0) {
    for (i = 0; i < TABLE_SIZE; i++) fprintf(stdout,"%d  ", in[i]);
    fprintf(stdout,"\n\n");
  }

  MPI_Allreduce(in, out, TABLE_SIZE, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

  if (rank == 0) {
    for (i = 0; i < TABLE_SIZE; i++) fprintf(stdout,"%d  ", out[i]);
  }

  MPI_Finalize();

  return errors;
}
