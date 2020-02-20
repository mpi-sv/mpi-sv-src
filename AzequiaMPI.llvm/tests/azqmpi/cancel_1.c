#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

//#define SIZE 4096
#define SIZE 4

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
    //sleep(1);
    buf[0] = myid;
    printf("Envío Síncrono\n"); fflush(stdout);
    MPI_Ssend (buf, SIZE, MPI_INT, 1, 99, MPI_COMM_WORLD);
  //MPI_Send (buf, SIZE, MPI_INT, 1, 99, MPI_COMM_WORLD);
  } 
  else if (myid == 1) {
    buf[0] = 0;
    printf("\tRecibo asíncrono\n"); fflush(stdout);
    MPI_Irecv (buf, SIZE, MPI_INT, 0, 99, MPI_COMM_WORLD, &req);

    printf("\tCancelo\n"); fflush(stdout);
    MPI_Cancel (&req);

    printf("\tEspero\n"); fflush(stdout);
    MPI_Wait (&req, &status);
    MPI_Test_cancelled (&status, &flag);
    if (!flag) {
      printf ("\t---------------------------Failed to cancel a Irecv request\n");
      goto salida;
    }
    else
      printf ("\t+++++++++++++++++++++++++++Recv Request SUCCESFULLY cancelled. Try again\n");

    printf("\n\tRecibo asíncrono\n"); fflush(stdout);
    MPI_Irecv (buf, SIZE, MPI_INT, 0, 99, MPI_COMM_WORLD, &req2);

    printf("\tEspero\n"); fflush(stdout);
    MPI_Wait (&req2, &status);
    MPI_Test_cancelled (&status, &flag);
    if (flag) {
      printf ("\tIntegrity error in the MPI library\n");
      exit(1);
    }
  }

salida:
  free (buf);
  if (myid == 0) {
    printf("Adiós\n");   fflush(stdout);
  }
  else {
    printf("\tAdiós\n"); fflush(stdout);
  }
  printf("vvvv %d vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n", myid); fflush(stdout);
  MPI_Finalize ();
  return 0;
}
