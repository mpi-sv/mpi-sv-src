/* _________________________________________________________________________
   |                                                                       |
   |  Azequia (embedded) Message Passing Interface   ( AzequiaMPI )        |
   |                                                                       |
   |  Authors: DSP Systems Group                                           |
   |           http://gsd.unex.es                                          |
   |           University of Extremadura                                   |
   |           Caceres, Spain                                              |
   |           jarico@unex.es                                              |
   |                                                                       |
   |  Date:    Sept 22, 2008                                               |
   |                                                                       |
   |  Description:                                                         |
   |                                                                       |
   |                                                                       |
   |_______________________________________________________________________| */

  /*----------------------------------------------------------------/
 /   Declaration of types and functions used by this module        /
/----------------------------------------------------------------*/
#include <mpi.h>

#include <env.h>
#include <errhnd.h>
#include <check.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Cartdim_get
#define MPI_Cartdim_get  PMPI_Cartdim_get
#endif

/**
 *  MPI_Cartdim_get
 */
int MPI_Cartdim_get (MPI_Comm comm, int *ndims) {

  int  mpi_errno;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Cartdim_get (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_comm_type(comm, INTRACOMM))        goto mpi_exception;
  if (mpi_errno = check_topo_type(comm, TOPO_CART))        goto mpi_exception;
#endif

  *ndims = commCartGetDim(comm);

#ifdef CHECK_MODE
  if (mpi_errno = check_ndims(*ndims))                     goto mpi_exception;
#endif

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Cartdim_get (end)\tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError(comm, mpi_errno, "MPI_Cartdim_Get");
}

