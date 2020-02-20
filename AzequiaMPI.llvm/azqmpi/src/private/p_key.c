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
#include <p_key.h>

  /*----------------------------------------------------------------/
 /   Declaration of public functions used by this module           /
/----------------------------------------------------------------*/
#if defined (__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <string.h>
  #include <pthread.h>
#endif

#include <azq_types.h>
#include <com.h>

#include <env.h>
#include <p_config.h>

  /*----------------------------------------------------------------/
 /                  Definition of constants                        /
/----------------------------------------------------------------*/
/* Global variable containing the values for default attributes. All threads returns
   the same value for all fields, so this global variable is enough */
struct DefaultAttributes def_attrs = { TAG_ANY, PROC_NULL, 0, 0, MAX_NODES };

  /*----------------------------------------------------------------/
 /                  Definition of opaque types                     /
/----------------------------------------------------------------*/
/* Keys */
struct Key {
  int        Allocated;
  int        Refs;
  void      *ExtraState;
  Copy_Fxn  *CopyFxn;
  Del_Fxn   *DelFxn;
};

/* Keys Table */
struct KeysTable {
  Key       *Keys;
  int        Count;
  /* Size is DEFAULT_ATTRIBUTES_COUNT by now */
};

  /*-------------------------------------------------------/
 /         Declaration of private interface               /
/-------------------------------------------------------*/
#define KEY_isAlloc(keystab, keyval)  ((keystab)->Keys[(keyval)].Allocated)

#ifdef DEBUG_MODE
PRIVATE  int   printKeysEntries   (KeysTable *keystab);
#endif

  /*-------------------------------------------------------/
 /                 Private interface                      /
/-------------------------------------------------------*/

  /*-------------------------------------------------------/
 /                 Public interface                       /
/-------------------------------------------------------*/
/**
 *  keysAllocTable():
 *   Allocate a keys Table for caching attributes
 */
int keysAllocTable (KeysTable_t *keyt) {

  KeysTable *keysTable;
  Key       *keys;
  int        i;

  /* 1. Allocate user buffer structure memory */
  if (NULL == (keysTable = calloc(sizeof(KeysTable), 1)))
    return KEY_E_EXHAUST;

  keysTable->Count = 0;

  *keyt = keysTable;

  if (NULL == (keys = calloc(sizeof(Key), DEFAULT_ATTRIBUTES_COUNT))) {
    free(keysTable);
    return KEY_E_EXHAUST;
  }

  keysTable->Keys = keys;
  for (i = 0; i < DEFAULT_ATTRIBUTES_COUNT; i++)
    keys[i].Allocated = FALSE;

  return ((DEFAULT_ATTRIBUTES_COUNT * sizeof(Key)) + sizeof (KeysTable));
}


/**
 *  keysFreeTable():
 *   
 */
int keysFreeTable (KeysTable_t *keyt) {

  Key  *keys;

  free((*keyt)->Keys);
	   
  free(*keyt);

  *keyt = NULL;

  return KEY_E_OK;
}


/*
 *  keyAlloc
 */
int keyAlloc (KeysTable *keystab, int *keyval, void *extra_state, void *copy_fxn, void *del_fxn) {

  Key  *pentry;
  int   i;

  /* 1. KEY table full */
  if (keystab->Count >= DEFAULT_ATTRIBUTES_COUNT)           return KEY_E_EXHAUST;

  /* 2. Find a free KEY entry */
  for (i = 0; i < DEFAULT_ATTRIBUTES_COUNT; i++)
    if (!keystab->Keys[i].Allocated) break;

  /* 3. Entry found. Fill key fields */
  pentry = &(keystab->Keys[i]);
  pentry->ExtraState = extra_state;
  pentry->CopyFxn    = (Copy_Fxn *)copy_fxn;
  pentry->DelFxn     = (Del_Fxn *)del_fxn;
  pentry->Refs       = 1;
  pentry->Allocated  = TRUE;

  keystab->Count++;

  *keyval = i;

  return KEY_E_OK;
}


/*
 *  keyFree
 */
int keyFree(KeysTable *keystab, int keyval) {

  Key  *pentry;

  pentry = &(keystab->Keys[keyval]);

  if (!pentry->Allocated)  return KEY_E_INTEGRITY;

  pentry->Refs--;
  if (pentry->Refs == 0) {
    pentry->Allocated   = FALSE;
	pentry->ExtraState  = NULL;
	pentry->CopyFxn     = NULL;
	pentry->DelFxn      = NULL;
    keystab->Count--;
  }

  return KEY_E_OK;
}


/*
 *  keyPutAttr
 */
