#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
#endif


#include "mpi.h"


/* Error handler */
void errhnd_fxn (MPI_Comm *comm, int *err) {

  int class;

  MPI_Error_class(*err, &class);
  if (class != MPI_ERR_GROUP) {
	  printf( "Error handler function: incorrect error class %d\n", class );
  }
  *err = MPI_SUCCESS;
}


int main (int argc, char **argv) {

  int            numprocs, myid;
  int            flag;
  MPI_Status     status;
  int            message[16];
  int            count;
  int            i;
  MPI_Errhandler errhnd;


  MPI_Init (NULL, NULL);
  MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);

  if (myid == 0) {

    /* Sending zero sized message */
    MPI_Send(message, 0,  MPI_INT, 1, 23, MPI_COMM_WORLD);

    /* Sending a shorter message than expected */
    for (i = 0; i < 8; i++) message[i] = i;
    MPI_Ssend(message, 8,  MPI_INT, 1, 24, MPI_COMM_WORLD);

    /* Sending a greater sized message than expected => BUFFER OVERFLOW ERROR */
    MPI_Rsend(message, 16, MPI_INT, 1, 25, MPI_COMM_WORLD);

  } else if (myid == 1) {

    MPI_Recv(message, 0, MPI_INT, 0, 23, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_INT, &count);
    printf("Received message with tag: %d and size: %d\n", status.MPI_TAG, count);

    for (i = 0; i < 16; i++) message[i] = 0;
    MPI_Recv(message, 16, MPI_INT, 0, 24, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_INT, &count);
    printf("Received message with tag: %d and size: %d\n", status.MPI_TAG, count);
    for (i = 0; i < 16; i++) printf("%d  ", message[i]);

    /* Trying to receive less size than sended (i.e. 8) cause an overflow error */
    MPI_Errhandler_create(errhnd_fxn, &errhnd);
    MPI_Errhandler_set(MPI_COMM_WORLD, errhnd);

    MPI_Recv(message, 8, MPI_INT, 0, 25, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_INT, &count);
    printf("\nReceived message with tag: %d and size: %d\n", status.MPI_TAG, count);

    MPI_Errhandler_free(&errhnd);
  }


  MPI_Finalize ();

  return MPI_SUCCESS;
}

