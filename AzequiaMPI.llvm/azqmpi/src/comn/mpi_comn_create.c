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
#undef MPI_Comm_create
#define MPI_Comm_create  PMPI_Comm_create
#endif


/**
 *  MPI_Comm_create
 */
int MPI_Comm_create (MPI_Comm comm, MPI_Group group, MPI_Comm *newcomm) {

  int       mpi_errno;
  MPI_Group newgroup;
  int       p_commNr;
  int       r_commNr;
  int       rank;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Comm_create (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_comm_type(comm, INTRACOMM))        goto mpi_exception;
  if (mpi_errno = check_group(group))                      goto mpi_exception;
#endif

  NEST_FXN_INCR();

  *newcomm = (MPI_Comm) MPI_COMM_NULL;

  /* 1. AllReduce operation for get a new communicator number. Tasks propose a number
        and the max is elected. All ranks take part in the election.
   */
  p_commNr = PCS_commGetNrMax() + 1;
  CALL_MPI_NEST(MPI_Allreduce (&p_commNr, &r_commNr, 1, MPI_INT, MPI_MAX, comm));

  /* 2. Caller task must be in the group in newcomm, else return ok */
  if (0 > (rank = groupGetLocalRank(group)))               goto retorno;

  /* 3. Only tasks in new group create new communicator entries */
  groupGetRef(group, &newgroup);
  CALL_FXN (comm_create (comm, newgroup, NULL, r_commNr, INTRACOMM, NULL, newcomm), MPI_ERR_COMM);

  /* 4. Copy the DEFAULT attributes to the new communicator */
  CALL_FXN (PCS_keyCopyDfltAttr(comm, *newcomm), MPI_ERR_OTHER);
  
retorno:
  NEST_FXN_DECR();

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Comm_create (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
  NEST_FXN_DECR();

mpi_exception:
  return commHandleError(comm, mpi_errno, "MPI_Comm_create");
}

