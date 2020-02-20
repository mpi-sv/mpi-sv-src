#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
#endif


#include "mpi.h"


#define NUM_RQST    5
#define MAX_ITER   10

#define ITER             (1024 * 256)
#define NUM_RQST_MAX     32
#define BUFF_SIZE_MAX    (1024 * 1024 )   
#define CACHE_LINE_SIZE  64


//#define TEST_COMMUNICATION



static int size2itera(int size, int nProcs)
{
  int        num_iter;

  if     (size <= 128)        num_iter = ( 1024 * 128 * 3);
  else if(size <= 1024 * 16)  num_iter = ( 1024 * 128 * 4);
  else if(size <= 1024 * 128) num_iter = ( 1024 * 16 *  6);
  else 	                      num_iter = size / 128;
  
//printf("%d iter\n", num_iter); fflush(stdout);

#ifdef TEST_COMMUNICATION
  return (2*num_iter) / nProcs;
#else
  return (10*num_iter) / nProcs;
#endif
}



int main (int argc, char **argv) 
{
  MPI_Request   request, request_1;
  char         *buf;
  MPI_Status    status;
  int           i, bufSize, k, nrIter;
  double        t_start, t_end, latency, bandwidth;
  int           numprocs, myid;

  MPI_Init (NULL, NULL);
  MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);

  if (myid == 0) {
    fprintf(stdout, "%20s\t%20.18s\t%20.18s\n", "Msg Size (bytes)", "Latency (us)", "Bandwidth (MB/s)"); fflush(stdout);
    fprintf(stdout, "%20s\t%20.18s\t%20.18s\n", "----------------", "------------", "----------------"); fflush(stdout);
  }
  if(posix_memalign((void *)&buf, CACHE_LINE_SIZE, 2*BUFF_SIZE_MAX * sizeof(char)))             goto exception;

  //fprintf(stdout, "Rank(%d) buf = %p, buf_2 = %p .....................................\n", myid, buf, buf_2); fflush(stdout);
  for(bufSize = 0; bufSize <= BUFF_SIZE_MAX; bufSize <<= 1) {
    for(k = 0; k < 2; k++) {
      if(k == 1) bufSize += bufSize/2;

      nrIter = size2itera(bufSize, numprocs);
      //nrIter = 1;

      if (myid == 1) {
        MPI_Send_init (buf, bufSize, MPI_BYTE, 0, 47, MPI_COMM_WORLD, &request);
        for (i = 0; i < nrIter; i++) {
#ifdef TEST_COMMUNICATION
          if(myid == 1) {
            int idx;
            if(bufSize >= 2*sizeof(int)) {
              for(idx = 1; idx < bufSize/sizeof(int); idx <<= 1/*idx++*/ ) {
                //fprintf(stdout, "%d ", bufSize + idx); 
                ((int *)buf)[idx] = bufSize + idx + i;
              }
              //fprintf(stdout, "\n"); fflush(stdout);
            }
          }
#endif
          switch(i%3) {
            case 0:
              MPI_Start(&request);
              MPI_Wait(&request, &status);
              break;
            case 1:
              MPI_Send(buf, bufSize, MPI_BYTE, 0, 47, MPI_COMM_WORLD);
              break;
            case 2:
              MPI_Isend(buf, bufSize, MPI_BYTE, 0, 47, MPI_COMM_WORLD, &request_1);
              MPI_Wait(&request_1, &status);
              break;
          }              
        }
        MPI_Request_free(&request);

      } 
      else if (myid == 0) {
        t_start = MPI_Wtime();
        /*
        for (i = 0; i < nrIter; i++) {
          MPI_Recv (buf, bufSize, MPI_BYTE, 1, 47, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        */
      //MPI_Recv_init(buf, bufSize, MPI_BYTE, 1, 47, MPI_COMM_WORLD, &request);
        MPI_Recv_init(buf, bufSize, MPI_BYTE, 1, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
        for (i = 0; i < nrIter; i++) {
          MPI_Start(&request);
          MPI_Wait(&request, &status);
#ifdef TEST_COMMUNICATION
          if(status.MPI_SOURCE == 0) {
            if(bufSize >= 2*sizeof(int)) {
              int idx;
              for(idx = 1; idx < bufSize/sizeof(int); idx <<= 1) {
                if( ((int *)buf)[idx] != bufSize + idx + i) {
                  fprintf(stdout, "ptent_10: Bad data\n"); fflush(stdout);
                  exit(1);
                  goto exception;
                }  
              }
            }
          }
#endif
        }
        MPI_Request_free(&request);

        t_end = MPI_Wtime() - t_start;
      
        latency = (t_end  / (nrIter * (numprocs - 1))) * 1000000; 
        bandwidth = bufSize / latency;
        fprintf(stdout, "%20d\t%20.9f\t%20.9f\n", bufSize, latency, bandwidth); fflush(stdout);
      }
      if(k == 1) bufSize -= bufSize/3;
      if(bufSize == 0) bufSize = 1;
    }
  }

  free(buf);
  MPI_Finalize ();
  return 0;

exception:
  printf("Exception in rank %d \n", myid); fflush(stdout);
  MPI_Finalize ();
  return 1;
}

