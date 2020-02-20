#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

#define TABLE_SIZE   256

struct float_int {
  float a;
  int   b;
};

typedef struct float_int float_int;


int main (int argc, char **argv) {

  int        rank, size;
  float_int  in[TABLE_SIZE], out[TABLE_SIZE];
  int        i;
  int        errors = 0, toterrors;
  double     t_start, t_end;


  MPI_Init(NULL, NULL);

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  for (i = 0; i < TABLE_SIZE; i++) {
    in[i].a = (float)-rank;
    in[i].b = rank;
  }

  /* MAXLOC operation */
  MPI_Allreduce(in, out, TABLE_SIZE, MPI_FLOAT_INT, MPI_MAXLOC, MPI_COMM_WORLD);

  /* Check to see that we got the right answers */
  for (i = 0; i < TABLE_SIZE; i++)
    if (out[i].b != 0) {
      printf("MAX ranks[%d] = %d != %d\n", i, out[i].b, rank);
      errors++;
    }

  for (i = 0; i < TABLE_SIZE; i++)  {
	  in[i].a = (float)rank;
	  in[i].b = rank;
  }


  /* MINLOC operation */
  t_start = MPI_Wtime();

  for (i = 0; i < 1000; i++) {
    MPI_Allreduce(in, out, TABLE_SIZE, MPI_FLOAT_INT, MPI_MINLOC, MPI_COMM_WORLD);
  }

  t_end = MPI_Wtime();

  for (i = 0; i < TABLE_SIZE; i++)
    if (out[i].b != 0) {
      printf("MIN ranks[%d] = %d != %d\n", i, out[i].b, rank );
      errors++;
    }

  if (rank == 0) printf("%d errors  (time: %.4f)\n", errors, t_end - t_start);

  MPI_Finalize();

  return errors;
}
