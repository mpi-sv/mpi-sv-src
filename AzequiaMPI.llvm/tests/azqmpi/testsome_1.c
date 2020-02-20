#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
#endif


#include "mpi.h"

#define COUNT         16
#define MAX_REQUESTS  16

int main (int argc, char **argv) {

  int          myid, numprocs;
  MPI_Request  request[MAX_REQUESTS];
  MPI_Status   status[MAX_REQUESTS];
  int          indices[MAX_REQUESTS];
  int          flag = 0;
  int          i, j;
  int         *recvbuf, *sendbuf;
  int          count;
  int          index;
  int          outcount;
  int          tot_satisfied = 0;


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  if (myid == 0) {
    recvbuf = (int *) malloc (COUNT * sizeof(int));
    for (i = 1; i < numprocs; i++)
      MPI_Irecv(recvbuf, COUNT, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &(request[i - 1]));
    sleep(1);
    while (tot_satisfied < numprocs - 1) {
        MPI_Testsome (numprocs - 1, request, &outcount, indices, status);
        if (outcount > 0) {
          tot_satisfied += outcount;
          printf("%d Requests COMPLETED (total: %d)\n", outcount, tot_satisfied);
          for (j = 0; j < outcount; j++)
            printf("Request: %d\n", indices[j]);
          for (j = 0; j < outcount; j++) {
            MPI_Get_count(&status[j], MPI_INT, &count);
            printf("Status. Received %d elements from %d process with tag %d\n", count, status[j].MPI_SOURCE, status[j].MPI_TAG);
          }
        }
      }
    free(recvbuf);
  } 
  else {

    sendbuf = (int *) malloc (COUNT * sizeof(int));
    for (i = 0; i < COUNT; i++)
      sendbuf[i] = myid + i;

    if (myid % 2)
      sleep(1);
    MPI_Send(sendbuf, myid, MPI_INT, 0, myid * 20, MPI_COMM_WORLD);

    free(sendbuf);

  }

  MPI_Finalize();
  return(0);
}
