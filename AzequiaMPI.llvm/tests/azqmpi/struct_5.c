#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
#endif


#include "mpi.h"

struct mytype {
  float     f;
};
typedef struct mytype mytype;


int main (int argc, char **argv) {

  int           myid, numprocs;
  MPI_Datatype  mtype;
  MPI_Datatype  T[3];
  int           B[3] = {1, 1, 1};
  MPI_Aint      D[3] = {-3, 0, 5};
  int           dtsize, dtextent;
  MPI_Aint      dtlb, dtub;
  int           i, j;
  MPI_Status    status;
  mytype        varmt;


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);


  /* Type 1 */
  T[0] = MPI_LB;
  T[1] = MPI_FLOAT;
  T[2] = MPI_UB;
  MPI_Type_struct(3, B, D, T, &mtype);
  MPI_Type_commit(&mtype);

  MPI_Type_size(mtype, &dtsize);
  MPI_Type_extent(mtype, &dtextent);
  MPI_Type_lb(mtype, &dtlb);
  MPI_Type_ub(mtype, &dtub);
  if (myid == 0) {
    printf("SIZE:   %d\n", dtsize);
    printf("EXTENT: %d\n", dtextent);
    printf("LB:     %d\n", dtlb);
    printf("UB:     %d\n", dtub);
  }

  if (myid == 0) {

    varmt.f = 0.7;

    printf("Sending message ...\n");
    MPI_Send(&varmt, 1, mtype, 1, 99, MPI_COMM_WORLD);
    sleep(1);

  } else if (myid == 1) {

    MPI_Recv(&varmt, 1, mtype, 0, 99, MPI_COMM_WORLD, &status);

    //memset(&particle, 0, sizeof(particle_type));
    printf("Received: %f\n", varmt.f);
  }

  if (MPI_Type_free(&mtype) != MPI_SUCCESS)
    printf("Type Free test\n");

  MPI_Finalize();
  return MPI_SUCCESS;
}

