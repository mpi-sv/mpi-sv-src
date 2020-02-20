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
#undef MPI_Group_difference
#define MPI_Group_difference  PMPI_Group_difference
#endif


/**
 *  MPI_Group_difference
 */
int MPI_Group_difference (MPI_Group group1, MPI_Group group2, MPI_Group *newgroup) {

  int   mpi_errno;
  int  *ranks_in_newgroup = NULL;
  int   i, j;
  int   size1, size2, totsize;
  int   gbrank;
  int   result;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Group_difference (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_group(group1))                     goto mpi_exception;
  if (mpi_errno = check_group(group2))                     goto mpi_exception;
#endif

  size1 = groupGetSize(group1);
  size2 = groupGetSize(group2);

  /* 1. Trivial cases */
  if (size1 == 0) {
    groupGetRef(GROUP_EMPTY_PTR, newgroup);
    return MPI_SUCCESS;
  }

  if (size2 == 0) {
    groupGetRef(group1, newgroup);
    return MPI_SUCCESS;
  }

  NEST_FXN_INCR();

  /* 2. If the groups have the same members, the result group is empty */
  CALL_MPI_NEST(MPI_Group_compare(group1, group2, &result));

  if (result != MPI_UNEQUAL) {
    groupGetRef(GROUP_EMPTY_PTR, newgroup);
    NEST_FXN_DECR();
    return MPI_SUCCESS;
  }

  /* 3. New group to create */
  /* 3.1. Size of the new group */
  totsize = 0;
  for (i = 0; i < size1; i++) {
    gbrank = groupGetGlobalRank(group1, i);
    if (0 > groupGetRankInGroup (group2, gbrank))
	  totsize++;
  }
  
  /* 3.2. Allocate space */
  if (NULL == (ranks_in_newgroup = (int *) malloc (totsize * sizeof(int)))) {
	mpi_errno = MPI_ERR_INTERN;
	goto mpi_exception;
  }
  
  /* 3.3. Fill the members of the new group */
  j = 0;
  for (i = 0; i < size1; i++) {
    gbrank = groupGetGlobalRank(group1, i);
    if (0 > groupGetRankInGroup (group2, gbrank))
	  ranks_in_newgroup[j++] = gbrank;
  }

  /* 4. Create the group */
  CALL_FXN (PCS_groupCreate(ranks_in_newgroup, j, newgroup), MPI_ERR_GROUP)

  /* 5. Free allocated space */
  free (ranks_in_newgroup);
  
  NEST_FXN_DECR();

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Group_difference (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
  NEST_FXN_DECR();

mpi_exception:
  if (ranks_in_newgroup) free (ranks_in_newgroup);
  return commHandleError(MPI_COMM_WORLD, mpi_errno, "MPI_Group_difference");
}

