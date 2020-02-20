#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"


#define  ARRAY_LEN   100


double sendbuf[ARRAY_LEN];
double recvbuf[ARRAY_LEN];


int main (int argc, char **argv) {

  int           myid, numprocs;
  int           dtsize, dtextent;
  MPI_Aint      dtlb, dtub;
  int           i, j, k;
  MPI_Status    status;
  int           count;
  MPI_Datatype  doubletype;


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  MPI_Type_vector(ARRAY_LEN / 2, 1, 2, MPI_DOUBLE, &doubletype);
  MPI_Type_commit(&doubletype);

  MPI_Type_size(doubletype, &dtsize);
  MPI_Type_extent(doubletype, &dtextent);
  MPI_Type_lb(doubletype, &dtlb);
  MPI_Type_ub(doubletype, &dtub);
  if (myid == 0) {
    printf("SIZE:   %d\n", dtsize);
    printf("EXTENT: %d\n", dtextent);
    printf("LB:   0x%x\n", dtlb);
    printf("UB:   0x%x\n", dtub);
  }


  if (myid == 0) {

    for (k = 0; k < ARRAY_LEN; k++) {
      sendbuf[k] = (double)k;
    }

    MPI_Send(sendbuf, 1, doubletype, 1, 99, MPI_COMM_WORLD);

  } else if (myid == 1) {

    memset(recvbuf, 0, sizeof(double) * ARRAY_LEN);

    /*
    MPI_Recv(recvbuf, ARRAY_LEN, MPI_DOUBLE, 0, 99, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_DOUBLE, &count);
    printf("Received  %d  items: \n", count);
    for (k = 0; k < count; k++) {
      printf("%lf   ", recvbuf[k]);
    }
    */

    MPI_Recv(recvbuf, 1, doubletype, 0, 99, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, doubletype, &count);

    printf("Received  %d  items: \n", count);
    for (k = 0; k < ARRAY_LEN; k++) {
      printf("%lf   ", recvbuf[k]);
    }

  }

  if (MPI_Type_free(&doubletype) != MPI_SUCCESS)
    printf("Type Free test\n");


  MPI_Finalize();
  return MPI_SUCCESS;
}

