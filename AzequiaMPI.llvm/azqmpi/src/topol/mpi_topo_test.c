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
#undef MPI_Topo_test
#define MPI_Topo_test  PMPI_Topo_test
#endif


/**
 *  MPI_Topo_test
 */
int MPI_Topo_test (MPI_Comm comm, int *status) {

  int  mpi_errno;
  int  type;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Topo_test (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
#endif

  if ((type = commTopoType(comm)) == TOPO_E_ALLOCATED)
                                                          {mpi_errno = MPI_ERR_TOPOLOGY; goto mpi_exception;}

  *status = type;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Topo_test (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError(comm, mpi_errno, "MPI_Topo_test");
}



