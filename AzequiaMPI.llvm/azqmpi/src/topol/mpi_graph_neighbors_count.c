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
#undef MPI_Graph_neighbors_count
#define MPI_Graph_neighbors_count  PMPI_Graph_neighbors_count
#endif


/**
 *  MPI_Graph_neighbors_count
 */
int MPI_Graph_neighbors_count (MPI_Comm comm, int rank, int *nneighbors) {

  int  mpi_errno;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Graph_neighbors_count (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_comm_type(comm, INTRACOMM))        goto mpi_exception;
  if (mpi_errno = check_topo_type(comm, TOPO_GRAPH))       goto mpi_exception;
  if (mpi_errno = check_rank_comm(rank, comm))             goto mpi_exception;
#endif

  CALL_FXN(commGraphNborsCnt(comm, rank, nneighbors), MPI_ERR_TOPOLOGY);

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Graph_neighbors_count (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError(comm, mpi_errno, "MPI_Graph_neighbors_count");
}


