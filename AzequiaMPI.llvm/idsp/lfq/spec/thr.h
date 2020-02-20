/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


#ifndef _THR_H_
#define _THR_H_

/*----------------------------------------------------------------*
 *   Declaration of types used by this module                     *
 *----------------------------------------------------------------*/
#include <addr.h>
#include <thr_dptr.h>


/*----------------------------------------------------------------*
 *   Constants                                                    *
 *----------------------------------------------------------------*/
/* Modes of THR_setmask */
#define MASK_ON           1
#define MASK_OFF          0

#define THR_NO_RANK      (-1)

    /*----------------------------------------------------------------*
     *   Exported exceptions                                          *
     *----------------------------------------------------------------*/
#define THR_E_OK          0
#define THR_E_EXHAUST    (THR_E_OK          - 1)
#define THR_E_INTEGRITY  (THR_E_EXHAUST     - 1)
#define THR_E_TIMEOUT    (THR_E_INTEGRITY   - 1)
#define THR_E_INTERFACE  (THR_E_TIMEOUT     - 1)
#define THR_E_SYSTEM     (THR_E_INTERFACE   - 1)
#define THR_E_SIGNALED   (THR_E_SYSTEM      - 1)
#define THR_E_DEADPART   (THR_E_SIGNALED    - 1)


/*----------------------------------------------------------------*
 *   Declaration of public opaque types                           *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*
 *   Declaration of public types                                  *
 *----------------------------------------------------------------*/
#define COMMATTR_FLAGS_SLM_ENABLED     0x00000001
struct CommAttr {
  int   Flags;
  /* int   SlmBufferNr; */
};
typedef struct CommAttr CommAttr, *CommAttr_t;

/*----------------------------------------------------------------*
 *   Declaration of public interface                              *
 *----------------------------------------------------------------*/

extern int       THR_init              (void);
extern void      THR_finalize          (void);
extern int       THR_create            (Thr_t *thr, Addr_t addr,  
                                                    int  (*bodyFxn)(int, char **),
                                                    int    stackSize, 
                                                    char **argv, 
						                            int    argc, 
                                              CommAttr_t   comAttr);
extern void      THR_destroy           (Thr_t  thr);
extern void      THR_wait              (Thr_t  thr, int *state);
extern int       THR_start             (Thr_t  thr);
extern void      THR_kill              (Thr_t  thr, int  signal);
extern void      THR_setmask           (Thr_t  thr, int  mode);
extern pthread_t THR_getSelf           (Thr_t  thr);
extern Thr_t     THR_self              (void);
extern void      THR_getClient         (Addr  *client, int *mchn);
extern void      THR_setClient         (Addr  *client, int  mchn);
extern void      THR_exit              (int    code);
extern void      THR_installLeave      (void (*leaveUpcall)(int code));
extern void      THR_setPlacementInfo  (Thr_t thr, Placement_t place);

#define          THR_getLocalRank(thr) (thr)->LocalRank
#define          THR_setLocalRank(thr, localRank) (thr)->LocalRank = (localRank)


#endif
