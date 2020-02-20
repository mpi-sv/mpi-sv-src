#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
#endif


#include "mpi.h"


#define NUM_RQST      5
#define MAX_ITER    500


int main (int argc, char **argv) {

  int          numprocs, myid;
  int          buf;
  MPI_Request  request;
  MPI_Status   status;
  int          i;


  MPI_Init (NULL, NULL);
  MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);

  if (myid == 1) {

    MPI_Ssend_init (&buf, 1, MPI_INT, 0, 47, MPI_COMM_WORLD, &request);

    for (i = 0; i < MAX_ITER; i++) {
      buf = i;
      MPI_Start(&request);
      MPI_Wait(&request, &status);
    }

    MPI_Request_free(&request);

  } else if (myid == 0) {

    MPI_Recv_init(&buf, 1, MPI_INT, 1, 47, MPI_COMM_WORLD, &request);

    for (i = 0; i < MAX_ITER; i++) {
      MPI_Start(&request);
      MPI_Wait(&request, &status);
      if (buf != i) {
        printf("ERROR (%i): Received value %d\n", i, buf);
        exit(1);
      }
      printf("(%d) Received  - %d -  from process %d \n", i, buf, status.MPI_SOURCE);
    }

    MPI_Request_free(&request);

  }

  MPI_Finalize ();
  return 0;
}

