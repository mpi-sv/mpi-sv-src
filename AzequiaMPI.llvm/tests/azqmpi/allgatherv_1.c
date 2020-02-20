#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"


#define MAX_PROCS   32

int main (int argc, char **argv) {

  int         myid;
  int         numprocs;
  MPI_Request rqst;
  MPI_Status  status;
  int         id;
  int         count;
  int         i;
  int         root;
  int        *sbuf, *rbuf;
  int         displs[MAX_PROCS];
  int         rcounts[MAX_PROCS];

  MPI_Init (NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  rbuf = (int *) malloc (numprocs * numprocs * sizeof(int));
  sbuf = (int *) malloc (numprocs * sizeof(int));

  for (i = 0; i <= myid; i++)
    sbuf[i] = i;

  for (i = 0; i < MAX_PROCS; i++)
   rcounts[i] = i + 1;

  displs[0] = 0;
  for (i = 1; i < MAX_PROCS; i++)
    displs[i] = displs[i - 1] + i;

  MPI_Allgatherv(sbuf, myid + 1, MPI_INT, rbuf, rcounts, displs, MPI_INT, MPI_COMM_WORLD);

  printf("\nProcess: %d\n", myid);
  for (i = 0; i < numprocs * numprocs; i++)
    printf("%d\t", rbuf[i]);

  MPI_Finalize();
  return(0);
}

