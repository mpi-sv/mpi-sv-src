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
#include <config.h>

#include <env.h>
#include <errhnd.h>
#include <check.h>
#include <p_status.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Sendrecv
#define MPI_Sendrecv  PMPI_Sendrecv
#endif


/*
 *  MPI_Sendrecv
 */
int MPI_Sendrecv (void *sendbuf, int sendcount, MPI_Datatype sendtype, int dest, int sendtag,
				  void *recvbuf, int recvcount, MPI_Datatype recvtype, int source, int recvtag,
				  MPI_Comm comm, MPI_Status *status) {

  int          mpi_errno;
  MPI_Request  request;
//#define DEBUG_MODE
#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Sendrecv (start)\tProcess: 0x%x\n", PCS_self()); fflush(stdout);
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_tag(sendtag))                      goto mpi_exception;
  if (mpi_errno = check_tag(recvtag))                      goto mpi_exception;
  if (mpi_errno = check_dest_comm(dest, comm))             goto mpi_exception;
  if (mpi_errno = check_source_comm(source, comm))         goto mpi_exception;
  if (mpi_errno = check_count(sendcount))                  goto mpi_exception;
  if (mpi_errno = check_count(recvcount))                  goto mpi_exception;
  if (mpi_errno = check_datatype(sendtype))                goto mpi_exception;
  if (mpi_errno = check_datatype(recvtype))                goto mpi_exception;
#endif

  NEST_FXN_INCR();

  /* 1. For avoiding interblocking it uses a non-blocking receive */
  CALL_MPI_NEST(MPI_Irecv (recvbuf, recvcount, recvtype, source, recvtag, comm, &request));
  CALL_MPI_NEST(MPI_Send  (sendbuf, sendcount, sendtype, dest,   sendtag, comm));
  
  /* 2. Wait for receiving only if source is not MPI_PROC_NULL */
  if ((source == MPI_PROC_NULL) && (status != MPI_STATUS_IGNORE)) {
    STATUS_setNull(status);
  } else {
	CALL_MPI_NEST(MPI_Wait  (&request, status));
  }
  
  NEST_FXN_DECR();

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Sendrecv (end)  \tProcess: 0x%x\n", PCS_self()); fflush(stdout);
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
  NEST_FXN_DECR();

mpi_exception:
  return commHandleError (comm, mpi_errno, "MPI_Sendrecv");
}
