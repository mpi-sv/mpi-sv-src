#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

#define  MAX_ITER       100
#define  MAX_PEND_REQ   2
#define  COUNT         (1024 * 4)


int main (int argc, char **argv) {

  int          myid, numprocs;
  int         *buf_1[MAX_PEND_REQ], *buf_2[MAX_PEND_REQ];
  int          i, j, k, err = 0;
  MPI_Status   st;
  MPI_Request  req[MAX_PEND_REQ];
  double       t_start, t_end;
  int          supported;


  MPI_Init_thread(NULL, NULL, MPI_THREAD_FUNNELED, &supported);  
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  if (numprocs != 2) {
    fprintf(stdout, "This program must be run with 2 processes\n");
    MPI_Finalize();
    return 0;
  }
    
  for (j = 0; j < MAX_PEND_REQ; j++) {
    buf_1[j] = (int *) malloc (COUNT * sizeof(int));
    buf_2[j] = (int *) malloc (COUNT * sizeof(int));
  }
  
  t_start = MPI_Wtime();
  
  for (k = 0; k < MAX_ITER; k++) {

    if (myid == 0) {
	  
	  for (j = 0; j < MAX_PEND_REQ; j++) {
        MPI_Isend(buf_1[j], COUNT, MPI_INT, 1, 77 + j, MPI_COMM_WORLD, &req[j]);
	  }
	  
      //for (j = 0; j < MAX_PEND_REQ; j++) {
		//MPI_Wait(&req[j], &st);
	  //}
	  MPI_Waitall(MAX_PEND_REQ, req, MPI_STATUSES_IGNORE);

    } else /* (myid == 1) */ {

	  for (j = 0; j < MAX_PEND_REQ; j++) {
        MPI_Irecv(buf_2[j], COUNT, MPI_INT, 0, 77 + j, MPI_COMM_WORLD, &req[j]);
	  }

	  for (j = 0; j < MAX_PEND_REQ; j++) {
		MPI_Wait(&req[j], &st);
		//MPI_Wait(&req[j], MPI_STATUS_IGNORE);
	  }
	  
    }
	
	if (myid == 0) { if (!(k % (MAX_ITER / 10))) { fprintf(stdout, "."); fflush(stdout); } }
	
  }
  
  t_end = MPI_Wtime();

  if (myid == 0) printf("\nTIME: %.3lf\nProcess %d has errors: %d \n", t_end - t_start, myid, err);

  for (j = 0; j < MAX_PEND_REQ; j++) {
    free(buf_1[j]);
    free(buf_2[j]);
  }
  
  MPI_Finalize();
  return(0);
}

