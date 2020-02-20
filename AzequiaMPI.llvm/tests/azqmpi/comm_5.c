#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

#define MAX_PROCS   32


int main (int argc, char **argv) {

  int        myid, numprocs;
  int        res, res2;
  int        root = 0;
  MPI_Group  newgroup, group_world;
  MPI_Comm   newcomm;
  int        i, j;
  int        ranks[MAX_PROCS];


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);


  j = 0;
  for (i = 0; i < numprocs; i++) {
    if (i % 2) ranks[j++] = i;
  }
  MPI_Comm_group(MPI_COMM_WORLD, &group_world);
  MPI_Group_excl(group_world, numprocs / 2, ranks, &newgroup);
  MPI_Comm_create(MPI_COMM_WORLD, newgroup, &newcomm);

  if (newcomm != MPI_COMM_NULL) {
    res  = myid;
    MPI_Reduce(&myid, &res, 1, MPI_INT, MPI_SUM, root, newcomm);
    if (myid == root)
      printf("Process %d result: %d\n", myid, res);
  }

  res2  = myid;
  MPI_Reduce(&myid, &res2, 1, MPI_INT, MPI_SUM, root, MPI_COMM_WORLD);
  if (myid == root)
    printf("Process %d result: %d\n", myid, res2);


  if (newcomm != MPI_COMM_NULL)  /* For MPICH2 to work properly */
    MPI_Comm_free(&newcomm);
  MPI_Group_free(&group_world);
  MPI_Group_free(&newgroup);

  MPI_Finalize();
  return MPI_SUCCESS;
}
