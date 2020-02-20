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

#ifndef P_ERRHND_H
#define P_ERRHND_H

  /*----------------------------------------------------------------/
 /   Declaration of types and functions used by this module        /
/----------------------------------------------------------------*/
#include <config.h>

#include <p_comn.h>
#include <object.h>

  /*-------------------------------------------------------/
 /                   Public constants                     /
/-------------------------------------------------------*/
#define HND_E_OK            0
#define HND_E_EXHAUST      (HND_E_OK          - 1)
#define HND_E_INTEGRITY    (HND_E_EXHAUST     - 1)
#define HND_E_TIMEOUT      (HND_E_INTEGRITY   - 1)
#define HND_E_INTERFACE    (HND_E_TIMEOUT     - 1)
#define HND_E_SYSTEM       (HND_E_INTERFACE   - 1)
#define HND_E_DISABLED     (HND_E_SYSTEM      - 1)

#define FATAL_ERROR        (0)
#define RETURN_ERROR       (1)

/* Blocks for error handlers creation */
#define PREALLOC_ERRHND_COUNT           4
#define BLOCK_ERRHND_COUNT              8
#define PREALLOC_MAX_ERRHND_BLOCKS     32

  /*-------------------------------------------------------/
 /                    Public types                        /
/-------------------------------------------------------*/
/* Error handler function type */
typedef void  (Mpi_P_Handler_function) (Mpi_P_Comm_t *comm, int *errcode, char *where);

/* An error handler. MPI opaque type */
struct Mpi_Errhandler {
  Mpi_P_Handler_function  *Function;
  int                      Refs;
  int                      Index;
};
typedef struct Mpi_Errhandler  Mpi_P_Errhandler, *Mpi_P_Errhandler_t;

/* Table of error handlers. Shared by all threads in each process */
struct ErrHndTable {
  Object_t             ErrorHandlers;
  Mpi_P_Errhandler    *Fatal;
  Mpi_P_Errhandler    *Return;
};
typedef struct ErrHndTable     ErrHndTable,      *ErrHndTable_t;

  /*-------------------------------------------------------/
 /           Declaration of public functions              /
/-------------------------------------------------------*/
extern int                errhndAllocTable (ErrHndTable_t *errhndtable);
extern int                errhndFreeTable  (ErrHndTable_t *errhndtable);

extern int                errhndCreate     (ErrHndTable *errhndtable, Mpi_P_Handler_function *function, Mpi_P_Errhandler_t *errhnd);
extern int                errhndDelete     (ErrHndTable *errhndtable, Mpi_P_Errhandler_t *errhnd);

extern Mpi_P_Errhandler  *errhndGetRef     (Mpi_P_Errhandler *errhnd);
extern int                errhndExec       (Mpi_P_Errhandler *errhnd, Mpi_P_Comm_t *comm, int *errorcode, char *where);

#endif

