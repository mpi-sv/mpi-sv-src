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
    |    testall                                                         |
    |    Test if all async request has finished                          |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o rqst (Input)                                                  |
    |        The tested asynchronous array of requests previously done   |
    |    o cnt (Input)                                                   |
    |        How many requests to test                                   |
    |    o flag (Output)                                                 |
    |        TRUE  if the request is done                                |
    |        FALSE if the request not done                               |
    |    o status (Output)                                               |
    |        The array of statuses                                       |
    |                                                                    |
    |    RETURN:                                                         |
    |    = 0 : On success                                                |
    |    < 0 : On other case                                             |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int testall (Rqst_t *rqst, int cnt, int *flag, Status *status) {

  int      i,
           excpn       = AZQ_SUCCESS;
  int      satfd       = 0;
  Thr_t    me          = self();
  static
  char    *where       = "testall";

  DBG_PRNT((stdout, "\ntestall(%p): \n", me));

  if(rqst == (Rqst_t *)AZQ_RQST_NULL)                                          {excpn = AZQ_E_REQUEST;
                                                                                goto exception;}
  *flag = FALSE;

  LOCK(me);

  for(i = 0; i < cnt; i++) {

    if (rqst[i] == AZQ_RQST_NULL)  {
      satfd++;
      continue;
    }

    if ( RQST_isInactive(rqst[i])     ||
         RQST_isSatisfied(rqst[i])    ||
         RQST_isCancelled(rqst[i]) )   {

      satfd++;

    }

  }

  UNLOCK(me);

  if(satfd == cnt) {

    *flag = TRUE;

    for (i = 0; i < cnt; i++) {

      if((rqst[i] == AZQ_RQST_NULL) || RQST_isInactive(rqst[i]))  {

        RQST_setEmptyStatus(&status[i]);

      } else {

        RQST_getStatus(rqst[i], &status[i]);

        if (RQST_isPersistent(rqst[i])) { RQST_clear(rqst[i]);   }
        else                            { RQST_destroy(rqst[i]); }

      }

    }

  }

  DBG_PRNT((stdout, "testall(%p): End\n", me));
  return AZQ_SUCCESS;

exception:
  XPN_print(excpn);
  return(excpn);
}
