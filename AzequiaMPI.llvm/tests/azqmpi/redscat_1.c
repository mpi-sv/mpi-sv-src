#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
#endif


#include "mpi.h"


#define MAX_PROCS   32


int main (int argc, char **argv) {

  int      *sbuf, rbuf;
  int      rank, size;
  int      *recvcounts;
  int      i;
  int      toterr, err = 0, sumval;
  double   t_start, t_end;

  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &size );
  MPI_Comm_rank(MPI_COMM_WORLD, &rank );

  sbuf = (int *) malloc(size * sizeof(int));
  for (i = 0; i < size; i++)
	  sbuf[i] = rank + i;

  recvcounts = (int *)malloc(size * sizeof(int));
  for (i = 0; i < size; i++)
	  recvcounts[i] = 1;

  t_start = MPI_Wtime();
  for (i = 0; i < 100; i++)
    MPI_Reduce_scatter(sbuf, &rbuf, recvcounts, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
  t_end = MPI_Wtime();

  sumval = size * rank + ((size - 1) * size)/2;
  if (rbuf != sumval) {
	  err++;
	  printf("Did not get expected value for reduce scatter\n");
	  printf("[%d] Got %d expected %d\n", rank, rbuf, sumval);
  }

  MPI_Allreduce( &err, &toterr, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD );
  if (rank == 0 && toterr == 0) {
	  printf(" No Errors\n");
  }

  printf("Process %d, value %d  (time: %.4f)\n", rank, rbuf, t_end - t_start);

  MPI_Finalize();
  return MPI_SUCCESS;
}
