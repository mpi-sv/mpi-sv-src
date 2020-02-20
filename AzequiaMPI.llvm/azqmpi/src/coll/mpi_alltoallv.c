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
#undef MPI_Alltoallv
#define MPI_Alltoallv  PMPI_Alltoallv
#endif


/*
 *  MPI_Alltoallv
 */
int MPI_Alltoallv (void *sendbuf, int *sendcounts, int *sdispls, MPI_Datatype sendtype,
                   void *recvbuf, int *recvcounts, int *rdispls, MPI_Datatype recvtype,
                   MPI_Comm comm )  {

  int      mpi_errno;
  int      i;
  int      rank;
  int      gsize;
  int      dtsendsz,
           dtrecvsz;
  int      src, dst;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Alltoallv (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_comm_type(comm, INTRACOMM))        goto mpi_exception;
  if (mpi_errno = check_group(commGetLocalGroup(comm)))    goto mpi_exception;
  if (mpi_errno = check_datatype(sendtype))                goto mpi_exception;
  if (mpi_errno = check_datatype(recvtype))                goto mpi_exception;
  if (mpi_errno = check_dtype_commit(sendtype))            goto mpi_exception;
  if (mpi_errno = check_dtype_commit(recvtype))            goto mpi_exception;
#endif

  rank  = commGetRank(comm);
  gsize = commGetSize(comm);

#ifdef CHECK_MODE
  for (i = 0; i < gsize; i++) {
    if (mpi_errno = check_count(sendcounts[i]))            goto mpi_exception;
    if (mpi_errno = check_count(recvcounts[i]))            goto mpi_exception;
  }
#endif

  NEST_FXN_INCR();

  dtsendsz = dtypeGetExtent(sendtype);
  dtrecvsz = dtypeGetExtent(recvtype);

  for (i = 0; i < gsize; i++) {
    src = (rank - i + gsize) % gsize;
    dst = (rank + i) % gsize;

    CALL_MPI_NEST(MPI_Sendrecv(
                 ((char *)sendbuf + sdispls[dst] * dtsendsz), sendcounts[dst], sendtype, dst, ALLTOALL_TAG,
                 ((char *)recvbuf + rdispls[src] * dtrecvsz), recvcounts[src], recvtype, src, ALLTOALL_TAG,
                 comm, NULL));
  }

  NEST_FXN_DECR();

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Alltoallv (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
  NEST_FXN_DECR();

mpi_exception:
  return commHandleError (comm, mpi_errno, "MPI_Alltoallv");
}
