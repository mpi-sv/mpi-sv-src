#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"


#define  SIZE      (32)
#define  USERSIZE  (4)

int main (int argc, char **argv) {

  int            numprocs, myid;
  int            flag;
  MPI_Status     status;
  int            message[SIZE];
  int            count;
  int            userbufsize;
  char          *userbuf;
  int            i;


  MPI_Init (NULL, NULL);
  MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);

  if (myid == 0) {

    /* Make room for USERSIZE messages in the user buffer */
    userbufsize = (USERSIZE * MPI_BSEND_OVERHEAD) + (USERSIZE * SIZE);
    userbuf = (char *)malloc(userbufsize * sizeof(int));
    MPI_Buffer_attach(userbuf, userbufsize * sizeof(int));

    printf("MPI_BSEND_OVERHEAD:  %d (total: %d)\n", MPI_BSEND_OVERHEAD, userbufsize);

    for (i = 0; i < 1000; i++) {
      MPI_Bsend(message, SIZE, MPI_INT, 1, 52, MPI_COMM_WORLD);
      if (!(i % USERSIZE)) usleep(10000);
      printf("(%d) message sended\n", i);
    }

    printf("Detaching buffer ... \n");
    MPI_Buffer_detach(&userbuf, &userbufsize);
    printf("Buffer detached (size: %d)\n", userbufsize);
    free(userbuf);

  } else if (myid == 1) {

    for (i = 0; i < 1000; i++) {
      MPI_Recv(message, SIZE, MPI_INT, 0, 52, MPI_COMM_WORLD, &status);
      MPI_Get_count(&status, MPI_INT, &count);
      printf("(%d) Received message with tag: %d and size: %d\n", i, status.MPI_TAG, count);
    }

  }

  MPI_Finalize ();

  return MPI_SUCCESS;
}
