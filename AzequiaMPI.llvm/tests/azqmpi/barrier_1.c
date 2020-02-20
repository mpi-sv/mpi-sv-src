#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"


int main (int argc, char **argv) {

  int          numprocs, myid;
  int          buf;
  MPI_Request  request;
  MPI_Status   status;
  int          i;
  MPI_Comm     newcomm;
  MPI_Group    group, newgrp;
  int         *ranks;
  double       t_start, t_end;


  MPI_Init (&argc, &argv);
  MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);

  fprintf(stdout, "Process %d creating group ... \n", myid);

  MPI_Comm_group (MPI_COMM_WORLD, &group);
  ranks = (int *)malloc((numprocs / 2) * sizeof(int));
  for (i = 0; i < numprocs / 2; i++) {
    ranks[i] = i * 2;
  }
  MPI_Group_incl(group, numprocs / 2, ranks, &newgrp);
  MPI_Comm_create (MPI_COMM_WORLD, newgrp, &newcomm);

  if (newcomm == MPI_COMM_NULL) {
    printf("Process %d has nothing to do\n", myid);
    MPI_Finalize();
    return 0;
  }

  MPI_Comm_rank(newcomm, &myid);
  
  if (myid == 3) {
    //sleep(1);
    printf("Process %d wasting time ... \n", myid);
  }

  fprintf(stdout, "Process %d doing barrier ... \n", myid);
  t_start = MPI_Wtime();
  for (i = 0; i < 1000; i++) {
    MPI_Barrier(newcomm);
  }
  t_end = MPI_Wtime();

  if (myid == 0) {
    printf("Time: %.4f\n", t_end - t_start);
  }

  MPI_Group_free(&group);
  MPI_Group_free(&newgrp);
  MPI_Comm_free(&newcomm);

  MPI_Finalize ();
  return 0;
}

