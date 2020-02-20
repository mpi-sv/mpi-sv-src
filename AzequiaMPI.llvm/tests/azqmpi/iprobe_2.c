#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

#define ITER             (1024 * 256)
#define NUM_RQST_MAX     32
#define BUFF_SIZE_MAX    (1024 * 128 * 8)   
#define CACHE_LINE_SIZE  64

//#define IPROBE_ANY_SOURCE
#ifndef IPROBE_ANY_SOURCE
#define TEST_COMMUNICATION
#endif

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

  if     (size <= 128)        num_iter = ( 1024 * 128 * 3);
  else if(size <= 1024 * 16)  num_iter = ( 1024 * 128 * 4);
  else if(size <= 1024 * 128) num_iter = ( 1024 * 16 *  4);
  else 	                      num_iter = size / 128;
  
//printf("%d iter\n", num_iter); fflush(stdout);

#ifdef TEST_COMMUNICATION
  return (num_iter/2) / nProcs;
#else
  return (2*num_iter) / nProcs;
#endif
}

int main (int argc, char **argv)  
{
  int          numprocs, myid;
  char        *buf;
  char        *bufrecv[NUM_RQST_MAX];
  MPI_Request  req    [NUM_RQST_MAX];
  MPI_Status   status;
  int          i, j, k, idx, bufSize, flag; 
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
  if (myid == 0) {
    for(j = 1; j < numprocs; j++) 
      if(posix_memalign((void *)&bufrecv[j], CACHE_LINE_SIZE, 2*BUFF_SIZE_MAX * sizeof(char)))    goto exception;
  }
  else 
    if(posix_memalign((void *)&buf, CACHE_LINE_SIZE, 2*BUFF_SIZE_MAX * sizeof(char)))             goto exception;

  for(bufSize = 0; bufSize <= BUFF_SIZE_MAX; bufSize <<= 1) {
    for(k = 0; k < 2; k++) {
      if(k == 1) bufSize += bufSize/2;
      nrIter = size2itera(bufSize, numprocs);
      //nrIter = 1;

      if ((myid > 0) && (myid <= NUM_RQST_MAX)) {
        for(i = 0; i < nrIter; i++) {
#ifdef TEST_COMMUNICATION
          if(myid == 1) {
            if(bufSize >= 2*sizeof(int)) {
              for(idx = 1; idx < bufSize/sizeof(int); idx <<= 1/*idx++*/ ) {
                //fprintf(stdout, "%d ", bufSize + idx); 
                ((int *)buf)[idx] = bufSize + idx + i;
              }
              //fprintf(stdout, "\n"); fflush(stdout);
            }
          }
#endif
          MPI_Send (buf, bufSize, MPI_CHAR, 0, 36, MPI_COMM_WORLD);
        }
      }

      else if (myid == 0) {
        t_start = MPI_Wtime();
        for(i = 0; i < nrIter; i++) {
          for(j = 1; j < numprocs; j++) {

            flag = 0;
            while (flag == 0) {
#ifdef IPROBE_ANY_SOURCE
              MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
#else
              MPI_Iprobe(j,              MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
#endif
            }
            MPI_Get_count(&status, MPI_CHAR, &count);
            /*
            if(count > bufSize * sizeof(char)) {
              fprintf(stdout, "Rank 0: Can not receive probed %d bytes in my %d bytes buffer\n", count, bufSize * sizeof(char)); 
              fflush(stdout);
              exit(0);
            }
            */
            MPI_Recv(bufrecv[j], count, MPI_CHAR, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
#ifdef TEST_COMMUNICATION
            if(status.MPI_SOURCE == 1) {
              if(bufSize >= 2*sizeof(int)) {
                for(idx = 1; idx < bufSize/sizeof(int); idx <<= 1/*idx++ */) {
                  if( ((int *)bufrecv[1])[idx] != bufSize + idx + i) {
                    fprintf(stdout, "Bad data\n"); fflush(stdout);
                    exit(1);
                    goto exception;
                  }  
                }
              }
            }
#endif
          }
        }
        t_end = MPI_Wtime() - t_start;
      
        latency = ((t_end ) / (nrIter * (numprocs - 1))) * 1000000; 
        bandwidth = bufSize / latency;
        fprintf(stdout, "%20d\t%20.9f\t%20.9f\n", bufSize, latency, bandwidth); fflush(stdout);
      }
      if(k == 1) bufSize -= bufSize/3;
      if(bufSize == 0) bufSize = 1;
    }
  }
  if (myid == 0) {
    for(j = 1; j < numprocs; j++) 
      free(bufrecv[j]);
  }
  else 
    free(buf);

  MPI_Finalize ();
  return 0;

exception:
  printf("Exception in rank %d \n", myid); fflush(stdout);
  MPI_Finalize ();
  return 0;
}



