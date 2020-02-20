#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

#define COUNT           128
#define MAX_REQUESTS    128
#define MSG_PER_STREAM  2

int main (int argc, char **argv) 
{
  int          myid, numprocs;
  MPI_Request  request[MAX_REQUESTS];
  MPI_Status   status[MAX_REQUESTS];
  int          i, j, k, flag;
  int         *recvbuf, *sendbuf;
  int          count;
  int          outcount = 0;
  int          total_cnt;
  int          idx[MAX_REQUESTS];

  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  printf("Waitall_4. ^^^ %d ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n", myid); fflush(stdout);
  if (myid == 0) {
    sendbuf = (int *) malloc (COUNT * sizeof(int));
    request[0] = MPI_REQUEST_NULL;
    for (i = 1; i < numprocs; i++)
      MPI_Send_init (sendbuf, COUNT, MPI_INT, i, i * 10, MPI_COMM_WORLD, &request[i]);
    for (j = 0; j < MSG_PER_STREAM; j++) {
      for (i = 1; i < numprocs; i++) {
        MPI_Start(&request[i]);
      }
      MPI_Cancel (&request[numprocs-1]);
      total_cnt = 0;
      outcount = 0;
      while ((total_cnt < numprocs) && (outcount != MPI_UNDEFINED)) {
        MPI_Waitsome(numprocs, request, &outcount, idx, status);
        if(outcount != MPI_UNDEFINED) {
          for(k = 0; k < outcount; k++) {
            if(idx[k] == numprocs-1) {
              MPI_Test_cancelled (&status[k], &flag);
              if (!flag) {
                printf ("\t------------------------------------------------- Failed to cancel a Isend request\n");
                MPI_Get_count(&status[k], MPI_INT, &count);
                printf("\t(Index: %d) Sent last message from %d (tag %d) with %d elements\n", (int)idx, status[k].MPI_SOURCE, 
                                                                                                        status[k].MPI_TAG, count);
                fflush(stdout);
              }
              else {
                printf ("\t+++++++++++++++++++++++++++++++++++++++++++++++++ Isend Request SUCCESFULLY cancelled. Try again\n");
                MPI_Start(&request[numprocs-1]);
                MPI_Wait (&request[numprocs-1], &status[numprocs-1]);
              }
              break;
            }
          } 
          total_cnt += outcount;
        }
      }
    }
    for (i = 1; i < numprocs; i++) 
      MPI_Request_free(&request[i]);
    free(sendbuf);
  } 
  else {
    recvbuf = (int *) malloc (COUNT * sizeof(int));
    for (i = 0; i < COUNT; i++)
      recvbuf[i] = myid + i;
    for (j = 0; j < MSG_PER_STREAM; j++)
      MPI_Recv(recvbuf, myid, MPI_INT, 0, myid * 10, MPI_COMM_WORLD, &status[i]);

    MPI_Get_count(&status[i], MPI_INT, &count);
    printf("Status. Received %d elements from %d process with tag: %d\n", count, status[i].MPI_SOURCE, status[i].MPI_TAG);
    free(recvbuf);
  }
  printf("Waitall_4. vvv %d vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n", myid); fflush(stdout);
  MPI_Finalize();
  return(0);
}
