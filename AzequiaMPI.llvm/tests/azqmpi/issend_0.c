#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>


#define BUFF_SIZE_MAX    (1024 * 128 * 8)   
#define CACHE_LINE_SIZE  64


static int size2itera(int size, int nProcs)
{
  int        num_iter;

  if     (size <= 128)        num_iter = (  1024 * 128 * 3);
  else if(size <= 1024 * 16)  num_iter = (  1024 * 128 * 4);
  else if(size <= 1024 * 128) num_iter = (  1024 * 64 *  4);
  else 	                      num_iter = size / 32;
  num_iter /= 10;
//printf("%d iter\n", num_iter); fflush(stdout);

#ifdef TEST_COMMUNICATION
  return (num_iter) / nProcs;
#else
  return (10*num_iter) / nProcs;
#endif
}



int main(int argc, char *argv[])
{
  int          myid, numprocs, left, right, i, bufSize, k, nrIter;
  char        *buffer, 
              *buffer2;
  MPI_Request  request, request2;
  MPI_Status   status;
  double       t_start, t_end, latency, bandwidth;

  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  if (myid == 0) {
    fprintf(stdout, "%20s\t%20.18s\t%20.18s\n", "Msg Size (bytes)", "Latency (us)", "Bandwidth (MB/s)"); fflush(stdout);
    fprintf(stdout, "%20s\t%20.18s\t%20.18s\n", "----------------", "------------", "----------------"); fflush(stdout);
  }
  if(posix_memalign((void *)&buffer,  CACHE_LINE_SIZE, 2*BUFF_SIZE_MAX * sizeof(char)))             goto exception;
  if(posix_memalign((void *)&buffer2, CACHE_LINE_SIZE, 2*BUFF_SIZE_MAX * sizeof(char)))             goto exception;

  //fprintf(stdout, "Rank(%d) buf = %p, buf_2 = %p .....................................\n", myid, buf, buf_2); fflush(stdout);
  for(bufSize = 0; bufSize <= BUFF_SIZE_MAX; bufSize <<= 1) {
    for(k = 0; k < 2; k++) {
      if(k == 1) bufSize += bufSize/2;

      nrIter = size2itera(bufSize, numprocs);
      //nrIter = 1;

      right = (myid + 1) % numprocs;
      left = myid - 1;
      if (left < 0)
        left = numprocs - 1;

      t_start = MPI_Wtime();
      for(i = 0; i < nrIter; i++) {
        
        MPI_Issend(buffer2, bufSize, MPI_BYTE, right, 123, MPI_COMM_WORLD, &request2);
        MPI_Irecv (buffer,  bufSize, MPI_BYTE, left,  123, MPI_COMM_WORLD, &request );
        MPI_Wait(&request, &status);
        MPI_Wait(&request2, &status);
        
        /*
        MPI_Irecv (buffer,  bufSize, MPI_BYTE, left,  123, MPI_COMM_WORLD, &request );
        MPI_Send  (buffer2, bufSize, MPI_BYTE, right, 123, MPI_COMM_WORLD);
        MPI_Wait(&request, &status);
        */
      }
      t_end = MPI_Wtime() - t_start;
      
      latency = ((t_end ) / (nrIter )) * 1000000; 
      bandwidth = bufSize / latency;
      if(myid==0) fprintf(stdout, "%20d\t%20.9f\t%20.9f\n", bufSize, latency, bandwidth); fflush(stdout);

      if(k == 1) bufSize -= bufSize/3;
      if(bufSize == 0) bufSize = 1;
    }
  }
  free(buffer );
  free(buffer2);
  MPI_Finalize();
  return 0;

exception:
  printf("Exception in rank %d \n", myid); fflush(stdout);
  MPI_Finalize ();
  return 0;
}

