/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


/*----------------------------------------------------------------*
 *   Declaration of public functions implemented by this module   *
 *----------------------------------------------------------------*/
#if defined(__OSI)
  #include <osi.h>
#else
  #include <pthread.h>
  #include <limits.h>
#endif

#include <opr.h>

/*----------------------------------------------------------------*
 *   Declaration of types and functions used by this module       *
 *----------------------------------------------------------------*/
#include <xpn.h>
#include <com.h>

/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
static REG_Entry  lnkTable[MAX_TYPE_OPERATORS];
static int        lnkTableDim = 0;
static char *e_names[8] = {  /*  0 */ "OPR_E_OK",
                             /*  1 */ "OPR_E_EXHAUST",
                             /*  2 */ "OPR_E_INTEGRITY",
                             /*  3 */ "OPR_E_TIMEOUT",         // This order has to be consistent
                             /*  4 */ "OPR_E_INTERFACE",       // with opr.h
                             /*  5 */ "OPR_E_SYSTEM",
                             /*  6 */ "OPR_E_SIGNALED",
                             /*  7 */ "OPR_E_DEADPART"
                           };

/*----------------------------------------------------------------*
 *   Declaration of private functions implemented by this module  *
 *----------------------------------------------------------------*/


/*----------------------------------------------------------------*
 *   Implementation of private functions                           *
 *----------------------------------------------------------------*/


/*----------------------------------------------------------------*
 *   Implementation of public functions                           *
 *----------------------------------------------------------------*/



    /*------------------------------------------------------------*\
   |*  OPR_register                                                *|
   |*                                                              *|
    \*------------------------------------------------------------*/
int OPR_register (REG_Entry *opertab, int count) {

  int         excpn;
  int         i;
  REG_Entry  *entry;
  char       *where = "OPR_register";


  if ((lnkTableDim + count) > MAX_TYPE_OPERATORS)                              {excpn = OPR_E_EXHAUST;
                                                                                goto exception;}

  if((NULL == opertab) || (count <= 0))                                        {excpn = OPR_E_INTERFACE;
                                                                                goto exception;}

  entry = &lnkTable[lnkTableDim];
  for(i = 0; i < count; i++) {
#ifdef _DEBUG
    fprintf(stdout, "Operator %d in table entry %d \n", opertab[i].Name, lnkTableDim);
#endif
    entry->Name       = opertab[i].Name;
    entry->Function   = opertab[i].Function;
	if (opertab[i].Stack_Size < PTHREAD_STACK_MIN)
	  entry->Stack_Size = PTHREAD_STACK_MIN;
	else
      entry->Stack_Size = opertab[i].Stack_Size;
	
	entry->Argc = opertab[i].Argc;
	entry->Argv = opertab[i].Argv;
	
	lnkTableDim++;
    entry++;
  }

  return(OPR_E_OK);

exception:
  XPN_print(excpn);
  return(excpn);
}


     /*----------------------------------------------------------------*\
    |    OPR_getStackSize                                                |
    |                                                                    |
    |                                                                    |
     \*----------------------------------------------------------------*/
int  OPR_getStackSize(int *stackSize, int name) {

  char   *where = "OPR_getStackSize";
  int     excpn;
  int     i;

  /* Given the code of the Operator, search its entry */
  for(i = 0; i < lnkTableDim; i++) {
    if(lnkTable[i].Name == name) {  /* entry found !! */
      *stackSize = lnkTable[i].Stack_Size;
      return(0);
    }
  }

  excpn = OPR_E_INTEGRITY;
  XPN_print(excpn);
  return(excpn);
}


     /*----------------------------------------------------------------*\
    |    OPR_getBody                                                     |
    |                                                                    |
    |                                                                    |
     \*----------------------------------------------------------------*/
int  OPR_getBody(void **function, int name) {

  char   *where = "OPR_getBody";
  int     excpn;
  int     i;


  /* Given the code of the Operator, search its entry */
  for(i = 0; i < lnkTableDim; i++) {
    if(lnkTable[i].Name == name) {  /* entry found !! */
      *function = lnkTable[i].Function;
      return(0);
    }
  }

  excpn = OPR_E_INTEGRITY;
  XPN_print(excpn);
  return(excpn);
}


     /*----------------------------------------------------------------*\
    |    OPR_getParams                                                   |
    |                                                                    |
    |                                                                    |
     \*----------------------------------------------------------------*/
int  OPR_getParams(int *argc, char ***argv, int name) {
  
  char   *where = "OPR_getParams";
  int     excpn;
  int     i;
  
  
  /* Given the code of the Operator, search its entry */
  for(i = 0; i < lnkTableDim; i++) {
    if(lnkTable[i].Name == name) {  /* entry found !! */
      *argv = lnkTable[i].Argv;
	  *argc = lnkTable[i].Argc;
      return(0);
    }
  }
  
  excpn = OPR_E_INTEGRITY;
  XPN_print(excpn);
  return(excpn);
}
