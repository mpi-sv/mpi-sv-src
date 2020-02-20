#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
#endif


#include "mpi.h"

int main (int argc, char **argv) {

	int         myid, numprocs;
  MPI_Status  status;
  int         i;
  int         buf[1000];

  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  if (myid == 0) {
    for (i = 0; i < 1000; i++)
      MPI_Send(buf, i, MPI_INT, 1, 678, MPI_COMM_WORLD);
  } else if (myid == 1) {
    for (i = 0; i < 1000; i++)
      MPI_Recv(buf, i, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
  }

  MPI_Finalize();
  return MPI_SUCCESS;
}

