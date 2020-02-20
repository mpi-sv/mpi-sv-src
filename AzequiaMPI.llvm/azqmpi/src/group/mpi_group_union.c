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
#undef MPI_Group_union
#define MPI_Group_union  PMPI_Group_union
#endif


/**
 *  MPI_Group_union
 */
int MPI_Group_union (MPI_Group group1, MPI_Group group2, MPI_Group *newgroup) {

  int   mpi_errno;
  int  *ranks_in_newgroup = NULL;
  int   i, j;
  int   size1, size2, totsize;
  int   gbrank;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Group_union (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_group(group1))                     goto mpi_exception;
  if (mpi_errno = check_group(group2))                     goto mpi_exception;
#endif

  size1 = groupGetSize(group1);
  size2 = groupGetSize(group2);

  /* 1. If 0 members, the group is MPI_GROUP_EMPTY */
  if ((size1 + size2) == 0) {
    groupGetRef(GROUP_EMPTY_PTR, newgroup);
    return MPI_SUCCESS;
  }

  /* 2. Trivial cases */
  if (size1 == 0) {
    groupGetRef(group2, newgroup);
    return MPI_SUCCESS;
  }

  if (size2 == 0) {
    groupGetRef(group1, newgroup);
    return MPI_SUCCESS;
  }

  /* 3. New group to create */
  /* 3.1. Find the size of the new group */
  totsize = size1;
  for (j = 0; j < size2; j++) {
    gbrank = groupGetGlobalRank(group2, j);
    if (0 > groupGetRankInGroup (group1, gbrank)) /* If not in group1 */
      totsize++;
  }
  
  /* 3.2. Allocate space for members vector */
  if (NULL == (ranks_in_newgroup = malloc (totsize * sizeof(int)))) {
	mpi_errno = MPI_ERR_INTERN;
	goto mpi_exception;
  }
	
  /* 3.3. Fill the members vector */
  for (i = 0; i < size1; i++)
    ranks_in_newgroup[i] = groupGetGlobalRank(group1, i);
  for (j = 0; j < size2; j++) {
    gbrank = groupGetGlobalRank(group2, j);
    if (0 > groupGetRankInGroup (group1, gbrank)) /* If not in group1 */
      ranks_in_newgroup[i++] = gbrank;
  }

  /* 4. Create the group */
  CALL_FXN (PCS_groupCreate(ranks_in_newgroup, i, newgroup), MPI_ERR_GROUP)
  
  /* 5. free the allocated memory */
  free(ranks_in_newgroup);

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Group_union (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  if (ranks_in_newgroup)  free(ranks_in_newgroup);
  return commHandleError(MPI_COMM_WORLD, mpi_errno, "MPI_Group_union");
}

