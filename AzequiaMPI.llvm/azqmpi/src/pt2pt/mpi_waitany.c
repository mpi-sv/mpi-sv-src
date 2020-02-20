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
#undef MPI_Waitany
#define MPI_Waitany  PMPI_Waitany
#endif


/*
 *  MPI_Waitany
 */
int MPI_Waitany (int count, MPI_Request *array_of_request, int *index, MPI_Status *status) {

  int             mpi_errno;
  Mpi_P_Request  *request;
  Status          stazq;
  int             size;
  int             pos;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Waitany (start)\tProcess: 0x%x\n", PCS_self());
#endif
  /* 1. Check integrity of parameters */
#ifdef CHECK_MODE
  if (mpi_errno = check_request_nr(count))                 goto mpi_exception;
  if (array_of_request == NULL)                           {mpi_errno = MPI_ERR_ARG;  goto mpi_exception;}
#endif

  /* 2. waitany */
  CALL_FXN (waitany(array_of_request, count, index, &stazq), MPI_ERR_INTERN);

  if (*index == MPI_UNDEFINED) {
    STATUS_setEmpty(status);
    return MPI_SUCCESS;
  }

  request = array_of_request[*index];

  /* 3. Unpack received buffer if necessary */
  if (RQST_isRecv(request)) {
    if (request->PackedBuffer != NULL) {
      size = packSize(request->Count, (Mpi_P_Datatype *)(request->Datatype));
      pos = 0;
      unpack(request->PackedBuffer, size, &pos, request->OrigBuffer, request->Count, request->Datatype);
    }
  }

  /* 4. Set the status fields needed */
  STATUS_setValue(RQST_isRecv(request), request->Comm, &stazq, status, 0);

  /* 5. Free non-persistent requests */
  if (  RQST_isPersistent(request) || 
      (!RQST_isRecv      (request) && RQST_isCancelled (request)) /* If request is CANCELLED, rqstFree can NOT 
                                                                     be invoked because request can be in the queues of the receptor */
       ) {
    //fprintf(stdout, "MPI_Waitany(0x%x). END\n", PCS_self()); fflush(stdout);
    return MPI_SUCCESS;
  }
  CALL_FXN(PCS_rqstFree(&array_of_request[*index]), MPI_ERR_REQUEST);
  
#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Waitany (end)  \tProcess: 0x%x\n", PCS_self());
#endif
//fprintf(stdout, "MPI_Waitany(0x%x). END\n", PCS_self()); fflush(stdout);
  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError(MPI_COMM_WORLD, mpi_errno, "MPI_Waitany");
}

