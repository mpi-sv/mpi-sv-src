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
#undef MPI_Is_thread_main
#define MPI_Is_thread_main  MPI_Is_thread_main
#endif


/*
 *  MPI_Is_thread_main
 */
int MPI_Is_thread_main (int *flag) {

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Is_thread_main (start)\tProcess: 0x%x\n", PCS_self());
#endif

  *flag = 0;
  
  if (PCS_self())
    *flag = 1;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Is_thread_main (end)  \tProcess: 0x%x\n", PCS_self());
#endif
  
  return MPI_SUCCESS;
}

