#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
#endif


#include "mpi.h"


#define MAX_PROCS   32


void user_fxn (void *invec, void *inoutvec, int *len, MPI_Datatype *datatype) {

	int    i;
  float *a = (float *)invec;
	float *b = (float *)inoutvec;

	for (i = 0; i < *len; i++)  b[i] = (a[i] > b[i]) ? a[i] : b[i];

}



int main (int argc, char **argv) {

  float   *sbuf, *rbuf;
  int      rank, size;
  int      i;
  double   t_start, t_end;
  MPI_Op   myop;


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  sbuf = (float *) malloc(size * sizeof(float));
  for (i = 0; i < size; i++)
	  sbuf[i] = rank + 1;

  rbuf = (float *) malloc(size * sizeof(float));

  MPI_Op_create(user_fxn, 1, &myop);
  t_start = MPI_Wtime();
  for (i = 0; i < 100; i++)
    MPI_Scan(sbuf, rbuf, size, MPI_FLOAT, myop, MPI_COMM_WORLD);
    //MPI_Allreduce(sbuf, rbuf, size, MPI_FLOAT, myop, MPI_COMM_WORLD);
  t_end = MPI_Wtime();
  MPI_Op_free(&myop);

  printf("\n*** Process %d  (time: %.4f)\n", rank, t_end - t_start);
  for (i = 0; i < size; i++)
    printf("%.2f  ", rbuf[i]);

  MPI_Finalize();
  return MPI_SUCCESS;
}
