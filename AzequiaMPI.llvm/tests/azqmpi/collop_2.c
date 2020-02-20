#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

#define TABLE_SIZE   1024

struct int_int {
  int   a;
  int   b;
};

typedef struct int_int int_int;


int main (int argc, char **argv) {

  int      rank, size;
  int_int  in[TABLE_SIZE], out[TABLE_SIZE];
  int      i;
  int      errors = 0, toterrors;
  int      root;
  double   t_start, t_end;


  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  for (i = 0; i < TABLE_SIZE; i++) {
    in[i].a = -rank;
    in[i].b = rank;

    out[i].a = 999;
    out[i].b = -999;
  }

  /* MAXLOC operation */
  root = 0;
  MPI_Reduce(in, out, TABLE_SIZE, MPI_2INT, MPI_MAXLOC, root, MPI_COMM_WORLD);

  /* Check to see that we got the right answers */
  if (rank == root) {
    for (i = 0; i < TABLE_SIZE; i++)
      if (out[i].b != root) {
        printf("MAX ranks[%d] = %d != %d\n", i, out[i].b, rank);
		    errors++;
      }
  }

  for (i = 0; i < TABLE_SIZE; i++)  {
	  in[i].a = rank;
	  in[i].b = rank;
  }


  /* MINLOC operation */
  t_start = MPI_Wtime();

  root = 2;
  for (i = 0; i < 1000; i++)
    MPI_Reduce(in, out, TABLE_SIZE, MPI_2INT, MPI_MINLOC, root, MPI_COMM_WORLD);

  t_end = MPI_Wtime();

  if (rank == root) {
    for (i = 0; i < TABLE_SIZE; i++)
      if (out[i].b != 0) {
        printf("MIN ranks[%d] = %d != %d\n", i, out[i].b, rank );
		    errors++;
      }
  }

  if (rank == root) printf("%d errors  (time: %.4f)\n", errors, t_end - t_start);

  MPI_Finalize();
  return errors;
}
