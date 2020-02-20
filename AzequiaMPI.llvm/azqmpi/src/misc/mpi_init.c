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
#undef MPI_Init
#define MPI_Init  PMPI_Init
#endif


/**
 *  MPI_Init
 */
int MPI_Init (int *argc, char ***argv) {
bool  old_chk_flag = klee_disable_sync_chk(0);
  int  ret;
//#define DEBUG_MODE
#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Init (start)\tProcess: 0x%x\n", PCS_self()); fflush(stdout);
#endif
      
  ret = common_init(argc, argv, MPI_THREAD_SINGLE);
  
#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Init (end)  \tProcess: 0x%x\n", self());
#endif
  if(old_chk_flag)
      	klee_enable_sync_chk(0);
  return ret;
}

