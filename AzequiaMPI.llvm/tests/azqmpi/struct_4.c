#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
#endif


#include "mpi.h"

struct mytype_1 {
  double d_t1;
  char   c_t1;
};
typedef struct mytype_1 mytype_1;


struct mytype_2 {
  float     f_t2;
  mytype_1  t_t2;
  char      c_t2;
};
typedef struct mytype_2 mytype_2;


int main (int argc, char **argv) {

  int           myid, numprocs;
  MPI_Datatype  mt1, mt2;
  MPI_Datatype  T[3];
  int           B[3] = {2, 1, 3};
  MPI_Aint      D[3] = {0, 16, 26};
  MPI_Datatype  T1[2];
  int           B1[2] = {1, 1};
  MPI_Aint      D1[2] = {0, 8};
  int           dtsize, dtextent;
  MPI_Aint      dtlb, dtub;
  int           i, j;
  MPI_Status    status;
  mytype_2      varmt2;


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);


  /* Type 1 */
  T1[0] = MPI_DOUBLE;
  T1[1] = MPI_CHAR;
  MPI_Type_struct(2, B1, D1, T1, &mt1);
  MPI_Type_commit(&mt1);

  MPI_Type_size(mt1, &dtsize);
  MPI_Type_extent(mt1, &dtextent);
  MPI_Type_lb(mt1, &dtlb);
  MPI_Type_ub(mt1, &dtub);
  if (myid == 0) {
    printf("SIZE:   %d\n", dtsize);
    printf("EXTENT: %d\n", dtextent);
    printf("LB:     %d\n", dtlb);
    printf("UB:     %d\n", dtub);
  }

  /* Type 2 */
  T[0] = MPI_FLOAT;
  T[1] = mt1;
  T[2] = MPI_CHAR;

  MPI_Type_struct(3, B, D, T, &mt2);
  MPI_Type_commit(&mt2);

  /* Values */
  MPI_Type_size(mt2, &dtsize);
  MPI_Type_extent(mt2, &dtextent);
  MPI_Type_lb(mt2, &dtlb);
  MPI_Type_ub(mt2, &dtub);
  if (myid == 0) {
    printf("SIZE:   %d\n", dtsize);
    printf("EXTENT: %d\n", dtextent);
    printf("LB:     %d\n", dtlb);
    printf("UB:     %d\n", dtub);
  }


  if (myid == 0) {

    varmt2.f_t2 = 0.3;
    varmt2.c_t2 = 'Z';
    varmt2.t_t2.d_t1 = 0.4;
    varmt2.t_t2.c_t1 = 'F';

    printf("Sending message ...\n");
    MPI_Send(&varmt2, 1, mt2, 1, 99, MPI_COMM_WORLD);
    //MPI_Send(MPI_BOTTOM, 1, particle_type, 1, 99, MPI_COMM_WORLD);
    sleep(1);

  } else if (myid == 1) {

    MPI_Recv(&varmt2, 1, mt2, 0, 99, MPI_COMM_WORLD, &status);

    //memset(&particle, 0, sizeof(particle_type));
    printf("Received: \n");
    printf("\tmt2:  %f %c \n", varmt2.f_t2, varmt2.c_t2);
    printf("\tmt1:  %lf %c \n", varmt2.t_t2.d_t1, varmt2.t_t2.c_t1);
  }


  if (MPI_Type_free(&mt2) != MPI_SUCCESS)
    printf("Type Free test\n");

  if (MPI_Type_free(&mt1) != MPI_SUCCESS)
    printf("Type Free test\n");

  MPI_Finalize();
  return MPI_SUCCESS;
}

