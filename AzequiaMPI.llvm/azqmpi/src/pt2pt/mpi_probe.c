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

#include <config.h>

#if defined (__OSI)
  #include <osi.h>
#else
  #include <string.h>
#endif

#include <azq_types.h>
#include <com.h>

#include <env.h>
#include <errhnd.h>
#include <check.h>
#include <p_status.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Probe
#define MPI_Probe  PMPI_Probe
#endif


/*
 *  MPI_Probe
 */
int MPI_Probe (int source, int tag, MPI_Comm comm, MPI_Status *status) {

  int      mpi_errno;
  Status   stazq;
  int      rank;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Probe (start)\tProcess: 0x%x\n", PCS_self());
#endif

  if (source == MPI_PROC_NULL) {
    STATUS_setNull(status);
    return MPI_SUCCESS;
  }

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_group(commGetGroup(comm)))         goto mpi_exception;
  if (mpi_errno = check_tag(tag))                          goto mpi_exception;
  if (mpi_errno = check_source(source,commGetGroup(comm))) goto mpi_exception;
#endif

  /* 1. Get the source in Azequia global scope and the communicator number */
  rank = (source == MPI_ANY_SOURCE) ? ADDR_RNK_ANY : commGetGlobalRank(comm, source);

  /* 2. Call to Azequia probe */
  CALL_FXN (probe(rank,                                    
                  EXTENDED_TAG(commGetContext(comm), tag),   
                  &stazq),                                    
            MPI_ERR_INTERN);

  /* 3. If message found, return data in status */
  STATUS_setValue(TRUE, comm, &stazq, status, 0);

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Probe (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError(comm, mpi_errno, "MPI_Probe");
}

