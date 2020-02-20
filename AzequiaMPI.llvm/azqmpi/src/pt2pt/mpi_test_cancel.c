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
#undef MPI_Test_cancelled
#define MPI_Test_cancelled  PMPI_Test_cancelled
#endif


/*
 *  MPI_Test_cancelled
 */
int MPI_Test_cancelled (MPI_Status *status, int *flag) {

  int  mpi_errno;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Test_cancelled (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
#endif

  if (status == NULL)
    return MPI_SUCCESS;

  *flag = status->Cancelled;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Test_cancelled (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError(MPI_COMM_WORLD, mpi_errno, "MPI_Test_cancelled");
}

