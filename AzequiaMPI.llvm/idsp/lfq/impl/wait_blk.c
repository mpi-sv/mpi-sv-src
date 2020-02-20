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
#include <rpc.h>
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

#include <azq_types.h>
#include <xpn.h>
#include <addr.h>
#include <inet.h>
#include <thr.h>
#include <thr_dptr.h>
#include <mbx.h>
#include <rqst.h>
#include <util.h>


/*----------------------------------------------------------------*
 *   Definition of private constants                              *
 *----------------------------------------------------------------*/
static const
       char  *e_names[8] = { /* This order has to be consistent with com.h */
                             /*  0 */ "COM_E_OK",
                             /*  1 */ "COM_E_EXHAUST",
                             /*  2 */ "COM_E_INTEGRITY",
                             /*  3 */ "COM_E_TIMEOUT",
                             /*  4 */ "COM_E_INTERFACE",
                             /*  5 */ "COM_E_SYSTEM",
                             /*  6 */ "COM_E_SIGNALED",
                             /*  7 */ "COM_E_DEADPART"
                           };


#define self()        ((Thr_t)pthread_getspecific(key))


      /*________________________________________________________________
     /                                                                  \
    |    timed_wait                                                      |
    |    Block the invoking thread until                                 |
    |    1) The request                                                  |
    |       a)  is satisfied                                             |
    |       b)  times out                                                |
    |    2) It is killed                                                 |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o rqst     (Input)                                              |
    |        Request                                                     |
    |    o status   (Output)                                             |
    |        Status of the satisfied request                             |
    |    o timeout  (Input)                                              |
    |        Relative timeout (in milliseconds)                          |
    |                                                                    |
    |    RETURN:                                                         |
    |    = 0 : On success                                                |
    |    < 0 : On other case                                             |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int rpc_timed_wait(Rqst_t *rqst, Status *status, unsigned timeout) {

  int      excpn;
  Thr_t    me          = self();
  static
  char    *where       = "rpc_timed_wait";

/*if((*rqst == AZQ_RQST_NULL) || RQST_isInactive(*rqst))                       {excpn = COM_E_INTEGRITY;
                                                                                RQST_setEmptyStatus(status);
                                                                                goto exception;}
*/
  DBG_PRNT((stdout, "\ntimed_wait(%p). Rqst: %p with state %x\n", me, *rqst, (*rqst)->State));

  RPC_LOCK(me);
  switch((*rqst)->State) {
    case RQST_PENDING:
      (*rqst)->State = RQST_MAKES_WAITING;
      DBG_PRNT((stdout, "\ntimed_wait(%p): timedwait\n", me));
      if (0 > (excpn = TIMEDWAIT(me, me, timeout)))                            {RPC_UNLOCK(me);
                                                                                goto exception;}
      break;
    case RQST_CANCELLED:
      break;
    case RQST_SATISFIED:
      break;
  }

  RPC_UNLOCK(me);
  if(status != AZQ_STATUS_IGNORE)
    RQST_getStatus(*rqst, status);

  if (RQST_isPersistent(*rqst))  { /*RQST_clear(*rqst); */  }
  else                           { /*RQST_destroy(*rqst); */}

  DBG_PRNT((stdout, "timed_wait(%p): End.\n", me));

  return AZQ_SUCCESS;

exception:
  XPN_print(excpn);
  return(azq_err[-excpn]);
}


