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
#undef MPI_Group_compare
#define MPI_Group_compare  PMPI_Group_compare
#endif


/**
 *  MPI_Group_compare
 */
int MPI_Group_compare (MPI_Group group1, MPI_Group group2, int *result) {

  int  mpi_errno;
  int  i, j;
  int  size1, size2;
  int  rank1, rank2;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Group_compare (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_group(group1))                     goto mpi_exception;
  if (mpi_errno = check_group(group2))                     goto mpi_exception;
#endif

  /* 1. Same reference, same group */
  if (group1 == group2) {
    *result = MPI_IDENT;
    return MPI_SUCCESS;
  }

  /* 2. Not the same member count */
  size1 = groupGetSize(group1);
  size2 = groupGetSize(group2);

  if (size1 != size2) {
    *result = MPI_UNEQUAL;
    return MPI_SUCCESS;
  }

  /* 3. Same size, same order */
  for (i = 0; i < size1; i++) {
    rank1 = groupGetGlobalRank(group1, i);
	rank2 = groupGetGlobalRank(group2, i);
	if (rank1 != rank2)  break;
  }
  if (i == size1) {
    *result = MPI_IDENT;
    return MPI_SUCCESS;
  }

  /* 4. Same size, same ranks, different order */
  for (i = 0; i < size1; i++) {
    rank1 = groupGetGlobalRank(group1, i);
	for (j = 0; j < size2; j++) {
      if (0 > groupGetRankInGroup (group2, rank1)) {
        *result = MPI_UNEQUAL;
        return MPI_SUCCESS;
      }
    }
  }

  *result = MPI_SIMILAR;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Group_compare (end) \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError(MPI_COMM_WORLD, mpi_errno, "MPI_Group_compare");
}


