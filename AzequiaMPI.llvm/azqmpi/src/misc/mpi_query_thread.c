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

#include <env.h>
#include <errhnd.h>
#include <p_group.h>
#include <p_key.h>


/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Query_thread
#define MPI_Query_thread  MPI_Query_thread
#endif


/*
 *  MPI_Query_thread
 */
int MPI_Query_thread (int *provided) {

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Query_thread (start)\tProcess: 0x%x\n", PCS_self());
#endif

  *provided = PCS_self()->ThreadLevelSupported;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Query_thread (end)  \tProcess: 0x%x\n", PCS_self());
#endif
  
  return MPI_SUCCESS;
}

