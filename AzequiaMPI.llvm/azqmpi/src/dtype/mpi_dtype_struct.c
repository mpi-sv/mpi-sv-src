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
#undef MPI_Type_struct
#define MPI_Type_struct  PMPI_Type_struct
#endif


/*
 *  MPI_Type_struct
 *
 */
int MPI_Type_struct (int count, int          *array_of_blocklengths,
                                MPI_Aint     *array_of_displacements,
                                MPI_Datatype *array_of_types,
                     MPI_Datatype *newtype)                                 {

  int  mpi_errno;

#ifdef CHECK_MODE
  int  i;

  for (i = 0; i < count; i++)
    if (mpi_errno = check_datatype(array_of_types[i]))     goto mpi_exception;
#endif

  CALL_FXN(PCS_dtypeCreate( count,                               /* Size of arrays    */
                            array_of_blocklengths,               /* Block lengths     */
                            array_of_displacements,              /* Displacements     */
                            (Mpi_P_Datatype_t *)array_of_types,  /* Types             */
                            DTYPE_STRUCT,                        /* Kind of MPI type  */
                            newtype),                            /* New type created  */
           MPI_ERR_TYPE);

  return MPI_SUCCESS;

mpi_exception_unnest:
mpi_exception:
  return commHandleError (MPI_COMM_WORLD, mpi_errno, "MPI_Type_struct");
}
