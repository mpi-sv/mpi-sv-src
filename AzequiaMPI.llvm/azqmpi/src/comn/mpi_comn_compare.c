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
#undef MPI_Comm_compare
#define MPI_Comm_compare  PMPI_Comm_compare
#endif


/**
 *  MPI_Comm_compare
 */
int MPI_Comm_compare (MPI_Comm comm1, MPI_Comm comm2, int *result) {

  int           mpi_errno;
  Mpi_P_Group  *group1, *group2;
  int           local, remote = MPI_IDENT;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Comm_compare (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm1))                       goto mpi_exception;
  if (mpi_errno = check_comm(comm2))                       goto mpi_exception;
#endif

  if (comm1 == comm2) {
    *result = MPI_IDENT;
    return MPI_SUCCESS;
  }

  if (commGetType(comm1) != commGetType(comm2)) {
    *result = MPI_UNEQUAL;
    return MPI_SUCCESS;
  }

  NEST_FXN_INCR();

  group1 = commGetLocalGroup(comm1);
  group2 = commGetLocalGroup(comm2);

  CALL_MPI_NEST(MPI_Group_compare(group1, group2, &local));

  if (local == MPI_UNEQUAL) {
    *result = MPI_UNEQUAL;
    NEST_FXN_DECR();
    return MPI_SUCCESS;
  }

  if (commGetType(comm1) == INTERCOMM) {
    group1 = commGetRemoteGroup(comm1);
    group2 = commGetRemoteGroup(comm2);

    CALL_MPI_NEST(MPI_Group_compare(group1, group2, &remote));
  }

  switch(local) {
    case MPI_IDENT:   if      (remote == MPI_IDENT)     *result = MPI_CONGRUENT;
                      else if (remote == MPI_SIMILAR)   *result = MPI_SIMILAR;
                      else  /* remote == MPI_UNEQUAL */ *result = MPI_UNEQUAL;
                      break;
	  case MPI_SIMILAR: if      (remote == MPI_IDENT)     *result = MPI_SIMILAR;
                      else if (remote == MPI_SIMILAR)   *result = MPI_SIMILAR;
                      else  /* remote == MPI_UNEQUAL */ *result = MPI_UNEQUAL;
                      break;
	  case MPI_UNEQUAL:                                   *result = MPI_UNEQUAL;
                      break;
  }

  NEST_FXN_DECR();

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Comm_compare (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
  NEST_FXN_DECR();

mpi_exception:
  return commHandleError(MPI_COMM_WORLD, mpi_errno, "MPI_Comm_compare");
}



