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
#include <rqst.h>

#include <mpi.h>
#include <env.h>
#include <errhnd.h>
#include <check.h>

#include <com.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Bsend
#define MPI_Bsend  PMPI_Bsend
#endif


/*
 *  MPI_Bsend
 */
int MPI_Bsend (void *buf, int count, MPI_Datatype datatype, int dest,
			   int tag, MPI_Comm comm) 
{
  int           mpi_errno;
  char         *user_buf;
  MPI_Request   subRequest;
  int           size;
  SegmentInfo_t segment;
  int           pos;
  int           rank;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Bsend (start)\tProcess: 0x%x\n", PCS_self());
#endif
  //fprintf(stdout, "\nMPI_Bsend(%p): BEGIN\n", self()); fflush(stdout);
  if (dest == MPI_PROC_NULL)                      return MPI_SUCCESS;

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

  /* 2. Set fields for Azequia asend */
  rank = commGetGlobalRank(comm, dest);

  /* 3. It needs a segment of the size of the packed buffer */
  size = packSize(count, datatype);
  if (0 > PCS_rqstGetSegment(&segment, size))             {mpi_errno = MPI_ERR_BUFFER; goto mpi_exception;}

  /* 4. Pack (or memcpy) directly to user attached buffer segment */
  user_buf = (char *)segment + INFO_SEGMENT_SIZE;
  
  if (dtypeIsContiguous(datatype)) {
    if (buf == MPI_BOTTOM)  buf = (char *)dtypeGetLb(datatype); 
    size = dtypeGetExtent(datatype) * count;
	MEMCPY(user_buf, buf, size);
  } else {
	pos = 0;
	pack(buf, count, datatype, user_buf, size, &pos);
  }
    
  /* 5. Allocate a subrequest */
//CALL_FXN(PCS_rqstAlloc(&request, comm, BSEND_RQST_TYPE), MPI_ERR_BUFFER);
  subRequest = (MPI_Request)&segment->SubRqst;
  PCS_rqstSet(subRequest, (char *)segment, NULL, datatype, count);
  
  /* 6. The segment has a INFO_SEGMENT_SIZE SegmentInfo struct that store the request. 
        "request" is a local reference */
//segment->Request = request;
  
  /* 7. Start the send operation */
  CALL_FXN (asend(rank,       user_buf,                                  
				  size,                                      
				  EXTENDED_TAG(commGetContext(comm), tag),   
				  subRequest),                                  
			MPI_ERR_INTERN);
  
#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Bsend (end)  \tProcess: 0x%x\n", PCS_self());
#endif
  //fprintf(stdout, "MPI_Bsend(%p) Segment %p - SubRequest %p: END\n", self(), segment, subRequest); fflush(stdout);
  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError (comm, mpi_errno, "MPI_Bsend");
}
