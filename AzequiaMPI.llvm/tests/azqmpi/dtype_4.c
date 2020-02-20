#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

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
  MPI_Datatype  dtypes[3];
  int           blklen[3] = {1, 6, 7};
  MPI_Aint      displs[3];
  int           dtsize, dtextent;
  MPI_Aint      dtlb, dtub;
  int           i, j;
  MPI_Status    status;


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  dtypes[0] = MPI_INT;
  dtypes[1] = MPI_DOUBLE;
  dtypes[2] = MPI_CHAR;

  MPI_Address(&particle,  &displs[0]);
  MPI_Address(particle.d, &displs[1]);
  MPI_Address(particle.b, &displs[2]);

  if (myid == 0) {
    printf("ADDRESSES\n");
    printf("0x%x  0x%x  0x%x\n", displs, displs + 1, displs + 2);
  }

  MPI_Type_struct(3, blklen, displs, dtypes, &particle_type);
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

    particle.Class = 11;
    for (j = 0; j < 6; j++) particle.d[j] = 1.0 / ((double)j + 1.0);
    for (j = 0; j < 7; j++) particle.b[j] = j + 65;

	sleep(1);
    printf("Sending message ...\n");
    MPI_Send(MPI_BOTTOM, 1, particle_type, 1, 99, MPI_COMM_WORLD);
	//MPI_Send(&particle, 1, particle_type, 1, 99, MPI_COMM_WORLD);

  } else if (myid == 1) {
	
    memset(&particle, 0, sizeof(particle_type));

    MPI_Recv(MPI_BOTTOM, 1, particle_type, 0, 99, MPI_COMM_WORLD, &status);

    printf("Received: \n");
    printf("\tClass:   %d \n", particle.Class);
    for (i = 0; i < 6; i++) printf("%lf  ", particle.d[i]); printf("\n");
    for (i = 0; i < 7; i++) printf("%c  ", particle.b[i]); printf("\n");
  }

  if (MPI_Type_free(&particle_type) != MPI_SUCCESS)
    printf("Type Free test\n");

  MPI_Finalize();
  return MPI_SUCCESS;
}

