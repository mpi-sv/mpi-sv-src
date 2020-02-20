#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

struct MyStruct {
  int    i1;
  int    i2;
  float  f1;
  float  f2;
  int    i3;
  double d1;
  double d2;
  double d3;
  char   c1;
  char   c2;
};
typedef struct MyStruct MyStruct;

int main (int argc, char **argv) {

  int           myid, numprocs;
  MPI_Datatype  dt_struct;
  int           dtsize, dtextent;
  MPI_Aint      dtlb, dtub;
  MPI_Status    status;
  int           blklen[10];
  MPI_Aint      displs[10];
  MPI_Datatype  dtypes[10];
  MyStruct      sdata, rdata;
  int           i;


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  blklen[0] = 2;
  blklen[1] = 1;
  blklen[2] = 1;
  blklen[3] = 1;
  blklen[4] = 2;
  blklen[5] = 1;
  displs[0] = 0;
  displs[1] = 8;
  displs[2] = 16;
  displs[3] = 24; 
  displs[4] = 48;
  displs[5] = 56;
  dtypes[0] = MPI_INT;
  dtypes[1] = MPI_FLOAT;
  dtypes[2] = MPI_INT;
  dtypes[3] = MPI_DOUBLE;
  dtypes[4] = MPI_CHAR;
  dtypes[5] = MPI_UB;

  if (myid == 0)  {
    printf("Types:  FLOAT: 0x%x   INT: 0x%x   DOUBLE: 0x%x\n", MPI_FLOAT, MPI_INT, MPI_DOUBLE);
  }

  MPI_Type_struct(6, blklen, displs, dtypes, &dt_struct);
  if (MPI_SUCCESS != MPI_Type_commit(&dt_struct)) {
    printf("Could not make char array type.\n");
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  MPI_Type_size(dt_struct, &dtsize);
  MPI_Type_extent(dt_struct, &dtextent);
  MPI_Type_lb(dt_struct, &dtlb);
  MPI_Type_ub(dt_struct, &dtub);
  if (myid == 0) {
    printf("SIZE:   %d\n", dtsize);
    printf("EXTENT: %d\n", dtextent);
    printf("LB:     %d\n", dtlb);
    printf("UB:     %d\n", dtub);
  }

  if (myid == 0) {

    sdata.i1 = 5;
    sdata.i2 = 6;
    sdata.f1 = 1.98;
    sdata.f2 = 0.2;
    sdata.i3 = 7;
    sdata.d1 = 34;
    sdata.d2 = 45;
    sdata.d3 = 56;
    sdata.c1 = 'a';
    sdata.c2 = 'b';

    MPI_Send(&sdata, 1, dt_struct, 1, 99, MPI_COMM_WORLD);

  } else if (myid == 1) {

    memset(&rdata, 0, sizeof(dt_struct));

    MPI_Recv(&rdata, 1, dt_struct, 0, 99, MPI_COMM_WORLD, &status);

    printf("Received: \n");
    printf("\tFloats:  %f   %f\n", rdata.f1, rdata.f2);
    printf("\tInts:    %d   %d  %d\n", rdata.i1, rdata.i2, rdata.i3);
    printf("\tDoubles: %lf  %lf   %lf\n", rdata.d1, rdata.d2, rdata.d3);
    printf("\tChars    %c   %c\n", rdata.c1, rdata.c2);

  }

  if (MPI_Type_free(&dt_struct) != MPI_SUCCESS)
    printf("Type Free test\n");

  MPI_Finalize();
  return MPI_SUCCESS;
}

