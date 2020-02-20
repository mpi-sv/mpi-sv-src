#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
#endif


#include "mpi.h"


#define  SIZE       32


int main (int argc, char **argv) {

  int         numprocs, myid;
  int         flag;
  MPI_Status  status;
  int         message[SIZE];
  int         bufrecv[SIZE];
  int         count;
  int         i;
  MPI_Request req;

  MPI_Init (NULL, NULL);
  MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);

  if (myid == 0) {

    /* 1. Sending a to myself using Non-blocking SEND */
    for (i = 0; i < SIZE; i++) message[i] = i;
    MPI_Isend(message, SIZE,  MPI_INT, myid, 12, MPI_COMM_WORLD, &req);

    for (i = 0; i < SIZE; i++) bufrecv[i] = 0;
    MPI_Recv(bufrecv, SIZE, MPI_INT, myid, 12, MPI_COMM_WORLD, &status);
    for (i = 0; i < SIZE; i++) {
      if (bufrecv[i] != i) {
        printf("ERROR: Received data\n");
        break;
      }
    }
    MPI_Get_count(&status, MPI_INT, &count);
    printf("1. Received message with tag: %d and size: %d\n", status.MPI_TAG, count);

    MPI_Wait(&req, &status);


    /* 2. Sending a to myself using Non-blocking RECV */
    for (i = 0; i < SIZE; i++) bufrecv[i] = 0;
    MPI_Irecv(bufrecv, SIZE, MPI_INT, myid, 12, MPI_COMM_WORLD, &req);

    for (i = 0; i < SIZE; i++) message[i] = i;
    MPI_Send(message, SIZE,  MPI_INT, myid, 12, MPI_COMM_WORLD);

    MPI_Wait(&req, &status);
    for (i = 0; i < SIZE; i++) {
      if (bufrecv[i] != i) {
        printf("ERROR: Received data\n");
        break;
      }
    }
    MPI_Get_count(&status, MPI_INT, &count);
    printf("2. Received message with tag: %d and size: %d\n", status.MPI_TAG, count);

  }


  MPI_Finalize ();

  return MPI_SUCCESS;
}

