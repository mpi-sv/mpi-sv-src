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


#if defined (__OSI)
  #include <osi.h>
#else
  #include <pthread.h>
  #include <stdio.h>
#endif

#include <mpi.h>
#include <env.h>
#include <check.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Abort
#define MPI_Abort  PMPI_Abort
#endif


/*
 *  MPI_Abort
 *    Try to abort all processes in the group of Comm
 */
int MPI_Abort (MPI_Comm comm, int errorcode) {

  int  mpi_errno;
  int  gix = getGroup();

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Abort (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
#endif

#ifdef VERBOSE_MODE
  fprintf(stderr, "************************************************\n");
  fprintf(stderr, "MPI_ABORT. Aborting application (nr: %d)\n", gix);
  fprintf(stderr, "           Rank  %d\n", commGetRank(comm));
  fprintf(stderr, "************************************************\n");
#endif

  GRP_kill(gix);
  GRP_destroy(gix);

  //GRP_shutdown();

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Abort (end).\n");
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError (comm, mpi_errno, "MPI_Abort");
}
