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

#include <common.h>
#include <env.h>
#include <errhnd.h>
#include <check.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Comm_free
#define MPI_Comm_free  PMPI_Comm_free
#endif


/**
 *  MPI_Comm_free
 */
int MPI_Comm_free (MPI_Comm *comm) {

  int       mpi_errno;
  MPI_Group group;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Comm_free (start)\tProcess: 0x%x\n", PCS_self());
#endif

  if (*comm == (MPI_Comm) MPI_COMM_NULL)
    return MPI_SUCCESS;

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(*comm))                       goto mpi_exception;
#endif

  /* 1. Free all attributes for the communicator */
  CALL_FXN (PCS_keyDelAllAttr(*comm),  MPI_ERR_OTHER);

  /* 2. Free group and communicator structures */
  group = commGetLocalGroup(*comm);
  CALL_FXN (PCS_groupDelete(&group),   MPI_ERR_INTERN);

  if (commGetType(*comm) == INTERCOMM) {
    group = commGetRemoteGroup(*comm);
    CALL_FXN (PCS_groupDelete(&group), MPI_ERR_INTERN);
  }

  CALL_FXN (comm_free (comm),      MPI_ERR_INTERN);

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Comm_free (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError(*comm, mpi_errno, "MPI_Comm_free");
}

