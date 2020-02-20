#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"


#define MAX_MPI_PROCS 32


int main (int argc, char **argv) {

  int       myid, numprocs;
  int       ranks[MAX_MPI_PROCS];
  int       ranks_tled[MAX_MPI_PROCS];
  int       n, i;
  MPI_Group group_world, newgroup;


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  MPI_Comm_group(MPI_COMM_WORLD, &group_world);


  n = 5;
  ranks[0] = 1;
  ranks[1] = 12;
  ranks[2] = 6;
  ranks[3] = 7;
  ranks[4] = 5;
  MPI_Group_incl(group_world, n, ranks, &newgroup);


  n = 6;
  ranks[0] = 12;
  ranks[1] = 5;
  ranks[2] = 9;
  ranks[3] = 7;
  ranks[4] = 0;
  ranks[5] = 10;
  MPI_Group_translate_ranks(group_world, n, ranks, newgroup, ranks_tled);

  fprintf(stdout, "\nRanks: ");
  for (i = 0; i < n; i++)
    fprintf(stdout, "%d -> %d  ", i, ranks_tled[i]);

  MPI_Group_free(&group_world);

  MPI_Finalize();
  return MPI_SUCCESS;
}
