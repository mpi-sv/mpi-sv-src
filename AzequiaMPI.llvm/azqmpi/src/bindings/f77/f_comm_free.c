#include <config.h>
/* This definitions must be before including proto.h */
#if defined(F77_NAME_LOWER_USCORE)
#define mpi_comm_free__ mpi_comm_free_
#elif defined(F77_NAME_LOWER_2USCORE)
/* as is */
#else 
#error "Naming Fortran functions not supported"
#endif


#include "f_proto.h"

void mpi_comm_free__ (MPI_Fint *comm, MPI_Fint *ierr) {

  Mpi_P_Comm  *comm_ptr;

  comm_f2c(comm, &comm_ptr);
  
  *ierr = MPI_Comm_free(&comm_ptr);

  if (*ierr == MPI_SUCCESS)
    *comm = MPI_FORTRAN_COMM_NULL;

}
