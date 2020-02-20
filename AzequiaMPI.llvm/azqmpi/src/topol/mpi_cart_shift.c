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
#undef MPI_Cart_shift
#define MPI_Cart_shift  PMPI_Cart_shift
#endif


/**
 *  MPI_Cart_shift
 */
int MPI_Cart_shift (MPI_Comm comm, int direction, int disp, int *rank_source, int *rank_dest) {

  int   mpi_errno;
  int   rank;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Cart_shift (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_comm_type(comm, INTRACOMM))        goto mpi_exception;
  if (mpi_errno = check_ndims(direction))                  goto mpi_exception;
  if (mpi_errno = check_topo_type(comm, TOPO_CART))        goto mpi_exception;
#endif

  rank = commGetRank(comm);

#ifdef CHECK_MODE
  if (mpi_errno = check_rank_comm(rank, comm))             goto mpi_exception;
#endif

  CALL_FXN(commCartShift(comm, direction, disp, rank, rank_source, rank_dest), MPI_ERR_TOPOLOGY);

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Cart_shift (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError(comm, mpi_errno, "MPI_Cart_shift");
}

