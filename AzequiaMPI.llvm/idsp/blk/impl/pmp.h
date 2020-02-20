/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


#ifndef _PMP_H_
#define _PMP_H_

/*----------------------------------------------------------------*
 *   Declaration of constants and types used by this file         *
 *----------------------------------------------------------------*/


/*----------------------------------------------------------------*
 *   1. Declaration of constants                                  *
 *----------------------------------------------------------------*/
#define PMP_GIX  1  /*(GIX_NONE + 1)*/

    /*----------------------------------------------------------------*
     *   Exported exceptions                                          *
     *----------------------------------------------------------------*/
#define PMP_E_OK          0
#define PMP_E_EXHAUST    (PMP_E_OK          - 1)
#define PMP_E_INTEGRITY  (PMP_E_EXHAUST     - 1)
#define PMP_E_TIMEOUT    (PMP_E_INTEGRITY   - 1)
#define PMP_E_INTERFACE  (PMP_E_TIMEOUT     - 1)
#define PMP_E_SYSTEM     (PMP_E_INTERFACE   - 1)
#define PMP_E_SIGNALED   (PMP_E_SYSTEM      - 1)
#define PMP_E_DEADPART   (PMP_E_SIGNALED    - 1)


/*----------------------------------------------------------------*
 *   2. Declaration of public function interface                  *
 *----------------------------------------------------------------*/
extern int   PMP_init       ();
extern void  PMP_finalize   (void);
extern int   PMP_shutdown   (void);
extern int   PMP_get        (int svc, int *gix, int *mchn);
extern int   PMP_getLoad    (int *load);
extern int   PMP_register   (int svc, int  gix);
extern void  PMP_unregister (int svc);

#endif
