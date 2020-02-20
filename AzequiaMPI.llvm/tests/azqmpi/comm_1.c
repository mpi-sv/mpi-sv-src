#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"


int main (int argc, char **argv) {

  int        myid, numprocs;
  MPI_Group  group_self;


  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  if (myid == 2) {

    printf("I am the process global number %d: \n", myid);

    printf("\tWORLD Comm size %d\n", numprocs);
    printf("\tWORLD Comm rank %d\n", myid);

    MPI_Comm_size(MPI_COMM_SELF, &numprocs);
    MPI_Comm_rank(MPI_COMM_SELF, &myid);

    printf("\tSELF Comm size %d\n", numprocs);
    printf("\tSELF Comm rank %d\n", myid);

    MPI_Group_size(MPI_GROUP_EMPTY, &numprocs);
    MPI_Group_rank(MPI_GROUP_EMPTY, &myid);

    printf("\tEMPTY group size %d\n", numprocs);
    printf("\tEMPTY group rank %d\n", myid);

    MPI_Comm_group(MPI_COMM_SELF, &group_self);
    MPI_Group_size(group_self, &numprocs);
    MPI_Group_rank(group_self, &myid);

    printf("\tGROUP_SELF group size %d\n", numprocs);
    printf("\tGROUP_SELF group rank %d\n", myid);
  }

  MPI_Finalize();
  return MPI_SUCCESS;
}
