/*-
 * Copyright (c) 2011 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */



/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   implemented by this module                                   *
 *----------------------------------------------------------------*/
#include <rqst.h>


/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <string.h>
#endif

#include <xpn.h>
#include <thr_dptr.h>
#include <azq_types.h>
#include <com.h>

extern pthread_key_t   key;
#define self()         ((Thr_t)pthread_getspecific(key))
extern  void panic     (char *where);



/*----------------------------------------------------------------*
 *   Definition of private macros                                 *
 *----------------------------------------------------------------*/
#ifdef __RQST_DEBUG
#define DBG_PRNT(pmsg)  fprintf pmsg
#else
#define DBG_PRNT(pmsg)
#endif


/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
static const
       char  *e_names[8] = { // This order has to be consistent with com.h
                             /*  0 */ "RQST_E_OK",
                             /*  1 */ "RQST_E_EXHAUST",
                             /*  2 */ "RQST_E_INTEGRITY",
                             /*  3 */ "RQST_E_TIMEOUT",
                             /*  4 */ "RQST_E_INTERFACE",
                             /*  5 */ "RQST_E_SYSTEM",
                             /*  6 */ "RQST_E_SIGNALED",
                             /*  7 */ "RQST_E_DEADPART"
                           };


/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/


/*----------------------------------------------------------------*
 *   Public variables                                             *
 *----------------------------------------------------------------*/



/*----------------------------------------------------------------*
 *   Implementation of public interface                           *
 *----------------------------------------------------------------*/


      /*________________________________________________________________
     /                                                                  \
    |    RQST_ready                                                      |
    |                                                                    |
    |    Mark a request as ready                                         |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o dstThr       (Input)                                          |
    |        Thread where is the request                                 |
    |    o rqst         (Output)                                         |
    |        Request                                                     |
    |                                                                    |
    |    RETURN:                                                         |
    |    = TRUE  : if rqst get ready                                     |
    |    = FALSE : if not                                                |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int RQST_ready (void *dstThr, Rqst_t rqst) {

  Thr_t  thr = (Thr_t)dstThr;

  DBG_PRNT((stdout, "RQST_ready(%p): rqst %p has state %x\n", self(), rqst, (rqst)->State));

  if(rqst->State == RQST_MAKES_WAITING) {
    return (TRUE);
  }
  return (FALSE);
}



