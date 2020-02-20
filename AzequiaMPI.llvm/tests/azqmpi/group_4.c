#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"


int main (int argc, char **argv) {

  int       myid, myrank_incl, myrank_excl, numprocs;
  MPI_Group group_world, grp_new_incl, grp_new_excl;
  int       range[2][3];


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  MPI_Comm_group(MPI_COMM_WORLD, &group_world);

  range[0][0] = 1;
  range[0][1] = numprocs - 1;
  range[0][2] = 2;
  MPI_Group_range_excl(group_world, 1, range, &grp_new_excl);


  range[0][0] = 1;
  range[0][1] = (numprocs / 2) - 1;
  range[0][2] = 2;
  range[1][0] = numprocs / 2;
  range[1][1] = numprocs - 1;
  range[1][2] = 1;
  MPI_Group_range_incl(group_world, 2, range, &grp_new_incl);


  MPI_Group_rank(grp_new_incl, &myrank_incl);
  MPI_Group_rank(grp_new_excl, &myrank_excl);
  printf("Ranks =>  World: %d,  grp_new_incl: %d  grp_new_excl: %d\n", myid, myrank_incl, myrank_excl);

  MPI_Group_free(&grp_new_incl);
  MPI_Group_free(&grp_new_excl);
  MPI_Group_free(&group_world);

  MPI_Finalize();
  return MPI_SUCCESS;
}
