#include <config.h>
/* This definitions must be before including proto.h */
#if defined(F77_NAME_LOWER_USCORE)
#define mpi_wait__ mpi_wait_
#elif defined(F77_NAME_LOWER_2USCORE)
/* as is */
#else 
#error "Naming Fortran functions not supported"
#endif


#include "f_proto.h"
#include "f_util.h"


void mpi_wait__ (MPI_Fint *request, MPI_Fint *status, MPI_Fint *ierr) {

  MPI_Status      *c_status;
  Mpi_P_Request   *c_req;

  request_f2c(request, &c_req);
  
  if (IS_FORTRAN_STATUS_IGNORE(status))
    c_status = MPI_STATUS_IGNORE;
  else
    c_status = status;

  *ierr = MPI_Wait (&c_req, c_status);

  if (*ierr == MPI_SUCCESS) {
    request = MPI_REQUEST_NULL;
    status_c2f(c_status, status);
  }

}
