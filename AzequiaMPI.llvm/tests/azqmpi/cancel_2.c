#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

#define SIZE 4096

int main (int argc, char **argv) {

  int          numprocs, myid;
  int          *buf;
  MPI_Request  req, req2;
  MPI_Status   status;
  int          flag;

  MPI_Init (NULL, NULL);
  MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);

  if (NULL == (buf = (int *) malloc (SIZE * sizeof(int)))) {
    printf("ERROR: in malloc\n");
    return -1;
  }
  printf("^^^^^ %d ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n", myid); fflush(stdout);
  if (myid == 0) {

    buf[0] = myid;
    MPI_Isend (buf, SIZE, MPI_INT, 1, 61, MPI_COMM_WORLD, &req);
    //usleep(1);
    MPI_Cancel(&req);
    MPI_Wait  (&req, &status);
    MPI_Test_cancelled(&status, &flag);
    if (!flag) {
      printf ("----------------------------------------------------Failed to cancel the Isend request 0x%x\n", req); 
      fflush(stdout);
    }
    else {
      printf ("++++++++++++++++++++++++++++++++++++++++++++++++++++Send Request SUCCESFULLY cancelled\n");  fflush(stdout);
      //printf ("-------------------------------------------Send again...\n");  fflush(stdout);
      MPI_Send(buf, SIZE, MPI_INT, 1, 62, MPI_COMM_WORLD);
    }
    printf ("\nAdiós\n"); 
  } 
  else if (myid == 1) {

    //usleep(10000);
    buf[0] = 0;
    MPI_Irecv (buf, SIZE, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &req);
    MPI_Wait (&req, &status);
    printf("\n\tReceived message from %d with tag %d\n", status.MPI_SOURCE, status.MPI_TAG);
    printf ("\tAdiós\n"); 
  }
  printf("vvvv %d vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n", myid); fflush(stdout);
  free (buf);
  MPI_Finalize ();
  return 0;
}
