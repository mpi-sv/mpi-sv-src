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


#include <mpi.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Pcontrol
#define MPI_Pcontrol  PMPI_Pcontrol
#endif


/*
 *  MPI_Pcontrol
 *
 */
int MPI_Pcontrol (const int level, ...) {

  return MPI_SUCCESS;
}
