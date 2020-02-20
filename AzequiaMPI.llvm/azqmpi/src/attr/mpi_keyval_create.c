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

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Keyval_create
#define MPI_Keyval_create  PMPI_Keyval_create
#endif


/**
 *  MPI_Keyval_create
 *
 */
int MPI_Keyval_create (MPI_Copy_function *copy_fn, MPI_Delete_function *delete_fn,
                       int *keyval, void *extra_state) {

  int   mpi_errno;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Keyval_create (start)\tProcess: 0x%x\n", PCS_self());
#endif

	/* 1. Check integrity of parameters */
#ifdef CHECK_MODE
#endif

  /* 2. Allocate a key */
  CALL_FXN(PCS_keyAlloc(keyval, extra_state, copy_fn, delete_fn), MPI_ERR_OTHER);

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Keyval_create (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError (MPI_COMM_WORLD, mpi_errno, "MPI_Keyval_create");
}

