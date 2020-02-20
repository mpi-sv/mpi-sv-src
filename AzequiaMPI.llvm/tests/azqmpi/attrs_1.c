#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

int main (int argc, char **argv) {

  int         numprocs, myid;
  int        *maxval;
  int         flag;
  int         tag_ub;
  MPI_Status  status;
  int         message[16];

  MPI_Init (NULL, NULL);
  MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);

  if ((myid == 0) || (myid == 1)) {

    MPI_Attr_get(MPI_COMM_WORLD, MPI_TAG_UB, &maxval, &flag);
    if (!flag) printf("ERROR: Attribute MPI_TAG_UB\n");
    printf("[Rank %d] MPI_TAG_UB:           %d\n", myid, *maxval);
    tag_ub = (int)*maxval;

    MPI_Attr_get(MPI_COMM_WORLD, MPI_HOST, &maxval, &flag);
    if (!flag) printf("ERROR: Attribute MPI_HOST\n");
    printf("[Rank %d] MPI_HOST:             %d\n", myid, *maxval);

    MPI_Attr_get(MPI_COMM_WORLD, MPI_IO, &maxval, &flag);
    if (!flag) printf("ERROR: Attribute MPI_IO\n");
    printf("[Rank %d] MPI_IO:               %d\n", myid, *maxval);

    MPI_Attr_get(MPI_COMM_WORLD, MPI_WTIME_IS_GLOBAL, &maxval, &flag);
    if (!flag) printf("ERROR: Attribute MPI_WTIME_IS_GLOBAL\n");
    printf("[Rank %d] MPI_WTIME_IS_GLOBAL:  %d\n", myid, *maxval);

    MPI_Attr_get(MPI_COMM_WORLD, MPI_MAX_NODES, &maxval, &flag);
	if (!flag) printf("ERROR: Attribute MPI_MAX_NODES\n");
    printf("[Rank %d] MPI_MAX_NODES:        %d\n", myid, *maxval);

  }

  if (myid == 0) {

    MPI_Send(message, 16, MPI_INT, 1, tag_ub, MPI_COMM_WORLD);
    MPI_Send(message, 16, MPI_INT, 1, tag_ub, MPI_COMM_WORLD);
    MPI_Send(message, 16, MPI_INT, 1, 67, MPI_COMM_WORLD);
    /* Next message must not arrived
    MPI_Send(message, 16, MPI_INT, 1, 76, MPI_COMM_WORLD);
    */

  } else if (myid == 1) {

    MPI_Recv(message, 16, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    printf("Received message with tag: %d\n", status.MPI_TAG);
    MPI_Recv(message, 16, MPI_INT, 0, tag_ub, MPI_COMM_WORLD, &status);
    printf("Received message with tag: %d\n", status.MPI_TAG);
    MPI_Recv(message, 16, MPI_INT, 0, 67, MPI_COMM_WORLD, &status);
    printf("Received message with tag: %d\n", status.MPI_TAG);
    /* Next message must not arrived
    MPI_Recv(message, 16, MPI_INT, 0, tag_ub, MPI_COMM_WORLD, &status);
    printf("Received message with tag: %d\n", status.MPI_TAG);
    */
  }

  MPI_Finalize ();

  return MPI_SUCCESS;
}

