#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

#define  NUM_RQST_MAX              32
#define  BUFF_SIZE_MAX   (1024 * 1024)


static int size2itera(int size, int nProcs)
{
  int        num_iter;

  if     (size <= 128)        num_iter = ( 1024 * 128 * 4) / nProcs;
  else if(size <= 1024 * 16)  num_iter = ( 1024 * 128 * 4) / nProcs;
  else if(size <= 1024 * 128) num_iter = ( 1024 * 16 *  4) / nProcs;
  else 	                      num_iter = size / 64;
  
//printf("%d iter\n", num_iter); fflush(stdout);
  return 4*num_iter;
}


int main (int argc, char **argv) {

  int          numprocs, myid;
  char        *buf;
  char        *bufrecv[NUM_RQST_MAX];
  MPI_Request  req[NUM_RQST_MAX];
  MPI_Status   status[NUM_RQST_MAX];
  int          iter;
  int          i, j, bufSize, nrIter;
  int          outcount = 0;
  int          idx[NUM_RQST_MAX];
  int          total_cnt;
  int          count;
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
  for (j = 0; j < NUM_RQST_MAX; j++)
    bufrecv[j] = (char *) malloc (BUFF_SIZE_MAX * sizeof(char));
  buf = (char *) malloc (BUFF_SIZE_MAX * sizeof(char));

  for(bufSize = 1; bufSize <= BUFF_SIZE_MAX; bufSize <<= 1) {
    nrIter = size2itera(bufSize, numprocs);
    //nrIter = 1;
    if (myid > 0) {
      for(iter = 0; iter < nrIter; iter++) {
      //buf = myid;
      //usleep(100000 * myid);
        MPI_Send (buf, bufSize, MPI_CHAR, 0, myid * 5, MPI_COMM_WORLD);
      }
    } 
    else if (myid == 0) {
    //printf("\nbufSize = %d ------------------------------------------\n", bufSize); fflush(stdout);
      t_start = MPI_Wtime();
      for(iter = 0; iter < nrIter; iter++) {
        req[0] = MPI_REQUEST_NULL;
        for(i = 1; i < numprocs; i++)
          MPI_Irecv(bufrecv[i], bufSize, MPI_CHAR, i, MPI_ANY_TAG, MPI_COMM_WORLD, &req[i]);
        total_cnt = 0;
        outcount  = 0;
        while ((total_cnt < numprocs) && (outcount != MPI_UNDEFINED)) {
          MPI_Waitsome(numprocs, req, &outcount, &idx, status);
          if(outcount != MPI_UNDEFINED) {
            //printf ("Completed %d requests\n", outcount); fflush(stdout);
            for (i = 0; i < outcount; i++) {
              j = idx[i];
              MPI_Get_count(&status[i], MPI_CHAR, &count);
              //printf ("Received:  %d -> %d  (count  %d  tag %d) (request 0x%x)\n", j, bufrecv[j], count, status[i].MPI_TAG, &req[j]);
            }
            total_cnt += outcount;
          }
        }
      }
      t_end = MPI_Wtime();
      latency = ((t_end - t_start) / nrIter) * 1000000;  
      latency /= (numprocs - 1); 
      bandwidth = bufSize / latency;

      fprintf(stdout, "%20d\t%20.9f\t%20.9f\n", bufSize, latency, bandwidth); fflush(stdout);
    }
  }

  MPI_Finalize ();
  return 0;
}

