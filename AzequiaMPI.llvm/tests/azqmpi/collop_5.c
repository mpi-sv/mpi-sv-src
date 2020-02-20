#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

/* N is the number of processes */
#define  N   16
#define  M   4

int main (int argc, char **argv) {

  int        myid, size;
  float      a[M], b[M][N];
  float      c[N];
  float      sum[N];
  int        i, j;
  double     t_start, t_end;


  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  for (i = 0; i < M; i++) {
    a[i]    = (float)myid;
    for (j = 0; j < N; j++)
	    b[i][j] = (float)myid * j;
  }

  /* local operation */
  for (j = 0; j < N; j++) {
    sum[j] = 0.0;
    c[j]   = 0.0;
    for (i = 0; i < N; i++)
      sum[j] = sum[j] + (a[i] * b[i][j]);
  }

  /* global operation */
  t_start = MPI_Wtime();
  MPI_Allreduce(sum, c, N, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
  t_end = MPI_Wtime();

  /* Check to see that we got the right answers */
  if (myid == 3) {
    printf("(time: %.4f) Results: \n", t_end - t_start);
    for (j = 0; j < N; j++)
      printf("%.2f  ", c[j]);
  }

  MPI_Finalize();
  return MPI_SUCCESS;
}
