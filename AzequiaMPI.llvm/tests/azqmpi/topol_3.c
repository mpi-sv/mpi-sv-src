#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

#define NUM_DIMS 2


int main (int argc, char **argv) {

  int    rank, size, i;
  int    errors = 0;
  int    dims[NUM_DIMS];
  int    periods[NUM_DIMS];
  int   *rbuf, *sbuf;
  int    new_rank;


  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  /* Clear dims array and get dims for topology */
  for(i = 0; i < NUM_DIMS; i++) { dims[i] = 0; periods[i] = 0; }

  MPI_Dims_create (size, NUM_DIMS, dims);

  /* Look at what rankings a cartesian topology MIGHT have */
  MPI_Cart_map (MPI_COMM_WORLD, 2, dims, periods, &new_rank);

  /* Check that all new ranks are used exactly once */
  rbuf = (int *)malloc(size * sizeof(int));
  sbuf = (int *)malloc(size * sizeof(int));
  if (!rbuf || !sbuf) {
	  MPI_Abort(MPI_COMM_WORLD, 1);
  }
  for (i=0; i<size; i++)
	  sbuf[i] = 0;
  sbuf[new_rank] = 1;

  MPI_Reduce(sbuf, rbuf, size, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

  if (rank == 0) {
	  for (i=0; i<size; i++) {
	    if (rbuf[i] != 1) {
		    errors++;
		    fprintf(stderr, "Rank %d used %d times\n", i, rbuf[i]);
	    }
	  }
	  if (errors == 0) printf("Cart map test passed\n");
  }

  free(rbuf);
  free(sbuf);

  MPI_Finalize();
  return MPI_SUCCESS;
}
