#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"


#define  LEN   10

double smatrix[LEN][LEN];
double rmatrix[LEN][LEN];


int main (int argc, char **argv) {

  int           myid, numprocs;
  int           dtsize, dtextent;
  MPI_Aint      dtlb, dtub;
  int           i, j, k;
  MPI_Status    status;
  int           count;
  MPI_Datatype  type[2];
  MPI_Aint      displs[2];
  int           blklens[2];
  MPI_Datatype  col, col1;
  int           double_extent;


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);


  MPI_Type_vector(LEN, 1, LEN, MPI_DOUBLE, &col);

  MPI_Type_size(col, &dtsize);
  MPI_Type_extent(col, &dtextent);
  MPI_Type_lb(col, &dtlb);
  MPI_Type_ub(col, &dtub);
  if (myid == 0) {
    printf("SIZE:   %d\n", dtsize);
    printf("EXTENT: %d\n", dtextent);
    printf("LB:   0x%x\n", dtlb);
    printf("UB:   0x%x\n", dtub);
  }

  MPI_Type_extent(MPI_DOUBLE, &double_extent);
  displs[0] = 0;
  displs[1] = double_extent;
  blklens[0] = 1;
  blklens[1] = 1;
  type[0] = col;
  type[1] = MPI_UB;
  MPI_Type_struct(2, blklens, displs, type, &col1);
  MPI_Type_commit(&col1);

  MPI_Type_size(col1, &dtsize);
  MPI_Type_extent(col1, &dtextent);
  MPI_Type_lb(col1, &dtlb);
  MPI_Type_ub(col1, &dtub);
  if (myid == 0) {
    printf("SIZE:   %d\n", dtsize);
    printf("EXTENT: %d\n", dtextent);
    printf("LB:   0x%x\n", dtlb);
    printf("UB:   0x%x\n", dtub);
  }

  k = 0;
  for (i = 0; i < LEN; i++) {
    for (j = 0; j < LEN; j++) {
      smatrix[i][j] = (double)(k++);
    }
  }

  if (myid == 0) {
    printf("Matrix: \n\n");
    for (i = 0; i < LEN; i++) {
      for (j = 0; j < LEN; j++) {
        printf("%lf  ", smatrix[i][j]);
      }
      printf("\n");
    }
    printf("\n");
  }

  //MPI_Sendrecv(smatrix, LEN,       col1,       myid, 67,
  //             rmatrix, LEN * LEN, MPI_DOUBLE, myid, 67,
  //             MPI_COMM_WORLD, &status);

  if (myid == 0) {
    MPI_Send(smatrix, LEN, col1, 1, 67, MPI_COMM_WORLD);
  } else if (myid == 1) {
    MPI_Recv(rmatrix, LEN * LEN, MPI_DOUBLE, 0, 67, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_DOUBLE, &count);
    printf("Received  %d  items\n", count);
  }

  if (myid == 1) {
    printf("Transpose matrix: \n\n");
    for (i = 0; i < LEN; i++) {
      for (j = 0; j < LEN; j++) {
        printf("%lf  ", rmatrix[i][j]);
      }
      printf("\n");
    }
    printf("\n");
  }


  if (MPI_Type_free(&col) != MPI_SUCCESS)
    printf("Type Free test\n");

  if (MPI_Type_free(&col1) != MPI_SUCCESS)
    printf("Type Free test\n");


  MPI_Finalize();
  return MPI_SUCCESS;
}

