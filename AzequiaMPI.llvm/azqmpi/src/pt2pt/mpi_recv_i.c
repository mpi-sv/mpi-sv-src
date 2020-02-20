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
#undef MPI_Irecv
#define MPI_Irecv  PMPI_Irecv
#endif

#define self()        ((Thr_t)pthread_getspecific(key))
/*
 *  MPI_Irecv
 */
//#define DEBUG_MODE
int MPI_Irecv (void *buf, int count, MPI_Datatype datatype, int source,
				int tag, MPI_Comm comm, MPI_Request *request) {

  int      mpi_errno;
  char    *tmpbuf;
  int      size;
  int      rank;

#ifdef DEBUG_MODE
  fprintf(stdout, "\MPI_Irecv(%d). tag %x. src %d BEGIN\n", commGetRank(comm), tag,  source);
  fflush(stdout);
#endif

  if (source == MPI_PROC_NULL) {
	*request = MPI_REQUEST_NULL;
	return (MPI_SUCCESS);
  }
  
  /* 1. check integrity */
#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_tag(tag))                          goto mpi_exception;
  if (mpi_errno = check_group(commGetGroup(comm)))         goto mpi_exception;
  if (mpi_errno = check_source(source,commGetGroup(comm))) goto mpi_exception;
  if (mpi_errno = check_count(count))                      goto mpi_exception;
  if (mpi_errno = check_datatype(datatype))                goto mpi_exception;
  if (mpi_errno = check_dtype_commit(datatype))            goto mpi_exception;
#endif

  rank = (source == MPI_ANY_SOURCE) ? ADDR_RNK_ANY : commGetGlobalRank(comm, source);
  if (dtypeIsContiguous(datatype)) {

    if (buf == MPI_BOTTOM)  buf = (char *)dtypeGetLb(datatype);
    size = dtypeGetExtent(datatype) * count;

    CALL_FXN(PCS_rqstAlloc(request, comm, EMPTY_RQST_TYPE), MPI_ERR_REQUEST);
#ifdef DEBUG_MODE
  fprintf(stdout, "\MPI_Irecv: size=. %d, datatype extent:%d,count%d\n", size,dtypeGetExtent(datatype),count);
  fflush(stdout);
#endif
    CALL_FXN (arecv (rank,                                     
                     buf,                                      
                     size,                                     
                     EXTENDED_TAG(commGetContext(comm), tag),  
                   *request),                                  
              MPI_ERR_INTERN);

  } else {

    size = packSize(count, datatype);
    CALL_FXN(MALLOC(tmpbuf, size), MPI_ERR_INTERN);

    CALL_FXN(PCS_rqstAlloc(request, comm, EMPTY_RQST_TYPE), MPI_ERR_REQUEST);
    PCS_rqstSet(*request, tmpbuf, buf, datatype, count);

    CALL_FXN (arecv (rank,                                     /* Src addr       */
                     tmpbuf,                                   /* Buffer         */
                     size,                                     /* Size           */
                     EXTENDED_TAG(commGetContext(comm), tag),  /* Tag            */
                   *request),                                  /* Request (out)  */
              MPI_ERR_INTERN);

  }
  
#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Irecv (end)  \tProcess: %d\n", commGetRank(comm)); fflush(stdout);
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError (comm, mpi_errno, "MPI_Irecv");
}

