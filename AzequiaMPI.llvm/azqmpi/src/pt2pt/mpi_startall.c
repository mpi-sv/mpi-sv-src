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
#include <config.h>

#include <rqst.h>

#include <env.h>
#include <errhnd.h>
#include <check.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Startall
#define MPI_Startall  PMPI_Startall
#endif


/*
 *  MPI_Startall
 */
int MPI_Startall (int count, MPI_Request *array_of_requests) {

  int    mpi_errno;
  int    i;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Startall (start)\tProcess: 0x%x\n", PCS_self());
#endif

  NEST_FXN_INCR();

  /* 1. Start each operation */
  for (i = 0; i < count; i++) {
    CALL_MPI_NEST(MPI_Start((MPI_Request *)(array_of_requests[i])));
  }

  NEST_FXN_DECR();

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Startall (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
  NEST_FXN_DECR();

mpi_exception:
  return commHandleError (MPI_COMM_WORLD, mpi_errno, "MPI_Startall");
}


