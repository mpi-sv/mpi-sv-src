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
#include <env.h>
#include <check.h>
#include <p_config.h>

#include <p_dtype.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Type_contiguous
#define MPI_Type_contiguous  PMPI_Type_contiguous
#endif


/*
 *  MPI_Type_contiguous
 *
 */
int MPI_Type_contiguous (int count, MPI_Datatype oldtype, MPI_Datatype *newtype) {

  int  mpi_errno;
  int  blklens;
  //int  displs;
  // Fixing memout error. memcpy in dtypeCreateStruct would leads this terrible error.
  Mpi_P_Aint displs[count];
  memset(displs,0,sizeof(Mpi_P_Aint)*count);

#ifdef CHECK_MODE
  if (mpi_errno = check_datatype(oldtype))                 goto mpi_exception;
#endif

  blklens = count;
  //displs  = 0;
  CALL_FXN(PCS_dtypeCreate( 1,                                   /* Size of arrays    */
                            &blklens,                            /* Block lengths     */
                            &displs,                             /* Displacements     */
                            (Mpi_P_Datatype_t *)&oldtype,        /* Types             */
                            DTYPE_CONTIGUOUS,                    /* Kind of MPI type  */
                            newtype),                            /* New type created  */
           MPI_ERR_TYPE);

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError (MPI_COMM_WORLD, mpi_errno, "MPI_Type_contiguous");
}
