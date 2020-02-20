#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

void handler_a   (MPI_Comm *comm, int *err);

static int a_errors;


int main (int argc, char **argv) {

  char            errstring[MPI_MAX_ERROR_STRING];
  MPI_Errhandler  errhandler_a, errhandler_b, errhandler, old_handler;
  int             err, world_rank, resultlen;
  int             n;

  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  a_errors = 0;

  MPI_Errhandler_create((MPI_Handler_function *)handler_a, &errhandler_a);
  MPI_Errhandler_set(MPI_COMM_WORLD, errhandler_a);
  MPI_Errhandler_free(&errhandler_a);

  /* Generate an exception. LIBRARY MUST BE MODIFIED */
  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
 
  printf("[%d] Errors: %d\n", world_rank, a_errors);

  MPI_Finalize();
  return MPI_SUCCESS;
}


/* User error handler function */
void handler_a (MPI_Comm *comm, int *err) {

  int errclass;
  int rank;

  MPI_Error_class(*err, &errclass);
  MPI_Comm_rank(*comm, &rank);

  printf("[%d] handler_a:  error class %d\n", rank, errclass);

  *err = MPI_SUCCESS;
  a_errors++;
}

