#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
#endif

#include "mpi.h"


int main (int argc, char **argv) {

  int        myid, numprocs;
  int        i = 0;
  int        randval;
  int        errs = 0;
  MPI_Comm   newcomm;


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  for (i = 0; i < 10000; i++) {

    if (myid == 0 && (i % 1000 == 0)) {
      printf("After %d\n", i);
    }

    randval = rand();

    if (randval % (myid + 2) == 0) {
      MPI_Comm_split(MPI_COMM_WORLD, 1, myid, &newcomm);
      MPI_Comm_free(&newcomm);
    } else {
      MPI_Comm_split(MPI_COMM_WORLD, MPI_UNDEFINED, myid, &newcomm);
      if (newcomm != MPI_COMM_NULL) {
	      errs++;
	      printf( "Created a non-null communicator with MPI_UNDEFINED\n" );
      }
    }

  }

  MPI_Finalize();
  return MPI_SUCCESS;
}
