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

#ifndef P_STATUS_H
#define P_STATUS_H

  /*----------------------------------------------------------------/
 /   Declaration of types and functions used by this module        /
/----------------------------------------------------------------*/
#include <config.h>

#include <addr.h>
#include <com.h>

#include <mpi.h>

  /*-------------------------------------------------------/
 /                   Public constants                     /
/-------------------------------------------------------*/
#define SRD_E_OK            0
#define SRD_E_EXHAUST      (SRD_E_OK          - 1)
#define SRD_E_INTEGRITY    (SRD_E_EXHAUST     - 1)
#define SRD_E_TIMEOUT      (SRD_E_INTEGRITY   - 1)
#define SRD_E_INTERFACE    (SRD_E_TIMEOUT     - 1)
#define SRD_E_SYSTEM       (SRD_E_INTERFACE   - 1)
#define SRD_E_DISABLED     (SRD_E_SYSTEM      - 1)

  /*-------------------------------------------------------/
 /                    Public types                        /
/-------------------------------------------------------*/
typedef MPI_Status Mpi_P_Status;

  /*-------------------------------------------------------/
 /           Declaration of public functions              /
/-------------------------------------------------------*/
/*
 *  STATUS_setNull
 *    Set the status by a call with PROC_NULL
 */
#define STATUS_setNull(status) {             \
  if ((status) != NULL) {                    \
    (status)->MPI_SOURCE = PROC_NULL;        \
    (status)->MPI_TAG    = TAG_ANY;          \
    (status)->Count      = 0;                \
  }                                          \
}


/*
 *  STATUS_setEmpty
 *    Set the status to empty form
 */
#define STATUS_setEmpty(status) {            \
  if ((status) != NULL) {                    \
    (status)->MPI_SOURCE = MPI_ANY_SOURCE;   \
    (status)->MPI_TAG    = MPI_ANY_TAG;      \
    (status)->MPI_ERROR  = MPI_SUCCESS;      \
    (status)->Count      = 0;                \
    (status)->Cancelled  = 0;                \
  }                                          \
}
typedef struct Mpi_Comm            Mpi_P_Comm,         *Mpi_P_Comm_t;
extern void  STATUS_setValue  (int rqst_recv, Mpi_P_Comm *comm, Status *stazq, Mpi_P_Status *status, int error);


#endif
