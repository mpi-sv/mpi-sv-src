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
#undef MPI_Testsome
#define MPI_Testsome  PMPI_Testsome
#endif


/*
 *  MPI_Testsome
 */
int MPI_Testsome (int incount, MPI_Request *array_of_request, int *outcount, int *array_of_indices, MPI_Status *array_of_status) {

  int            mpi_errno;
  int            i, j;
  Mpi_P_Request *request;
  Status        *stazq    = AZQ_STATUS_IGNORE;
  int             size;
  int             pos;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Testsome (start)\tProcess: 0x%x\n", PCS_self());
#endif

  /* 1. Check integrity of parameters */
#ifdef CHECK_MODE
  if (mpi_errno = check_request_nr(incount))               goto mpi_exception;
#endif

  /* 2. testsome */
  if (array_of_status != MPI_STATUSES_IGNORE) {
    if (posix_memalign(&stazq, CACHE_LINE_SIZE, incount * sizeof(Status))) {
	  mpi_errno = MPI_ERR_UNKNOWN;  
	  goto mpi_exception;
	}
  }
//CALL_FXN (testsome(array_of_request, incount, outcount, array_of_indices, stazq),           MPI_ERR_INTERN);
  CALL_FXN (testsome(array_of_request, incount,           array_of_indices, stazq, outcount), MPI_ERR_INTERN);

  /* 3. Check for all request null */
  if (*outcount == MPI_UNDEFINED) {
    fprintf(stdout, "MPI_Testsome(0x%x). END\n", PCS_self()); fflush(stdout);
    return MPI_SUCCESS;
  }

  /* 4. Complete requests */
  for (i = 0; i < *outcount; i++) {

    j = array_of_indices[i];
    request = array_of_request[j];

    /* 2.1. Unpack received buffer if necessary */
    if (RQST_isRecv(request)) {
      if (request->PackedBuffer != NULL) {
        size = packSize(request->Count, (Mpi_P_Datatype *)(request->Datatype));
        pos = 0;
        unpack(request->PackedBuffer, size, &pos, request->OrigBuffer, request->Count, request->Datatype);
      }
    }

    /* 4.1. Get status */
    if (array_of_status != MPI_STATUSES_IGNORE)
      STATUS_setValue(RQST_isRecv(request), request->Comm, &stazq[i], &array_of_status[i], 0);

    /* 4.2. Free non-persistent finished requests */
    if (  RQST_isPersistent(request) || 
        (!RQST_isRecv      (request) && RQST_isCancelled (request)) /* If request is CANCELLED, rqstFree can NOT 
																	 be invoked because request can be in the LFQ of the receptor */
		) {
      fprintf(stdout, "MPI_Testsome(0x%x) for rqst 0x%x. CONTINUE\n", PCS_self(), (int)(request)); fflush(stdout);
      continue;
    }
    CALL_FXN(PCS_rqstFree(&array_of_request[j]), MPI_ERR_REQUEST);

  }
  
  if (array_of_status != MPI_STATUSES_IGNORE) {
	free(stazq);
  }

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Testsome (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError(MPI_COMM_WORLD, mpi_errno, "MPI_Testsome");
}

