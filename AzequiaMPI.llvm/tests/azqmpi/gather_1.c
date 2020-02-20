#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"


#define BUFLEN 2
#define NPROCS 32

int main (int argc, char **argv) {

  int        myid, numprocs, tot = 0, i;
  int        buffer[BUFLEN];
  int        buf2[BUFLEN * NPROCS];
  MPI_Status status;
  MPI_Comm   newcomm;
  double     t_start, t_end;


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD,&myid);


  for (i = 0; i < BUFLEN; i++)
    buffer[i] = myid;
  for (i = 0; i < BUFLEN * NPROCS; i++)
    buf2[i] = 999;

  t_start = MPI_Wtime();
  for (i = 0; i < NPROCS; i++)
    MPI_Gather(buffer, BUFLEN, MPI_INT, buf2, BUFLEN, MPI_INT, i, MPI_COMM_WORLD);
  t_end  = MPI_Wtime();

  if (myid == 0) {
    printf("Rank %d (Time: %.4f)\n", myid, t_end - t_start);
    for (i = 0; i < BUFLEN * NPROCS; i++)
      printf("%d/%d -> %d   ", myid, i, buf2[i]);
    printf("\n");
  }

  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
  return (0);
}
