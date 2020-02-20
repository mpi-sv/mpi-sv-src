#include <config.h>
/* This definitions must be before including proto.h */
#if defined(F77_NAME_LOWER_USCORE)
#define mpi_alltoall__ mpi_alltoall_
#elif defined(F77_NAME_LOWER_2USCORE)
/* as is */
#else 
#error "Naming Fortran functions not supported"
#endif


#include "f_proto.h"

void mpi_alltoall__ (char *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, char *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) {

  Mpi_P_Comm      *comm_ptr;
  Mpi_P_Datatype  *s_dtype_ptr;
  Mpi_P_Datatype  *r_dtype_ptr;

  comm_f2c  (comm,     &comm_ptr);
  dtype_f2c (sendtype, &s_dtype_ptr);
  dtype_f2c (recvtype, &r_dtype_ptr);
  
  *ierr = MPI_Alltoall(sendbuf, *sendcount, s_dtype_ptr, recvbuf, *recvcount, r_dtype_ptr, comm_ptr);

}
