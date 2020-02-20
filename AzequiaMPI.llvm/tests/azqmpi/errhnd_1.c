#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

void handler_a   (MPI_Comm *comm, int *err);

static int a_errors;


int main (int argc, char **argv) {

  char            errstring[MPI_MAX_ERROR_STRING];
  MPI_Comm        dup_comm_world, dummy;
  MPI_Errhandler  errhandler_a, errhandler_b, errhandler, old_handler;
  int             err, world_rank, resultlen;


  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_dup(MPI_COMM_WORLD, &dup_comm_world);

  a_errors = 0;

  if (world_rank % 2) {
    MPI_Errhandler_create((MPI_Handler_function *)handler_a, &errhandler_a);
    MPI_Errhandler_set(dup_comm_world, errhandler_a);
    MPI_Errhandler_free(&errhandler_a);

    /* Generate an exception */
    MPI_Comm_create(dup_comm_world, MPI_GROUP_NULL, &dummy);
    if (a_errors == 0) {
	    printf("    error handler A not invoked\n");
    }
  }

  MPI_Comm_free(&dup_comm_world);

  if (world_rank == 0) printf("Errors: %d\n", a_errors);

  MPI_Finalize();
  return MPI_SUCCESS;
}


/* User error handler function */
void handler_a (MPI_Comm *comm, int *err) {

  int errclass;
  int rank;

  MPI_Error_class(*err, &errclass);
  MPI_Comm_rank(*comm, &rank);

  if (errclass != MPI_ERR_GROUP) {
	  printf( "handler_a: incorrect error class %d\n", errclass );
  }

  printf("handler_a: Error handler invoked by %d\n", rank);

  *err = MPI_SUCCESS;
  a_errors++;
}

