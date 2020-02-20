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
#define self()        ((Thr_t)pthread_getspecific(key))



      /*________________________________________________________________
     /                                                                  \
    |    test                                                            |
    |    Test if an async request has finished                           |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o rqst     (Input)                                              |
    |        The tested asynchronous request previously done             |
    |    o flag (Output)                                                 |
    |        TRUE  if the request is done                                |
    |        FALSE if the request not done                               |
    |    o status   (Output)                                             |
    |        Status of the satisfied request                             |
    |                                                                    |
    |    RETURN:                                                         |
    |    = 0 : On success                                                |
    |    < 0 : On other case                                             |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int test (Rqst_t *rqst, int *flag, Status *status) {

  Thr_t    me       = self();

  DBG_PRNT((stdout, "\ntest(%p): \n", me));

  *flag = FALSE;

  LOCK(me);

  if((*rqst == AZQ_RQST_NULL) || RQST_isInactive(*rqst)) {
    UNLOCK(me);
    RQST_setEmptyStatus(status);
    *flag = TRUE;
    return AZQ_SUCCESS;
  }

  if(RQST_isSatisfied(*rqst) || RQST_isCancelled(*rqst)) {

    *flag = TRUE;

    RQST_getStatus(*rqst, status);

    if (RQST_isPersistent(*rqst)) { RQST_clear(*rqst);   }
    else                          { RQST_destroy(*rqst); }

  }

  UNLOCK(me);

  DBG_PRNT((stdout, "test(%p): End\n", me));
  return AZQ_SUCCESS;
}
