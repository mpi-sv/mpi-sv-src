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
#undef MPI_Bsend_init
#define MPI_Bsend_init  PMPI_Bsend_init
#endif


/*
 *  MPI_Bsend_init
 */
int MPI_Bsend_init (void *buf, int count, MPI_Datatype datatype, int dest,
				       int tag, MPI_Comm comm, MPI_Request *request) {

  int            mpi_errno;
  char          *user_buf;
  int            size;
  SegmentInfo_t  segment;
  int            dstRank;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Bsend_init (start)\tProcess: 0x%x\n", PCS_self());
#endif

  if (dest == MPI_PROC_NULL)                      
    return (MPI_SUCCESS);
  dstRank = commGetGlobalRank(comm, dest);

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

  /* 2. Allocate the master request */
  CALL_FXN(PCS_rqstAlloc(request, comm, PBSEND_RQST_TYPE), MPI_ERR_BUFFER);
  PCS_rqstSet(*request, (char *)NULL, buf, datatype, count);

  /* 3. Init the master request */
  size = packSize(count, datatype);
  CALL_FXN (psend_init( dstRank,                                  /* Dest. address */
                        user_buf,                                 /* Message       */
                        size,                                     /* Size          */
                        EXTENDED_TAG(commGetContext(comm), tag),  /* Tag           */
                       *request),                                 /* Request (out) */
                        MPI_ERR_INTERN);

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Bsend_init (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError (comm, mpi_errno, "MPI_Bsend_init");
}


