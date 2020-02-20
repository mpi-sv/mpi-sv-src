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
#include <config.h>

#if defined (__OSI)
  #include <osi.h>
#else
  #include <pthread.h>
  #include <stdio.h>
#endif

#include <mpi.h>
#include <env.h>
#include <errhnd.h>
#include <check.h>

#include <com.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Buffer_detach
#define MPI_Buffer_detach  PMPI_Buffer_detach
#endif


/*
 *  MPI_Buffer_detach
 *    The argument buffer_addr is a pointer address (standard)
 */
int MPI_Buffer_detach (void *buffer_addr, int *size) {

  int  mpi_errno;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Buffer_detach (start)\tProcess: 0x%x\n", PCS_self());
#endif

  CALL_FXN (PCS_rqstBufferDetach(buffer_addr, size), MPI_ERR_INTERN);

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Buffer_detach (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError (MPI_COMM_WORLD, mpi_errno, "MPI_Buffer_detach");
}

