#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"


#define  SIZE      (32)

int main (int argc, char **argv) {

  int            numprocs, myid;
  MPI_Status     status;
  int            message[SIZE];
  int            count;


  MPI_Init (NULL, NULL);
  MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);

  if (myid == 0) {

    MPI_Send(message, SIZE, MPI_INT, 1, 52, MPI_COMM_WORLD);

    MPI_Recv(message, SIZE, MPI_INT, 1, 53, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_INT, &count);
    printf("[Rank %d] Received message with tag: %d and size: %d\n", myid, status.MPI_TAG, count);

  } else if (myid == 1) {

    MPI_Send(message, SIZE,  MPI_INT, 0, 53, MPI_COMM_WORLD);

    MPI_Recv(message, SIZE, MPI_INT, 0, 52, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_INT, &count);
    printf("[Rank %d] Received message with tag: %d and size: %d\n", myid, status.MPI_TAG, count);

  }

  MPI_Finalize ();

  return MPI_SUCCESS;
}

