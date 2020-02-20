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
#include <p_errhnd.h>

  /*----------------------------------------------------------------/
 /   Declaration of types and functions used by this module        /
/----------------------------------------------------------------*/
#if defined (__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
#endif

#include <azq_types.h>

#include <mpi.h>
#include <p_config.h>

  /*-------------------------------------------------------/
 /                 Private interface                      /
/-------------------------------------------------------*/
#ifdef DEBUG_MODE
PRIVATE int                printHandlers  (ErrHndTable *errhndtable, int index);
#endif

  /*--------------------------------------------------------------------------/
 /    Implementation of exported interface for error handling management     /
/--------------------------------------------------------------------------*/
/*
 *  errhndAllocTable
 *    Allocate memory for the table of error handlers.
 *    Returns the bytes allocated.
 */
int errhndAllocTable (ErrHndTable_t *errhndtable) {

  Mpi_P_Errhandler  *errhnd;
  ErrHndTable       *ehTable;
  int                i, error;
//fprintf(stdout, "\nerrhndAllocTable (%p): BEGIN\n", self()); fflush(stdout);

  /* 1. Allocate space for error handlers table */
  if (posix_memalign((void *)&ehTable, CACHE_LINE_SIZE, sizeof(struct ErrHndTable))) 
	return HND_E_EXHAUST;
  
  objInit (&errhnd, PREALLOC_MAX_ERRHND_BLOCKS, 
                    sizeof(Mpi_P_Errhandler), 
                    PREALLOC_ERRHND_COUNT, 
                    BLOCK_ERRHND_COUNT, &error) 
  if(error) goto exception;
  ehTable->ErrorHandlers = errhnd;
  *errhndtable = ehTable;
//fprintf(stdout, "errhndAllocTable (%p): END\n", self()); fflush(stdout);
  return ((PREALLOC_ERRHND_COUNT * sizeof (struct Mpi_Errhandler)) + sizeof(struct ErrHndTable));
  
exception:
  free(ehTable);
  return -1;
}


/*
 *  errhndFreeTable
 *    Free the error handler table.
 */
int errhndFreeTable (ErrHndTable_t *errhndtable) {

  Mpi_P_Errhandler *errhnd;
  int i;

  /*****  ??????  ******
  for (i = 0; i < (*errhndtable)->Size; i++) {
    errhnd = &((*errhndtable)->ErrorHandlers[i]);
    if (errhnd->Allocated == TRUE)
      errhndDelete(*errhndtable, &errhnd);
  }
  /*****  ??????  ******/
  
  objFinalize(&(*errhndtable)->ErrorHandlers);
  
  free(*errhndtable);

  *errhndtable = (ErrHndTable *)NULL;

  return HND_E_OK;
}


/*
 *  errhndCreate
 *    Create a new entry for a new error handler
 */
int errhndCreate (ErrHndTable *errhndtable, Mpi_P_Handler_function *function, Mpi_P_Errhandler_t *errhnd) {

  Mpi_P_Errhandler *eh;
  int               idx;
  
  /*
  if (0 > objAlloc(errhndtable->ErrorHandlers, &eh, &idx)) {
	*errhnd = (Mpi_P_Errhandler *)NULL;
    return(HND_E_EXHAUST);
  }
  */
  objAlloc(errhndtable->ErrorHandlers, &eh, &idx);
  eh->Index = idx;
  
  eh->Refs = 1;
  eh->Function = function;

  *errhnd = eh;

#ifdef DEBUG_MODE
  printHandlers(errhndtable, -1);
#endif

  return HND_E_OK;
}


/*
 *  errhndDelete
 *    Delete an error handler.
 *    An error handler is deleted when all references to it are released
 */
int errhndDelete (ErrHndTable *errhndtable, Mpi_P_Errhandler_t *errhnd) {

  Mpi_P_Errhandler *eh = *errhnd;

  eh->Refs--;
  if (eh->Refs == 0) 
	objFree(errhndtable->ErrorHandlers, errhnd, (*errhnd)->Index);

  *errhnd = (Mpi_P_Errhandler *) NULL;

  return HND_E_OK;
}


/*
 *  errhndExec
 *    Execute an error handler
*/
int errhndExec (Mpi_P_Errhandler *errhnd, Mpi_P_Comm_t *comm, int *errorcode, char *where) {

  (errhnd->Function)(comm, errorcode, where);

  return HND_E_OK;
}


/*
 *  errhndGetRef
 *    New reference to an error handler
 */
Mpi_P_Errhandler *errhndGetRef (Mpi_P_Errhandler *errhnd) {

  errhnd->Refs++;

  return errhnd;
}

  /*-------------------------------------------------------/
 /    Implementation of private DEBUG interface           /
/-------------------------------------------------------*/

#ifdef DEBUG_MODE

PRIVATE int printHandlers (ErrHndTable *errhndtable, int index) {

  int i;

  fprintf(stdout, ">>>>>  Error Handlers Table (Addr %x)  <<<<<\n", errhndtable);

  /* TODO */
  /*
   objPrint(table);
   */
  
  return 0;
}

#endif
