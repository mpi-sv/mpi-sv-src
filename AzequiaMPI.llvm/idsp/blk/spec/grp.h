/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


#ifndef _GRP_H_
#define _GRP_H_

/*----------------------------------------------------------------*
 *   Declaration of types used by this module                     *
 *----------------------------------------------------------------*/
#include <rpc.h>
#include <thr.h>
#include <azq_types.h>

/*----------------------------------------------------------------*
 *   Declaration of exported constants                            *
 *----------------------------------------------------------------*/
#define GIX_NONE          0
#define GRP_PORT          (NO_PORT + 1)

/* Number of groups allowed to launch by users */
#define USER_GROUPS       8

    /*----------------------------------------------------------------*
     *   Exported exceptions                                          *
     *----------------------------------------------------------------*/
#define GRP_E_OK          0
#define GRP_E_EXHAUST    (GRP_E_OK          - 1)
#define GRP_E_INTEGRITY  (GRP_E_EXHAUST     - 1)
#define GRP_E_TIMEOUT    (GRP_E_INTEGRITY   - 1)
#define GRP_E_INTERFACE  (GRP_E_TIMEOUT     - 1)
#define GRP_E_SYSTEM     (GRP_E_INTERFACE   - 1)
#define GRP_E_SIGNALED   (GRP_E_SYSTEM      - 1)
#define GRP_E_DEADPART   (GRP_E_SIGNALED    - 1)


/*----------------------------------------------------------------*
 *   Declaration of the module interface                          *
 *----------------------------------------------------------------*/
extern  int   GRP_init         ();
extern  void  GRP_finalize     (void);
extern  int   GRP_create       (int *gix, int *mchn,  int    size, int crtr);
extern  int   GRP_getSize      (int  gix, int *size);
extern  int   GRP_getLocalSize (int  gix, int *locSize);
extern  int   GRP_getMchn      (int  gix, int  rank,  int   *mchn);
extern  int   GRP_getThread    (int  gix, int  rank,  Thr_t *thr);
extern  int   GRP_join         (int  gix, int  rank,  int    name, CommAttr *commAttr);
extern  int   GRP_destroy      (int  gix);
extern  int   GRP_shutdown     (void);
extern  int   GRP_start        (int  gix);
extern  int   GRP_kill         (int  gix);
extern  int   GRP_wait         (int  gix, int *status);
extern  int   GRP_wait2        (int  gix, int *status);
extern  int   GRP_enroll       (void);


#endif

