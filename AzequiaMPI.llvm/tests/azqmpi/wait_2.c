#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
#endif


#include "mpi.h"


#define NUM_RQST 4

int main (int argc, char **argv) {

  int          numprocs, myid;
  int          buf;
  int          bufrecv;
  MPI_Request  req;
  MPI_Status   status;
  int          flag;
  int          i;
  int          idx;
  int          count;


  MPI_Init (NULL, NULL);
  MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);

  if (myid == 0) {

    buf = myid;
    MPI_Ssend (&buf, 1, MPI_INT, 1, 72, MPI_COMM_WORLD);
    MPI_Send  (&buf, 1, MPI_INT, 1, 75, MPI_COMM_WORLD);

  } else if (myid == 1) {

    MPI_Irecv (&bufrecv, 1, MPI_INT, 0, 72, MPI_COMM_WORLD, &req);

    MPI_Recv  (&bufrecv, 1, MPI_INT, 0, 75, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_INT, &count);
    printf("RECV: from %d (tag %d) with %d elements\n", status.MPI_SOURCE, status.MPI_TAG, count);

    MPI_Wait(&req, &status);
    MPI_Get_count(&status, MPI_INT, &count);
    printf("IRECV: from %d (tag %d) with %d elements\n", status.MPI_SOURCE, status.MPI_TAG, count);

  }

  MPI_Finalize ();

  return 0;
}

