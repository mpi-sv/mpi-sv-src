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
  int          flag = 0;
  int          i, j;
  int         *recvbuf, *sendbuf;
  int          count;
  int          index;


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  if (myid == 0) {

    recvbuf = (int *) malloc (COUNT * sizeof(int));

    for (i = 1; i < numprocs; i++)
      MPI_Irecv(recvbuf, COUNT, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &(request[i - 1]));

      while (!flag) {
        MPI_Testall(numprocs - 1, request, &flag, status);
        usleep(100 * 1000); printf("."); fflush(stdout);
      }

      printf("All requests COMPLETED \n");
      for (i = 0; i < numprocs - 1; i++) {
        MPI_Get_count(&status[i], MPI_INT, &count);
        printf("Status. Received %d elements from %d process with tag: %d\n", count, status[i].MPI_SOURCE, status[i].MPI_TAG);
      }

    free(recvbuf);

  } else {

    sendbuf = (int *) malloc (COUNT * sizeof(int));
    for (i = 0; i < COUNT; i++)
      sendbuf[i] = myid + i;

    if (myid == 1)
      sleep(1);
    MPI_Send(sendbuf, myid, MPI_INT, 0, myid * 10, MPI_COMM_WORLD);

    free(sendbuf);

  }

  MPI_Finalize();
  return(0);
}
