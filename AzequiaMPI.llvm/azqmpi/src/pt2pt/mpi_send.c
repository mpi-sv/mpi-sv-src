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
#include <stdlib.h>

#include <env.h>
#include <errhnd.h>
#include <p_status.h>
#include <check.h>
#include <p_dtype.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Send
#define MPI_Send  PMPI_Send
#endif


/**
 *  MPI_Send
 */
//#define DEBUG_MODE
int MPI_Send (void *buf, int count, MPI_Datatype datatype,
              int dest, int tag, MPI_Comm comm) 
{
  int    mpi_errno;
  char  *tmpbuf;
  int    size;
  int    pos; 
  int    rank;

#ifdef DEBUG_MODE
  fprintf(stdout, "\nMPI_Send(0x%x). tag %x. eTag %x BEGIN\n", (Thr_t)pthread_getspecific(key),
		       tag, EXTENDED_TAG(commGetContext(comm),tag));
  fflush(stdout);
#endif
#undef DEBUG_MODE
  if (dest == MPI_PROC_NULL)                     return (MPI_SUCCESS);

  /* 1. Check integrity of parameters */
#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_tag(tag))                          goto mpi_exception;
  if (mpi_errno = check_group(commGetGroup(comm)))         goto mpi_exception;
  if (mpi_errno = check_dest(dest,commGetGroup(comm)))     goto mpi_exception;
  if (mpi_errno = check_count(count))                      goto mpi_exception;
  if (mpi_errno = check_datatype(datatype))                goto mpi_exception;
  if (mpi_errno = check_dtype_commit(datatype))            goto mpi_exception;
#endif
  bool old_chk_flag = klee_disable_sync_chk(0);
  rank = commGetGlobalRank(comm, dest);

  if (dtypeIsContiguous(datatype)) {
    if (buf == MPI_BOTTOM)  buf = (char *)dtypeGetLb(datatype); 
    size = dtypeGetExtent(datatype) * count;
  
    CALL_FXN (send( rank,                                       
                    buf,                                       
                    size,         
                    EXTENDED_TAG(commGetContext(comm), tag)),  
              MPI_ERR_INTERN);

  } else {
    size = packSize(count, datatype);
    CALL_FXN(MALLOC(tmpbuf, size), MPI_ERR_INTERN);

    pos = 0;
    pack(buf, count, datatype, tmpbuf, size, &pos);
    size = pos;

    CALL_FXN (send( rank,                                       
                    tmpbuf,                                    
                    size,                                      
                    EXTENDED_TAG(commGetContext(comm), tag)),  
              MPI_ERR_INTERN);

    FREE(tmpbuf);

  }

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Send(end)  \tProcess: 0x%p\n", self()); fflush(stdout);
#endif
  if(old_chk_flag)   	klee_enable_sync_chk(0);
  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  if(old_chk_flag)   	klee_enable_sync_chk(0);
  return commHandleError (comm, mpi_errno, "MPI_Send");
}

