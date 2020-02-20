#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
#endif


#include "mpi.h"


#define NUM_RQST      5


int main (int argc, char **argv) {

  int          numprocs, myid;
  int          buf, buf2;
  MPI_Request  request, req2;
  MPI_Status   status;


  MPI_Init (NULL, NULL);
  MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);


  if (myid == 1) {

    buf = 1;
    MPI_Send(&buf, 1, MPI_INT, 0, 24, MPI_COMM_WORLD);

    buf = 2;
    MPI_Send(&buf, 1, MPI_INT, 0, 24, MPI_COMM_WORLD);


  } else if (myid == 0) {

    MPI_Recv_init (&buf, 1, MPI_INT, 1, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
    printf("MPI_Recv_init() done\n");

    MPI_Irecv(&buf2, 1, MPI_INT, 1, MPI_ANY_TAG, MPI_COMM_WORLD, &req2);
    printf("MPI_Irecv() done\n");

    MPI_Start(&request);
    printf("MPI_Start() done\n");

    MPI_Wait(&req2, &status);
    printf("\n1. Received  - %d -  from process %d (should be 1)\n", buf2, status.MPI_SOURCE);
    MPI_Wait(&request, &status);
    printf("\n2. Received  - %d -  from process %d (should be 2)\n", buf, status.MPI_SOURCE);

    MPI_Request_free(&request);

  }

  MPI_Finalize ();
  return 0;
}

