#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

#define  COUNT  128

int main (int argc, char **argv) 
{
  int        *recvbuf, *sendbuf;
  int         numprocs;
  int         myid;
  int         i;
  int         flag = 0;
  int         count;
  MPI_Status  status, st2;


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  printf("Probe_2. ^^^ %d ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n", myid); fflush(stdout);
  if (myid == 0) {
    recvbuf = (int *) malloc (COUNT * sizeof(int));
    for (i = 0; i < numprocs - 1; i++) {
      while (flag == 0) {
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
      }
      flag = 0;
      MPI_Get_count(&status, MPI_INT, &count);
      printf("Probed a buffer with %d elements from %d process\n", count, status.MPI_SOURCE);
      printf("\tTag is %d\n", status.MPI_TAG);
      MPI_Recv(recvbuf, count, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &st2);
      MPI_Get_count(&st2, MPI_INT, &count);
      printf("Received %d elements from %d process\n", count, st2.MPI_SOURCE);
    }
    free(recvbuf);
  } 
  else {
    sendbuf = (int *) malloc (COUNT * sizeof(int));
    sendbuf[0] = myid;
    if (myid % 2) sleep(1);
    MPI_Send(sendbuf, myid, MPI_INT, 0, myid, MPI_COMM_WORLD);
    free(sendbuf);
  }
  printf("Probe_2. vvv %d vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n", myid); fflush(stdout);
  MPI_Finalize();
  return(0);
}

