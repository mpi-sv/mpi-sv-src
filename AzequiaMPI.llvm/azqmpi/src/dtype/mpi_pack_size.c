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


#if defined (__OSI)
  #include <osi.h>
#else
  #include <pthread.h>
  #include <stdio.h>
  #include <string.h>
#endif

#include <mpi.h>
#include <env.h>

#include <p_dtype.h>

#include <check.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Pack_size
#define MPI_Pack_size  PMPI_Pack_size
#endif


/*
 *  MPI_Pack_size
 *    Size of buffer needed for packing data
 */
int MPI_Pack_size (int incount, MPI_Datatype datatype, MPI_Comm comm, int *size) {

  int  mpi_errno;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Pack_size (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_datatype(datatype))                goto mpi_exception;
  if (mpi_errno = check_dtype_commit(datatype))            goto mpi_exception;
#endif

  *size = packSize(incount, datatype);
  
  if (0 >= *size)                                          goto mpi_exception;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Pack_size (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError (comm, mpi_errno, "MPI_Pack_size");
}
