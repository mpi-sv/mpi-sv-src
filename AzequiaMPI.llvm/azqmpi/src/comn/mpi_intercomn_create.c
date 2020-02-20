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
#include <stdlib.h>

#include <env.h>
#include <errhnd.h>
#include <check.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Intercomm_create
#define MPI_Intercomm_create  PMPI_Intercomm_create
#endif


/**
 *  MPI_Intercomm_create
 */
int MPI_Intercomm_create (MPI_Comm local_comm,  int local_leader,
                          MPI_Comm peer_comm,   int remote_leader,
                          int tag, MPI_Comm *newintercomm) {

  int           mpi_errno;
  int           i;
  Mpi_P_Group  *group, *local_group, *remote_group;
  int           comm_nr, comm_max, comm_final;
  int           size, remote_size;
  int          *members, *remote_members;
  int           rank;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Intercomm_create (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(local_comm))                     goto mpi_exception;
  if (mpi_errno = check_comm_type(local_comm, INTRACOMM))     goto mpi_exception;
  if (mpi_errno = check_tag(tag))                             goto mpi_exception;
  if (mpi_errno = check_rank_comm(local_leader, local_comm))  goto mpi_exception;
#endif

  /* 1. Local group */
  group = commGetLocalGroup(local_comm);
  rank  = commGetRank(local_comm);

#ifdef CHECK_MODE
  if (mpi_errno = check_group(group))                         goto mpi_exception;
  if (rank == local_leader) {
    if (mpi_errno = check_comm(peer_comm))                    goto mpi_exception;
    if (mpi_errno = check_rank_comm(remote_leader,peer_comm)) goto mpi_exception;
  }
#endif

  NEST_FXN_INCR();

  size  = groupGetSize(group);
  CALL_FXN(MALLOC(members, size * sizeof(int)), MPI_ERR_INTERN);

  /* 2. New context (communicator number) for the intercommunicator to create */
  comm_nr = PCS_commGetNrMax() + 1;
  CALL_MPI_NEST(MPI_Allreduce (&comm_nr, &comm_max, 1, MPI_INT, MPI_MAX, local_comm));

  if (rank == local_leader) {

    CALL_MPI_NEST(MPI_Sendrecv(&comm_max,   1, MPI_INT, remote_leader, tag,
                               &comm_final, 1, MPI_INT, remote_leader, tag, peer_comm, NULL));

    if (comm_final < comm_max) comm_final = comm_max;

  }

  CALL_MPI_NEST(MPI_Bcast(&comm_final, 1, MPI_INT, local_leader, local_comm));

  /* 3. Remote group to create */
  if (rank == local_leader) {

    CALL_MPI_NEST(MPI_Sendrecv(&size,        1, MPI_INT, remote_leader, tag,
                               &remote_size, 1, MPI_INT, remote_leader, tag, peer_comm, NULL));

  }

  CALL_MPI_NEST(MPI_Bcast(&remote_size, 1, MPI_INT, local_leader, local_comm));

  if (MALLOC(remote_members, remote_size * sizeof(int)))
  //if (NULL == (remote_members = (int *)malloc(remote_size * sizeof(int))))
                                                           {mpi_errno = MPI_ERR_INTERN; goto mpi_exception_2;}
  if (rank == local_leader) {

    for (i = 0; i < size; i++)
      members[i] = groupGetGlobalRank(group, i);

    CALL_MPI_NEST(MPI_Sendrecv(members,        size,        MPI_INT, remote_leader, tag,
                               remote_members, remote_size, MPI_INT, remote_leader, tag, peer_comm, NULL));
  }

#ifdef CHECK_MODE
  if (rank == local_leader) {
    if (mpi_errno = check_disjoint(size, members, remote_size, remote_members))
                                                           goto mpi_exception_3;
  }
#endif

  CALL_MPI_NEST(MPI_Bcast(remote_members, remote_size, MPI_INT, local_leader, local_comm));

  /* 4. Create a new reference to the local group and a new remote group */
  groupGetRef(group, &local_group);
  CALL_FXN(PCS_groupCreate (remote_members, remote_size, &remote_group), MPI_ERR_GROUP);

  /* 5. Create the new intercommunicator */
  CALL_FXN (PCS_commCreate (local_comm, local_group, remote_group, rank, comm_final, INTERCOMM, NULL, newintercomm), MPI_ERR_COMM);

  /* 6. Copy attributes */
  CALL_FXN (PCS_keyCopyAllAttr(local_comm, *newintercomm), MPI_ERR_OTHER);

  /* 7. Free allocated memory */
  FREE(members);
  FREE(remote_members);

  NEST_FXN_DECR();

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Intercomm_create (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_3:
  FREE(remote_members);

mpi_exception_2:
  FREE(members);

mpi_exception_unnest:
  NEST_FXN_DECR();

mpi_exception:
  return commHandleError(local_comm, mpi_errno, "MPI_Intercomm_create");
}

