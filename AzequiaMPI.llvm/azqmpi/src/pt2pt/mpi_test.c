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

#include <env.h>
#include <errhnd.h>
#include <p_status.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Test
#define MPI_Test  PMPI_Test
#endif


/*
 *  MPI_Test
 */
int MPI_Test (MPI_Request *request, int *flag, MPI_Status *status) {

  int      mpi_errno;
  Status   stazq;
  int      size;
  int      pos;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Test (start)\tProcess: 0x%x\n", PCS_self());
#endif

  /* 1. A NULL request returns an empty status */
  if (*request == MPI_REQUEST_NULL) {
    *flag = TRUE;
    STATUS_setEmpty(status);
    return MPI_SUCCESS;
  }

  /* 2. test */
  CALL_FXN (test(request, flag, &stazq), MPI_ERR_INTERN);

  /* 3. Complete communication */
  if (*flag) {

    /* 3.1. Unpack received buffer if necessary */
    if (RQST_isRecv(*request)) {
      if ((*request)->PackedBuffer != NULL) {
        size = packSize((*request)->Count, (Mpi_P_Datatype *)((*request)->Datatype));
        pos = 0;
        unpack((*request)->PackedBuffer, size, &pos, (*request)->OrigBuffer, (*request)->Count, (*request)->Datatype);
      }
    }

    /* 3.2. Get status */
    //set MPI buffer to FIN
    klee_mpi_nonblock(-1, -1, 7,(void *)request);
  }

  /* 4. Free non-persistent requests */
  if(*flag) {
    if (    RQST_isPersistent(*request) || 
		(!RQST_isRecv      (*request) && RQST_isCancelled (*request)) /* If request is CANCELLED, rqstFree can NOT 
																	   be invoked because request can be in the LFQ of the receptor */
		) {
      return MPI_SUCCESS;
    }
    CALL_FXN(PCS_rqstFree(request), MPI_ERR_REQUEST);
  }
  
#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Test (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError(MPI_COMM_WORLD, mpi_errno, "MPI_Test");
}





