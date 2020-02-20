#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

struct mytype_1 {
  double d_t1;
  char   c_t1;
};
typedef struct mytype_1 mytype_1;


struct mytype_2 {
  float     f_t2_1;
  float     f_t2_2;
  mytype_1  t_t2_1;
  char      c_t2_1;
  char      c_t2_2;
  char      c_t2_3;
  mytype_1  t_t2_2;
};
typedef struct mytype_2 mytype_2;


int main (int argc, char **argv) {

  int           myid, numprocs;
  MPI_Datatype  mt1, mt2;
  MPI_Datatype  T[5];
  int           B[5] = {2, 1, 3, 1, 1};
  MPI_Aint      D[5] = {0, 8, 24, 28, 44};
  MPI_Datatype  T1[4];
  int           B1[4] = {1, 1, 1, 1};
  MPI_Aint      D1[4] = {0, 0, 8, 16};
  int           dtsize, dtextent;
  MPI_Aint      dtlb, dtub;
  int           i, j;
  MPI_Status    status;
  mytype_2      varmt2;


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);


  /* Type 1 */
  T1[0] = MPI_LB;
  T1[1] = MPI_DOUBLE;
  T1[2] = MPI_CHAR;
  T1[3] = MPI_UB;
  MPI_Type_struct(4, B1, D1, T1, &mt1);
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
  T[3] = mt1;
  T[4] = MPI_UB;

  MPI_Type_struct(5, B, D, T, &mt2);
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

    varmt2.f_t2_1 = 0.3456;
    varmt2.f_t2_2 = 0.1234;
    varmt2.c_t2_1 = 'V';
    varmt2.c_t2_2 = 'W';
    varmt2.c_t2_3 = 'Y';
    varmt2.t_t2_1.d_t1 = 0.4;
    varmt2.t_t2_1.c_t1 = 'A';
    varmt2.t_t2_2.d_t1 = 0.8;
    varmt2.t_t2_2.c_t1 = 'B';

    printf("Sending message ...\n");
    MPI_Send(&varmt2, 1, mt2, 1, 99, MPI_COMM_WORLD);

    //MPI_Send(MPI_BOTTOM, 1, particle_type, 1, 99, MPI_COMM_WORLD);
    //sleep(1);

  } else if (myid == 1) {

    MPI_Recv(&varmt2, 1, mt2, 0, 99, MPI_COMM_WORLD, &status);

    //memset(&particle, 0, sizeof(particle_type));
    printf("Received: \n");
    printf("\tmt2:  %f  %f  \n", varmt2.f_t2_1, varmt2.f_t2_2);
    printf("\tmt2:  %c  %c  %c\n", varmt2.c_t2_1, varmt2.c_t2_2, varmt2.c_t2_3);
    printf("\tmt1:  %lf %c \n", varmt2.t_t2_1.d_t1, varmt2.t_t2_1.c_t1);
    printf("\tmt1:  %lf %c \n", varmt2.t_t2_2.d_t1, varmt2.t_t2_2.c_t1);
  }


  if (MPI_Type_free(&mt2) != MPI_SUCCESS)
    printf("Type Free test\n");

  if (MPI_Type_free(&mt1) != MPI_SUCCESS)
    printf("Type Free test\n");

  MPI_Finalize();
  return MPI_SUCCESS;
}

