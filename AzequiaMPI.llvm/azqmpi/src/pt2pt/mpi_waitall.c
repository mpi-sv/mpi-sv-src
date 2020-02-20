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
#include <p_config.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Waitall
#define MPI_Waitall  PMPI_Waitall
#endif


/*
 *  MPI_Waitall
 */
int MPI_Waitall (int count, MPI_Request *array_of_request, MPI_Status *array_of_statuses) {

  int             mpi_errno;
  int             i;
  Status         *stazq    = AZQ_STATUS_IGNORE;
  Mpi_P_Request  *request;
  int             size;
  int             pos;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Waitall (start)\tProcess: 0x%x\n", PCS_self());
#endif
  /* 1. Check integrity of parameters */
#ifdef CHECK_MODE
  if (mpi_errno = check_request_nr(count))                 goto mpi_exception;
  if (array_of_request == NULL)                           {mpi_errno = MPI_ERR_ARG;  goto mpi_exception;}
#endif

  /* 1. waitall */
  if (array_of_statuses != MPI_STATUSES_IGNORE) {
    if (posix_memalign(&stazq, CACHE_LINE_SIZE, count * sizeof(Status))) {
	  mpi_errno = MPI_ERR_UNKNOWN;  
	  goto mpi_exception;
	}
  }

  
  mpi_errno = waitall(array_of_request, count, stazq);

  /* 2. Set statuses */
  for (i = 0; i < count; i++) {

    /* 2.1. Empty status for null requests */
    if (array_of_request[i] == MPI_REQUEST_NULL) {
      if (array_of_statuses != MPI_STATUSES_IGNORE) {
		STATUS_setNull(&array_of_statuses[i]);
	  }
      continue;
    }

    request = array_of_request[i];

    /* 2.2. Unpack received buffer if necessary */
    if (RQST_isRecv(request)) {
      if (request->PackedBuffer != NULL) {
        size = packSize(request->Count, (Mpi_P_Datatype *)(request->Datatype));
        pos = 0;
        unpack(request->PackedBuffer, size, &pos, request->OrigBuffer, request->Count, request->Datatype);
      }
    }

    /* 2.3. Set status */
    if (array_of_statuses != MPI_STATUSES_IGNORE)
      STATUS_setValue(RQST_isRecv(request), request->Comm, &stazq[i], &array_of_statuses[i], mpi_errno);

    /* 2.4. Free non-persistent requests */
    if (  RQST_isPersistent(request) || 
        (!RQST_isRecv      (request) && RQST_isCancelled (request)) /*   If request is CANCELLED, rqstFree can NOT 
																	 be invoked because request can be in the LFQ of the receptor */
		) {
      continue;
    }
    CALL_FXN(PCS_rqstFree(&array_of_request[i]), MPI_ERR_REQUEST);
  }
  
  if (array_of_statuses != MPI_STATUSES_IGNORE) {
	free(stazq);
  }

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Waitall (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  if (!mpi_errno)
    return MPI_SUCCESS;

  mpi_errno = MPI_ERR_IN_STATUS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError(MPI_COMM_WORLD, mpi_errno, "MPI_Waitall");
}


