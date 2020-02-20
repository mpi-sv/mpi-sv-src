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

#ifndef P_KEY_H
#define P_KEY_H

  /*----------------------------------------------------------------/
 /   Declaration of types and functions used by this module        /
/----------------------------------------------------------------*/
#include <config.h>

#include <com.h>
#include <p_comn.h>

  /*-------------------------------------------------------/
 /                   Public constants                     /
/-------------------------------------------------------*/
#define KEY_E_OK            0
#define KEY_E_EXHAUST      (KEY_E_OK          - 1)
#define KEY_E_INTEGRITY    (KEY_E_EXHAUST     - 1)
#define KEY_E_TIMEOUT      (KEY_E_INTEGRITY   - 1)
#define KEY_E_INTERFACE    (KEY_E_TIMEOUT     - 1)
#define KEY_E_SYSTEM       (KEY_E_INTERFACE   - 1)
#define KEY_E_DISABLED     (KEY_E_SYSTEM      - 1)

/*Last number of default keyvals (from MPI_TAG_UB to MAX_NODES). This cannot be
   changed by users */
#define LAST_DEFAULT_KEYVAL   5

  /*-------------------------------------------------------/
 /                    Public types                        /
/-------------------------------------------------------*/
typedef struct Key        Key,        *Key_t;
typedef struct KeysTable  KeysTable,  *KeysTable_t;

/* Copy and delete functions types */
typedef int  Copy_Fxn        (Mpi_P_Comm *oldcomm, int keyval, void *extra_state,
                              void *attribute_va_in, void *attribute_val_out, int *flag);
typedef int  Del_Fxn         (Mpi_P_Comm *comm,    int keyval, void *attribute_val, void *extra_state);


/* Global variable containing the values for default attributes. All threads returns
   the same value for all fields, so this global variable is enough */
struct DefaultAttributes {
  int  TagUb;
  int  Host;
  int  Io;
  int  WTimeIsGlobal;
  int  MaxNodes;
};

  /*-------------------------------------------------------/
 /           Declaration of public functions              /
/-------------------------------------------------------*/
/* Pending Message Table allocate and free */
extern int   keysAllocTable  (KeysTable_t *keyt);
extern int   keysFreeTable   (KeysTable_t *keyt);

extern int   keyAlloc        (KeysTable *keystab, int *keyval, void *extra_state, void *copy_fxn, void *del_fxn);
extern int   keyFree         (KeysTable *keystab, int  keyval);

extern int   keyPutAttr      (KeysTable *keystab, Mpi_P_Comm *comm, int keyval, void  *attribute_val);
extern int   keyGetAttr      (KeysTable *keystab, Mpi_P_Comm *comm, int keyval, void **attribute_val, int *flag);
extern int   keyDelAttr      (KeysTable *keystab, Mpi_P_Comm *comm, int keyval);

extern int   keyDelAllAttr   (KeysTable *keystab, Mpi_P_Comm *comm);
extern int   keyCopyAllAttr  (KeysTable *keystab, Mpi_P_Comm *comm, Mpi_P_Comm *newcomm);
extern int   keyCopyDfltAttr (KeysTable *keystab, Mpi_P_Comm *comm, Mpi_P_Comm *newcomm);


#endif

