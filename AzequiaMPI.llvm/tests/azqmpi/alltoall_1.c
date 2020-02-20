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
  double      t_start, t_end;
  double      resol;

  MPI_Init (NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  rbuf = (int *) malloc (numprocs * sizeof(int));
  sbuf = (int *) malloc (numprocs * sizeof(int));

  for (i = 0; i < numprocs; i++)
    sbuf[i] = myid;

  for (i = 0; i < numprocs; i++)
    rbuf[i] = -1;

  resol = MPI_Wtick();
  fprintf(stdout, "Resolution: %.12lf\n", resol);
  
  t_start = MPI_Wtime();

  //for (i = 0; i < 1000; i++)
    for(i=0;i<10;i++)
    MPI_Alltoall(sbuf, 1, MPI_INT, rbuf, 1, MPI_INT, MPI_COMM_WORLD);

  t_end = MPI_Wtime();

  printf("\nProcess: %d  (Time: %.4f)\n", myid, t_end - t_start);
  for (i = 0; i < numprocs; i++)
    printf("%d\t", rbuf[i]);

  MPI_Finalize();
  return(0);
}

