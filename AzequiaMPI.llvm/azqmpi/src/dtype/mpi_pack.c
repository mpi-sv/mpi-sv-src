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
#undef MPI_Pack
#define MPI_Pack  PMPI_Pack
#endif


/*
 *  MPI_Pack
 *    Packing data for sending
 */
int MPI_Pack (void *inbuf,  int incount, MPI_Datatype datatype,
              void *outbuf, int outsize, int *position, MPI_Comm comm) {

  int  mpi_errno;
  int  realsize;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Pack (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_datatype(datatype))                goto mpi_exception;
  if (mpi_errno = check_dtype_commit(datatype))            goto mpi_exception;
#endif

  realsize = packSize(incount, datatype);
  if (outsize < realsize)                                  {mpi_errno = MPI_ERR_BUFFER; goto mpi_exception;}

  CALL_FXN (pack(inbuf, incount, datatype, outbuf, outsize, position), MPI_ERR_INTERN);

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Pack (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError (comm, mpi_errno, "MPI_Pack");
}

