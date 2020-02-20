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
#include <mpi.h>

#undef wait
#include <stdlib.h>
#include <string.h>

#include <env.h>
#include <errhnd.h>
#include <check.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Reduce_scatter
#define MPI_Reduce_scatter  PMPI_Reduce_scatter
#endif


/**
 *  MPI_Reduce_scatter
 */
int MPI_Reduce_scatter (void *sendbuf, void *recvbuf, int *recvcounts, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm) {

  int     mpi_errno;
  int     rank;
  int     i;
  int     sumcount;
  int     displs[MAX_MPI_PROCS];
  char   *tmpbuf;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Reduce_scatter (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_comm_type(comm, INTRACOMM))        goto mpi_exception;
  if (mpi_errno = check_datatype(datatype))                goto mpi_exception;
  if (mpi_errno = check_dtype_commit(datatype))            goto mpi_exception;
  for (i = 0; i < commGetSize(comm); i++)
    if (mpi_errno = check_count(recvcounts[i]))            goto mpi_exception;
#endif

  NEST_FXN_INCR();

  rank  = commGetRank(comm);

  /* 1. Run the simplest algorithm, reduce-scatter = reduce + scatter */
  sumcount = 0;
  for (i = 0; i < commGetSize(comm); i++) {
    displs[i] = sumcount;
    sumcount += recvcounts[i];
  }
  CALL_FXN (MALLOC(tmpbuf, sumcount * dtypeGetExtent(datatype)), MPI_ERR_INTERN);

  /* 2. Reduce operation */
  CALL_MPI_NEST(MPI_Reduce (sendbuf, tmpbuf, sumcount, datatype, op, 0, comm));

  /* 3. Scatter the buffer in root to all the other processes */
  CALL_MPI_NEST(MPI_Scatterv(tmpbuf, recvcounts, displs, datatype, recvbuf, recvcounts[rank], datatype, 0, comm));

  FREE(tmpbuf);

  NEST_FXN_DECR();

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Reduce_scatter (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
  NEST_FXN_DECR();

mpi_exception:
  FREE(tmpbuf);
  return commHandleError (comm, mpi_errno, "MPI_Reduce_scatter");
}

