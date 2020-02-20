#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

#define NUM_RQST 4

int main (int argc, char **argv) {

  int          numprocs, myid;
  int          buf;
  int          bufrecv[NUM_RQST];
  MPI_Request  req[NUM_RQST];
  MPI_Status   status[NUM_RQST];
  int          i;
  int          count;


  MPI_Init (NULL, NULL);
  MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);
  
  if (numprocs != 4) {
	fprintf (stdout, "This program must be runned with 4 ranks\n");
	MPI_Finalize();
	exit(0);
  }

  if ((myid > 0) && (myid < NUM_RQST)) {

    buf = myid;
    sleep(myid);
    MPI_Send (&buf, 1, MPI_INT, 0, 99 - myid, MPI_COMM_WORLD);

  } else if (myid == 0) {

    req[0] = MPI_REQUEST_NULL;
    MPI_Irecv(&bufrecv[0], 1, MPI_INT, 1, MPI_ANY_TAG, MPI_COMM_WORLD, &req[1]);
    MPI_Irecv(&bufrecv[1], 1, MPI_INT, 2, MPI_ANY_TAG, MPI_COMM_WORLD, &req[2]);
    MPI_Irecv(&bufrecv[2], 1, MPI_INT, 3, MPI_ANY_TAG, MPI_COMM_WORLD, &req[3]);

    printf ("Waiting for all requests ...\n");
    MPI_Waitall(NUM_RQST, req, status);

    for (i = 0; i < NUM_RQST; i++) {
      MPI_Get_count(&status[i], MPI_INT, &count);
      printf("Status. Received %d elements from %d process with %d tag\n", count, status[i].MPI_SOURCE, status[i].MPI_TAG);
    }
  }

  MPI_Finalize ();
  return 0;
}

