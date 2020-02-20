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

#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
#endif

#include <thr.h>
#include <com.h>

#include <env.h>
#include <errhnd.h>
#include <check.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Group_rank
#define MPI_Group_rank  PMPI_Group_rank
#endif


/**
 *  MPI_Group_rank
 */
int MPI_Group_rank (MPI_Group group, int *rank) {

  int  mpi_errno;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Group_rank (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_group(group))                      goto mpi_exception;
#endif

  *rank = groupGetLocalRank(group);

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Group_rank (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError(MPI_COMM_WORLD, mpi_errno, "MPI_Group_rank");
}


