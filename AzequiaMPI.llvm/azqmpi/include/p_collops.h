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

#ifndef	P_COLLOPS_H
#define	P_COLLOPS_H

  /*----------------------------------------------------------------/
 /   Declaration of types and functions used by this module        /
/----------------------------------------------------------------*/
#include <config.h>

#include <p_dtype.h>
#include <object.h>

  /*-------------------------------------------------------/
 /                   Public constants                     /
/-------------------------------------------------------*/

/* Return error codes */
#define OPS_E_OK                   0
#define OPS_E_EXHAUST             (OPS_E_OK          - 1)
#define OPS_E_INTEGRITY           (OPS_E_EXHAUST     - 1)
#define OPS_E_TIMEOUT             (OPS_E_INTEGRITY   - 1)
#define OPS_E_INTERFACE           (OPS_E_TIMEOUT     - 1)
#define OPS_E_SYSTEM              (OPS_E_INTERFACE   - 1)
#define OPS_E_DISABLED            (OPS_E_SYSTEM      - 1)

/* Collective predefined operations */
#define MPI_SUM_INDEX             (0)
#define MPI_MAX_INDEX             (MPI_SUM_INDEX    + 1)
#define MPI_MIN_INDEX             (MPI_MAX_INDEX    + 1)
#define MPI_PROD_INDEX            (MPI_MIN_INDEX    + 1)
#define MPI_LAND_INDEX            (MPI_PROD_INDEX   + 1)
#define MPI_BAND_INDEX            (MPI_LAND_INDEX   + 1)
#define MPI_LOR_INDEX             (MPI_BAND_INDEX   + 1)
#define MPI_BOR_INDEX             (MPI_LOR_INDEX    + 1)
#define MPI_LXOR_INDEX            (MPI_BOR_INDEX    + 1)
#define MPI_BXOR_INDEX            (MPI_LXOR_INDEX   + 1)
#define MPI_MAXLOC_INDEX          (MPI_BXOR_INDEX   + 1)
#define MPI_MINLOC_INDEX          (MPI_MAXLOC_INDEX + 1)
#define MPI_LAST_DEFAULT_COLLOP   (MPI_MINLOC_INDEX + 1)

  /*-------------------------------------------------------/
 /                   Public constants                     /
/-------------------------------------------------------*/
/* Blocks for communication creation */
#define PREALLOC_COPS_COUNT           0
#define BLOCK_COPS_COUNT             16
#define PREALLOC_MAX_COPS_BLOCKS     32

  /*-------------------------------------------------------/
 /                    Public types                        /
/-------------------------------------------------------*/
typedef void (Mpi_P_User_function) (void *invec, void *inoutvec, int *len, Mpi_P_Datatype *datatype);

struct Mpi_Op {
  Mpi_P_User_function  *Function;
  int                   Commute;
  int                   Index;
};
typedef struct Mpi_Op    Mpi_P_Op,  *Mpi_P_Op_t;

struct CopsTable {
  Object_t       Operation;
};
typedef struct CopsTable CopsTable, *CopsTable_t;


  /*-------------------------------------------------------/
 /                    Public data                         /
/-------------------------------------------------------*/
extern Mpi_P_Op       BasicCopsTable  [MPI_LAST_DEFAULT_COLLOP];

  /*-------------------------------------------------------/
 /           Declaration of public functions              /
/-------------------------------------------------------*/
/* Per process table with default MPI collective operations */
extern  int         copsCreateDefault ();
extern  int         copsDeleteDefault ();


extern  int         copsAllocTable    (CopsTable_t *copsTable);
extern  int         copsFreeTable     (CopsTable_t *copsTable);


extern  int         copsCreate        (CopsTable *copsTable, Mpi_P_User_function *function, int commute, Mpi_P_Op_t *collop);
extern  int         copsDelete        (CopsTable *copstable, Mpi_P_Op_t *collop);

#define             copsIsConmute(collop)    (collop)->Commute
#define             copsGetFunction(collop)  (collop)->Function

/* Collective predefined operations */
extern  void        mpi_sum           (void *invec, void *inoutvec, int *len, Mpi_P_Datatype *datatype);
extern  void        mpi_max	          (void *invec, void *inoutvec, int *len, Mpi_P_Datatype *datatype);
extern  void        mpi_min	          (void *invec, void *inoutvec, int *len, Mpi_P_Datatype *datatype);
extern  void        mpi_prod	      (void *invec, void *inoutvec, int *len, Mpi_P_Datatype *datatype);
extern  void        mpi_land	      (void *invec, void *inoutvec, int *len, Mpi_P_Datatype *datatype);
extern  void        mpi_band	      (void *invec, void *inoutvec, int *len, Mpi_P_Datatype *datatype);
extern  void        mpi_lor	          (void *invec, void *inoutvec, int *len, Mpi_P_Datatype *datatype);
extern  void        mpi_bor	          (void *invec, void *inoutvec, int *len, Mpi_P_Datatype *datatype);
extern  void        mpi_lxor	      (void *invec, void *inoutvec, int *len, Mpi_P_Datatype *datatype);
extern  void        mpi_bxor	      (void *invec, void *inoutvec, int *len, Mpi_P_Datatype *datatype);
extern  void        mpi_maxloc	      (void *invec, void *inoutvec, int *len, Mpi_P_Datatype *datatype);
extern  void        mpi_minloc	      (void *invec, void *inoutvec, int *len, Mpi_P_Datatype *datatype);


#endif



