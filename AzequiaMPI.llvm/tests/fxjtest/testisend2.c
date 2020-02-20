#include <stdio.h>
#include <string.h>
#include "mpi.h"

#define buf_size 8

int
main (int argc, char **argv)
{
  int nprocs = -1;
  int rank = -1;
  char processor_name[128];
  int namelen = 128;
  int i=0;
  char buf0[buf_size];
  char buf1[buf_size];
  memset(buf0, 0, 8);
  memset(buf1, '1', 8);
  buf0[7]=buf1[7]='\n';

  MPI_Status statuses;
  

  /* init */
  MPI_Init (&argc, &argv);
  MPI_Comm_size (MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name (processor_name, &namelen);
  printf ("(%d) is alive on %s\n", rank, processor_name);
  fflush (stdout);

  if (rank == 0)
    {
      MPI_Request req;
      MPI_Isend (buf1, buf_size, MPI_CHAR, 1, 1, MPI_COMM_WORLD,&req);
      //MPI_Recv (buf0, buf_size,MPI_CHAR,MPI_ANY_SOURCE,1,MPI_COMM_WORLD,&status);
      MPI_Wait(&req,&statuses);

      
    }
   else if(rank == 1){
     MPI_Request req;
    MPI_Irecv (buf0, buf_size, MPI_CHAR, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &req);      
   // printf("buff0[0]:%c\n", buf0[0]);
    MPI_Wait (&req,&statuses);

    printf ("content of buf0:\n");
    for(i=0;i<8;i++)
	printf("%c",buf0[i]);    
   }

  MPI_Finalize ();
  printf ("(%d) Finished normally\n", rank);
}

/* EOF */
