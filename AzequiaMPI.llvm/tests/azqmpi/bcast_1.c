#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

//#define CHECK

#define MAX_SIZE  (1024 * 1024)


int main (int argc, char **argv) {

  int    myid, numprocs, i, j, k, num_iter;
  double startwtime = 0.0, endwtime;
  int    namelen;
  char   processor_name[MPI_MAX_PROCESSOR_NAME];
  int    root;
  int   *buff;

/*
  cpu_set_t  cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(getRank(), &cpuset);
  fprintf(stdout, "[%d] con cpuset %x\n", getRank(), cpuset);
  if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) <0) {
	perror("pthread_setaffinity_np");
  }
*/
  
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD,&myid);
  MPI_Get_processor_name(processor_name,&namelen);

  fprintf(stdout, "Procesador: %s\n",processor_name); 

  posix_memalign(&buff, 64, MAX_SIZE * sizeof(char));

  for (i = 1; i < MAX_SIZE; i <<= 1) {

	if      (i < 1024)                num_iter = 1000000;
	else if (i < 1024 * 1024)         num_iter = 100000;
	else if (i < 1024 * 1024 * 1024)  num_iter = 10000;
	else                              num_iter = 1000;

       num_iter = 1;
	
	startwtime = MPI_Wtime();
	
	for (k = 0; k < num_iter; k++) {
	  
    //root = i % numprocs;
      root = 0;
	 
#ifdef CHECK
    if (myid == root) {
      for (j = 0; j < i / sizeof(int); j++) ((int *)buff)[j] = j;
    } else {
      for (j = 0; j < i / sizeof(int); j++) ((int *)buff)[j] = 0;
    }
#endif

      MPI_Bcast(buff, i, MPI_CHAR, root, MPI_COMM_WORLD);

#ifdef CHECK
    for (j = 0; j < i / sizeof(int); j++) {
      if (((int *)buff)[j] != j) {
        fprintf(stdout, "[%d] ERROR: %d en %d\n", getRank(), myid, j); fflush(stdout);
		break;
      }
    }
#endif
	
	//sleep(2);
	}
	
	endwtime = MPI_Wtime() - startwtime;
	MPI_Reduce(&endwtime, &startwtime, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	
    if (myid == 0) {
	  fprintf(stdout, "%d\t%.9f\n", i, (startwtime / (2 * num_iter)) * 1000000);
    }	

  }
  
  free(buff);
  MPI_Finalize();
  return 0;
}
