#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

#define DIMS  3

#define TRUE  1
#define FALSE 0

int main (int argc, char **argv) {

  int        dims[DIMS]    = {2,3,2};
  int        periods[DIMS] = {TRUE, TRUE, TRUE};
  int        remain[DIMS];
	int        coords[DIMS];
	int        size, myid;
  MPI_Comm   cart_comm, newcomm;
	int        ndims;
	int        i;
	int        myid_sub;
	MPI_Status status;


  MPI_Init(NULL, NULL);

  /* First, create a DIMS cartesian communicator */
  MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  MPI_Cart_create(MPI_COMM_WORLD, DIMS, dims, periods, TRUE, &cart_comm);

	if (cart_comm == MPI_COMM_NULL) {
    fprintf(stdout, "Process %d EXITING\n", myid);
	  goto end;
	}

	fprintf(stdout, "Process %d INVOKING CART_SUB\n", myid);

  /* Create 2 3x2 subgrids from a 2x3x2 grid
  remain[0] = FALSE;
	remain[1] = TRUE;
	remain[2] = TRUE; */
  /* Create 3 2x2 subgrids from a 2x3x2 grid */
  remain[0] = TRUE;
	remain[1] = FALSE;
	remain[2] = TRUE;

  MPI_Cart_sub(cart_comm, remain, &newcomm);

	fprintf(stdout, "Process %d MPI_COMM_WORLD\n", myid);

  MPI_Comm_size(newcomm, &size);
  MPI_Comm_rank(newcomm, &myid_sub);
	MPI_Cart_coords(newcomm, myid, DIMS, coords);
	fprintf(stdout, "Process %d Communicator 0x%x\n", myid_sub, newcomm);
  fprintf(stdout, "\tCoords: \n");
	fprintf(stdout, "\t[%d,", coords[0]);
	fprintf(stdout, "\t %d,", coords[1]);
	fprintf(stdout, "\t %d]\n", coords[2]);

	if (myid_sub == 0) {
	  MPI_Send(&size, 1, MPI_INT, 1, 99, newcomm);
	  fprintf(stdout, "SENDED process %d MPI_COMM_WORLD %d\n", myid_sub, myid);
  } else if (myid_sub == 1) {
	  MPI_Recv(&size, 1, MPI_INT, MPI_ANY_SOURCE, 99, newcomm, &status);
    fprintf(stdout, "RECEIVED process %d MPI_COMM_WORLD %d\n", myid_sub, myid);
	}

  /*MPI_Cartdim_get(newcomm, &ndims);
	fprintf(stdout, "Process %d. La informacion de CART es:\n\tNDIMS: %d ", myid, ndims);
	MPI_Cart_get(newcomm, ndims, dims, periods, coords);
	fprintf(stdout, "DIMS: ");
	for (i = 0; i < ndims; i++)
	  fprintf(stdout, "%d ", dims[i]);
  fprintf(stdout, "PERIODS: ");
	for (i = 0; i < ndims; i++)
	  fprintf(stdout, "%c ", (periods[i] == 1) ? 'T' : 'F');
  */

  MPI_Comm_free(&newcomm);
  MPI_Comm_free(&cart_comm);

end:
  MPI_Finalize();
  return MPI_SUCCESS;
}

