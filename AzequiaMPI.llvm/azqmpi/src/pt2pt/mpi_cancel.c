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

#include <config.h>

#if defined (__OSI)
  #include <osi.h>
#else
  #include <string.h>
#endif

#include <com.h>

#include <env.h>
#include <errhnd.h>
#include <check.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Cancel
#define MPI_Cancel PMPI_Cancel
#endif


/*
 *  MPI_Cancel
 */
int MPI_Cancel (MPI_Request *request) {

  int  mpi_errno;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Cancel (start)\tProcess: 0x%x\n", PCS_self());
#endif

  /* 1. If request NULL returns OK */
  if (*request == MPI_REQUEST_NULL)
    return MPI_SUCCESS;

  /* 2. Call to Azequia cancel */
  CALL_FXN (cancel(*request), MPI_ERR_INTERN);

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Cancel (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError(MPI_COMM_WORLD, mpi_errno, "MPI_Cancel");
}

