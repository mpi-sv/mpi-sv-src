#include <config.h>
/* This definitions must be before including proto.h */
#if defined(F77_NAME_LOWER_USCORE)
#define mpi_recv__ mpi_recv_
#elif defined(F77_NAME_LOWER_2USCORE)
/* as is */
#else 
#error "Naming Fortran functions not supported"
#endif


#include "f_proto.h"
#include "f_util.h"


void mpi_recv__ (char *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *status, MPI_Fint *ierr) {

  MPI_Status      *c_status;
  Mpi_P_Comm      *comm_ptr;
  Mpi_P_Datatype  *dtype_ptr;
  

  if (IS_FORTRAN_STATUS_IGNORE(status))
    c_status = MPI_STATUS_IGNORE;
  else
    c_status = status;

  comm_f2c  (comm,     &comm_ptr);
  dtype_f2c (datatype, &dtype_ptr);
  
  *ierr = MPI_Recv (buf, *count, dtype_ptr, *source, *tag, comm_ptr, c_status);

  status_c2f(c_status, status);
}
