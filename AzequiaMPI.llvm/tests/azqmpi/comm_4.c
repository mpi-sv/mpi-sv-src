#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"


int lib_fxn (int root, int a, int b, int *res) {

  MPI_Comm  dup_comm_world;

  MPI_Comm_dup(MPI_COMM_WORLD, &dup_comm_world);

  /* Do work */
  MPI_Reduce(&a, &b, 1, MPI_INT, MPI_SUM, root, dup_comm_world);
  *res = b;

  MPI_Comm_free(&dup_comm_world);
}



int main (int argc, char **argv) {

  int     myid, numprocs;
  int     res;
  int     root;


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);


  root = 2;
  lib_fxn(root, myid, myid, &res);
  if (myid == root)
    printf("Process %d result: %d\n", myid, res);


  MPI_Finalize();
  return MPI_SUCCESS;
}
