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
#include <com.h>

#undef wait
#include <stdlib.h>

#include <env.h>
#include <errhnd.h>
#include <p_status.h>
#include <check.h>
#include <p_dtype.h>


/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Recv
#define MPI_Recv  PMPI_Recv
#endif


/**
 *  MPI_Recv
 */
//#define DEBUG_MODE
int MPI_Recv (void *buf, int count, MPI_Datatype datatype,
              int source, int tag, MPI_Comm comm, MPI_Status *status) {

  int      mpi_errno;
  Status   stazq;
  char    *tmpbuf;
  int      size;
  int      pos;
  int      rank;

#ifdef DEBUG_MODE
  fprintf(stdout, "\nMPI_Recv(%d). tag %x. source %d BEGIN\n", commGetRank(comm), tag,  source);
  fflush(stdout);
#endif

  if (source == MPI_PROC_NULL) {
    STATUS_setNull(status);
    return MPI_SUCCESS;
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

    CALL_FXN (recv ( rank,                                      
                     buf,                                      
                     size,                                     
                     EXTENDED_TAG(commGetContext(comm), tag),  
                    &stazq),                                   
              MPI_ERR_INTERN);
   
  //recv(rank, buf, size, EXTENDED_TAG(commGetContext(comm), tag), &stazq); 
  }  
  else {     

    size = packSize(count, datatype);
    CALL_FXN(MALLOC(tmpbuf, size), MPI_ERR_INTERN);

    CALL_FXN (recv ( rank,                                      
                     tmpbuf,                                    
                     size,                                     
                     EXTENDED_TAG(commGetContext(comm), tag),  
                    &stazq),                                   
              MPI_ERR_INTERN);

  //recv(rank, tmpbuf, size, EXTENDED_TAG(commGetContext(comm), tag), &stazq);
    pos = 0; 
    unpack(tmpbuf, stazq.Count, &pos, buf, count, datatype);
    FREE(tmpbuf);
  }

  /* 6. Set status if possible */
  STATUS_setValue(TRUE, comm, &stazq, status, 0);

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Recv (end)  \tProcess: %d\n", commGetRank(comm)); fflush(stdout);
#endif
//added by Herman
//#define DEBUG_MODE
#ifdef DEBUG_MODE
  fprintf(stdout, "\nMPI_Recv(0x%x). tag %x. eTag %x buff %c ,count %dBEGIN\n", (Thr_t)pthread_getspecific(key),
		  tag, EXTENDED_TAG(commGetContext(comm),tag), buf, count);
  fflush(stdout);
#endif
//#undef DEBUG_MODE

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError (comm, mpi_errno, "MPI_Recv");
}
