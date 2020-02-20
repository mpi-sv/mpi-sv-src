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

#include <azq_types.h>
#include <rqst.h>
#include <com.h>

#include <env.h>
#include <errhnd.h>
#include <p_status.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Request_free
#define MPI_Request_free  PMPI_Request_free
#endif


/*
 *  MPI_Request_free
 */
int MPI_Request_free (MPI_Request *request) {

  int  mpi_errno;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Request_free (start)\tProcess: 0x%x\n", PCS_self());
#endif

  /* 1. If request NULL returns */
  if (*request == MPI_REQUEST_NULL)
    return MPI_SUCCESS;

  /* 2. wait */
  if (!RQST_isPersistent(*request)) {
    CALL_FXN (waitone(request, NULL), MPI_ERR_INTERN);
  }

  /* 3. Free */
  if (  (!RQST_isRecv(*request) && RQST_isCancelled (*request)) /* If request is CANCELLED, rqstFree can NOT 
                                                                   be invoked because request can be in the LFQ of the receptor */
     ) {
#ifdef DEBUG_MODE
    fprintf(stdout, "MPI_Request_free(%p) rqst not liberated %p. END\n", PCS_self(), *request); fflush(stdout);
#endif
    return MPI_SUCCESS;
  }
  CALL_FXN(PCS_rqstFree(request), MPI_ERR_REQUEST);

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Request_free (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError(MPI_COMM_WORLD, mpi_errno, "MPI_Request_free");
}

