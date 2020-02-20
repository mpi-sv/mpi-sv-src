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
#include <p_status.h>
#include <p_rqst.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Testany
#define MPI_Testany  PMPI_Testany
#endif


/*
 *  MPI_Testany
 */
int MPI_Testany (int count, MPI_Request *array_of_request, int *index, int *flag, MPI_Status *status) {

  int             mpi_errno;
  Mpi_P_Request  *request;
  Status          stazq;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Testany (start)\tProcess: 0x%x\n", PCS_self());
#endif

  /* 1. Check integrity of parameters */
#ifdef CHECK_MODE
  if (mpi_errno = check_request_nr(count))                 goto mpi_exception;
#endif

  /* 2. testany */
  CALL_FXN (testany(array_of_request, count, index, flag, &stazq), MPI_ERR_INTERN);

  /* 3. Set status */
  if (*flag) {

    if (*index == MPI_UNDEFINED) {
      STATUS_setEmpty(status);
      return MPI_SUCCESS;
    }

    request = array_of_request[*index];
    klee_mpi_nonblock(-1, -1, 7,(void *)&request);
    STATUS_setValue(RQST_isRecv(request), request->Comm, &stazq, status, 0);

    /* 3.1. Free non-persistent requests */
    if (   RQST_isPersistent(request) || 
         (!RQST_isRecv      (request) && RQST_isCancelled (request)) /* If request is CANCELLED, rqstFree can NOT 
                                                                         be invoked because request can be in LFQ or MBX of receiver */
         ) {
        fprintf(stdout, "MPI_Testany(0x%x) for rqst 0x%x. DO NOT FREE it\n", PCS_self(), (int)(request)); fflush(stdout);
        goto retorno;
    }
    CALL_FXN(PCS_rqstFree(&array_of_request[*index]), MPI_ERR_REQUEST);
	
  }

retorno:
#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Testany (end)  \tProcess: 0x%x\n", PCS_self());
#endif
  fprintf(stdout, "MPI_Testany(0x%x). Flag %d. Index %d. END\n", PCS_self(), *flag, *index); fflush(stdout);
  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError(MPI_COMM_WORLD, mpi_errno, "MPI_Testany");
}

