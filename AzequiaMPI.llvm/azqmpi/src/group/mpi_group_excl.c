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
#undef MPI_Group_excl
#define MPI_Group_excl  PMPI_Group_excl
#endif


/**
 *  MPI_Group_excl
 */
int MPI_Group_excl (MPI_Group group, int n, int *ranks, MPI_Group *newgroup) {

  int   mpi_errno;
  int  *ranks_in_newgroup = NULL;
  int   i, j, k = 0;
  int   size, totsize;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Group_excl (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_group(group))                      goto mpi_exception;
  if (mpi_errno = check_valid_ranks(group, n, ranks))      goto mpi_exception;
#endif

  /* 1. If n == 0 group is the same */
  if (n == 0) {
    groupGetRef(group, newgroup);
    return MPI_SUCCESS;
  }

  /* 2. New group to create */
  size = groupGetSize(group);

  /* 2.1. Size of the new group */
  totsize = 0;
  for (i = 0; i < size; i++) {
    for (j = 0; j < n; j++)
      if (i == ranks[j])   break;
    if (j == n) totsize++;
  }

  /* 2.2. Allocate space for members in the new group */
  if (NULL == (ranks_in_newgroup = (int *) malloc (totsize * sizeof(int)))) {
	mpi_errno = MPI_ERR_INTERN;
	goto mpi_exception;
  }
  
  /* 2.3. Fill members in the new group */
  for (i = 0; i < size; i++) {
    for (j = 0; j < n; j++)
      if (i == ranks[j])   break;
    if (j == n) /* Not found. To the new group */
      ranks_in_newgroup[k++] = groupGetGlobalRank(group, i);
	}

  /* 3. Create the group */
  CALL_FXN (PCS_groupCreate(ranks_in_newgroup, size - n, newgroup), MPI_ERR_ARG);

  /* 4. free allocated space */
  free(ranks_in_newgroup);
  
#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Group_excl (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  if (ranks_in_newgroup)  free(ranks_in_newgroup);
  return commHandleError(MPI_COMM_WORLD, mpi_errno, "MPI_Group_excl");
}

