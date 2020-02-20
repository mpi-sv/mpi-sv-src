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

#include <azq_types.h>
#include <xpn.h>
#include <addr.h>
#include <addr_hddn.h>
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


#define self()        ((Thr_t)pthread_getspecific(key))


      /*________________________________________________________________
     /                                                                  \
    |    timed_waitany                                                  |
    |    Block the invoking thread until                                 |
    |    1) Any of the requests in the "rqst" vector                     |
    |       a)  is satisfied                                             |
    |       b)  times out                                                |
    |    2) It is killed                                                 |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o rqst     (Input)                                              |
    |        Vector of requests                                          |
    |    o count    (Input)                                              |
    |        Dimension of the "rqst" request vector                      |
    |    o index    (Output)                                             |
    |        The either satisfied or timedout request in "rqst"          |
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
int timed_waitany(Rqst_t *rqst, int count, int *index, Status *status, unsigned timeout) {

  int      i = 0,
           nullCnt  = 0,
           excpn    = AZQ_SUCCESS;
  Thr_t    me       = self();
  static
  char    *where    = "timed_waitany";


  DBG_PRNT((stdout, "\ntimed_waitany(%p). Rqst: %p \n", me, *rqst));

  if(rqst == (Rqst_t *)AZQ_RQST_NULL)                                          {excpn = AZQ_E_REQUEST;
                                                                                goto exception;}
  LOCK(me);

  me->SatisfiedRqst = NONE_SATISFIED;
  for(i = 0; i < count; i++) {

    if((rqst[i] == AZQ_RQST_NULL) || RQST_isInactive(rqst[i])) {

      nullCnt++;

    } else {

      rqst[i]->WaitIdx = i;

      if (rqst[i]->State == RQST_SATISFIED) {

        me->SatisfiedRqst = i;
        klee_mpi_nonblock(-1, -1, 7,(void *)&rqst[i]);
        break;

      } else if (rqst[i]->State == RQST_CANCELLED) {

        nullCnt++;

      } else if (rqst[i]->State == RQST_PENDING) {

        rqst[i]->State = RQST_MAKES_WAITING;

      }

    }

    if(nullCnt == count) {
      *index = AZQ_UNDEFINED;
      UNLOCK(me);
      return AZQ_SUCCESS;
    }

  }

  if(me->SatisfiedRqst == NONE_SATISFIED) {
    me->WaitRqstLeftCnt   = 1;
    me->WaitAnyRqstVector = rqst;
    me->WaitAnyRqstCnt    = count;
    excpn = TIMEDWAIT(me, me, timeout);

    DBG_PRNT((stdout, "timed_waitany(%p): Tras TIMEDWAIT. excpn = %d\n", me, excpn));
    if(0 > excpn) XPN_print(excpn);
  }

  DBG_PRNT((stdout, "timed_waitany(%p): %d satisfied\n", me, me->SatisfiedRqst));

  for(i = 0; i < count; i++) {
    if ((rqst[i] == AZQ_RQST_NULL) || RQST_isInactive(rqst[i]))  continue;
    if (rqst[i]->State == RQST_MAKES_WAITING)
      rqst[i]->State = RQST_PENDING;
  }

  UNLOCK(me);

  /* Build the status */
  if(excpn == AZQ_SUCCESS) {
    RQST_getStatus(rqst[me->SatisfiedRqst], status);
  }

  if (RQST_isPersistent(rqst[me->SatisfiedRqst])) {
    RQST_clear(rqst[me->SatisfiedRqst]);
  } else {
    RQST_destroy(rqst[me->SatisfiedRqst]);
  }

  *index = me->SatisfiedRqst;

  DBG_PRNT((stdout, "timed_waitany(%p): End\n", me));
  return AZQ_SUCCESS;

exception:
  XPN_print(excpn);
  return(azq_err[-excpn]);
}


