#include <config.h>
/* This definitions must be before including proto.h */
#if defined(F77_NAME_LOWER_USCORE)
#define mpi_isend__ mpi_isend_
#elif defined(F77_NAME_LOWER_2USCORE)
/* as is */
#else 
#error "Naming Fortran functions not supported"
#endif


#include "f_proto.h"

void mpi_isend__ (char *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr) {

  Mpi_P_Comm      *comm_ptr;
  Mpi_P_Datatype  *dtype_ptr; 
  Mpi_P_Request   *c_req;

  
  dtype_f2c (datatype, &dtype_ptr);
  comm_f2c  (comm,     &comm_ptr);
  
  *ierr = MPI_Isend (buf, *count, dtype_ptr, *dest, *tag, comm_ptr, &c_req);

  if (*ierr == MPI_SUCCESS)
    request_c2f(c_req, request);

}
