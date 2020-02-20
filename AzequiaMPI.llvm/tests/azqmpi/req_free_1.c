#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
#endif


#include "mpi.h"


#define NUM_RQST 4

int main (int argc, char **argv) {

  int          numprocs, myid;
  int          buf;
  int          bufrecv[NUM_RQST];
  MPI_Request  req[NUM_RQST];
  MPI_Status   status;
  int          flag;
  int          i;
  int          idx;
  int          count;


  MPI_Init (NULL, NULL);
  MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);

  if ((myid > 0) && (myid <= NUM_RQST)) {

    buf = myid;
    MPI_Isend (&buf, 1, MPI_INT, 0, 36, MPI_COMM_WORLD, &req[myid]);
    MPI_Request_free(&req[myid]);
    printf("Process %d send message \n", myid);

  } else if (myid == 0) {

    MPI_Irecv(&bufrecv[0], 1, MPI_INT, 1, MPI_ANY_TAG, MPI_COMM_WORLD, &req[0]);
    MPI_Irecv(&bufrecv[1], 1, MPI_INT, 2, MPI_ANY_TAG, MPI_COMM_WORLD, &req[1]);
    MPI_Irecv(&bufrecv[2], 1, MPI_INT, 3, MPI_ANY_TAG, MPI_COMM_WORLD, &req[2]);
    req[3] = MPI_REQUEST_NULL;

    printf("MPI_ANY_TAG is %d and MPI_ANY_SOURCE is %d\n", MPI_ANY_TAG, MPI_ANY_SOURCE);

    for (i = 0; i < NUM_RQST; i++) {

      if (i % 2) {

        MPI_Wait(&req[i], &status);
        printf("Request %d is 0x%x\n", i, req[i]);
        MPI_Get_count(&status, MPI_INT, &count);
        printf("Received last message from %d (tag %d) with %d elements\n", status.MPI_SOURCE, status.MPI_TAG, count);

      } else {

        MPI_Request_free(&req[i]);

      }

    }

  } else if (myid > NUM_RQST) {

    /* Test request_free wth a request null */
    req[0] = MPI_REQUEST_NULL;
    MPI_Request_free(&req[0]);

  }

  MPI_Finalize ();

  return 0;
}

