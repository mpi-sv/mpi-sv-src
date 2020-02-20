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
#include <env.h>
#include <errhnd.h>
#include <check.h>


/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Attr_delete
#define MPI_Attr_delete  PMPI_Attr_delete
#endif


/**
 *  MPI_Attr_delete
 *
 */
int MPI_Attr_delete (MPI_Comm comm, int keyval) {

  int    mpi_errno;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Attr_delete (start)\tProcess: 0x%x\n", PCS_self());
#endif

  /* 1. Check integrity of parameters */
#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_keyval(keyval))                    goto mpi_exception;
#endif

  /* 2. Get the attribute for this key/comm */
  CALL_FXN(PCS_keyDelAttr(comm, keyval), MPI_ERR_OTHER);


#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Attr_delete (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError (comm, mpi_errno, "MPI_Attr_delete");
}

