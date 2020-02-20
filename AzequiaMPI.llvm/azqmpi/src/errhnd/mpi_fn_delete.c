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
#include <env.h>
#include <errhnd.h>

  /*----------------------------------------------------------------/
 /   Implementation of public functions                            /
/----------------------------------------------------------------*/
/**
 *  MPI_NULL_DELETE_FN
 *
 */
int MPI_NULL_DELETE_FN (MPI_Comm comm, int keyval, void *attribute_val, void *extra_state) {

#ifdef DEBUG_MODE
  int rank;

  MPI_Comm_rank(comm, &rank);

  fprintf(stdout, "----------------------------------------------------------------------\n");
  fprintf(stdout, "\tMPI_NULL_DELETE_FN\n");
  fprintf(stdout, "\tAttribute value: 0x%x\tExtra_state: 0x%x\n", (int *)attribute_val, (int *)extra_state);
  fprintf(stdout, "\tRank (caller): %d\n", rank);
  fprintf(stdout, "\tCommunicator: 0x%x (CommNR %d)\n", (unsigned int)comm, commGetContext((Mpi_P_Comm *)comm));
  fprintf(stdout, "----------------------------------------------------------------------\n");
#endif

  return MPI_SUCCESS;
}






