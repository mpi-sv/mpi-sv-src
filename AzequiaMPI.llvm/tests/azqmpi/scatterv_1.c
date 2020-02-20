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
  int         i, j;
  int         root;
  int        *sbuf, *rbuf;
  int         displs[MAX_PROCS];
  int         scounts[MAX_PROCS];
  double      t_start, t_end;

  MPI_Init (NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  t_start = MPI_Wtime();
  for (j = 0; j < 1000; j++) {

  root = j % numprocs; /* Rand() NOT VALID */

  if (myid == root) {

    sbuf = (int *) malloc (numprocs * numprocs * sizeof(int));

    for (i = 0; i < numprocs * numprocs; i++)
       sbuf[i] = i;

    for (i = 0; i < MAX_PROCS; i++)
      scounts[i] = i + 1;

    displs[0] = 0;
    for (i = 1; i < MAX_PROCS; i++)
      displs[i] = displs[i - 1] + i;

  }

  rbuf = (int *) malloc (numprocs * sizeof(int));
  for (i = 0; i < numprocs; i++)
    rbuf[i] = -1;

  MPI_Scatterv(sbuf, scounts, displs, MPI_INT, rbuf, myid + 1, MPI_INT, root, MPI_COMM_WORLD);

  }

  t_end = MPI_Wtime();

  printf("\nProccess: %d\n", myid);
  for (i = 0; i < numprocs; i++)
    printf("%d\t", rbuf[i]);

  if (myid == 0)  fprintf(stdout, "Time: %lf\n", t_end - t_start);

  MPI_Finalize();
  return(0);
}
