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
 /   Declaration of public functions implemented by this module    /
/----------------------------------------------------------------*/
#include <errhnd.h>

  /*----------------------------------------------------------------/
 /   Declaration of types and functions used by this module        /
/----------------------------------------------------------------*/
#if defined (__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <string.h>
#endif

#include <com.h>

#include <mpi.h>
#include <p_errhnd.h>
#include <check.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Errhandler_set
#define MPI_Errhandler_set  PMPI_Errhandler_set
#endif

/*
 *  MPI_Errhandler_set
 */
int MPI_Errhandler_set (MPI_Comm comm, MPI_Errhandler errhandler) {

  int                mpi_errno;
  Mpi_P_Errhandler  *errhnd;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Errhandler_set (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
#endif

  /* 1. Delete the old error handler */
  commGetErrHnd (comm, (void *)&errhnd);
  PCS_errhndDelete(&errhnd);

  /* 2. Set the new error handler for the communicator */
  CALL_FXN (commSetErrHnd (comm, errhandler), MPI_ERR_ARG);

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Errhandler_set (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError (MPI_COMM_WORLD, mpi_errno, "MPI_Errhanler_set");
}
