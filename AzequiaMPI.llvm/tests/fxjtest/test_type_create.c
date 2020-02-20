#include <stdio.h>
#include <mpi.h>


int main(int argc,char** argv){
  int nprocs = -1;
  int rank = -1;
  MPI_Datatype duntype;
  /* init */
  MPI_Init (&argc, &argv);
  MPI_Comm_size (MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);

  MPI_Type_contiguous(1,MPI_INT,&duntype);
  MPI_Type_commit(&duntype);

 MPI_Finalize ();
  printf ("(%d) Finished normally\n", rank);
	return 0;
}

