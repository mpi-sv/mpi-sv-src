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
#undef MPI_Group_translate_ranks
#define MPI_Group_translate_ranks  PMPI_Group_translate_ranks
#endif


/**
 *  MPI_Group_translate_ranks
 */
int MPI_Group_translate_ranks (MPI_Group group1, int n, int *ranks1, MPI_Group group2, int *ranks2) {

  int  mpi_errno;
  int  i;
  int  gbrank;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Group_translate_ranks (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_group(group1))                     goto mpi_exception;
  if (mpi_errno = check_group(group2))                     goto mpi_exception;
  if (mpi_errno = check_valid_ranks(group1, n, ranks1))    goto mpi_exception;
#endif

  for (i = 0; i < n; i++) {
    gbrank = groupGetGlobalRank(group1, ranks1[i]);
    ranks2[i] = groupGetRankInGroup (group2, gbrank);
  }

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Group_translate_ranks (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError(MPI_COMM_WORLD, mpi_errno, "MPI_Group_translate_ranks");
}

