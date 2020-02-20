/*
 *FILE: head-to-head.c
 *DATE: Created on: 2013-3-31
 *Author: Jerry.F
 *
 *Copyright Jerry.F, all rights reserved.
 */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <mpi.h>
void main(int argc, char**argv)
{
	int i=0;
	MPI_Status stat;
        MPI_Request req,rreq;
	int x, y, myrank,flag;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	if (myrank==0) {
	  x = 10;
	  MPI_Isend(&x, 1, MPI_INT, 0, 99, MPI_COMM_WORLD,&req);
          MPI_Irecv(&y,1,MPI_INT,0,99,MPI_COMM_WORLD,&rreq);
          MPI_Wait( &req, &stat );          

          /*MPI_Isend(&x, 1, MPI_INT, 0, 99, MPI_COMM_WORLD,&req);
          MPI_Irecv(&y,1,MPI_INT,0,99,MPI_COMM_WORLD,&rreq);
           MPI_Wait( &req, &stat );*/
	  }
	 else if (myrank==1) {
	 
          /*MPI_Isend(&x, 1, MPI_INT, 0, 99, MPI_COMM_WORLD,&req);
          MPI_Irecv(&y,1,MPI_INT,0,99,MPI_COMM_WORLD,&rreq);
          MPI_Wait( &req, &stat ); */

          MPI_Isend(&x, 1, MPI_INT, 0, 99, MPI_COMM_WORLD,&req);
          MPI_Irecv(&y,1,MPI_INT,0,99,MPI_COMM_WORLD,&rreq);
          MPI_Wait( &req, &stat );

         }
	MPI_Finalize();
}

