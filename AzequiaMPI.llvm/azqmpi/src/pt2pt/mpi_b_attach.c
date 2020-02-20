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
#undef MPI_Buffer_attach
#define MPI_Buffer_attach  PMPI_Buffer_attach
#endif

/*
 *  MPI_Buffer_attach
 */
int MPI_Buffer_attach (void *buffer, int size) {

  int  mpi_errno;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Buffer_attach (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (0 > size)                                           {mpi_errno = MPI_ERR_ARG; goto mpi_exception;}
#endif

  CALL_FXN (PCS_rqstBufferAttach(buffer, size), MPI_ERR_BUFFER);

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Buffer_attach (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError (MPI_COMM_WORLD, mpi_errno, "MPI_Buffer_attach");
}

