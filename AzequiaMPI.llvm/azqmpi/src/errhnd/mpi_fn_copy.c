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
#include <azq_types.h>

#include <env.h>
#include <errhnd.h>

  /*----------------------------------------------------------------/
 /   Implementation of public functions                            /
/----------------------------------------------------------------*/
/**
 *  MPI_NULL_COPY_FN
 *
 */
int MPI_NULL_COPY_FN (MPI_Comm oldcomm, int keyval, void *extra_state,
                      void *attribute_val_in, void *attribute_val_out, int *flag) {

#ifdef DEBUG_MODE
  int rank;

  MPI_Comm_rank(oldcomm, &rank);

  fprintf(stdout, "----------------------------------------------------------------------\n");
  fprintf(stdout, "\tMPI_NULL_COPY_FN\n");
  fprintf(stdout, "\tAttribute value IN: 0x%x\tExtra_state: 0x%x\n", (int *)attribute_val_in, (int *)extra_state);
  fprintf(stdout, "\tRank (caller): %d\n", rank);
  fprintf(stdout, "\tCommunicator: 0x%x (CommNR %d)\n", (unsigned int)oldcomm, commGetContext((Mpi_P_Comm *)oldcomm));
  fprintf(stdout, "----------------------------------------------------------------------\n");
#endif

  *flag = FALSE;

  return MPI_SUCCESS;
}

