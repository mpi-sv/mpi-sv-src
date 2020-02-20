#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

volatile int cuantos = 0;

int main (int argc, char **argv) {

  int        numprocs;
  int        i;
  int        randval;
  int        errs = 0;
  MPI_Comm   comm, scomm;
  int        rank, size, color, srank;
  int        err;


  __sync_fetch_and_add(&cuantos, 1);
  if (cuantos == 256) fprintf(stdout, "%d\n", cuantos);
  sleep(2);
  
  MPI_Init(NULL, NULL);

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Barrier(MPI_COMM_WORLD);
  __sync_fetch_and_add(&cuantos, 1);
  MPI_Barrier(MPI_COMM_WORLD);
  if (rank == 0) fprintf(stdout, "%d\n", cuantos);

/*
  MPI_Comm_dup(MPI_COMM_WORLD, &comm);
  MPI_Comm_rank(comm, &rank);
  MPI_Comm_size(comm, &size);
  
  if (size < 4) {
	  fprintf(stderr, "This test requires at least four processes.");
	  MPI_Abort(MPI_COMM_WORLD, 1);
  }

  for (i = 0; i < 10; i++) {
  color = MPI_UNDEFINED;
  if (rank < 2) color = 1;
  if (0 > (err = MPI_Comm_split(comm, color, size - rank, &scomm))) {
	fprintf(stdout, "Error en MPI_Comm_split: %d\n", err);
	MPI_Abort(MPI_COMM_WORLD, 0);
  }

  if (rank < 2) {

	MPI_Comm_rank(scomm, &srank);
	  if (srank != 1 - rank) {
	    errs++;
	  }
	  MPI_Comm_free(&scomm);
  } else {
	  if (scomm != MPI_COMM_NULL) {
	    errs++;
	  }
  }
 
  }

  MPI_Comm_free(&comm);
*/  
  if (rank == 0) {
	fprintf(stdout, "All OK\n");
  }

  if (errs != 0) {
    printf("Process %d detected %d errors\n", rank, errs);
    sleep(1);
  }

  MPI_Finalize();
  return MPI_SUCCESS;
}
