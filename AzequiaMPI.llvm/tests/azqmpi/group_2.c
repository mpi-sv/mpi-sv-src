#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

#define MAX_MPI_PROCS  32
#define RANK            3


void compare (int res) {

  if      (res == MPI_IDENT)    fprintf(stdout, "MPI_IDENT");
  else if (res == MPI_SIMILAR)  fprintf(stdout, "MPI_SIMILAR");
  else if (res == MPI_UNEQUAL)  fprintf(stdout, "MPI_UNEQUAL");
  else                          fprintf(stdout, "compare ERROR");
}


int main (int argc, char **argv) {

  int       myid, numprocs;
  MPI_Group group_inter1, group_diff1, group_world, group_empty;
  int       res;
  int       ranks[MAX_MPI_PROCS];


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  MPI_Comm_group(MPI_COMM_WORLD, &group_world);

  MPI_Group_incl(group_world, 0, ranks, &group_empty);

  if (myid == RANK) {
    MPI_Group_intersection(group_world, group_empty, &group_inter1);
    MPI_Group_compare(group_inter1, group_world, &res); /* MPI_UNEQUAL */
	  fprintf(stdout,"\nGROUP_INTER1 vs GROUP_WORLD (MPI_UNEQUAL):"); compare(res);
    MPI_Group_compare(group_inter1, group_empty, &res); /* MPI_IDENT */
	  fprintf(stdout,"\nGROUP_INTER1 vs GROUP_EMPTY (MPI_IDENT):"); compare(res);

	  MPI_Group_difference(group_world, group_empty, &group_diff1);
    MPI_Group_compare(group_diff1, group_world, &res); /* MPI_IDENT */
	  fprintf(stdout,"\nGROUP_DIFF1 vs GROUP_WORLD (MPI_IDENT):"); compare(res);
    MPI_Group_compare(group_diff1, group_empty, &res); /* MPI_UNEQUAL */
	  fprintf(stdout,"\nGROUP_DIFF1 vs GROUP_EMPTY (MPI_UNEQUAL):"); compare(res);
  }

  MPI_Group_free(&group_world);
  MPI_Group_free(&group_empty);

  MPI_Finalize();
  return MPI_SUCCESS;
}
