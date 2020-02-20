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
 /   Declaration of public functions implemented by this module    /
/----------------------------------------------------------------*/
#include <config.h>

#include <azq_types.h>

#include <stdlib.h>

#include <env.h>
#include <errhnd.h>
#include <check.h>
#include <p_status.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Sendrecv_replace
#define MPI_Sendrecv_replace  PMPI_Sendrecv_replace
#endif


/*
 *  MPI_Sendrecv_replace
 */
int MPI_Sendrecv_replace (void *buf, int count, MPI_Datatype datatype, int dest, int sendtag,
				                  int source, int recvtag, MPI_Comm comm, MPI_Status *status) {

  int          mpi_errno;
  MPI_Request  request;
  char        *tmpbuf;
  int          size;
  int          pos;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Sendrecv_replace (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_tag(sendtag))                      goto mpi_exception;
  if (mpi_errno = check_tag(recvtag))                      goto mpi_exception;
  if (mpi_errno = check_dest_comm(dest, comm))             goto mpi_exception;
  if (mpi_errno = check_source_comm(source, comm))         goto mpi_exception;
  if (mpi_errno = check_count(count))                      goto mpi_exception;
  if (mpi_errno = check_datatype(datatype))                goto mpi_exception;
  if (buf == NULL)                                        {mpi_errno = MPI_ERR_BUFFER;  goto mpi_exception;}
#endif

  NEST_FXN_INCR();

  if (dtypeIsContiguous(datatype)) {

    size = dtypeGetExtent(datatype) * count;

    CALL_FXN(MALLOC(tmpbuf, size), MPI_ERR_INTERN);
    pos = 0;
    pack(buf, count, datatype, tmpbuf, size, &pos);

    /* 1.1. For avoiding interblocking it uses a non-blocking receive */
    CALL_MPI_NEST(MPI_Irecv (buf,    count, datatype, source, recvtag, comm, &request));
    CALL_MPI_NEST(MPI_Send  (tmpbuf, count, datatype, dest,   sendtag, comm));
	
	/* 1.2. Wait for receiving only if source is not MPI_PROC_NULL */
	if ((source == MPI_PROC_NULL) && (status != MPI_STATUS_IGNORE)) {
	  STATUS_setNull(status);
	} else {
	  CALL_MPI_NEST(MPI_Wait  (&request, status));
	}

    FREE(tmpbuf);

  } else {

    size = packSize(count, datatype);

    CALL_FXN(MALLOC(tmpbuf, size), MPI_ERR_INTERN);
    pos = 0;
    pack(buf, count, datatype, tmpbuf, size, &pos);

    CALL_MPI_NEST(MPI_Irecv (buf,    count, datatype,   source, recvtag, comm, &request));
    CALL_MPI_NEST(MPI_Send  (tmpbuf, size,  MPI_PACKED, dest,   sendtag, comm));
    
	/* 2.1. Wait for receiving only if source is not MPI_PROC_NULL */
	if ((source == MPI_PROC_NULL) && (status != MPI_STATUS_IGNORE)) {
	  STATUS_setNull(status);
	} else {
	  CALL_MPI_NEST(MPI_Wait  (&request, status));
	}

    FREE(tmpbuf);

  }

  NEST_FXN_DECR();

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Sendrecv_replace (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
  NEST_FXN_DECR();

mpi_exception:
  return commHandleError (comm, mpi_errno, "MPI_Sendrecv_replace");
}

