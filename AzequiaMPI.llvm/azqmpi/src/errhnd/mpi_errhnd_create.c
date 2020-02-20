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
#undef MPI_Errhandler_create
#define MPI_Errhandler_create  PMPI_Errhandler_create
#endif


/*
 *  MPI_Errhandler_create
 */
int MPI_Errhandler_create (MPI_Handler_function *function, MPI_Errhandler *errhandler) {

  int  mpi_errno;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Errhandler_create (start)\tProcess: 0x%x\n", PCS_self());
#endif

  CALL_FXN (PCS_errhndCreate ((Mpi_P_Handler_function *)function, errhandler), MPI_ERR_INTERN);

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Errhandler_create (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError (MPI_COMM_WORLD, mpi_errno, "MPI_Errhanler_create");
}

