#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

int main (int argc, char **argv) {

  int          numprocs, myid;
  int          buf;
  MPI_Request  request, req2;
  MPI_Status   status;
  int          flag;


  MPI_Init (NULL, NULL);
  MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);


  if (myid == 1) {

    buf = 123;
    MPI_Send_init (&buf, 1, MPI_INT, 0, 99, MPI_COMM_WORLD, &request);

    MPI_Isend(&myid, 1, MPI_INT, 0, 99, MPI_COMM_WORLD, &req2);

    MPI_Start(&request);
    MPI_Cancel(&request);

    MPI_Wait(&req2, &status);
    MPI_Wait(&request, &status);

    MPI_Test_cancelled(&status, &flag);
    if (flag)
      printf("Request CANCELLED succesfully\n");
    MPI_Request_free(&request);

  } else if (myid == 0) {

    sleep(1);
    MPI_Recv (&buf, 1, MPI_INT, 1, 99, MPI_COMM_WORLD, &status);
    printf("\n1. Received  - %d -  from process %d \n", buf, status.MPI_SOURCE);
    MPI_Recv (&buf, 1, MPI_INT, 1, 99, MPI_COMM_WORLD, &status);
    printf("\n2. Received  - %d -  from process %d \n", buf, status.MPI_SOURCE);
  }

  MPI_Finalize ();
  return 0;
}
