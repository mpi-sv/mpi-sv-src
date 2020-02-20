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
 /   Declaration of public functions implemented by this module    /
/----------------------------------------------------------------*/
#include <p_status.h>

  /*----------------------------------------------------------------/
 /   Declaration of public functions used by this module           /
/----------------------------------------------------------------*/
#if defined (__OSI)
  #include <osi.h>
#else
  #include <pthread.h>
  #include <stdio.h>
#endif

#include <addr.h>
#include <rqst.h>

#include <env.h>
#include <check.h>
#include <p_group.h>
#include <p_config.h>

  /*-------------------------------------------------------/
 /                 Public interface                       /
/-------------------------------------------------------*/
/*
 *  STATUS_setValue
 *    Set the fields of the status. For a receive request, recv_req == TRUE, else FALSE
 * TODO: El error de Azequia no se establece correctamente
 */
void STATUS_setValue(int rqst_recv, Mpi_P_Comm *comm, Status *stazq, Mpi_P_Status *status, int error) {

  if (status == NULL) return;

  if (rqst_recv) {
    status->MPI_SOURCE = groupGetRankInGroup(commGetGroup(comm), stazq->Src.Rank);
    status->MPI_TAG    = TAG_EXT(stazq->Tag);
    status->Count      = stazq->Count;
  }

  if (error) {
    if      (stazq->Error == AZQ_WAIT_ERR_PENDING)  status->MPI_ERROR = MPI_ERR_PENDING;
    else if (stazq->Error == AZQ_WAIT_SUCCESS)      status->MPI_ERROR = MPI_SUCCESS;
    else                                            status->MPI_ERROR = MPI_ERR_OTHER;
  }

  status->Cancelled = stazq->Cancelled;
  return;
}

