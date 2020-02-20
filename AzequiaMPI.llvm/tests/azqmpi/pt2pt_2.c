#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

#define COUNT  (8)

int main (int argc, char **argv) {

  int          myid, numprocs;
  int         *buf_1, *buf_2;
  int          i, k, err = 0;
  MPI_Status   st;
  double       t_start, t_end;


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  if (numprocs != 2) {
    fprintf(stdout, "This program must be run with 2 processes\n");
    MPI_Finalize();
    return 0;
  }
  
  buf_1 = (int *) malloc (COUNT * sizeof(int));
  buf_2 = (int *) malloc (COUNT * sizeof(int));

  if (myid == 0) {

	t_start = MPI_Wtime();
    for (k = 0; k < 1000000; k++) {
      
	  MPI_Send(buf_1, COUNT, MPI_INT, 1, 76, MPI_COMM_WORLD);
      MPI_Recv(buf_2, COUNT, MPI_INT, 1, 77, MPI_COMM_WORLD, &st);
    
	}
	t_end = MPI_Wtime() - t_start;
	
  } else if (myid == 1) {

	t_start = MPI_Wtime();
    for (k = 0; k < 1000000; k++) {
	  
      MPI_Recv(buf_2, COUNT, MPI_INT, 0, 76, MPI_COMM_WORLD, &st);
      MPI_Send(buf_1, COUNT, MPI_INT, 0, 77, MPI_COMM_WORLD);
	  
	}
	t_end = MPI_Wtime() - t_start;
	
  }
  
  MPI_Reduce(&t_end, &t_start, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  
  if (myid == 0) {
	fprintf(stdout, "TIME: %.9f\n", t_start / 2);
  }

  free(buf_1);
  free(buf_2);

  MPI_Finalize();
  return(0);
}

