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
  #include <pthread.h>
#endif

#include <thr.h>
#include <com.h>
#include <grp.h>

#include <common.h>
#include <env.h>
#include <errhnd.h>
#include <p_group.h>
#include <p_key.h>


/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Init_thread
#define MPI_Init_thread  PMPI_Init_thread
#endif


/*
 *  MPI_Init_thread
 */
int MPI_Init_thread (int *argc, char **argv[], int required, int *supported) {

  int  ret;
  
#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Init_thread (start)\tProcess: 0x%x\n", PCS_self());
#endif

  /* This implementation supports threads, but only the main thread (with a rank)
   can call MPI functions */
  if (required == MPI_THREAD_SINGLE)
	*supported = MPI_THREAD_SINGLE;
  else
	*supported = MPI_THREAD_FUNNELED;
  
  ret = common_init(argc, argv, *supported);

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Init_thread (end)  \tProcess: 0x%x\n", PCS_self());
#endif
  
  return ret;
}

