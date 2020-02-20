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
    |    testsome                                                        |
    |    Test if some async request has finished                         |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o rqst (Input)                                                  |
    |        The tested asynchronous array of requests previously done   |
    |    o incnt (Input)                                                 |
    |        How many requests to test                                   |
    |    o outcnt (Output)                                               |
    |        How many requests completed                                 |
    |    o indices (Output)                                              |
    |        Requests completes in array of requests                     |
    |    o status (Output)                                               |
    |        Statuses of requests completed                              |
    |                                                                    |
    |    RETURN:                                                         |
    |    = 0 : On success                                                |
    |    < 0 : On other case                                             |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int testsome (Rqst_t *rqst, int incnt, int *outcnt, int *indices, Status *status) {

  int      i,
           j           = 0,
           excpn       = AZQ_SUCCESS;
  int      satfdrqst   = 0,
           nullrqst    = 0;
  Thr_t    me          = self();
  static
  char    *where       = "testsome";


  DBG_PRNT((stdout, "\ntestsome(%p): \n", me));

  if(rqst == (Rqst_t *)AZQ_RQST_NULL)                                          {excpn = AZQ_E_REQUEST;
                                                                                goto exception;}
  LOCK(me);

  for(i = 0; i < incnt; i++) {

    if( (rqst[i] == AZQ_RQST_NULL)    ||
         RQST_isInactive(rqst[i])     ||
         RQST_isCancelled(rqst[i]) )   {

      nullrqst++;

      if (nullrqst == incnt) {
        *outcnt = AZQ_UNDEFINED;
        UNLOCK(me);
        return AZQ_SUCCESS;
      }

    } else {

      if(RQST_isSatisfied(rqst[i])) {
        satfdrqst++;
        indices[j++] = i;
      }

    }

  }

  if(satfdrqst == 0) {

    *outcnt = 0;

  } else { /* some requests satisfied */

    *outcnt = j;

    for (j = 0; j < *outcnt; j++) {

      i = indices[j];

      RQST_getStatus(rqst[i], &status[j]);

      if (RQST_isPersistent(rqst[i])) { RQST_clear(rqst[i]);   }
      else                            { RQST_destroy(rqst[i]); }

    }

  }

  UNLOCK(me);

  DBG_PRNT((stdout, "testsome(%p): End\n", me));
  return AZQ_SUCCESS;

exception:
  XPN_print(excpn);
  return(excpn);
}
