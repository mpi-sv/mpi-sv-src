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
#ifdef __OSI
  #include <osi.h>
#else
  #include <stdio.h>
  #include <string.h>
  #include <time.h>
  #include <pthread.h>
#endif

#include <thr.h>
#include <com.h>
#include <grp.h>

#include <env.h>
#include <errhnd.h>
#include <p_group.h>
#include <p_errhnd.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Finalize
#define MPI_Finalize  PMPI_Finalize
#endif




#define MAX_CATEGORIAS  10
#define MAX_NODE_NR       128

#define TIPO unsigned long

TIPO cant_total_patron[MAX_NODE_NR][MAX_NODE_NR];
TIPO vol_total_patron[MAX_NODE_NR][MAX_NODE_NR];


/**
 *  MPI_Finalize
 */
int MPI_Finalize (void) {
	 bool old_chk_flag = klee_disable_sync_chk(0);
  int                mpi_errno;
  Mpi_P_Group       *group;
  Mpi_P_Comm        *comm;
  Mpi_P_Errhandler  *errhnd;
  void              *buff_addr = NULL;
  int                size;
  
#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Finalize (start)\tProcess: 0x%x\n", PCS_self());fflush(stdout);
#endif
    
  /* 1. MPI 1.2: Finalize is required to call detach buffer */
  PCS_rqstBufferDetach(&buff_addr, &size);

  /* 2. MPI 1.2: Finalize must assure that all pending communications
                 have finished (mainly for cancel) */
  NEST_FXN_INCR();
  CALL_MPI_NEST(MPI_Barrier(MPI_COMM_WORLD));
  NEST_FXN_DECR();
  
  /* 3. Delete keys for default communicators */
  PCS_keyDelAllAttr(MPI_COMM_WORLD);
  PCS_keyDelAllAttr(MPI_COMM_SELF);

  /* 4. Delete default error handlers */
  //errhnd = PCS_errhndGet(FATAL_ERROR);
  errhnd = FATAL_ERROR_PTR;
  PCS_errhndDelete(&errhnd);
  //errhnd = PCS_errhndGet(RETURN_ERROR);
  errhnd = RETURN_ERROR_PTR;
  PCS_errhndDelete(&errhnd);

  /* 5. Delete default communicators */
  comm = MPI_COMM_WORLD;
  PCS_commDelete(&comm);
  comm = MPI_COMM_SELF;
  PCS_commDelete(&comm);
  
  /* 6. Delete default groups */
  //group = PCS_groupGet(GROUP_WORLD);
  group = GROUP_WORLD_PTR;
  PCS_groupDelete(&group);
  //group = PCS_groupGet(GROUP_EMPTY);
  group = GROUP_EMPTY_PTR;
  PCS_groupDelete(&group);
  //group = PCS_groupGet(GROUP_SELF);
  group = GROUP_SELF_PTR;
  PCS_groupDelete(&group);

  /* 7. Delete this process */
  PCS_finalize();

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Finalize (end)  \tProcess: 0x%x\n", PCS_self());
#endif
  if(old_chk_flag)   	klee_enable_sync_chk(0);
  return MPI_SUCCESS;

mpi_exception_unnest:
  NEST_FXN_DECR();

mpi_exception:
if(old_chk_flag)   	klee_enable_sync_chk(0);
  return commHandleError (MPI_COMM_WORLD, mpi_errno, "MPI_Bcast");
}

