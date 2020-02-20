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
#undef MPI_Alltoall
#define MPI_Alltoall  PMPI_Alltoall
#endif

/*
 *  MPI_Alltoall
 */
int MPI_Alltoall (void *sendbuf, int sendcount, MPI_Datatype sendtype,
                  void *recvbuf, int recvcount, MPI_Datatype recvtype,
                  MPI_Comm comm )  {

  int mpi_errno;
  int i;
  int rank;
  int gsize;
  int dtsendsz, dtrecvsz;
  int src,dst;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Alltoall (start)\tProcess: 0x%x\n", PCS_self());
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

  if (sendcount == 0)  return MPI_SUCCESS;

  rank  = commGetRank(comm);
  gsize = commGetSize(comm);

  dtsendsz = dtypeGetExtent(sendtype) * sendcount;
  dtrecvsz = dtypeGetExtent(recvtype) * recvcount;

  NEST_FXN_INCR();

  for (i = 0; i < gsize; i++) {
    src = (rank - i + gsize) % gsize;
    dst = (rank + i) % gsize;

    CALL_MPI_NEST(MPI_Sendrecv(
                 ((char *)sendbuf + dst * dtsendsz), sendcount, sendtype, dst, ALLTOALL_TAG,
                 ((char *)recvbuf + src * dtrecvsz), recvcount, recvtype, src, ALLTOALL_TAG,
                 comm, NULL));
  }

  NEST_FXN_DECR();

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Alltoall (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
  NEST_FXN_DECR();

mpi_exception:
  return commHandleError (comm, mpi_errno, "MPI_Alltoall");
}