int keyPutAttr (KeysTable *keystab, Mpi_P_Comm *comm, int keyval, void *attribute_val) {

  Key   *pentry;
  void  *value;

  if (!KEY_isAlloc(keystab, keyval))             return KEY_E_INTEGRITY;

  if (commGetAttr(comm, keyval, &value)) {
    pentry = &(keystab->Keys[keyval]);
    if (0 != ((pentry->DelFxn)(comm, keyval, value, pentry->ExtraState)))
                                                 return KEY_E_INTERFACE;
    keystab->Keys[keyval].Refs--;
  }

  commPutAttr(comm, keyval, attribute_val);
  keystab->Keys[keyval].Refs++;

  return KEY_E_OK;
}


/*
 *  keyGetAttr
 */
int keyGetAttr (KeysTable *keystab, Mpi_P_Comm *comm, int keyval, void **attribute_val, int *flag) {

  if (!KEY_isAlloc(keystab, keyval))             return KEY_E_INTEGRITY;

  *flag = commGetAttr(comm, keyval, attribute_val);

  return KEY_E_OK;
}


/*
 *  keyDelAttr
 */
int keyDelAttr (KeysTable *keystab, Mpi_P_Comm *comm, int keyval) {

  Key   *pentry;
  void  *value;

  if (!KEY_isAlloc(keystab, keyval))              return KEY_E_INTEGRITY;

  if (comm && comm->Attributes && commGetAttr(comm, keyval, &value)) {
    pentry = &(keystab->Keys[keyval]);
    if (0 != ((pentry->DelFxn)(comm, keyval, value, pentry->ExtraState)))
                                                 return KEY_E_INTERFACE;
    commDelAttr (comm, keyval);
    keyFree(keystab, keyval);
  }

  return KEY_E_OK;
}


/*
 *  keyDelAllAttr
 */
int keyDelAllAttr (KeysTable *keystab, Mpi_P_Comm *comm) {

  int i;


  if (keystab == (KeysTable *)NULL)              return KEY_E_OK;

  for (i = 0; i < DEFAULT_ATTRIBUTES_COUNT; i++) {
    if (KEY_isAlloc(keystab, i)) {
      if (0 > keyDelAttr(keystab, comm, i))      return KEY_E_INTERFACE;
    }
  }

  return KEY_E_OK;
}


/*
 *  keyCopyAllAttr
 */
int keyCopyAllAttr (KeysTable *keystab, Mpi_P_Comm *comm, Mpi_P_Comm *newcomm) {

  Key   *pentry;
  int    i;
  void  *value_in;
  void  *value_out;
  int    flag;


  if (keystab == (KeysTable *)NULL)              return KEY_E_OK;

  for (i = 0; i < DEFAULT_ATTRIBUTES_COUNT; i++) {

    pentry = &(keystab->Keys[i]);
    if (pentry->Allocated) {

      if (commGetAttr(comm, i, &value_in)) {

        if (0 != ((pentry->CopyFxn)(comm, i, pentry->ExtraState, value_in, &value_out, &flag)))
                                                 return KEY_E_INTERFACE;

        if (flag) {
          commPutAttr (newcomm, i, value_out);
          pentry->Refs++;
        }

	    }

    }

  }

  return KEY_E_OK;
}


/*
 *  keyCopyDfltAttr
 */
int keyCopyDfltAttr (KeysTable *keystab, Mpi_P_Comm *comm, Mpi_P_Comm *newcomm) {

  Key   *pentry;
  int    i;
  void  *value_in;
  void  *value_out;
  int    flag;

  if (keystab == (KeysTable *)NULL)              return KEY_E_OK;

  for (i = 0; i < LAST_DEFAULT_KEYVAL; i++) {

    pentry = &(keystab->Keys[i]);
    if (!pentry->Allocated)                      return KEY_E_INTEGRITY;

    if (commGetAttr(comm, i, &value_in)) {

      if (0 != ((pentry->CopyFxn)(comm, i, pentry->ExtraState, value_in, &value_out, &flag)))
                                                 return KEY_E_INTERFACE;

      if (flag) {
        commPutAttr (newcomm, i, value_out);
        pentry->Refs++;
      }

	}

  }

  return KEY_E_OK;
}


  /*-------------------------------------------------------/
 /    Implementation of private DEBUG interface           /
/-------------------------------------------------------*/

#ifdef DEBUG_MODE

PRIVATE int printKeyEntries (KeysTable *keystab) {

  int i;

  fprintf(stdout, " >>>>>> KEYS ENTRIES (TASK %d) <<<<<<  (KEY 0x%x  size %d)\n", getRank(), keystab->Keys, keystab->Count);
  for(i = 0; i < keystab->Count; i++) {
    fprintf(stdout, "\nKey Entry %d  --  ADDR  %x  -- ", i, &keystab->Keys[i]);
    fprintf(stdout, "Extra_State: 0x%x   Refs %d\n", keystab->Keys[i].ExtraState, keystab->Keys[i].Refs);
  }
  fprintf(stdout, "\n-----------------------------------------\n");

  return 0;
}

#endif
