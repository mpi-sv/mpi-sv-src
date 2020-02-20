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
#undef MPI_Comm_rank
#define MPI_Comm_rank  PMPI_Comm_rank
#endif


/**
 *  MPI_Comm_rank
 */
int MPI_Comm_rank (MPI_Comm comm, int *rank) {

  int  mpi_errno;
  int old_chk_flag = klee_disable_sync_chk(0);
#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Comm_rank (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
#endif

  *rank = commGetRank(comm);

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Comm_rank (end)  \tProcess: 0x%x\n", PCS_self());
#endif
  if(old_chk_flag)    	klee_enable_sync_chk(0);
  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  if(old_chk_flag)    	klee_enable_sync_chk(0);
  return commHandleError(MPI_COMM_WORLD, mpi_errno, "MPI_Comm_rank");
}


