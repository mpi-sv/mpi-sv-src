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
#include <azq_types.h>
#include <env.h>
#include <errhnd.h>
#include <p_status.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Wait
#define MPI_Wait  PMPI_Wait
#endif

#define self()        ((Thr_t)pthread_getspecific(key))
/*
 *  MPI_Wait
 */
//#define DEBUG_MODE
int MPI_Wait (MPI_Request *request, MPI_Status *status) {

  int      mpi_errno;
  Status   stazq;
  int      size;
  int      pos;
  bool old_chk_flag = klee_disable_sync_chk(0);
//#define DEBUG_MODE
#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Wait (start)\t Rank: %d\n", commGetRank(MPI_COMM_WORLD));
  fflush(stdout);
#endif

  /* 1. If request NULL returns an empty status */
  if (*request == MPI_REQUEST_NULL) {
    STATUS_setNull(status);
    return MPI_SUCCESS;
  }

  /* 2. wait */
  CALL_FXN (waitone(request, (RQST_Status_t)&stazq), MPI_ERR_INTERN);
  //waitone(request, (RQST_Status_t)&stazq);
  
  /* 3. Unpack received buffer if necessary */
  if (RQST_isRecv(*request)) {
    if ((*request)->PackedBuffer != NULL) {
      size = packSize((*request)->Count, (Mpi_P_Datatype *)((*request)->Datatype));
      pos = 0;
      unpack((*request)->PackedBuffer, size, &pos, (*request)->OrigBuffer, (*request)->Count, (*request)->Datatype);
    }
  }

  /* 4. Set the status fields needed */
  STATUS_setValue(RQST_isRecv(*request), (*request)->Comm, &stazq, status, 0);
  
  /* 5. Free non-persistent requests */
  if (    RQST_isPersistent(*request) || 
	  (!RQST_isRecv      (*request) && RQST_isCancelled (*request)) /* If request is CANCELLED, rqstFree can NOT be invoked 
                                                                           because request can be in the LFQ of the receptor */
	  ) {
    return MPI_SUCCESS;
  }
	
  CALL_FXN(PCS_rqstFree(request), MPI_ERR_REQUEST);

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Wait (end)  \tProcess:%d\n", commGetRank(MPI_COMM_WORLD)); fflush(stdout);
#endif
  if(old_chk_flag)   	klee_enable_sync_chk(0);
  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  if(old_chk_flag)   	klee_enable_sync_chk(0);
  return commHandleError(MPI_COMM_WORLD, mpi_errno, "MPI_Wait");
}

#undef DEBUG_MODE

