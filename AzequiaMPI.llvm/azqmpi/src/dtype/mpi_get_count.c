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
#include <p_dtype.h>


/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Get_count
#define MPI_Get_count  PMPI_Get_count
#endif


/**
 *  MPI_Get_count
 *   Return the number of elements of "datatype" type received
 */
int MPI_Get_count (MPI_Status *status, MPI_Datatype datatype, int *count) {

  int  mpi_errno;
  int  dsize;

#ifdef CHECK_MODE
  if (mpi_errno = check_status(status))                    goto mpi_exception;
  if (mpi_errno = check_datatype(datatype))                goto mpi_exception;
#endif

  if (status->Count == 0) {

    *count = 0;

  } else {

    //dsize = dtypeGetSize(datatype);
	/* Data could come packed. Count is the bytes received / bytes in a packet */
	dsize = packSize(1, datatype);

    if (status->Count % dsize)
      *count = MPI_UNDEFINED;
    else
      *count = status->Count / dsize;

  }

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
	return commHandleError (MPI_COMM_WORLD, mpi_errno, "MPI_Get_count");
}

