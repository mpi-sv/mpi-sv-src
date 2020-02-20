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

  int      *sbuf, *rbuf;
  int      rank, size;
  int      i;
  int      toterr, err = 0, sumval;
  int      root;

  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &size );
  MPI_Comm_rank(MPI_COMM_WORLD, &rank );

  sbuf = (int *) malloc(size * sizeof(int));
  for (i = 0; i < size; i++)
	  sbuf[i] = rank + i;

  rbuf = (int *) malloc( size * sizeof(int) );


  MPI_Scan(sbuf, rbuf, size, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

  printf("\nProcess %d\n", rank);
  for (i = 0; i < size; i++)
    printf("%d  ", rbuf[i]);

  MPI_Finalize();
  return MPI_SUCCESS;
}
