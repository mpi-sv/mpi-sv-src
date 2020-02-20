#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"


#define  SIZE      (1024)
#define  USERSIZE  (8)


int main (int argc, char **argv) {

  int            numprocs, myid;
  int            flag;
  MPI_Status     status;
  int            message[SIZE];
  int            count;
  int            userbufsize;
  char          *userbuf;


  MPI_Init (NULL, NULL);
  MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);

  if (myid == 0) {

    /* Make room for USERSIZE messages in the user buffer */
    userbufsize = (USERSIZE * MPI_BSEND_OVERHEAD) + (USERSIZE * SIZE);
    userbuf = (char *)malloc(userbufsize * sizeof(int));
    MPI_Buffer_attach(userbuf, userbufsize * sizeof(int));

    MPI_Bsend(message, SIZE, MPI_INT, 1, 52, MPI_COMM_WORLD);

    MPI_Bsend(message, SIZE,  MPI_INT, 1, 52, MPI_COMM_WORLD);

    MPI_Buffer_detach(&userbuf, &userbufsize);
    free(userbuf);

  } else if (myid == 1) {

    MPI_Recv(message, SIZE, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_INT, &count);
    printf("Received message with tag: %d and size: %d\n", status.MPI_TAG, count);

    MPI_Recv(message, SIZE, MPI_INT, 0, 52, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_INT, &count);
    printf("Received message with tag: %d and size: %d\n", status.MPI_TAG, count);

  }

  MPI_Finalize ();

  return MPI_SUCCESS;
}

