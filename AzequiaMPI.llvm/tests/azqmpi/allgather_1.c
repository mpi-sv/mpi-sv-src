#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"


#define MAX_PROCS   32
#define BUFLEN      1024

int main (int argc, char **argv) {

  int        myid, numprocs, tot = 0, i;
  int       *buffer;
  int       *buf2;
  MPI_Status status;
  MPI_Comm   newcomm;
  double     t_start, t_end;


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD,&myid);

  buffer = (int *) malloc (BUFLEN * sizeof(int));
  buf2   = (int *) malloc (BUFLEN * MAX_PROCS * sizeof(int));

  for (i = 0; i < BUFLEN; i++)
    buffer[i] = myid;
  for (i = 0; i < BUFLEN * MAX_PROCS; i++)
    buf2[i] = -1;

  t_start = MPI_Wtime();
  for (i = 0; i < 100; i++)
    MPI_Allgather(buffer, BUFLEN, MPI_INT, buf2, BUFLEN, MPI_INT, MPI_COMM_WORLD);
  t_end  = MPI_Wtime();


  if (myid == 0) {
    for (i = 0; i < BUFLEN * MAX_PROCS; i++)
      printf("%d/%d -> %d   ", myid, i, buf2[i]);
    printf("\n");
    printf("Rank %d (Time: %.4f)\n\n", myid, t_end - t_start);
  }

  MPI_Barrier(MPI_COMM_WORLD);

  free (buffer);
  free (buf2);

  MPI_Finalize();
  return (0);
}
