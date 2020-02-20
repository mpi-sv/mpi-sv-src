#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "mpi.h"

int main (int argc, char **argv) {
  
  
  int     myid;
  
  
  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
  

  MPI_Finalize();
  
  return MPI_SUCCESS;
}
