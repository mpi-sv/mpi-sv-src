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
#undef MPI_Gatherv
#define MPI_Gatherv  PMPI_Gatherv
#endif


/*
 *  MPI_Gatherv
 */
int MPI_Gatherv (void *sendbuf, int sendcount, MPI_Datatype sendtype,
                 void *recvbuf, int *recvcounts, int *displs, MPI_Datatype recvtype,
                 int root, MPI_Comm comm )  {

  int          mpi_errno;
  int          i;
  int          rank;
  int          gsize;
  int          dtrecvsz;
  MPI_Request  req;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Gatherv (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_comm_type(comm, INTRACOMM))        goto mpi_exception;
  if (mpi_errno = check_group(commGetLocalGroup(comm)))    goto mpi_exception;
  if (mpi_errno = check_count(sendcount))                  goto mpi_exception;
  if (mpi_errno = check_datatype(sendtype))                goto mpi_exception;
  if (mpi_errno = check_dtype_commit(sendtype))            goto mpi_exception;
#endif

  NEST_FXN_INCR();

  rank  = commGetRank(comm);

  if (rank == root) {
    gsize = commGetSize(comm);

#ifdef CHECK_MODE
    if (mpi_errno = check_root(root, comm))                goto mpi_exception_unnest;
    for (i = 0; i < gsize; i++) {
      if (mpi_errno = check_count(recvcounts[i]))          goto mpi_exception_unnest;
    }
    if (mpi_errno = check_datatype(recvtype))              goto mpi_exception_unnest;
    if (mpi_errno = check_dtype_commit(recvtype))          goto mpi_exception_unnest;
#endif

    dtrecvsz = dtypeGetExtent(recvtype);
    for (i = 0; i < gsize; i++) {
      if (i == root) { /* root copy the buffer without sending it */

        CALL_MPI_NEST(MPI_Irecv((char *) recvbuf + displs[i] * dtrecvsz,
                              recvcounts[i], recvtype, i, GATHER_TAG, comm, &req));

      } else { /* root receive from the other processes */

        CALL_MPI_NEST(MPI_Recv((char *) recvbuf + displs[i] * dtrecvsz,
                             recvcounts[i], recvtype, i, GATHER_TAG, comm, NULL));

      }
    }
  }

  /* Send the data to root, itself included */
  CALL_MPI_NEST(MPI_Send(sendbuf, sendcount, sendtype, root, GATHER_TAG, comm));

  if (rank == root) { /* Root must deallocate the request */
    CALL_MPI_NEST(MPI_Wait(&req, NULL));
  }

  NEST_FXN_DECR();

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Gatherv (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
  NEST_FXN_DECR();

mpi_exception:
  return commHandleError (comm, mpi_errno, "MPI_Gatherv");
}

