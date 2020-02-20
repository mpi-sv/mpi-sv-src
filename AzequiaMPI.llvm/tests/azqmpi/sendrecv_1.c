#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

#define BUFLEN  4//512

int main (int argc, char **argv) {

    int myid, numprocs, tot = 0, i;
    int buffer[BUFLEN];
    int buf2[BUFLEN];
    MPI_Status status;


    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);


    if (myid == 0) {

      for (i = 0; i < BUFLEN; i++)
        buffer[i] = i + myid;

      MPI_Sendrecv(buffer, BUFLEN, MPI_INT, 1, 99, buf2, BUFLEN, MPI_INT, 1, 99, MPI_COMM_WORLD, &status);

      for (i = 0; i < BUFLEN; i++)
        tot += buf2[i];
      fprintf(stdout, "Process %d. Result %d\n", myid, tot);

    } else {

      for (i = 0; i < BUFLEN; i++)
        buffer[i] = i + myid;

      MPI_Sendrecv(buffer, BUFLEN, MPI_INT, 0, 99, buf2, BUFLEN, MPI_INT, 0, 99, MPI_COMM_WORLD, &status);

      for (i = 0; i < BUFLEN; i++)
        tot += buf2[i];
      fprintf(stdout, "Process %d. Result %d\n", myid, tot);
    }

    fflush(NULL);
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return (0);
}
