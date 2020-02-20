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

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Initialized
#define MPI_Initialized  PMPI_Initialized
#endif


/**
 *  MPI_Initialized
 */
int MPI_Initialized (int *flag) {

  *flag = PCS_isInit();

  return MPI_SUCCESS;
}

