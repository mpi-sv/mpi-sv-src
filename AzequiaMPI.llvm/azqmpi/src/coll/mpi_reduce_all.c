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

#include <string.h>

#include <env.h>
#include <errhnd.h>
#include <check.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Allreduce
#define MPI_Allreduce  PMPI_Allreduce
#endif


/*
 *  MPI_Allreduce
 */
int MPI_Allreduce (void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm) {
  int old_chk_flag = klee_disable_sync_chk(0);
  int   mpi_errno;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Allreduce (start)\tProcess: 0x%x\n", PCS_self());
#endif

  NEST_FXN_INCR();

  /* 1. Call reduce with root = 0 */
  CALL_MPI_NEST(MPI_Reduce(sendbuf, recvbuf, count, datatype, op, 0, comm));

  /* 2. Broadcast the result to other processes */
  CALL_MPI_NEST(MPI_Bcast(recvbuf, count, datatype, 0, comm));

  NEST_FXN_DECR();

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Allreduce (end)  \tProcess: 0x%x\n", PCS_self());
#endif
  if(old_chk_flag)   	klee_enable_sync_chk(0);
  return MPI_SUCCESS;

mpi_exception_unnest:
  NEST_FXN_DECR();

mpi_exception:
if(old_chk_flag)   	klee_enable_sync_chk(0);
  return commHandleError (comm, mpi_errno, "MPI_Allreduce");
}

