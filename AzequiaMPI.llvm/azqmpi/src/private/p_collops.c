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
#include <p_collops.h>

  /*----------------------------------------------------------------/
 /   Declaration of types and functions used by this module        /
/----------------------------------------------------------------*/
#if defined (__OSI)
  #include <osi.h>
#else
  #include <pthread.h>
  #include <stdio.h>
#endif

#include <azq_types.h>

#include <p_config.h>
#include <p_dtype.h>
#include <object.h>

  /*-------------------------------------------------------/
 /   Declaration of collective predefined operations      /
/-------------------------------------------------------*/
Mpi_P_Op  BasicCopsTable  [MPI_LAST_DEFAULT_COLLOP];

  /*----------------------------------------------------------------/
 /   Implementation of public functions                            /
/----------------------------------------------------------------*/
/*
 *  copsAllocTable()
 */
int copsAllocTable (CopsTable_t *copsTable) {

  Mpi_P_Op  *ops;
  CopsTable *copstab;
  int        error;

  /* 1. Allocate collective object structure */
  if (posix_memalign(&copstab, CACHE_LINE_SIZE, sizeof(CopsTable))) 
    return OPS_E_EXHAUST;

  /* 2. Allocate collective operations */
  objInit(&ops, PREALLOC_MAX_COPS_BLOCKS, 
                 sizeof(Mpi_P_Op), 
                 PREALLOC_COPS_COUNT, 
                 BLOCK_COPS_COUNT, &error); 
  if(error)                                                                              goto exception;
  
  /* 3. Set fields */
  copstab->Operation = ops;
  
  *copsTable = copstab;

  return (OPS_E_OK);

exception:
  free(copstab);
  return OPS_E_EXHAUST;
}


/*
 *  copsFreeTable()
 */
int copsFreeTable (CopsTable_t *copsTable) {

  objFinalize(&(*copsTable)->Operation);
  
  free(*copsTable);

  *copsTable = (CopsTable *)NULL;

  return OPS_E_OK;
}


/*
 *  copsCreateDefault()
 *    Per process datatype table with default MPI datatypes
 */
int copsCreateDefault () {
  
  /* 1. Default collective operations.
        All default collective operations are conmutative
   */
  
  /* 1.1 SUM collective operation */
  BasicCopsTable[MPI_SUM_INDEX].Function = mpi_sum;
  BasicCopsTable[MPI_SUM_INDEX].Commute  = TRUE;
  
  /* 1.2 MAX collective Operation */
  BasicCopsTable[MPI_MAX_INDEX].Function = mpi_max;
  BasicCopsTable[MPI_MAX_INDEX].Commute  = TRUE;
  
  /* 1.3 MIN collective operation */
  BasicCopsTable[MPI_MIN_INDEX].Function = mpi_min;
  BasicCopsTable[MPI_MIN_INDEX].Commute  = TRUE;
  
  /* 1.4 PRODUCT collective operation */
  BasicCopsTable[MPI_PROD_INDEX].Function = mpi_prod;
  BasicCopsTable[MPI_PROD_INDEX].Commute  = TRUE;
  
  /* 1.5 Logical AND collective operation */
  BasicCopsTable[MPI_LAND_INDEX].Function = mpi_land;
  BasicCopsTable[MPI_LAND_INDEX].Commute  = TRUE;
  
  /* 1.6 Bit-wise AND collective operation */
  BasicCopsTable[MPI_BAND_INDEX].Function = mpi_band;
  BasicCopsTable[MPI_BAND_INDEX].Commute  = TRUE;
  
  /* 1.7 Logical OR collective operation */
  BasicCopsTable[MPI_LOR_INDEX].Function = mpi_lor;
  BasicCopsTable[MPI_LOR_INDEX].Commute  = TRUE;
  
  /* 1.8 Bit-wise OR collective operation */
  BasicCopsTable[MPI_BOR_INDEX].Function = mpi_bor;
  BasicCopsTable[MPI_BOR_INDEX].Commute  = TRUE;
  
  /* 1.9 Logical XOR collective operation */
  BasicCopsTable[MPI_LXOR_INDEX].Function = mpi_lxor;
  BasicCopsTable[MPI_LXOR_INDEX].Commute  = TRUE;
  
  /* 1.10 Bit-wise XOR collective operation */
  BasicCopsTable[MPI_BXOR_INDEX].Function = mpi_bxor;
  BasicCopsTable[MPI_BXOR_INDEX].Commute  = TRUE;
  
  /* 1.11 MAXLOC collective operation */
  BasicCopsTable[MPI_MAXLOC_INDEX].Function = mpi_maxloc;
  BasicCopsTable[MPI_MAXLOC_INDEX].Commute  = TRUE;
  
  /* 1.12 MINLOC collective operation */
  BasicCopsTable[MPI_MINLOC_INDEX].Function = mpi_minloc;
  BasicCopsTable[MPI_MINLOC_INDEX].Commute  = TRUE;
  
  
  return OPS_E_OK;
}


int copsDeleteDefault () {
  return OPS_E_OK;
}



/*
 *  copsCreate
 */
int copsCreate (CopsTable *copstable, Mpi_P_User_function *function, int commute, Mpi_P_Op_t *collop) {

  Mpi_P_Op  *op;
  int        i;
  int        idx;

  /* 1. Get object */
/*
  if (0 > objAlloc(copstable->Operation, (void **)&op, &idx)) {
    *collop = (Mpi_P_Op *)NULL;
    return(OPS_E_EXHAUST);
  }
*/
  objAlloc(copstable->Operation, (void **)&op, &idx);
  
  op->Function = function;
  op->Commute  = commute;
  op->Index    = idx;
  
  *collop = op;

  return OPS_E_OK;
}


/*
 *  copsDelete
 */
int copsDelete (CopsTable *copstable, Mpi_P_Op_t *collop) {

  int i;
  
  objFree(copstable->Operation, collop, (*collop)->Index);

  *collop = (Mpi_P_Op *) NULL;

  return OPS_E_OK;
}

