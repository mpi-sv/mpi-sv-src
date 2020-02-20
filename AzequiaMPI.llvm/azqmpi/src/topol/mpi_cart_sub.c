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
#include <mpi.h>

#include <env.h>
#include <errhnd.h>
#include <check.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Cart_sub
#define MPI_Cart_sub  PMPI_Cart_sub
#endif


/**
 *  MPI_Cart_sub
 */
int MPI_Cart_sub (MPI_Comm comm, int *remain_dims, MPI_Comm *newcomm) {

  int   mpi_errno;
  int   dims       [MAX_CART_DIMS];
  int   periods    [MAX_CART_DIMS];
  int   coords     [MAX_CART_DIMS];
  int   newdims    [MAX_CART_DIMS];
  int   newperiods [MAX_CART_DIMS];
  int   ndims;
  int   nnodes_sub;
  int   ndims_sub;
  int   key;
  int   color;
  int   rank;
  int   i, j;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Cart_sub (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_comm_type(comm, INTRACOMM))        goto mpi_exception;
  if (mpi_errno = check_topo_type(comm, TOPO_CART))        goto mpi_exception;
#endif

  /* 1. Determine the number of remaining dimensions and nodes */
  ndims = commCartGetDim(comm);
  commCartGet(comm, ndims, dims, periods, coords);
  ndims_sub  = 0;
  nnodes_sub = 1;
  for (i = 0; i < ndims; i++) {
    if (remain_dims[i]) {
      ndims_sub  += 1;
      nnodes_sub *= dims[i];
    }
  }

  /* 2. Split the cart communicator */
  key   = 0;
  color = 0;
  for (i = 0; i < ndims; i++) {
	if (remain_dims[i])
      key = (key * dims[i]) + coords[i];
	else
      color = (color * dims[i]) + coords[i];
  }

  NEST_FXN_INCR();
  CALL_MPI_NEST(MPI_Comm_split(comm, color, key, newcomm));
  NEST_FXN_DECR();

  /* 3. Save the topology of the new communicator */
  j = 0;
  for (i = 0; i < ndims; i++) {
    if (remain_dims[i]) {
      newdims[j]    = dims[i];
	  newperiods[j] = periods[i];
      j++;
    }
  }

  rank = commGetRank(*newcomm);

  CALL_FXN(commCartCreate(*newcomm, nnodes_sub, rank, ndims_sub, newdims, newperiods), MPI_ERR_TOPOLOGY);

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Cart_sub (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
  NEST_FXN_DECR();

mpi_exception:
  return commHandleError(comm, mpi_errno, "MPI_Cart_sub");
}

