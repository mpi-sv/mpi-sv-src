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
#undef MPI_Address
#define MPI_Address  PMPI_Address
#endif


/*
 *  MPI_Address
 *   Return the address of a location
 */
int MPI_Address (void *location, MPI_Aint *address) {

  *address = (MPI_Aint) ((char *)location - (char *)MPI_BOTTOM);

  return MPI_SUCCESS;
}

