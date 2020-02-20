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
    |    timed_waitall                                                  |
    |    Block the invoking thread until                                 |
    |    1) All of the requests in the "rqst" vector are satisfied       |
    |    2) It is killed                                                 |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o rqst     (Input)                                              |
    |        Vector of requests                                          |
    |    o count    (Input)                                              |
    |        Dimension of the "rqst" request vector                      |
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
int timed_waitall(Rqst_t *rqst, int count, Status *status, unsigned  timeout) {

  int      i,
           excpn       = AZQ_SUCCESS;
  Thr_t    me          = self();
  static
  char    *where       = "timed_waitall";

  DBG_PRNT((stdout, "\ntimed_waitall(%p): \n", me));

  if(rqst == (Rqst_t *)AZQ_RQST_NULL)                                         {excpn = AZQ_E_REQUEST;
                                                                               goto exception;}
  LOCK(me);

  me->WaitRqstLeftCnt = 0;
  for(i = 0; i < count; i++) {

    if((rqst[i] == AZQ_RQST_NULL) || RQST_isInactive(rqst[i]))  continue;

    if (rqst[i]->State == RQST_PENDING) {
      me->WaitRqstLeftCnt += 1;
      rqst[i]->State = RQST_MAKES_WAITING;
    }

  }

  if(me->WaitRqstLeftCnt) {
    me->WaitAnyRqstVector = rqst;
    me->WaitAnyRqstCnt    = count;
    excpn = TIMEDWAIT(me, me, timeout);

    if(0 > excpn)
      DBG_PRNT((stdout, "timed_waitall(%p): excpn = %d, TIMEOUT !!!!\n", me, excpn));
  }

  DBG_PRNT((stdout, "timed_waitall(%p): %d rqsts satisfied\n", me, count));

  UNLOCK(me);

  for(i = 0; i < count; i++) {

    if (excpn) {

      if (&status[i] != AZQ_STATUS_IGNORE) {
        if(RQST_isSatisfied(rqst[i]) || RQST_isCancelled(rqst[i])) {
          status[i].Error = AZQ_WAIT_SUCCESS;
        } else {
          status[i].Error = AZQ_WAIT_ERR_PENDING;
        }
      }

    } else {

      if((rqst[i] == AZQ_RQST_NULL) || RQST_isInactive(rqst[i]))  {

        if ((status != AZQ_STATUSES_IGNORE) && (&status[i] != AZQ_STATUS_IGNORE))
		  RQST_setEmptyStatus(&status[i]);

      } else {

		if ((status != AZQ_STATUSES_IGNORE) && (&status[i] != AZQ_STATUS_IGNORE))
          RQST_getStatus(rqst[i], &status[i]);

        if (RQST_isPersistent(rqst[i])) { RQST_clear(rqst[i]);   }
        else                            { RQST_destroy(rqst[i]); }

      }
      klee_mpi_nonblock(-1, -1, 7,(void *)&rqst[i]);
    }

  }

  DBG_PRNT((stdout, "timed_waitall(%p): End\n", me));
  return AZQ_SUCCESS;

exception:
  XPN_print(excpn);
  return(azq_err[-excpn]);
}


