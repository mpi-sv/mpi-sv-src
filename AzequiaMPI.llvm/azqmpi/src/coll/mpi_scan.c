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
  #include <stdlib.h>
  #include <string.h>
#endif

#include <env.h>
#include <errhnd.h>
#include <check.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Scan
#define MPI_Scan  PMPI_Scan
#endif


/*
 *  MPI_Scan
 */
int MPI_Scan (void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)  {

  int          mpi_errno;
  int          i;
  int          rank;
  int          gsize, size;
  int          root = 0;
  char        *tmpbuf;
  MPI_Request  req;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Scan (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_comm_type(comm, INTRACOMM))        goto mpi_exception;
  if (mpi_errno = check_group(commGetLocalGroup(comm)))    goto mpi_exception;
  if (mpi_errno = check_count(count))                      goto mpi_exception;
  if (mpi_errno = check_datatype(datatype))                goto mpi_exception;
  if (mpi_errno = check_dtype_commit(datatype))            goto mpi_exception;
#endif

  NEST_FXN_INCR();

  /* 1. Collect data and allocate buffers */
  rank  = commGetRank(comm);
  gsize = commGetSize(comm);

  CALL_FXN (MALLOC(tmpbuf, count * dtypeGetExtent(datatype)), MPI_ERR_INTERN);

  /* 2. Performs reduces on all ranks < size */
  for (size = gsize; size >= 1; size--) {
    if (rank >= size) continue;

    root = size - 1;

    if (rank == root) {

      if (root == 0) {

        CALL_MPI_NEST(MPI_Irecv (recvbuf, count, datatype, root, REDUCE_TAG, comm, &req));
        CALL_MPI_NEST(MPI_Send (sendbuf, count, datatype, root, REDUCE_TAG, comm));
        CALL_MPI_NEST(MPI_Wait(&req, NULL));

      } else {

        CALL_MPI_NEST(MPI_Recv (recvbuf, count, datatype, 0, REDUCE_TAG, comm, NULL));

      }

      for (i = 1; i < size; i++) {

        if (root == i) {

          CALL_MPI_NEST(MPI_Irecv (tmpbuf, count, datatype, root, REDUCE_TAG, comm, &req));
          CALL_MPI_NEST(MPI_Send (sendbuf, count, datatype, root, REDUCE_TAG, comm));
          CALL_MPI_NEST(MPI_Wait(&req, NULL));

        } else {

          CALL_MPI_NEST(MPI_Recv (tmpbuf, count, datatype, i, REDUCE_TAG, comm, NULL));

        }

        (copsGetFunction(op)) (tmpbuf, recvbuf, &count, datatype);
      }

    } else {

      CALL_MPI_NEST(MPI_Send (sendbuf, count, datatype, root, REDUCE_TAG, comm));

    }

  }

  FREE(tmpbuf);

  NEST_FXN_DECR();

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Scan (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
  NEST_FXN_DECR();

mpi_exception:
  FREE(tmpbuf);
  return commHandleError (comm, mpi_errno, "MPI_Scan");
}

