#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"


#define MAX_PROCS   32

int main (int argc, char **argv) {

  int      *sbuf, *rbuf;
  int      rank, size;
  int      *sendcounts, *recvcounts, *rdispls, *sdispls;
  int      i, j, *p, err;

  MPI_Init(NULL, NULL);
  err = 0;

  /* Create the buffer */
  MPI_Comm_size(MPI_COMM_WORLD, &size );
  MPI_Comm_rank(MPI_COMM_WORLD, &rank );
  sbuf = (int *)malloc( size * size * sizeof(int) );
  rbuf = (int *)malloc( size * size * sizeof(int) );
  if (!sbuf || !rbuf) {
    printf("Could not allocated buffers!\n" );
	  MPI_Abort(MPI_COMM_WORLD, 1 );
  }

  /* Load up the buffers */
  for (i=0; i<size*size; i++) {
	  sbuf[i] = i + 100*rank;
	  rbuf[i] = -i;
  }

  /* Create and load the arguments to alltoallv */
  sendcounts = (int *)malloc( size * sizeof(int) );
  recvcounts = (int *)malloc( size * sizeof(int) );
  rdispls    = (int *)malloc( size * sizeof(int) );
  sdispls    = (int *)malloc( size * sizeof(int) );
  if (!sendcounts || !recvcounts || !rdispls || !sdispls) {
	  printf("Could not allocate arg items!\n" );
	  MPI_Abort(MPI_COMM_WORLD, 1 );
  }

  for (i = 0; i < size; i++) {
	  sendcounts[i] = i;
	  recvcounts[i] = rank;
	  rdispls[i]    = i * rank;
	  sdispls[i]    = (i * (i+1))/2;
  }
 
  if (rank == 0) {
  fprintf(stdout, "\nsbuf:  ");
  for (i = 0; i < size; i++) {
    fprintf(stdout, "%d  ", sbuf[i]);
  }
  fprintf(stdout, "\nsdispls:  ");
  for (i = 0; i < size; i++) {
    fprintf(stdout, "%d  ", sdispls[i]);
  }
  fprintf(stdout, "\nsendcounts:  ");
  for (i = 0; i < size; i++) {
    fprintf(stdout, "%d  ", sendcounts[i]);
  }
  }

  MPI_Alltoallv(sbuf, sendcounts, sdispls, MPI_INT,
                rbuf, recvcounts, rdispls, MPI_INT, MPI_COMM_WORLD);

  /* Check rbuf */
  for (i=0; i<size; i++) {
	  p = rbuf + rdispls[i];
	  for (j=0; j<rank; j++) {
	    if (p[j] != i * 100 + (rank*(rank+1))/2 + j) {
	      fprintf( stderr, "[%d] got %d expected %d for %dth\n",
                         rank, p[j],(i*(i+1))/2 + j, j );
        err++;
	    }
	  }
  }

  free( sdispls );
  free( rdispls );
  free( recvcounts );
  free( sendcounts );
  free( rbuf );
  free( sbuf );

  printf("Process %d got %d errors\n", rank, err);

  MPI_Finalize();
  return(0);
}
