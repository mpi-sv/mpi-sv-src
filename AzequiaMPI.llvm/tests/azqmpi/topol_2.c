#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

#define NUM_DIMS 2
int verbose = 1;


int main (int argc, char **argv) {


  int       rank, size, i;
  int       dims[NUM_DIMS];
  int       periods[NUM_DIMS];
  int       new_coords[NUM_DIMS];
  int       new_new_coords[NUM_DIMS];
  int       reorder = 1;
  int       left, right, top, bottom;
  MPI_Comm  comm_cart;

  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (rank == 0) printf("For see the results, this program must be runned with 4 processes\n");

  /* Clear dims array and get dims for topology */
  for(i = 0; i < NUM_DIMS; i++) { dims[i] = 0; periods[i] = 0; }
  MPI_Dims_create (size, NUM_DIMS, dims);

  /* Make a new communicator with a topology */
  MPI_Cart_create (MPI_COMM_WORLD, 2, dims, periods, reorder, &comm_cart);

  /* Does the mapping from rank to coords work */
  MPI_Cart_coords (comm_cart, rank, NUM_DIMS, new_coords);

  /* 2nd call to Cart coords gives us an error - why? */
  MPI_Cart_coords (comm_cart, rank, NUM_DIMS, new_new_coords);

  /* Try cart shift */
  MPI_Cart_shift(comm_cart, 0, 1, &left, &right);
  MPI_Cart_shift(comm_cart, 1, 1, &bottom, &top);

  if (dims[0] == 2) {
	  /* We should see
	   [0] -1 2 -1 1
	   [1] -1 3 0 -1
	   [2] 0 -1 -1 3
	   [3] 1 -1 2 -1
    */
	  if (verbose) {
	    printf("[%d] final dims = [%d,%d]\n", rank, dims[0], dims[1]);
	    printf("[%d] left = %d, right = %d, bottom = %d, top = %d\n",
		    rank, left, right, bottom, top);
	  }
  }

  MPI_Comm_free(&comm_cart);

  MPI_Finalize();
  return MPI_SUCCESS;
}
