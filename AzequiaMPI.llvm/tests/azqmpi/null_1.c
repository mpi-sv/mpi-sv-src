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
  int         buf;
  MPI_Status  status;
  int         count;


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);


  if (myid == 0) {

    MPI_Recv(&buf, 1, MPI_INT, MPI_PROC_NULL, 52, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_INT, &count);
    printf("\nProcess %d. [Status] source: %d  tag: %d  Count %d\n", myid, status.MPI_SOURCE, status.MPI_TAG, count);
  }


  MPI_Finalize();
  return(0);
}

