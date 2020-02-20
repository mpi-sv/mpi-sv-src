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


#include <mpi.h>
#include <env.h>
#include <check.h>
#include <p_config.h>

#include <p_dtype.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Type_lb
#define MPI_Type_lb  PMPI_Type_lb
#endif


/*
 *  MPI_Type_lb
 */
int MPI_Type_lb (MPI_Datatype datatype, MPI_Aint *displacement) {

  int  mpi_errno;

#ifdef CHECK_MODE
  if (mpi_errno = check_datatype(datatype))                goto mpi_exception;
#endif

  *displacement = dtypeGetLb(datatype);

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError (MPI_COMM_WORLD, mpi_errno, "MPI_Type_lb");
}

