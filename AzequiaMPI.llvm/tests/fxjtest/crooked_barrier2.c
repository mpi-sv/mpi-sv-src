#include "mpi.h"
#include <stdio.h>
#include <math.h>

int main(int argc,char *argv[])
{
    int  myid, numprocs, i;
    int  namelen;
    int buf0;
    int buf1;
    MPI_Request req;
   MPI_Request req1;
    MPI_Status status;
    char processor_name[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);
    MPI_Get_processor_name(processor_name,&namelen);


        if (myid == 0)
        {
        buf0=0;
	  //MPI_Isend (&buf0, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, &req1);
        MPI_Isend (&buf0, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, &req);
        MPI_Barrier (MPI_COMM_WORLD);	  
        MPI_Wait (&req, &status);
        }
        else if (myid == 1)
        {
	    buf1=3;
         // MPI_Irecv (&buf1, 1, MPI_INT,  0, 1, MPI_COMM_WORLD, &req1);
          MPI_Irecv (&buf1, 1, MPI_INT,  MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &req);
	    MPI_Barrier (MPI_COMM_WORLD);	   
          MPI_Wait (&req, &status);
           
	    printf("received :%d\n",buf1); 
         
        }

        else   if (myid == 2)
	    {
               buf0=1;
            
            MPI_Barrier (MPI_COMM_WORLD);	  
            //MPI_Isend (&buf0, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, &req); 
            //MPI_Wait (&req, &status);            

	    }

    MPI_Finalize();
    return 0;
}


