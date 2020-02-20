/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   implemented by this module                                   *
 *----------------------------------------------------------------*/
#include <com.h>

/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#if defined (__OSI)
  #include <osi.h>
#else
  #include <pthread.h>
  #include <string.h>
  #include <stdio.h>
  #include <errno.h>
#endif

#include <atomic.h>
#include <azq_types.h>
#include <xpn.h>
#include <addr.h>
#include <inet.h>
#include <thr.h>
#include <thr_dptr.h>
#include <mbx.h>
#include <rpq.h>
#include <rqst.h>
#include <util.h>


/*----------------------------------------------------------------*
 *   Definition of private constants                              *
 *----------------------------------------------------------------*/
static const
       char  *e_names[10] = { /* This order has to be consistent with com.h */
                              /*  0 */ "COM_E_OK",
                              /*  1 */ "COM_E_EXHAUST",
                              /*  2 */ "COM_E_INTEGRITY",
                              /*  3 */ "COM_E_TIMEOUT",
                              /*  4 */ "COM_E_INTERFACE",
                              /*  5 */ "COM_E_SYSTEM",
                              /*  6 */ "COM_E_SIGNALED",
                              /*  7 */ "COM_E_DEADPART",
                              /*  8 */ "COM_E_REQUEST",
                              /*  9 */ "COM_E_INTERN"
                            };


      /*________________________________________________________________
     /                                                                  \
    |    cancel                                                          |
    |    Try to cancel a non-blocking or persistent request              |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o rqst     (Input / Output)                                     |
    |        Request to cancel                                           |
    |                                                                    |
    |    RETURN:                                                         |
    |    0  : On success                                                 |
    |    < 0: On other case                                              |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int AZQ_cancel(Rqst_t rqst) {

  int      excpn;
  static
  char    *where       = "AZQ_cancel";

  DBG_PRNT((stdout, "\nAZQ_cancel(%p): \n", self()));

  if(rqst == (Rqst_t)AZQ_RQST_NULL)                                             {excpn = AZQ_E_REQUEST;
                                                                                 goto retorno;}
  /* The state must be RQST_PENDING to be successfully cancelled */
  if (RQST_isRecv(rqst)) {
    if(rqst->State == RQST_PENDING) {
      RQST_setCancelled(rqst);
      rqst->Status.Cancelled = 1;
      RPQ_remove(&me->RecvPendReg, rqst);
    }
  }
  else {
    rqst->PersistentWait = TRUE;
    /* To cancel "rqst", needs to assure that no receive has seen it */
	rqst->Status.Cancelled = 0;
    if(__sync_bool_compare_and_swap(&rqst->State, RQST_PENDING, RQST_CANCELLED)) {
      rqst->Status.Cancelled = 1;
    }
    
  }
  DBG_PRNT((stdout, "AZQ_cancel(%p). Rqst: %p with state %x END\n", self(), rqst, (rqst)->State));

  return AZQ_SUCCESS;

retorno:
  XPN_print(excpn);
  return(azq_err[-excpn]);
}


