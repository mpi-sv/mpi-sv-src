#include <config.h>
/* This definitions must be before including proto.h */
#if defined(F77_NAME_LOWER_USCORE)
#define mpi_alltoallv__ mpi_alltoallv_
#elif defined(F77_NAME_LOWER_2USCORE)
/* as is */
#else 
#error "Naming Fortran functions not supported"
#endif


#include "f_proto.h"

void mpi_alltoallv__ (char *sendbuf, MPI_Fint *sendcounts, MPI_Fint *sdispls, MPI_Fint *sendtype, char *recvbuf, MPI_Fint *recvcounts, MPI_Fint *rdispls, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) {

  Mpi_P_Comm      *comm_ptr;
  Mpi_P_Datatype  *s_dtype_ptr;
  Mpi_P_Datatype  *r_dtype_ptr;

  comm_f2c  (comm,     &comm_ptr);
  dtype_f2c (sendtype, &s_dtype_ptr);
  dtype_f2c (recvtype, &r_dtype_ptr);
  
  *ierr = MPI_Alltoallv(sendbuf, sendcounts, sdispls, s_dtype_ptr, recvbuf, recvcounts, rdispls, r_dtype_ptr, comm_ptr);

}
