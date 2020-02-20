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
#undef MPI_Get_version
#define MPI_Get_version  PMPI_Get_version
#endif


/**
 *  MPI_Get_version
 */
int MPI_Get_version (int *version, int *subversion) {

  *version    = MPI_VERSION;
  *subversion = MPI_SUBVERSION;

  return MPI_SUCCESS;
}
