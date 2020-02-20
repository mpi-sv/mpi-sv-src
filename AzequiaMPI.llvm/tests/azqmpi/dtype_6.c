#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"


#define  ARRAY_LEN   1000


struct Partstruct {
  int    Class;
  double d[6];
  char   b[7];
};
typedef struct Partstruct Partstruct;

Partstruct sendpart[ARRAY_LEN];
Partstruct recvpart[ARRAY_LEN];


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
  MPI_Datatype  array_part_type;
  MPI_Aint      ZD[ARRAY_LEN];
  int           ZB[ARRAY_LEN];
  int           count;


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


  if (myid == 0) {

    for (k = 0; k < ARRAY_LEN; k++) {
      sendpart[k].Class = ((k + 1) % 100) ? k : 0;
      for (j = 0; j < 6; j++) sendpart[k].d[j] = (double)(k + 1) / ((double)j + 1.0);
      for (j = 0; j < 7; j++) sendpart[k].b[j] = ((k + j) % 20) + 65;
    }

    /* Create array of particles type */
    printf("Particles with ZERO value:\n");
    j = 0;
    for (i = 0; i < ARRAY_LEN; i++) {
      if (sendpart[i].Class == 0) {
        ZD[j] = i;
        ZB[j] = 1;
        j++;
        printf("%d   ", i);
      }
    }
    printf("\n");

    MPI_Type_indexed(j, ZB, ZD, particle_type, &array_part_type);
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

    MPI_Send(sendpart, 1, array_part_type, 1, 99, MPI_COMM_WORLD);

    if (MPI_Type_free(&array_part_type) != MPI_SUCCESS)
      printf("Type Free test\n");

  } else if (myid == 1) {

    memset(recvpart, 0, sizeof(array_part_type));

    MPI_Recv(recvpart, ARRAY_LEN, particle_type, 0, 99, MPI_COMM_WORLD, &status);

    MPI_Get_count(&status, particle_type, &count);

    printf("Received  %d  zero-particles: \n", count);
    for (k = 0; k < count; k++) {
      printf("\tClass:   %d \n", recvpart[k].Class);
      for (i = 0; i < 6; i++) printf("%lf  ", recvpart[k].d[i]); printf("\n");
      for (i = 0; i < 7; i++) printf("%c  ",  recvpart[k].b[i]); printf("\n");
    }

  }

  if (MPI_Type_free(&particle_type) != MPI_SUCCESS)
    printf("Type Free test\n");


  MPI_Finalize();
  return MPI_SUCCESS;
}

