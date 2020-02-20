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

#include <p_config.h>


/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Cart_create
#define MPI_Cart_create  PMPI_Cart_create
#endif


/**
 *  MPI_Cart_create
 */
int MPI_Cart_create (MPI_Comm comm_old, int ndims, int *dims, int *periods,
                                        int reorder, MPI_Comm *comm_cart)   {

  int           mpi_errno;
  int           nnodes;
  int           grpsize;
  int           rank, newrank;
  Mpi_P_Group  *group;
  Mpi_P_Group  *newgroup;
  int           commNr, r_commNr;
  int           i;
  int          *ranks_in_newgroup;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Cart_create (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm_old))                    goto mpi_exception;
  if (mpi_errno = check_comm_type(comm_old, INTRACOMM))    goto mpi_exception;
  if (mpi_errno = check_ndims(ndims))                      goto mpi_exception;
#endif

  /* 1. How many nodes for the topology */
  nnodes = 1;
  for (i = 0; i < ndims; i++)
    nnodes *= dims[i];

  group = commGetLocalGroup(comm_old);
  rank  = commGetRank(comm_old);

#ifdef CHECK_MODE
  if (mpi_errno = check_rank_comm(rank, comm_old))         goto mpi_exception;
  if (mpi_errno = check_group(group))                      goto mpi_exception;
#endif

  grpsize = groupGetSize(group);

#ifdef CHECK_MODE
  if ((nnodes > grpsize) || (nnodes <= 0))                {mpi_errno = MPI_ERR_INTERN; goto mpi_exception;}
#endif

  *comm_cart = (MPI_Comm) MPI_COMM_NULL;

  NEST_FXN_INCR();

  /* 2. Create a new group for the communicator */
  if (reorder) {

    CALL_FXN(commCartMap (comm_old, rank, ndims, dims, periods, &newrank), MPI_ERR_TOPOLOGY);
    CALL_MPI_NEST(MPI_Comm_split (comm_old, (newrank == MPI_UNDEFINED) ? MPI_UNDEFINED : 1, newrank, comm_cart));

	  if (newrank == MPI_UNDEFINED) {
	    NEST_FXN_DECR();
	    return MPI_SUCCESS;
	  }

  } else {

    if (nnodes == grpsize) {
	    groupGetRef(group, &newgroup);
	  } else { /* nnodes < grpsize => Create a new group */
		
		if (posix_memalign(&ranks_in_newgroup, CACHE_LINE_SIZE, nnodes * sizeof(int))) {
		  mpi_errno = MPI_ERR_INTERN;
          goto mpi_exception_unnest; 
	    }
		
	    for (i = 0; i < nnodes; i++)
	      ranks_in_newgroup[i] = groupGetGlobalRank(group, i);
        CALL_FXN (PCS_groupCreate(ranks_in_newgroup, nnodes, &newgroup), MPI_ERR_ARG);
		
		free(ranks_in_newgroup);
	  }

    commNr = PCS_commGetNrMax() + 1;
    CALL_MPI_NEST(MPI_Allreduce (&commNr, &r_commNr, 1, MPI_INT, MPI_MAX, comm_old));

    if (rank == MPI_UNDEFINED) {
      NEST_FXN_DECR();
      return MPI_SUCCESS;
    }

    if (0 > (rank = groupGetLocalRank(newgroup))) {
      NEST_FXN_DECR();
      return MPI_SUCCESS;
    }

    CALL_FXN (PCS_commCreate (comm_old, newgroup, NULL, rank, r_commNr, INTRACOMM, NULL, comm_cart), MPI_ERR_COMM);
  } /* end: else REORDER */

  /* 3. Add topology info to the new created communicator */
  CALL_FXN(commCartCreate(*comm_cart, nnodes, rank, ndims, dims, periods), MPI_ERR_TOPOLOGY);

  NEST_FXN_DECR();

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Cart_create (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
  NEST_FXN_DECR();

mpi_exception:
  return commHandleError(comm_old, mpi_errno, "MPI_Cart_create");
}

