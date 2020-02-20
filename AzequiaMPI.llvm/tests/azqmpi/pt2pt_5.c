#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include "mpi.h"

//#define MAX_SIZE  (4 * 1024 * 1024)
#define MAX_SIZE  (4 * 1024)

int main (int argc, char **argv) {

  int          myid, numprocs;
  int         *buf_1, *buf_2;
  int          i, k, err = 0;
  int          num_iter;
  double       t_start, t_end;
  unsigned long mask;
  
/*
  cpu_set_t  cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(getRank(), &cpuset);
  fprintf(stdout, "[%d] con cpuset %x\n", getRank(), cpuset);
  if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) <0) {
	perror("pthread_setaffinity_np");
  }
*/
  
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  if (numprocs != 2) {
    fprintf(stdout, "This program must be run with 2 processes\n");
    MPI_Finalize();
    return 0;
  }
  
  MPI_Barrier(MPI_COMM_WORLD);
  
  fprintf(stdout, "\n ********************** [%d   %p] RUNNING THE TEST\n\n", myid, pthread_self()); fflush(stdout);
  
  for (i = 1; i <= MAX_SIZE; i <<= 1) {
	
    buf_1 = (int *) malloc (i * sizeof(char));

    if      (i < 1024)                num_iter = 1000000;
	else if (i < 1024 * 1024)         num_iter = 100000;
	else if (i < 1024 * 1024 * 1024)  num_iter = 10000;
	else                              num_iter = 1000;
    num_iter = 10;	
	MPI_Barrier(MPI_COMM_WORLD);
	
    if (myid == 0) {

	  t_start = MPI_Wtime();
      for (k = 0; k < num_iter; k++) {
      
	    MPI_Send(buf_1, i, MPI_BYTE, 1, 76, MPI_COMM_WORLD);
    
	  }
	  t_end = MPI_Wtime() - t_start;
	
    } else if (myid == 1) {

	  t_start = MPI_Wtime();
      for (k = 0; k < num_iter; k++) {
	  
        MPI_Recv(buf_1, i, MPI_BYTE, 0, 76, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	  
	  }
	  t_end = MPI_Wtime() - t_start;
	
	}
	
	free(buf_1);
  
    MPI_Reduce(&t_end, &t_start, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  
    if (myid == 0) {
	  fprintf(stdout, "%d\t%.9f\n", i, (t_start / (2 * num_iter)) * 1000000);
    }
  }

  fprintf(stdout, "Ending %d ...\n", myid);  
  MPI_Finalize();
  return(0);
}

