/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


#ifndef _OPR_H_
#define _OPR_H_

/*----------------------------------------------------------------*
 *   Declaration of types used by this module                     *
 *----------------------------------------------------------------*/
#include <azq_types.h>

    /*----------------------------------------------------------------*
     *   Exported exceptions                                          *
     *----------------------------------------------------------------*/
/* Maximum number of types of operators */
#define MAX_TYPE_OPERATORS     64


#define OPR_E_OK          0
#define OPR_E_EXHAUST    (OPR_E_OK          - 1)
#define OPR_E_INTEGRITY  (OPR_E_EXHAUST     - 1)
#define OPR_E_TIMEOUT    (OPR_E_INTEGRITY   - 1)
#define OPR_E_INTERFACE  (OPR_E_TIMEOUT     - 1)
#define OPR_E_SYSTEM     (OPR_E_INTERFACE   - 1)
#define OPR_E_SIGNALED   (OPR_E_SYSTEM      - 1)
#define OPR_E_DEADPART   (OPR_E_SIGNALED    - 1)


/*----------------------------------------------------------------*
 *   Definition of private data types                             *
 *----------------------------------------------------------------*/
struct REG_Entry {
  int     Name;           /* Globally known program name           */
  void   *Function;       /* Body Function pointer in this machine */
  int     Stack_Size;     /* Body Function stack size              */
  int     Argc;           /* main argc value copy                  */
  char  **Argv;           /* main argv value copy                  */
};
typedef struct REG_Entry REG_Entry, *REG_Entry_t;


/*----------------------------------------------------------------*
 *   Declaration of public interface                              *
 *----------------------------------------------------------------*/
extern int  OPR_register     (REG_Entry  *opertab,     int count);
extern int  OPR_getStackSize (int   *stackSize,        int name);
extern int  OPR_getBody      (void **function,         int name);
extern int  OPR_getParams    (int *argc, char ***argv, int name);

#endif
