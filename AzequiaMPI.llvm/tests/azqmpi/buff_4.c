#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include "mpi.h" 

#define BUFF_ITER        64
#define BUFF_SIZE_MAX    (1024 * 1024 * 2)   
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
  int          i, iter, k, bufSize, cancelCnt; 
  int          nrIter;
  double       t_start, t_end, latency, bandwidth;
  int          count, flag;
  int          userbufsize, userbufsize2;
  char        *userbuf, *userbuf2;
  MPI_Request  req, master;

  MPI_Init (NULL, NULL);
  MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
  if(numprocs != 1) {
    printf("This program admits just 1 rank\n"); fflush(stdout);
    exit(1);
  }
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);
  if (myid == 0) {
    fprintf(stdout, "%20s\t%20.18s\t%53.51s\n", "Msg Size (bytes)", "Latency (us)", 
                                                "Bandwidth (MB/s)    Iterations     Cancellations"); fflush(stdout);
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
      nrIter /= BUFF_ITER;
      nrIter /= 2;
      //nrIter = 1;
      userbufsize = (bufSize +  MPI_BSEND_OVERHEAD) * BUFF_ITER ;
      //printf("Buffer attach de %d octetos (MPI_BSEND_OVERHEAD = %d)\n", userbufsize, MPI_BSEND_OVERHEAD);
      if(posix_memalign((void *)&userbuf, CACHE_LINE_SIZE, userbufsize * sizeof(char))) {
        fprintf(stdout, "autoBuffer: Error in posix_memalign\n"); fflush(stdout);
        goto exception;
      }
      MPI_Buffer_attach(userbuf, userbufsize);

      if(MPI_SUCCESS != MPI_Bsend_init(buf, bufSize, MPI_BYTE, 0, 52, MPI_COMM_WORLD, &master)) {
        fprintf(stdout, "autoBuffer: Error in Bsend_init\n"); fflush(stdout);
        goto exception;
      }

      cancelCnt = 0;
      t_start = MPI_Wtime();
      for(i = 0; i < nrIter; i++) {
        for(iter = 0; iter < BUFF_ITER; iter++) {
          switch(iter % 3) {
            case 0:
              if(MPI_SUCCESS != MPI_Bsend(buf, bufSize, MPI_BYTE, 0, 52, MPI_COMM_WORLD)) {
                fprintf(stdout, "autoBuffer: Error in MPI_Bsend\n"); fflush(stdout);
                goto exception;
              }
              break;
            case 1:
              if(MPI_SUCCESS != MPI_Ibsend(buf, bufSize, MPI_BYTE, 0, 52, MPI_COMM_WORLD, &req)) {
                fprintf(stdout, "autoBuffer: Error in MPI_Ibsend\n"); fflush(stdout);
                goto exception;
              }
              MPI_Cancel(&req); 
              MPI_Wait(&req, &status);
              MPI_Test_cancelled(&status, &flag);
              if (flag) {
                cancelCnt++;
                MPI_Bsend(buf, bufSize, MPI_BYTE, 0, 52, MPI_COMM_WORLD);
              }
              break;
            case 2:
              MPI_Start( &master );
              MPI_Cancel(&req); 
              MPI_Wait( &master , &status );
              break;
          }
        }
        for(iter = 0; iter < BUFF_ITER; iter++) {
          MPI_Recv(buf, bufSize, MPI_BYTE, 0, 52, MPI_COMM_WORLD, &status);
          //printf("Received message with tag: %d and size: %d\n", status.MPI_TAG, count);
        }
      }
      t_end = MPI_Wtime() - t_start;

      latency = (t_end / nrIter) * 1000000; 
      latency /= BUFF_ITER;
      bandwidth = bufSize / latency;
      fprintf(stdout, "%20d\t%20.9f\t%20.9f\t%11d\t%12.4f%\n", bufSize, latency, bandwidth, nrIter, (cancelCnt*100.0)/nrIter); 
      fflush(stdout);
      MPI_Buffer_detach(&userbuf2, &userbufsize2);
      MPI_Request_free( &master );
      free(userbuf);
      if(k == 1) bufSize -= bufSize/3;
      if(bufSize == 0) bufSize = 1;
    } /* k */
  }
  free(buf);

  MPI_Finalize ();
  return 0;

exception:
  printf("autoBuffer: Exception in rank %d \n", myid); fflush(stdout);
  MPI_Finalize ();
  return 0;
}


