#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

#define MAX_MPI_PROCS  32
#define RANK            3


void compare (int res) {

  if      (res == MPI_IDENT)    fprintf(stdout, "MPI_IDENT\n");
  else if (res == MPI_SIMILAR)  fprintf(stdout, "MPI_SIMILAR\n");
  else if (res == MPI_UNEQUAL)  fprintf(stdout, "MPI_UNEQUAL\n");
  else                          fprintf(stdout, "compare ERROR\n");
}


int main (int argc, char **argv) {

  int       myid, numprocs;
  int       myid1, numprocs1;
  int       myid2, numprocs2;
  MPI_Comm  newcomm1, newcomm2;
  MPI_Group group1, group2, newgroup, commgroup, newgroup_15, newgroup_1, group_empty;
  int       res1, res2, res3, res4, res5, res6;
  int       ranks[MAX_MPI_PROCS];
  int       i;


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  MPI_Comm_split(MPI_COMM_WORLD, (myid == RANK) ? 0: 1, myid, &newcomm1);
  MPI_Comm_split(MPI_COMM_WORLD, (myid != 0) ? 0: 1, myid, &newcomm2);

  MPI_Comm_group(MPI_COMM_WORLD, &group_empty);

  for (i = 0; i < 15; i++) /* All but rank 0, inverse */
    ranks[i] = 15 - i;
  MPI_Group_incl(group_empty, 15, ranks, &newgroup_15);
  
  ranks[0] = 3;
  MPI_Group_incl(group_empty, 1, ranks, &newgroup_1);


  if (myid == RANK) {
    MPI_Comm_size(newcomm1, &numprocs1);
    MPI_Comm_rank(newcomm1, &myid1);

	  MPI_Comm_size(newcomm2, &numprocs2);
    MPI_Comm_rank(newcomm2, &myid2);

    fprintf(stdout, "Process %d of %d in newcomm1\n", myid1, numprocs1);
	  fprintf(stdout, "Process %d of %d in newcomm2\n", myid2, numprocs2);

	  MPI_Comm_group(newcomm1, &group1);
	  MPI_Comm_group(newcomm2, &group2);
	  MPI_Comm_group(MPI_COMM_WORLD, &commgroup);

    MPI_Group_union(group1, group2, &newgroup);

	  MPI_Group_compare(group1, group2, &res1); /* MPI_UNEQUAL */
	  fprintf(stdout, "Comparing GROUP1 and GROUP2...\n");
	  compare(res1);

	  MPI_Group_compare(newgroup, commgroup, &res2); /* MPI_UNEQUAL */
	  fprintf(stdout, "Comparing NEWGROUP and COMMGROUP...\n");
	  compare(res2);

	  MPI_Group_compare(newgroup, newgroup, &res3); /* MPI_IDENT */
	  fprintf(stdout, "Comparing NEWGROUP and NEWGROUP...\n");
	  compare(res3);

	  MPI_Group_compare(newgroup_15, group2, &res5); /* MPI_SIMILAR */
	  fprintf(stdout, "Comparing NEWGROUP_15 and GROUP2...\n");
	  compare(res5);

	  MPI_Group_compare(newgroup_1, group1, &res6); /* MPI_IDENT */
	  fprintf(stdout, "Comparing NEWGROUP_1 and GROUP1...\n");
	  compare(res6);


	  MPI_Group_free(&commgroup);
	  MPI_Group_free(&group1);
	  MPI_Group_free(&group2);
	  MPI_Group_free(&newgroup);
  }
  
  fflush(stdout);

  MPI_Group_free(&newgroup_1);
  MPI_Group_free(&newgroup_15);
  MPI_Group_free(&group_empty);
  MPI_Comm_free(&newcomm1);
  MPI_Comm_free(&newcomm2);

  MPI_Finalize();
  return MPI_SUCCESS;
}
