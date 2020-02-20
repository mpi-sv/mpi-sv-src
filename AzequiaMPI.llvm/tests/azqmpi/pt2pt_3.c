#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"


#define  MAX_PEND_REQ            4
#define  BUFF_SIZE_MAX   (1024 * 1024)


static int size2itera(int size, int nProcs)
{
  int        num_iter;

  if     (size <= 128)        num_iter = ( 1024 * 128 * 4) / nProcs;
  else if(size <= 1024 * 16)  num_iter = ( 1024 * 128 * 4) / nProcs;
  else if(size <= 1024 * 128) num_iter = ( 1024 * 16 *  4) / nProcs;
  else 	                      num_iter = size / 128;
  
//printf("%d iter\n", num_iter); fflush(stdout);
  return num_iter;
}

int main (int argc, char **argv) 
{
  int          myid, numprocs;
  char        *buf_1[MAX_PEND_REQ], *buf_2[MAX_PEND_REQ];
  int          j, k, err = 0, bufSize, nrIter;
  MPI_Status   st;
  MPI_Request  req[MAX_PEND_REQ];
  int          supported;
  double       t_start, t_end, latency, bandwidth;

  MPI_Init_thread(NULL, NULL, MPI_THREAD_FUNNELED, &supported);  
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  if (numprocs != 2) {
    fprintf(stdout, "This program must be run with 2 processes\n");
    MPI_Finalize();
    return 0;
  }

  if (myid == 0) {
    fprintf(stdout, "%20s\t%20.18s\t%20.18s\n", "Msg Size (bytes)", "Latency (us)", "Bandwidth (MB/s)"); fflush(stdout);
    fprintf(stdout, "%20s\t%20.18s\t%20.18s\n", "----------------", "------------", "----------------"); fflush(stdout);
  }
    
  for (j = 0; j < MAX_PEND_REQ; j++) {
    buf_1[j] = (char *) malloc (BUFF_SIZE_MAX * sizeof(char));
    buf_2[j] = (char *) malloc (BUFF_SIZE_MAX * sizeof(char));
  }

  for(bufSize = 1; bufSize <= BUFF_SIZE_MAX; bufSize <<= 1) {
//for(bufSize = 1; bufSize <= 1; bufSize <<= 1) {
  //printf("bufSize = %d\n", bufSize);
    nrIter = size2itera(bufSize, numprocs);
  //nrIter = 10;
    t_start = MPI_Wtime();
    for (k = 0; k < nrIter; k++) {
      if (myid == 0) {
        for (j = 0; j < MAX_PEND_REQ; j++) {
        //MPI_Isend(buf_1[j], bufSize, MPI_CHAR, 1, 77 + j, MPI_COMM_WORLD, &req[j]);
          MPI_Send (buf_1[j], bufSize, MPI_CHAR, 1, 77 + j, MPI_COMM_WORLD);
        }
        //for (j = 0; j < MAX_PEND_REQ; j++) 
        //  MPI_Wait(&req[j], &st);
        //MPI_Waitall(MAX_PEND_REQ, req, MPI_STATUSES_IGNORE);
      } 
      else /* (myid == 1) */ {
        //printf("\n");
        //usleep(1000);
        for (j = 0; j < MAX_PEND_REQ; j++) {
          MPI_Irecv(buf_2[j], bufSize, MPI_CHAR, 0, 77 + j, MPI_COMM_WORLD, &req[j]);
        //MPI_Recv (buf_2[j], bufSize, MPI_CHAR, 0, 77 + j, MPI_COMM_WORLD, &st);
        }
        
        for (j = 0; j < MAX_PEND_REQ; j++) {
          MPI_Wait(&req[j], &st);
        //MPI_Wait(&req[j], MPI_STATUS_IGNORE);
        }
        
        //MPI_Waitall(MAX_PEND_REQ, req, MPI_STATUSES_IGNORE);	  
      }
      //if (myid == 0) { if (!(k % (MAX_ITER / 10))) { fprintf(stdout, "."); fflush(stdout); } }
    }
    t_end = MPI_Wtime();

    if (myid == 0) {
      latency = ((t_end - t_start) / nrIter) * 1000000;  
      latency /= MAX_PEND_REQ; 
      bandwidth = bufSize / latency;
      fprintf(stdout, "%20d\t%20.9f\t%20.9f\n", bufSize, latency, bandwidth); fflush(stdout);
    }
    //if (myid == 0) printf("\nTIME: %.3lf\nProcess %d has errors: %d \n", t_end - t_start, myid, err);
  }

  for (j = 0; j < MAX_PEND_REQ; j++) {
    free(buf_1[j]);
    free(buf_2[j]);
  }
  
  MPI_Finalize();
  return(0);
}

