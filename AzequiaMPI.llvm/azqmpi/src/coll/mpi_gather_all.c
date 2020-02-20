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
#if defined (__OSI)
  #include <osi.h>
#else
  #include <string.h>
#endif

#include <env.h>
#include <errhnd.h>
#include <check.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Allgather
#define MPI_Allgather  PMPI_Allgather
#endif


/*
 *  MPI_Allgather
 */
int MPI_Allgather ( void *sendbuf, int sendcount, MPI_Datatype sendtype,
                    void *recvbuf, int recvcount, MPI_Datatype recvtype,
                    MPI_Comm comm )   {
  int  mpi_errno;
  int  root;
  int  gsize;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Allgather (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_comm_type(comm, INTRACOMM))        goto mpi_exception;
  if (mpi_errno = check_group(commGetLocalGroup(comm)))    goto mpi_exception;
  if (mpi_errno = check_count(sendcount))                  goto mpi_exception;
  if (mpi_errno = check_count(recvcount))                  goto mpi_exception;
  if (mpi_errno = check_datatype(sendtype))                goto mpi_exception;
  if (mpi_errno = check_datatype(recvtype))                goto mpi_exception;
  if (mpi_errno = check_dtype_commit(sendtype))            goto mpi_exception;
  if (mpi_errno = check_dtype_commit(recvtype))            goto mpi_exception;
#endif

  NEST_FXN_INCR();

  gsize = commGetSize(comm);

  for (root = 0; root < gsize; root++) {

    CALL_MPI_NEST(MPI_Gather(sendbuf, sendcount, sendtype,
                           recvbuf, recvcount, recvtype, root, comm));

  }

  NEST_FXN_DECR();

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Allgather (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
  NEST_FXN_DECR();

mpi_exception:
  return commHandleError (comm, mpi_errno, "MPI_Allgather");
}

