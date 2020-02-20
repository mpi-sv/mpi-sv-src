#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"


#define  NUM_MSGS    1000


int main (int argc, char **argv) {

  int          numprocs, myid;
  int          i;
  double       t_start, t_end;


  MPI_Init (&argc, &argv);
  MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);

  if (myid == 0) fprintf(stdout, "Empiezo el Barrier\n"); fflush(stdout);

  t_start = MPI_Wtime();
  for (i = 0; i < NUM_MSGS; i++) {
	//fprintf(stdout, "-> %d \n", myid);fflush(stdout);
    MPI_Barrier(MPI_COMM_WORLD);
    //fprintf(stdout, "<- %d \n", myid);fflush(stdout);
	//if (myid == 0) fprintf(stdout, "%d ", i);fflush(stdout);
  }
  t_end = MPI_Wtime();

  if (myid == 0) {
    printf("Time: %.4f\n", t_end - t_start); fflush(stdout);
  }

  //fprintf(stdout, "Terminado: %d\n", myid); fflush(stdout);

  MPI_Finalize ();
  return 0;
}

