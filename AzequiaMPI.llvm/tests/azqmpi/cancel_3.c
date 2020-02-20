#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

#define BUFF_ITER         32
#define NUM_RQST_MAX     32
#define BUFF_SIZE_MAX    (1024 * 128 * 8)   
#define CACHE_LINE_SIZE  64
//#define TEST_COMMUNICATION

static int size2itera(int size, int nProcs)
{
  int        num_iter;

  if     (size <= 128)       num_iter = ( 1024 * 128 * 8);
  else if(size <= 1024 * 4)  num_iter = ( 1024 * 128 * 4);
  else if(size <= 1024 * 16) num_iter = ( 1024 * 128 * 2);
  else                       num_iter = 1024 * ((1024 * 1024 * 8) / size);
  
#ifdef TEST_COMMUNICATION
  return (num_iter/2) / nProcs;
#else
  return (num_iter) / nProcs;
#endif
}


int main (int argc, char **argv)  
{
  int          numprocs, myid;
  char        *buf = NULL;
  MPI_Status   status;
  int          i, k, bufSize, cancelCnt; 
  int          nrIter;
  double       t_start, t_end, latency, bandwidth;
  int          count, flag;
  int          userbufsize, userbufsize2;
  char        *userbuf, *userbuf2;
  MPI_Request  req;

  MPI_Init (NULL, NULL);
  MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
  if(numprocs != 2) {
    printf("This program admits just 2 ranks\n"); fflush(stdout);
    exit(1);
  }
  if(numprocs > NUM_RQST_MAX) {
    printf("This program admits up to NUM_RQST_MAX ranks\n"); fflush(stdout);
    exit(1);
  }
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);
  if (myid == 0) {
    fprintf(stdout, "%20s\t%20.18s\t%53.51s\n", "Msg Size (bytes)", "Latency (us)", 
                                                "Bandwidth (MB/s)    Iterarions     Cancellations"); fflush(stdout);
    fprintf(stdout, "%20s\t%20.18s\t%53.51s\n", "----------------", "------------", 
                                                "----------------    ----------     -------------"); fflush(stdout);
  }
  if(posix_memalign((void *)&buf, CACHE_LINE_SIZE, 2*BUFF_SIZE_MAX * sizeof(char))) {
    fprintf(stdout, "buff_0: posix_memalign failed\n"); fflush(stdout);
    goto exception;
  }
  if(NULL == buf) {
    fprintf(stdout, "User buffer NULL\n"); fflush(stdout);
    exit(0);
  }  
  for(bufSize = 0; bufSize <= BUFF_SIZE_MAX; bufSize <<= 1) { 
    for(k = 0; k < 2; k++) {
      if(k == 1) bufSize += bufSize/2;
      nrIter = size2itera(bufSize, numprocs);
      nrIter *= 2;
      //nrIter = BUFF_ITER;

      if (myid == 0) {
        userbufsize = (bufSize +  MPI_BSEND_OVERHEAD) * BUFF_ITER;
        if(posix_memalign((void *)&userbuf, CACHE_LINE_SIZE, userbufsize * sizeof(char)))     goto exception;
        MPI_Buffer_attach(userbuf, userbufsize);
        cancelCnt = 0;
        t_start = MPI_Wtime();
        for(i = 0; i < nrIter; i++) {
          if(i % BUFF_ITER == 0) {
            MPI_Buffer_detach(&userbuf2, &userbufsize2);
            MPI_Buffer_attach(userbuf, userbufsize);
          }
        //if(MPI_SUCCESS != MPI_Isend (buf, bufSize, MPI_BYTE, 1, 52, MPI_COMM_WORLD, &req)) {
          if(MPI_SUCCESS != MPI_Ibsend(buf, bufSize, MPI_BYTE, 1, 52, MPI_COMM_WORLD, &req)) {
            fprintf(stdout, "buff_0: Error in MPI_Bsend\n"); fflush(stdout);
            goto exception;
          }
          MPI_Cancel(&req);
          MPI_Wait(&req, &status);
          MPI_Test_cancelled(&status, &flag);
          if (flag) {
            cancelCnt++;
            MPI_Send(buf, bufSize, MPI_BYTE, 1, 52, MPI_COMM_WORLD);
          }
        }
        t_end = MPI_Wtime() - t_start;

        latency = ((t_end ) / (nrIter * (numprocs - 1))) * 1000000; 
        bandwidth = bufSize / latency;
        fprintf(stdout, "%20d\t%20.9f\t%20.9f\t%11d\t%12.4f%\n", bufSize, latency, bandwidth, nrIter, (cancelCnt*100.0)/nrIter); 
        fflush(stdout);

        MPI_Buffer_detach(&userbuf2, &userbufsize2);
        free(userbuf);
      }

      else if (myid == 1) {
        for(i = 0; i < nrIter; i++) {
          MPI_Recv(buf, bufSize, MPI_BYTE, 0, 52, MPI_COMM_WORLD, &status);
        //printf("Received message with tag: %d and size: %d\n", status.MPI_TAG, count);
        }
      }
      if(k == 1) bufSize -= bufSize/3;
      if(bufSize == 0) bufSize = 1;
    } /* k */
  }
  free(buf);

  MPI_Finalize ();
  return 0;

exception:
  printf("Exception in rank %d \n", myid); fflush(stdout);
  MPI_Finalize ();
  return 0;
}



