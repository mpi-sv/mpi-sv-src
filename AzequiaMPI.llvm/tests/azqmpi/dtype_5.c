#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"


#define  ARRAY_LEN   10


struct Partstruct {
  int    Class;
  double d[6];
  char   b[7];
};
typedef struct Partstruct Partstruct;


int main (int argc, char **argv) {

  int           myid, numprocs;
  Partstruct    particle;
  MPI_Datatype  particle_type;
  MPI_Datatype  dtypes[4];
  int           blklen[4] = {1, 6, 7, 1};
  MPI_Aint      displs[4] = {0, 4, 52, 60};
  int           dtsize, dtextent;
  MPI_Aint      dtlb, dtub;
  int           i, j, k;
  MPI_Status    status;
  Partstruct    part[ARRAY_LEN];
  MPI_Datatype  array_part_type;


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  dtypes[0] = MPI_INT;
  dtypes[1] = MPI_DOUBLE;
  dtypes[2] = MPI_CHAR;
  dtypes[3] = MPI_UB;

  /* Create particle type */
  MPI_Type_struct(4, blklen, displs, dtypes, &particle_type);
  MPI_Type_commit(&particle_type);

  MPI_Type_size(particle_type, &dtsize);
  MPI_Type_extent(particle_type, &dtextent);
  MPI_Type_lb(particle_type, &dtlb);
  MPI_Type_ub(particle_type, &dtub);
  if (myid == 0) {
    printf("SIZE:   %d\n", dtsize);
    printf("EXTENT: %d\n", dtextent);
    printf("LB:   0x%x\n", dtlb);
    printf("UB:   0x%x\n", dtub);
  }

  /* Create array of particles type */
  MPI_Type_contiguous(ARRAY_LEN, particle_type, &array_part_type);
  MPI_Type_commit(&array_part_type);

  MPI_Type_size(array_part_type, &dtsize);
  MPI_Type_extent(array_part_type, &dtextent);
  MPI_Type_lb(array_part_type, &dtlb);
  MPI_Type_ub(array_part_type, &dtub);
  if (myid == 0) {
    printf("SIZE:   %d\n", dtsize);
    printf("EXTENT: %d\n", dtextent);
    printf("LB:   0x%x\n", dtlb);
    printf("UB:   0x%x\n", dtub);
  }


  if (myid == 0) {

    for (k = 0; k < ARRAY_LEN; k++) {
      part[k].Class = k + 1;
      for (j = 0; j < 6; j++) part[k].d[j] = (double)(k + 1) / ((double)j + 1.0);
      for (j = 0; j < 7; j++) part[k].b[j] = k + j + 65;
    }

    printf("Sending message ...\n");
    MPI_Send(part, 1, array_part_type, 1, 99, MPI_COMM_WORLD);

  } else if (myid == 1) {

    memset(part, 0, sizeof(array_part_type));

    printf("Receiving message of type 0x%x ...\n", particle_type);
    //MPI_Recv(&part, 1, array_part_type, 0, 99, MPI_COMM_WORLD, &status);
    MPI_Recv(part, 10, particle_type, 0, 99, MPI_COMM_WORLD, &status);

    printf("Received: \n");
    for (k = 0; k < ARRAY_LEN; k++) {
      printf("\tClass:   %d \n", part[k].Class);
      for (i = 0; i < 6; i++) printf("%lf  ", part[k].d[i]); printf("\n");
      for (i = 0; i < 7; i++) printf("%c  ",  part[k].b[i]); printf("\n");
    }

  }

  if (MPI_Type_free(&particle_type) != MPI_SUCCESS)
    printf("Type Free test\n");

  if (MPI_Type_free(&array_part_type) != MPI_SUCCESS)
    printf("Type Free test\n");

  MPI_Finalize();
  return MPI_SUCCESS;
}

