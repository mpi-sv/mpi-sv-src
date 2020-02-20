#include <config.h>
/* This definitions must be before including proto.h */
#if defined(F77_NAME_LOWER_USCORE)
#define mpi_comm_split__ mpi_comm_split_
#elif defined(F77_NAME_LOWER_2USCORE)
/* as is */
#else 
#error "Naming Fortran functions not supported"
#endif


#include "f_proto.h"

void mpi_comm_split__ (MPI_Fint *comm, MPI_Fint *color, MPI_Fint *key, MPI_Fint *newcomm, MPI_Fint *ierr) {

  Mpi_P_Comm  *comm_ptr;
  Mpi_P_Comm  *newcomm_ptr;

  if (*comm == MPI_FORTRAN_COMM_NULL) {
    *newcomm = MPI_FORTRAN_COMM_NULL;
    *ierr    = MPI_SUCCESS;
    return;
  }

  comm_f2c(comm, &comm_ptr);  

  *ierr = MPI_Comm_split(comm_ptr, *color, *key, &newcomm_ptr);
  
  if (*ierr == MPI_SUCCESS)
	comm_c2f(newcomm_ptr, newcomm);

}
