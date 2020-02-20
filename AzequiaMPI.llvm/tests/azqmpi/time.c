#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

int main (int argc, char **argv) {

  int          numprocs, myid;
  char         message[20];
  MPI_Status   status;
  double       t_start, t_end;


  MPI_Init (NULL, NULL);
  MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);

  t_start = MPI_Wtime();
  sleep(1);
  t_end = MPI_Wtime();
  if (myid == 0) fprintf(stdout, "Time of 1 sec: %.4f\n", t_end - t_start);
  
  t_start = MPI_Wtime();
  
  if (myid == 0) {

    strcpy(message, "Hello, there");
    MPI_Send (message, strlen(message), MPI_CHAR, 1, 99, MPI_COMM_WORLD);

  } else if(myid == 1) {

    MPI_Recv(message, 20, MPI_CHAR, 0, 99, MPI_COMM_WORLD, &status);
    //printf("Received :%s:\n", message);

  }
  
  t_end = MPI_Wtime();
  if (myid == 0) fprintf(stdout, "Time of message passing: %.4f\n", t_end - t_start);

  MPI_Finalize ();

  return MPI_SUCCESS;
}

