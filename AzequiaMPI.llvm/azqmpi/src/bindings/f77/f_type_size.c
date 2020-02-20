#include <config.h>
/* This definitions must be before including proto.h */
#if defined(F77_NAME_LOWER_USCORE)
#define mpi_type_size__ mpi_type_size_
#elif defined(F77_NAME_LOWER_2USCORE)
/* as is */
#else 
#error "Naming Fortran functions not supported"
#endif


#include "f_proto.h"

void mpi_type_size__ (MPI_Fint *type, MPI_Fint *size, MPI_Fint *ierr) {

  Mpi_P_Datatype  *dtype_ptr;

  dtype_f2c (type, &dtype_ptr);
  
  *ierr = MPI_Type_size(&dtype_ptr, size);

}
