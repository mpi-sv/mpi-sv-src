#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"


#define MAX_MPI_PROCS 32


int main (int argc, char **argv) {

  int       myid, numprocs;
  MPI_Group group_world;
  int       rank, grpsize;


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  
  MPI_Comm_group(MPI_COMM_WORLD, &group_world);

  MPI_Group_rank(group_world, &rank);
  MPI_Group_size(group_world, &grpsize);

  fprintf(stdout, "[%d] WORLD  Size: %d  Rank: %d \n", myid, grpsize, rank);

  MPI_Group_free(&group_world);
  
  MPI_Finalize();
  return MPI_SUCCESS;
}
