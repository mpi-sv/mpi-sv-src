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
#undef MPI_Group_incl
#define MPI_Group_incl  PMPI_Group_incl
#endif


/**
 *  MPI_Group_incl
 */
int MPI_Group_incl (MPI_Group group, int n, int *ranks, MPI_Group *newgroup) {

  int   mpi_errno;
  int  *ranks_in_newgroup = NULL;
  int   i;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Group_incl (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_group(group))                      goto mpi_exception;
#endif

  /* 1. If 0 members, the group is MPI_GROUP_EMPTY */
  if (n == 0) {
    groupGetRef(GROUP_EMPTY_PTR, newgroup);
    return MPI_SUCCESS;
  }

  /* 2. New group to create */
  /* 2.1. Allocate space */
  if (NULL == (ranks_in_newgroup = (int *) malloc (n * sizeof(int)))) {
	mpi_errno = MPI_ERR_INTERN;
	goto mpi_exception;
  }
  
  /* 2.2. Fill the members of newgroup */
  for (i = 0; i < n; i++)
    ranks_in_newgroup[i] = groupGetGlobalRank(group, ranks[i]);

  /* 3. Create the group */
  CALL_FXN (PCS_groupCreate(ranks_in_newgroup, n, newgroup), MPI_ERR_ARG)

  /* 4. Free the allocated space */
  free(ranks_in_newgroup);
  
#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Group_incl (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  if (ranks_in_newgroup) free(ranks_in_newgroup);
  return commHandleError(MPI_COMM_WORLD, mpi_errno, "MPI_Group_incl");
}

