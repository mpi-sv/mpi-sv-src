#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

#define ITER             (1024 * 256)
#define NUM_RQST_MAX     32
#define BUFF_SIZE_MAX    (1024 * 128 * 8)   /* Number of components of integer buffers */
#define CACHE_LINE_SIZE  64

static inline void message(int i)
{
  if(i == ITER/2)
    printf(" 1/2 ");
  else if(i == ITER/4)
    printf(" 1/4 ");
  else if(i == (3*ITER)/4)
    printf(" 3/4 ");
  else if(i% (ITER/100) == 0) {printf("."); fflush(stdout);}
  return;
}

static int size2itera(int size, int nProcs)
{
  int        num_iter;

  if     (size <= 128)        num_iter = ( 1024 * 64 * 3);
  else if(size <= 1024 * 16)  num_iter = ( 1024 * 64 * 4);
  else if(size <= 1024 * 128) num_iter = ( 1024 * 8 *  4);
  else 	                      num_iter = size / 128;
  
//printf("%d iter\n", num_iter); fflush(stdout);
  return (3*num_iter) / nProcs;
}


int main (int argc, char **argv)  
{
  int          numprocs, myid;
  char        *buf;
  char        *bufrecv[NUM_RQST_MAX];
  MPI_Request  req    [NUM_RQST_MAX];
  MPI_Status   status;
  MPI_Status   stat[NUM_RQST_MAX];
  int          i, j, k, bufSize; 
  int          count;
  int          nrIter;
  double       t_start, t_end, latency, bandwidth;

  MPI_Init (NULL, NULL);
  MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
  if(numprocs > NUM_RQST_MAX) {
    printf("This program admits up to NUM_RQST_MAX ranks\n"); fflush(stdout);
    exit(1);
  }
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);

  if (myid == 0) {
    fprintf(stdout, "%20s\t%20.18s\t%20.18s\n", "Msg Size (bytes)", "Latency (us)", "Bandwidth (MB/s)"); fflush(stdout);
    fprintf(stdout, "%20s\t%20.18s\t%20.18s\n", "----------------", "------------", "----------------"); fflush(stdout);
  }

  for(bufSize = 0; bufSize <= BUFF_SIZE_MAX; bufSize <<= 1) {
   for(k = 0; k < 2; k++) {
    if(k == 1) bufSize += bufSize/2;
    /*
    if (myid == 0) {
      fprintf(stdout, "Size %d\n", bufSize); 
      fprintf(stdout, "%s\n", "----------------"); fflush(stdout);
    }
    */
    nrIter = size2itera(bufSize, numprocs);
    //nrIter = 1;

    if ((myid > 0) && (myid <= NUM_RQST_MAX)) {
      posix_memalign((void *)&buf, CACHE_LINE_SIZE, bufSize * sizeof(char));
      //if((bufSize) >= sizeof(int)) 
      //  buf[0] = myid;
      for(i = 0; i < nrIter; i++) {
        //message(i);
        MPI_Send (buf, bufSize, MPI_CHAR, 0, 36, MPI_COMM_WORLD);
        /*
        if (myid % 2) {
          MPI_Send (buf, bufSize, MPI_CHAR, 0, 36, MPI_COMM_WORLD);
        } else {
          MPI_Isend (buf, bufSize, MPI_CHAR, 0, 36, MPI_COMM_WORLD, &req[myid]);
          MPI_Wait(&req[myid], MPI_STATUS_IGNORE);
        }
        */
      }
      free(buf);
    }

    else if (myid == 0) {
      for(j = 1; j < numprocs; j++) 
        if(posix_memalign((void *)&bufrecv[j], CACHE_LINE_SIZE, bufSize * sizeof(char)))             goto exception;

      t_start = MPI_Wtime();
      for(i = 0; i < nrIter; i++) {
        /* versión 1
        for(j = 1; j < numprocs; j++) {
          MPI_Probe(j             , MPI_ANY_TAG, MPI_COMM_WORLD, &status);
          MPI_Recv(bufrecv[j], count, MPI_CHAR, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
        }
        */
        /* versión 2 */
        for(j = 1; j < numprocs; j++) 
          MPI_Probe(j             , MPI_ANY_TAG, MPI_COMM_WORLD, &stat[j]);
        for(j = 1; j < numprocs; j++) 
          MPI_Recv(bufrecv[j], count, MPI_CHAR, stat[j].MPI_SOURCE, stat[j].MPI_TAG, MPI_COMM_WORLD, &status);
      }
      t_end = MPI_Wtime() - t_start;

      for(j = 1; j < numprocs; j++)  
        free(bufrecv[j]);
      
      latency = ((t_end ) / (nrIter * (numprocs - 1))) * 1000000; 
      bandwidth = bufSize / latency;
      fprintf(stdout, "%20d\t%20.9f\t%20.9f\n", bufSize, latency, bandwidth); fflush(stdout);
      
    }
    if(k == 1) bufSize -= bufSize/3;
    if(bufSize == 0) bufSize = 1;
   }
  }
  MPI_Finalize ();
  return 0;

exception:
  printf("Exception in rank %d \n", myid); fflush(stdout);
  MPI_Finalize ();
  return 0;
}



