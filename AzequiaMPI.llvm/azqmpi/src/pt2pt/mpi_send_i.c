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
#include <check.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Isend
#define MPI_Isend  PMPI_Isend
#endif
#define self()        ((Thr_t)pthread_getspecific(key))

/*
 *  MPI_Isend
 */
//#define DEBUG_MODE
int MPI_Isend (void *buf, int count, MPI_Datatype datatype, int dest,
				       int tag, MPI_Comm comm, MPI_Request *request) {

  int    mpi_errno;
  Addr   dst;
  char  *tmpbuf;
  int    size;
  int    pos;
  int    rank;

#ifdef DEBUG_MODE
  fprintf(stdout, "\nMPI_Isend(%d). tag %x. dest %d BEGIN\n", commGetRank(comm), tag,  dest);
  fflush(stdout);
#endif

  if (dest == MPI_PROC_NULL) {
	*request = MPI_REQUEST_NULL;
	return (MPI_SUCCESS);
  }
  
  /* 1. Check integrity of parameters */
#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_tag(tag))                          goto mpi_exception;
  if (mpi_errno = check_group(commGetGroup(comm)))         goto mpi_exception;
  if (mpi_errno = check_dest(dest, commGetGroup(comm)))    goto mpi_exception;
  if (mpi_errno = check_count(count))                      goto mpi_exception;
  if (mpi_errno = check_datatype(datatype))                goto mpi_exception;
  if (mpi_errno = check_dtype_commit(datatype))            goto mpi_exception;
#endif

  rank = commGetGlobalRank(comm, dest);

  if (dtypeIsContiguous(datatype)) {

    if (buf == MPI_BOTTOM) buf = (char *)dtypeGetLb(datatype);
    size = dtypeGetExtent(datatype) * count;

    CALL_FXN(PCS_rqstAlloc(request, comm, EMPTY_RQST_TYPE), MPI_ERR_REQUEST);
    
    CALL_FXN (asend( rank,                                      
                     buf,                                      
                     size,                                     
                     EXTENDED_TAG(commGetContext(comm), tag),  
                    *request),                                 
              MPI_ERR_INTERN);
  } 
  else {
    size = packSize(count, datatype);
    CALL_FXN(MALLOC(tmpbuf, size), MPI_ERR_INTERN);

    pos = 0;
    pack(buf, count, datatype, tmpbuf, size, &pos);
    size = pos;

    CALL_FXN(PCS_rqstAlloc(request, comm, EMPTY_RQST_TYPE), MPI_ERR_REQUEST);
    PCS_rqstSet(*request, tmpbuf, NULL, datatype, count);
    
    CALL_FXN (asend( rank,                                      
                     tmpbuf,                                  
                     size,                                     
                     EXTENDED_TAG(commGetContext(comm), tag), 
                    *request),                                
              MPI_ERR_INTERN);
  }

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Isend (end)  \tProcess: %d\n", commGetRank(comm)); fflush(stdout);
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError (comm, mpi_errno, "MPI_Isend");
}


