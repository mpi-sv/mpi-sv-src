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

#if defined (__OSI)
  #include <osi.h>
#else
  #include <pthread.h>
  #include <stdio.h>
#endif

#include <azq_types.h>

#include <mpi.h>
#include <env.h>
#include <errhnd.h>
#include <check.h>

#include <com.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Ibsend
#define MPI_Ibsend  PMPI_Ibsend
#endif


/*
 *  MPI_Ibsend
 */
int MPI_Ibsend (void *buf, int count, MPI_Datatype datatype, int dest,
				int tag, MPI_Comm comm, MPI_Request *request) 
{
  int            mpi_errno;
  int            size;
  char          *user_buf;
  SegmentInfo_t  segment;
  int            pos;
  int            dstRank;
  MPI_Request    subRequest;


#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Ibsend (start)\tProcess: 0x%x\n", PCS_self());
#endif

  if (dest == MPI_PROC_NULL)                     
    return MPI_SUCCESS;
  dstRank = commGetGlobalRank(comm, dest);

  /* 1. Check parameters integrity */
#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_tag(tag))                          goto mpi_exception;
  if (mpi_errno = check_group(commGetGroup(comm)))         goto mpi_exception;
  if (mpi_errno = check_dest(dest, commGetGroup(comm)))    goto mpi_exception;
  if (mpi_errno = check_count(count))                      goto mpi_exception;
  if (mpi_errno = check_datatype(datatype))                goto mpi_exception;
  if (mpi_errno = check_dtype_commit(datatype))            goto mpi_exception;
#endif

  /* 2. Allocate a master request */
  CALL_FXN(PCS_rqstAlloc(request, comm, IBSEND_RQST_TYPE), MPI_ERR_BUFFER);
  RQST_setSatisfied(*request);

  /* 3. Allocate a subrequest */
  size = packSize(count, datatype);
  if (0 > PCS_rqstGetSegment(&segment, size))             {mpi_errno = MPI_ERR_BUFFER; 
                                                           goto mpi_exception;}
  subRequest = (MPI_Request)&segment->SubRqst;
  PCS_rqstSet(subRequest, (char *)segment, NULL, datatype, count);

  /* 4. Pack or memcpy directly to user attached buffer */
  user_buf = (char *)segment + INFO_SEGMENT_SIZE;
  if (dtypeIsContiguous(datatype)) {
    if (buf == MPI_BOTTOM)  buf = (char *)dtypeGetLb(datatype); 
    size = dtypeGetExtent(datatype) * count;
    MEMCPY(user_buf, buf, size);
  } 
  else {
    pos = 0;
    pack(buf, count, datatype, user_buf, size, &pos);
  }
  
  /* 5. Start the send operation */
  CALL_FXN (asend(dstRank,                                       
                  user_buf,                                  
                  size,                                      
                  EXTENDED_TAG(commGetContext(comm), tag),   
                  subRequest),                                  
            MPI_ERR_INTERN);
  
#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Ibsend (end)  \tProcess: 0x%x\n", PCS_self());
#endif
  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError (comm, mpi_errno, "MPI_Ibsend");
}
