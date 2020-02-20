#include <config.h>
/* This definitions must be before including proto.h */
#if defined(F77_NAME_LOWER_USCORE)
#define mpi_wtick__ mpi_wtick_
#elif defined(F77_NAME_LOWER_2USCORE)
/* as is */
#else 
#error "Naming Fortran functions not supported"
#endif


#include "f_util.h"


double mpi_wtick__ (void) {

  return MPI_Wtick();

}
