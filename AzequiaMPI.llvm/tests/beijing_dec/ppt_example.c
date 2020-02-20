/* -*- Mode: C; -*- */
/* Creator: Bronis R. de Supinski (bronis@llnl.gov) Tue Sep 30 2003 */
/* any_src-can-deadlock10.c -- deadlock occurs if task 0 receives */
/*                             from task 1 first; sleeps generally */
/*                             make order 1 before 2 with all task */
/*                             0 ops being posted before both 1 and 2 */


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
  MPI_Status status1,status2;
  MPI_Request req1,req2;

  /* init */
  MPI_Init (&argc, &argv);
  MPI_Comm_size (MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name (processor_name, &namelen);
  printf ("(%d) is alive on %s\n", rank, processor_name);
  fflush (stdout);

  MPI_Barrier (MPI_COMM_WORLD);

  if (nprocs < 4)
    {
      printf ("not enough tasks\n");
      exit(1);
    }
   else if(argc!=2){
       printf("not enough args\n");
       exit(1);
    }
  else if (rank == 0)
    {
      
        if(argv[1][0]=='a')
              MPI_Irecv (buf0, buf_size, MPI_INT, 1, 0, MPI_COMM_WORLD, &req1);
          else
                  
		MPI_Irecv (buf0, buf_size, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &req1);
      MPI_Recv (buf1, buf_size, MPI_INT, 3, 0, MPI_COMM_WORLD, &status1);

      MPI_Isend (buf1, buf_size, MPI_INT, 3, 0, MPI_COMM_WORLD,&req2);


      MPI_Wait (&req1, &status1);
      MPI_Wait (&req2, &status2);
    }
 else if (rank == 1)
    {
      memset (buf1, 1, buf_size);

     MPI_Isend (buf1, buf_size, MPI_INT, 0, 0, MPI_COMM_WORLD,&req1);
    }
  
   else if (rank == 2)
    {
      memset (buf0, 1, buf_size);
  
      MPI_Isend (buf1, buf_size, MPI_INT, 0, 0, MPI_COMM_WORLD,&req1);

      
    }
  else if (rank == 3)
    {
      memset (buf0, 0, buf_size);
  
      MPI_Isend (buf1, buf_size, MPI_INT, 0, 0, MPI_COMM_WORLD,&req1);

      MPI_Recv (buf1, buf_size, MPI_INT, 0, 0, MPI_COMM_WORLD, &status1);

      MPI_Wait (&req1, &status2);
    }

  MPI_Finalize ();
  printf ("(%d) Finished normally\n", rank);
}

/* EOF */
