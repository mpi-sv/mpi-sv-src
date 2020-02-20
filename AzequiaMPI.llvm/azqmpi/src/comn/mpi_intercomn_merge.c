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

#include <azq_types.h>

#include <env.h>
#include <errhnd.h>
#include <check.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Intercomm_merge
#define MPI_Intercomm_merge  PMPI_Intercomm_merge
#endif


/**
 *  MPI_Intercomm_merge
 */
int MPI_Intercomm_merge (MPI_Comm intercomm, int high, MPI_Comm *newintracomm) {

  int           mpi_errno;
  int           remote_high, final_high, group_high;
  Mpi_P_Group  *local_group, *remote_group, *newintragroup;
  Mpi_P_Comm   *local_intracomm;
  int           comm_nr, comm_nr_max, comm_nr_remote, comm_nr_final;
  int           local_size, remote_size, local_intra_size;
  int           local_rank, global_rank, global_remote_rank;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Intercomm_merge (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(intercomm))                   goto mpi_exception;
  if (mpi_errno = check_comm_type(intercomm, INTERCOMM))   goto mpi_exception;
#endif

  NEST_FXN_INCR();

  /* 2. Create a temporal local communicator for the local group */
  CALL_MPI_NEST(MPI_Comm_group(intercomm, &local_group));

  CALL_MPI_NEST(MPI_Comm_create(MPI_COMM_WORLD, local_group, &local_intracomm));

#ifdef CHECK_MODE
  high = (high == FALSE) ? 0 : 1;

  CALL_MPI_NEST(MPI_Comm_size(local_intracomm, &local_intra_size));

  CALL_MPI_NEST(MPI_Allreduce(&high, &group_high, 1, MPI_INT, MPI_SUM, local_intracomm));

  if ((group_high != 0) && (group_high != local_intra_size))
                                                           {mpi_errno = MPI_ERR_ARG;  goto mpi_exception_unnest;}
#endif

  /* 3. Exchange the high value between local and remote groups */
  CALL_MPI_NEST(MPI_Comm_rank(intercomm, &local_rank));

  if (local_rank == 0) {

    CALL_MPI_NEST(MPI_Sendrecv(&high, 1, MPI_INT, 0, 0, &remote_high, 1, MPI_INT, 0, 0, intercomm, NULL));

    /* 3.1. Arbitrary order based on global ranks of process 0 in local and remote group */
    if (high == remote_high) {

      global_rank = groupGetGlobalRank(local_group, 0);

      CALL_MPI_NEST(MPI_Sendrecv(&global_rank, 1, MPI_INT, 0, 0, &global_remote_rank, 1, MPI_INT, 0, 0,intercomm, NULL));

      if (global_rank < global_remote_rank)
        final_high = FALSE;
      else
        final_high = TRUE;

    /* 3.2. Order required by the argument */
    } else
      final_high = high;
  }

  /* 4. Broadcast the high value for the members of the local group */
  CALL_MPI_NEST(MPI_Bcast(&final_high, 1, MPI_INT, 0, local_intracomm));

  /* 5. Find a context id (number of communicator) */
  comm_nr = PCS_commGetNrMax() + 1;
  CALL_MPI_NEST(MPI_Allreduce(&comm_nr, &comm_nr_max, 1, MPI_INT, MPI_MAX, local_intracomm));

  /* 6. Process leaders choose the final communicator number */
  if (local_rank == 0) {

    CALL_MPI_NEST(MPI_Sendrecv(&comm_nr_max, 1, MPI_INT, 0, 0, &comm_nr_remote, 1, MPI_INT, 0, 0, intercomm, NULL));

    comm_nr_final = (comm_nr_max > comm_nr_remote) ? comm_nr_max : comm_nr_remote;

  }

  CALL_MPI_NEST(MPI_Bcast(&comm_nr_final, 1, MPI_INT, 0, local_intracomm));

  /* 7. Delete temporal local intracomm and group */
  CALL_MPI_NEST(MPI_Group_free(&local_group));

  CALL_MPI_NEST(MPI_Comm_free(&local_intracomm));

  /* 8. Create the new intracomm group */
  CALL_MPI_NEST(MPI_Comm_group(intercomm, &local_group));

  CALL_MPI_NEST(MPI_Comm_remote_group(intercomm, &remote_group));

  CALL_MPI_NEST(MPI_Comm_size(intercomm, &local_size));

  CALL_MPI_NEST(MPI_Comm_remote_size(intercomm, &remote_size));

  /* 8.1. Order the union of groups */
  if (final_high == FALSE) {

    CALL_MPI_NEST(MPI_Group_union(local_group, remote_group, &newintragroup));

  } else {

    CALL_MPI_NEST(MPI_Group_union(remote_group, local_group, &newintragroup));

  }

  /* 9. Create the new intracommunicator */
  local_rank = groupGetLocalRank(newintragroup);
  PCS_commCreate(intercomm, newintragroup, NULL, local_rank, comm_nr_final, INTRACOMM, NULL, newintracomm);

  /* 10. Copy all the attributes from intercomm to new intracomm */
  PCS_keyCopyAllAttr(intercomm, *newintracomm);

  NEST_FXN_DECR();

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Intercomm_merge (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
  NEST_FXN_DECR();

mpi_exception:
  return commHandleError(intercomm, mpi_errno, "MPI_Intercomm_merge");
}

