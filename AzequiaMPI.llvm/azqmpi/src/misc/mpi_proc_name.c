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
#undef MPI_Get_processor_name
#define MPI_Get_processor_name  PMPI_Get_processor_name
#endif


/**
 *  MPI_Get_processor_name
 */
int MPI_Get_processor_name (char *name, int *resultlen) {

  /* This is the way usually followed in Linux environments:
    if (gethostname(name, MPI_MAX_PROCESSOR_NAME) == 0)
    but we use only the Azequia number of machine */
  snprintf(name, MPI_MAX_PROCESSOR_NAME, "%d", getCpuId());

  *resultlen = strlen(name);

  return MPI_SUCCESS;
}
