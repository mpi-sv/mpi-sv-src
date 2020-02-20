#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

#define MAX_MPI_PROCS  32
#define RANK            3

int testComm (MPI_Comm comm) {

  int i;

  for (i = 0; i< 10; i++) {
    MPI_Barrier(comm);
  }

  return 0;
}


void compare (int res) {

  if      (res == MPI_IDENT)     fprintf(stdout, "MPI_IDENT\n");
  else if (res == MPI_SIMILAR)   fprintf(stdout, "MPI_SIMILAR\n");
  else if (res == MPI_UNEQUAL)   fprintf(stdout, "MPI_UNEQUAL\n");
  else if (res == MPI_CONGRUENT) fprintf(stdout, "MPI_CONGRUENT\n");
  else                           fprintf(stdout, "compare ERROR\n");
}


int main (int argc, char **argv) {

  int       myid, numprocs;
  MPI_Group group_world1, group_world2, group_full1, group_full2;
  MPI_Comm  newfull1, newfull2, commworld, comm_split;
  int       res;
  int       ranks[MAX_MPI_PROCS];
  int       i, j;
  int       flag;
  int       err  = 0;
  double    t_start, t_end;


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  if (myid == RANK) {
    MPI_Comm_test_inter(MPI_COMM_WORLD, &flag);
    //fprintf(stdout, "MPI_COMM_WORLD is an %s\n", flag ? "Intercommunicator" : "Intracommunicator");
  }

  t_start = MPI_Wtime();
  for (j = 0; j < 100; j++) {

    MPI_Comm_group(MPI_COMM_WORLD, &group_world1);
    MPI_Comm_group(MPI_COMM_WORLD, &group_world2);

    for (i = 0; i < numprocs; i++)
      ranks[i] = numprocs - i - 1;
    MPI_Group_incl(group_world1, numprocs, ranks, &group_full1);

    MPI_Group_excl(group_world2, 0, ranks, &group_full2);

    MPI_Comm_create(MPI_COMM_WORLD, group_full1, &newfull1);
    MPI_Comm_create(MPI_COMM_WORLD, group_full2, &newfull2);

    if (myid == RANK) {
      MPI_Comm_test_inter(newfull1, &flag);
      //fprintf(stdout, "newfull1 is an %s\n", flag ? "Intercommunicator" : "Intracommunicator");
    }

    //fprintf(stdout, "[%d] MPI_Comm_dup\n", myid);
    MPI_Comm_dup(MPI_COMM_WORLD, &commworld);
    //fprintf(stdout, "[%d] MPI_Comm_split\n", myid);
    MPI_Comm_split(MPI_COMM_WORLD, myid % 2, myid, &comm_split);

    //fprintf(stdout, "[%d] MPI_Comm_compare\n", myid);

    if (myid == RANK) {

      MPI_Comm_compare(MPI_COMM_WORLD, MPI_COMM_WORLD, &res);
      //fprintf(stdout,"MPI_COMM_WORLD vs MPI_COMM_WORLD (MPI_IDENT): ");  compare(res);

      MPI_Comm_compare(MPI_COMM_WORLD, newfull1, &res);
      //fprintf(stdout,"MPI_COMM_WORLD vs NEWFULL1 (MPI_SIMILAR): ");      compare(res);

      MPI_Comm_compare(MPI_COMM_WORLD, newfull2, &res);
      //fprintf(stdout,"MPI_COMM_WORLD vs NEWFULL2 (MPI_CONGRUENT): ");    compare(res);

      MPI_Comm_compare(MPI_COMM_WORLD, commworld, &res);
      //fprintf(stdout,"MPI_COMM_WORLD vs COMM_WORLD (MPI_CONGRUENT): ");  compare(res);

      MPI_Comm_compare(MPI_COMM_WORLD, comm_split, &res);
      //fprintf(stdout,"MPI_COMM_WORLD vs COMM_SPLIT (MPI_UNEQUAL): ");    compare(res);
    }

    if (0 > testComm(MPI_COMM_WORLD))  err++;
    if (0 > testComm(newfull1))        err++;
    if (0 > testComm(newfull2))        err++;
    if (0 > testComm(commworld))       err++;
    if (0 > testComm(comm_split))      err++;

    if (err && (myid == 0)) 
      fprintf(stdout, "[%d] ERRORS: %d\n", myid, err);

    //fprintf(stdout, "[%d] Freeing comms & groups\n", myid);
    MPI_Group_free(&group_world1);
    MPI_Group_free(&group_world2);
    MPI_Group_free(&group_full1);
    MPI_Group_free(&group_full2);
    MPI_Comm_free(&comm_split);
    MPI_Comm_free(&commworld);
    MPI_Comm_free(&newfull1);
    MPI_Comm_free(&newfull2);

  }

  t_end = MPI_Wtime();
  if (myid == 0)
    fprintf(stdout, "Time: %lf\n", t_end - t_start);

  MPI_Finalize();
  return MPI_SUCCESS;
}
