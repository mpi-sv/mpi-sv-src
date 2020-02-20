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
#include <check.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Start
#define MPI_Start  PMPI_Start
#endif


/*
 *  MPI_Start
 */
int MPI_Start (MPI_Request *request) 
{
  int            mpi_errno;
  char          *tmp_buf;
  int            offset;
  int            pos;
  int            size;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Start(%p): Rqst %p. BEGIN\n", self(), *request); fflush(stdout);
#endif

  /* 1. Check integrity of parameters */
#ifdef CHECK_MODE
  if (mpi_errno = check_request(1, request))               goto mpi_exception;
#endif

  /* 2. Start the operation */
  if (RQST_isRecv(*request)) {
    CALL_FXN (precv_start(*request), MPI_ERR_INTERN);
  } 
  else {
    if(RQST_isBuffered(*request)) {
      int            count     =                   (*request)->Count;
      char          *buf       =                   (*request)->OrigBuffer;
      MPI_Datatype   datatype  = (Mpi_P_Datatype *)(*request)->Datatype;
      MPI_Comm       comm      =                   (*request)->Comm;
      int            tag       =                   (*request)->Hdr.Tag;
      int            dstRank   =                   (*request)->Hdr.Dst.Rank;
      MPI_Request    subRequest;
      char          *user_buf;
      int            size;
      SegmentInfo_t  segment;

      /* 1. Allocate a subrequest */
      size = packSize(count, datatype);
      if (0 > PCS_rqstGetSegment(&segment, size))             {mpi_errno = MPI_ERR_BUFFER; 
                                                               goto mpi_exception;}
      subRequest = (MPI_Request)&segment->SubRqst;
      PCS_rqstSet(subRequest, (char *)segment, NULL, datatype, count);

      /* 2. Pack or memcpy directly to user attached buffer */
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
  
      /* 3. Start the send operation */
      CALL_FXN (asend(dstRank,                                       
                      user_buf,                                  
                      size,                                      
                      EXTENDED_TAG(commGetContext(comm), tag),   
                      subRequest),                                  
                      MPI_ERR_INTERN);
      RQST_setSatisfied(*request);
    } 
    else {
      if ((*request)->PackedBuffer != NULL) {
        pos = 0;
        size = packSize((*request)->Count, (Mpi_P_Datatype *)(*request)->Datatype);
        pack((*request)->OrigBuffer, (*request)->Count, (*request)->Datatype, (*request)->PackedBuffer, size, &pos);
      }
      CALL_FXN (psend_start(*request), MPI_ERR_INTERN);
    }
  }

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Start(%p). END\n", self());  fflush(stdout);
#endif
  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError (MPI_COMM_WORLD, mpi_errno, "MPI_Start");
}


