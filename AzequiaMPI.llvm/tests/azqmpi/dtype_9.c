#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"


#define  NUM_ITER  1000
#define  LEN       100

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
  MPI_Datatype  col, xpose;
  int           double_extent;
  int           psize;
  double        t_start, t_end;


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);


  MPI_Type_vector(LEN, 1, LEN, MPI_DOUBLE, &col);
  MPI_Type_commit(&col);

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
  MPI_Type_hvector(LEN, 1, double_extent, col, &xpose);
  MPI_Type_commit(&xpose);

  MPI_Type_size(xpose, &dtsize);
  MPI_Type_extent(xpose, &dtextent);
  MPI_Type_lb(xpose, &dtlb);
  MPI_Type_ub(xpose, &dtub);
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

  if (myid == 99) {
    printf("Matrix: \n\n");
    for (i = 0; i < LEN; i++) {
      for (j = 0; j < LEN; j++) {
        printf("%lf  ", smatrix[i][j]);
      }
      printf("\n");
    }
    printf("\n");
  }

  //MPI_Sendrecv(smatrix, 1,         xpose,      myid, 67,
  //             rmatrix, LEN * LEN, MPI_DOUBLE, myid, 67,
  //             MPI_COMM_WORLD, &status);

  MPI_Pack_size(1, col, MPI_COMM_WORLD, &psize);
  if (myid == 0) printf("col PACK SIZE: %d\n", psize);

  MPI_Pack_size(1, xpose, MPI_COMM_WORLD, &psize);
  if (myid == 0) printf("xpose PACK SIZE: %d\n", psize);


  if (myid == 0) {

    for (k = 0; k < NUM_ITER; k++)
      MPI_Send(smatrix, 1, xpose, 1, 67, MPI_COMM_WORLD);

  } else if (myid == 1) {

    t_start = MPI_Wtime();
    for (k = 0; k < NUM_ITER; k++)
      MPI_Recv(rmatrix, LEN * LEN, MPI_DOUBLE, 0, 67, MPI_COMM_WORLD, &status);
    t_end = MPI_Wtime();
    printf("Received  %d  transposed matrices in %lf sec\n", k, t_end - t_start);
    MPI_Get_count(&status, MPI_DOUBLE, &count);
    printf("Received  %d  items\n", count);

  }

  if (myid == 99) {
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

  if (MPI_Type_free(&xpose) != MPI_SUCCESS)
    printf("Type Free test\n");


  MPI_Finalize();
  return MPI_SUCCESS;
}

