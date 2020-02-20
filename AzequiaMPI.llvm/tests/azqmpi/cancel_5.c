#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

int main (int argc, char **argv) {

  int          numprocs, myid;
  int          buf;
  MPI_Request  req;
  MPI_Status   status;
  int          flag;


  MPI_Init (NULL, NULL);
  MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);

  if (myid == 0) {

    sleep(1);
    buf = 222;
    MPI_Send (&buf, 1, MPI_INT, 1, 83, MPI_COMM_WORLD);
    buf = 333;
    MPI_Send (&buf, 1, MPI_INT, 1, 84, MPI_COMM_WORLD);

  } else if (myid == 1) {

    MPI_Recv_init(&buf, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &req);

    buf = 0;

    MPI_Start(&req);

    MPI_Cancel (&req);
	  MPI_Wait (&req, &status);
	  printf ("1. Completed wait. Received %d  (tag: %d)\n", buf, status.MPI_TAG);

    MPI_Test_cancelled (&status, &flag);
	  if (!flag)
	    printf ("Failed to cancel a Irecv request\n");
    else
      printf ("Request 0x%x SUCCESFULLY cancelled\n", req);

    MPI_Start(&req);
	  MPI_Wait (&req, &status);
	  printf ("2. Completed wait. Received %d  (tag: %d)\n", buf, status.MPI_TAG);

    MPI_Start(&req);
	  MPI_Wait (&req, &status);
	  printf ("3. Completed wait. Received %d  (tag: %d)\n", buf, status.MPI_TAG);

    MPI_Request_free(&req);

    MPI_Test_cancelled (&status, &flag);
	  if (!flag)
	    printf ("Failed to cancel an Irecv request\n");

  }

  MPI_Finalize ();
  return 0;
}
