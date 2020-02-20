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

#include <common.h>
#include <env.h>
#include <errhnd.h>
#include <check.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Comm_dup
#define MPI_Comm_dup  PMPI_Comm_dup
#endif


/**
 *  MPI_Comm_dup
 */
int MPI_Comm_dup (MPI_Comm comm, MPI_Comm *newcomm) {

  int        mpi_errno;
  MPI_Group  local_group, remote_group, new_local_group, new_remote_group;
  int        local_rank;
  int        p_commNr, r_commNr, comm_nr_final;
  MPI_Comm   local_comm;
  int        comm_type;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Comm_dup (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
#endif

  NEST_FXN_INCR();

  *newcomm = (MPI_Comm) MPI_COMM_NULL;

  comm_type = commGetType(comm);
  if (comm_type == INTRACOMM) {

    /* 1. Generate a communication number (context_id) */
    p_commNr = PCS_commGetNrMax() + 1;
    CALL_MPI_NEST(MPI_Allreduce (&p_commNr, &r_commNr, 1, MPI_INT, MPI_MAX, comm));

    /* 2. Get a new reference to the group */
    local_group = commGetLocalGroup(comm);
    groupGetRef(local_group, &new_local_group);

    /* 3. Get the local rank in the group */
    local_rank = groupGetLocalRank(new_local_group);
    if (0 > local_rank)                                    {mpi_errno = MPI_ERR_RANK; goto mpi_exception_unnest;}

    /* 4. Create the communicator in the task that belongs to the group */
    CALL_FXN (comm_create (comm, new_local_group, NULL, r_commNr, 
								 comm_type, NULL, newcomm), MPI_ERR_INTERN);

  } else /* INTERCOMM */ {

    /* 1. Generate a communication number. It needs to allreduce in the local group */
    CALL_MPI_NEST(MPI_Comm_group(comm, &local_group));
    CALL_MPI_NEST(MPI_Comm_create(MPI_COMM_WORLD, local_group, &local_comm));

    p_commNr = PCS_commGetNrMax() + 1;
    CALL_MPI_NEST(MPI_Allreduce (&p_commNr, &r_commNr, 1, MPI_INT, MPI_MAX, local_comm));

    CALL_MPI_NEST(MPI_Comm_rank(comm, &local_rank));
    if (local_rank == 0) {
      CALL_MPI_NEST(MPI_Sendrecv(&r_commNr, 1, MPI_INT, 0, 0, &comm_nr_final, 1, MPI_INT, 0, 0, comm, NULL));
      if (comm_nr_final < r_commNr) comm_nr_final = r_commNr;
    }
    CALL_MPI_NEST(MPI_Bcast(&comm_nr_final, 1, MPI_INT, 0, local_comm));

    /* 2. Get a new reference to the group */
    local_group = commGetLocalGroup(comm);
    groupGetRef(local_group, &new_local_group);
    remote_group = commGetRemoteGroup(comm);
    groupGetRef(remote_group, &new_remote_group);

    /* 3. Free temporary group and comm */
    CALL_MPI_NEST(MPI_Group_free(&local_group));
    CALL_MPI_NEST(MPI_Comm_free(&local_comm));

    /* 4. Create the communicator in the task that belongs to the group */
    CALL_FXN (comm_create (comm, new_local_group, new_remote_group, comm_nr_final, 
			                     comm_type, NULL, newcomm), MPI_ERR_INTERN);

  }

  /* 5. Copy the attributes to the new communicator */
  CALL_FXN (PCS_keyCopyAllAttr(comm, *newcomm), MPI_ERR_OTHER);

  /* 6. Copy the topology information */
  CALL_FXN (commCopyTopol (comm, *newcomm), MPI_ERR_TOPOLOGY);

  NEST_FXN_DECR();

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Comm_dup (end)\tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
  NEST_FXN_DECR();

mpi_exception:
  return commHandleError(comm, mpi_errno, "MPI_Comm_dup");
}

