#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#define buf_size 128

int
main (int argc, char **argv)
{
  int nprocs = -1;
  int rank = -1;
  char processor_name[128];
  int namelen = 128;
  int buf0[buf_size];
  int buf1[buf_size];
  MPI_Status status;
  MPI_Request req[3];

  /* init */
  MPI_Init (&argc, &argv);
  MPI_Comm_size (MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);
 

  MPI_Barrier (MPI_COMM_WORLD);

if (rank == 0)
    {

      MPI_Recv (buf1, buf_size, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
      MPI_Recv (buf1, buf_size, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
      MPI_Recv (buf1, buf_size, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
    }
  else if (rank == 1)
    {
      memset (buf0, 0, buf_size);

      MPI_Isend (buf1, buf_size, MPI_INT, 0, 0, MPI_COMM_WORLD,&req[0]);
      MPI_Isend (buf1, buf_size, MPI_INT, 0, 0, MPI_COMM_WORLD,&req[1]);
      MPI_Isend (buf1, buf_size, MPI_INT, 0, 0, MPI_COMM_WORLD,&req[2]);
      //MPI_Wait (&req[2], &status);
    }
 
  //MPI_Barrier (MPI_COMM_WORLD);

  MPI_Finalize ();
  printf ("(%d) Finished normally\n", rank);
}

/* EOF */
