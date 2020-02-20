#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"


int main (int argc, char **argv) {

    int errs = 0;
    int rank, size;
    int minsize = 2, count; 
    MPI_Comm      comm;
    int *sendbuf, *recvbuf, *p;
    int sendcount, recvcount;
    int i, j;
    MPI_Datatype sendtype, recvtype;
    int k;
    double t_end, t_start;


    /* The following illustrates the use of the routines to 
       run through a selection of communicators and datatypes.
       Use subsets of these for tests that do not involve combinations 
       of communicators, datatypes, and counts of datatypes */
    //while (MTestGetIntracommGeneral( &comm, minsize, 1 )) {

    t_start = MPI_Wtime();
    comm = MPI_COMM_WORLD;
    for (k = 0; k < 100; k++) {
	if (comm == MPI_COMM_NULL) continue;

	/* Determine the sender and receiver */
	MPI_Comm_rank( comm, &rank );
	MPI_Comm_size( comm, &size );
	
	/* printf( "Size of comm = %d\n", size ); */
	//for (count = 1; count < 65000; count = count * 2) {
        for (count = 1; count < 9000; count = count * 2) {
	    
	    /* Create a send buf and a receive buf suitable for testing
	       all to all.  */
	    sendcount = count;
	    recvcount = count;
	    sendbuf   = (int *)malloc( count * size * sizeof(int) );
	    recvbuf   = (int *)malloc( count * size * sizeof(int) );
	    sendtype  = MPI_INT;
	    recvtype  = MPI_INT;

	    if (!sendbuf || !recvbuf) {
		errs++;
		fprintf( stderr, "Failed to allocate sendbuf and/or recvbuf\n" );
		MPI_Abort( MPI_COMM_WORLD, 1 );
	    }
	    for (i=0; i<count*size; i++) 
		recvbuf[i] = -1;
	    p = sendbuf;
	    for (j=0; j<size; j++) {
		for (i=0; i<count; i++) {
		    *p++ = j * size + rank + i;
		}
	    }
	    
	    MPI_Alltoall( sendbuf, sendcount, sendtype,
			  recvbuf, recvcount, recvtype, comm );
            if ((rank == 0) && (!(k % 10))) 
              printf("%d - MPI_Alltoall (0x%x, %d, 0x%x, 0x%x, %d, 0x%x, 0x%x)\n", k, sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm);
	    p = recvbuf;
	    for (j=0; j<size; j++) {
		for (i=0; i<count; i++) {
		    if (*p != rank * size + j + i) {
			errs++;
			if (errs < 10) {
			    fprintf( stderr, "Error with communicator %s and count %d\n", 
				     /*MTestGetIntracommName()*/"COMM", count );
			    fprintf( stderr, "recvbuf[%d,%d] = %d, should %d\n",
				     j,i, *p, rank * size + j + i );
			}
		    }
		    p++;
		}
	    }
	    free( recvbuf );
	    free( sendbuf );
	}
	//MTestFreeComm( &comm );
    }
    MPI_Barrier(MPI_COMM_WORLD);
    t_end = MPI_Wtime();
    if (rank == 0) printf("Total time: %lf\n", t_end - t_start);

    //MTest_Finalize( errs );
    MPI_Finalize();
    return 0;
}
