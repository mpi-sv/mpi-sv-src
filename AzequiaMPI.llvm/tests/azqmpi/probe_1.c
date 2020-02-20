#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"


//#define  SIZE       (64 * 1024)
#define  SIZE       (8)


int main (int argc, char **argv)
{
  int         numprocs, myid;
//int         flag;
  MPI_Status  status;
  int        *message;
  int         count;
  int         i, k = 9;
//MPI_Request req;
  //printf("Probe_1: MPI_ANY_TAG is %x\n", MPI_ANY_TAG); fflush(stdout); exit(1);
  MPI_Init (NULL, NULL);
  MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);

  printf("Probe_1. ^^^ %d ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n", myid); fflush(stdout);
  message = (int *) malloc (SIZE * sizeof(int));
  
  if (myid == 0) {
    for (i = 0; i < SIZE; i++) message[i] = i;
    //sleep(1);
    MPI_Send(message, SIZE, MPI_INT, 1, k, MPI_COMM_WORLD);
  } 
  else if (myid == 1) {
    //sleep(1);
    for (i = 0; i < SIZE; i++) message[i] = 0;
  //printf("Probe_1: tag %x\n", MPI_ANY_TAG);
    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
  //MPI_Probe(MPI_ANY_SOURCE, k, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_INT, &count);
    printf("Probe results in a message with tag: %d and size: %d integers\n", status.MPI_TAG, count);
    MPI_Recv(message, count, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
  //printf("Message received \n");
    for (i = 0; i < SIZE; i++) {
      if (message[i] != i) {
        printf("ERROR: Received data\n");
        break;
      }
    }
  }
  free(message);
  printf("Probe_1. vvv %d vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n", myid); fflush(stdout);
  MPI_Finalize ();

  return MPI_SUCCESS;
}

